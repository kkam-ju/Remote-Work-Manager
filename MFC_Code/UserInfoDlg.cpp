#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "UserInfoDlg.h"

IMPLEMENT_DYNAMIC(CUserInfoDlg, CDialogEx)


CUserInfoDlg::CUserInfoDlg(int userId, CWnd* pParent )
    : CDialogEx(IDD_USER_INFO_DIALOG, pParent), m_nUserId(userId)
{
    m_whiteBrush.CreateSolidBrush(RGB(255, 255, 255)); // 흰색(R,G,B) 브러시 생성
}

CUserInfoDlg::~CUserInfoDlg()
{
}

void CUserInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_STATIC_NAME, m_strName);
    DDX_Text(pDX, IDC_STATIC_DEPT, m_strDept);
    DDX_Text(pDX, IDC_STATIC_EMAIL, m_strEmail);
    DDX_Text(pDX, IDC_STATIC_PHONE, m_strPhone);
}

BEGIN_MESSAGE_MAP(CUserInfoDlg, CDialogEx)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// ✨ 추가: OnInitDialog 함수 구현 (핵심 로직)
BOOL CUserInfoDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_fontLabel.CreateFont(
        20,                         // nHeight (글자 높이)
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
    GetDlgItem(IDC_STATIC_NAME)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_DEPT)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_EMAIL)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_PHONE)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_1)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_2)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_3)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_4)->SetFont(&m_fontLabel);


    if (m_nUserId == -1) {
        AfxMessageBox(_T("사용자 정보를 가져올 수 없습니다."));
        EndDialog(IDCANCEL);
        return TRUE;
    }

    if (ConnectDatabase())
    {
        try
        {
            CString strSQL;
            strSQL.Format(
                _T("SELECT e.full_name, e.department, u.email, u.phone_number ")
                _T("FROM users u JOIN employees e ON u.employee_id = e.employee_id ")
                _T("WHERE u.user_id = %d"), m_nUserId);

            CRecordset rs(&m_db);
            rs.Open(CRecordset::forwardOnly, strSQL);

            if (!rs.IsEOF())
            {
                // DB에서 읽어온 값들을 멤버 변수에 저장
                rs.GetFieldValue(_T("full_name"), m_strName);
                rs.GetFieldValue(_T("department"), m_strDept);
                rs.GetFieldValue(_T("email"), m_strEmail);
                rs.GetFieldValue(_T("phone_number"), m_strPhone);
            }
            rs.Close();
        }
        catch (CDBException* e)
        {
            AfxMessageBox(_T("데이터를 불러오는 중 오류가 발생했습니다."));
            e->Delete();
        }
    }
    UpdateData(FALSE);
    return TRUE;
}

BOOL CUserInfoDlg::ConnectDatabase()
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
        TCHAR szError[1024];
        e->GetErrorMessage(szError, 1024);
        AfxMessageBox(szError);
        e->Delete();
        return FALSE;
    }
}

// 다이얼로그 및 컨트롤의 배경을 그리기 직전에 호출되는 함수
HBRUSH CUserInfoDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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