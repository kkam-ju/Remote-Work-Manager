#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "WriteNoticeDlg.h"
#include <afxdb.h>

IMPLEMENT_DYNAMIC(CWriteNoticeDlg, CDialogEx)

// 생성자 수정
CWriteNoticeDlg::CWriteNoticeDlg(int userId, CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_WRITE_NOTICE_DIALOG, pParent) // IDD_WRITE_NOTICE_DIALOG는 실제 리소스 ID로
{
    m_nUserId = userId; // 전달받은 userId를 멤버 변수에 저장
    m_whiteBrush.CreateSolidBrush(RGB(255, 255, 255)); // 흰색(R,G,B) 브러시 생성
}

CWriteNoticeDlg::~CWriteNoticeDlg()
{
}

void CWriteNoticeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_editTitle);
    DDX_Control(pDX, IDC_EDIT2, m_editContent);
}

BEGIN_MESSAGE_MAP(CWriteNoticeDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &CWriteNoticeDlg::OnBnClickedOk)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CWriteNoticeDlg::OnInitDialog()
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
    GetDlgItem(IDC_STATIC_NTITLE)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_NCON)->SetFont(&m_fontLabel);


    return TRUE;
}

BOOL CWriteNoticeDlg::ConnectDatabase()
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

void CWriteNoticeDlg::OnBnClickedOk()
{
    CString title, content;
    m_editTitle.GetWindowText(title);
    m_editContent.GetWindowText(content);
    title.Trim();
    content.Trim();

    if (title.IsEmpty() || content.IsEmpty()) {
        AfxMessageBox(_T("제목과 내용을 모두 입력해주세요."));
        return;
    }

    if (ConnectDatabase()) {
        try {
            CString safeTitle = title, safeContent = content;
            safeTitle.Replace(_T("'"), _T("''"));
            safeContent.Replace(_T("'"), _T("''"));

            CString strSQL;
            strSQL.Format(
                _T("INSERT INTO notices (user_id, title, content, created_at) VALUES (%d, '%s', '%s', NOW())"),
                m_nUserId, safeTitle, safeContent);

            m_db.ExecuteSQL(strSQL);

            EndDialog(IDOK);
        }
        catch (CDBException* e) {
            AfxMessageBox(_T("공지사항 등록 중 오류가 발생했습니다."));
            e->Delete();
        }
    }
}

// 다이얼로그 및 컨트롤의 배경을 그리기 직전에 호출되는 함수
HBRUSH CWriteNoticeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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