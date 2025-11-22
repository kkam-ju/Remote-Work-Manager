#pragma once
#include "afxdialogex.h"
#include "afxdb.h" // ODBC 관련 헤더 파일

// CPostDetailDlg 대화 상자

class CPostDetailDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CPostDetailDlg)

public:
    CPostDetailDlg(int postId, int currentUserId, CWnd* pParent = nullptr);
    virtual ~CPostDetailDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_POST_DETAIL_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    CFont m_fontLabel; // 폰트 멤버 변수 추가
    CBrush m_whiteBrush; // 흰색 배경을 담을 브러시 변수

    virtual BOOL OnInitDialog(); // OnInitDialog 선언

    afx_msg void OnBnClickedSubmitComment();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

    int m_nPostId;
    int m_nCurrentUserId;
    CDatabase m_db;

    CStatic m_staticPostTitle;
    CEdit m_editPostContent;
    CListBox m_listComments;
    CEdit m_editNewComment;

    BOOL ConnectDatabase();
    void LoadPostDetails();

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnStnClickedStaticComtInput();
};
