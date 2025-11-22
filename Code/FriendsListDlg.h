#pragma once

#include <vector>
#include <map>
#include <gdiplus.h>
#include "afxdb.h"

struct FriendInfo
{
    int userId;
    CString fullName;
    CString department;

    // 그리기 및 클릭 감지를 위한 좌표 정보
    CRect rectItem;       // 전체 아이템 영역
    CRect rectChatButton; // 채팅 버튼 영역
    CRect rectInfoButton; // 정보 버튼 영역
};

// CFriendsListDlg 대화 상자

class CFriendsListDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFriendsListDlg)

public:
	CFriendsListDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CFriendsListDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FRIENDS_LIST_DIALOG };
#endif
protected:
    CEdit m_editSearch;
    CFont m_fontSearch;

    CDatabase m_db; // DB 연결 객체
    int m_nCurrentUserId; // 현재 로그인한 사용자 ID

    std::map<CString, std::vector<FriendInfo>> m_friendsByDept;
    std::map<CString, std::vector<FriendInfo>> m_filteredFriendsByDept;

    Gdiplus::Font* m_pDeptFont;
    Gdiplus::Font* m_pNameFont;
    Gdiplus::Font* m_pButtonFont;
    CScrollBar m_scrollFriends;
    int m_nScrollPos;
    int m_nTotalContentHeight;

public:
    void InitializeAndLoadData(int userId);

    // ✨ 추가: 내부 사용 함수
protected:
    BOOL ConnectDatabase();
    void LoadFriendsListFromDB();
    void OnLButtonDown(UINT nFlags, CPoint point);
    void OnPaint();
    int GetOrCreateDirectChatRoom(int targetUserId);
    void RepositionLayout(); 
    
    void UpdateScrollbar(); 
    BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnEnChangeSearch();

	DECLARE_MESSAGE_MAP()
};
