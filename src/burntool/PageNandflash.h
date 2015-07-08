#if !defined(AFX_PAGENANDFLASH_H__F131652A_1138_4020_BCC0_3A2C5562DAD4__INCLUDED_)
#define AFX_PAGENANDFLASH_H__F131652A_1138_4020_BCC0_3A2C5562DAD4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageNandflash.h : header file
//

#include "Config.h"
#include "ListCtrlEx.h"

/////////////////////////////////////////////////////////////////////////////
// CPageNandflash dialog

class CPageNandflash : public CPropertyPage
{
	DECLARE_DYNCREATE(CPageNandflash)

// Construction
public:
	CPageNandflash();
	~CPageNandflash();

// Dialog Data
	//{{AFX_DATA(CPageNandflash)
	enum { IDD = IDD_CFG_NANDFLASH };
	CListCtrlEx	m_nandflash_param_list;
	//}}AFX_DATA

public:
	BOOL get_config_data(CConfig &config);
	BOOL set_config_item(CConfig &config);

	void SetupDisplay();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageNandflash)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageNandflash)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGENANDFLASH_H__F131652A_1138_4020_BCC0_3A2C5562DAD4__INCLUDED_)
