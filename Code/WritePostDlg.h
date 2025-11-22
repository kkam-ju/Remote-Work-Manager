#pragma once
#include "afxdialogex.h"
#include <afxdb.h>

#pragma once

class CWritePostDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CWritePostDlg)

public:
    CWritePostDlg(int currentUserId, CWnd* pParent = nullptr); // 생성자 수정
    virtual ~CWritePostDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_WRITE_POST_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    CFont m_fontLabel; // 폰트 멤버 변수 추가
    CBrush m_whiteBrush; // 흰색 배경을 담을 브러시 변수

    int m_nCurrentUserId;
    CDatabase m_db;
    CEdit m_editTitle;
    CEdit m_editContent;

    BOOL ConnectDatabase();
	virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk(); // '등록'(IDOK) 버튼 핸들러
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    DECLARE_MESSAGE_MAP()
};
