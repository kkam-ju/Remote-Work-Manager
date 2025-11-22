#pragma once
#include "afxdialogex.h"
#include <afxdb.h>

#pragma once

class CWriteNoticeDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CWriteNoticeDlg)

public:
    CWriteNoticeDlg(int userId, CWnd* pParent = nullptr);   // 표준 생성자입니다.
    virtual ~CWriteNoticeDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_WRITE_NOTICE_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    CFont m_fontLabel; // 폰트 멤버 변수 추가
    CBrush m_whiteBrush; // 흰색 배경을 담을 브러시 변수

    int m_nCurrentUserId;
    CDatabase m_db;
    CEdit m_editTitle;
    CEdit m_editContent;

	virtual BOOL OnInitDialog();
    BOOL ConnectDatabase();
    afx_msg void OnBnClickedOk(); // '등록'(IDOK) 버튼 핸들러
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

    DECLARE_MESSAGE_MAP()

private:
    // 사용자 ID를 저장할 멤버 변수를 추가
    int m_nUserId;

};
