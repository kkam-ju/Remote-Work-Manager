
#include "afxdb.h" 
#include <vector>  
#include <algorithm>
#include <gdiplus.h> 
#include "ModernButton.h"
#include "ModernEdit.h"


// 채팅방 정보를 담는 구조체
struct ChatRoom
{
    int roomId;
    CString roomName;
    CString lastMessage;
};

// 메시지 정보를 담는 구조체
struct Message
{
    CString senderName;
    CString text;
    bool isMine;
};


class CMessengerDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CMessengerDlg)

public:
    CMessengerDlg(CWnd* pParent = nullptr);
    virtual ~CMessengerDlg();
    void SetCurrentUserId(int userId) { m_nCurrentUserId = userId; }
    void LoadChatRoomsOnStart() { LoadChatRoomsFromDB(); } // 필요 시 외부 호출용
    void InitializeAndLoadData(int userId);
    void OpenChatRoom(int roomId); 
    void RepositionLayout();
    void StartTimer();
    void StopTimer();
    // 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MESSENGER_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnBnClickedSend(); // ID 1001
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTimer(UINT_PTR nIDEvent); //메세지 실시간수신을 위한 타이머 
    DECLARE_MESSAGE_MAP()

protected:
    CModernEdit m_editInput;
    CModernButton m_btnSend;
    CFont m_font;
    int m_nScrollPos;           // 현재 스크롤의 위치 (Y 좌표)
    int m_nTotalContentHeight;  // 모든 메시지를 그렸을 때의 총 높이

    void UpdateScrollbar();
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);

private:
    
    int m_nRoomId; // 현재 선택된 메시지 로드용
    int m_nCurrentChatRoomId; // UI 상태 관리용
    int m_nCurrentUserId = -1; //로그인된 사용자 ID 

    std::vector<ChatRoom> m_chatRooms;
    std::vector<CRect> m_chatRoomRects;
    std::vector<Message> m_messages;

    // ODBC 관련 멤버 변수
    CDatabase m_db;             // 데이터베이스 연결 객체
    BOOL ConnectDatabase();     // DB 연결 메서드

    // DB 접근 함수
    void LoadChatRoomsFromDB();
    void LoadMessagesFromDB(int roomId);
    BOOL SendMessageToDB(int roomId, const CString& content, int senderId);
    void LoadChatRoom(int roomId);
    void ScrollToBottom();
};