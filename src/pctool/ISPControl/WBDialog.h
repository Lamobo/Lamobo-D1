#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include "afxcmn.h"

// CWBDialog 对话框

#define  ISP_CID_WHITE_BALANCE		0x66		//white balance

typedef struct isp_white_balance {
	int type;
	int	enable;
	unsigned int co_r;		//register table 0x24: offset register 0 [11:0]
	unsigned int co_g;		//register table 0x24: offset register 0 [23:12]
	unsigned int co_b;		//register table 0x24: offset register 1 [11:0]
}WBALANCE;

class CWBDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CWBDialog)

public:
	CWBDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWBDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_WB };

	int SetWBEnable(BOOL bEnable);
	BOOL GetWBEnable(){return m_bWBEnable;};

	virtual int GetPageEnable() {return m_bWBEnable;};

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual int Clear();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedCheckWb();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButton1();

	void ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID);

private:
	BOOL m_bWBEnable;

	WBALANCE m_WBalance;
	CButton m_WBCheck;

	CSliderCtrl m_WBRSlider, m_WBGSlider, m_WBBSlider;
};
