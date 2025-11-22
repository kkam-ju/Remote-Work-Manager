// static/js/employee.js (이 코드로 전체를 덮어쓰세요)

document.addEventListener('DOMContentLoaded', function () {

    // =======================================================
    // 1. 전역 변수 및 요소 초기화
    // =======================================================
    const video = document.getElementById('video');
    const canvas = document.getElementById('canvas');
    const processedImage = document.getElementById('processed_image');
    const socket = io();

    // 졸음 감지 관련
    const drowsyWarning = document.getElementById('drowsy-warning-overlay');
    const alarmSound = document.getElementById('alarm-sound');
    let drowsyTimer = null;
    let isActivelyDrowsy = false;

    // ✨ 근무 이탈 감지 관련
    let unattendedTimer = null;

    // 근무 상태 관련
    const clockInBtn = document.getElementById('clock-in-btn');
    const awayBtn = document.getElementById('away-btn');
    const clockOutBtn = document.getElementById('clock-out-btn');
    const chartCanvas = document.getElementById('myWorkChart');
    const totalTimerDisplay = document.getElementById('total-timer');
    const workTimerDisplay = document.getElementById('work-timer');
    const awayTimerDisplay = document.getElementById('away-timer');
    const timerContainer = document.getElementById('timer-display');
    
    let mainTimer = null;
    let totalSeconds = 0, workedSeconds = 0, awaySeconds = 0;
    let isWorking = false;
    let isAway = false;
    let myWorkChart;
    const TOTAL_WORK_GOAL_SECONDS = 5 * 60;

    let isProcessing = false;
    const context = canvas.getContext('2d');

    // =======================================================
    // 2. 실시간 영상 및 상태 처리
    // =======================================================
    socket.on('connect', () => console.log('Socket.IO 서버에 연결되었습니다.'));

    socket.on('drowsiness_update', function(data) {
        if (data.image_data) processedImage.src = data.image_data;
        isProcessing = false;

        if (isWorking) {
            handleDrowsiness(data.is_drowsy);
            handleUnattended(data.face_detected); // ✨ 근무 이탈 감지 함수 호출
        }
    });

    function handleDrowsiness(isDrowsyDetected) {
        if (isDrowsyDetected && !isActivelyDrowsy) {
            isActivelyDrowsy = true;
            console.warn("졸음 감지 시작!");
            drowsyWarning.style.display = 'flex';
            if(alarmSound) alarmSound.play();
            if (drowsyTimer) clearTimeout(drowsyTimer);
            drowsyTimer = setTimeout(() => {
                console.error("10초 이상 졸음 지속! 자동으로 자리 비움 처리합니다.");
                if (!isAway) awayBtn.click();
            }, 10000);
        } else if (!isDrowsyDetected && isActivelyDrowsy) {
            resetDrowsinessState();
        }
    }

    // ✨ [신규] 근무 이탈 감지 처리 함수
    function handleUnattended(isFaceDetected) {
        if (isWorking && !isAway) {
            if (!isFaceDetected) {
                // 얼굴이 감지되지 않았고, 타이머가 없다면 시작
                if (!unattendedTimer) {
                    console.warn("얼굴 미감지! 10초 후 자리 이탈로 처리됩니다.");
                    unattendedTimer = setTimeout(() => {
                        console.error("10초 이상 얼굴 미감지! 관리자에게 알리고 자리 비움 처리.");
                        if (!isAway) {
                            socket.emit('unattended_workstation'); // 서버에 이탈 이벤트 전송
                            awayBtn.click(); // 자리 비움 버튼 클릭
                        }
                    }, 10000); // 10초
                }
            } else {
                // 얼굴이 다시 감지되면 타이머 취소
                if (unattendedTimer) {
                    console.log("얼굴 재감지. 이탈 타이머를 취소합니다.");
                    clearTimeout(unattendedTimer);
                    unattendedTimer = null;
                }
            }
        }
    }

    function resetDrowsinessState() {
        if (!isActivelyDrowsy) return;
        console.log("졸음 상태 해제.");
        isActivelyDrowsy = false;
        drowsyWarning.style.display = 'none';
        if(alarmSound) {
            alarmSound.pause();
            alarmSound.currentTime = 0;
        }
        if (drowsyTimer) clearTimeout(drowsyTimer);
        drowsyTimer = null;
    }

    // ✨ [신규] 모든 경고 상태를 초기화하는 함수
    function resetAllWarnings() {
        resetDrowsinessState();
        if (unattendedTimer) {
            clearTimeout(unattendedTimer);
            unattendedTimer = null;
            console.log("상태 변경으로 이탈 타이머를 초기화합니다.");
        }
    }

    // =======================================================
    // 3. 버튼 이벤트 핸들러
    // =======================================================
    clockInBtn.addEventListener('click', async () => {
        try {
            const response = await fetch('/api/clock_in', { method: 'POST' });
            const result = await response.json();
            if (!response.ok) throw new Error(result.message || '출근 처리 실패');
            isAway = false;
            awayBtn.textContent = '자리 비움';
            updateButtonStates(true);
            startTimer();
            alert(result.message);
        } catch (error) {
            alert(error.message);
        }
    });

    awayBtn.addEventListener('click', async () => {
        isAway = !isAway;
        awayBtn.textContent = isAway ? '업무 복귀' : '자리 비움';
        resetAllWarnings(); // ✨ 자리 비움/복귀 시 모든 경고 초기화
        try {
            await fetch('/api/set_status', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ status: isAway ? 'away' : 'working' })
            });
        } catch (error) {
            console.error("상태 변경 요청 실패:", error);
        }
    });

    clockOutBtn.addEventListener('click', async () => {
        resetAllWarnings(); // ✨ 근무 종료 시 모든 경고 초기화
        if (workedSeconds < TOTAL_WORK_GOAL_SECONDS) {
            alert('아직 기준 근무 시간(5분)을 채우지 못했습니다.');
            return;
        }
        try {
            await fetch('/api/set_status', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ status: 'goal_met' })
            });
        } catch (error) {
            console.error("초록불 켜기 요청 실패:", error);
        }
        try {
            const response = await fetch('/api/clock_out', { method: 'POST' });
            const result = await response.json();
            if (!response.ok) throw new Error(result.message || '퇴근 처리 실패');
            stopTimer();
            updateButtonStates(false);
            await fetchAndUpdateWorkStatus();
            alert('근무가 종료되었습니다. 수고하셨습니다.');
        } catch (error) {
            alert(error.message);
        }
    });

    // =======================================================
    // 4. 유틸리티 함수 (타이머, 차트, 웹캠 등)
    // =======================================================
    function updateButtonStates(isWorkingState) {
        isWorking = isWorkingState;
        if (isWorking) {
            timerContainer.style.display = 'flex';
            clockInBtn.disabled = true;
            awayBtn.disabled = false;
            clockOutBtn.disabled = false;
        } else {
            timerContainer.style.display = 'none';
            clockInBtn.disabled = false;
            awayBtn.disabled = true;
            clockOutBtn.disabled = true;
        }
    }

    function formatTime(seconds) {
        const h = String(Math.floor(seconds / 3600)).padStart(2, '0');
        const m = String(Math.floor((seconds % 3600) / 60)).padStart(2, '0');
        const s = String(Math.floor(seconds % 60)).padStart(2, '0');
        return `${h}:${m}:${s}`;
    }

    function createOrUpdateChart(data) {
        const colors = ['#28a745', '#ffc107', '#e9ecef'];
        if (myWorkChart) {
            myWorkChart.data.datasets[0].data = data;
            myWorkChart.update('none');
        } else {
            myWorkChart = new Chart(chartCanvas, {
                type: 'doughnut',
                data: { labels: ['근무', '자리비움', '미근무'], datasets: [{ data, backgroundColor: colors, borderWidth: 0 }] },
                options: { responsive: true, cutout: '70%', plugins: { legend: { display: false } } }
            });
        }
    }

    function updateDisplay() {
        totalTimerDisplay.textContent = formatTime(totalSeconds);
        workTimerDisplay.textContent = formatTime(workedSeconds);
        awayTimerDisplay.textContent = formatTime(awaySeconds);
        const remainingGoal = Math.max(0, TOTAL_WORK_GOAL_SECONDS - workedSeconds);
        createOrUpdateChart([workedSeconds, awaySeconds, remainingGoal]);
    }
    
    function startTimer() {
        if (mainTimer) return;
        mainTimer = setInterval(() => {
            totalSeconds++;
            if (isAway) {
                awaySeconds++;
            } else {
                workedSeconds++;
            }
            updateDisplay();
        }, 1000);
    }
    
    function stopTimer() {
        clearInterval(mainTimer);
        mainTimer = null;
    }
    
    async function fetchAndUpdateWorkStatus() {
        try {
            const response = await fetch('/api/my_work_hours');
            if (!response.ok) throw new Error('서버 응답 오류');
            const data = await response.json();
            workedSeconds = Math.round(data.worked_hours * 3600);
            totalSeconds = workedSeconds;
            awaySeconds = 0;
            updateDisplay();
        } catch (error) {
            console.error("근무 시간 조회 실패:", error);
            updateButtonStates(false);
            createOrUpdateChart([0, 0, 1]);
        }
    }

    async function setupCamera() {
        try {
            const stream = await navigator.mediaDevices.getUserMedia({ video: { width: 640, height: 480 }, audio: false });
            video.srcObject = stream;
        } catch (err) {
            console.error("웹캠을 사용할 수 없습니다:", err);
            alert("웹캠을 사용할 수 없습니다. 카메라 권한을 확인해주세요.");
        }
    }
    
    const FPS = 10;
    let lastFrameTime = 0;
    function sendFrameLoop() {
        requestAnimationFrame(sendFrameLoop);
        const now = Date.now();
        if (now - lastFrameTime < 1000 / FPS) return;
        lastFrameTime = now;

        if (!isProcessing && video.readyState === video.HAVE_ENOUGH_DATA) {
            isProcessing = true;
            canvas.width = video.videoWidth;
            canvas.height = video.videoHeight;
            context.drawImage(video, 0, 0, canvas.width, canvas.height);
            const data = canvas.toDataURL('image/jpeg', 0.8);
            socket.emit('image', data);
        }
    }
    video.addEventListener('loadeddata', sendFrameLoop);

    // --- 페이지 초기화 ---
    fetchAndUpdateWorkStatus();
    updateButtonStates(false);
    createOrUpdateChart([0, 0, 1]);
    setupCamera();
});