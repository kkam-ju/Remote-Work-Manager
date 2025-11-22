document.addEventListener('DOMContentLoaded', function() {
    const form = document.getElementById('register-form');
    const messageArea = document.getElementById('message-area');

    form.addEventListener('submit', function(event) {
        event.preventDefault();

        const password = document.getElementById('password').value;
        const confirmPassword = document.getElementById('confirm_password').value;

        if (password !== confirmPassword) {
            showMessage('비밀번호가 일치하지 않습니다.', 'error');
            return;
        }

        const formData = new FormData(form);
        const data = Object.fromEntries(formData.entries());

        fetch('/signup', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data),
        })
        .then(response => response.json())
        .then(result => {
            if (result.success) {
                showMessage(result.message, 'success');
                setTimeout(() => {
                    window.location.href = '/login';
                }, 2000);
            } else {
                showMessage(result.message, 'error');
            }
        })
        .catch(error => {
            console.error('Error:', error);
            showMessage('회원가입 중 오류가 발생했습니다. 네트워크 연결을 확인해주세요.', 'error');
        });
    });

    function showMessage(message, type) {
        messageArea.innerHTML = `<div class="alert ${type}">${message}</div>`;
    }
});