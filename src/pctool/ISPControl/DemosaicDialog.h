#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include "afxcmn.h"

// CDemosaicDialog 对话框

#define  ISP_CID_DEMOSAIC			0x62		//demosaic

//demosaic
typedef struct isp_demosaic {
	int type;
	int	enable;					//register table 0x20: offset register 2 [12]
	unsigned int threshold;		//register table 0x20: offset register 2 [11:0]
}DEMOSAIC;

class CDemosaicDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CDemosaicDialog)

public:
	CDemosaicDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDemosaicDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_DEMOSAIC };

	int SetDemosaicEnable(BOOL bEnable);
	BOOL GetDemosaicEnable(){return m_bDEnable;};

	virtual int GetPageEnable() {return m_bDEnable;};

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual int Clear();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	void ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedCheckDEnable();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButton1();

private:
	BOOL m_bDEnable;
	DEMOSAIC m_Demosaic;
	CButton m_DCheck;
	CSliderCtrl m_DThreSlider;
};
