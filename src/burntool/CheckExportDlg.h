#if !defined(AFX_CHECKEXPORTDLG_H__CE3571A4_7988_4167_8328_4086F4C78FDB__INCLUDED_)
#define AFX_CHECKEXPORTDLG_H__CE3571A4_7988_4167_8328_4086F4C78FDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CheckExportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCheckExportDlg dialog

class CCheckExportDlg : public CDialog
{
// Construction
public:
	CCheckExportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCheckExportDlg)
	enum { IDD = IDD_DLG_CHECK_EXPORT };
	CString	m_str_check_export;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckExportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCheckExportDlg)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnChangeEditCheckString();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHECKEXPORTDLG_H__CE3571A4_7988_4167_8328_4086F4C78FDB__INCLUDED_)
