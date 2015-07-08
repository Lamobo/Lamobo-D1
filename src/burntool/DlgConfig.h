#if !defined(AFX_DLGCONFIG_H__2728CB38_85BA_4422_9B20_1216E4DD6594__INCLUDED_)
#define AFX_DLGCONFIG_H__2728CB38_85BA_4422_9B20_1216E4DD6594__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgConfig.h : header file
//

#include "TreePropSheet.h"
#include "PageGeneral.h"
#include "PageDownload.h"
#include "PageHardware.h"
#include "PageFormat.h"
#include "PageNandflash.h"
#include "PageSpiNandflash.h"
#include "PageSpiflash.h"
#include "PageBaseband.h"

#include "MainFrm.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgConfig dialog

class CDlgConfig : public CDialog
{
// Construction
public:
	CDlgConfig(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgConfig)
	enum { IDD = IDD_CONFIG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgConfig)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL check_input();

public:
	CPageGeneral	m_page_general;  
	CPageFormat		m_page_format;
	CPageHardware	m_page_hardware;
protected:
	CPageDownload	m_page_download;
	CPageNandflash	m_page_nandflash;
	CPageSpiNandflash	m_page_spinandflash;
	CPageSpiflash	m_page_spiflash;
	CPageBaseband   m_page_baseband;

	CTreePropSheet	m_sheet;

	// Generated message map functions
	//{{AFX_MSG(CDlgConfig)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCONFIG_H__2728CB38_85BA_4422_9B20_1216E4DD6594__INCLUDED_)
