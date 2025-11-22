// ModernEdit.h
#pragma once

#include <gdiplus.h>

class CModernEdit : public CEdit
{
    DECLARE_DYNAMIC(CModernEdit)

public:
    CModernEdit();
    virtual ~CModernEdit();

protected:
    // 컨트롤의 테두리(Non-client 영역)를 직접 그리는 함수
    afx_msg void OnNcPaint();
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    afx_msg void OnKillFocus(CWnd* pNewWnd);

    DECLARE_MESSAGE_MAP()

private:
    bool m_bHasFocus; // 포커스를 가지고 있는지 여부
    Gdiplus::Color m_clrBorder;
    Gdiplus::Color m_clrFocusBorder;
};