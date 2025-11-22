#include "pch.h"
#include "TheMoon.h"
#include "afxdialogex.h"
#include "CommunityDlg.h"
#include <vector>
#include <gdiplus.h>
#include "afxdb.h"

#include "NoticeDetailDlg.h"
#include "PostDetailDlg.h"     
#include "WritePostDlg.h" 
#include "WriteNoticeDlg.h"


using namespace Gdiplus;

// CCommunityDlg 대화 상자

IMPLEMENT_DYNAMIC(CCommunityDlg, CDialogEx)

CCommunityDlg::CCommunityDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMMUNITY_DIALOG, pParent)
{

}

CCommunityDlg::~CCommunityDlg()
{
}

void CCommunityDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCommunityDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_SIZE()         
    ON_WM_VSCROLL()      
    ON_WM_MOUSEWHEEL()   
    ON_WM_ERASEBKGND()                 //송
END_MESSAGE_MAP()



//1. DB 연결 함수 추가 (다른 다이얼로그와 동일)
BOOL CCommunityDlg::ConnectDatabase()
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



//2. 데이터 로드 함수 추가
void CCommunityDlg::LoadDataFromDB()
{
    if (!ConnectDatabase()) return;

    m_notices.clear();
    m_posts.clear();

    //공지사항 목록 로드
    try
    {
        // notices 테이블과 users, employees 테이블을 조인하여 작성자 이름까지 가져옴
        CString strSQL = _T("SELECT n.notice_id, n.title, e.full_name, DATE_FORMAT(n.created_at, '%Y-%m-%d') as created_at ")
            _T("FROM notices n JOIN users u ON n.user_id = u.user_id ")
            _T("JOIN employees e ON u.employee_id = e.employee_id ")
            _T("ORDER BY n.created_at DESC LIMIT 10"); // 최근 10개만

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            NoticeInfo info;
            CString strNoticeId;
            rs.GetFieldValue(_T("notice_id"), strNoticeId);
            info.noticeId = _ttoi(strNoticeId);
            rs.GetFieldValue(_T("title"), info.title);
            rs.GetFieldValue(_T("full_name"), info.authorName);
            rs.GetFieldValue(_T("created_at"), info.createdAt);
            m_notices.push_back(info);
            rs.MoveNext();
        }
        rs.Close();
    }
    catch (CDBException* e) { e->Delete(); }

    // --- 익명 게시판 목록 로드 ---
    try
    {
        // posts 테이블과 comments 테이블을 LEFT JOIN하여 게시물별 댓글 수까지 함께 가져옴
        CString strSQL = _T("SELECT p.post_id, p.title, DATE_FORMAT(p.created_at, '%Y-%m-%d') as created_at, COUNT(c.comment_id) as comment_count ")
            _T("FROM posts p LEFT JOIN comments c ON p.post_id = c.post_id ")
            _T("GROUP BY p.post_id ")
            _T("ORDER BY p.created_at DESC LIMIT 10");

        CRecordset rs(&m_db);
        rs.Open(CRecordset::forwardOnly, strSQL);
        while (!rs.IsEOF())
        {
            PostInfo info;
            CString strPostId, strCommentCount;
            rs.GetFieldValue(_T("post_id"), strPostId);
            info.postId = _ttoi(strPostId);
            rs.GetFieldValue(_T("title"), info.title);
            rs.GetFieldValue(_T("created_at"), info.createdAt);
            rs.GetFieldValue(_T("comment_count"), strCommentCount);
            info.commentCount = _ttoi(strCommentCount);
            m_posts.push_back(info);
            rs.MoveNext();
        }
        rs.Close();
    }
    catch (CDBException* e) { e->Delete(); }


    UpdateNoticeScrollbar();
    UpdatePostScrollbar();
    Invalidate();
}


void CCommunityDlg::OnPaint()
{
    CPaintDC dc(this);

    Graphics graphics(dc.GetSafeHdc());
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);

    CRect clientRect;
    GetClientRect(&clientRect);

    int scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);

    // --- 폰트, 브러시, 펜 정의 ---
    Gdiplus::Font headerFont(L"맑은 고딕", 14, FontStyleBold);
    Gdiplus::Font titleFont(L"맑은 고딕", 10, FontStyleRegular);
    Gdiplus::Font infoFont(L"맑은 고딕", 8, FontStyleRegular);
    SolidBrush backBrush(Color(255, 255, 255));
    SolidBrush headerBrush(Color(0, 0, 0));
    SolidBrush titleBrush(Color(50, 50, 50));
    SolidBrush infoBrush(Color(150, 150, 150));
    Pen linePen(Color(230, 230, 230));

    int margin = 20;

    // --- 전체 배경을 먼저 한 번에 그립니다. ---
    graphics.FillRectangle(&backBrush, RectF((float)clientRect.left, (float)clientRect.top, (float)clientRect.Width(), (float)clientRect.Height()));

    // --- 그리기 영역 정의 ---
    RectF topHalfRect(0.0f, 0.0f, (float)(clientRect.Width() - scrollBarWidth), (float)clientRect.Height() / 2.0f);
    RectF bottomHalfRect(0.0f, topHalfRect.Height, (float)(clientRect.Width() - scrollBarWidth), (float)clientRect.Height() / 2.0f);

    float itemHeight = 50.0f;

    // =======================================================
    // 1. 상단: 공지사항 그리기
    // =======================================================
    {
        graphics.SetClip(topHalfRect);

        RectF noticeHeaderRect((float)margin, 20.0f, 200.0f, 30.0f);
        float listStartY = noticeHeaderRect.GetBottom() + 10.0f;

        // 1-1. 스크롤되는 공지사항 목록을 *먼저* 그립니다.
        float currentY = listStartY;
        currentY -= m_nNoticeScrollPos;

        for (NoticeInfo& info : m_notices)
        {
            info.rectItem.SetRect(margin, (int)currentY, (int)topHalfRect.Width - margin, (int)(currentY + itemHeight));
            if (currentY + itemHeight > topHalfRect.Y && currentY < topHalfRect.GetBottom())
            {
                graphics.DrawString(info.title, -1, &titleFont, PointF((float)margin, currentY + 8.0f), &titleBrush);
                CString noticeDetails;
                noticeDetails.Format(_T("%s  |  %s"), info.authorName, info.createdAt);
                graphics.DrawString(noticeDetails, -1, &infoFont, PointF((float)margin, currentY + 30.0f), &infoBrush);
                graphics.DrawLine(&linePen, (float)margin, currentY + itemHeight - 1.0f, topHalfRect.Width - margin, currentY + itemHeight - 1.0f);
            }
            currentY += itemHeight;
        }

        // 1-2. 목록을 그린 후, 그 위에 헤더 영역을 덮어씌웁니다.
        RectF headerBarRect(topHalfRect.X, topHalfRect.Y, topHalfRect.Width, listStartY - topHalfRect.Y);
        graphics.FillRectangle(&backBrush, headerBarRect);

        // 1-3. 깨끗해진 헤더 배경 위에 제목과 버튼들을 그립니다.
        graphics.DrawString(L"📢 공지사항", -1, &headerFont, noticeHeaderRect, NULL, &headerBrush);

        Gdiplus::Font btnFont(L"맑은 고딕", 9, FontStyleRegular);

        // 새로고침 버튼 (항상 보임)
        CString refreshBtnText = _T("[ 새로고침 ]");
        RectF refreshTextBounds;
        graphics.MeasureString(refreshBtnText, -1, &btnFont, PointF(0, 0), &refreshTextBounds);
        float refreshBtnWidth = refreshTextBounds.Width + 10;
        float refreshBtnHeight = refreshTextBounds.Height;
        float refreshBtnX = topHalfRect.Width - margin - refreshBtnWidth;
        float refreshBtnY = noticeHeaderRect.Y + (noticeHeaderRect.Height - refreshBtnHeight) / 2;
        m_rectRefreshButton.SetRect((int)refreshBtnX, (int)refreshBtnY, (int)(refreshBtnX + refreshBtnWidth), (int)(refreshBtnY + refreshBtnHeight));
        graphics.DrawString(refreshBtnText, -1, &btnFont, PointF(refreshBtnX, refreshBtnY), &infoBrush);

        // 공지쓰기 버튼 (관리자일 때만 보임)
        if (m_bIsAdmin)
        {
            CString writeNoticeBtnText = _T("+ 공지쓰기");
            RectF noticeTextBounds;
            graphics.MeasureString(writeNoticeBtnText, -1, &btnFont, PointF(0, 0), &noticeTextBounds);
            float noticeBtnWidth = noticeTextBounds.Width + 20.0f;
            float noticeBtnHeight = noticeTextBounds.Height + 10.0f;
            float noticeBtnX = refreshBtnX - noticeBtnWidth - 10;
            float noticeBtnY = noticeHeaderRect.Y + (noticeHeaderRect.Height - noticeBtnHeight) / 2;
            m_rectWriteNoticeButton.SetRect((int)noticeBtnX, (int)noticeBtnY, (int)(noticeBtnX + noticeBtnWidth), (int)(noticeBtnY + noticeBtnHeight));

            SolidBrush btnBackBrush(Color(245, 245, 245));
            graphics.FillRectangle(&btnBackBrush, noticeBtnX, noticeBtnY, noticeBtnWidth, noticeBtnHeight);
            StringFormat strFormat;
            strFormat.SetAlignment(StringAlignmentCenter);
            strFormat.SetLineAlignment(StringAlignmentCenter);
            RectF noticeBtnTextRect(noticeBtnX, noticeBtnY, noticeBtnWidth, noticeBtnHeight);
            graphics.DrawString(writeNoticeBtnText, -1, &btnFont, noticeBtnTextRect, &strFormat, &headerBrush);
        }

        graphics.ResetClip();
    }

    // =======================================================
    // 2. 하단: 익명 게시판 그리기
    // =======================================================
    {
        graphics.SetClip(bottomHalfRect);

        graphics.FillRectangle(&backBrush, bottomHalfRect);

        // 헤더는 아래쪽 영역의 Y=20px 위치에 고정으로 그립니다.
        RectF postHeaderRect((float)margin, bottomHalfRect.Y + 20.0f, 200.0f, 30.0f);
        


        // [+ 글쓰기] 버튼 그리기 (고정 위치)
        Gdiplus::Font writeBtnFont(L"맑은 고딕", 9, FontStyleRegular);
        CString writeBtnText = _T("+ 글쓰기");
        RectF textBounds;
        graphics.MeasureString(writeBtnText, -1, &writeBtnFont, PointF(0, 0), &textBounds);
        float btnWidth = textBounds.Width + 20.0f;
        float btnHeight = textBounds.Height + 10.0f;
        float btnX = bottomHalfRect.Width - margin - btnWidth;
        float btnY = postHeaderRect.Y + (postHeaderRect.Height - btnHeight) / 2;
        m_rectWritePostButton.SetRect((int)btnX, (int)btnY, (int)(btnX + btnWidth), (int)(btnY + btnHeight));


        float listStartY = postHeaderRect.GetBottom() + 10.0f; // 목록이 시작될 Y 좌표
        float currentY = listStartY;
        currentY -= m_nPostScrollPos; // 스크롤 위치 적용

        for (PostInfo& info : m_posts)
        {
            info.rectItem.SetRect(margin, (int)currentY, (int)bottomHalfRect.Width - margin, (int)(currentY + itemHeight));

            // 그리기 영역(bottomHalfRect)과 겹치는 부분만 그립니다.
            if (currentY + itemHeight > bottomHalfRect.Y && currentY < bottomHalfRect.GetBottom())
            {
                CString titleWithComments;
                titleWithComments.Format(_T("%s  [%d]"), info.title, info.commentCount);
                graphics.DrawString(titleWithComments, -1, &titleFont, PointF((float)margin, currentY + 8.0f), &titleBrush);
                graphics.DrawString(info.createdAt, -1, &infoFont, PointF((float)margin, currentY + 30.0f), &infoBrush);
                graphics.DrawLine(&linePen, (float)margin, currentY + itemHeight - 1.0f, bottomHalfRect.Width - margin, currentY + itemHeight - 1.0f);
            }
            currentY += itemHeight;
        }

        // 4. ✨핵심✨: 목록을 그린 후, 그 위에 헤더 영역을 덮어씌웁니다.
       // 이렇게 하면 목록의 글자가 헤더 뒤로 가려집니다.
        RectF headerBarRect(
            bottomHalfRect.X,
            bottomHalfRect.Y,
            bottomHalfRect.Width,
            listStartY - bottomHalfRect.Y
        );
        graphics.FillRectangle(&backBrush, headerBarRect); // 헤더 배경을 다시 칠해서 아래 글자를 지움

        // 5. 이제 깨끗해진 헤더 배경 위에 제목과 버튼을 그립니다.
        graphics.DrawString(L"😎 익명 게시판", -1, &headerFont, postHeaderRect, NULL, &headerBrush);

        SolidBrush btnBackBrush(Color(240, 240, 240));
        Pen btnBorderPen(Color(200, 200, 200));
        graphics.FillRectangle(&btnBackBrush, btnX, btnY, btnWidth, btnHeight);
        graphics.DrawRectangle(&btnBorderPen, btnX, btnY, btnWidth, btnHeight);
        StringFormat strFormat;
        strFormat.SetAlignment(StringAlignmentCenter);
        strFormat.SetLineAlignment(StringAlignmentCenter);
        RectF textRect(btnX, btnY, btnWidth, btnHeight);
        graphics.DrawString(writeBtnText, -1, &writeBtnFont, textRect, &strFormat, &headerBrush);

      

        graphics.ResetClip();
    }
}



void CCommunityDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    if (m_rectRefreshButton.PtInRect(point))
    {
        LoadDataFromDB(); // 공지사항과 게시판 데이터를 모두 다시 로드합니다.
        return;           // 새로고침을 실행했으므로 함수를 종료합니다.
    }

    // ▼▼▼ 여기에 '공지쓰기' 버튼 처리 코드를 추가합니다 ▼▼▼
    if (m_bIsAdmin && m_rectWriteNoticeButton.PtInRect(point))
    {
        CWriteNoticeDlg dlg(m_nCurrentUserId, this);
        if (dlg.DoModal() == IDOK)
        {
            LoadDataFromDB(); // 새 공지사항을 반영하기 위해 데이터를 다시 로드
        }
        return; // 버튼 처리가 끝났으므로 함수를 종료
    }



    CRect clientRect;
    GetClientRect(&clientRect);
    CRect topHalfRect(0, 0, clientRect.Width(), clientRect.Height() / 2);
    CRect bottomHalfRect(0, topHalfRect.bottom, clientRect.Width(), clientRect.Height());

    if (m_rectWritePostButton.PtInRect(point))
    {
        CWritePostDlg dlg(m_nCurrentUserId, this);
        // DoModal()의 반환값이 IDOK이면 글쓰기 성공 -> 목록 새로고침
        if (dlg.DoModal() == IDOK)
        {
            LoadDataFromDB();
        }
        return;
    }

    // 공지사항 클릭 확인 (이 부분은 이미 완성됨)
    if (topHalfRect.PtInRect(point))
    {
        for (const auto& info : m_notices)
        {
            if (info.rectItem.PtInRect(point))
            {
                CNoticeDetailDlg dlg(info.noticeId, this);
                dlg.DoModal();
                return;
            }
        }
    }
    // 익명 게시판 클릭 확인
    else if (bottomHalfRect.PtInRect(point))
    {
        for (const auto& info : m_posts)
        {
            if (info.rectItem.PtInRect(point))
            {
                
                CPostDetailDlg dlg(info.postId, m_nCurrentUserId, this);
                dlg.DoModal();
                return; // 처리 후 종료
            }
        }
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}

void CCommunityDlg::InitializeAndLoadData(int userId, bool isAdmin)
{
    m_nCurrentUserId = userId;
    m_bIsAdmin = isAdmin;
    LoadDataFromDB();
}

BOOL CCommunityDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 스크롤바 컨트롤 생성
    CRect rectDummy(0, 0, 0, 0); // 처음에는 보이지 않게 생성
    m_scrollNotices.Create(SBS_VERT | WS_CHILD, rectDummy, this, 1001);
    m_scrollPosts.Create(SBS_VERT | WS_CHILD, rectDummy, this, 1002);

    return TRUE;
}

void CCommunityDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    // 창이 최소화되거나 크기가 0이면 아무것도 하지 않음
    if (nType == SIZE_MINIMIZED || cx == 0 || cy == 0) return;

    CRect clientRect;
    GetClientRect(&clientRect);
    int scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);
    int headerAreaHeight = 60; // 헤더 영역 높이

    // --- 공지사항 스크롤바 위치/크기 설정 ---
    if (m_scrollNotices.GetSafeHwnd()) {
        m_scrollNotices.MoveWindow(
            clientRect.right - scrollBarWidth,
            headerAreaHeight,
            scrollBarWidth,
            clientRect.Height() / 2 - headerAreaHeight
        );
        m_scrollNotices.ShowWindow(SW_SHOW);
    }

    // --- 게시판 스크롤바 위치/크기 설정 ---
    if (m_scrollPosts.GetSafeHwnd()) {
        m_scrollPosts.MoveWindow(
            clientRect.right - scrollBarWidth,
            clientRect.Height() / 2 + headerAreaHeight,
            scrollBarWidth,
            clientRect.Height() / 2 - headerAreaHeight
        );
        m_scrollPosts.ShowWindow(SW_SHOW);
    }

    // ★★★★★ 핵심 수정 부분 ★★★★★
    // 데이터가 로드된 상태인지 확인
    if (!m_notices.empty() || !m_posts.empty())
    {
        // 1. 올바른 창 크기로 스크롤바 정보(범위, 페이지 크기)를 다시 계산
        UpdateNoticeScrollbar();
        UpdatePostScrollbar();

        // 2. 스크롤 위치 변수와 실제 컨트롤의 위치를 모두 맨 위(0)로 강제 초기화
        m_nNoticeScrollPos = 0;
        m_nPostScrollPos = 0;
        m_scrollNotices.SetScrollPos(0);
        m_scrollPosts.SetScrollPos(0);
    }

    // 3. 위에서 설정한 상태로 화면을 즉시 다시 그림
    Invalidate();
    UpdateWindow();
}
void CCommunityDlg::UpdateNoticeScrollbar()
{
    if (!m_scrollNotices.GetSafeHwnd()) return;

    CRect clientRect;
    GetClientRect(&clientRect);

    int headerAreaHeight = 60;

    CRect noticeAreaRect(0, 0, clientRect.Width(), clientRect.Height() / 2);

    // 1. 공지사항 콘텐츠의 총 높이 계산
    float totalHeight = 20.0f + 30.0f + 10.0f; // 헤더 영역 높이
    totalHeight += m_notices.size() * 50.0f; // 아이템 높이 * 개수
    m_nNoticeTotalHeight = static_cast<int>(totalHeight);

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = m_nNoticeTotalHeight;

    si.nPage = (clientRect.Height() / 2) - headerAreaHeight;
    m_scrollNotices.SetScrollInfo(&si, TRUE);
}

void CCommunityDlg::UpdatePostScrollbar()
{
    if (!m_scrollPosts.GetSafeHwnd()) return;

    CRect clientRect;
    GetClientRect(&clientRect);

    int headerAreaHeight = 60;

    CRect postAreaRect(0, clientRect.Height() / 2, clientRect.Width(), clientRect.Height());

    // 1. 게시판 콘텐츠의 총 높이 계산
    float totalHeight = 20.0f + 30.0f + 10.0f; // 헤더 영역 높이
    totalHeight += m_posts.size() * 50.0f; // 아이템 높이 * 개수
    m_nPostTotalHeight = static_cast<int>(totalHeight);

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = m_nPostTotalHeight;

    si.nPage = (clientRect.Height() / 2) - headerAreaHeight;
    m_scrollPosts.SetScrollInfo(&si, TRUE);
}


void CCommunityDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int* pScrollPos = nullptr;
    int totalHeight = 0;
    int pageHeight = 0;
    CScrollBar* pTargetScrollBar = nullptr;

    // 1. CRect 객체를 먼저 선언합니다.
    CRect clientRect;
    // 2. GetClientRect 함수로 clientRect 변수의 내용을 채웁니다.
    GetClientRect(&clientRect);

    if (pScrollBar == &m_scrollNotices) {
        pScrollPos = &m_nNoticeScrollPos;
        totalHeight = m_nNoticeTotalHeight;
        pageHeight = clientRect.Height() / 2; // 3. 이제 clientRect 변수를 사용합니다.
        pTargetScrollBar = &m_scrollNotices;
    }
    else if (pScrollBar == &m_scrollPosts) {
        pScrollPos = &m_nPostScrollPos;
        totalHeight = m_nPostTotalHeight;
        pageHeight = clientRect.Height() / 2; // 3. 이제 clientRect 변수를 사용합니다.
        pTargetScrollBar = &m_scrollPosts;
    }
    else {
        CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
        return;
    }

    int oldPos = *pScrollPos;

    switch (nSBCode) {
        case SB_LINEUP:    *pScrollPos -= 20; break;
        case SB_LINEDOWN:  *pScrollPos += 20; break;
        case SB_PAGEUP:    *pScrollPos -= pageHeight; break;
        case SB_PAGEDOWN:  *pScrollPos += pageHeight; break;
        case SB_THUMBTRACK: *pScrollPos = nPos; break;
    }

    int maxScrollPos = totalHeight - pageHeight;
    if (maxScrollPos < 0) maxScrollPos = 0;
    *pScrollPos = max(0, min(*pScrollPos, maxScrollPos));

    if (oldPos != *pScrollPos) {
        pTargetScrollBar->SetScrollPos(*pScrollPos);
        Invalidate();
    }
}

BOOL CCommunityDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    CRect clientRect;
    GetClientRect(&clientRect);
    CRect topHalf(0, 0, clientRect.Width(), clientRect.Height() / 2);

    // 마우스 커서가 어느 영역에 있는지 확인
    if (topHalf.PtInRect(pt)) {
        // 공지사항 스크롤
        int oldPos = m_nNoticeScrollPos;
        m_nNoticeScrollPos -= zDelta / 2; // zDelta는 보통 120의 배수

        int maxScrollPos = m_nNoticeTotalHeight - topHalf.Height();
        if (maxScrollPos < 0) maxScrollPos = 0;
        m_nNoticeScrollPos = max(0, min(m_nNoticeScrollPos, maxScrollPos));
        
        if (oldPos != m_nNoticeScrollPos) {
            m_scrollNotices.SetScrollPos(m_nNoticeScrollPos);
            Invalidate();
        }
    }
    else {
        // 게시판 스크롤
        int oldPos = m_nPostScrollPos;
        m_nPostScrollPos -= zDelta / 2;

        CRect bottomHalf(0, topHalf.bottom, clientRect.Width(), clientRect.Height());
        int maxScrollPos = m_nPostTotalHeight - bottomHalf.Height();
        if (maxScrollPos < 0) maxScrollPos = 0;
        m_nPostScrollPos = max(0, min(m_nPostScrollPos, maxScrollPos));

        if (oldPos != m_nPostScrollPos) {
            m_scrollPosts.SetScrollPos(m_nPostScrollPos);
            Invalidate();
        }
    }

    return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

BOOL CCommunityDlg::OnEraseBkgnd(CDC* pDC)
{
    // 배경 지우기 기능을 OnPaint에서 완전히 처리하도록 하고,
    // 기본 기능은 비활성화하여 깜빡임과 잔상을 방지합니다.
    return TRUE;
}