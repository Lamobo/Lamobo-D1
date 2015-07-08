#if !defined(AFX_DLGPASSWORDCHANGE_H__15E24866_5ABC_483F_984A_B735AF3D9633__INCLUDED_)
#define AFX_DLGPASSWORDCHANGE_H__15E24866_5ABC_483F_984A_B735AF3D9633__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgPasswordChange.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgPasswordChange dialog

class CDlgPasswordChange : public CDialog
{
// Construction
public:
	CDlgPasswordChange(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgPasswordChange)
	enum { IDD = IDD_DLG_CHANGE_PASSWORD };
	CComboBox	m_combo_uid;
	CString	m_password_confirm;
	CString	m_password_new;
	CString	m_password_old;
	CString	m_user_id;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgPasswordChange)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgPasswordChange)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPASSWORDCHANGE_H__15E24866_5ABC_483F_984A_B735AF3D9633__INCLUDED_)
