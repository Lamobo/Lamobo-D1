#pragma once

#include "BasePage.h"
#include "afxwin.h"

typedef struct Enable_st
{
	BOOL bBBEnable;
	BOOL bLensEnable;
	BOOL bDemosaicEnable;
	BOOL bRGBFilterEnable;
	BOOL bUVFilterEnable;
	BOOL bDefectPixelEnable;
	BOOL bWBEnable;
	BOOL bAWBEnable;
	BOOL bCCorrectEnable;
	BOOL bGammaEnable;
	BOOL bBEnhanceEnable;
	BOOL bSaturationEnable;
	BOOL bSpecEffEnable;
}Enable;


// CEnableDialog 对话框

class CEnableDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CEnableDialog)

public:
	CEnableDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CEnableDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_Enable };

	int SetEnable(int nFlag, BOOL bEnable);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);

	DECLARE_MESSAGE_MAP()

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int Clear();

	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedCheckLensAdjust();
	afx_msg void OnBnClickedCheckDemosaic();
	afx_msg void OnBnClickedCheckRgbFilter();
	afx_msg void OnBnClickedCheckWb();
	afx_msg void OnBnClickedCheckAwb();
	afx_msg void OnBnClickedCheckCcorrect();
	afx_msg void OnBnClickedCheckGamma();
	afx_msg void OnBnClickedCheckUvFilter();
	afx_msg void OnBnClickedCheckDefectPixel();
	afx_msg void OnBnClickedCheckBenhance();
	afx_msg void OnBnClickedCheckSaturation();
	afx_msg void OnBnClickedCheckSpecial();
	afx_msg void OnBnClickedCheckBb();

public:
	Enable m_stEnable;

private:
	CButton m_BBCheck;
	CButton m_LensCheck;
	CButton m_DemosaicCheck;
	CButton m_RGBFCheck;
	CButton m_UVFCheck;
	CButton m_DPixelCheck;
	CButton m_WBCheck;
	CButton m_AWBCheck;
	CButton m_CCorrectCheck;
	CButton m_GammaCheck;
	CButton m_BEnhanceCheck;
	CButton m_SaturCheck;
	CButton m_SpecEffCheck;
};
