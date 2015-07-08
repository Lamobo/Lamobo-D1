#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include "afxcmn.h"

// CBEnhanceDialog 对话框

#define  ISP_CID_BRIGHTNESS_ENHANCE	0x70 //brightness edge enhancement

typedef struct isp_brightness_enhance {
	int type;
	int	enable;				//register table 0x24: offset register 11 [29]
	int	ygain;				//register table 0x24: offset register 12 [31:24]
	unsigned int y_thrs;	//register table 0x24: offset register 12 [22:12]
	unsigned int y_edgek;	//register table 0x24: offset register 12 [10:0]
}BRIENHANCE;


class CBEnhanceDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CBEnhanceDialog)

public:
	CBEnhanceDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CBEnhanceDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_BRI_ENHANCE };

	int SetBEnhanceEnable(BOOL bEnable);
	BOOL GetBEnhanceEnable(){return m_bBEEnable;};

	virtual int GetPageEnable() {return m_bBEEnable;};

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual int Clear();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	void ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedCheckBriEnable();
	afx_msg void OnBnClickedButton1();

private:
	BOOL m_bBEEnable;
	CButton m_BEnhCheck;

	BRIENHANCE m_BriEnhance;

	CSliderCtrl m_ThreSlider;
	CSliderCtrl m_KregSlider;
	CSliderCtrl m_GainSlider;
};
