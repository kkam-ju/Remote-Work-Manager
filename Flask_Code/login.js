document.addEventListener('DOMContentLoaded', function() {
    const loginTitle = document.getElementById('login-title');
    const loginButton = document.getElementById('login-button');
    const radioButtons = document.querySelectorAll('input[name="login_type"]');
    const passwordInput = document.getElementById('password');
    const togglePassword = document.getElementById('toggle-password');

    // 로그인 타입에 따라 UI 텍스트를 변경하는 함수
    function updateFormUI(loginType) {
        if (loginType === 'admin') {
            loginTitle.textContent = '관리자 로그인';
            loginButton.textContent = '관리자 로그인';
        } else {
            loginTitle.textContent = '사용자 로그인';
            loginButton.textContent = '로그인';
        }
    }

    // 라디오 버튼 변경 시 UI 업데이트
    radioButtons.forEach(radio => {
        radio.addEventListener('change', function() {
            updateFormUI(this.value);
        });
    });

    // 비밀번호 보기/숨기기 기능
    togglePassword.addEventListener('click', function() {
        const type = passwordInput.getAttribute('type') === 'password' ? 'text' : 'password';
        passwordInput.setAttribute('type', type);
        this.textContent = type === 'password' ? '👁️' : '🙈';
    });

    // 페이지가 처음 로드될 때 선택된 라디오 버튼 기준으로 UI 초기화
    const initialLoginType = document.querySelector('input[name="login_type"]:checked').value;
    updateFormUI(initialLoginType);
});
