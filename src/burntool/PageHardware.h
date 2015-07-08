#if !defined(AFX_PAGEHARDWARE_H__DF6DED29_7C6B_49F4_8E06_D1B13E5372D7__INCLUDED_)
#define AFX_PAGEHARDWARE_H__DF6DED29_7C6B_49F4_8E06_D1B13E5372D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PageHardware.h : header file
//

#include "Config.h"

/////////////////////////////////////////////////////////////////////////////
// CPageHardware dialog

class CPageHardware : public CPropertyPage
{
	DECLARE_DYNCREATE(CPageHardware)

// Construction
public:
	CPageHardware();
	~CPageHardware();

// Dialog Data
	//{{AFX_DATA(CPageHardware)
	enum { IDD = IDD_CFG_HARDWARE };
	CComboBox	m_chip_type_list;
    CComboBox	m_chip_sel_list;
	UINT	m_feq;
	int		m_radio_burn_mode;
	BOOL 	initFlag;
	//}}AFX_DATA

public:
	BOOL get_config_data(CConfig &config);
	BOOL set_config_item(CConfig &config);

	void select_change_chip_type(CString strChipType);
	void select_boot_chip_type();
	void SetupDisplay();
    void SetBlgAttr(CString planformType);

	int  GetComboBoxIndex(int nIDDlgItem);
    void CPageHardware::burned_box_control(BOOL check);
// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPageHardware)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
		// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPageHardware)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeChipTypeList();
	afx_msg void OnRadioDefine();
	afx_msg void OnRadioLoop();
	afx_msg void OnRadioMcpMode();
	afx_msg void OnRadioSdramMode();
	afx_msg void OnRadioDdrMode();
	afx_msg void OnRadioDdr216Mode();
	afx_msg void OnRadioDdr232Mode();
	afx_msg void OnRadioDdrMode32();
	afx_msg void OnButtonExportRamConfig();
	afx_msg void OnButtonImportRamConfig();
    afx_msg void OnChipSelList();
    afx_msg void OnRadioBurnedNone();
    afx_msg void OnRadioBurnedReset();
    afx_msg void OnRadioBurnedPoweroff();
    afx_msg void OnCheckGpio();
    afx_msg void OnCheckRtcWakup();
    afx_msg void OnRadioBurnPullup();
    afx_msg void OnRadioBurnPulldown();
    afx_msg void OnCheckLedGpio();
	afx_msg void OnRadioDdr16Mobile();
	afx_msg void OnRadioDdr32Mobile();
	afx_msg void OnCheckUsb2();
	afx_msg void OnCheckUdiskUpdate();
	afx_msg void OnCheckUpdate();
	afx_msg void OnRadioSflash();
	afx_msg void OnRadioNandflash();
	afx_msg void OnRadioSd();
	afx_msg void OnRadioDebug();
	afx_msg void OnRadioJtag();
	afx_msg void OnCheckPllfrep();
	afx_msg void OnRadioSpiNandflash();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGEHARDWARE_H__DF6DED29_7C6B_49F4_8E06_D1B13E5372D7__INCLUDED_)
