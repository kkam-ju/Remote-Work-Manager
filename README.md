# 🏠 Remote Work Manager (스마트 원격 근무 관리 시스템)

> **Flask 웹 서버, Windows MFC, IoT(Raspberry Pi), 그리고 MySQL을 연동한 종합 원격 근무 관리 솔루션**

이 프로젝트는 재택근무 및 원격 근무 환경에서의 업무 효율성을 높이고, 물리적으로 떨어져 있는 직원들 간의 원활한 소통과 협업을 지원하기 위해 개발되었습니다.
기본적인 **근태 관리**와 **AI 기반 모니터링**뿐만 아니라, **실시간 채팅, 업무 할당, 사내 게시판** 등 다양한 협업 도구를 하나의 플랫폼으로 통합하여 완벽한 가상 오피스 환경을 제공합니다.

---

## ✨ 주요 기능 (Key Features)

### 1️⃣ 근무 관리 및 모니터링 (Work Management)
* **출퇴근 기록**: 웹 대시보드 및 MFC 클라이언트를 통한 정밀한 근무 시간 기록 및 초과 근무 자동 계산
* **AI 기반 감지**: YOLO 모델을 활용하여 졸음 및 이석(자리 비움) 감지 시 관리자에게 즉시 알림 전송
* **IoT 상태 연동**: 근무 상태(집중, 휴식, 퇴근)에 따라 라즈베리파이 LED 조명을 제어하고, 서보모터를 통해 원격 카메라 각도를 조절

### 2️⃣ 협업 및 소통 (Collaboration & Communication)
* **실시간 채팅 시스템**: 부서별 그룹 채팅방 지원 및 직원 간 1:1 다이렉트 메시지(DM) 기능 제공
* **업무 요청 및 할당**: 마감 기한(Due Date)이 포함된 업무 요청을 등록하고, 담당자를 지정하여 진행 상황(접수, 처리중, 완료)을 추적 관리
* **커뮤니티 기능**:
    * **공지사항**: 관리자가 전사 공지 등록 및 파일 첨부 가능
    * **익명 게시판**: 직원들이 자유롭게 소통할 수 있는 익명 게시글 및 댓글 기능

### 3️⃣ 보안 및 시스템 (Security & System)
* **계정 권한 분리**: 관리자(Admin)와 일반 사원(Employee)의 권한을 철저히 분리하여 메뉴 접근 제어
* **보안 통신**: SSL 인증서를 적용한 HTTPS 통신 및 JWT 토큰 기반의 API 인증

---

## 🛠️ 사용 기술 스택 (Tech Stack)

| 분류 | 기술 | 설명 |
| :--- | :--- | :--- |
| **Backend** | **Python (Flask)** | Socket.IO 실시간 통신, REST API, 비즈니스 로직 처리 |
| **Database** | **MySQL** | 관계형 데이터베이스 설계 (ERD 기반의 유저, 채팅, 업무 데이터 관리) |
| **Frontend** | **HTML/CSS/JS** | 반응형 웹 대시보드, 관리자 페이지, 화상 채팅 UI |
| **Client** | **C++ (MFC)** | Windows 데스크톱 전용 메신저 및 근태 관리 클라이언트 |
| **IoT / AI** | **Raspberry Pi, YOLO** | 객체 인식 AI 서버 및 하드웨어 제어 (TCP/IP 소켓 통신) |

---

## 💾 데이터베이스 구조 (Database Schema)

본 프로젝트는 `yolo_webapp_db` 스키마를 사용하며, 주요 테이블은 다음과 같습니다.

* **사용자 관리**: `users`, `employees` (사원 정보 및 로그인 계정)
* **근태 관리**: `work_logs` (출퇴근 시간 및 근무 내역 기록)
* **채팅 시스템**: `chat_rooms` (채팅방), `participants` (참여자), `messages` (대화 내역)
* **게시판**: `notices` (공지), `posts` (익명글), `comments` (댓글), `attachments` (첨부파일)
* **업무 협업**: `work_requests` (업무 요청서), `work_assignees` (업무 담당자)

---

## ⚙️ 설치 및 실행 가이드 (Setup Guide)

### 1. 데이터베이스 설정 (Database Setup)
MySQL 데이터베이스에 접속하여 제공된 SQL 파일을 순서대로 실행합니다.
1. **스키마 생성**: `DB/sql테이블 최종.sql` 실행 (테이블 생성)
2. **초기 데이터 삽입**: `DB/value값 최종.sql` 실행 (부서, 사원, 초기 채팅방 생성)

### 2. 서버 설정 (Server Configuration)
`Flask_Code/app.py` 파일에서 DB 접속 정보와 하드웨어 IP를 본인의 환경에 맞게 수정합니다.

```python
# app.py 내부 설정
app.config['MYSQL_HOST'] = 'localhost'
app.config['MYSQL_USER'] = 'root'
app.config['MYSQL_PASSWORD'] = 'your_password' # 본인의 DB 비밀번호 입력
app.config['MYSQL_DB'] = 'yolo_webapp_db'

# IoT 장비 IP 설정 (필요 시 수정)
RPI_DETECT_URL = "[http://192.168.0.104:18080/detect](http://192.168.0.104:18080/detect)"
RPI_LED_SERVER_HOST = '192.168.0.94'

## 📂 프로젝트 파일 구조 (File Structure)

이 프로젝트는 크게 **데이터베이스(DB)**, **웹 서버(Flask)**, **윈도우 클라이언트(MFC)**로 구성되어 있습니다.

### 1. 🗄️ Database (DB)
* **`sql테이블 최종.sql`**: 프로젝트에 필요한 모든 테이블(사원, 채팅, 업무, 로그 등)을 생성하는 DDL 스크립트
* **`value값 최종.sql`**: 부서 정보, 관리자 계정 등 초기 테스트 데이터를 삽입하는 DML 스크립트

### 2. 🖥️ Server (Flask_Code)
웹 대시보드 및 전체 시스템의 백엔드 로직을 담당합니다.
* **`app.py`**: 메인 서버 애플리케이션 파일
    * REST API (로그인, 데이터 조회) 및 Socket.IO (실시간 통신) 구현
    * MySQL DB 연동 및 Raspberry Pi(IoT) 제어 명령 전송
    * SSL 인증서(`cert.pem`, `key.pem`)를 이용한 HTTPS 보안 서버 구동
* **Frontend Resources (HTML/CSS/JS)**:
    * **`admin.*`**: 관리자용 통합 대시보드 (근태 모니터링, 사원 관리)
    * **`employee.*`**: 일반 사원용 페이지 (개인 근태 조회)
    * **`login.*`, `register.*`**: 사용자 인증 및 회원가입 페이지
    * **`registerface.*`**: AI 안면 인식 학습을 위한 얼굴 데이터 등록 페이지

### 3. 💻 Client (MFC_Code)
직원들이 PC에서 사용하는 Windows 전용 메신저 및 협업 프로그램입니다.
* **Core**:
    * **`TheMoon.cpp/h`**: 프로그램 진입점 및 애플리케이션 초기화
    * **`TheMoonDlg.cpp/h`**: 메인 컨테이너 다이얼로그 (화면 네비게이션 및 인증 토큰 관리)
* **Features (Dialogs)**:
    * **🏠 홈 (Home)**: `HomeDlg` (출퇴근 버튼, 나의 상태 변경, 공지사항 요약)
    * **💬 메신저 (Messenger)**: `MessengerDlg` (실시간 1:1 및 그룹 채팅)
    * **👥 사원 조회 (Friends)**: `FriendsListDlg` (조직도 확인 및 동료 정보 조회)
    * **📋 게시판 (Community)**: `CommunityDlg`, `WritePostDlg`, `PostDetailDlg` (익명 게시판 및 공지사항)
    * **💼 업무 결재 (Work)**: `NewWorkRequestDlg`, `WorkDetailDlg` (업무 기안 및 진행 상황 추적)
* **UI Components**:
    * **`ModernButton`, `ModernEdit`**: 세련된 디자인을 위한 커스텀 UI 컨트롤 클래스
