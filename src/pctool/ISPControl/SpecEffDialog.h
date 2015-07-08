#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include "afxcmn.h"

// CSpecEffDialog 对话框
#define  ISP_CID_SPECIAL_EFFECT		0x74		//special effect

typedef struct isp_special_effect {
	int type;
	int	enable;					//register table 0x24: offset register 11 [28]
	int	solar_enable;			//register table 0x24: offset register 11 [27]
	unsigned int solar_thrs;	//register table 0x24: offset register 0 [31:24]
	int          y_eff_coefa;	//register table 0x24: offset register 14 [15:8]
    unsigned int y_eff_coefb;	//register table 0x24: offset register 14 [7:0]
	int          u_eff_coefa;  	//register table 0x24: offset register 15 [31:24]
    unsigned int u_eff_coefb;	//register table 0x24: offset register 15 [23:16]
	int          v_eff_coefa;	//register table 0x24: offset register 15 [15:8]
    unsigned int v_eff_coefb;	//register table 0x24: offset register 15 [7:0]
}SPECIALEFF;


class CSpecEffDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CSpecEffDialog)

public:
	CSpecEffDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSpecEffDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_SPECIAL_EFFECT };

	int SetSpecEffEnable(BOOL bEnable);
	BOOL GetSolarEnable(){return m_bSolarEnable;};
	BOOL GetYUVEffEnable(){return m_bYUVEffEnable;};

	virtual int GetPageEnable() {return (m_bYUVEffEnable || m_bSolarEnable);};

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual int Clear();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCheckYuveEnable();
	afx_msg void OnBnClickedCheckSolarEnable();
	afx_msg void OnBnClickedButton1();

	void ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID);

private:
	BOOL m_bYUVEffEnable, m_bSolarEnable;
	
	SPECIALEFF m_SpecEff;

	CButton m_EffectEnable, m_SolarEnable;

	CSliderCtrl m_SolarThreSlider, m_YEffCoefaSlider, m_YEffCoefbSlider,
				m_UEffCoefaSlider, m_UEffCoefbSlider, m_VEffCoefaSlider,
				m_VEffCoefbSlider;
};
