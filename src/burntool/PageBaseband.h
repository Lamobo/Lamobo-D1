#if !defined(AFX_PAGEBASEBAND_H__598D0D91_0163_494F_BBE7_70828B40A34A__INCLUDED_)
#define AFX_PAGEBASEBAND_H__598D0D91_0163_494F_BBE7_70828B40A34A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageBaseband.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPageBaseband dialog

class CPageBaseband : public CPropertyPage
{
	DECLARE_DYNCREATE(CPageBaseband)

// Construction
public:
	CPageBaseband();
	~CPageBaseband();

// Dialog Data
	//{{AFX_DATA(CPageBaseband)
	enum { IDD = IDD_CFG_BASEBAND };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

public:
	BOOL check_input();
	BOOL get_config_data(CConfig &config);
	
	void check_download();
	BOOL set_config_item(CConfig &config);
	
	void SetupDisplay();
// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageBaseband)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageBaseband)
	virtual BOOL OnInitDialog();
	afx_msg void OnCheckDownloadFls();
	afx_msg void OnCheckDownloadEep();
	afx_msg void OnBtnBrowseFls();
	afx_msg void OnBtnBrowseEep();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGEBASEBAND_H__598D0D91_0163_494F_BBE7_70828B40A34A__INCLUDED_)
