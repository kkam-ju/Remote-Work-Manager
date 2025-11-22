#include "pch.h"
#include "framework.h"
#include "TheMoon.h"
#include "MessengerDlg.h"
#include "afxdialogex.h"
#include "ModernButton.h"
#include "ModernEdit.h"
#include <vector>
#include <algorithm>
#include <gdiplus.h> 

using namespace Gdiplus;


IMPLEMENT_DYNAMIC(CMessengerDlg, CDialogEx)

CMessengerDlg::CMessengerDlg(CWnd* pParent )
    : CDialogEx(IDD_MESSENGER_DIALOG, pParent)
    , m_nRoomId(-1)
    , m_nCurrentChatRoomId(-1)
    , m_nScrollPos(0)           
    , m_nTotalContentHeight(0)    
{
    m_font.CreatePointFont(100, _T("맑은 고딕"));
}

CMessengerDlg::~CMessengerDlg()
{
   // KillTimer(1);
    if (m_db.IsOpen())
    {
        m_db.Close(); 
    }
}

void CMessengerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMessengerDlg, CDialogEx)
    ON_WM_PAINT()
    ON_BN_CLICKED(1001, &CMessengerDlg::OnBnClickedSend)
    ON_WM_LBUTTONDOWN()
    ON_WM_SIZE()
    ON_WM_VSCROLL()     
    ON_WM_MOUSEWHEEL()   
    ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CMessengerDlg::ConnectDatabase()
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

void CMessengerDlg::LoadChatRoomsFromDB()
{
    if (!ConnectDatabase()) return;

    if (m_nCurrentUserId == -1) {
        AfxMessageBox(_T("로그인 정보가 유효하지 않아 채팅방을 로드할 수 없습니다."));
        return;
    }

    m_chatRooms.clear();

    // SQL 쿼리 수정
    // 각 채팅방(cr.room_id)의 마지막 메시지를 서브쿼리를 이용해 함께 가져옵니다.
    // 메시지가 없는 방은 '대화를 시작해보세요.'로 표시합니다.
    CString strSQL;
    strSQL.Format(
        _T("SELECT ")
        _T("    cr.room_id, ")
        _T("    COALESCE(cr.room_name, (")
        _T("        SELECT e_other.full_name ")
        _T("        FROM participants p_other ")
        _T("        JOIN users u_other ON p_other.user_id = u_other.user_id ")
        _T("        JOIN employees e_other ON u_other.employee_id = e_other.employee_id ")
        _T("        WHERE p_other.room_id = p.room_id AND p_other.user_id != %d LIMIT 1")
        _T("    )) AS room_name_final, ")
        _T("    COALESCE(( ")
        _T("        SELECT m.content FROM messages m ")
        _T("        WHERE m.room_id = cr.room_id ")
        // 'ORDER BY' 절에 message_id DESC를 추가하여 정렬 기준을 명확하게 합니다.
        _T("        ORDER BY m.created_at DESC, m.message_id DESC LIMIT 1 ")
        _T("    ), '대화를 시작해보세요.') AS last_message ")
        _T("FROM participants p ")
        _T("JOIN chat_rooms cr ON p.room_id = cr.room_id ")
        _T("WHERE p.user_id = %d"), m_nCurrentUserId, m_nCurrentUserId);

    CRecordset rs(&m_db);
    try
    {
        rs.Open(CRecordset::forwardOnly, strSQL);

        while (!rs.IsEOF())
        {
            CString strRoomId, roomName, lastMessage;

            rs.GetFieldValue(_T("room_id"), strRoomId);
            rs.GetFieldValue(_T("room_name_final"), roomName);
            rs.GetFieldValue(_T("last_message"), lastMessage);

            ChatRoom room;
            room.roomId = _ttoi(strRoomId);
            room.roomName = roomName;
            room.lastMessage = lastMessage;
            m_chatRooms.push_back(room);

            rs.MoveNext();
        }
    }
    catch (CDBException* e)
    {
        AfxMessageBox(e->m_strError);
        e->Delete();
    }
    rs.Close();

    Invalidate();
}


void CMessengerDlg::LoadMessagesFromDB(int roomId)
{
    if (!ConnectDatabase()) return;

    m_messages.clear();

    // 특정 방의 메시지 목록을 가져오는 SQL
    CString strSQL;
    strSQL.Format(_T("SELECT m.content, m.sender_id, e.full_name AS sender_name ")
        _T("FROM messages m ")
        _T("JOIN users u ON m.sender_id = u.user_id ")
        _T("JOIN employees e ON u.employee_id = e.employee_id ")
        _T("WHERE m.room_id = %d ORDER BY m.created_at"), roomId);

    CRecordset rs(&m_db);
    try
    {
        rs.Open(CRecordset::forwardOnly, strSQL);

        while (!rs.IsEOF())
        {
            CString content, senderName;

            CString strSenderId;
            int senderId = 0; // 초기화

            rs.GetFieldValue(_T("content"), content);
            rs.GetFieldValue(_T("sender_name"), senderName);
            rs.GetFieldValue(_T("sender_id"), strSenderId);

            // CString 값이 로드되었는지 확인하고 정수로 변환
            // CString이 비어있는 경우 NULL 필드를 대비하여 체크.
            if (!strSenderId.IsEmpty())
            {
                senderId = _ttoi(strSenderId);
            }
            Message msg;
            msg.senderName = senderName;
            msg.text = content;
            msg.isMine = (senderId == m_nCurrentUserId); // 현재 사용자와 비교
            m_messages.push_back(msg);

            rs.MoveNext();
        }
    }
    catch (CDBException* e)
    {
        // DB 오류가 발생하면 AfxMessageBox에 오류 메시지 표시하여 원인을 확인.
        AfxMessageBox(e->m_strError);
        e->Delete();
    }
    rs.Close();

    UpdateScrollbar();
    ScrollToBottom(); 
    Invalidate();
}

BOOL CMessengerDlg::SendMessageToDB(int roomId, const CString& content, int senderId)
{
    if (!ConnectDatabase()) return FALSE;

    // 작은따옴표 이스케이프
    CString safeContent = content;
    safeContent.Replace(_T("'"), _T("''"));

    CString strSQL;
    strSQL.Format(_T("INSERT INTO messages (room_id, sender_id, content) VALUES (%d, %d, '%s')"),
        roomId, senderId, safeContent);

    try
    {
        m_db.ExecuteSQL(strSQL);
        return TRUE;
    }
    catch (CDBException* e)
    {
        AfxMessageBox(_T("메시지 전송 중 DB 오류 발생: SQL 삽입 실패."));
        e->Delete();
        return FALSE;
    }
}

// UI 및 이벤트 핸들러 (DB 통신 호출)

BOOL CMessengerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    CRect clientRect;
    GetClientRect(&clientRect);

    int listWidth = 200; // 채팅방 목록 너비
    int inputHeight = 30;
    int buttonWidth = 80;
    int margin = 5;

    // 입력 상자(CEdit) 생성
    m_editInput.Create(WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        CRect(listWidth + margin, clientRect.bottom - inputHeight - margin,
            clientRect.right - buttonWidth - margin * 2, clientRect.bottom - margin),
        this, 1000);

    // 2. 전송 버튼(CButton) 생성: BS_PUSHBUTTON 대신 BS_OWNERDRAW 스타일을 적용
    m_btnSend.Create(_T("전송"), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        CRect(clientRect.right - buttonWidth - margin, clientRect.bottom - inputHeight - margin,
            clientRect.right - margin, clientRect.bottom - margin),
        this, 1001);

    m_editInput.SetFont(&m_font);
   // SetTimer(1, 2000, nullptr);
    return TRUE;
}


void CMessengerDlg::LoadChatRoom(int roomId)
{
    m_nRoomId = roomId;
    LoadMessagesFromDB(roomId);
    Invalidate();
}

void CMessengerDlg::OnBnClickedSend()
{
    if (m_nRoomId == -1)
    {
        AfxMessageBox(_T("대화할 채팅방을 먼저 선택하세요."));
        return;
    }
    if (m_nCurrentUserId == -1)
    {
        AfxMessageBox(_T("로그인 정보가 유효하지 않아 메시지를 보낼 수 없습니다."));
        return;
    }

    CString msgText;
    m_editInput.GetWindowText(msgText);

    if (!msgText.IsEmpty())
    {
        // OnBnClickedSend()의 개선된 코드
        if (SendMessageToDB(m_nRoomId, msgText, m_nCurrentUserId))
        {
            LoadChatRoomsFromDB(); // <-- 이 줄을 추가하여 왼쪽 목록을 갱신합니다.
            LoadMessagesFromDB(m_nRoomId); // 기존의 오른쪽 대화 내용 갱신
            m_editInput.SetWindowText(_T(""));
            // UpdateScrollbar(); // LoadMessagesFromDB 안에서 호출되므로 중복
        }
    }
}

void CMessengerDlg::InitializeAndLoadData(int userId)
{
    // 로그인된 사용자의 ID를 멤버 변수에 저장합니다.
    m_nCurrentUserId = userId;

    // 이 시점에서 DB에 연결하고 채팅방 목록을 로드합니다.
    if (ConnectDatabase())
    {
        LoadChatRoomsFromDB();
    }
}

void CMessengerDlg::OnLButtonDown(UINT nFlags, CPoint point)
{

    for (size_t i = 0; i < m_chatRoomRects.size(); ++i)
    {
        if (m_chatRoomRects[i].PtInRect(point))
        {
            int clickedRoomId = m_chatRooms[i].roomId;
            if (m_nCurrentChatRoomId != clickedRoomId)
            {
                m_nCurrentChatRoomId = clickedRoomId;
                LoadChatRoom(clickedRoomId); // 선택된 채팅방 로드
            }
            break;
        }
    }
    CDialogEx::OnLButtonDown(nFlags, point);
}


void CMessengerDlg::OnPaint()
{
    CPaintDC dc(this);
    Graphics graphics(dc.GetSafeHdc());
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

    CRect clientRect;
    GetClientRect(&clientRect);
    int listWidth = 200;

    //1. 왼쪽 채팅방 목록 그리기
    RectF listPanelRect(0.0f, 0.0f, (float)listWidth, (float)clientRect.bottom);
    SolidBrush listPanelBrush(Color(245, 245, 245));
    graphics.FillRectangle(&listPanelBrush, listPanelRect);

    Gdiplus::Font roomNameFont(L"맑은 고딕", 11, FontStyleBold, UnitPoint);
    Gdiplus::Font lastMsgFont(L"맑은 고딕", 9, FontStyleRegular, UnitPoint);
    SolidBrush textBrush(Color(255, 0, 0, 0));
    SolidBrush selectedBrush(Color(220, 220, 220));

    m_chatRoomRects.clear();
    float currentListY = 0.0f;
    float itemHeight = 60.0f;

    for (const auto& room : m_chatRooms)
    {
        RectF itemRect(0.0f, currentListY, (float)listWidth, itemHeight);
        if (room.roomId == m_nCurrentChatRoomId)
        {
            graphics.FillRectangle(&selectedBrush, itemRect);
        }

        graphics.DrawString(room.roomName, -1, &roomNameFont, PointF(itemRect.X + 10, itemRect.Y + 10), &textBrush);
        StringFormat format;
        format.SetTrimming(StringTrimmingEllipsisCharacter);
        RectF lastMsgRect(itemRect.X + 10, itemRect.Y + 30, itemRect.Width - 20, 20);
        graphics.DrawString(room.lastMessage, -1, &lastMsgFont, lastMsgRect, &format, &textBrush);

        m_chatRoomRects.push_back(CRect((int)itemRect.GetLeft(), (int)itemRect.GetTop(), (int)itemRect.GetRight(), (int)itemRect.GetBottom()));
        currentListY += itemHeight;
    }

    // --- 2. 오른쪽 대화 내용 그리기 (수정된 부분) ---
    int inputAreaHeight = 40;

    RectF chatAreaRect((float)listWidth, 0.0f, (float)(clientRect.Width() - listWidth), (float)(clientRect.bottom - inputAreaHeight));

    // ✨ 추가: 이 영역 안에서만 그리도록 클리핑 설정
    graphics.SetClip(chatAreaRect);

    SolidBrush backBrush(Color(255, 255, 255));
    graphics.FillRectangle(&backBrush, chatAreaRect);


    SolidBrush myBubbleBrush(Color(255, 255, 236, 94));
    SolidBrush otherBubbleBrush(Color(255, 240, 240, 240));
    Gdiplus::Font chatFont(L"맑은 고딕", 10, FontStyleRegular, UnitPoint);


    Gdiplus::Font nameFont(L"맑은 고딕", 8, FontStyleRegular, UnitPoint);
    float nameMargin = 5.0f; // 말풍선과 이름 사이의 간격



    float currentChatY = 10.0f;
    currentChatY -= m_nScrollPos;

    int margin = 10;
    int bubblePadding = 10;
    int maxTextWidth = (int)chatAreaRect.Width - margin * 2 - bubblePadding * 2 - 80;

    for (const auto& msg : m_messages)
    {
        // 내가 보낸 메시지가 아닐 경우에만 이름을 표시합니다.
        if (!msg.isMine)
        {
            // 이름이 그려질 위치를 계산합니다 (말풍선 왼쪽 위).
            PointF namePoint(listWidth + margin, currentChatY);
            graphics.DrawString(msg.senderName, -1, &nameFont, namePoint, &textBrush);

            // 이름의 높이만큼 Y 좌표를 아래로 이동시켜 다음 컨트롤(말풍선)이 겹치지 않게 합니다.
            currentChatY += 15.0f; // 이름 폰트 높이를 대략 15로 가정
        }
        RectF layoutRect(0, 0, maxTextWidth > 0 ? maxTextWidth : 1, 0);
        RectF boundRect;
        graphics.MeasureString(msg.text, -1, &chatFont, layoutRect, &boundRect);

        float bubbleWidth = boundRect.Width + bubblePadding * 2;
        float bubbleHeight = boundRect.Height + bubblePadding * 2;

        // bubbleX 계산은 그대로 유지합니다.
        float bubbleX = msg.isMine ? (clientRect.right - bubbleWidth - margin) : (listWidth + margin);

        // bubbleRect의 Y 좌표는 이름이 추가된 것을 고려한 currentChatY를 사용합니다.
        RectF bubbleRect(bubbleX, currentChatY, bubbleWidth, bubbleHeight);
        SolidBrush* pBubbleBrush = msg.isMine ? &myBubbleBrush : &otherBubbleBrush;
        graphics.FillRectangle(pBubbleBrush, bubbleRect);

        RectF textRect(bubbleRect.X + bubblePadding, bubbleRect.Y + bubblePadding, boundRect.Width, boundRect.Height);
        graphics.DrawString(msg.text, -1, &chatFont, textRect, NULL, &textBrush);

        currentChatY += bubbleHeight + margin;
    }
    graphics.ResetClip();
}

void CMessengerDlg::RepositionLayout()
{
    // 컨트롤 핸들이 유효한지(생성되었는지) 확인합니다.
    if (m_editInput.GetSafeHwnd() && m_btnSend.GetSafeHwnd())
    {
        CRect clientRect;
        GetClientRect(&clientRect);

        // OnInitDialog에서 사용했던 것과 동일한 레이아웃 변수
        int listWidth = 200;
        int inputHeight = 30;
        int buttonWidth = 80;
        int margin = 5;

        // 새로운 clientRect.bottom 값을 기준으로 컨트롤들의 위치를 다시 계산
        CRect editRect(
            listWidth + margin,
            clientRect.bottom - inputHeight - margin,
            clientRect.right - buttonWidth - margin * 2,
            clientRect.bottom - margin
        );

        CRect btnRect(
            clientRect.right - buttonWidth - margin,
            clientRect.bottom - inputHeight - margin,
            clientRect.right - margin,
            clientRect.bottom - margin
        );

        // MoveWindow 함수를 이용해 컨트롤들의 위치와 크기를 한 번에 변경
        m_editInput.MoveWindow(&editRect);
        m_btnSend.MoveWindow(&btnRect);
    }
}

void CMessengerDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);
    if (nType == SIZE_MINIMIZED) return;

    RepositionLayout();
    UpdateScrollbar(); // ✨ 추가
    Invalidate();
}

void CMessengerDlg::OpenChatRoom(int roomId)
{
    m_nRoomId = roomId;
    // 1. 새로운 채팅방이 생겼을 수 있으므로, 왼쪽 채팅방 목록을 새로고침
    LoadChatRoomsFromDB();

    // 2. 전달받은 roomId로 현재 채팅방을 설정
    m_nCurrentChatRoomId = roomId;

    // 3. 해당 방의 메시지를 불러옴
    LoadMessagesFromDB(roomId);

    // 4. 화면을 완전히 새로 그려서 변경사항을 즉시 반영
    Invalidate();
}

void CMessengerDlg::UpdateScrollbar()
{
    // 1. 모든 메시지의 총 높이 계산
    CRect clientRect;
    GetClientRect(&clientRect);
    int listWidth = 200;

    float totalHeight = 10.0f; // 상단 여백
    int margin = 10;

    // OnPaint에서 그리는 로직과 거의 동일하게 높이만 계산
    Gdiplus::Graphics graphics(GetSafeHwnd());
    Gdiplus::Font chatFont(L"맑은 고딕", 10, FontStyleRegular, UnitPoint);
    int maxTextWidth = clientRect.Width() - listWidth - margin * 2 - 20 * 2 - 80;

    for (const auto& msg : m_messages)
    {
        if (!msg.isMine) {
            totalHeight += 15.0f; // 이름 높이
        }

        RectF layoutRect(0, 0, maxTextWidth > 0 ? maxTextWidth : 1, 0);
        RectF boundRect;
        graphics.MeasureString(msg.text, -1, &chatFont, layoutRect, &boundRect);

        totalHeight += boundRect.Height + 10 * 2; // 말풍선 높이 (텍스트 높이 + 패딩*2)
        totalHeight += margin; // 말풍선 간 여백
    }
    m_nTotalContentHeight = static_cast<int>(totalHeight);

    // --- 2. 스크롤바 정보 설정 ---

    int inputAreaHeight = 40;

    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = m_nTotalContentHeight;

    si.nPage = clientRect.Height() - inputAreaHeight;
    si.nPos = m_nScrollPos;
    SetScrollInfo(SB_VERT, &si, TRUE);
}

// OnVScroll 함수 전체를 교체
void CMessengerDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int oldPos = m_nScrollPos;
    CRect clientRect;
    GetClientRect(&clientRect);
    int inputAreaHeight = 40; // 하단 입력창 높이

    switch (nSBCode)
    {
    case SB_LINEUP:    m_nScrollPos -= 20; break;
    case SB_LINEDOWN:  m_nScrollPos += 20; break;
    case SB_PAGEUP:    m_nScrollPos -= (clientRect.Height() - inputAreaHeight); break;
    case SB_PAGEDOWN:  m_nScrollPos += (clientRect.Height() - inputAreaHeight); break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION: m_nScrollPos = nPos; break;
    default: return;
    }

    int maxScrollPos = m_nTotalContentHeight - (clientRect.Height() - inputAreaHeight);
    if (maxScrollPos < 0) maxScrollPos = 0;
    m_nScrollPos = max(0, min(m_nScrollPos, maxScrollPos));

    if (oldPos != m_nScrollPos)
    {
        SetScrollPos(SB_VERT, m_nScrollPos, TRUE);
        Invalidate();
    }

    CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}

// OnMouseWheel 함수 전체를 교체
BOOL CMessengerDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    int oldPos = m_nScrollPos;
    CRect clientRect;
    GetClientRect(&clientRect);
    int inputAreaHeight = 40; // 하단 입력창 높이

    if (zDelta < 0) // 아래로 스크롤
        m_nScrollPos += 60;
    else // 위로 스크롤
        m_nScrollPos -= 60;


    int maxScrollPos = m_nTotalContentHeight - (clientRect.Height() - inputAreaHeight);
    if (maxScrollPos < 0) maxScrollPos = 0;
    m_nScrollPos = max(0, min(m_nScrollPos, maxScrollPos));

    if (oldPos != m_nScrollPos)
    {
        SetScrollPos(SB_VERT, m_nScrollPos, TRUE);
        Invalidate();
    }

    return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}

// CMessengerDlg.cpp 파일 하단에 추가
void CMessengerDlg::ScrollToBottom()
{
    CRect clientRect;
    GetClientRect(&clientRect);
    int inputAreaHeight = 40;

    // 스크롤 가능한 최대 위치 계산
    int maxScrollPos = m_nTotalContentHeight - (clientRect.Height() - inputAreaHeight);
    if (maxScrollPos < 0) maxScrollPos = 0;

    // 스크롤 위치를 맨 아래로 설정하고 화면 갱신
    if (m_nScrollPos != maxScrollPos)
    {
        m_nScrollPos = maxScrollPos;
        SetScrollPos(SB_VERT, m_nScrollPos, TRUE);
        Invalidate();
    }
}

void CMessengerDlg::OnTimer(UINT_PTR nIDEvent)
{
    // 여러 타이머가 있을 수 있으므로, ID가 1인 타이머일 때만 동작하도록 합니다.
    if (nIDEvent == 1)
    {
        // 현재 열려있는 채팅방이 있을 경우에만 새로고침을 실행합니다.
        if (m_nCurrentChatRoomId != -1)
        {
            // 1. 왼쪽 채팅방 목록을 새로고침 (마지막 메시지 업데이트)
            LoadChatRoomsFromDB();

            // 2. 오른쪽 대화 내용을 새로고침
            LoadMessagesFromDB(m_nCurrentChatRoomId);
        }
    }

    CDialogEx::OnTimer(nIDEvent);
}

// MessengerDlg.cpp 파일 하단에 이 두 함수를 추가하세요.

void CMessengerDlg::StartTimer()
{
    // ID가 1인 타이머를 2초 간격으로 시작합니다.
    SetTimer(1, 2000, nullptr);
}

void CMessengerDlg::StopTimer()
{
    // ID가 1인 타이머를 중지합니다.
    KillTimer(1);
}