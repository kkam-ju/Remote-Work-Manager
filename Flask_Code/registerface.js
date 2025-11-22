document.addEventListener('DOMContentLoaded', () => {
    const video = document.getElementById('webcam-video');
    const captureButton = document.getElementById('capture-btn');
    const feedbackMessage = document.getElementById('feedback-message');
    
    // 웹캠 스트림에 접근하여 video 요소에 연결하는 함수
    async function setupWebcam() {
        try {
            const stream = await navigator.mediaDevices.getUserMedia({ 
                video: true, 
                audio: false 
            });
            video.srcObject = stream;
        } catch (err) {
            console.error("웹캠에 접근할 수 없습니다:", err);
            displayFeedback("웹캠을 찾을 수 없거나 접근 권한이 없습니다.");
        }
    }

    // 사용자에게 피드백 메시지를 보여주는 함수
    function displayFeedback(message) {
        feedbackMessage.textContent = message;
        feedbackMessage.classList.add('message-visible');
        setTimeout(() => {
            feedbackMessage.classList.remove('message-visible');
        }, 3000); // 3초 후에 메시지 사라짐
    }

    // 사진 촬영 버튼 클릭 이벤트 리스너
    captureButton.addEventListener('click', () => {
        // 1. 캔버스 생성 및 설정
        const canvas = document.createElement('canvas');
        canvas.width = video.videoWidth;
        canvas.height = video.videoHeight;
        
        // 2. 캔버스에 현재 비디오 프레임 그리기 (좌우 반전된 것을 다시 되돌리기)
        const context = canvas.getContext('2d');
        context.translate(canvas.width, 0);
        context.scale(-1, 1);
        context.drawImage(video, 0, 0, canvas.width, canvas.height);

        // 3. 캔버스 이미지를 데이터 URL(base64)로 변환
        const dataUrl = canvas.toDataURL('image/jpeg');
        
        // TODO: 여기서 dataUrl을 서버로 전송하는 로직을 구현해야 합니다.
        // 예를 들어, fetch API를 사용하여 서버의 '/api/register-face'와 같은 엔드포인트로 POST 요청을 보낼 수 있습니다.
        
        console.log("캡처된 이미지 데이터:", dataUrl.substring(0, 50) + "..."); // 데이터 확인용 로그
        displayFeedback("사진이 촬영되었습니다! (서버 전송 로직 필요)");

        // 예시: 서버 전송 코드
        /*
        fetch('/api/register-face-data', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ image: dataUrl })
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                displayFeedback("얼굴 등록이 성공적으로 완료되었습니다.");
            } else {
                displayFeedback("얼굴 등록에 실패했습니다: " + data.message);
            }
        })
        .catch(error => {
            console.error('서버 통신 오류:', error);
            displayFeedback("서버와 통신 중 오류가 발생했습니다.");
        });
        */
    });

    // 페이지가 로드되면 웹캠 설정 함수를 즉시 실행
    setupWebcam();
});