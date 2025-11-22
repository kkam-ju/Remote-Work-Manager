# ---------------------------------------------------------------------------
# [중요] gevent는 다른 라이브러리보다 먼저 임포트하고 패치해야 합니다.
# ---------------------------------------------------------------------------
from gevent import monkey
monkey.patch_all()

from flask import Flask, render_template, request, jsonify, session, redirect, url_for
from flask_socketio import SocketIO, emit, join_room
import base64
import requests
from functools import wraps
import pymysql
import threading
from datetime import datetime, timedelta, date
import jwt
import ssl
from gevent.pywsgi import WSGIServer
from flask import send_from_directory
from geventwebsocket.handler import WebSocketHandler
import socket # ✨ 1. LED 서버와 통신하기 위한 표준 소켓 라이브러리 임포트

# --- 1. 기본 설정 ---
app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-very-secret-key-for-session-management'

# --- MySQL DB 설정 ---
app.config['MYSQL_HOST'] = 'localhost'
app.config['MYSQL_USER'] = 'root'
app.config['MYSQL_PASSWORD'] = 'moble'
app.config['MYSQL_DB'] = 'yolo_webapp_db'

# [성능 개선] async_mode를 'gevent'로 명시적으로 설정
socketio = SocketIO(app, async_mode='gevent')

# --- 외부 서버 정보 ---
RPI_DETECT_URL = "http://192.168.0.104:18080/detect"
RPI_REGISTER_URL = "http://192.168.0.104:18080/register"

# ✨ 2. 라즈베리파이 LED 제어 서버 정보
# 💡 [필수] C++ TCP 서버가 실행 중인 라즈베리파이의 IP 주소를 정확하게 입력하세요!
RPI_LED_SERVER_HOST = '192.168.0.94' # C++ 서버의 IP 주소
RPI_LED_SERVER_PORT = 8080           # C++ 서버가 사용하는 포트

# --- 근무 시간 설정 ---
STANDARD_WORK_MINUTES = 5  # 기준 근무 시간(분)
STANDARD_WORK_SECONDS = STANDARD_WORK_MINUTES * 60 # 기준 근무 시간(초)
STANDARD_WORK_HOURS = STANDARD_WORK_MINUTES / 60  # 기준 근무 시간(시간 단위)

# --- 상태 관리를 위한 전역 변수 ---
yolo_lock = threading.Lock()
pi_socket_lock = threading.Lock() # ✨ LED 서버 동시 접근을 막기 위한 잠금장치
sid_to_user = {}
active_yolo_target_username = None


# --- 2. 헬퍼 함수 및 데코레이터 ---

# ✨ 3. [수정됨] 라즈베리파이 LED 서버 소켓 통신 함수
def send_command_to_pi(command):
    """
    라즈베리파이의 C++ LED 서버에 일반 TCP 소켓으로 연결하여 명령어를 전송합니다.
    """
    with pi_socket_lock: # 한 번에 하나의 스레드만 이 코드를 실행하도록 보장
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.settimeout(2.0)
                print(f"Connecting to RPi LED server at {RPI_LED_SERVER_HOST}:{RPI_LED_SERVER_PORT}...")
                s.connect((RPI_LED_SERVER_HOST, RPI_LED_SERVER_PORT))
                
                print(f"✅ Sending command to RPi: '{command}'")
                s.sendall(command.encode('utf-8'))
                
                response = s.recv(1024)
                print(f"✅ Received response from RPi: '{response.decode('utf-8')}'")

        except socket.timeout:
            print(f"❌ ERROR: Connection to RPi server timed out.")
        except ConnectionRefusedError:
            print(f"❌ ERROR: Connection refused. Is the C++ server running on the RPi?")
        except Exception as e:
            print(f"❌ ERROR: An unexpected error occurred while sending command to RPi: {e}")

def get_db_connection():
    """데이터베이스 커넥션을 생성하여 반환합니다."""
    return pymysql.connect(
        host=app.config['MYSQL_HOST'],
        user=app.config['MYSQL_USER'],
        password=app.config['MYSQL_PASSWORD'],
        db=app.config['MYSQL_DB'],
        charset='utf8mb4',
        cursorclass=pymysql.cursors.DictCursor
    )

def login_required(f):
    """로그인 여부를 확인하는 데코레이터."""
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if 'user_id' not in session or 'full_name' not in session:
            session.clear()
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return decorated_function

def admin_required(f):
    """관리자 계정인지 확인하는 데코레이터."""
    @wraps(f)
    def decorated_function(*args, **kwargs):
        if 'user_id' not in session or 'full_name' not in session or not session.get('is_admin'):
            session.clear()
            return redirect(url_for('login'))
        return f(*args, **kwargs)
    return decorated_function


# --- 3. 페이지 라우팅 --- (이하 코드는 기존과 동일)

@app.route('/')
def home():
    if 'user_id' in session:
        return redirect(url_for('admin_page') if session.get('is_admin') else url_for('employee_page'))
    return redirect(url_for('login'))

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('password')
        login_type = request.form.get('login_type')
        conn = get_db_connection()
        try:
            with conn.cursor() as cursor:
                sql = "SELECT u.*, e.full_name, e.is_admin FROM users u JOIN employees e ON u.employee_id = e.employee_id WHERE u.username = %s"
                cursor.execute(sql, (username,))
                user = cursor.fetchone()

                if user and user['password'] == password:
                    actual_is_admin = user['is_admin'] == 1
                    selected_is_admin = login_type == 'admin'

                    if actual_is_admin != selected_is_admin:
                        return render_template('login.html', error='선택한 계정 유형이 올바르지 않습니다.')
                    
                    session['user_id'] = user['user_id']
                    session['full_name'] = user['full_name']
                    session['username'] = user['username'] 
                    session['is_admin'] = actual_is_admin
                    
                    return redirect(url_for('admin_page') if actual_is_admin else url_for('employee_page'))
                else:
                    return render_template('login.html', error='아이디 또는 비밀번호가 잘못되었습니다.')
        finally:
            conn.close()
    return render_template('login.html')

@app.route('/signup', methods=['GET', 'POST'])
def signup():
    if request.method == 'POST':
        data = request.json
        conn = get_db_connection()
        try:
            with conn.cursor() as cursor:
                cursor.execute("SELECT * FROM employees WHERE employee_number = %s", (data.get('employee_number'),))
                employee = cursor.fetchone()
                if not employee: return jsonify({'success': False, 'message': '존재하지 않는 사원 번호입니다.'})
                if employee['is_registered']: return jsonify({'success': False, 'message': '이미 등록된 사원입니다.'})
                
                cursor.execute("SELECT * FROM users WHERE username = %s OR email = %s", (data.get('username'), data.get('email')))
                if cursor.fetchone(): return jsonify({'success': False, 'message': '이미 사용 중인 사용자 이름 또는 이메일입니다.'})
                
                insert_sql = "INSERT INTO users (employee_id, username, password, email, phone_number, status) VALUES (%s, %s, %s, %s, %s, 'offline')"
                cursor.execute(insert_sql, (employee['employee_id'], data.get('username'), data.get('password'), data.get('email'), data.get('phone_number')))
                
                update_sql = "UPDATE employees SET full_name = %s, is_registered = 1 WHERE employee_id = %s"
                cursor.execute(update_sql, (data.get('full_name'), employee['employee_id']))
            conn.commit()
            return jsonify({'success': True, 'message': '등록 성공! 로그인 페이지로 이동합니다.'})
        except Exception as e:
            conn.rollback()
            return jsonify({'success': False, 'message': f'데이터베이스 오류: {e}'})
        finally:
            conn.close()
    return render_template('register.html')

@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('login'))

@app.route('/admin')
@admin_required
def admin_page():
    try:
        payload = {
            'user_id': session['user_id'],
            'exp': datetime.utcnow() + timedelta(seconds=60)
        }
        mfc_token = jwt.encode(payload, app.config['SECRET_KEY'], algorithm='HS256')
    except Exception as e:
        mfc_token = None
        print(f"Admin page token generation error: {e}")

    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            sql = "SELECT u.username, e.full_name, u.status FROM users u JOIN employees e ON u.employee_id = e.employee_id ORDER BY u.username"
            cursor.execute(sql)
            all_users = cursor.fetchall()
    finally:
        conn.close()
    
    return render_template('admin.html', 
                           all_users=all_users, 
                           display_name=session.get('full_name'),
                           mfc_token=mfc_token)

@app.route('/employee')
@login_required
def employee_page():
    try:
        payload = {
            'user_id': session['user_id'],
            'exp': datetime.utcnow() + timedelta(seconds=60)
        }
        mfc_token = jwt.encode(payload, app.config['SECRET_KEY'], algorithm='HS256')
    except Exception as e:
        mfc_token = None
        print(f"Token generation error: {e}")
    
    return render_template('employee.html', 
                           username=session.get('username'),
                           display_name=session.get('full_name'),
                           mfc_token=mfc_token)

@app.route('/register-face')
@login_required
def register_face():
    return render_template('registerface.html')


# --- 4. API 라우팅 ---

@app.route('/api/work_hours/<username>')
@admin_required
def get_work_hours(username):
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT user_id FROM users WHERE username = %s", (username,))
            user = cursor.fetchone()
            if not user: return jsonify({'error': 'User not found'}), 404
            user_id = user['user_id']

            today = date.today()
            sql = "SELECT start_time, end_time FROM work_logs WHERE user_id = %s AND DATE(start_time) = %s"
            cursor.execute(sql, (user_id, today))
            logs = cursor.fetchall()

            total_worked_seconds = 0
            for log in logs:
                start_time = log['start_time']
                end_time = log['end_time'] if log['end_time'] else datetime.now()
                total_worked_seconds += (end_time - start_time).total_seconds()
            
            worked_hours = total_worked_seconds / 3600
    except Exception as e:
        print(f"Error in get_work_hours for user '{username}': {e}")
        return jsonify({'error': 'Internal server error'}), 500
    finally:
        conn.close()

    total_work_hours = STANDARD_WORK_HOURS
    remaining_hours = max(0, total_work_hours - worked_hours)
    overtime_hours = max(0, worked_hours - total_work_hours)

    return jsonify({
        'worked_hours': round(worked_hours, 2), 
        'remaining_hours': round(remaining_hours, 2),
        'overtime_hours': round(overtime_hours, 2)
    })

@app.route('/api/my_work_hours')
@login_required
def get_my_work_hours():
    if session.get('is_admin'):
        return jsonify({'worked_hours': 0, 'remaining_hours': STANDARD_WORK_HOURS})

    user_id = session.get('user_id')
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            today = date.today()
            sql = "SELECT start_time, end_time FROM work_logs WHERE user_id = %s AND DATE(start_time) = %s"
            cursor.execute(sql, (user_id, today))
            logs = cursor.fetchall()
            total_worked_seconds = 0
            for log in logs:
                start_time = log['start_time']
                end_time = log['end_time'] if log['end_time'] else datetime.now()
                total_worked_seconds += (end_time - start_time).total_seconds()
            worked_hours = total_worked_seconds / 3600
    except Exception as e:
        worked_hours = 0
    finally:
        conn.close()

    total_work_hours = STANDARD_WORK_HOURS
    remaining_hours = max(0, total_work_hours - worked_hours)
    return jsonify({'worked_hours': round(worked_hours, 2), 'remaining_hours': round(remaining_hours, 2)})

# ✨✨✨✨✨ [수정됨] clock_in 함수 ✨✨✨✨✨
@app.route('/api/clock_in', methods=['POST'])
@login_required
def clock_in():
    if session.get('is_admin'):
        return jsonify({'success': False, 'message': '관리자는 출퇴근 기록을 할 수 없습니다.'}), 403

    user_id = session.get('user_id')
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            sql = "INSERT INTO work_logs (user_id, start_time) VALUES (%s, %s)"
            cursor.execute(sql, (user_id, datetime.now()))
        conn.commit()
        socketio.emit('work_log_updated', {'username': session.get('username')}, room='admins')

        # '근무 시작' 상태 명령 전송
        send_command_to_pi('work_start') 

        return jsonify({'success': True, 'message': '출근 처리되었습니다.'})
    except Exception as e:
        conn.rollback()
        return jsonify({'success': False, 'message': f'오류 발생: {e}'}), 500
    finally:
        conn.close()

# ✨✨✨✨✨ [수정됨] clock_out 함수 ✨✨✨✨✨
@app.route('/api/clock_out', methods=['POST'])
@login_required
def clock_out():
    if session.get('is_admin'):
        return jsonify({'success': False, 'message': '관리자는 출퇴근 기록을 할 수 없습니다.'}), 403

    user_id = session.get('user_id')
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            # 1. 퇴근 직전, 오늘 총 근무 시간 계산
            today = date.today()
            sql_select = "SELECT start_time, end_time FROM work_logs WHERE user_id = %s AND DATE(start_time) = %s"
            cursor.execute(sql_select, (user_id, today))
            logs = cursor.fetchall()
            
            total_worked_seconds = 0
            # 현재 진행중인 로그를 포함하여 총 근무시간 계산
            for log in logs:
                start_time = log['start_time']
                end_time = log['end_time'] if log['end_time'] else datetime.now()
                total_worked_seconds += (end_time - start_time).total_seconds()

            # 2. 퇴근 시간 DB에 업데이트
            sql_update = "UPDATE work_logs SET end_time = %s WHERE user_id = %s AND DATE(start_time) = %s AND end_time IS NULL ORDER BY start_time DESC LIMIT 1"
            cursor.execute(sql_update, (datetime.now(), user_id, today))

        conn.commit()
        socketio.emit('work_log_updated', {'username': session.get('username')}, room='admins')
        
        # 3. 계산된 근무 시간에 따라 다른 명령 전송
        if total_worked_seconds >= STANDARD_WORK_SECONDS:
            print(f"기준 시간({STANDARD_WORK_SECONDS}초) 이상 근무. 목표 달성(green) 신호 전송.")
            send_command_to_pi('work_complete') # 5분 이상 근무 -> 초록불
        else:
            print(f"기준 시간({STANDARD_WORK_SECONDS}초) 미만 근무. 업무 종료(off) 신호 전송.")
            send_command_to_pi('work_end')      # 5분 미만 근무 -> 모든 불 끄기

        return jsonify({'success': True, 'message': '퇴근 처리되었습니다.'})
    except Exception as e:
        conn.rollback()
        return jsonify({'success': False, 'message': f'오류 발생: {e}'}), 500
    finally:
        conn.close()

# ✨✨✨✨✨ [수정됨] set_status 함수 ✨✨✨✨✨
@app.route('/api/set_status', methods=['POST'])
@login_required
def set_status():
    data = request.get_json()
    status = data.get('status')
    
    # C++ 서버의 상태 명령어에 맞게 매핑
    command_map = {
        'away': 'work_away',      # '자리 비움' 상태
        'working': 'work_resume', # '업무 복귀' 상태
        'goal_met': 'work_complete' # (사용자 직접 클릭 시) '목표 달성' 상태
    }
    
    if status in command_map:
        send_command_to_pi(command_map[status]) # 라즈베리파이에 상태 명령 전송
    else:
        return jsonify({'message': 'Invalid status'}), 400
        
    return jsonify({'message': f'Status updated to {status}'}), 200

@app.route('/api/trigger-retrain', methods=['POST'])
@login_required
def trigger_retrain():
    try:
        retrain_url = RPI_DETECT_URL.replace("/detect", "/retrain")
        res = requests.post(retrain_url, timeout=120)
        return jsonify(res.json()), res.status_code
    except requests.exceptions.RequestException as e:
        return jsonify({'success': False, 'message': '모델 학습 서버에 연결할 수 없습니다.'}), 503
    except Exception as e:
        return jsonify({'success': False, 'message': f'내부 서버 오류: {e}'}), 500

@app.route('/api/register-face-capture', methods=['POST'])
@login_required
def register_face_capture():
    try:
        data = request.get_json()
        image_data = data.get('image')
        username = session.get('username')
        if not image_data or not username:
            return jsonify({'success': False, 'message': '필수 데이터가 누락되었습니다.'}), 400

        header, encoded = image_data.split(',', 1)
        image_bytes = base64.b64decode(encoded)

        payload = {'username': (None, username), 'image': ('face.jpg', image_bytes, 'image/jpeg')}
        res = requests.post(RPI_REGISTER_URL, files=payload, timeout=10)
        return jsonify(res.json()), res.status_code
    except requests.exceptions.RequestException as e:
        return jsonify({'success': False, 'message': '라즈베리파이 서버에 연결할 수 없습니다.'}), 503
    except Exception as e:
        return jsonify({'success': False, 'message': f'내부 서버 오류가 발생했습니다: {e}'}), 500

@app.route("/api/get_statuses")
@admin_required
def get_statuses():
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("SELECT username, status FROM users")
            users = cursor.fetchall()
            user_statuses = {user['username']: user['status'] for user in users}
            return jsonify(user_statuses)
    finally:
        conn.close()

@app.route("/api/update_status", methods=["POST"])
@login_required
def update_status():
    new_status = request.json.get('status')
    user_id = session.get('user_id')
    conn = get_db_connection()
    try:
        with conn.cursor() as cursor:
            cursor.execute("UPDATE users SET status = %s WHERE user_id = %s", (new_status, user_id))
        conn.commit()
        return jsonify({"status": "success", "message": f"상태가 '{new_status}'로 변경되었습니다."})
    finally:
        conn.close()

@app.route('/api/test')
def api_test():
    return jsonify({'status': 'ok', 'message': 'Flask 서버가 성공적으로 응답했습니다!'})

@app.route('/download/installer')
def download_installer():
    try:
        return send_from_directory(
            directory='static/setups', 
            path='ThemoonSetup.msi',
            as_attachment=True
        )
    except FileNotFoundError:
        return "설치 파일을 찾을 수 없습니다.", 404
        
@app.route('/api/mfc/verify-token', methods=['POST'])
def verify_mfc_token():
    data = request.get_json()
    token = data.get('token')
    if not token:
        return jsonify({'success': False, 'message': '토큰이 없습니다.'})

    try:
        payload = jwt.decode(token, app.config['SECRET_KEY'], algorithms=['HS256'])
        user_id = payload['user_id']

        conn = get_db_connection()
        with conn.cursor() as cursor:
            sql = "SELECT u.*, e.full_name, e.is_admin FROM users u JOIN employees e ON u.employee_id = e.employee_id WHERE u.user_id = %s"
            cursor.execute(sql, (user_id,))
            user = cursor.fetchone()
        conn.close()

        if not user:
            return jsonify({'success': False, 'message': '사용자를 찾을 수 없습니다.'})

        user_info = {
            'user_id': user['user_id'],
            'username': user['username'],
            'full_name': user['full_name'],
            'is_admin': user['is_admin'] == 1
        }
        return jsonify({'success': True, 'user_info': user_info})

    except jwt.ExpiredSignatureError:
        return jsonify({'success': False, 'message': '토큰이 만료되었습니다.'})
    except jwt.InvalidTokenError:
        return jsonify({'success': False, 'message': '유효하지 않은 토큰입니다.'})


# --- 5. Socket.IO 이벤트 핸들러 ---

@socketio.on('connect')
@login_required
def handle_connect():
    username = session.get('username')
    sid = request.sid
    sid_to_user[sid] = username
    
    if session.get('is_admin'):
        join_room('admins')
        print(f"Admin connected: {username} ({sid}), joined 'admins' room.")
    else:
        print(f"Employee connected: {username} ({sid}).")

@socketio.on('disconnect')
def handle_disconnect():
    global active_yolo_target_username
    sid = request.sid
    if sid in sid_to_user:
        username = sid_to_user.pop(sid)
        print(f"Client disconnected: {username} ({sid}).")
        if username == active_yolo_target_username:
            active_yolo_target_username = None
            print(f"Active target '{username}' disconnected. Target reset.")

@socketio.on('set_active_target')
@admin_required
def set_active_target(data):
    global active_yolo_target_username
    username = data.get('username')
    active_yolo_target_username = username
    print(f"Admin set new target: {username}")
    emit('target_changed', {'username': username}, room='admins')

@socketio.on('unattended_workstation')
@login_required
def handle_unattended_alert():
    """직원 클라이언트로부터 근무 이탈 신호를 받아 관리자에게 알림을 보냅니다."""
    full_name = session.get('full_name', '알 수 없는 사용자')
    message = f"🚨 {full_name} 님이 근무 중 자리를 이탈했습니다."
    print(f"관리자에게 알림 전송: {message}")
    emit('workstation_alert', {'message': message}, room='admins')

@socketio.on('image')
@login_required
def handle_image(data_image):
    if session.get('is_admin'):
        return
    
    image_sender_username = session.get('username') 
    if not image_sender_username:
        return

    if image_sender_username == active_yolo_target_username:
        emit('raw_target_feed', data_image, room='admins')

    if not yolo_lock.acquire(blocking=False):
        emit('drowsiness_update', {'image_data': '', 'is_drowsy': False, 'face_detected': True}, to=request.sid)
        return

    try:
        img_bytes = base64.b64decode(data_image.split(',')[1])
        payload = {
            'target_username': (None, image_sender_username),
            'image': ('image.jpg', img_bytes, 'image/jpeg')
        }
        res = requests.post(RPI_DETECT_URL, files=payload, timeout=1)
        
        result_image = ''
        is_drowsy = False
        face_detected = True
        if res.status_code == 200:
            response_data = res.json()
            result_image = response_data.get('image', '')
            is_drowsy = response_data.get('is_drowsy', False)
            face_detected = response_data.get('face_detected', True)
        
        emit('drowsiness_update', {
            'image_data': result_image, 
            'is_drowsy': is_drowsy, 
            'face_detected': face_detected
        }, to=request.sid)

    except requests.exceptions.RequestException:
        emit('drowsiness_update', {'image_data': '', 'is_drowsy': False, 'face_detected': True}, to=request.sid)
    finally:
        yolo_lock.release()

@socketio.on('work_log_updated')
@admin_required
def on_work_log_updated(data):
    emit('work_log_updated', data, room='admins', broadcast=True)

@socketio.on('joystick_move')
def handle_joystick_move(data):
    """
    웹 관리자 페이지로부터 조이스틱 데이터를 받아
    다른 Socket.IO 클라이언트(서보모터 제어용 RPi)로 전달합니다.
    """
    try:
        processed_data = {
            'x': float(data.get('x', 0.0)),
            'y': float(data.get('y', 0.0))
        }
        print(f"Joystick data received: {data}. Forwarding to RPi (servo): {processed_data}")
        # 'move_servo' 이벤트를 모든 클라이언트에게 브로드캐스트
        emit('move_servo', processed_data, broadcast=True)

    except (ValueError, TypeError) as e:
        print(f"ERROR: Invalid joystick data received. data: {data}, error: {e}")

# --- 6. 서버 실행 ---
def get_ip_address():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return "127.0.0.1"

if __name__ == '__main__':
    print("Starting server with gevent-websocket...")
    
    ip_address = get_ip_address()
    
    try:
        # HTTPS를 위한 SSL 컨텍스트. cert.pem과 key.pem 파일이 필요합니다.
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain('cert.pem', 'key.pem')
        
        server = WSGIServer(('0.0.0.0', 5000), app,
                            handler_class=WebSocketHandler,
                            ssl_context=context)
                            
        print(f"✅ Server is running securely on https://{ip_address}:5000")
        server.serve_forever()

    except FileNotFoundError:
        # SSL 인증서 파일이 없을 경우, HTTP로 서버를 실행합니다. (개발용)
        print("[WARNING] SSL certificate files (cert.pem, key.pem) not found.")
        print("[WARNING] Starting server without HTTPS. This is not secure for production.")
        
        server = WSGIServer(('0.0.0.0', 5000), app, handler_class=WebSocketHandler)

        print(f"✅ Server is running on http://{ip_address}:5000")
        server.serve_forever()