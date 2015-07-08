#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include "afxcmn.h"

#define  ISP_CID_COLOR				0x68		//color correct

// CCCorrectDialog 对话框
typedef struct isp_color_correct {
	int type;
	int	enable;					//register table 0x24: offset register 11 [31]
	unsigned int cc_thrs_low;
	unsigned int cc_thrs_high;
	int ccMtrx[3][3];
}CCORRECT;

class CCCorrectDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CCCorrectDialog)

public:
	CCCorrectDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CCCorrectDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_COLOR_CORRECT };

	int SetCCorrectEnable(BOOL bEnable);
	BOOL GetCCorrectEnable() {return m_bCCEnable;};

	virtual int GetPageEnable() {return m_bCCEnable;};

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
	afx_msg void OnBnClickedCheckCcEnable();
	afx_msg void OnBnClickedButton1();

	int JudgeHighLow(int & nSliderID, int & nPos);

private:
	BOOL m_bCCEnable;
	CCORRECT m_CCorrect;
	CButton m_CCCheck;
	CSliderCtrl m_C11Slider, m_C12Slider, m_C13Slider, m_C21Slider, m_C22Slider,
				m_C23Slider, m_C31Slider, m_C32Slider, m_C33Slider, m_ILowSlider,
				m_IHighSlider;
};
