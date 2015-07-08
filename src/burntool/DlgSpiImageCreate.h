#if !defined(AFX_DLGSPIIMAGECREATE_H__C8D2F5B1_C8BC_402C_95B9_08A1AC72E0B9__INCLUDED_)
#define AFX_DLGSPIIMAGECREATE_H__C8D2F5B1_C8BC_402C_95B9_08A1AC72E0B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgSpiImageCreate.h : header file
//
#include "ListCtrlEx.h"
/////////////////////////////////////////////////////////////////////////////
// DlgSpiImageCreate dialog

class DlgSpiImageCreate : public CDialog
{
// Construction
public:
	DlgSpiImageCreate(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgSpiImageCreate)
	enum { IDD = IDD_DLG_SPI_IMAGE };
	CListCtrlEx	m_spiflash_type_list;
	CComboBox	m_combo_spi;
	CString	m_spi_id;
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgSpiImageCreate)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgSpiImageCreate)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGSPIIMAGECREATE_H__C8D2F5B1_C8BC_402C_95B9_08A1AC72E0B9__INCLUDED_)
