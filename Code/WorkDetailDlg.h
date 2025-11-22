#pragma once
#include <vector>
#include <afxdb.h>



struct AttachmentInfo
{
    int id;
    CString filename;
    CString filepath;
};

class CWorkDetailDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CWorkDetailDlg)

public:
    CWorkDetailDlg(int requestId, int currentUserId, bool isAdmin, CWnd* pParent = nullptr);
    virtual ~CWorkDetailDlg();

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_WORK_DETAIL_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    // 멤버 변수
    int m_nRequestId;
    int m_nCurrentUserId;
    bool m_bIsAdmin;
    CDatabase m_db;
    std::vector<AttachmentInfo> m_attachments;

    // 컨트롤 변수
    CStatic m_staticTitle;
    CStatic m_staticRequester;
    CStatic m_staticDueDate;
    CEdit m_editContent;
    CListBox m_listAttachments;
    CComboBox m_comboStatus;
    CBrush m_whiteBrush;
    // 헬퍼 함수
    BOOL ConnectDatabase();
    void LoadWorkDetails();

    // 메시지 핸들러
    afx_msg void OnBnClickedButtonUpdateStatus();
    afx_msg void OnLbnDblclkListAttachments(); // 리스트 박스 더블클릭 이벤트
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    DECLARE_MESSAGE_MAP()
};