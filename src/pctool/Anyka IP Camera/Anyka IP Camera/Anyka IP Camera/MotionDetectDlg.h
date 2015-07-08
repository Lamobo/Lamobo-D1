#pragma once
#include "afxwin.h"
#include "ServerSearch.h"
#include "PreviewDialog.h"
#include "InformWaitDialog.h"
#include "Aimer39RTSPClient.h"
#include "FfmpegEnvoy.h"
#include "VideoRender.h"

// CMotionDetectDlg 对话框
class CMotionDetectDlg : public CDialog
{
	DECLARE_DYNAMIC(CMotionDetectDlg)

public:
	CMotionDetectDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMotionDetectDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_MOTION_DETECT };

	int PutServerEntry(string strKey, IServer * pIServer);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();
	BOOL PreTranslateMessage(MSG * pMsg);

	void InitWidgetPosition();
	void InitCombo();
	void ChangePreviewPosition();
	void InitMotionDetectedAreas(IServer * pIServer);
	void DrawTheLine();
	void DrawTheArea();
	void Close();

	int OpenStream(IServer * m_pIServer);
	int CloseTheStream();
	char * MakeRTSPUrl(IServer * pIServer);

	DECLARE_MESSAGE_MAP()

	int GetVideoHW(int & nHeight, int & nWidth, int & nHScale, int & nWScale);

	afx_msg void OnCbnSelchangeDeviceCombo();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	afx_msg void OnCbnSelchangeSensitivityCombo();

private:
	CComboBox m_DevicesCombo;
	CComboBox m_SensitivityCombo;
	CButton m_CheckBox;

	CAimer39RTSPClient * m_pClient;
	CFfmpegEnvoy * m_videoDecoder;
	CVideoRender * m_videoRender;
	
	BOOL m_bNeedWait;
	UINT m_nSensitivity;
	uint64_t m_nMDAreas;
	
	IServer * m_pCurServer;

	CPreviewDialog m_PreviewDlg;

	map<string, IServer*>	m_mapIServer;
	CInformWaitDialog m_waitDialog;
};
