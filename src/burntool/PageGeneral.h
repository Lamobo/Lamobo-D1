#if !defined(AFX_PAGEGENERAL_H__59DD4080_6AB4_4938_AD83_1E2C4EFE9FFB__INCLUDED_)
#define AFX_PAGEGENERAL_H__59DD4080_6AB4_4938_AD83_1E2C4EFE9FFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageGeneral.h : header file
//

#include "Config.h"

/////////////////////////////////////////////////////////////////////////////
// CPageGeneral dialog

class CPageGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(CPageGeneral)

// Construction
public:
	CPageGeneral();
	~CPageGeneral();

// Dialog Data
	//{{AFX_DATA(CPageGeneral)
	enum { IDD = IDD_CFG_GENERAL };
	CString	m_edit_mac_start_addr_high;
	CString	m_edit_mac_start_addr_low;
	CString	m_edit_mac_end_addr_high;
	CString	m_edit_mac_end_addr_low;
	CString	m_edit_sequence_end_addr_high;
	CString	m_edit_sequence_end_addr_low;
	CString	m_edit_sequence_start_addr_high;
	CString	m_edit_sequence_start_addr_low;
	//}}AFX_DATA

public:
	BOOL check_input();

public:
	BOOL get_config_data(CConfig &config);
	BOOL set_config_item(CConfig &config);
	void set_mac_serial_data();
	void set_nandboot_new_37L();
	BOOL is_multicast_ether_addr(TCHAR *addr);
    BOOL is_zero_ether_addr(TCHAR *addr);

	void check_com(BOOL bCheck); 

	void SetupDisplay();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageGeneral)
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnBrowseProducer();
	afx_msg void OnBtnBrowseNandboot();
	afx_msg void OnCheckOpenCom();
	afx_msg void OnChangeEditProdRunAddr();
	afx_msg void OnChkConfigCmdline();
	afx_msg void OnChangeEditCmdlineAddr();
	afx_msg void OnChangeEditCmdlineData();
	afx_msg void OnCheckMacAddr();
	afx_msg void OnCheckForceWriteMacAddr();
	afx_msg void OnCheckForceWriteSequenceAddr();
	afx_msg void OnCheckSequenceAddr();
	afx_msg void OnChangeEditMacStartAddrLow();
	afx_msg void OnChangeEditMacStartAddrHigh();
	afx_msg void OnChangeEditMacEndAddrHigh();
	afx_msg void OnChangeEditMacEndAddrLow();
	afx_msg void OnChangeEditSequenceStartAddrHigh();
	afx_msg void OnChangeEditSequenceStartAddrLow();
	afx_msg void OnChangeEditSequenceEndAddrHigh();
	afx_msg void OnChangeEditSequenceEndAddrLow();
	afx_msg void OnChangeEditProjectName();
	afx_msg void OnSelchangeComboPalnformType();
	afx_msg void OnSequenceAddrReset();
	afx_msg void OnMacaddrReset();
	afx_msg void OnChangeEditPid();
	afx_msg void OnChangeEditVid();
	afx_msg void OnBtnBrowseNandbootNew();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void limit_addr_input(UINT nID);
	void limit_mac_addr_input(UINT nID, UINT len,TCHAR *pBuf);
	void limit_serial_addr_input(UINT nID, UINT len, TCHAR *pBuf);
	void limit_serial_addr_input_low(UINT nID, UINT len);
	void limit_pid_vid_input(UINT nID, UINT len);
	BOOL Mac_iserror(UINT nID, UINT len, CString str, TCHAR *pBuf);
	BOOL Serial_iserror(UINT nID, UINT len, CString str, TCHAR *pBuf);
	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGEGENERAL_H__59DD4080_6AB4_4938_AD83_1E2C4EFE9FFB__INCLUDED_)
