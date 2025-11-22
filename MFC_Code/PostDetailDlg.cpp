#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "PostDetailDlg.h"


// CPostDetailDlg 대화 상자

IMPLEMENT_DYNAMIC(CPostDetailDlg, CDialogEx)

CPostDetailDlg::CPostDetailDlg(int postId, int currentUserId, CWnd* pParent )
    : CDialogEx(IDD_POST_DETAIL_DIALOG, pParent)
    , m_nPostId(postId)
    , m_nCurrentUserId(currentUserId)
{
    m_whiteBrush.CreateSolidBrush(RGB(255, 255, 255)); // 흰색(R,G,B) 브러시 생성
}
CPostDetailDlg::~CPostDetailDlg()
{
}

void CPostDetailDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_POST_TITLE, m_staticPostTitle);
    DDX_Control(pDX, IDC_EDIT_POST_CONTENT, m_editPostContent);
    DDX_Control(pDX, IDC_LIST_COMMENTS, m_listComments);
    DDX_Control(pDX, IDC_EDIT_NEW_COMMENT, m_editNewComment);
}


BEGIN_MESSAGE_MAP(CPostDetailDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_SUBMIT_COMMENT, &CPostDetailDlg::OnBnClickedSubmitComment)
    ON_WM_CTLCOLOR()
    ON_STN_CLICKED(IDC_STATIC_COMT_INPUT, &CPostDetailDlg::OnStnClickedStaticComtInput)
END_MESSAGE_MAP()

BOOL CPostDetailDlg::OnInitDialog()
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
    GetDlgItem(IDC_STATIC_POST_TITLE3)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_POST_TITLE2)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_CON)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_COMT)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STATIC_COMT_INPUT)->SetFont(&m_fontLabel);
    

    LoadPostDetails(); // 데이터 로드 함수 호출

    return TRUE;
}


BOOL CPostDetailDlg::ConnectDatabase()
{
    if (m_db.IsOpen())
        return TRUE;

    // 성공한 연결 방식 (DSN, UID, PWD를 문자열에 포함)
    CString strConnect;
    strConnect.Format(
        _T("DRIVER={MySQL ODBC 9.4 Unicode Driver};") // 1. 사용할 ODBC 드라이버 이름
        _T("SERVER=192.168.0.95;")                   // 2. DB 서버 IP 주소 해당 IP
        _T("DATABASE=yolo_webapp_db;")               // 3. 접속할 데이터베이스 이름
        _T("UID=root;")                              // 4. 사용자 ID
        _T("PWD=moble;")                              // 5. 비밀번호
        _T("PORT=3306;")                             // 6. DB 포트 번호
    );
    try
    {
        if (!m_db.OpenEx(strConnect, CDatabase::noOdbcDialog))
        {
            AfxMessageBox(_T("MySQL 데이터베이스 연결 실패. DSN 및 인증 정보를 확인하세요."));
            return FALSE;
        }
        return TRUE;
    }
    catch (CDBException* e)
    {
        TCHAR szError[1024];
        e->GetErrorMessage(szError, 1024);
        CString errorMsg;
        errorMsg.Format(_T("DB 연결 중 예외 발생: %s"), szError);
        AfxMessageBox(errorMsg);
        e->Delete();
        return FALSE;
    }
}

void CPostDetailDlg::LoadPostDetails()
{
    if (!ConnectDatabase()) return;

    // --- 1. 게시글 본문 불러오기 ---
    try
    {
        CString strSQL;
        strSQL.Format(_T("SELECT title, content FROM posts WHERE post_id = %d"), m_nPostId);

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        if (!rs.IsEOF())
        {
            CString title, content;
            rs.GetFieldValue(_T("title"), title);
            rs.GetFieldValue(_T("content"), content);

            // 컨트롤에 텍스트 설정
            m_staticPostTitle.SetWindowText(title);
            m_editPostContent.SetWindowText(content);
        }
        rs.Close();
    }
    catch (CDBException* e) { e->Delete(); }

    // --- 2. 댓글 목록 불러오기 ---
    try
    {
        m_listComments.ResetContent(); // 기존 목록 삭제

        CString strSQL;
        strSQL.Format(_T("SELECT content, DATE_FORMAT(created_at, '%%Y-%%m-%%d %%H:%%i') as created_at ")
            _T("FROM comments WHERE post_id = %d ORDER BY created_at ASC"), m_nPostId);

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            CString content, createdAt;
            rs.GetFieldValue(_T("content"), content);
            rs.GetFieldValue(_T("created_at"), createdAt);

            CString formattedComment;
            formattedComment.Format(_T("익명 (%s) : %s"), createdAt, content);

            m_listComments.AddString(formattedComment);
            rs.MoveNext();
        }
        rs.Close();

        // 스크롤을 맨 아래로 이동
        int nCount = m_listComments.GetCount();
        if (nCount > 0) {
            m_listComments.SetTopIndex(nCount - 1);
        }
    }
    catch (CDBException* e) { e->Delete(); }
}
// CPostDetailDlg 메시지 처리기
// '등록' 버튼 클릭 시 호출되는 함수
void CPostDetailDlg::OnBnClickedSubmitComment()
{
    // 1. 입력창에서 댓글 내용 가져오기
    CString commentText;
    m_editNewComment.GetWindowText(commentText);
    commentText.Trim(); // 앞뒤 공백 제거

    // 2. 내용이 비어있는지 확인
    if (commentText.IsEmpty())
    {
        AfxMessageBox(_T("댓글 내용을 입력하세요."));
        return;
    }

    // 3. DB에 댓글 삽입 (INSERT 쿼리)
    if (ConnectDatabase())
    {
        try
        {
            // 3-1. SQL 인젝션 방지를 위해 작은따옴표(')를 두 개('')로 변경
            CString safeContent = commentText;
            safeContent.Replace(_T("'"), _T("''"));

            // 3-2. INSERT 쿼리 실행
            CString strSQL;
            strSQL.Format(
                _T("INSERT INTO comments (post_id, user_id, content) VALUES (%d, %d, '%s')"),
                m_nPostId, m_nCurrentUserId, safeContent);

            m_db.ExecuteSQL(strSQL);

            // 4. 댓글 목록 새로고침 및 입력창 비우기
            LoadPostDetails(); // 댓글 목록을 다시 불러와 화면을 갱신
            m_editNewComment.SetWindowText(_T("")); // 입력창 초기화
        }
        catch (CDBException* e)
        {
            AfxMessageBox(_T("댓글 등록 중 오류가 발생했습니다."));
            e->Delete();
        }
    }
}

// 다이얼로그 및 컨트롤의 배경을 그리기 직전에 호출되는 함수
HBRUSH CPostDetailDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
void CPostDetailDlg::OnStnClickedStaticComtInput()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
