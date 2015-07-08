#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include "afxcmn.h"

// CFilterDialog 对话框

#define  ISP_CID_RGB_FILTER			0x63		//RGB filter, noise reduce
#define  ISP_CID_UV_FILTER			0x64		//uv filter
#define  ISP_CID_DEFECT_PIXEL		0x65		//bad color correct

// noise reduce
typedef struct isp_rgb_filter {
	int type;
	int	enable;					//register table 0x24: offset register 1 [31]
	unsigned int threshold;		//register table 0x24: offset register 1 [21:12]
}RGBFILTER;

//uv iso filter
typedef struct isp_uv_filter {
	int type;
	int	enable;					//register table 0x24: offset register 13 [30]
}UVFILTER;

// defect pixel 
typedef struct isp_defect_pixel {
	int type;
	int	enable;					//register table 0x24: offset register 1 [30]
	unsigned int threshold;		//register table 0x24: offset register 1 [29:28]
}DFPDEFECT;

enum PageInfoIndex_en{
	PAGE_INFO_RGBFILTER = 0,
	PAGE_INFO_UVFILTER ,
	PAGE_INFO_DFPDEFECT,
	PAGE_INFO_MAX
};

class CFilterDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CFilterDialog)

public:
	CFilterDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFilterDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_FILTER };

	int SetRGBFEnable(BOOL bEnable);
	BOOL GetRGBFEnable() {return m_bRGBFEnable;};

	int SetUVFEnable(BOOL bEnable);
	BOOL GetUVFEnable() {return m_bUVFEnable;};

	int SetDFPEnable(BOOL bEnable);
	BOOL GetDFPEnable() {return m_bDFPEnable;};

	virtual int GetPageEnable() {return (m_bRGBFEnable || m_bUVFEnable || m_bDFPEnable);};

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual int Clear();

	int GetPageInfoByIndex(int & nPageID, void * pPageInfoSt, int & nStLen, int iIndex);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	void ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedCheckRgbenable();
	afx_msg void OnBnClickedCheckDfpenable();
	afx_msg void OnBnClickedCheckUvenable();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButtonDfp();
	afx_msg void OnBnClickedButtonRGB();

private:
	RGBFILTER m_RGBFilter;
	UVFILTER m_UVFilter;
	DFPDEFECT m_DFPDefect;

	BOOL m_bRGBFEnable, m_bDFPEnable, m_bUVFEnable;

	CButton m_RGBFCheck, m_DfpCheck, m_UVFCheck;
	CSliderCtrl m_RGBFSlider, m_DfpSlider;
};
