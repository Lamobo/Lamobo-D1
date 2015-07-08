// ISPControlDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "Aimer39RTSPClient.h"
#include "FfmpegEnvoy.h"
#include "PreviewDlag.h"
#include "VideoRender.h"
#include "ServerSearch.h"

#include "EnableDialog.h"
#include "LensDialog.h"
#include "AWBDialog.h"
#include "BBDialog.h"
#include "BEnhanceDialog.h"
#include "CCorrectDialog.h"
#include "DemosaicDialog.h"
#include "FilterDialog.h"
#include "GammaDialog.h"
#include "SaturationDialog.h"
#include "SpecEffDialog.h"
#include "WBDialog.h"
#include "RegisterDialog.h"
#include "RGBAvgDialog.h"

#define	TAB_PAGE_CNT		14
#define SAVE_PAGE_CNT		12	

// CISPControlDlg 对话框
class CISPControlDlg : public CDialog
{
// 构造
public:
	CISPControlDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CISPControlDlg();

// 对话框数据
	enum { IDD = IDD_ISPCONTROL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	static void __stdcall OnClientFinish(void * pLParam, void * pRParam);
	static void __stdcall OnClientDisConnect(void * pLParam, void * pRParam);
	static void __stdcall OnFullScreenMessage(UINT message,	WPARAM wParam, LPARAM lParam, void * pClassParam);
	static void __stdcall OnIspInfoParamRespond(IServer * pIServer, BYTE * pInfoParam, unsigned int nlen, void * pClassParam);
	static void __stdcall OnServerReturnInfo(IServer * pIServer, RETINFO * pstRetInfo, void * pClassParam);

	// 生成的消息映射函数
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	
	void InitTreeCtrlPosition();
	void InitPreviewWindows(BOOL bNeedCreate = TRUE);
	void InitTabCtrlPosition(BOOL bNeedInsert = TRUE);
	void InitButtonPosition();
	void InitTabPage(BOOL bNeedCreate = TRUE);
	void InitGrayImangeWindowRect();

	void ShowPage(int iSelect);

	void Monitor();
	static unsigned int WINAPI thread_begin( void * pParam );

	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMove(int x, int y);
	afx_msg LRESULT OnPageEnableChanegeMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPageSubmissionMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPageGetInfoMsg(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSearchDevice();
	afx_msg void OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonSubmission();
	afx_msg void OnBnClickedButtonOut();
	afx_msg void OnBnClickedButtonIn();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnClose();
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnImageCalcTypeLog();
	afx_msg void OnImageCalcTypeNormal();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg LRESULT OnCalcRGBAvgMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnServerRetInfo(WPARAM wParam, LPARAM lParam);
	
	void DrawGrayImageWindow();
	void InitGrayBarBitmap();

	void CalcYuvCntFormeDisplayFrame();
	void CalcRGBAvgInRect();

	int RegisterThePreviewServer(IServer * pIServer, const char * strURL);
	int UnregisterThePreviewServer(IServer * pIServer);

	char * MakeRTSPUrl(IServer * pIServer);
	int OpenTheStream(const char * strURL, BOOL bNeedENotify = TRUE);
	int CloseTheStream();
	int ServerDisConnect();

	void SendIspParam(BOOL bNeedJudge = FALSE, int nPageID = -1, int nFlag = -1);

	void FullScreenProcess(BOOL bIsFullScreen);

	void WriteIspFileHeader(CFile & IspFile);
	void WriteEnablePageInfo2IspFile(CFile & IspFile);
	void WriteIspIndexContent(CFile & IspFile, int nIndex, int enable, int nLen, int nFileIndex);

	int ReadIspFileHeader(CFile & IspFile, DWORD & dwTag, DWORD & dwIndexCnt);
	int ReadIspIndex(CFile & IspFile, int nIndex, int & enable, int & nLen, int & nFileIndex);
	int ReadIspContent(CFile & IspFile, int nFileIndex, int nLen, int & nPageID, void * aPageStInfo, int & nInfoLen);

	void WiatForMonitorThreadEnd();

private:
	CAimer39RTSPClient * m_pClient;
	CFfmpegEnvoy * m_videoDecoder;
	CVideoRender * m_videoRender;
	IServer * m_pServerPreview;
	string m_strURL;

	CServerSearch m_Search;
	BOOL m_bIsSearch;

	CTreeCtrl m_TreeCtrl;
	CPreviewDlag m_Preview;
	CTabCtrl m_TabCtrl;
	
	CLensDialog	m_pageLens;
	CEnableDialog m_pageEnable;
	CAWBDialog	m_pageAWB;
	CBBDialog m_pageBB;
	CBEnhanceDialog m_pageBEnhance;
	CCCorrectDialog m_pageCCorrect;
	CDemosaicDialog m_pageDemosaic;
	CFilterDialog m_pageFilter;
	CGammaDialog m_pageGamma;
	CSaturationDialog m_pageSaturation;
	CSpecEffDialog m_pageSpecEff;
	CWBDialog m_pageWB;
	CRegisterDialog m_pageRegister;
	CRGBAvgDialog m_pageRGBAvg;

	HANDLE m_MonitorThread;
	BOOL m_runThreadFlag;
	BOOL m_bNeedJudgeDisConnWork;

	ULONG_PTR m_gdiplusToken;
	CDC m_MemDC;
	CBitmap m_MemBitmap;
	CBitmap* m_pOldMemBitmap;
	CBitmap m_GrayBarBitmap;

	CRect m_GrayImageWindowRect;
	CRect m_GrayImageRect;
	CRect m_GrayImageBarRect;
	BOOL m_bDrawEnable;

	unsigned int m_YCnt[256];
	unsigned int m_YMaxNum;

	CRect m_DrawRect;
	BOOL m_bStartDraw;
	BOOL m_bNeedUpdateRGBAvg;
	BOOL m_bLogImage;
	RETINFO	m_stRetInfo;
	
	CriticalSection m_csForServerConnect, m_csForOpenCloseStream, m_csForRet;
};
