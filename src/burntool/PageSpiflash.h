#if !defined(AFX_PAGESPIFLASH_H__F131652A_1138_4020_BCC0_3A2C5562DAD4__INCLUDED_)
#define AFX_PAGESPIFLASH_H__F131652A_1138_4020_BCC0_3A2C5562DAD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageSpiflash.h : header file
//

#include "Config.h"
#include "ListCtrlEx.h"

/////////////////////////////////////////////////////////////////////////////
// CPageSpiflash dialog

class CPageSpiflash : public CPropertyPage
{
	DECLARE_DYNCREATE(CPageSpiflash)

// Construction
public:
	CPageSpiflash();
	~CPageSpiflash();

// Dialog Data
	//{{AFX_DATA(CPageSpiflash)
	enum { IDD = IDD_CFG_SPIFLASH };
	CListCtrlEx	m_spiflash_param_list;
	//}}AFX_DATA

public:
	BOOL get_config_data(CConfig &config);
	BOOL set_config_item(CConfig &config);

	void SetupDisplay();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageSpiflash)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageSpiflash)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGESPIFLASH_H__F131652A_1138_4020_BCC0_3A2C5562DAD4__INCLUDED_)
