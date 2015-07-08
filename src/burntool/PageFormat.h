#if !defined(AFX_PAGEFORMAT_H__B14CE35B_4720_4CFB_AF5A_563C7046AF8A__INCLUDED_)
#define AFX_PAGEFORMAT_H__B14CE35B_4720_4CFB_AF5A_563C7046AF8A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageFormat.h : header file
//

#include "Config.h"
#include "ListCtrlEx.h"

/////////////////////////////////////////////////////////////////////////////
// CPageFormat dialog

class CPageFormat : public CPropertyPage
{
	DECLARE_DYNCREATE(CPageFormat)

// Construction
public:
	CPageFormat();
	~CPageFormat();

// Dialog Data
	//{{AFX_DATA(CPageFormat)
	enum { IDD = IDD_CFG_FORMAT };
	CListCtrlEx	m_fs_disk_format_list;
    BOOL f_initFlag;
	//}}AFX_DATA

public:
	BOOL get_config_data(CConfig &config);
	BOOL set_config_item(CConfig &config);

	void SetupDisplay();
	void ShowLowFormat();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageFormat)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageFormat)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonLowFormat();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGEFORMAT_H__B14CE35B_4720_4CFB_AF5A_563C7046AF8A__INCLUDED_)
