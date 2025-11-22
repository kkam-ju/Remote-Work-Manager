#include "pch.h"
#include "framework.h"
#include "TheMoon.h"
#include "TheMoonDlg.h"
#include "afxdialogex.h"

#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <afxdb.h>
using namespace web;
using namespace web::http;
using namespace web::http::client;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CTheMoonDlg::CTheMoonDlg(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_THEMOON_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    // GDI+를 사용하기 위해 초기화합니다.
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

    m_nCurrentView = 0;
    m_pHomeDlg = nullptr;
    m_pCommunityDlg = nullptr;
    m_pMessengerDlg = nullptr;
    m_pFriendsListDlg = nullptr;

    // m_nLoggedInUserId는 헤더에서 -1로 초기화됨
}


BOOL CTheMoonDlg::VerifyTokenAndLogin(const CString& token)
{
    BOOL bLoginSuccess = FALSE;

    json::value post_data;
    std::wstring token_wstr(token);
    post_data[L"token"] = json::value::string(token_wstr);

    http_client_config config;
    config.set_validate_certificates(false);

    http_client client(U("https://192.168.0.81:5000"), config);

    try
    {
        http_response response = client.request(methods::POST, U("/api/mfc/verify-token"), post_data).get();

        if (response.status_code() == status_codes::OK)
        {
            json::value json_response = response.extract_json().get();
            if (!json_response.is_null() && json_response.at(U("success")).as_bool())
            {
                // ✨ 자동 로그인 성공! ✨
                auto user_info = json_response.at(U("user_info"));

                // 1. user_id 추출 및 멤버 변수에 저장 (가장 중요!)
                int userId = user_info.at(U("user_id")).as_integer();
                this->m_nLoggedInUserId = userId;

                std::wstring full_name = user_info.at(U("full_name")).as_string();
                bool isAdmin = user_info.at(U("is_admin")).as_bool();
                this->m_bIsAdmin = isAdmin;

                bLoginSuccess = TRUE;
            }
        }
    }
    catch (const std::exception& e)
    {
        CString error;
        error.Format(_T("Token verification failed: %s"), CString(e.what()));
        AfxMessageBox(error);
        bLoginSuccess = FALSE;
    }

    return bLoginSuccess;
}

// 소멸자 추가: 프로그램 종료 시 GDI+를 종료하고 동적 할당된 메모리를 해제합니다.
CTheMoonDlg::~CTheMoonDlg()
{
    // 동적 할당된 자식 다이얼로그 메모리 해제
    delete m_pHomeDlg;
    delete m_pCommunityDlg;
    delete m_pMessengerDlg;
    delete m_pFriendsListDlg;

    // GDI+를 종료합니다.
    GdiplusShutdown(m_gdiplusToken);
}

void CTheMoonDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTheMoonDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_LBUTTONDOWN()
    ON_WM_SIZE()
    ON_BN_CLICKED(IDC_BTN_TEST, &CTheMoonDlg::OnBnClickedButton1)
    ON_MESSAGE(WM_USER_START_CHAT, &CTheMoonDlg::OnStartChat)
END_MESSAGE_MAP()



BOOL CTheMoonDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    RecalcButtonLayout();


    m_pHomeDlg = new CHomeDlg();
    m_pHomeDlg->Create(IDD_HOME_DIALOG, this);
    if (m_pHomeDlg) {
        m_pHomeDlg->InitializeAndLoadData(m_nLoggedInUserId, m_bIsAdmin);
    }

    m_pCommunityDlg = new CCommunityDlg();
    m_pCommunityDlg->Create(IDD_COMMUNITY_DIALOG, this);
    if (m_pCommunityDlg) {
        m_pCommunityDlg->InitializeAndLoadData(m_nLoggedInUserId, m_bIsAdmin);
    }

    m_pMessengerDlg = new CMessengerDlg();
    m_pMessengerDlg->Create(IDD_MESSENGER_DIALOG, this);

    if (m_pMessengerDlg) {
        m_pMessengerDlg->InitializeAndLoadData(m_nLoggedInUserId);
    }


    m_pFriendsListDlg = new CFriendsListDlg();
    m_pFriendsListDlg->Create(IDD_FRIENDS_LIST_DIALOG, this);

    if (m_pFriendsListDlg) {
        m_pFriendsListDlg->InitializeAndLoadData(m_nLoggedInUserId);
    }

    ShowView(m_nCurrentView);
    return TRUE;
}


   

void CTheMoonDlg::OnPaint()
{
    if (IsIconic())
    {
    }
    else
    {
        CPaintDC dc(this);

        // pch.h에 using namespace Gdiplus;를 추가했으므로 'Gdiplus::'를 생략할 수 있습니다.
        Graphics graphics(dc.GetSafeHdc());
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);

        // 배경 그리기
        CRect clientRect;
        GetClientRect(&clientRect);

        RectF gdiClientRect(
            static_cast<float>(clientRect.left),
            static_cast<float>(clientRect.top),
            static_cast<float>(clientRect.Width()),
            static_cast<float>(clientRect.Height())
        );

        SolidBrush whiteBrush(Color(255, 255, 255, 255));
        graphics.FillRectangle(&whiteBrush, gdiClientRect);

        // DrawButton 함수를 호출하여 버튼들을 그립니다.
        DrawButton(graphics, m_rectHomeButton, L"홈", m_nCurrentView == 0);
        DrawButton(graphics, m_rectCommunityButton, L"커뮤니티", m_nCurrentView == 1);
        DrawButton(graphics, m_rectMessengerButton, L"메신저", m_nCurrentView == 2);
        DrawButton(graphics, m_rectFriendsButton, L"사원", m_nCurrentView == 3);
        DrawButton(graphics, m_rectExitButton, L"종료", false);
    }
}

HCURSOR CTheMoonDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void CTheMoonDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    bool bViewChanged = false;
    if (m_rectHomeButton.PtInRect(point) && m_nCurrentView != 0)
    {
        ShowView(0);
        bViewChanged = true;
    }
    else if (m_rectCommunityButton.PtInRect(point) && m_nCurrentView != 1)
    {
        ShowView(1);
        bViewChanged = true;
    }
    else if (m_rectMessengerButton.PtInRect(point) && m_nCurrentView != 2)
    {
        ShowView(2);
        bViewChanged = true;
    }
    else if (m_rectFriendsButton.PtInRect(point) && m_nCurrentView != 3)
    {
        ShowView(3);
        bViewChanged = true;
    }
    else if (m_rectExitButton.PtInRect(point))
    {
        OnOK(); // 다이얼로그를 닫고 프로그램을 종료합니다.
        return;
    }

    if (bViewChanged)
    {
        CRect rectButtons(0, 0, 80, 500);
        InvalidateRect(&rectButtons);
    }

    CDialogEx::OnLButtonDown(nFlags, point);
}

void CTheMoonDlg::ShowView(int nViewIndex)
{
    // 뷰를 바꾸기 전에, 메신저 타이머를 일단 끕니다.
    if (m_pMessengerDlg)
    {
        m_pMessengerDlg->StopTimer();
    }

    if (m_pHomeDlg) m_pHomeDlg->ShowWindow(SW_HIDE);
    if (m_pCommunityDlg) m_pCommunityDlg->ShowWindow(SW_HIDE);
    if (m_pMessengerDlg) m_pMessengerDlg->ShowWindow(SW_HIDE);
    if (m_pFriendsListDlg) m_pFriendsListDlg->ShowWindow(SW_HIDE);

    CRect clientRect;
    GetClientRect(&clientRect);
    int leftPanelWidth = 80;
    CRect viewRect(leftPanelWidth, 0, clientRect.right, clientRect.bottom);

    switch (nViewIndex)
    {
    case 0: // 홈
        if (m_pHomeDlg) {
            m_pHomeDlg->SetWindowPos(NULL, viewRect.left, viewRect.top, viewRect.Width(), viewRect.Height(), SWP_NOZORDER);
            m_pHomeDlg->ShowWindow(SW_SHOW);
            m_pHomeDlg->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(viewRect.Width(), viewRect.Height()));
            m_pHomeDlg->RedrawWindow();
        }
        break;
    case 1: // 커뮤니티
        if (m_pCommunityDlg) {
            m_pCommunityDlg->SetWindowPos(NULL, viewRect.left, viewRect.top, viewRect.Width(), viewRect.Height(), SWP_NOZORDER);
            m_pCommunityDlg->ShowWindow(SW_SHOW);
            m_pCommunityDlg->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(viewRect.Width(), viewRect.Height()));
            m_pCommunityDlg->RedrawWindow();
        }
        break;
    case 2: // 메신저
        if (m_pMessengerDlg) {
            m_pMessengerDlg->SetWindowPos(NULL, viewRect.left, viewRect.top, viewRect.Width(), viewRect.Height(), SWP_NOZORDER);
            m_pMessengerDlg->ShowWindow(SW_SHOW);
            m_pMessengerDlg->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(viewRect.Width(), viewRect.Height()));
            m_pMessengerDlg->RedrawWindow();

            // 메신저 뷰가 선택되었을 때만 타이머를 다시 시작합니다.
            m_pMessengerDlg->StartTimer();
        }
        break;
    case 3: // 사원 (FriendsListDlg)
        if (m_pFriendsListDlg) {
            m_pFriendsListDlg->SetWindowPos(NULL, viewRect.left, viewRect.top, viewRect.Width(), viewRect.Height(), SWP_NOZORDER);
            m_pFriendsListDlg->ShowWindow(SW_SHOW);
            m_pFriendsListDlg->SendMessage(WM_SIZE, SIZE_RESTORED, MAKELPARAM(viewRect.Width(), viewRect.Height()));
            m_pFriendsListDlg->RedrawWindow();
        }
        break;
    }
    m_nCurrentView = nViewIndex;
}

void CTheMoonDlg::DrawButton(Gdiplus::Graphics& graphics, const CRect& rect, CString text, bool isSelected)
{
    // 함수 내부 코드는 그대로 둡니다.
    RectF gdiRect(
        static_cast<float>(rect.left),
        static_cast<float>(rect.top),
        static_cast<float>(rect.Width()),
        static_cast<float>(rect.Height())
    );

    Color btnColor = isSelected ? Color(240, 240, 240) : Color(255, 255, 255);
    SolidBrush brush(btnColor);
    graphics.FillRectangle(&brush, gdiRect);

    Pen pen(Color(220, 220, 220));
    graphics.DrawRectangle(&pen, gdiRect.X, gdiRect.Y, gdiRect.Width, gdiRect.Height);

    FontFamily fontFamily(L"맑은 고딕");
    Gdiplus::Font font(&fontFamily, 11, FontStyleBold, UnitPoint);
    SolidBrush textBrush(Color(0, 0, 0));
    StringFormat stringFormat;
    stringFormat.SetAlignment(StringAlignmentCenter);
    stringFormat.SetLineAlignment(StringAlignmentCenter);

    graphics.DrawString(text, -1, &font, gdiRect, &stringFormat, &textBrush);
}

void CTheMoonDlg::RecalcButtonLayout()
{
    CRect clientRect;
    GetClientRect(&clientRect);

    int buttonWidth = 80;
    int exitButtonHeight = 50; // 종료 버튼의 높이를 50px로 고정

    // 1. 종료 버튼의 위치를 먼저 계산 (하단에 배치)
    m_rectExitButton.SetRect(0, clientRect.bottom - exitButtonHeight, buttonWidth, clientRect.bottom);

    // 2. 종료 버튼을 제외한 나머지 공간 계산
    int mainButtonsAreaHeight = clientRect.Height() - exitButtonHeight;

    // 3. 나머지 공간을 4개의 메인 버튼이 나눠 가짐
    int buttonHeight = mainButtonsAreaHeight / 4;

    m_rectHomeButton.SetRect(0, 0, buttonWidth, buttonHeight);
    m_rectCommunityButton.SetRect(0, buttonHeight, buttonWidth, buttonHeight * 2);
    m_rectMessengerButton.SetRect(0, buttonHeight * 2, buttonWidth, buttonHeight * 3);
    m_rectFriendsButton.SetRect(0, buttonHeight * 3, buttonWidth, buttonHeight * 4);
}

void CTheMoonDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);

    // 창 크기가 변경되었을 때만 레이아웃을 다시 계산합니다.
    if (nType != SIZE_MINIMIZED)
    {
        RecalcButtonLayout();

        // 현재 선택된 View의 위치도 새로운 크기에 맞게 업데이트합니다.
        // ShowView 함수를 다시 호출하면 위치가 재설정됩니다.
        if (m_pHomeDlg && m_pHomeDlg->GetSafeHwnd()) // 윈도우 핸들이 유효할 때만 호출
        {
            ShowView(m_nCurrentView);
        }

        // 화면을 갱신하여 변경된 버튼 크기를 그리도록 합니다.
        Invalidate();
    }
}
void CTheMoonDlg::OnBnClickedButton1()
{
    // --- 1. 클라이언트 설정 객체 생성 ---
    http_client_config config;
    // 자체 서명된 인증서의 유효성 검사를 비활성화합니다. (개발용)
    config.set_validate_certificates(false);

    // --- 2. URL을 https로 변경하고, 위에서 만든 config 객체를 전달 ---
    // IP 주소는 실제 Flask 서버의 주소로 입력해야 합니다.
    http_client client(U("https://192.168.0.64:5000"), config);

    // GET 방식으로 /api/test 경로에 요청을 보냅니다.
    pplx::create_task([client]() mutable {
        return client.request(methods::GET, U("/api/test"));
        })
        .then([](http_response response) {
        if (response.status_code() == status_codes::OK)
        {
            return response.extract_json();
        }
        return pplx::task_from_result(json::value());
            })
        .then([](json::value json_response) {
        try
        {
            if (!json_response.is_null() && json_response.is_object())
            {
                std::wstring message = json_response.at(U("message")).as_string();
                AfxMessageBox(CString(message.c_str()));
            }
            else
            {
                AfxMessageBox(_T("서버로부터 잘못된 응답을 받았습니다."));
            }
        }
        catch (const std::exception& e)
        {
            CString error_msg;
            // HTTPS 연결 실패 시 "Error resolving address" 같은 메시지가 나타날 수 있습니다.
            error_msg.Format(_T("응답 처리 중 오류 발생: %s"), CString(e.what()));
            AfxMessageBox(error_msg);
        }
            });
}

afx_msg LRESULT CTheMoonDlg::OnStartChat(WPARAM wParam, LPARAM lParam)
{
    int roomId = static_cast<int>(wParam);
    if (m_pMessengerDlg && roomId != -1)
    {
        ShowView(2); // 메신저 뷰(인덱스 2)로 전환
        m_pMessengerDlg->OpenChatRoom(roomId);

        // 왼쪽 버튼 영역만 새로 그리도록 강제 명령
        CRect rectButtons(0, 0, 80, 500); // 80은 버튼 패널의 너비
        InvalidateRect(&rectButtons);
    }
    return 0;
}