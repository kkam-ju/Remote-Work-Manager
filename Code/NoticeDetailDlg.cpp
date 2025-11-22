#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "NoticeDetailDlg.h"


// CNoticeDetailDlg 대화 상자

IMPLEMENT_DYNAMIC(CNoticeDetailDlg, CDialogEx)

CNoticeDetailDlg::CNoticeDetailDlg(int noticeId, CWnd* pParent )
	: CDialogEx(IDD_NOTICE_DETAIL_DIALOG, pParent), m_nNoticeId(noticeId)
{
    m_whiteBrush.CreateSolidBrush(RGB(255, 255, 255)); // 흰색(R,G,B) 브러시 생성
}

CNoticeDetailDlg::~CNoticeDetailDlg()
{
}

void CNoticeDetailDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);

    DDX_Text(pDX, IDC_STATIC_TITLE, m_strTitle);
    DDX_Text(pDX, IDC_STATIC_AUTHOR, m_strAuthor);
    DDX_Text(pDX, IDC_STATIC_DATE, m_strDate);
    DDX_Text(pDX, IDC_EDIT_CONTENT, m_strContent);
}

BEGIN_MESSAGE_MAP(CNoticeDetailDlg, CDialogEx)
    ON_STN_CLICKED(IDC_STATIC_TITLE, &CNoticeDetailDlg::OnStnClickedStaticTitle)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CNoticeDetailDlg::ConnectDatabase()
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

BOOL CNoticeDetailDlg::OnInitDialog()
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
    GetDlgItem(IDC_STC_TI)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STC_CON)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STC_NA)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STC_DA)->SetFont(&m_fontLabel); 
    GetDlgItem(IDC_STATIC_TITLE)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_AUTHOR)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_DATE)->SetFont(&m_fontLabel);
    // ▲ 여기까지 추가 ▲

    if (m_nNoticeId == -1) {
        AfxMessageBox(_T("공지사항 정보를 가져올 수 없습니다."));
        EndDialog(IDCANCEL);
        return TRUE;
    }

    if (ConnectDatabase())
    {
        try
        {
            CString strSQL;
            strSQL.Format(
                _T("SELECT n.title, n.content, e.full_name, DATE_FORMAT(n.created_at, '%%Y-%%m-%%d %%H:%%i') as created_at ")
                _T("FROM notices n JOIN users u ON n.user_id = u.user_id ")
                _T("JOIN employees e ON u.employee_id = e.employee_id ")
                _T("WHERE n.notice_id = %d"), m_nNoticeId);

            CRecordset rs(&m_db);
            rs.Open(CRecordset::forwardOnly, strSQL);

            if (!rs.IsEOF())
            {
                // DB에서 읽어온 값들을 멤버 변수에 저장
                rs.GetFieldValue(_T("title"), m_strTitle);
                rs.GetFieldValue(_T("content"), m_strContent);
                rs.GetFieldValue(_T("full_name"), m_strAuthor);
                rs.GetFieldValue(_T("created_at"), m_strDate);
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

// 다이얼로그 및 컨트롤의 배경을 그리기 직전에 호출되는 함수
HBRUSH CNoticeDetailDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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


void CNoticeDetailDlg::OnStnClickedStaticTitle()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
