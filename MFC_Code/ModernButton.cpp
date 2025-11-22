// ModernButton.cpp
#include "pch.h"
#include "ModernButton.h"

using namespace Gdiplus;

IMPLEMENT_DYNAMIC(CModernButton, CButton)

CModernButton::CModernButton()
    : m_bMouseOver(false), m_bIsPressed(false),
    m_clrNormal(255, 0, 120, 215),   // 일반 상태: 파란색
    m_clrHover(255, 0, 140, 255),    // 마우스 오버: 밝은 파란색
    m_clrPressed(255, 0, 100, 195)   // 눌렸을 때: 어두운 파란색
{
}

CModernButton::~CModernButton()
{
}

BEGIN_MESSAGE_MAP(CModernButton, CButton)
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CModernButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    CRect rect = lpDrawItemStruct->rcItem;

    Graphics graphics(pDC->GetSafeHdc());
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    // 현재 버튼 상태에 맞는 색상 선택
    Color clrBackground = m_clrNormal;
    if (m_bIsPressed) {
        clrBackground = m_clrPressed;
    }
    else if (m_bMouseOver) {
        clrBackground = m_clrHover;
    }

    // 둥근 사각형 그리기
    SolidBrush brush(clrBackground);
    graphics.FillRectangle(&brush, 0, 0, rect.Width(), rect.Height());

    // 버튼 텍스트 가져오기
    CString strText;
    GetWindowText(strText);

    // 텍스트 그리기
    Gdiplus::Font font(L"맑은 고딕", 10, FontStyleBold, UnitPoint);
    SolidBrush textBrush(Color(255, 255, 255, 255)); // 흰색 텍스트
    StringFormat strFormat;
    strFormat.SetAlignment(StringAlignmentCenter);
    strFormat.SetLineAlignment(StringAlignmentCenter);

    RectF textRect(0, 0, (float)rect.Width(), (float)rect.Height());
    graphics.DrawString(strText, -1, &font, textRect, &strFormat, &textBrush);
}

void CModernButton::OnMouseMove(UINT nFlags, CPoint point)
{
    if (!m_bMouseOver)
    {
        m_bMouseOver = true;
        // 마우스가 들어왔음을 알리기 위해 TrackMouseEvent 사용
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hWnd;
        TrackMouseEvent(&tme);
        Invalidate();
    }
    CButton::OnMouseMove(nFlags, point);
}

void CModernButton::OnMouseLeave()
{
    m_bMouseOver = false;
    Invalidate();
    CButton::OnMouseLeave();
}

void CModernButton::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_bIsPressed = true;
    Invalidate();
    CButton::OnLButtonDown(nFlags, point);
}

void CModernButton::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_bIsPressed = false;
    Invalidate();
    CButton::OnLButtonUp(nFlags, point);
}