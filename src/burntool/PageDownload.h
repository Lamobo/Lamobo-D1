#if !defined(AFX_PAGEDOWNLOAD_H__1FA6812B_0C06_43A9_AC11_C1A4BEA48894__INCLUDED_)
#define AFX_PAGEDOWNLOAD_H__1FA6812B_0C06_43A9_AC11_C1A4BEA48894__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageDownload.h : header file
//

#include "Config.h"
#include "ListCtrlEx.h"

/////////////////////////////////////////////////////////////////////////////
// CPageDownload dialog

class CPageDownload : public CPropertyPage
{
	DECLARE_DYNCREATE(CPageDownload)

// Construction
public:
	CPageDownload();
	~CPageDownload();

// Dialog Data
	//{{AFX_DATA(CPageDownload)
	enum { IDD = IDD_CFG_DOWNLOAD };
	CListCtrlEx	m_download_mtd_list;
	CListCtrlEx	m_download_nand_list;
	CListCtrlEx	m_download_udisk_list;
	//}}AFX_DATA

public:
	BOOL get_config_data(CConfig &config);
	BOOL set_config_item(CConfig &config);

	void SetupDisplay();
	
// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageDownload)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageDownload)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGEDOWNLOAD_H__1FA6812B_0C06_43A9_AC11_C1A4BEA48894__INCLUDED_)
