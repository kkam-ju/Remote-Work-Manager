#pragma once
#include "afxdialogex.h"
#include <afxdb.h>

#include <vector> // CString 목록을 사용하기 위해 추가

class CNewWorkRequestDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CNewWorkRequestDlg)

public:
    CNewWorkRequestDlg(int currentUserId, CWnd* pParent = nullptr);
    virtual ~CNewWorkRequestDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_NEW_WORK_REQUEST_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

	CFont m_fontLabel; // 폰트 멤버 변수 추가
    CBrush m_whiteBrush; // 흰색 배경을 담을 브러시 변수

    virtual BOOL OnInitDialog();

    // 멤버 변수
    int m_nCurrentUserId;
    CDatabase m_db;

    // 컨트롤 변수
    CEdit m_editTitle;
    CEdit m_editContent;
    CDateTimeCtrl m_datePicker;
    CButton m_btnAddFile;
    CListBox m_listAttachments;
    CCheckListBox m_listEmployees; 

    // 첨부 파일 경로 목록
    std::vector<CString> m_vecFilePaths;

    // 헬퍼 함수
    BOOL ConnectDatabase();
    void LoadEmployeeList();

    // 메시지 핸들러
    afx_msg void OnBnClickedButtonAddFile();
    afx_msg void OnBnClickedOk();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    DECLARE_MESSAGE_MAP()
};