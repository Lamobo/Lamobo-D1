#pragma once

#include "afxcmn.h"
#include "PreviewDialog.h"
#include "ServerSearch.h"
#include "Aimer39RTSPClient.h"
#include "FfmpegEnvoy.h"
#include "VideoRender.h"
#include "AudioRender.h"
#include "FtpDownloadThread.h"
#include "Clock.h"
#include "AutoLock.h"
#include "ExactnessSliderCtrl.h"
#include <string>
#include <vector>
#include <list>
using namespace std;


// CRecordPlayDlg 对话框

class CRecordPlayDlg : public CDialog
{
	DECLARE_DYNAMIC(CRecordPlayDlg)
	
	static void __stdcall OnFileListAdd(string * strName, IServer * pIServer, void * pClassParam);
	static void __stdcall OnClientFinish(void * pLParam, void * pRParam);
	static void __stdcall OnStartPlay(double duration, double startDuration, void * pLParam, void * pRParam);
	static void __stdcall OnPlayNPTUpdate(double dNPT, const char * strMediumName, void * pLParam, void * pRParam);

public:
	CRecordPlayDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRecordPlayDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_RECORD_PLAY };

	int PutServerEntry(IServer * pIServer);
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();
	BOOL PreTranslateMessage(MSG * pMsg);

	afx_msg void OnBnClickedButtonGetFiles();
	afx_msg void OnHdnItemdblclickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHdnDividerdblclickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonPlay();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonDownload();
	afx_msg LRESULT OnStartPlayMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNPTUpdateMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPlayFinish(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddFile(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnLvnInsertitemList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg LRESULT OnScrollBegin(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnScrollEnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnThumbTrack(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	char * MakeRTSPUrl(CString & strFile, IServer * pIServer);
	int OpenStream(char * strURL, IServer * pIServer);
	int CloseTheStream();
	void StopPlay();
	
	void Close();

private:
	CAimer39RTSPClient * m_pClient;
	CFfmpegEnvoy * m_videoDecoder;
	CVideoRender * m_videoRender;
	CFfmpegEnvoy * m_AudioDecoder;
	CAudioRender * m_AudioRender;
	
	CClock	m_SyncClock;

	CListCtrl		m_FilesList;
	CPreviewDialog	m_Preview;
	CExactnessSliderCtrl m_DurationSlider;
	IServer *	m_pIServer;
	double	m_dCurrentDuration, m_dStartPlayTime, m_dDuration, m_dLastSendDuration;

	BOOL m_bIsScroll;
	BOOL m_bIsPlay;
	BOOL m_bStartPlayIncome;

	int m_nCurrentSelect;
	CString m_strDownloadDir;
	
	list<string> m_Files;
	vector<CFtpDownloadThread *> m_ftpDownloadThread;
	CriticalSection	m_cs4ListUpdate;
};
