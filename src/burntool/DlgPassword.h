#if !defined(AFX_DLGPASSWORD_H__52F75AB0_549A_4C48_A5F2_AAD1F0E1DF64__INCLUDED_)
#define AFX_DLGPASSWORD_H__52F75AB0_549A_4C48_A5F2_AAD1F0E1DF64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgPassword.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgPassword dialog

class CDlgPassword : public CDialog
{
// Construction
public:
	CDlgPassword(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgPassword)
	enum { IDD = IDD_DLG_PASSWORD };
	CComboBox	m_combo_uid;
	CString	m_password;
	CString	m_user_id;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPassword)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgPassword)
	virtual void OnOK();
	afx_msg void OnBtnPasswdCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPASSWORD_H__52F75AB0_549A_4C48_A5F2_AAD1F0E1DF64__INCLUDED_)
