// RecordPlayDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Anyka IP Camera.h"
#include "RecordPlayDlg.h"
#include <shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

#define RTSP_PREFIX				"rtsp://"
#define SEPARATOR				"/"
#define PORT_PREFIX				":"

#define WM_START_PLAY			WM_USER + 400
#define WM_NPT_UPDATE			WM_USER + 401
#define WM_PLAY_FINISH			WM_USER + 402
#define WM_ADD_FILE				WM_USER + 403

#define MAX_DURATION			31536000.0		//one year

#define MAX_DOWNLOAD_THREAD		5
#define FTP_SERVER_PORT			21
#define DOWNLOAD_FILE_DIR		_T("D:\\AnykaIPC")
#define DOWNLOAD_FILE_DIR2		_T("AnykaIPC")

// CRecordPlayDlg 对话框

typedef struct StartPlay_st
{
	double duration;
	double startDuration;
	CAimer39RTSPClient * pClient;
}STARTPLAY_PARAM;

typedef struct PlayNPTUpdate_St
{
	double dNPT;
	string strMediumName;
	CAimer39RTSPClient * pClient;
}NPT_UPDATE;

IMPLEMENT_DYNAMIC(CRecordPlayDlg, CDialog)

void CRecordPlayDlg::OnFileListAdd(string * strName, IServer * pIServer, void * pClassParam)
{
	CRecordPlayDlg * pthis = (CRecordPlayDlg *)pClassParam;
	
	{
		CAutoLock lock(&(pthis->m_cs4ListUpdate));
		pthis->m_Files.push_back(*strName);
	}
	
	pthis->PostMessage(WM_ADD_FILE);
}

void CRecordPlayDlg::OnStartPlay(double duration, double startDuration, void * pLParam, void * pRParam)
{
	CRecordPlayDlg * pthis = (CRecordPlayDlg *)pLParam;

	STARTPLAY_PARAM * pParam = new STARTPLAY_PARAM;
	pParam->duration = duration;
	pParam->startDuration = startDuration;
	pParam->pClient = (CAimer39RTSPClient *)pRParam;
	
	pthis->SendMessage(WM_START_PLAY, 0, LPARAM(pParam));
	
	delete pParam;
}

void CRecordPlayDlg::OnPlayNPTUpdate(double dNPT, const char * strMediumName, void * pLParam, void * pRParam)
{
	CRecordPlayDlg * pthis = (CRecordPlayDlg *)pLParam;

	//now we just process video is npt
	if ((strcmp(strMediumName, "VIDEO") != 0) && (strcmp(strMediumName, "video") != 0)) return;

	NPT_UPDATE * pParam = new NPT_UPDATE;
	pParam->dNPT = dNPT;
	pParam->strMediumName = strMediumName;
	pParam->pClient = (CAimer39RTSPClient *)pRParam;

	if (pParam->pClient != pthis->m_pClient){
		return;
	}

	pthis->m_dCurrentDuration = dNPT;

	if ((pthis->m_dDuration - pthis->m_dCurrentDuration <= 1.0) || 
		((uint64_t)dNPT - (uint64_t)(pthis->m_dLastSendDuration) >= 1.0)) {
		pthis->SendMessage(WM_NPT_UPDATE, 0, LPARAM(pParam));
		pthis->m_dLastSendDuration = dNPT;
	}

	delete pParam;
}

void CRecordPlayDlg::OnClientFinish(void * pLParam, void * pRParam)
{
	CRecordPlayDlg * pthis = (CRecordPlayDlg *)pLParam;
	CAimer39RTSPClient * pClient = (CAimer39RTSPClient *)pRParam;

	uint64_t nID = pClient->GetID();
	unsigned int nLow = nID;
	unsigned int nHight = nID >> 32;
	
	if (pthis->m_bIsPlay) //pause 状态不处理Finish消息。
		pthis->PostMessage(WM_PLAY_FINISH, (WPARAM)nLow, (LPARAM)nHight);
}

CRecordPlayDlg::CRecordPlayDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRecordPlayDlg::IDD, pParent)
{
	m_pClient = NULL;
	m_videoDecoder = NULL;
	m_videoRender = NULL;
	m_pIServer = NULL;
	m_AudioDecoder = NULL;
	m_AudioRender = NULL;

	m_bIsPlay = FALSE;
	m_bIsScroll = FALSE;
	m_dCurrentDuration = 0.0;
	m_dStartPlayTime = 0.0;
	m_dDuration = 0.0;
	m_nCurrentSelect = -1;
	m_dLastSendDuration = 0.0;
}

CRecordPlayDlg::~CRecordPlayDlg()
{
}

void CRecordPlayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_FilesList);
	DDX_Control(pDX, IDC_SLIDER1, m_DurationSlider);
}


BEGIN_MESSAGE_MAP(CRecordPlayDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GET_FILES, &CRecordPlayDlg::OnBnClickedButtonGetFiles)
	ON_NOTIFY(HDN_ITEMDBLCLICK, 0, &CRecordPlayDlg::OnHdnItemdblclickList1)
	ON_NOTIFY(HDN_DIVIDERDBLCLICK, 0, &CRecordPlayDlg::OnHdnDividerdblclickList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CRecordPlayDlg::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON_PLAY, &CRecordPlayDlg::OnBnClickedButtonPlay)
	ON_MESSAGE(WM_START_PLAY, &CRecordPlayDlg::OnStartPlayMsg)
	ON_MESSAGE(WM_NPT_UPDATE, &CRecordPlayDlg::OnNPTUpdateMsg)
	ON_MESSAGE(WM_PLAY_FINISH, &CRecordPlayDlg::OnPlayFinish)
	ON_MESSAGE(WM_ADD_FILE, &CRecordPlayDlg::OnAddFile)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CRecordPlayDlg::OnBnClickedButtonStop)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CRecordPlayDlg::OnNMClickList1)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CRecordPlayDlg::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD, &CRecordPlayDlg::OnBnClickedButtonDownload)
	ON_WM_TIMER()
	ON_NOTIFY(LVN_INSERTITEM, IDC_LIST1, &CRecordPlayDlg::OnLvnInsertitemList1)
	ON_MESSAGE(WM_SCROLL_BEGIN, &CRecordPlayDlg::OnScrollBegin)
	ON_MESSAGE(WM_SCROLL_END, &CRecordPlayDlg::OnScrollEnd)
	ON_MESSAGE(WM_THUMB_TRACK, &CRecordPlayDlg::OnThumbTrack)
END_MESSAGE_MAP()


// CRecordPlayDlg 消息处理程序

BOOL CRecordPlayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_DurationSlider.SetRange(0, 0);

	m_FilesList.InsertColumn(0, L"录像文件");
	m_FilesList.InsertColumn(1, L"下载进度");
	m_FilesList.SetColumnWidth(0, 120);
	m_FilesList.SetColumnWidth(1, 50);
	m_FilesList.SetExtendedStyle(LVS_EX_GRIDLINES);

	CRect rect, listRect;
	GetClientRect(rect);
	
	int nPreviewX = 0, nPreviewY = 0, nPreviewHeight = 0, nPreviewWidth = 0;
	
	m_FilesList.GetWindowRect(listRect);
	ScreenToClient(listRect);

	m_Preview.Create(IDD_DIALOG_PREVIEW1, this);
	
	nPreviewX = rect.left + 10;
	nPreviewY = rect.top + 10;
	
	nPreviewHeight = rect.Height() - (rect.bottom - listRect.bottom) - 20;
	nPreviewWidth = rect.Width() - (rect.right - listRect.left) - 20;

	m_Preview.MoveWindow(nPreviewX, nPreviewY, nPreviewWidth, nPreviewHeight);

	m_Preview.ShowWindow(SW_SHOW);

	SetTimer(1, 500, NULL);
	
	m_strDownloadDir = DOWNLOAD_FILE_DIR;
	if (!PathFileExists(DOWNLOAD_FILE_DIR)) {
		if (!CreateDirectory(DOWNLOAD_FILE_DIR, NULL)) {
			if (!CreateDirectory(DOWNLOAD_FILE_DIR2, NULL)) {
				m_strDownloadDir = _T("");
			}else {
				m_strDownloadDir = DOWNLOAD_FILE_DIR2;
			}
		}
	}

	return TRUE;
}

BOOL CRecordPlayDlg::PreTranslateMessage(MSG * pMsg)
{
	// TODO: 在此添加控件通知处理程序代码
	if (pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam) 
		{
		case VK_RETURN:
			return TRUE;
		case VK_ESCAPE:
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

int CRecordPlayDlg::PutServerEntry(IServer * pIServer)
{
	m_pIServer = pIServer;
	return 0;
}

void CRecordPlayDlg::OnBnClickedButtonGetFiles()
{
	// TODO: 在此添加控件通知处理程序代码
	USES_CONVERSION;
	
	m_pIServer->DisConnect();
	m_pIServer->SetFileListAddCallBack(NULL, NULL);

	if (m_pIServer->Connect()) {
		WCHAR astrMsg[100] = {0};
		char strIPAddr[MAX_IP_LEN] = {0};
		unsigned int nLen = MAX_IP_LEN;

		m_pIServer->GetServerIp(strIPAddr, &nLen);

		_sntprintf(astrMsg, 100, L"Unable to connect to server%s", A2W(strIPAddr));
		AfxMessageBox( astrMsg, 0, 0 );
		return;
	}
	
	if (m_pClient) {
		int iRet = AfxMessageBox(L"Currently being played back, re-retrieval operations will force stop playback, continue?(Y/N)", MB_YESNO);
		if (iRet == IDNO) return;
	}

	vector<CFtpDownloadThread *>::iterator iter = m_ftpDownloadThread.begin();
	for (; iter != m_ftpDownloadThread.end(); ++iter) {
		if ((*iter)->GetLastErrorCode() < 0) continue;

		if (((*iter)->GetDownloadLen() < (*iter)->GetFtpFileLen()) && ((*iter)->IsStart())) {
			int iRet = AfxMessageBox(L"The download task is not complete. Forced to stop downloading?(Y/N)", MB_YESNO);
			if (iRet == IDNO) return;
			else break;
		}
	}
	
	StopPlay();

	iter = m_ftpDownloadThread.begin();
	for (; iter != m_ftpDownloadThread.end(); ++iter) {
		(*iter)->Stop();
		delete *iter;
	}

	m_ftpDownloadThread.clear();

	m_nCurrentSelect = -1;
	
	{
		CAutoLock lock(&m_cs4ListUpdate);
		m_FilesList.DeleteAllItems();
		m_Files.clear();
	}

	m_pIServer->SetFileListAddCallBack(OnFileListAdd, (void*)this);
	
	if (m_pIServer->SendGetFiles() < 0) {
		AfxMessageBox(L"Send video files obtaining command failed! And the server may have been disconnected; please return the main interface to connect this server.");
		return;
	}
}

void CRecordPlayDlg::OnHdnItemdblclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CRecordPlayDlg::OnHdnDividerdblclickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CRecordPlayDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	NM_LISTVIEW * pNMListView = (NM_LISTVIEW *)pNMHDR;
	if (pNMListView->iItem == -1) return;
	
	//if ((m_nCurrentSelect == pNMListView->iItem) && m_pClient) return; //already play this file

	StopPlay();

	m_nCurrentSelect = pNMListView->iItem;

	m_DurationSlider.SetRange(0, 0);

	CString name = m_FilesList.GetItemText(pNMListView->iItem, 0);
	if (OpenStream(MakeRTSPUrl(name, m_pIServer), m_pIServer) < 0) return;

	m_bIsPlay = TRUE;
	GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText(L"Pause");
	m_dCurrentDuration = 0.0;
	m_dLastSendDuration = 0.0;
}

void CRecordPlayDlg::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<NMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	NM_LISTVIEW * pNMListView = (NM_LISTVIEW *)pNMHDR;
	if (pNMListView->iItem == -1) return;

	m_nCurrentSelect = pNMListView->iItem;
}

char * CRecordPlayDlg::MakeRTSPUrl(CString & strFile, IServer * pIServer)
{
	USES_CONVERSION;

	WCHAR astrMsg[100] = {0};
	static char strURL[MAX_PATH] = { 0 };

	unsigned int nPort = 0;
	
	char strStreamName[MAX_STREAM_NAME] = {0};
	char strIPAddr[MAX_IP_LEN];
	unsigned int len = MAX_IP_LEN;
	
	ZeroMemory(strURL, sizeof(strURL));

	strncpy(strURL, RTSP_PREFIX, strlen(RTSP_PREFIX));
	pIServer->GetServerIp(strIPAddr, &len);
	//memcpy(strIPAddr, "172.16.6.19", 12);
	//memcpy(strIPAddr, "192.168.1.2", 12);
	strncat(strURL, strIPAddr, len);

	pIServer->GetServerStreamPort(nPort);
	if (nPort) {
		strncat(strURL, PORT_PREFIX, strlen(PORT_PREFIX));
		char strPort[10] = {0};
		sprintf(strPort, "%d", nPort);
		strncat(strURL, strPort, strlen(strPort));
	}

	strncat(strURL, SEPARATOR, strlen(SEPARATOR));
	
	string strDir;
	pIServer->GetServerFileDir(strDir);
	if (strDir.length() > 0) {
		if (strDir.at(0) != '/') {
			strncat(strURL, SEPARATOR, strlen(SEPARATOR));
		}

		strncat(strURL, strDir.c_str(), strDir.length());
	}

	strncat(strURL, SEPARATOR, strlen(SEPARATOR));

	strncat(strURL, W2A(strFile), strFile.GetLength());
	
	return strURL;

	//return "rtsp://172.16.6.19/3.mkv";
}

#define MAX_WAIT_CNT	20

int CRecordPlayDlg::OpenStream(char * strURL, IServer * pIServer)
{
	USES_CONVERSION;

	WCHAR astrMsg[300] = {0};
	int iErrorCode = 0;

	if (m_pClient != NULL)	CloseTheStream();

	m_pClient = CAimer39RTSPClient::CreateNew();
	if (NULL == m_pClient) {
		AfxMessageBox( L"Unable to create stream ... lack of memory!", 0, 0 );
		return -1;
	}

	m_pClient->RegisterFinishCallback(OnClientFinish, this);
	m_pClient->RegistrerStartPlayCallback(OnStartPlay, this);
	m_pClient->RegistrerPlayNPTCallback(OnPlayNPTUpdate, this, 0.0);
	m_pClient->RegisterDisConnCallback(OnClientFinish, this, 2000); //2s 未收到任何数据，认为断开连接，断开连接按播放完成处理。

	iErrorCode = m_pClient->OpenURL(strURL);
	if (iErrorCode < 0) {
		_sntprintf(astrMsg, 300, L"OpenURL %s error! error = %s", A2W(strURL), A2W(m_pClient->GetLastError()));
		AfxMessageBox(astrMsg, 0, 0);
		return -1;
	}
	
	int nWaitCnt = 0;
	bool isPrepare = false;
	while ( !isPrepare ) {
		iErrorCode = m_pClient->IsPrepare( isPrepare );
		if ((iErrorCode != 0) || (nWaitCnt >= MAX_WAIT_CNT)) {
			if ((iErrorCode == 0) && (nWaitCnt >= MAX_WAIT_CNT)) 
				_sntprintf(astrMsg, 300, L"Connect to server% s, timeout!", A2W(strURL));
			else
				_sntprintf(astrMsg, 300, L"OpenURL %s error! error = %s", A2W(strURL), A2W(m_pClient->GetLastError()));

			AfxMessageBox(astrMsg, 0, 0);
			if (m_pClient) {
				m_pClient->Close();
				delete m_pClient;
				m_pClient = NULL;
			}
			return -1;
		}
		
		++nWaitCnt;
		Sleep(100);
	}
	
	unsigned int iStreamCnt = 0;
	STREAM_TYPE type = STREAM_AUDIO;
	m_pClient->GetStreamCount(iStreamCnt);

	for (unsigned int i = 0; i < iStreamCnt; ++i) {
		m_pClient->GetStreamType(i, type);

		if (type == STREAM_AUDIO) {
			m_AudioDecoder = CFfmpegEnvoy::createNew();
			m_AudioRender = CAudioRender::createNew();

			m_AudioDecoder->OpenFfmpeg();
			m_AudioRender->OpenRender();

			//m_AudioRender->setClock(&m_SyncClock);

			m_pClient->RegisterSink(type, m_AudioDecoder);
			m_AudioDecoder->RegisterSink(m_AudioRender, SINK_AUDIO);
		}else if (type == STREAM_VIDEO) {
			m_videoDecoder = CFfmpegEnvoy::createNew();
			m_videoRender = CVideoRender::createNew();

			m_videoDecoder->OpenFfmpeg();
			m_videoRender->OpenRender(m_Preview.m_hWnd);

			//m_SyncClock.ReInit();
			//m_videoRender->setClock(&m_SyncClock);
			
			m_pClient->RegisterSink(type, m_videoDecoder);
			m_videoDecoder->RegisterSink(m_videoRender, SINK_VIDEO);
		}
	}
	
	m_pClient->Play();
	
	if (m_videoDecoder)
		m_videoDecoder->Start();

	if (m_AudioDecoder)
		m_AudioDecoder->Start();

	return 0;
}

int CRecordPlayDlg::CloseTheStream()
{
	if (m_pClient) {
		m_pClient->Close();
		delete m_pClient;
	}

	m_pClient = NULL;
	
	if (m_videoDecoder) delete m_videoDecoder;
	m_videoDecoder = NULL;
	if (m_videoRender) delete m_videoRender;
	m_videoRender = NULL;

	if (m_AudioDecoder)	delete m_AudioDecoder;
	m_AudioDecoder = NULL;
	if (m_AudioRender) delete m_AudioRender;
	m_AudioRender = NULL;

	return 0;
}

LRESULT CRecordPlayDlg::OnStartPlayMsg(WPARAM wParam, LPARAM lParam)
{
	STARTPLAY_PARAM * pParam = (STARTPLAY_PARAM *)lParam;

	if (pParam->pClient != m_pClient){
		AfxMessageBox(L"a client no under control start play!\n");
		return -1;
	}

	if (m_bIsScroll) return 0;
	
	m_dDuration = pParam->duration;
	if (pParam->duration < 0) {
		pParam->duration = 0.0;
		m_dDuration = MAX_DURATION; //服务端没有返回总时间，默认使用最大时间作为播放总时间。
	}

	int nMax = 0, nPos = 0;
	nMax = (int)(pParam->duration);

	if (!pParam->startDuration || !m_DurationSlider.GetRangeMax()) {
		m_DurationSlider.SetRange(0, nMax, 1);

		CString strDuration;
		strDuration.Format(L"%d:%02d", nMax / 60, nMax % 60);
		GetDlgItem(IDC_STATIC_DURATION)->SetWindowText(strDuration);
	}

	if (pParam->startDuration < 0) pParam->startDuration = 0.0;
	
	m_dStartPlayTime = pParam->startDuration;
	
	if (m_dCurrentDuration < pParam->startDuration)
		m_dCurrentDuration = pParam->startDuration;

	nPos = (int)(m_dCurrentDuration + 0.5);

	m_DurationSlider.SetPos(nPos);

	CString strTime;
	strTime.Format(L"%d:%02d", nPos / 60, nPos % 60);
	GetDlgItem(IDC_STATIC_CTIME)->SetWindowText(strTime);

	return 0;
}

LRESULT CRecordPlayDlg::OnNPTUpdateMsg(WPARAM wParam, LPARAM lParam)
{
	NPT_UPDATE * pParam = (NPT_UPDATE*)lParam;

	if (m_bIsScroll) return 0;

	int nPos = (int)(m_dCurrentDuration + 0.5);
	m_DurationSlider.SetPos(nPos);
	
	CString strTime;
	strTime.Format(L"%d:%02d", nPos / 60, nPos % 60);
	GetDlgItem(IDC_STATIC_CTIME)->SetWindowText(strTime);

	return 0;
}

LRESULT CRecordPlayDlg::OnPlayFinish(WPARAM wParam, LPARAM lParam)
{
	if (NULL == m_pClient){
		return -1;
	}

	uint64_t nID = lParam;
	nID = (nID << 32) | wParam;

	if (nID != m_pClient->GetID())
		return -1;
	
	StopPlay();

	return 0;
}

void CRecordPlayDlg::OnBnClickedButtonPlay()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_bIsPlay) {
		if (!m_pClient) return;
		m_pClient->PausePlay();
		m_bIsPlay = FALSE;
		GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText(L"Play");
	}else {
		BOOL bNeedStartPlay = TRUE;
		if (!m_pClient) {
			if (m_nCurrentSelect < 0) return;
			CString name = m_FilesList.GetItemText(m_nCurrentSelect, 0);
			if (OpenStream(MakeRTSPUrl(name, m_pIServer), m_pIServer) < 0) return;
			bNeedStartPlay = FALSE;
		}

		if (bNeedStartPlay) m_pClient->StartPlay(m_dCurrentDuration);

		m_bIsPlay = TRUE;
		GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText(L"Pause");
	}

	m_FilesList.SetFocus();
}

void CRecordPlayDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_pClient) return;
	
	StopPlay();

	m_FilesList.SetFocus();
}

void CRecordPlayDlg::StopPlay()
{
	CloseTheStream();
	m_bIsPlay = FALSE;
	m_dCurrentDuration = 0.0;
	m_dLastSendDuration = 0.0;
	m_dStartPlayTime = 0.0;
	m_DurationSlider.SetPos(0);
	m_DurationSlider.SetRange(0, 0);
	m_Preview.Invalidate();
	GetDlgItem(IDC_STATIC_CTIME)->SetWindowText(L"0");
	GetDlgItem(IDC_STATIC_DURATION)->SetWindowText(L"0");
	GetDlgItem(IDC_BUTTON_PLAY)->SetWindowText(L"Play");
}

void CRecordPlayDlg::Close()
{
	vector<CFtpDownloadThread *>::iterator iter = m_ftpDownloadThread.begin();
	for (; iter != m_ftpDownloadThread.end(); ++iter) {
		(*iter)->Stop();
		delete *iter;
	}

	m_ftpDownloadThread.clear();

	StopPlay();

	if (m_pIServer) m_pIServer->DisConnect();
		
	m_FilesList.DeleteAllItems();
	
	m_DurationSlider.SetRange(0, 0);

	m_pIServer->SetFileListAddCallBack(NULL, NULL);
	
	m_Files.clear();

	KillTimer(1);
}

void CRecordPlayDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	vector<CFtpDownloadThread *>::iterator iter = m_ftpDownloadThread.begin();
	for (; iter != m_ftpDownloadThread.end(); ++iter) {
		if ((*iter)->GetLastErrorCode() < 0) continue;

		if (((*iter)->GetDownloadLen() < (*iter)->GetFtpFileLen()) && ((*iter)->IsStart())) {
			int iRet = AfxMessageBox(L"The download task is not finished, need to force quit?(Y/N)", MB_YESNO);
			if (iRet == IDNO) return;
			else break;
		}
	}

	Close();
	CDialog::OnClose();
}

void CRecordPlayDlg::OnBnClickedButtonExit()
{
	// TODO: 在此添加控件通知处理程序代码
	vector<CFtpDownloadThread *>::iterator iter = m_ftpDownloadThread.begin();
	for (; iter != m_ftpDownloadThread.end(); ++iter) {
		if ((*iter)->GetLastErrorCode() < 0) continue;

		if (((*iter)->GetDownloadLen() < (*iter)->GetFtpFileLen()) && ((*iter)->IsStart())) {
			int iRet = AfxMessageBox(L"The download task is not finished, need to force quit?(Y/N)", MB_YESNO);
			if (iRet == IDNO) return;
			else break;
		}
	}

	Close();
	EndDialog(0);
}

void CRecordPlayDlg::OnBnClickedButtonDownload()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_nCurrentSelect < 0) {
		AfxMessageBox(L"Please select a file from the file list to download!");
		return;
	}

	if (m_strDownloadDir.GetLength() == 0) {
		AfxMessageBox(L"Unable to create the download folder! Please make sure whether there is AnykaIPC directory in the D disk. Don’t download.");
		return;	
	}

	vector<CFtpDownloadThread *>::iterator iter = m_ftpDownloadThread.begin();
	for (; iter != m_ftpDownloadThread.end(); ++iter) {
		if ((*iter)->GetID() == m_nCurrentSelect) break;
	}

	if (iter != m_ftpDownloadThread.end()) {
		if ((*iter)->GetLastErrorCode() < 0) {
			(*iter)->Stop();
			delete *iter;
			iter = m_ftpDownloadThread.erase(iter);
			iter = m_ftpDownloadThread.end();
		}else if (((*iter)->GetDownloadLen() < (*iter)->GetFtpFileLen()) && (*iter)->IsStart()) {
			AfxMessageBox(L"Current file being downloaded");
			return;
		}else if (((*iter)->GetDownloadLen() == (*iter)->GetFtpFileLen()) && ((*iter)->GetDownloadLen())) {
			int iRet = AfxMessageBox(L"The current file has been downloaded, re-download it?(Y/N)", MB_YESNO);
			if (iRet == IDNO) return;
			else {
				(*iter)->Stop();
				delete *iter;
				iter = m_ftpDownloadThread.erase(iter);
				iter = m_ftpDownloadThread.end();
			}
		}
	}
	
	if (iter == m_ftpDownloadThread.end()) {
		if (m_ftpDownloadThread.size() >= MAX_DOWNLOAD_THREAD) {
			AfxMessageBox(L"Currently downloading thread is full! Please wait for a new download again after the download is complete!");
			return;
		}

		char strIPAddr[MAX_IP_LEN] = {0};
		unsigned int len = MAX_IP_LEN;

		m_pIServer->GetServerIp(strIPAddr, &len);
		CString name = m_FilesList.GetItemText(m_nCurrentSelect, 0);
		
		CFtpDownloadThread *pdownloadThread = new CFtpDownloadThread(m_nCurrentSelect, strIPAddr, FTP_SERVER_PORT, name.GetBuffer(), m_strDownloadDir.GetBuffer());
		m_ftpDownloadThread.push_back(pdownloadThread);
		iter = m_ftpDownloadThread.end() - 1;
	}
	
	int iRet = (*iter)->Start();
	if (iRet < 0) {
		string strErrMsg;
		(*iter)->GetLastErrorMsg((*iter)->GetLastErrorCode(), strErrMsg);
		CString strError;

		CString strFileName = m_FilesList.GetItemText((*iter)->GetID(), 0);
		strError.Format(L"Download file% s error:% s", strFileName, strErrMsg.c_str());
		
		(*iter)->Stop();
		delete *iter;
		iter = m_ftpDownloadThread.erase(iter);
		AfxMessageBox(strError);
	}
}

void CRecordPlayDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	static volatile BOOL just_one = FALSE;

	USES_CONVERSION;
	if (nIDEvent == 1) {
		if (just_one) return;

		just_one = TRUE;

		double fDownloadPercent = 0;
		CString strPercent;
		CString strErrMsg;
		int nErrorCode = 0;
		uint64_t nDownloadLen = 0, nFtpFileLen = 0;
		vector<CFtpDownloadThread *>::iterator iter = m_ftpDownloadThread.begin();
		for (; iter != m_ftpDownloadThread.end(); ) {
			nDownloadLen = (*iter)->GetDownloadLen(nErrorCode);
			nFtpFileLen = (*iter)->GetFtpFileLen(nErrorCode);
			
			if (!nDownloadLen || !nFtpFileLen) fDownloadPercent = 0;
			else fDownloadPercent = ((double)nDownloadLen / (double)nFtpFileLen) * 100.0;

			if (nErrorCode < 0) {
				string strError;
				(*iter)->GetLastErrorMsg(nErrorCode, strError);
				CString strFileName = m_FilesList.GetItemText((*iter)->GetID(), 0);

				strErrMsg.Format(L"Download file error:%s", A2W(strError.c_str()));

				(*iter)->Stop();
				m_FilesList.SetItemText((*iter)->GetID(), 1, strErrMsg);
				delete *iter;
				iter = m_ftpDownloadThread.erase(iter);
				continue;
			}
			
			if (!fDownloadPercent && !((*iter)->IsStart())) fDownloadPercent = 100;

			strPercent.Format(L"%d%%", (int)fDownloadPercent);
			m_FilesList.SetItemText((*iter)->GetID(), 1, strPercent);
			
			//download complete
			if ((nDownloadLen == nFtpFileLen) && !((*iter)->IsStart())) {
				delete *iter;
				iter = m_ftpDownloadThread.erase(iter);
				continue;
			}

			 ++iter;
		}
		
		just_one = FALSE;
	}

	CDialog::OnTimer(nIDEvent);
}

void CRecordPlayDlg::OnLvnInsertitemList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

LRESULT CRecordPlayDlg::OnAddFile(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;
	
	CAutoLock lock(&m_cs4ListUpdate);
	while (!m_Files.empty()) {
		int iInsertIdx = m_FilesList.GetItemCount();
		m_FilesList.InsertItem(iInsertIdx, A2W(m_Files.front().c_str()));
		m_Files.pop_front();
	}

	return 0;
}

LRESULT CRecordPlayDlg::OnScrollBegin(WPARAM wParam, LPARAM lParam)
{
	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)lParam;
	int iID = pSliderCtrl->GetDlgCtrlID();

	if (iID == IDC_SLIDER1) {
		m_bIsScroll = TRUE;
	}

	return 0;
}

LRESULT CRecordPlayDlg::OnScrollEnd(WPARAM wParam, LPARAM lParam)
{
	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)lParam;
	int iID = pSliderCtrl->GetDlgCtrlID();
	int iPos = pSliderCtrl->GetPos();

	if (iID == IDC_SLIDER1) {
		if (!pSliderCtrl->GetRangeMax() || !m_pClient || iPos <= 0)  {
			m_bIsScroll = FALSE;
			return 0;
		}

		m_pClient->PausePlay();

		double dPlayTime = iPos - 0.5;
		m_pClient->StartPlay(dPlayTime);
		m_bIsScroll = FALSE;
	}

	return 0;
}

LRESULT CRecordPlayDlg::OnThumbTrack(WPARAM wParam, LPARAM lParam)
{
	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)lParam;
	int iID = pSliderCtrl->GetDlgCtrlID();
	int iPos = pSliderCtrl->GetPos();

	if (iID == IDC_SLIDER1) {
		m_bIsScroll = TRUE;
		CString strTime;
		strTime.Format(L"%d:%02d", iPos / 60, iPos % 60);
		GetDlgItem(IDC_STATIC_CTIME)->SetWindowText(strTime);
	}

	return 0;
}