#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "MyInfoDlg.h"

IMPLEMENT_DYNAMIC(CMyInfoDlg, CDialogEx)

// 생성자
CMyInfoDlg::CMyInfoDlg(int userId, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MY_INFO_DIALOG, pParent), m_nCurrentUserId(userId)
{
	m_whiteBrush.CreateSolidBrush(RGB(255, 255, 255)); // 흰색(R,G,B) 브러시 생성
}

CMyInfoDlg::~CMyInfoDlg()
{
}

// 컨트롤 변수와 실제 컨트롤을 연결
void CMyInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PHONE, m_editPhone);
	DDX_Control(pDX, IDC_EDIT_EMAIL, m_editEmail);
	DDX_Control(pDX, IDC_EDIT_NEW_PASSWORD, m_editNewPassword);
	DDX_Control(pDX, IDC_EDIT_CONFIRM_PASSWORD, m_editConfirmPassword);
}

// 버튼 ID와 함수를 연결
BEGIN_MESSAGE_MAP(CMyInfoDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_CHANGE_PHONE, &CMyInfoDlg::OnBnClickedChangePhone)
	ON_BN_CLICKED(IDC_BTN_CHANGE_EMAIL, &CMyInfoDlg::OnBnClickedChangeEmail)
	ON_BN_CLICKED(IDC_BTN_CHANGE_PASSWORD, &CMyInfoDlg::OnBnClickedChangePassword)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// DB 연결 함수
BOOL CMyInfoDlg::ConnectDatabase()
{
	if (m_db.IsOpen()) return TRUE;
	CString strConnect;
	strConnect.Format(
		_T("DRIVER={MySQL ODBC 9.4 Unicode Driver};") // 1. 사용할 ODBC 드라이버 이름
		_T("SERVER=192.168.0.95;")                   // 2. DB 서버 IP 주소 해당 IP
		_T("DATABASE=yolo_webapp_db;")               // 3. 접속할 데이터베이스 이름
		_T("UID=root;")                              // 4. 사용자 ID
		_T("PWD=moble;")                              // 5. 비밀번호
		_T("PORT=3306;")                             // 6. DB 포트 번호
	);
	try {
		if (!m_db.OpenEx(strConnect, CDatabase::noOdbcDialog)) {
			AfxMessageBox(_T("MySQL DB 연결 실패"));
			return FALSE;
		}
		return TRUE;
	}
	catch (CDBException* e) {
		AfxMessageBox(_T("DB 연결 오류가 발생했습니다."));
		e->Delete();
		return FALSE;
	}
}

// 다이얼로그가 처음 켜질 때 실행되는 함수
BOOL CMyInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_fontLabel.CreateFont(
		15,                         // nHeight (글자 높이)
		0,                          // nWidth (0이면 자동)
		0,                          // nEscapement
		0,                          // nOrientation
		FW_BOLD,                    // nWeight (FW_NORMAL, FW_BOLD 등)
		FALSE,                      // bItalic
		FALSE,                      // bUnderline
		0,                          // cStrikeOut
		DEFAULT_CHARSET,            // nCharSet
		OUT_DEFAULT_PRECIS,         // nOutPrecision
		CLIP_DEFAULT_PRECIS,        // nClipPrecision
		DEFAULT_QUALITY,            // nQuality
		DEFAULT_PITCH | FF_SWISS,   // nPitchAndFamily
		_T("맑은 고딕"));            // lpszFacename

	// 2. 1단계에서 설정한 ID를 가진 Static Text 컨트롤에 폰트를 적용합니다.
	GetDlgItem(IDC_STC_POHNENUMBER)->SetFont(&m_fontLabel);
	GetDlgItem(IDC_STC_EMAIL)->SetFont(&m_fontLabel);
	GetDlgItem(IDC_STC_PASSWORD)->SetFont(&m_fontLabel);
	GetDlgItem(IDC_STC_PASSWORD2)->SetFont(&m_fontLabel);
	// ▲ 여기까지 추가 ▲

	LoadCurrentUserData();
	return TRUE;
}

// DB에서 현재 사용자 정보를 가져오는 함수
void CMyInfoDlg::LoadCurrentUserData()
{
	if (!ConnectDatabase()) return;
	try
	{
		CString strSQL;
		strSQL.Format(_T("SELECT phone_number, email FROM users WHERE user_id = %d"), m_nCurrentUserId);
		CRecordset rs(&m_db);
		rs.Open(CRecordset::forwardOnly, strSQL);
		if (!rs.IsEOF())
		{
			CString phoneNumber, email;
			rs.GetFieldValue(_T("phone_number"), phoneNumber);
			rs.GetFieldValue(_T("email"), email);

			m_editPhone.SetWindowText(phoneNumber);
			m_editEmail.SetWindowText(email);
		}
		rs.Close();
	}
	catch (CDBException* e)
	{
		AfxMessageBox(_T("사용자 정보를 불러오는 중 오류가 발생했습니다."));
		e->Delete();
	}
}

// '전화번호 변경' 버튼 클릭 시
void CMyInfoDlg::OnBnClickedChangePhone()
{
	CString phone;
	m_editPhone.GetWindowText(phone);
	phone.Trim();
	if (phone.IsEmpty()) { AfxMessageBox(_T("전화번호를 입력하세요.")); return; }
	if (!ConnectDatabase()) return;
	try
	{
		CString safePhone = phone;
		safePhone.Replace(_T("'"), _T("''"));
		CString strSQL;
		strSQL.Format(_T("UPDATE users SET phone_number = '%s' WHERE user_id = %d"), safePhone, m_nCurrentUserId);
		m_db.ExecuteSQL(strSQL);
		AfxMessageBox(_T("전화번호가 성공적으로 변경되었습니다."));
	}
	catch (CDBException* e) { AfxMessageBox(_T("전화번호 변경 중 오류가 발생했습니다.")); e->Delete(); }
}

// '이메일 변경' 버튼 클릭 시
void CMyInfoDlg::OnBnClickedChangeEmail()
{
	CString email;
	m_editEmail.GetWindowText(email);
	email.Trim();

	if (email.IsEmpty())
	{
		AfxMessageBox(_T("이메일을 입력하세요."));
		return;
	}

	if (!ConnectDatabase()) return;
	try
	{
		CString safeEmail = email;
		safeEmail.Replace(_T("'"), _T("''"));

		CString strSQL;
		strSQL.Format(_T("UPDATE users SET email = '%s' WHERE user_id = %d"),
			safeEmail, m_nCurrentUserId);

		m_db.ExecuteSQL(strSQL);
		AfxMessageBox(_T("이메일이 성공적으로 변경되었습니다."));
	}
	catch (CDBException* e)
	{
		AfxMessageBox(_T("이메일 변경 중 오류가 발생했습니다."));
		e->Delete();
	}
}


// '비밀번호 변경' 버튼 클릭 시
void CMyInfoDlg::OnBnClickedChangePassword()
{
	CString newPassword, confirmPassword;
	m_editNewPassword.GetWindowText(newPassword);
	m_editConfirmPassword.GetWindowText(confirmPassword);

	if (newPassword.IsEmpty()) { AfxMessageBox(_T("새 비밀번호를 입력하세요.")); return; }
	if (newPassword != confirmPassword) { AfxMessageBox(_T("새 비밀번호가 일치하지 않습니다.")); return; }
	if (!ConnectDatabase()) return;
	try
	{
		CString safePassword = newPassword;
		safePassword.Replace(_T("'"), _T("''"));
		CString strSQL;
		strSQL.Format(_T("UPDATE users SET password = '%s' WHERE user_id = %d"), safePassword, m_nCurrentUserId);
		m_db.ExecuteSQL(strSQL);
		AfxMessageBox(_T("비밀번호가 성공적으로 변경되었습니다."));
		m_editNewPassword.SetWindowText(_T(""));
		m_editConfirmPassword.SetWindowText(_T(""));
	}
	catch (CDBException* e) { AfxMessageBox(_T("비밀번호 변경 중 오류가 발생했습니다.")); e->Delete(); }
}


// 다이얼로그 및 컨트롤의 배경을 그리기 직전에 호출되는 함수
HBRUSH CMyInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// 기본적으로 부모 클래스의 함수를 호출합니다.
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// nCtlColor를 통해 어떤 컨트롤을 그리는지 알 수 있습니다.
	// CTLCOLOR_DLG는 다이얼로그 자체의 배경을 의미합니다.
	if (nCtlColor == CTLCOLOR_DLG)
	{
		// 우리가 만든 흰색 브러시를 반환하여 배경을 칠하도록 합니다.
		return (HBRUSH)m_whiteBrush.GetSafeHandle();
	}

	// Static Text(라벨)의 배경도 투명하게 만들어 흰색 배경과 어울리게 합니다.
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT); // 배경을 투명하게 설정
		return (HBRUSH)m_whiteBrush.GetSafeHandle(); // 글자 주변 배경도 흰색으로
	}

	return hbr;
}