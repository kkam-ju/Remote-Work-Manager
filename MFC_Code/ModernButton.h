#pragma once

#include <gdiplus.h>

class CModernButton : public CButton
{
    DECLARE_DYNAMIC(CModernButton)

public:
    CModernButton();
    virtual ~CModernButton();

protected:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    // 마우스 이벤트를 처리하기 위한 함수
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnMouseLeave();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

    DECLARE_MESSAGE_MAP()

private:
    bool m_bMouseOver; // 마우스가 버튼 위에 있는지 여부
    bool m_bIsPressed; // 버튼이 눌렸는지 여부

    Gdiplus::Color m_clrNormal;
    Gdiplus::Color m_clrHover;
    Gdiplus::Color m_clrPressed;
};