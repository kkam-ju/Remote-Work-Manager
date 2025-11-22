#pragma once
#include "afxdialogex.h"
#include <vector>
#include <gdiplus.h>
#include "afxdb.h" // ODBC 관련 헤더 파일

// CCommunityDlg 대화 상자
struct NoticeInfo
{
    int noticeId;
    CString title;
    CString authorName; // 작성자 이름
    CString createdAt;  // 작성일

    CRect rectItem; // 클릭 감지를 위한 좌표
};

struct PostInfo
{
    int postId;
    CString title;
    CString createdAt;
    int commentCount; // 댓글 수

    CRect rectItem;
};



class CCommunityDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCommunityDlg)

protected:
    CDatabase m_db; // DB 연결 객체
    int m_nCurrentUserId;
    bool m_bIsAdmin;
    std::vector<NoticeInfo> m_notices;
    std::vector<PostInfo> m_posts;
    CScrollBar m_scrollNotices;
    CScrollBar m_scrollPosts;

    CRect m_rectWritePostButton;
    CRect m_rectRefreshButton;
    int m_nNoticeScrollPos;
    int m_nNoticeTotalHeight;
    int m_nPostScrollPos;
    int m_nPostTotalHeight;


public:
	CCommunityDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CCommunityDlg();
    void InitializeAndLoadData(int userId, bool isAdmin);


// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMMUNITY_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
    BOOL ConnectDatabase();
    void LoadDataFromDB();
    void OnPaint();
    void OnLButtonDown(UINT nFlags, CPoint point);
    BOOL OnInitDialog();
    void UpdateNoticeScrollbar();
    void UpdatePostScrollbar();
    CRect m_rectWriteNoticeButton;

	DECLARE_MESSAGE_MAP()

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC); // 배경 지우기 방지
};
