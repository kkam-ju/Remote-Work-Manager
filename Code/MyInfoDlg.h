#pragma once
#include "afxdialogex.h"
#include "afxdb.h"

class CMyInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMyInfoDlg)

public:
	CMyInfoDlg(int userId, CWnd* pParent = nullptr);
	virtual ~CMyInfoDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MY_INFO_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	CFont m_fontLabel; // 폰트 멤버 변수 추가
	CBrush m_whiteBrush; // 흰색 배경을 담을 브러시 변수
	

	virtual BOOL OnInitDialog();

	// 멤버 변수 선언
	int m_nCurrentUserId;
	CDatabase m_db;

	// 컨트롤 변수 선언 (2단계에서 마법사로 연결됨)
	CEdit m_editPhone;
	CEdit m_editEmail;
	CEdit m_editNewPassword;
	CEdit m_editConfirmPassword;

	// 내부 함수 선언
	BOOL ConnectDatabase();
	void LoadCurrentUserData();

	// 버튼 클릭 함수 선언
	afx_msg void OnBnClickedChangePhone();
	afx_msg void OnBnClickedChangeEmail();
	afx_msg void OnBnClickedChangePassword();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	DECLARE_MESSAGE_MAP()
};