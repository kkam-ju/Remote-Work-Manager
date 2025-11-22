#pragma once

#include "HomeDlg.h"
#include "CommunityDlg.h"
#include "MessengerDlg.h"
#include "FriendsListDlg.h"


class CTheMoonDlg : public CDialogEx
{
	// 생성입니다.
public:
	CTheMoonDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CTheMoonDlg(); 
	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_THEMOON_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


	// 구현입니다.
protected:
	HICON m_hIcon;

	ULONG_PTR m_gdiplusToken;

	CRect m_rectHomeButton;
	CRect m_rectCommunityButton;
	CRect m_rectMessengerButton;
	CRect m_rectFriendsButton;
	CRect m_rectExitButton;

	// 현재 선택된 뷰를 나타내는 변수
	int m_nCurrentView; // 0: 홈, 1: 커뮤니티, 2: 메신저, 3: 친구

	// 각 화면을 표시할 자식 다이얼로그 포인터
	CHomeDlg* m_pHomeDlg;
	CCommunityDlg* m_pCommunityDlg;
	CMessengerDlg* m_pMessengerDlg;
	CFriendsListDlg* m_pFriendsListDlg;

	// GDI+로 버튼을 그리는 헬퍼 함수
	void DrawButton(Gdiplus::Graphics& graphics, const CRect& rect, CString text, bool isSelected);

	// 특정 뷰를 보여주는 헬퍼 함수
	void ShowView(int nViewIndex);
	bool m_bIsAdmin; 

	void RecalcButtonLayout(); 
	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy); 
	afx_msg LRESULT OnStartChat(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
public:
	BOOL VerifyTokenAndLogin(const CString& token); 
	

private:
	int m_nLoggedInUserId = -1; 
};