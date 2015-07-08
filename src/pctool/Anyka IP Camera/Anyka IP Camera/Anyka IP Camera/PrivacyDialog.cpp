// PrivacyDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "Anyka IP Camera.h"
#include "PrivacyDialog.h"

#define RTSP_PREFIX				"rtsp://"
#define SEPARATOR				"/"
#define PORT_PREFIX				":"

// CPrivacyDialog 对话框

IMPLEMENT_DYNAMIC(CPrivacyDialog, CDialog)

CPrivacyDialog::CPrivacyDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPrivacyDialog::IDD, pParent)
{
	m_nCurSelectAreaNum = -1;
	m_pCurServer = NULL;

	memset(m_nIsUse, -1, sizeof(int) * 4);

	m_bStartDraw = FALSE;
	m_bNeedWait = FALSE;
	m_pClient = NULL;
	m_videoDecoder = NULL;
	m_videoRender = NULL;

	memset(m_DrawRect, 0, sizeof(CRect) * 4);
	memset(m_OriDrawRect, 0, sizeof(CRect) * 4);
}

CPrivacyDialog::~CPrivacyDialog()
{
}

void CPrivacyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK1, m_Check);
	DDX_Control(pDX, IDC_COMBO1, m_ServerCombox);
	DDX_Control(pDX, IDC_COMBO2, m_PrivacyCombo);
}

BEGIN_MESSAGE_MAP(CPrivacyDialog, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CPrivacyDialog::OnCbnSelchangeCombo1)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_CBN_SELCHANGE(IDC_COMBO2, &CPrivacyDialog::OnCbnSelchangeCombo2)
	ON_BN_CLICKED(IDC_BUTTON1, &CPrivacyDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK1, &CPrivacyDialog::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPrivacyDialog::OnBnClickedButton2)
END_MESSAGE_MAP()

#define WIDGET_START_XPOS_OFFSET		140
#define WIDGET_START_YPOS_OFFSET		20

#define STATIC_TEXT_WIDTH				30
#define STATIC_TEXT_HEIGHT				15
#define STATIC2_TEXT_WIDTH				55

#define WIDGET_APART_HEIGTH				20
#define WIDGET_APART_WIDTH				10

#define CHECK_BOX_WIDTH					40
#define CHECK_BOX_HEIGHT				15

#define	SERVER_COMBO_WIDTH				150
#define SERVER_COMBO_HEIGHT				20

#define	PRIVACY_COMBO_WIDTH				80
#define PRIVACY_COMBO_HEIGHT			20

#define PRIVACY_DIALOG_START_OFFSET		10

#define BUTTON_WIDTH					70
#define BUTTON_HEIGHT					25

int CPrivacyDialog::GetVideoHW(int & nHeight, int & nWidth, int & nHScale, int & nWScale)
{
	nHeight = nWidth = nHScale = nWScale = 0;
	if (!m_pCurServer) return -1;
	
	STREAMMODE enMode = STREAM_MODE_MAX;
	m_pCurServer->GetServerStreamMode(0, enMode);

	switch(enMode) {
	case STREAM_MODE_VIDEO_720P:
		nHeight = 720;
		nWidth = 1280;
		nWScale = 16;
		nHScale = 9;
		break;
	case STREAM_MODE_VIDEO_VGA:
		nHeight = 360; //480是标准的VGA高度，但当前产品中使用的是360
		nWidth = 640;
		nWScale = 16; //4:3是标准的VGA长宽比例，但由于图片高度变为360所以真实比例为16:9
		nHScale = 9;  //4:3是标准的VGA长宽比例，但由于图片高度变为360所以真实比例为16:9
		break;
	case STREAM_MODE_VIDEO_QVGA:
		nHeight = 240;
		nWidth = 320;
		nWScale = 4;
		nHScale = 3;
		break;
	case STREAM_MODE_VIDEO_D1:
		nHeight = 408;	//576是标准的D1高度，但当前产品中使用的是408
		nWidth = 720;
		nWScale = 30;	//5:4是标准的D1长宽比例，但由于图片高度变为408所以真实比例为30:17
		nHScale = 17;	//5:4是标准的D1长宽比例，但由于图片高度变为408所以真实比例为30:17
		break;
	default:
		return -1;
	}

	return 0;
}

void CPrivacyDialog::InitWidgetPosition()
{
	int xPos = WIDGET_START_XPOS_OFFSET;
	int yPos = WIDGET_START_YPOS_OFFSET;

	CWnd * pStaticText1 = GetDlgItem(IDC_STATIC1);
	CWnd * pStaticText2 = GetDlgItem(IDC_STATIC2);
	
	pStaticText1->MoveWindow(xPos, yPos, STATIC_TEXT_WIDTH, STATIC_TEXT_HEIGHT);
	pStaticText1->SetWindowText(L"Device:");

	xPos = xPos + STATIC_TEXT_WIDTH + WIDGET_APART_WIDTH;
	yPos = yPos - 4;

	m_ServerCombox.MoveWindow(xPos, yPos, SERVER_COMBO_WIDTH, SERVER_COMBO_HEIGHT);

	CRect rect;
	m_ServerCombox.GetWindowRect(&rect);
	ScreenToClient(&rect);

	m_Check.MoveWindow(WIDGET_START_XPOS_OFFSET, rect.bottom + WIDGET_APART_HEIGTH, CHECK_BOX_WIDTH, CHECK_BOX_HEIGHT);
	
	m_Check.GetWindowRect(&rect);
	ScreenToClient(&rect);

	pStaticText2->MoveWindow(rect.right + (6 * WIDGET_APART_WIDTH), rect.top, STATIC2_TEXT_WIDTH, STATIC_TEXT_HEIGHT);
	pStaticText2->SetWindowText(L"Block Area:");

	pStaticText2->GetWindowRect(&rect);
	ScreenToClient(&rect);
	
	m_PrivacyCombo.MoveWindow(rect.right + WIDGET_APART_WIDTH, rect.top - 4, PRIVACY_COMBO_WIDTH, PRIVACY_COMBO_HEIGHT);

	m_PrivacyCombo.GetWindowRect(&rect);
	ScreenToClient(&rect);
	
	CRect PRect;
	GetWindowRect(&PRect);
	ScreenToClient(&PRect);

	int nPreviewDialogWidth = PRect.Width() - (2 * PRIVACY_DIALOG_START_OFFSET);
	int nPreviewDialogWidth1 = (nPreviewDialogWidth / 16) * 16;
	int nStartOffSet = abs(nPreviewDialogWidth1 - nPreviewDialogWidth) / 4 + PRIVACY_DIALOG_START_OFFSET;

	int nPreviewDialogHeight = (nPreviewDialogWidth1 / 16) * 9;

	m_PreviweDialog.Create(IDD_DIALOG_PREVIEW1, this);
	m_PreviweDialog.MoveWindow(nStartOffSet, rect.bottom + WIDGET_APART_HEIGTH, 
								nPreviewDialogWidth1, nPreviewDialogHeight);
	m_PreviweDialog.ShowWindow(SW_SHOW);

	CWnd * pSureButton = GetDlgItem(IDC_BUTTON1);
	CWnd * pCancelButton = GetDlgItem(IDC_BUTTON2);
	
	m_PreviweDialog.GetWindowRect(&rect);
	ScreenToClient(&rect);

	pSureButton->MoveWindow(WIDGET_START_XPOS_OFFSET, rect.bottom + WIDGET_APART_HEIGTH, BUTTON_WIDTH, BUTTON_HEIGHT);
	pCancelButton->MoveWindow(WIDGET_START_XPOS_OFFSET + BUTTON_WIDTH + (10 * WIDGET_APART_WIDTH), 
							rect.bottom + WIDGET_APART_HEIGTH, BUTTON_WIDTH, BUTTON_HEIGHT);
}

void CPrivacyDialog::ChangePreviewPosition()
{
	int nVideoHeight = 0, nVideoWidth = 0, nHScale = 0, nWScale = 0;
	if (GetVideoHW(nVideoHeight, nVideoWidth, nHScale, nWScale) != 0) {
		AfxMessageBox(L"Unable to obtain the length and the width of the server\'s main video stream!", 0, 0);
		return;
	}
	
	CRect rect;
	m_PreviweDialog.GetWindowRect(&rect);
	ScreenToClient(&rect);

	int nPreviewDialogHeight = (rect.Width() / nWScale) * nHScale;
	int nHeightDiff = nPreviewDialogHeight - rect.Height();

	CRect PRect;
	GetWindowRect(&PRect);
	
	int nPrivacyDlgHeight = PRect.Height() + nHeightDiff;
	MoveWindow(PRect.left, PRect.top, PRect.Width(), nPrivacyDlgHeight);

	m_PreviweDialog.MoveWindow(rect.left, rect.top, rect.Width(), nPreviewDialogHeight);

	CWnd * pSureButton = GetDlgItem(IDC_BUTTON1);
	CWnd * pCancelButton = GetDlgItem(IDC_BUTTON2);
	
	pSureButton->GetWindowRect(&rect);
	ScreenToClient(&rect);
	pSureButton->MoveWindow(rect.left, rect.top + nHeightDiff, rect.Width(), rect.Height());

	pCancelButton->GetWindowRect(&rect);
	ScreenToClient(&rect);
	pCancelButton->MoveWindow(rect.left, rect.top + nHeightDiff, rect.Width(), rect.Height());
}

#define DEVICE_PREFIX	L"Device%d:"

void CPrivacyDialog::InitCombo()
{
	USES_CONVERSION;

	char strServerID[MAX_ID_LEN] = {0};
	unsigned int nLen = MAX_ID_LEN;
	CString strItemText;

	map<string, IServer*>::iterator iter = m_mapIServer.begin();
	for (int i = 0; iter != m_mapIServer.end(); ++iter, ++i) {
		nLen = MAX_ID_LEN;
		ZeroMemory(strServerID, sizeof(strServerID));

		iter->second->GetServerID(strServerID, &nLen);
		
		strItemText = L"";
		strItemText.Format(DEVICE_PREFIX, i);
		strItemText.Append(A2W(strServerID));

		m_ServerCombox.InsertString(i, strItemText);
		m_ServerCombox.SetItemData(i, (DWORD_PTR)iter->second);
	}

	for(int i = 1; i < 5; ++i) {
		strItemText = L"";
		strItemText.Format(L"%d", i);
		m_PrivacyCombo.InsertString(i - 1, strItemText);
	}
}

// CPrivacyDialog 消息处理程序
void CPrivacyDialog::InitPreviewWindows()
{
}

BOOL CPrivacyDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	SetTimer(1, 1000, NULL);
	
	InitWidgetPosition();
	InitCombo();
	InitPreviewWindows();
	
	return TRUE;
}

BOOL CPrivacyDialog::PreTranslateMessage(MSG * pMsg)
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

int CPrivacyDialog::PutServerEntry(string strKey, IServer * pIServer)
{
	InsertRet ret = m_mapIServer.insert(pair<string, IServer *>(strKey, pIServer));
	return 0;
}

void CPrivacyDialog::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nCurSelectAreaNum = -1;
	
	int iIndex = m_ServerCombox.GetCurSel();

	IServer * pIServer = (IServer *)m_ServerCombox.GetItemData(iIndex);
	if (m_pCurServer) m_pCurServer->DisConnect();
	int ret = pIServer->Connect();
	if (ret < 0) {
		AfxMessageBox(L"can't connect to server! please check the service network\n", 0, 0);
		return;
	}

	if (pIServer) pIServer->SendGetPrivacyArea();
	//if (pIServer == m_pCurServer) goto wait;
	
	m_pCurServer = pIServer;
	ChangePreviewPosition();

	OpenStream(m_pCurServer);

	m_bNeedWait = TRUE;
	m_waitDialog.DoModal();

	m_Check.SetCheck(0);
	m_PrivacyCombo.SetCurSel(-1);

	InitPrivacyArea(m_pCurServer);
}

void CPrivacyDialog::InitPrivacyArea(IServer * pIServer)
{
	if (!pIServer) return;

	memset(m_nIsUse, -1, sizeof(int) * 4);
	memset(m_nOriIsUse, -1, sizeof(int) * 4);
	memset(m_OriDrawRect, 0, sizeof(CRect) * 4);
	
	CRect rect;
	m_PreviweDialog.GetClientRect(&rect);
	//ScreenToClient(&rect);
	
	int nHeight = 0, nWidth = 0, nHeightScale = 0, nWidthScale = 0;
	if (GetVideoHW(nHeight, nWidth, nHeightScale, nWidthScale) < 0 ) {
		AfxMessageBox(L"Unable to obtain the length and the width of the server\'s main video stream!", 0, 0);
		return;
	}

	unsigned int nCount = 0;
	pIServer->GetPrivacyAreaCount(nCount);

	if (nCount == 0) {
		memset(m_OriDrawRect, 0, sizeof(CRect) * 4);
		memset(m_DrawRect, 0, sizeof(CRect) * 4);
		return;
	}
	
	double dHScale = (double)(rect.Height()) / (double)(nHeight);
	double dWScale = (double)(rect.Width()) / (double)(nWidth);

	PRIVACY_AREA stArea = {0};
	for (unsigned int i = 0; i < nCount; ++i) {
		pIServer->GetPrivacyArea(i, stArea);

		if (stArea.nNumber > 0) {
			m_DrawRect[stArea.nNumber - 1].left = stArea.nLeftTopX * dWScale + 0.5;
			m_DrawRect[stArea.nNumber - 1].top = stArea.nLeftTopY * dHScale + 0.5;
			m_DrawRect[stArea.nNumber - 1].right = m_DrawRect[stArea.nNumber - 1].left + stArea.nWidth * dWScale + 0.5;
			m_DrawRect[stArea.nNumber - 1].bottom = m_DrawRect[stArea.nNumber - 1].top + stArea.nHeight * dHScale + 0.5;

			m_OriDrawRect[stArea.nNumber - 1] = m_DrawRect[stArea.nNumber - 1];
			m_nIsUse[stArea.nNumber - 1] = TRUE;
			m_nOriIsUse[stArea.nNumber - 1] = TRUE;
		}
	}
}

void CPrivacyDialog::Close()
{
	KillTimer(1);
	
	m_ServerCombox.Clear();
	m_PrivacyCombo.Clear();

	m_mapIServer.clear();

	if (m_pCurServer) m_pCurServer->DisConnect();
	m_pCurServer = NULL;

	CloseTheStream();

	memcpy(m_DrawRect, m_OriDrawRect, sizeof(CRect) * 4);
	memcpy(m_nIsUse, m_nOriIsUse, sizeof(int) * 4);

	m_nCurSelectAreaNum = -1;
}

void CPrivacyDialog::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	memcpy(m_DrawRect, m_OriDrawRect, sizeof(CRect) * 4);
	memcpy(m_nIsUse, m_nOriIsUse, sizeof(int) * 4);

	Close();
	CDialog::OnClose();
}

void CPrivacyDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1) {
		if (m_bNeedWait) {
			static int nWaitCount = 0;
			if (nWaitCount >= 5) {
				m_bNeedWait = FALSE;
				m_waitDialog.SendMessage(WM_CLOSE);
				nWaitCount = 0;
			}
			
			if (!m_pCurServer){
				m_bNeedWait = FALSE;
				m_waitDialog.SendMessage(WM_CLOSE);
				nWaitCount = 0;
				goto End;
			}

			BOOL bIsComplete = FALSE;
			m_pCurServer->GetRespondComplete(COMM_TYPE_GET_INFO, GET_COMM_PRIVACY_AREA, bIsComplete);
			if (bIsComplete) {
				m_waitDialog.SendMessage(WM_CLOSE);
				nWaitCount = 0;
				m_bNeedWait = FALSE;
			}

			if (m_bNeedWait) ++nWaitCount;
		}
	}

End:
	CDialog::OnTimer(nIDEvent);
}

#define MAX_WAIT_CNT	20

int CPrivacyDialog::OpenStream(IServer * pIServer)
{
	USES_CONVERSION;

	WCHAR astrMsg[100] = {0};
	char * strURL = NULL;
	int iErrorCode = 0;

	strURL = MakeRTSPUrl(pIServer);

	if (m_pClient != NULL)	CloseTheStream();

	m_pClient = CAimer39RTSPClient::CreateNew();
	if (NULL == m_pClient) {
		AfxMessageBox( L"Unable to create stream ... lack of memory!", 0, 0 );
		return -1;
	}

	//m_pClient->RegisterFinishCallback(OnClientFinish, this);

	iErrorCode = m_pClient->OpenURL(strURL);
	if (iErrorCode < 0) {
		_sntprintf(astrMsg, 100, L"OpenURL %s error! error = %s", A2W(strURL), A2W(m_pClient->GetLastError()));
		AfxMessageBox(astrMsg, 0, 0);
		return -1;
	}
	
	int nWaitCnt = 0;
	bool isPrepare = false;
	while ( !isPrepare ) {
		iErrorCode = m_pClient->IsPrepare( isPrepare );
		if ((iErrorCode != 0) || (nWaitCnt >= MAX_WAIT_CNT)) {
			if ((iErrorCode == 0) && (nWaitCnt >= MAX_WAIT_CNT)) 
				_sntprintf(astrMsg, 100, L"Connect to server% s, timeout!", A2W(strURL));
			else
				_sntprintf(astrMsg, 100, L"OpenURL %s error! error = %s", A2W(strURL), A2W(m_pClient->GetLastError()));
			
			AfxMessageBox(astrMsg, 0, 0);
			m_pClient->Close();
			delete m_pClient;
			m_pClient = NULL;
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
			m_pClient->NoNeedStream(i + 1);
			//m_AudioDecoder = CFfmpegEnvoy::createNew();
			//m_AudioRender = CAudioRender::createNew();

			//m_AudioDecoder->OpenFfmpeg();
			//m_AudioRender->OpenRender();
			//m_AudioRender->setClock(&m_SyncClock);

			//m_pClient->RegisterSink(type, m_AudioDecoder);
			//m_AudioDecoder->RegisterSink(m_AudioRender, SINK_AUDIO);
		}else if (type == STREAM_VIDEO) {
			m_videoDecoder = CFfmpegEnvoy::createNew();
			m_videoRender = CVideoRender::createNew();

			m_videoDecoder->OpenFfmpeg();
			m_videoRender->OpenRender(m_PreviweDialog.m_hWnd);
			//m_videoRender->setClock(&m_SyncClock);
			
			m_pClient->RegisterSink(type, m_videoDecoder);
			m_videoDecoder->RegisterSink(m_videoRender, SINK_VIDEO);
		}
	}
	
	m_pClient->Play();
	
	if (m_videoDecoder)
		m_videoDecoder->Start();
	
	//if (m_AudioDecoder)
		//m_AudioDecoder->Start();

	return 0;
}

char * CPrivacyDialog::MakeRTSPUrl(IServer * pIServer)
{
	WCHAR astrMsg[100] = {0};
	static char strURL[MAX_PATH] = { 0 };

	unsigned int nPort = 0;
	
	char strStreamName[MAX_STREAM_NAME] = {0};
	char strIPAddr[MAX_IP_LEN];
	unsigned int len = MAX_IP_LEN;
	
	ZeroMemory(strURL, sizeof(strURL));

	strncpy(strURL, RTSP_PREFIX, strlen(RTSP_PREFIX));
	pIServer->GetServerIp(strIPAddr, &len);
	strncat(strURL, strIPAddr, len);

	pIServer->GetServerStreamPort(nPort);
	if (nPort) {
		strncat(strURL, PORT_PREFIX, strlen(PORT_PREFIX));
		char strPort[10] = {0};
		sprintf(strPort, "%d", nPort);
		strncat(strURL, strPort, strlen(strPort));
	}

	strncat(strURL, SEPARATOR, strlen(SEPARATOR));
	
	len = MAX_STREAM_NAME;
	pIServer->GetServerStreamName(0, strStreamName, &len); //get main stream is name
	strncat(strURL, strStreamName, len);

	return strURL;

	//return "rtsp://172.16.6.19/3.mkv";
}

int CPrivacyDialog::CloseTheStream()
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

	return 0;
}

#define DEFAULT_COLOR	D3DCOLOR_XRGB(8, 46, 84)
#define LINE_WIDTH		2

void CPrivacyDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_nCurSelectAreaNum == -1) {
		AfxMessageBox(L"Please select a region as an operating object!", 0, 0);
		return;
	}

	CRect rect = 0;
	CPoint cTempPoint = point;
	m_PreviweDialog.GetClientRect(&rect);
	//ScreenToClient(&rect);
	ClientToScreen(&point);
	m_PreviweDialog.ScreenToClient(&point);

	if (rect.PtInRect(point) && m_videoRender) {
		//ClientToScreen(&point);
		//m_PreviweDialog.ScreenToClient(&point);
		m_DrawRect[m_nCurSelectAreaNum].left = point.x;
		m_DrawRect[m_nCurSelectAreaNum].top = point.y;
		m_bStartDraw = TRUE;
	}

	point = cTempPoint;

	CDialog::OnLButtonDown(nFlags, point);
}

void CPrivacyDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect = 0;
	CPoint cTempPoint = point;
	m_PreviweDialog.GetClientRect(&rect);
	//ScreenToClient(&rect);
	ClientToScreen(&point);
	m_PreviweDialog.ScreenToClient(&point);

	if (rect.PtInRect(point) && m_bStartDraw && m_videoRender) { 
		//ClientToScreen(&point);
		//m_PreviweDialog.ScreenToClient(&point);		
		m_DrawRect[m_nCurSelectAreaNum].bottom = point.y;
		m_DrawRect[m_nCurSelectAreaNum].right = point.x;

		m_DrawRect[m_nCurSelectAreaNum].NormalizeRect();
		
		if (m_DrawRect[m_nCurSelectAreaNum].IsRectEmpty()){
			m_DrawRect[m_nCurSelectAreaNum] = m_OriDrawRect[m_nCurSelectAreaNum];
		}

		m_videoRender->DrawRectangle(0, m_DrawRect[m_nCurSelectAreaNum], LINE_WIDTH, DEFAULT_COLOR);

		CRect r1;
		for (int i = 0; i < 4; ++i) {
			if (i == m_nCurSelectAreaNum) continue;

			if (r1.IntersectRect(m_DrawRect[m_nCurSelectAreaNum], m_DrawRect[i])) {
				WCHAR astrMsg[100] = {0};
				_sntprintf(astrMsg, 100, L"Depicts the current rectangle in the block % d rectangular intersecting, please describe again!", i + 1);
				AfxMessageBox( astrMsg, 0, 0 );
				m_bStartDraw = FALSE;
				m_DrawRect[m_nCurSelectAreaNum] = m_OriDrawRect[m_nCurSelectAreaNum];
				m_videoRender->DrawRectangle(0, m_DrawRect[m_nCurSelectAreaNum], LINE_WIDTH, DEFAULT_COLOR);

				break;
			}
		}

		m_bStartDraw = FALSE;
	}

	point = cTempPoint;

	CDialog::OnLButtonUp(nFlags, point);
}

void CPrivacyDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect = 0;
	CPoint cTempPoint = point;
	m_PreviweDialog.GetClientRect(&rect);
	//ScreenToClient(&rect);
	ClientToScreen(&point);
	m_PreviweDialog.ScreenToClient(&point);

	if (nFlags == 1) { // Left button down
		if (m_videoRender && m_bStartDraw && rect.PtInRect(point)) {
			//ClientToScreen(&point);
			//m_PreviweDialog.ScreenToClient(&point);
			
			m_DrawRect[m_nCurSelectAreaNum].bottom = point.y;
			m_DrawRect[m_nCurSelectAreaNum].right = point.x;
			
			m_videoRender->DrawRectangle(0, m_DrawRect[m_nCurSelectAreaNum], LINE_WIDTH, DEFAULT_COLOR);
		}else if (m_videoRender && m_bStartDraw && (!rect.PtInRect(point))) {
			//ClientToScreen(&point);
			//m_PreviweDialog.ScreenToClient(&point);

			m_PreviweDialog.GetClientRect(&rect);
			//m_PreviweDialog.ScreenToClient(&rect);

			if (point.y < 0) point.y = 0;
			if (point.x < 0) point.x = 0;

			if (point.x >= rect.right) point.x = rect.right;
			if (point.y >= rect.bottom) point.y = rect.bottom;

			m_DrawRect[m_nCurSelectAreaNum].bottom = point.y;
			m_DrawRect[m_nCurSelectAreaNum].right = point.x;
			m_DrawRect[m_nCurSelectAreaNum].NormalizeRect();

			CRect r1;
			for (int i = 0; i < 4; ++i) {
				if (i == m_nCurSelectAreaNum) continue;

				if (r1.IntersectRect(m_DrawRect[m_nCurSelectAreaNum], m_DrawRect[i])) {
					WCHAR astrMsg[100] = {0};
					_sntprintf(astrMsg, 100, L"Depicts the current rectangle in the block % d rectangular intersecting, please describe again!", i + 1);
					AfxMessageBox( astrMsg, 0, 0 );
					m_bStartDraw = FALSE;
					m_DrawRect[m_nCurSelectAreaNum].SetRectEmpty();
					m_videoRender->DrawRectangle(0, m_DrawRect[m_nCurSelectAreaNum], LINE_WIDTH, DEFAULT_COLOR);

					break;
				}
			}

			m_bStartDraw = FALSE;
		}
	}

	point = cTempPoint;

	CDialog::OnMouseMove(nFlags, point);
}

void CPrivacyDialog::OnCbnSelchangeCombo2()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nCurSelectAreaNum = m_PrivacyCombo.GetCurSel();
	m_Check.SetCheck(0);

	if (m_videoRender) {
		m_videoRender->DrawRectangle(0, m_DrawRect[m_nCurSelectAreaNum], LINE_WIDTH, DEFAULT_COLOR); //clear

		if (m_pCurServer) {
			CString strText;
			unsigned int nCount = 0;
			PRIVACY_AREA stArea;

			m_PrivacyCombo.GetLBText(m_nCurSelectAreaNum, strText);
			m_pCurServer->GetPrivacyAreaCount(nCount);

			for (unsigned int i = 0; i < nCount; ++i){
				m_pCurServer->GetPrivacyArea(i, stArea);

				if (stArea.nNumber == _ttoi(strText)) m_Check.SetCheck(1);
			}

			if (m_nIsUse[m_nCurSelectAreaNum] != m_nOriIsUse[m_nCurSelectAreaNum])
				if (m_nIsUse[m_nCurSelectAreaNum] == TRUE) m_Check.SetCheck(1);
				else m_Check.SetCheck(0);
		}
	}
}

void CPrivacyDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	do {
		if (!m_videoRender) break;

		if (!m_pCurServer) break;
		
		int nVideoHeight = 0, nVideoWidth = 0, nHScale = 0, nWScale = 0;
		if (GetVideoHW(nVideoHeight, nVideoWidth, nHScale, nWScale) < 0 ) {
			AfxMessageBox(L"Unable to obtain the length and the width of the server\'s main video stream!", 0, 0);
			EndDialog(0);
			return;
		}
		
		int iSendNumber = 0;
		int iSendSelect[4] = {0};

		for (int i = 0, j = 0; i < 4; ++i) {
			if (m_nIsUse[i] != m_nOriIsUse[i]) {
				if (m_nIsUse[i] == TRUE) {
					if (!m_DrawRect[i].IsRectEmpty()) {
						++iSendNumber;
						iSendSelect[j++] = i;
					}
				} else if (m_nIsUse[i] == FALSE && m_nOriIsUse[i] == TRUE) {
					m_DrawRect[i].SetRectEmpty();

					++iSendNumber;
					iSendSelect[j++] = i;
				}
			}else {
				if (m_nIsUse[i] == TRUE) {
					if ((!m_DrawRect[i].IsRectEmpty()) && (m_DrawRect[i] != m_OriDrawRect[i])) {
						++iSendNumber;
						iSendSelect[j++] = i;
					}
				}
			}
		}
		
		if (iSendNumber == 0) break; //noting to send

		PRIVACY_AREA * pstArea = NULL;
		pstArea = new PRIVACY_AREA[iSendNumber];

		ZeroMemory(pstArea, sizeof(PRIVACY_AREA) * iSendNumber);
		
		CRect rect;
		m_PreviweDialog.GetClientRect(&rect);
		
		double dHScale = (double)(nVideoHeight) / (double)(rect.Height());
		double dWScale = (double)(nVideoWidth) / (double)(rect.Width());

		for (int i = 0; i < iSendNumber; ++i) {
			pstArea[i].nNumber = iSendSelect[i] + 1;
			pstArea[i].nLeftTopX = m_DrawRect[iSendSelect[i]].left * dWScale + 0.5;
			pstArea[i].nLeftTopY = m_DrawRect[iSendSelect[i]].top * dHScale + 0.5;
			
			pstArea[i].nHeight = m_DrawRect[iSendSelect[i]].Height() * dHScale + 0.5;
			pstArea[i].nWidth = m_DrawRect[iSendSelect[i]].Width() * dWScale + 0.5;

			m_pCurServer->SendSetPrivacyArea(pstArea + i, 1);
			Sleep(200);
			//dWScale
		}

		delete[] pstArea;

		InitPrivacyArea(m_pCurServer);
	} while(0);

	memcpy(m_DrawRect, m_OriDrawRect, sizeof(CRect) * 4);
	memcpy(m_nIsUse, m_nOriIsUse, sizeof(int) * 4);

	Close();
	EndDialog(0);
}

void CPrivacyDialog::OnBnClickedCheck1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_nCurSelectAreaNum == -1) {
		AfxMessageBox(L"Please select a region as an operating object!", 0, 0);
		m_Check.SetCheck(0);
		return;
	}

	int nState = m_Check.GetCheck();
	if (nState) {
		m_nIsUse[m_nCurSelectAreaNum] = TRUE;
	}
	else {
		m_nIsUse[m_nCurSelectAreaNum] = FALSE;
		//ZeroMemory(&m_DrawRect[m_nCurSelectAreaNum], sizeof(CRect));
	}
}

void CPrivacyDialog::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_videoRender) goto end;

	if (!m_pCurServer) goto end;

	memcpy(m_DrawRect, m_OriDrawRect, sizeof(CRect) * 4);
	memcpy(m_nIsUse, m_nOriIsUse, sizeof(int) * 4);

end:
	Close();
	EndDialog(0);
}