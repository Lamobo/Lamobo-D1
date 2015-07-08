#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include "AutoLock.h"

// CRGBAvgDialog 对话框

class CRGBAvgDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CRGBAvgDialog)

public:
	CRGBAvgDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRGBAvgDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_SHOW_RGB_AVG };

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);
	int SetWH(int nWidth, int nHight);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	//virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	afx_msg LRESULT OnUpdateRGB(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()

	void CalcRGBAvgAndPrepareBitmap();

private:
	BYTE *	m_pRGBData;
	int m_nHeight, m_nWidth;
	int		m_nDataLen;

	CDC m_MemDC;
	CBitmap m_PicBitmap;
	CBitmap* m_pOldMemBitmap;

	CriticalSection m_cs4RGBData;
};
