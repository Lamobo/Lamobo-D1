#if !defined(AFX_LISTCELLEDIT_H__485EB068_E4D0_40BA_914C_24DBD9CE0887__INCLUDED_)
#define AFX_LISTCELLEDIT_H__485EB068_E4D0_40BA_914C_24DBD9CE0887__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListCellEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CListCellEdit window

class CListCellEdit : public CEdit
{
// Construction
public:
	CListCellEdit();

	CListCellEdit(int nItem, int nSubItem, CString strInitTex);
	void SetListItemText();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListCellEdit)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CListCellEdit();

protected:
	int			m_nItem;
    int			m_nSubItem;
    CString		m_strInitText;
    BOOL		m_bEscape;
	int			m_left;
	int			m_width;

	// Generated message map functions
protected:
	//{{AFX_MSG(CListCellEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChange();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTCELLEDIT_H__485EB068_E4D0_40BA_914C_24DBD9CE0887__INCLUDED_)
