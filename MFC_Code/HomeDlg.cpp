// HomeDlg.cpp: 구현 파일
//

#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "HomeDlg.h"
#include "NewWorkRequestDlg.h" // 새 업무 등록 다이얼로그 헤더 포함
#include "WorkDetailDlg.h" // ✨ 추가: WorkDetailDlg.h 헤더 포함
#include "MyInfoDlg.h"
// CHomeDlg 대화 상자

IMPLEMENT_DYNAMIC(CHomeDlg, CDialogEx)

CHomeDlg::CHomeDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_HOME_DIALOG, pParent)
    , m_nCurrentUserId(-1)
    , m_bIsAdmin(false)
{
}

CHomeDlg::~CHomeDlg()
{
}

void CHomeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CHomeDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CHomeDlg 메시지 처리기
BOOL CHomeDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    return TRUE;
}

void CHomeDlg::InitializeAndLoadData(int userId, bool isAdmin)
{
    m_nCurrentUserId = userId;
    m_bIsAdmin = isAdmin;
    LoadCurrentUserInfo();
    LoadWorkRequestsFromDB();
}

BOOL CHomeDlg::ConnectDatabase()
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

void CHomeDlg::LoadWorkRequestsFromDB()
{
    if (!ConnectDatabase()) return;

    m_workRequests.clear();
    CString strSQL;

    if (m_bIsAdmin)
    {
        // 관리자인 경우: 자신이 등록한 모든 업무를 조회
        strSQL.Format(
            _T("SELECT wr.request_id, wr.status, wr.title, e.full_name, DATE_FORMAT(wr.due_date, '%%Y-%%m-%%d') as due_date ")
            _T("FROM work_requests wr JOIN users u ON wr.user_id = u.user_id JOIN employees e ON u.employee_id = e.employee_id ")
            _T("WHERE wr.user_id = %d ORDER BY wr.created_at DESC"), m_nCurrentUserId);
    }
    else
    {
        // 일반 직원인 경우: 자신에게 할당된 업무만 조회
        strSQL.Format(
            _T("SELECT wr.request_id, wr.status, wr.title, e.full_name, DATE_FORMAT(wr.due_date, '%%Y-%%m-%%d') as due_date ")
            _T("FROM work_requests wr ")
            _T("JOIN work_assignees wa ON wr.request_id = wa.request_id ")
            _T("JOIN users u ON wr.user_id = u.user_id ")
            _T("JOIN employees e ON u.employee_id = e.employee_id ")
            _T("WHERE wa.assignee_id = %d ORDER BY wr.due_date ASC"), m_nCurrentUserId);
    }

    // ✨ 디버깅을 위한 카운터 변수
    int nRowsFound = 0;

    try
    {
        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            nRowsFound++; // ✨ 찾은 데이터 개수 증가

            WorkRequestInfo info;
            CString strRequestId;
            rs.GetFieldValue(_T("request_id"), strRequestId);
            info.requestId = _ttoi(strRequestId);
            rs.GetFieldValue(_T("status"), info.status);
            rs.GetFieldValue(_T("title"), info.title);
            rs.GetFieldValue(_T("full_name"), info.requesterName);
            rs.GetFieldValue(_T("due_date"), info.dueDate);
            m_workRequests.push_back(info);
            rs.MoveNext();
        }
        rs.Close();
    }
    catch (CDBException* e)
    {
        AfxMessageBox(_T("업무 목록 로딩 중 DB 오류 발생:\n") + e->m_strError);
        e->Delete();
    }

   

    Invalidate();
}

void CHomeDlg::OnPaint()
{


    CPaintDC dc(this);
    Graphics graphics(dc.GetSafeHdc());
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    CRect clientRect;
    GetClientRect(&clientRect);
    SolidBrush backBrush(Color::White);
    graphics.FillRectangle(&backBrush, 0, 0, clientRect.Width(), clientRect.Height());

    // --- 폰트 및 브러시 선언 ---
    Gdiplus::Font headerFont(L"맑은 고딕", 12, FontStyleBold);
    Gdiplus::Font titleFont(L"맑은 고딕", 10, FontStyleRegular);
    Gdiplus::Font infoFont(L"맑은 고딕", 9, FontStyleRegular);
    Gdiplus::Font myInfoTextFont(L"맑은 고딕", 10, FontStyleRegular);
    Gdiplus::Font myInfoBtnFont(L"맑은 고딕", 9, FontStyleRegular);

    SolidBrush blackBrush(Color::Black);
    SolidBrush grayBrush(Color(120, 120, 120));
    Pen linePen(Color(220, 220, 220));

    int margin = 30;
    float currentY = 20.0f;

    // --- 1. 헤더 및 버튼 그리기 (수정된 부분) ---
    graphics.DrawString(m_bIsAdmin ? L"등록한 업무 목록" : L"할당된 업무 목록", -1, &headerFont, PointF((float)margin, currentY), &blackBrush);

    // 1-1. 새로고침 버튼 위치 계산 및 그리기 (항상 보임)
    CString refreshBtnText = _T("[ 새로고침 ]");
    RectF refreshTextBounds;
    graphics.MeasureString(refreshBtnText, -1, &myInfoBtnFont, PointF(0, 0), &refreshTextBounds);

    float refreshBtnWidth = refreshTextBounds.Width + 10;
    float refreshBtnHeight = refreshTextBounds.Height;
    float refreshBtnX = clientRect.Width() - margin - refreshBtnWidth;
    float refreshBtnY = currentY;

    m_rectRefreshButton.SetRect((int)refreshBtnX, (int)refreshBtnY, (int)(refreshBtnX + refreshBtnWidth), (int)(refreshBtnY + refreshBtnHeight));
    graphics.DrawString(refreshBtnText, -1, &myInfoBtnFont, PointF(refreshBtnX, refreshBtnY), &grayBrush);

    // 1-2. '+ 새 업무 등록' 버튼 위치 계산 및 그리기 (관리자일 때만 보임)
    if (m_bIsAdmin)
    {
        CString btnText = _T("+ 새 업무 등록");
        RectF textBounds;
        graphics.MeasureString(btnText, -1, &myInfoBtnFont, PointF(0, 0), &textBounds);
        float btnWidth = textBounds.Width + 20;
        float btnHeight = textBounds.Height + 10;

        // 새로고침 버튼의 바로 왼쪽에 배치되도록 X 좌표 계산
        float btnX = refreshBtnX - btnWidth - 10; // 10px 간격
        float btnY = currentY;
        m_rectNewRequestButton.SetRect((int)btnX, (int)btnY, (int)(btnX + btnWidth), (int)(btnY + btnHeight));

        SolidBrush btnBackBrush(Color(245, 245, 245));
        graphics.FillRectangle(&btnBackBrush, m_rectNewRequestButton.left, m_rectNewRequestButton.top, m_rectNewRequestButton.Width(), m_rectNewRequestButton.Height());
        StringFormat strFormat; strFormat.SetAlignment(StringAlignmentCenter); strFormat.SetLineAlignment(StringAlignmentCenter);
        graphics.DrawString(btnText, -1, &myInfoBtnFont, RectF((float)m_rectNewRequestButton.left, (float)m_rectNewRequestButton.top, (float)m_rectNewRequestButton.Width(), (float)m_rectNewRequestButton.Height()), &strFormat, &blackBrush);
    }
    currentY += 40;

    // --- 2. 목록 컬럼 헤더 그리기 ---
    graphics.DrawString(L"상태", -1, &infoFont, PointF((float)margin, currentY), &grayBrush);
    graphics.DrawString(L"제목", -1, &infoFont, PointF((float)margin + 100, currentY), &grayBrush);
    graphics.DrawString(L"요청자", -1, &infoFont, PointF((float)margin + 400, currentY), &grayBrush);
    graphics.DrawString(L"기한", -1, &infoFont, PointF((float)margin + 500, currentY), &grayBrush);
    currentY += 20;
    graphics.DrawLine(&linePen, (float)margin, currentY, (float)(clientRect.Width() - margin), currentY);
    currentY += 10;

    // --- 3. 업무 목록 그리기 ---
    for (WorkRequestInfo& info : m_workRequests)
    {
        float itemHeight = 40.0f;
        info.rectItem.SetRect(margin, (int)currentY, clientRect.Width() - margin, (int)(currentY + itemHeight));

        // 상태 표시 (색상 태그)
        SolidBrush statusBrush(Color(200, 200, 200)); // 기본 회색
        if (info.status == _T("처리중")) statusBrush.SetColor(Color(204, 230, 255)); // 파란색
        else if (info.status == _T("완료")) statusBrush.SetColor(Color(214, 245, 214)); // 녹색
        else if (info.status == _T("반려")) statusBrush.SetColor(Color(255, 214, 214)); // 빨간색
        graphics.FillRectangle(&statusBrush, (float)margin, currentY + 5, 70.0f, itemHeight - 15);
        StringFormat strFormat; strFormat.SetAlignment(StringAlignmentCenter); strFormat.SetLineAlignment(StringAlignmentCenter);
        graphics.DrawString(info.status, -1, &infoFont, RectF((float)margin, currentY + 5, 70.0f, itemHeight - 15), &strFormat, &grayBrush);

        // 나머지 정보 표시
        graphics.DrawString(info.title, -1, &titleFont, PointF((float)margin + 100, currentY + 10), &blackBrush);
        graphics.DrawString(info.requesterName, -1, &infoFont, PointF((float)margin + 400, currentY + 12), &grayBrush);
        graphics.DrawString(info.dueDate, -1, &infoFont, PointF((float)margin + 500, currentY + 12), &grayBrush);

        currentY += itemHeight;
        graphics.DrawLine(&linePen, (float)margin, currentY, (float)(clientRect.Width() - margin), currentY);
    }
    // HomeDlg.cpp - OnPaint() 함수 내부

  // --- 4. 사용자 정보 텍스트 (이름, 전화번호, 이메일) 및 '내 정보 수정' 버튼 그리기 ---

    float myInfoAreaTop = clientRect.Height() - margin - 80; // 텍스트와 버튼이 그려질 영역의 시작 높이
    float myInfoTextX = (float)margin;


    // 이름 텍스트 그리기
    CString nameText;
    nameText.Format(_T("이름: %s"), m_strUserName.IsEmpty() ? _T("정보 없음") : m_strUserName);
    graphics.DrawString(nameText, -1, &myInfoTextFont, PointF(myInfoTextX, myInfoAreaTop), &blackBrush);
    myInfoAreaTop += 25; // 다음 줄로 Y 좌표 이동

    // 전화번호 텍스트 그리기
    CString phoneText;
    phoneText.Format(_T("전화번호: %s"), m_strUserPhone.IsEmpty() ? _T("정보 없음") : m_strUserPhone);
    graphics.DrawString(phoneText, -1, &myInfoTextFont, PointF(myInfoTextX, myInfoAreaTop), &blackBrush);
    myInfoAreaTop += 25; // 다음 줄로 Y 좌표 이동

    // 이메일 텍스트 그리기
    CString emailText;
    emailText.Format(_T("이메일: %s"), m_strUserEmail.IsEmpty() ? _T("정보 없음") : m_strUserEmail);
    graphics.DrawString(emailText, -1, &myInfoTextFont, PointF(myInfoTextX, myInfoAreaTop), &blackBrush);

    // '내 정보 수정' 버튼 그리기 (우측 하단)
    CString btnText_MyInfo = _T("내 정보 수정");
    RectF btnTextBounds_MyInfo;
    graphics.MeasureString(btnText_MyInfo, -1, &myInfoBtnFont, PointF(0, 0), &btnTextBounds_MyInfo);
    float btnWidth_MyInfo = btnTextBounds_MyInfo.Width + 20;
    float btnHeight_MyInfo = btnTextBounds_MyInfo.Height + 10;

    float btnX_MyInfo = clientRect.Width() - margin - btnWidth_MyInfo;
    float btnY_MyInfo = clientRect.Height() - margin - btnHeight_MyInfo;
    m_rectMyInfoButton.SetRect((int)btnX_MyInfo, (int)btnY_MyInfo, (int)(btnX_MyInfo + btnWidth_MyInfo), (int)(btnY_MyInfo + btnHeight_MyInfo));

    SolidBrush btnBackBrush_MyInfo(Color(245, 245, 245));
    graphics.FillRectangle(&btnBackBrush_MyInfo, (float)m_rectMyInfoButton.left, (float)m_rectMyInfoButton.top, (float)m_rectMyInfoButton.Width(), (float)m_rectMyInfoButton.Height());

    StringFormat btnStrFormat_MyInfo;
    btnStrFormat_MyInfo.SetAlignment(StringAlignmentCenter);
    btnStrFormat_MyInfo.SetLineAlignment(StringAlignmentCenter);
    graphics.DrawString(btnText_MyInfo, -1, &myInfoBtnFont, RectF((float)m_rectMyInfoButton.left, (float)m_rectMyInfoButton.top, (float)m_rectMyInfoButton.Width(), (float)m_rectMyInfoButton.Height()), &btnStrFormat_MyInfo, &blackBrush);
}

void CHomeDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_rectRefreshButton.PtInRect(point))
    {
        LoadCurrentUserInfo();    // 사용자 정보 다시 로드
        LoadWorkRequestsFromDB(); // 업무 목록 다시 로드 (이 함수 안에 Invalidate() 포함)
        return; // 처리 완료
    }
    // '+ 새 업무 등록' 버튼 클릭 확인 (기존과 동일)
    if (m_bIsAdmin && m_rectNewRequestButton.PtInRect(point))
    {
        CNewWorkRequestDlg dlg(m_nCurrentUserId, this);
        if (dlg.DoModal() == IDOK)
        {
            LoadWorkRequestsFromDB(); // 등록 성공 시 목록 새로고침
        }
        return;
    }
    // --- ✨ '내 정보 수정' 버튼 클릭 확인✨ ---
    if (m_rectMyInfoButton.PtInRect(point))
    {
        // 현재 로그인한 사용자 ID를 전달하며 MyInfoDlg를 생성합니다.
        CMyInfoDlg dlg(m_nCurrentUserId, this);
        // DoModal()의 반환값을 확인하여 '닫기' 버튼을 눌렀는지 체크합니다.
        if (dlg.DoModal() == IDOK)
        {
            // '닫기' 버튼(IDOK)을 눌렀을 경우에만 이 부분이 실행됩니다.
            LoadCurrentUserInfo(); // 1. DB에서 사용자 정보를 다시 불러옵니다.
            Invalidate();          // 2. 화면을 강제로 새로고침합니다.
        }
        return;
    }
    // 목록 아이템 클릭 확인
    for (const auto& info : m_workRequests)
    {
        if (info.rectItem.PtInRect(point))
        {
            // ✨✨ 수정된 부분 ✨✨
            // 기존 AfxMessageBox 대신 CWorkDetailDlg 띄우기
            CWorkDetailDlg dlg(info.requestId, m_nCurrentUserId, m_bIsAdmin, this);

            // 상세 보기 창에서 '상태 변경'이 성공하면 IDOK가 반환됨
            if (dlg.DoModal() == IDOK)
            {
                LoadWorkRequestsFromDB(); // 목록을 새로고침하여 변경된 상태를 반영
            }
            return;
        }
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}

void CHomeDlg::LoadCurrentUserInfo()
{
    if (!ConnectDatabase()) return;

    try
    {
        CString strSQL;
        strSQL.Format(_T("SELECT u.phone_number, u.email, e.full_name ")
            _T("FROM users u JOIN employees e ON u.employee_id = e.employee_id ")
            _T("WHERE u.user_id = %d"), m_nCurrentUserId);
        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);

        if (!rs.IsEOF())
        {
            // DB에서 값을 읽어와 멤버 변수에 저장
            rs.GetFieldValue(_T("phone_number"), m_strUserPhone);
            rs.GetFieldValue(_T("email"), m_strUserEmail);
            rs.GetFieldValue(_T("full_name"), m_strUserName);
        }
        rs.Close();
    }
    catch (CDBException* e)
    {
        AfxMessageBox(_T("사용자 정보를 불러오는 중 오류가 발생했습니다."));
        e->Delete();
    }
}