
// TheMoon.cpp: 애플리케이션에 대한 클래스 동작을 정의합니다.
//

#include "pch.h"
#include "framework.h"
#include "TheMoon.h"
#include "TheMoonDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTheMoonApp

BEGIN_MESSAGE_MAP(CTheMoonApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTheMoonApp 생성

CTheMoonApp::CTheMoonApp()
{
	// TODO: 여기에 생성 코드를 추가합니다.
	// InitInstance에 모든 중요한 초기화 작업을 배치합니다.
}


// 유일한 CTheMoonApp 개체입니다.

CTheMoonApp theApp;


// CTheMoonApp 초기화

BOOL CTheMoonApp::InitInstance()
{
	// 애플리케이션 매니페스트를 사용하여 ComCtl32.dll 버전 6 이상을 사용하는 경우
	// Windows XP 테마를 활성화하도록 InitCommonControlsEx()가 필요합니다. 그렇지 않으면
	// 창 만들기가 실패합니다.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 응용 프로그램에서 사용할 모든 공용 컨트롤 클래스를 포함하도록
	// 이 항목을 설정하십시오.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 대화 상자에 셸 트리 뷰 또는
	// 셸 목록 뷰 컨트롤이 포함되어 있는 경우 셸 관리자를 만듭니다.
	CShellManager* pShellManager = new CShellManager;

	// MFC 컨트롤의 테마를 사용하기 위해 "Windows 원형" 비주얼 관리자 활성화
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	SetRegistryKey(_T("로컬 애플리케이션 마법사에서 생성된 애플리케이션"));

	CTheMoonDlg dlg;
	m_pMainWnd = &dlg;

	//커맨드 라인에서 토큰 추출
	CString commandLine(m_lpCmdLine);
	CString token;
	int tokenPos = commandLine.Find(_T("token="));
	if (tokenPos != -1)
	{
		token = commandLine.Mid(tokenPos + 6);
		token.Trim(L"\"/");
	}
	// --- 토큰 추출 끝 ---

	INT_PTR nResponse;
	if (!token.IsEmpty())
	{
		// 토큰이 있으면, 검증을 먼저 시도
		if (dlg.VerifyTokenAndLogin(token))
		{
			// 토큰 검증 및 자동 로그인 성공 시 메인 다이얼로그 실행
			nResponse = dlg.DoModal();
		}
		else
		{
			// 토큰 검증 실패.
			AfxMessageBox(_T("자동 로그인에 실패했습니다."));
			nResponse = IDCANCEL;
		}
	}
	else
	{
		// 토큰이 없으면, 일반적인 방식으로 다이얼로그를 띄움
		// TODO: 나중에 별도의 로그인 다이얼로그를 먼저 띄우도록 수정할 수 있음
		nResponse = dlg.DoModal();
	}


	if (nResponse == IDOK)
	{
		// TODO: 여기에 [확인]을 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 여기에 [취소]를 클릭하여 대화 상자가 없어질 때 처리할
		//  코드를 배치합니다.
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 애플리케이션이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}

	// 위에서 만든 셸 관리자를 삭제합니다.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	return FALSE;
}