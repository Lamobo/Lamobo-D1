#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "WBDialog.h"

#define  ISP_CID_AUTO_WHITE_BALANCE		0x67	//auto white balance
#define	 AWBNUM		5
// CAWBDialog 对话框
typedef struct isp_auto_white_balance {
	int type;
	int	enable;				//register table 0x20: offset register 25 [31]
	int index;
	unsigned int gr_low;              // 0.8125
	unsigned int gr_high;             // 1.555
	unsigned int gb_low;              // 0.863
	unsigned int gb_high;             // 1.820
	unsigned int grb_low;             // 0.461
	unsigned int grb_high;            // 1.559
	//range of pixel 
	unsigned int r_low;
	unsigned int r_high;
	unsigned int g_low;
	unsigned int g_high;
	unsigned int b_low;
	unsigned int b_high;

	unsigned int co_r;		//register table 0x24: offset register 0 [11:0]
	unsigned int co_g;		//register table 0x24: offset register 0 [23:12]
	unsigned int co_b;		//register table 0x24: offset register 1 [11:0]
}AWBINFO;

class CAWBDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CAWBDialog)

public:
	CAWBDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAWBDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_AWB };

	int SetAWBEnable(BOOL bEnable);
	BOOL GetAWBEnable(){return m_bEnable;};

	int SetAwbParamRespond(BYTE * pAwbParam, unsigned int nLen);

	virtual int GetPageEnable() {return m_bEnable;};

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual int Clear();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedCheckAwbEnable();
	afx_msg void OnNMThemeChangedSliderRhigh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTRBNThumbPosChangingSliderRhigh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdrawSliderRhigh(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();

	int JudgeHighLow(int & nSliderID, int & nPos);
	void ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID);

private:
	CButton m_AWBCheck;
	BOOL	m_bEnable;
	AWBINFO m_stAwbInfo[AWBNUM];
	CSliderCtrl m_SliderRHigh, m_SliderRLow, m_SliderGHigh, m_SliderGLow, m_SliderBHigh, m_SliderBLow,
				m_SliderRGBHigh, m_SliderRGBLow, m_SliderGrHigh, m_SliderGrLow, m_SliderGbHigh, m_SliderGbLow;
	CComboBox m_ComGroupValue;
public:
//	afx_msg void OnCbnEditchangeCombo1();
	afx_msg void OnCbnSelchangeCombo1();
	virtual int GetPageInfoStAll(int & nPageID, void * pPageInfoSt, int & nStlen);
	virtual int SetPageInfoStAll(void * pPageInfoSt, int nStLen);
};
