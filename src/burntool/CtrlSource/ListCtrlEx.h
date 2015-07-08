#if !defined(AFX_LISTCTRLEX_H__A87A9D3A_CD85_4BD5_81E7_40AB380ADB26__INCLUDED_)
#define AFX_LISTCTRLEX_H__A87A9D3A_CD85_4BD5_81E7_40AB380ADB26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListCtrlEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CListCtrlEx window

#define IDC_MENUID 9001
#define IMENU_ITEM_ID 9002

typedef enum 
{
    T_POP_MENU = 0,
    T_POP_CMBBOX,
    T_POP_NULL,
}T_POP_TYPE;

typedef struct{
    T_POP_TYPE SubItemType;
    CString ItemString;
}T_POP_ITEM;

#define T_POP_ITEM_NUM 255

class CListCtrlEx : public CListCtrl
{
// Construction
public:
	CListCtrlEx();

// Attributes
public:

	int m_ItemIndex;
	int m_SubItemIndex;
	CComboBox m_cmbBox;
	int m_cmbBox_nItem;
	int m_cmbBox_nSubItem;
    T_POP_ITEM m_pop_item[T_POP_ITEM_NUM];

// Operations
public:

	int	    HitTestEx (CPoint& Point, int& nSubItem);

	void MoveNextItem();
	virtual int GetSubItemIndex();
	virtual int GetItemIndex();
	CEdit* EditSubItem(int nItem, int nSubItem);
    bool SetSubItemPopItem(int nSubItem, T_POP_TYPE type, CString str);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCtrlEx)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CListCtrlEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(CListCtrlEx)
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	afx_msg void OnKillfocusCmbBox();
	afx_msg void OnSelchangeCmbBox();
	afx_msg void OnMenuSelect(UINT nID);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTCTRLEX_H__A87A9D3A_CD85_4BD5_81E7_40AB380ADB26__INCLUDED_)
