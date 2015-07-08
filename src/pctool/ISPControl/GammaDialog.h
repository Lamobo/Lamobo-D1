#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include <vector>

using namespace std;

#define  ISP_CID_GAMMA				0x69		//gamma calculate

typedef struct isp_gamma_calculate {
	int type;
	int	enable;					//register table 0x24: offset register 11 [30]
	int	sync;					//count: 0: 1
	unsigned int gamma[32];		//register table 0x28: 64 register
}GAMMACALC;

// CGammaDialog 对话框

class CGammaDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CGammaDialog)

public:
	CGammaDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CGammaDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_GAMMA };

	int SetGammaEnable(BOOL bEnable);
	BOOL GetGammaEnable() {return m_bGammaEnable;};

	virtual int GetPageEnable() {return m_bGammaEnable;};

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	
	int GetPageInfoStIndex(int & nPageID, void * pPageInfoSt, int & nStLen, int nIndex);
	
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual int Clear();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	afx_msg void OnBnClickedCheckGamma();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedCheckGamma2();

	void GetImageLevel();

private:
	CButton m_GammaCheck;
	CButton m_FileLoadCheck;

	CDC m_MemDC;
	CBitmap m_MemBitmap;
	CBitmap* m_pOldMemBitmap;

	vector<CPoint> m_keyPts;
	vector<int>	m_FileLoadPtsY;
	BYTE m_level[256];
	BOOL m_drag;
	int m_moveflag;

	ULONG_PTR m_gdiplusToken;
	HCURSOR	m_handCursor;

	CRect m_CurveRect, m_CurveFrameRect;
	
	BOOL m_bGammaEnable, m_bUseFildLoadCurve;
};
