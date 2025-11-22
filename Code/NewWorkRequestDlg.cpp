#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "NewWorkRequestDlg.h"


// CNewWorkRequestDlg 대화 상자

IMPLEMENT_DYNAMIC(CNewWorkRequestDlg, CDialogEx)

CNewWorkRequestDlg::CNewWorkRequestDlg(int currentUserId, CWnd* pParent )
    : CDialogEx(IDD_NEW_WORK_REQUEST_DIALOG, pParent)
    , m_nCurrentUserId(currentUserId)
{
    m_whiteBrush.CreateSolidBrush(RGB(255, 255, 255)); // 흰색(R,G,B) 브러시 생성
}

CNewWorkRequestDlg::~CNewWorkRequestDlg()
{
}

void CNewWorkRequestDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_TITLE, m_editTitle);
    DDX_Control(pDX, IDC_EDIT_CONTENT_2, m_editContent);
    DDX_Control(pDX, IDC_DATETIMEPICKER_DUE_DATE, m_datePicker);
    DDX_Control(pDX, IDC_BUTTON_ADD_FILE, m_btnAddFile);
    DDX_Control(pDX, IDC_LIST_ATTACHMENTS, m_listAttachments);
    // ✨ CCheckListBox로 연결
    DDX_Control(pDX, IDC_LIST_EMPLOYEES, m_listEmployees);
}


BEGIN_MESSAGE_MAP(CNewWorkRequestDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_ADD_FILE, &CNewWorkRequestDlg::OnBnClickedButtonAddFile)
    ON_BN_CLICKED(IDOK, &CNewWorkRequestDlg::OnBnClickedOk)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


BOOL CNewWorkRequestDlg::OnInitDialog()
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
    GetDlgItem(IDC_STC_TITLE)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STC_CONTENT)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STC_FILES)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STC_EMPLOYEES2)->SetFont(&m_fontLabel);
    GetDlgItem(IDC_STC_MOC)->SetFont(&m_fontLabel);
    // ▲ 여기까지 추가 ▲


    LoadEmployeeList(); // 다이얼로그가 뜰 때 직원 목록을 불러옵니다.



    return TRUE;
}

void CNewWorkRequestDlg::LoadEmployeeList()
{
    if (!ConnectDatabase()) return;

    try
    {
        // 관리자가 아닌 일반 직원 목록만 불러옵니다.
        CString strSQL = _T("SELECT u.user_id, e.full_name ")
            _T("FROM users u JOIN employees e ON u.employee_id = e.employee_id ")
            _T("WHERE e.is_admin = 0 ORDER BY e.full_name");

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            CString userId, fullName;
            rs.GetFieldValue(_T("user_id"), userId);
            rs.GetFieldValue(_T("full_name"), fullName);

            int nIndex = m_listEmployees.AddString(fullName);
            // 각 항목에 user_id를 데이터로 저장해둡니다.
            m_listEmployees.SetItemData(nIndex, _ttoi(userId));

            rs.MoveNext();
        }
        rs.Close();
    }
    catch (CDBException* e) { e->Delete(); }
}



void CNewWorkRequestDlg::OnBnClickedButtonAddFile()
{
    // CFileDialog를 이용해 파일을 선택합니다.
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("All Files (*.*)|*.*||"), this);

    if (dlg.DoModal() == IDOK)
    {
        CString filePath = dlg.GetPathName(); // 선택한 파일의 전체 경로
        CString fileName = dlg.GetFileName(); // 파일 이름만

        m_listAttachments.AddString(fileName);
        m_vecFilePaths.push_back(filePath); // 전체 경로는 별도로 저장
    }
}

void CNewWorkRequestDlg::OnBnClickedOk()
{
    // --- 1. 입력 값 유효성 검사 ---
    CString title, content;
    m_editTitle.GetWindowText(title);
    m_editContent.GetWindowText(content);
    title.Trim();
    content.Trim();

    if (title.IsEmpty()) {
        AfxMessageBox(_T("제목을 입력하세요."));
        return;
    }

    std::vector<int> vecTargetUserIDs;
    for (int i = 0; i < m_listEmployees.GetCount(); i++) {
        if (m_listEmployees.GetCheck(i)) {
            vecTargetUserIDs.push_back((int)m_listEmployees.GetItemData(i));
        }
    }
    if (vecTargetUserIDs.empty()) {
        AfxMessageBox(_T("업무를 받을 직원을 한 명 이상 선택하세요."));
        return;
    }

    CTime dueDate;
    m_datePicker.GetTime(dueDate);
    CString strDueDate = dueDate.Format(_T("%Y-%m-%d"));

    // --- 2. DB 저장 (트랜잭션 사용) ---
    if (!ConnectDatabase()) return;

    try
    {
        m_db.BeginTrans(); // 트랜잭션 시작

        // 2-1. work_requests 테이블에 업무 정보 INSERT
        CString safeTitle = title, safeContent = content;
        safeTitle.Replace(_T("'"), _T("''"));
        safeContent.Replace(_T("'"), _T("''"));

        CString strSQL;
        strSQL.Format(
            _T("INSERT INTO work_requests (user_id, title, content, due_date) VALUES (%d, '%s', '%s', '%s')"),
            m_nCurrentUserId, safeTitle, safeContent, strDueDate);
        m_db.ExecuteSQL(strSQL);

        // 2-2. 방금 생성된 업무의 request_id 가져오기
        int newRequestId = -1;
        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, _T("SELECT LAST_INSERT_ID() as id"));
        if (!rs.IsEOF()) {
            CString strId;
            rs.GetFieldValue(_T("id"), strId);
            newRequestId = _ttoi(strId);
        }
        rs.Close();

        if (newRequestId == -1) {
            throw new CDBException(); // ID를 못가져왔으면 에러 발생
        }

        // 2-3. work_assignees 테이블에 담당자 정보 INSERT
        for (int targetId : vecTargetUserIDs) {
            strSQL.Format(_T("INSERT INTO work_assignees (request_id, assignee_id) VALUES (%d, %d)"), newRequestId, targetId);
            m_db.ExecuteSQL(strSQL);
        }

        // 2-4. attachments 테이블에 첨부파일 정보 INSERT (파일 복사는 생략)
        for (size_t i = 0; i < m_vecFilePaths.size(); ++i) {
            CString fileName;
            m_listAttachments.GetText(i, fileName);
            // TODO: 실제 파일 복사 로직 추가 필요
            // CString newFilePath = "C:\\server_uploads\\" + fileName;
            // CopyFile(m_vecFilePaths[i], newFilePath, FALSE);

            CString safeFileName = fileName;
            safeFileName.Replace(_T("'"), _T("''"));

            strSQL.Format(
                _T("INSERT INTO attachments (related_type, related_id, original_filename, stored_filename, file_path, file_size) ")
                _T("VALUES ('work_request', %d, '%s', '%s', '%s', 0)"),
                newRequestId, safeFileName, safeFileName, _T("C:/server_uploads/")); // file_size는 예시로 0
            m_db.ExecuteSQL(strSQL);
        }

        m_db.CommitTrans(); // 모든 쿼리가 성공하면 최종 저장
        AfxMessageBox(_T("새 업무가 성공적으로 등록되었습니다."));

        CDialogEx::OnOK(); // 다이얼로그 닫기

    }
    catch (CDBException* e)
    {
        m_db.Rollback(); // 하나라도 실패하면 모두 취소
        AfxMessageBox(_T("업무 등록 중 오류가 발생하여 모든 작업이 취소되었습니다."));
        e->Delete();
    }
}
BOOL CNewWorkRequestDlg::ConnectDatabase()
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
HBRUSH CNewWorkRequestDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

