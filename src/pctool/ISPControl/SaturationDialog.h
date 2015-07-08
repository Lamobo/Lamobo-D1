#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include "afxcmn.h"

// CSaturationDialog 对话框

#define  ISP_CID_SATURATION			0x72		//saturation

typedef struct isp_saturation {
	int type;
	int	enable;				//register table 0x24: offset register 13 [31]
	int	Khigh;				//register table 0x24: offset register 13 [29:20], (0.1~3) *256 _8_8 
	int	Klow;				//register table 0x24: offset register 13 [19:10], register value  (0.1~3)  
	int	Kslope;				//register table 0x24: offset register 13 [9:0], (ih-il)*256/(ch-cl)
	unsigned int Chigh;		//register table 0x24: offset register 14 [31:24],0~255
	unsigned int Clow;		//register table 0x24: offset register 14 [23:16], 0~255  
}SATURATION;

class CSaturationDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CSaturationDialog)

public:
	CSaturationDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSaturationDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_SATURATION };

	int SetSaturationEnable(BOOL bEnable);
	BOOL GetSaturationEnable(){return m_bSatEnable;};

	virtual int GetPageEnable() {return m_bSatEnable;};

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual int Clear();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	
	afx_msg void OnBnClickedCheckSEnable();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButton1();

	void ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID);
	int JudgeHighLow(int & nSliderID, int & nPos);

private:
	BOOL m_bSatEnable;
	
	SATURATION m_Saturation;
	CButton m_SaturationCheck;
	CSliderCtrl m_KlowSlider, m_KhighSlider, m_ChighSlider, m_ClowSlider;
};
