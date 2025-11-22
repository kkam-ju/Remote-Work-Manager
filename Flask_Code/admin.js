// static/js/admin.js 파일의 모든 내용을 이 코드로 교체하세요.
document.addEventListener('DOMContentLoaded', function () {
    // 필수 HTML 요소들을 먼저 찾습니다.
    const employeeList = document.getElementById('employee-list');
    const videoFeed = document.getElementById('video-feed');
    const statusTitle = document.getElementById('status-title');
    const welcomeMessage = document.getElementById('welcome-message');
    const subMessage = document.getElementById('sub-message');
    const refreshBtn = document.getElementById('refresh-work-status-btn');
    const joystickBase = document.getElementById('joystick-base');
    const joystickHandle = document.getElementById('joystick-handle');
    
    // ✨ [수정] 시간 및 차트 관련 요소 추가
    const workedTimeEl = document.getElementById('worked-time');
    const extraTimeLabelEl = document.getElementById('extra-time-label');
    const extraTimeValueEl = document.getElementById('extra-time-value');
    const chartCanvas = document.getElementById('adminWorkChart'); // 차트 캔버스
    
    // 필수 요소가 하나라도 없으면 스크립트를 중단합니다.
    if (!employeeList || !videoFeed || !statusTitle || !refreshBtn || !joystickBase || !joystickHandle || !chartCanvas) {
        console.error("오류: HTML에서 필수 요소를 찾을 수 없습니다. ID가 올바른지 확인하세요.");
        return;
    }

    const socket = io.connect(location.protocol + '//' + document.domain + ':' + location.port);
    let activeUsername = null;
    let activeDisplayName = null;
    const STANDARD_WORK_SECONDS = 5 * 60; // 기준 근무 시간 (초)
    let adminWorkChart; // 차트 인스턴스를 저장할 변수

    function showNotification(message) {
        const container = document.getElementById('notification-container');
        if (!container) return;

        const notif = document.createElement('div');
        notif.className = 'notification-toast';
        notif.textContent = message;
        container.appendChild(notif);

        // 애니메이션을 위해 약간의 딜레이 후 'show' 클래스 추가
        setTimeout(() => notif.classList.add('show'), 100);

        // 5초 후에 알림을 서서히 사라지게 하고 DOM에서 제거
        setTimeout(() => {
            notif.classList.remove('show');
            notif.addEventListener('transitionend', () => notif.remove());
        }, 5000);
    }

    // ✨ [신규] 차트를 생성하거나 업데이트하는 함수
    function createOrUpdateAdminChart(workedSeconds, remainingSeconds) {
        const data = [workedSeconds, remainingSeconds];
        const labels = ['근무 시간', '남은 시간'];
        const colors = ['#28a745', '#e9ecef'];

        if (adminWorkChart) {
            // 차트가 이미 있으면 데이터만 업데이트
            adminWorkChart.data.datasets[0].data = data;
            adminWorkChart.update('none'); 
        } else {
            // 차트가 없으면 새로 생성
            adminWorkChart = new Chart(chartCanvas, {
                type: 'doughnut',
                data: {
                    labels: labels,
                    datasets: [{
                        data: data,
                        backgroundColor: colors,
                        borderWidth: 0
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    cutout: '70%',
                    plugins: {
                        legend: { display: false },
                        tooltip: { enabled: false }
                    }
                }
            });
        }
    }

    // ✨ [개선] 시간 텍스트와 차트를 모두 업데이트하는 통합 함수
    function updateTimeDisplay(data) {
        const workedSeconds = Math.round((data.worked_hours || 0) * 3600);
        const overtimeSeconds = Math.round((data.overtime_hours || 0) * 3600);
        
        // 1. 총 근무 시간 텍스트 업데이트
        workedTimeEl.textContent = formatTime(workedSeconds);

        // 2. 남은 시간 또는 초과 근무 시간 텍스트 업데이트
        if (overtimeSeconds > 0) {
            extraTimeLabelEl.textContent = '초과 근무';
            extraTimeValueEl.textContent = formatTime(overtimeSeconds);
            extraTimeValueEl.classList.add('overtime');
        } else {
            const remainingSeconds = Math.max(0, STANDARD_WORK_SECONDS - workedSeconds);
            extraTimeLabelEl.textContent = '남은 시간';
            extraTimeValueEl.textContent = formatTime(remainingSeconds);
            extraTimeValueEl.classList.remove('overtime');
        }
        
        // 3. 차트 업데이트
        const remainingForChart = Math.max(0, STANDARD_WORK_SECONDS - workedSeconds);
        createOrUpdateAdminChart(workedSeconds, remainingForChart);
    }

    // 서버에서 데이터를 가져와 대시보드를 업데이트합니다.
    async function fetchAndUpdateDashboard(username, displayName) {
        if (!username) return;
        statusTitle.textContent = `${displayName} 근무 현황`;
        welcomeMessage.textContent = `[ ${displayName} ] 님 현황`;
        subMessage.textContent = `실시간 영상과 근무 현황을 확인합니다.`;
        try {
            const response = await fetch(`/api/work_hours/${username}`);
            if (!response.ok) throw new Error('Server response was not ok');
            const data = await response.json();
            // ✨ [수정] 개선된 함수 호출
            updateTimeDisplay(data);
        } catch (error) {
            console.error(`Error fetching work hours for ${username}:`, error);
            updateTimeDisplay({ worked_hours: 0, overtime_hours: 0 }); // 에러 발생 시 초기화
            statusTitle.textContent = "데이터 로딩 실패";
        }
    }

    // 직원 목록 클릭 이벤트
    employeeList.addEventListener('click', function (event) {
        const li = event.target.closest('li');
        if (li && li.dataset.username !== activeUsername) {
            employeeList.querySelectorAll('li').forEach(item => item.classList.remove('active'));
            li.classList.add('active');
            activeUsername = li.dataset.username;
            activeDisplayName = li.dataset.displayname;
            if (videoFeed) videoFeed.alt = `${activeDisplayName}(${activeUsername}) 님의 실시간 영상`;
            socket.emit('set_active_target', { 'username': activeUsername });
            fetchAndUpdateDashboard(activeUsername, activeDisplayName);
        }
    });

    // 조회 버튼 클릭 이벤트
    refreshBtn.addEventListener('click', function() {
        if (activeUsername) {
            fetchAndUpdateDashboard(activeUsername, activeDisplayName);
        } else {
            alert("먼저 직원을 선택해주세요.");
        }
    });
    
    // 소켓 이벤트 (영상 수신)
    socket.on('raw_target_feed', (image_data) => {
        if (activeUsername && videoFeed) {
            videoFeed.src = image_data;
        }
    });

    socket.on('workstation_alert', (data) => {
        if (data.message) {
            console.log("Workstation alert received:", data.message);
            showNotification(data.message);
        }
    });
    let isDragging = false;
    let joystickInterval = null; // [추가] 주기적으로 데이터를 보내기 위한 인터벌 ID
    let lastJoystickData = { x: 0, y: 0 }; // [추가] 마지막 조이스틱 좌표를 저장할 변수
    const JOYSTICK_SEND_INTERVAL = 100; // [추가] 100ms (0.1초) 간격으로 데이터 전송

    const drag = (event) => {
        if (!isDragging) return;
        event.preventDefault();

        const rect = joystickBase.getBoundingClientRect();
        const maxDistance = joystickBase.offsetWidth / 2;
        const clientX = event.clientX || event.touches[0].clientX;
        const clientY = event.clientY || event.touches[0].clientY;

        let dx = clientX - (rect.left + maxDistance);
        let dy = clientY - (rect.top + maxDistance);

        const distance = Math.min(maxDistance, Math.sqrt(dx * dx + dy * dy));
        const angle = Math.atan2(dy, dx);

        const x = distance * Math.cos(angle);
        const y = distance * Math.sin(angle);

        joystickHandle.style.transform = `translate(${x}px, ${y}px)`;

        // [변경] socket.emit을 직접 호출하는 대신, 마지막 좌표를 변수에 저장만 합니다.
        lastJoystickData = { 
            x: (x / maxDistance).toFixed(3), 
            y: (-y / maxDistance).toFixed(3) // Y축은 화면 좌표계와 반대이므로 -1 곱함
        };
    };

    const startDrag = (event) => {
        isDragging = true;
        joystickHandle.style.transition = 'none';
        if (event.type === 'touchstart') event.preventDefault();

        // [추가] 드래그를 시작하면, 설정된 간격(JOYSTICK_SEND_INTERVAL)마다 데이터 전송을 시작합니다.
        if (!joystickInterval) {
            joystickInterval = setInterval(() => {
                socket.emit('joystick_move', lastJoystickData);
            }, JOYSTICK_SEND_INTERVAL);
        }
    };

    const stopDrag = () => {
        if (!isDragging) return;
        isDragging = false;

        // [추가] 주기적인 전송을 멈춥니다.
        clearInterval(joystickInterval);
        joystickInterval = null;

        joystickHandle.style.transition = 'transform 0.2s ease-out';
        joystickHandle.style.transform = `translate(0px, 0px)`;
        
        // [변경] 마지막으로 초기화 데이터를 보내고, 저장된 좌표도 초기화합니다.
        lastJoystickData = { x: 0, y: 0 };
        socket.emit('joystick_move', lastJoystickData);
    };

    // 기존 이벤트 리스너를 새로운 함수에 맞게 수정합니다.
    joystickHandle.addEventListener('mousedown', startDrag);
    document.addEventListener('mousemove', drag);
    document.addEventListener('mouseup', stopDrag);
    joystickHandle.addEventListener('touchstart', startDrag);
    document.addEventListener('touchmove', drag, { passive: false });
    document.addEventListener('touchend', stopDrag);
    
    // 시간 포맷 함수 (기존과 동일)
    function formatTime(totalSeconds) {
        const h = String(Math.floor(totalSeconds / 3600)).padStart(2, '0');
        const m = String(Math.floor((totalSeconds % 3600) / 60)).padStart(2, '0');
        const s = String(Math.floor(totalSeconds % 60)).padStart(2, '0');
        return `${h}:${m}:${s}`;
    }

    // 페이지 초기화: 첫 번째 직원을 자동으로 선택하고 데이터를 로드
    const firstEmployee = employeeList.querySelector('li');
    if (firstEmployee) {
        firstEmployee.click();
    } else {
        // 직원이 한 명도 없을 경우 초기 상태 설정
        updateTimeDisplay({ worked_hours: 0, overtime_hours: 0 });
    }
});