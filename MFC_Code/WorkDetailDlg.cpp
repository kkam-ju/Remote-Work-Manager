#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "WorkDetailDlg.h"
#include <afxdb.h>

IMPLEMENT_DYNAMIC(CWorkDetailDlg, CDialogEx)

CWorkDetailDlg::CWorkDetailDlg(int requestId, int currentUserId, bool isAdmin, CWnd* pParent )
    : CDialogEx(IDD_WORK_DETAIL_DIALOG, pParent)
    , m_nRequestId(requestId)
    , m_nCurrentUserId(currentUserId)
    , m_bIsAdmin(isAdmin)
{
    m_whiteBrush.CreateSolidBrush(RGB(255, 255, 255));
}

CWorkDetailDlg::~CWorkDetailDlg()
{
}

void CWorkDetailDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_TITLE, m_staticTitle);
    DDX_Control(pDX, IDC_STATIC_REQUESTER, m_staticRequester);
    DDX_Control(pDX, IDC_STATIC_DUE_DATE, m_staticDueDate);
    DDX_Control(pDX, IDC_EDIT_CONTENT, m_editContent);
    DDX_Control(pDX, IDC_LIST_ATTACHMENTS, m_listAttachments);
    DDX_Control(pDX, IDC_COMBO_STATUS, m_comboStatus);
}

BEGIN_MESSAGE_MAP(CWorkDetailDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_UPDATE_STATUS, &CWorkDetailDlg::OnBnClickedButtonUpdateStatus)
    ON_LBN_DBLCLK(IDC_LIST_ATTACHMENTS, &CWorkDetailDlg::OnLbnDblclkListAttachments)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


BOOL CWorkDetailDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    LoadWorkDetails();

    // 관리자가 아닐 경우 상태 변경 컨트롤들을 숨김
    if (!m_bIsAdmin)
    {
        GetDlgItem(IDC_COMBO_STATUS)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_BUTTON_UPDATE_STATUS)->ShowWindow(SW_HIDE);
    }

    return TRUE;
}

BOOL CWorkDetailDlg::ConnectDatabase()
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

void CWorkDetailDlg::LoadWorkDetails()
{
    if (!ConnectDatabase()) return;

    // --- 1. 업무 기본 정보 불러오기 ---
    try
    {
        CString strSQL;
        strSQL.Format(
            _T("SELECT wr.title, wr.content, wr.status, e.full_name, DATE_FORMAT(wr.due_date, '%%Y-%%m-%%d') as due_date ")
            _T("FROM work_requests wr JOIN users u ON wr.user_id = u.user_id JOIN employees e ON u.employee_id = e.employee_id ")
            _T("WHERE wr.request_id = %d"), m_nRequestId);

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        if (!rs.IsEOF())
        {
            CString title, content, status, requester, dueDate;
            rs.GetFieldValue(_T("title"), title);
            rs.GetFieldValue(_T("content"), content);
            rs.GetFieldValue(_T("status"), status);
            rs.GetFieldValue(_T("full_name"), requester);
            rs.GetFieldValue(_T("due_date"), dueDate);

            m_staticTitle.SetWindowText(title);
            m_editContent.SetWindowText(content);
            m_staticRequester.SetWindowText(requester);
            m_staticDueDate.SetWindowText(dueDate);

            // 콤보 박스 채우고 현재 상태 선택
            m_comboStatus.ResetContent();
            m_comboStatus.AddString(_T("접수"));
            m_comboStatus.AddString(_T("처리중"));
            m_comboStatus.AddString(_T("완료"));
            m_comboStatus.AddString(_T("반려"));
            m_comboStatus.SelectString(-1, status);
        }
        rs.Close();
    }
    catch (CDBException* e) { e->Delete(); }

    // --- 2. 첨부 파일 목록 불러오기 ---
    try
    {
        m_listAttachments.ResetContent();
        m_attachments.clear();

        CString strSQL;
        strSQL.Format(_T("SELECT attachment_id, original_filename, file_path FROM attachments ")
            _T("WHERE related_type = 'work_request' AND related_id = %d"), m_nRequestId);

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            AttachmentInfo info;
            CString strId;
            rs.GetFieldValue(_T("attachment_id"), strId);
            info.id = _ttoi(strId);
            rs.GetFieldValue(_T("original_filename"), info.filename);
            rs.GetFieldValue(_T("file_path"), info.filepath);
            m_attachments.push_back(info);

            int index = m_listAttachments.AddString(info.filename);
            m_listAttachments.SetItemData(index, info.id); // 리스트 항목에 ID 저장

            rs.MoveNext();
        }
        rs.Close();
    }
    catch (CDBException* e) { e->Delete(); }
}

void CWorkDetailDlg::OnBnClickedButtonUpdateStatus()
{
    if (!m_bIsAdmin) return;

    int nSel = m_comboStatus.GetCurSel();
    if (nSel == CB_ERR) {
        AfxMessageBox(_T("변경할 상태를 선택하세요."));
        return;
    }
    CString newStatus;
    m_comboStatus.GetLBText(nSel, newStatus);

    try
    {
        CString strSQL;
        strSQL.Format(_T("UPDATE work_requests SET status = '%s' WHERE request_id = %d"), newStatus, m_nRequestId);
        m_db.ExecuteSQL(strSQL);
        AfxMessageBox(_T("상태가 성공적으로 변경되었습니다."));

        // 부모창(HomeDlg)에 변경사항을 알리기 위해 IDOK로 닫기
        EndDialog(IDOK);
    }
    catch (CDBException* e)
    {
        AfxMessageBox(_T("상태 변경 중 오류가 발생했습니다."));
        e->Delete();
    }
}

// 첨부파일 목록에서 항목을 더블클릭했을 때
void CWorkDetailDlg::OnLbnDblclkListAttachments()
{
    int nSel = m_listAttachments.GetCurSel();
    if (nSel == LB_ERR) return;

    // TODO: 파일 열기/다운로드 로직 구현
    // ShellExecute(NULL, _T("open"), m_attachments[nSel].filepath + m_attachments[nSel].filename, ...);
    CString msg;
    msg.Format(_T("파일 '%s' 열기를 시도합니다."), m_attachments[nSel].filename);
    AfxMessageBox(msg);
}

HBRUSH CWorkDetailDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    // 다이얼로그 자체의 배경일 경우
    if (nCtlColor == CTLCOLOR_DLG)
    {
        return (HBRUSH)m_whiteBrush.GetSafeHandle();
    }

    // Static Text 컨트롤일 경우 (제목, 요청자 등)
    if (nCtlColor == CTLCOLOR_STATIC)
    {
        pDC->SetBkMode(TRANSPARENT); // 글자 배경을 투명하게
        return (HBRUSH)m_whiteBrush.GetSafeHandle();
    }

    // Edit Control (읽기 전용인 경우)
    if (nCtlColor == CTLCOLOR_EDIT && GetDlgItem(IDC_EDIT_CONTENT) == pWnd)
    {
        // m_editContent가 읽기 전용이라면 배경을 흰색으로, 테두리 없애기 등 가능
        pDC->SetBkColor(RGB(255, 255, 255)); // 배경 흰색
        pDC->SetTextColor(RGB(0, 0, 0));       // 글자 검은색
        return (HBRUSH)m_whiteBrush.GetSafeHandle();
    }


    return hbr;
}