#if !defined(AFX_PAGESPINANDFLASH_H__3F69F5A9_6C5C_4C1E_A517_C4870338AF56__INCLUDED_)
#define AFX_PAGESPINANDFLASH_H__3F69F5A9_6C5C_4C1E_A517_C4870338AF56__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageSpiNandflash.h : header file
//

#include "Config.h"
#include "ListCtrlEx.h"
/////////////////////////////////////////////////////////////////////////////
// CPageSpiNandflash dialog

class CPageSpiNandflash : public CPropertyPage
{
	DECLARE_DYNCREATE(CPageSpiNandflash)

// Construction
public:
	CPageSpiNandflash();
	~CPageSpiNandflash();

// Dialog Data
	//{{AFX_DATA(CPageSpiNandflash)
	enum { IDD = IDD_CFG_SPINANDFLASH };
	CListCtrlEx	m_spi_nandflash_param_list;
	//}}AFX_DATA


public:
	BOOL get_config_data(CConfig &config);
	BOOL set_config_item(CConfig &config);
	
	void SetupDisplay();


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageSpiNandflash)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageSpiNandflash)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGESPINANDFLASH_H__3F69F5A9_6C5C_4C1E_A517_C4870338AF56__INCLUDED_)
