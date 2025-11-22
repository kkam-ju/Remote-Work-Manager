// ModernEdit.cpp
#include "pch.h"
#include "ModernEdit.h"

using namespace Gdiplus;

IMPLEMENT_DYNAMIC(CModernEdit, CEdit)

CModernEdit::CModernEdit()
    : m_bHasFocus(false),
    m_clrBorder(255, 220, 220, 220),       // 일반 테두리: 연한 회색
    m_clrFocusBorder(255, 0, 120, 215)  // 포커스 테두리: 파란색
{
}

CModernEdit::~CModernEdit()
{
}

BEGIN_MESSAGE_MAP(CModernEdit, CEdit)
    ON_WM_NCPAINT()
    ON_WM_SETFOCUS()
    ON_WM_KILLFOCUS()
END_MESSAGE_MAP()


void CModernEdit::OnNcPaint()
{
    // 기본 테두리 그리기를 막기 위해 Default()를 호출하지 않음
    // Default(); 

    CWindowDC dc(this);
    CRect rect;
    GetWindowRect(&rect);
    ScreenToClient(&rect); // 컨트롤 기준 좌표로 변경

    Graphics graphics(dc.GetSafeHdc());

    // 현재 상태에 맞는 테두리 색상 선택
    Color clrCurrent = m_bHasFocus ? m_clrFocusBorder : m_clrBorder;
    Pen borderPen(clrCurrent, 1.5f); // 1.5px 두께의 펜

    // 컨트롤 전체 크기에 맞춰 사각형 테두리를 그립니다.
    graphics.DrawRectangle(&borderPen, 0, 0, rect.Width() - 1, rect.Height() - 1);
}

void CModernEdit::OnSetFocus(CWnd* pOldWnd)
{
    CEdit::OnSetFocus(pOldWnd);
    m_bHasFocus = true;
    // 테두리를 다시 그리도록 Non-client 영역을 갱신
    RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
}

void CModernEdit::OnKillFocus(CWnd* pNewWnd)
{
    CEdit::OnKillFocus(pNewWnd);
    m_bHasFocus = false;
    RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
}