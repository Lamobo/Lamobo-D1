#pragma once
#include "afxwin.h"
#include "PreviewDialog.h"
#include "ServerSearch.h"
#include "InformWaitDialog.h"
#include "Aimer39RTSPClient.h"
#include "FfmpegEnvoy.h"
#include "VideoRender.h"
#include "AudioRender.h"
#include <map>
#include <string>
using namespace std;

// CPrivacyDialog 对话框

typedef pair<map<string, IServer*>::iterator, BOOL>		InsertRet;

class CPrivacyDialog : public CDialog
{
	DECLARE_DYNAMIC(CPrivacyDialog)

public:
	CPrivacyDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPrivacyDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_PRIVACY };

	int PutServerEntry(string strKey, IServer * pIServer);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	BOOL PreTranslateMessage(MSG * pMsg);

	DECLARE_MESSAGE_MAP()
	
	int GetVideoHW(int & nHeight, int & nWidth, int & nHScale, int & nWScale);

	void ChangePreviewPosition();
	void InitWidgetPosition();
	void InitCombo();
	void InitPreviewWindows();

	void InitPrivacyArea(IServer * pIServer);

	int OpenStream(IServer * pIServer);
	char * MakeRTSPUrl(IServer * pIServer);
	int CloseTheStream();
	void Close();

	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnCbnSelchangeCombo2();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCheck1();
	afx_msg void OnBnClickedButton2();

private:
	int	m_nCurSelectAreaNum;
	IServer * m_pCurServer;

	CPreviewDialog	m_PreviweDialog;
	CButton m_Check;
	CComboBox m_ServerCombox;
	CComboBox m_PrivacyCombo;

	CAimer39RTSPClient * m_pClient;
	CFfmpegEnvoy * m_videoDecoder;
	CVideoRender * m_videoRender;
	
	CRect m_DrawRect[4];
	int m_nIsUse[4];

	CRect m_OriDrawRect[4];
	int m_nOriIsUse[4];

	BOOL m_bStartDraw;
	BOOL m_bNeedWait;
	CInformWaitDialog m_waitDialog;
	map<string, IServer*>	m_mapIServer;
};
