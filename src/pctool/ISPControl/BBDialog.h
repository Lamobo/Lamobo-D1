#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include "afxcmn.h"

// CBBDialog 对话框

#define  ISP_CID_BLACK_BALANCE		0x60 //black balance

/* response pc tool control command structure define */
typedef struct isp_black_balance {
	int type;
	int	enable;
	unsigned int r_offset;		//register table 0x20: offset register 13 [31:22]
	unsigned int g_offset;		//register table 0x20: offset register 14 [31:22]
	unsigned int b_offset;		//register table 0x20: offset register 15 [31:22]
}BLACKBALANCE;

class CBBDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CBBDialog)

public:
	CBBDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CBBDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_BB };

	int SetBBEnable(BOOL bEnable);
	BOOL GetBBEnable(){return m_bBBEnable;};

	virtual int GetPageEnable() {return m_bBBEnable;};

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual int Clear();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	
	void ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID);

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedCheckBbEnable();
	afx_msg void OnBnClickedButton1();

private:
	BOOL			m_bBBEnable;
	BLACKBALANCE	m_BackBalance;
	CButton			m_EnableCheck;
	CSliderCtrl m_ROffsetSlider, m_GOffsetSlider, m_BOffsetSlider;
};
