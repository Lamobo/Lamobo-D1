// MotionDetectDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Anyka IP Camera.h"
#include "MotionDetectDlg.h"

#define WIDGET_START_XPOS_OFFSET		140
#define WIDGET_START_YPOS_OFFSET		20

#define STATIC_TEXT_WIDTH				30
#define STATIC_TEXT_HEIGHT				15

#define	SERVER_COMBO_WIDTH				150
#define SERVER_COMBO_HEIGHT				20

#define WIDGET_APART_HEIGTH				20
#define WIDGET_APART_WIDTH				10

#define CHECK_BOX_WIDTH					40
#define CHECK_BOX_HEIGHT				15

#define	SENSITIVITY_COMBO_WIDTH			80
#define SENSITIVITY_COMBO_HEIGHT		20

#define FRAME_MARGIN_WIDTH				16
#define FRAME_MARGIN_HEIGHT				9

#define BUTTON_WIDTH					70
#define BUTTON_HEIGHT					25

// CMotionDetectDlg 对话框

#define RTSP_PREFIX				"rtsp://"
#define SEPARATOR				"/"
#define PORT_PREFIX				":"

#define LINE_CNT				8.0 //64 matrix
#define LINE_WIDTH				2.0
#define DEFAULT_COLOR			D3DCOLOR_XRGB(41, 36, 33)
#define AREA_LINE_ID(x, y)		(((y) * LINE_CNT + (x)) + (LINE_CNT - 1) * 2)
#define AREA_LINE_COLOR			D3DCOLOR_XRGB(50, 205, 50)
#define LINE_OPACITY			255
#define AREA_LINE_OPACITY		120

typedef pair<map<string, IServer*>::iterator, BOOL>		InsertRet;

IMPLEMENT_DYNAMIC(CMotionDetectDlg, CDialog)

CMotionDetectDlg::CMotionDetectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMotionDetectDlg::IDD, pParent)
{
	
	m_pClient = NULL;
	m_videoDecoder = NULL;
	m_videoRender = NULL;

	m_bNeedWait = FALSE;
	m_nSensitivity = 0;
	m_nMDAreas = 0;

	m_pCurServer = NULL;
}

CMotionDetectDlg::~CMotionDetectDlg()
{
}

void CMotionDetectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_DevicesCombo);
	DDX_Control(pDX, IDC_COMBO2, m_SensitivityCombo);
	DDX_Control(pDX, IDC_CHECK1, m_CheckBox);
}


BEGIN_MESSAGE_MAP(CMotionDetectDlg, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CMotionDetectDlg::OnCbnSelchangeDeviceCombo)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_BN_CLICKED(IDOK, &CMotionDetectDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMotionDetectDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBO2, &CMotionDetectDlg::OnCbnSelchangeSensitivityCombo)
END_MESSAGE_MAP()


// CMotionDetectDlg 消息处理程序
void CMotionDetectDlg::InitWidgetPosition()
{
	int xPos = WIDGET_START_XPOS_OFFSET;
	int yPos = WIDGET_START_YPOS_OFFSET;

	CWnd * pStaticText1 = GetDlgItem(IDC_STATIC1);
	CWnd * pStaticText2 = GetDlgItem(IDC_STATIC2);

	pStaticText1->MoveWindow(xPos, yPos, STATIC_TEXT_WIDTH, STATIC_TEXT_HEIGHT);
	pStaticText1->SetWindowText(L"Device:");

	xPos = xPos + STATIC_TEXT_WIDTH + WIDGET_APART_WIDTH;
	yPos = yPos - 4;

	m_DevicesCombo.MoveWindow(xPos, yPos, SERVER_COMBO_WIDTH, SERVER_COMBO_HEIGHT);

	CRect rect;
	m_DevicesCombo.GetWindowRect(&rect);
	ScreenToClient(&rect);

	m_CheckBox.MoveWindow(WIDGET_START_XPOS_OFFSET, rect.bottom + WIDGET_APART_HEIGTH, CHECK_BOX_WIDTH, CHECK_BOX_HEIGHT);

	m_CheckBox.GetWindowRect(&rect);
	ScreenToClient(&rect);

	pStaticText2->MoveWindow(rect.right + (6 * WIDGET_APART_WIDTH), rect.top, STATIC_TEXT_WIDTH * 2, STATIC_TEXT_HEIGHT);
	pStaticText2->SetWindowText(L"Sensitivity:");

	pStaticText2->GetWindowRect(&rect);
	ScreenToClient(&rect);
	
	m_SensitivityCombo.MoveWindow(rect.right + WIDGET_APART_WIDTH, rect.top - 4, SENSITIVITY_COMBO_WIDTH, SENSITIVITY_COMBO_HEIGHT);
	
	m_SensitivityCombo.GetWindowRect(&rect);
	ScreenToClient(&rect);

	CRect MDDlgRect;
	GetWindowRect(&MDDlgRect);
	ScreenToClient(&MDDlgRect);

	CWnd * pStaticText3 = GetDlgItem(IDC_STATIC3);
	pStaticText3->MoveWindow(FRAME_MARGIN_WIDTH, rect.bottom + WIDGET_APART_HEIGTH,
		MDDlgRect.Width() - (2 * FRAME_MARGIN_WIDTH), 
		MDDlgRect.Height() - rect.bottom - 2 * WIDGET_APART_HEIGTH - (2 * BUTTON_HEIGHT));

	pStaticText3->GetWindowRect(&rect);
	ScreenToClient(&rect);

	int nPreviewDialogWidth = rect.Width() - (2 * WIDGET_APART_WIDTH);
	int nPreviewDialogWidth1 = (nPreviewDialogWidth / 16) * 16;
	int nPreviewDialogHeight = (nPreviewDialogWidth1 / 16) * 9;

	int nStartOffSet = abs(nPreviewDialogWidth1 - nPreviewDialogWidth) / 2 + WIDGET_APART_WIDTH;
	
	m_PreviewDlg.Create(IDD_DIALOG_PREVIEW1, this);
	m_PreviewDlg.MoveWindow(rect.left + nStartOffSet, rect.top + WIDGET_APART_HEIGTH, nPreviewDialogWidth1, nPreviewDialogHeight );
	m_PreviewDlg.ShowWindow(SW_SHOW);

	CWnd * pSureButton = GetDlgItem(IDOK);
	CWnd * pCancelButton = GetDlgItem(IDCANCEL);

	pSureButton->MoveWindow(WIDGET_START_XPOS_OFFSET, rect.bottom + WIDGET_APART_HEIGTH / 2, BUTTON_WIDTH, BUTTON_HEIGHT);
	pCancelButton->MoveWindow(WIDGET_START_XPOS_OFFSET + BUTTON_WIDTH + (10 * WIDGET_APART_WIDTH), 
							rect.bottom + WIDGET_APART_HEIGTH / 2, BUTTON_WIDTH, BUTTON_HEIGHT);
}

#define DEVICE_PREFIX	L"Device%d:"

void CMotionDetectDlg::InitCombo()
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

		m_DevicesCombo.InsertString(i, strItemText);
		m_DevicesCombo.SetItemData(i, (DWORD_PTR)iter->second);
	}

	for(int i = 1; i < 7; ++i) {
		strItemText = L"";
		strItemText.Format(L"%d", i);
		m_SensitivityCombo.InsertString(i - 1, strItemText);
	}
}

BOOL CMotionDetectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	SetTimer(1, 1000, NULL);

	InitWidgetPosition();
	InitCombo();
	
	return TRUE;
}

BOOL CMotionDetectDlg::PreTranslateMessage(MSG * pMsg)
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

int CMotionDetectDlg::PutServerEntry(string strKey, IServer * pIServer)
{
	InsertRet ret = m_mapIServer.insert(pair<string, IServer *>(strKey, pIServer));
	return 0;
}

void CMotionDetectDlg::ChangePreviewPosition()
{
	int nVideoHeight = 0, nVideoWidth = 0, nHScale = 0, nWScale = 0;
	if (GetVideoHW(nVideoHeight, nVideoWidth, nHScale, nWScale) != 0) {
		AfxMessageBox(L"Unable to obtain the length and the width of the server\'s main video stream!", 0, 0);
		return;
	}
	
	CRect rect;
	m_PreviewDlg.GetWindowRect(&rect);
	ScreenToClient(&rect);
	
	int nPreviewDialogHeight = (rect.Width() / nWScale) * nHScale;
	int nHeightDiff = nPreviewDialogHeight - rect.Height();
	
	CRect TextRect;
	CWnd * pStaticText3 = GetDlgItem(IDC_STATIC3);
	pStaticText3->GetWindowRect(&TextRect);
	ScreenToClient(&TextRect);

	CRect PRect;
	GetWindowRect(&PRect);
	
	int nTextHeight = TextRect.Height() + nHeightDiff;
	int nMDDlgHeight = PRect.Height() + nHeightDiff;
	
	MoveWindow(PRect.left, PRect.top, PRect.Width(), nMDDlgHeight);

	pStaticText3->MoveWindow(TextRect.left, TextRect.top, TextRect.Width(), nTextHeight);
	m_PreviewDlg.MoveWindow(rect.left, rect.top, rect.Width(), nPreviewDialogHeight);
	
	CWnd * pSureButton = GetDlgItem(IDOK);
	CWnd * pCancelButton = GetDlgItem(IDCANCEL);
	pSureButton->GetWindowRect(&rect);
	ScreenToClient(&rect);

	pSureButton->MoveWindow(rect.left, rect.top + nHeightDiff, rect.Width(), rect.Height());
	
	pCancelButton->GetWindowRect(&rect);
	ScreenToClient(&rect);
	pCancelButton->MoveWindow(rect.left, rect.top + nHeightDiff, rect.Width(), rect.Height());
}

int CMotionDetectDlg::GetVideoHW(int & nHeight, int & nWidth, int & nHScale, int & nWScale)
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

void CMotionDetectDlg::DrawTheLine()
{
	if (!m_videoRender) return;

	CRect rect;
	m_PreviewDlg.GetClientRect(&rect);
	//m_PreviewDlg.ScreenToClient(&rect);

	double dYposIncrement = rect.Height() / LINE_CNT;
	double dYposIncrements = 0.0;

	POINT stPStart, stPEnd;
	stPStart.x = rect.left;
	stPStart.y = rect.top;
	stPEnd.x = rect.right;
	stPEnd.y = rect.top;

	for (int i = 0; i < LINE_CNT - 1; ++i) {
		dYposIncrements = dYposIncrement * (i + 1) + 0.5;

		stPStart.y = dYposIncrements;
		stPEnd.y = dYposIncrements;

		m_videoRender->DrawLine(i, stPStart, stPEnd, LINE_WIDTH, DEFAULT_COLOR, LINE_OPACITY);
	}

	double dXposIncrement = rect.Width() / LINE_CNT;
	double dXposIncrements = 0.0;
	
	stPStart.x = rect.left;
	stPEnd.x = rect.left;
	stPStart.y = rect.top;
	stPEnd.y = rect.bottom;

	for (int i = LINE_CNT - 1; i < 2 * LINE_CNT - 2; ++i) {
		dXposIncrements = dXposIncrement * (i - LINE_CNT + 2) + 0.5;

		stPStart.x = dXposIncrements;
		stPEnd.x = dXposIncrements;

		m_videoRender->DrawLine(i, stPStart, stPEnd, LINE_WIDTH, DEFAULT_COLOR, LINE_OPACITY);
	}
}

void CMotionDetectDlg::DrawTheArea()
{
	if (!m_videoRender) return;
	
	CPoint StartPos, EndPos;
	int nMatrixX = 0, nMatrixY = 0;
	
	CRect rect = 0;
	m_PreviewDlg.GetClientRect(&rect);
	
	int LineWidth = rect.Height() / LINE_CNT + 0.5;
	
	for (int i = 0; i < (LINE_CNT * LINE_CNT); ++i) {
		nMatrixX = i % (unsigned int(LINE_CNT));
		nMatrixY = i / LINE_CNT;

		StartPos.x = (rect.Width() / LINE_CNT) * nMatrixX + 0.5; //- LINE_WIDTH;
		StartPos.y = (rect.Height() / LINE_CNT) * nMatrixY + 0.5 +((rect.Height() / LINE_CNT) / 2);// - LINE_WIDTH;

		EndPos.x = (rect.Width() / LINE_CNT) * (nMatrixX + 1) + 0.5;// - LINE_WIDTH;
		EndPos.y = StartPos.y;

		if (m_nMDAreas & ((uint64_t)(1) << i))
			m_videoRender->DrawLine(AREA_LINE_ID(nMatrixX, nMatrixY), StartPos, EndPos, LineWidth, AREA_LINE_COLOR, AREA_LINE_OPACITY);
		else
			m_videoRender->DrawLine(AREA_LINE_ID(nMatrixX, nMatrixY), StartPos, EndPos, LineWidth, AREA_LINE_COLOR, 0);
	}
}

void CMotionDetectDlg::InitMotionDetectedAreas(IServer * pIServer)
{
	if (!pIServer) return;

	pIServer->GetMotionDetectedSensitivity(m_nSensitivity);
	pIServer->GetMotionDetectedAreas(m_nMDAreas);
	
	if (m_pClient && m_videoRender) {
		DrawTheLine();
		DrawTheArea();
	}

	if (m_nSensitivity && m_nMDAreas) {
		m_CheckBox.SetCheck(1);
	}
	
	if (m_nSensitivity)
		m_SensitivityCombo.SetCurSel(m_nSensitivity - 1);
	else  {
		m_SensitivityCombo.SetCurSel(2); //default sensitivity 3
		m_nSensitivity = 3;	
	}
}

void CMotionDetectDlg::OnCbnSelchangeDeviceCombo()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nSensitivity = 0;
	m_nMDAreas = (uint64_t)0;

	int iIndex = m_DevicesCombo.GetCurSel();

	IServer * pIServer = (IServer *)m_DevicesCombo.GetItemData(iIndex);
	if (m_pCurServer) m_pCurServer->DisConnect();
	int ret = pIServer->Connect();
	if (ret < 0) {
		AfxMessageBox(L"can't connect to server! please check the service is network\n", 0, 0);
		return;
	}

	m_pCurServer = pIServer;
	ChangePreviewPosition();

	m_pCurServer->SendGetMotionDetectedAreas();

	OpenStream(pIServer);
	//OpenStream(NULL);

	m_bNeedWait = TRUE;
	m_waitDialog.DoModal();

	InitMotionDetectedAreas(m_pCurServer);
}

void CMotionDetectDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1) {
		static int nWaitCount = 0;
		if (m_bNeedWait) {
			if (nWaitCount >= 5) {
				m_bNeedWait = FALSE;
				m_waitDialog.SendMessage(WM_CLOSE);
				nWaitCount = 0;
			}
			
			if (!m_pCurServer) {
				m_bNeedWait = FALSE;
				m_waitDialog.SendMessage(WM_CLOSE);
				nWaitCount = 0;
				goto End;
			}

			unsigned int nSensitivity = 0;
			m_pCurServer->GetMotionDetectedSensitivity(nSensitivity);
			if (nSensitivity) {
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

int CMotionDetectDlg::OpenStream(IServer * pIServer)
{
	if (!pIServer) return -1;

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
				_sntprintf(astrMsg, 100, L"Connect to the server%s, timeout!", A2W(strURL));
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

		}else if (type == STREAM_VIDEO) {
			m_videoDecoder = CFfmpegEnvoy::createNew();
			m_videoRender = CVideoRender::createNew();

			m_videoDecoder->OpenFfmpeg();
			m_videoRender->OpenRender(m_PreviewDlg.m_hWnd);

			m_pClient->RegisterSink(type, m_videoDecoder);
			m_videoDecoder->RegisterSink(m_videoRender, SINK_VIDEO);
		}
	}
	
	m_pClient->Play();
	
	if (m_videoDecoder)
		m_videoDecoder->Start();

	return 0;
}

char * CMotionDetectDlg::MakeRTSPUrl(IServer * pIServer)
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

int CMotionDetectDlg::CloseTheStream()
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

void CMotionDetectDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint cTempPoint = point;

	do {
		if (!m_videoRender) break;
		
		CRect rect = 0;
		m_PreviewDlg.GetClientRect(&rect);
		ClientToScreen(&point);
		m_PreviewDlg.ScreenToClient(&point);

		if (!rect.PtInRect(point)) break;
		
		//m_PreviewDlg.ScreenToClient(&point);

		int nMatrixX = point.x / (rect.Width() / LINE_CNT + 0.5);
		int nMatrixY = point.y / (rect.Height() / LINE_CNT + 0.5);
		
		CPoint StartPos, EndPos;
		int LineWidth = rect.Height() / LINE_CNT + 0.5;

		StartPos.x = (rect.Width() / LINE_CNT) * nMatrixX + 0.5;// - LINE_WIDTH;
		StartPos.y = (rect.Height() / LINE_CNT) * nMatrixY + 0.5 + ((rect.Height() / LINE_CNT) / 2);// - LINE_WIDTH;

		EndPos.x = (rect.Width() / LINE_CNT) * (nMatrixX + 1) + 0.5;// - LINE_WIDTH;
		EndPos.y = StartPos.y;
		
		if (m_nMDAreas & ((uint64_t)(1) << (unsigned int)((nMatrixY * LINE_CNT) + nMatrixX))) {
			m_videoRender->DrawLine(AREA_LINE_ID(nMatrixX, nMatrixY), StartPos, EndPos, LineWidth, AREA_LINE_COLOR, 0);
			m_nMDAreas &= (~((uint64_t)1 << (unsigned int)((nMatrixY * LINE_CNT) + nMatrixX)));
		}else {
			m_videoRender->DrawLine(AREA_LINE_ID(nMatrixX, nMatrixY), StartPos, EndPos, LineWidth, AREA_LINE_COLOR, AREA_LINE_OPACITY);
			m_nMDAreas |= ((uint64_t)1 << (unsigned int)((nMatrixY * LINE_CNT) + nMatrixX));
		}
	} while(0);

	point = cTempPoint;

	CDialog::OnLButtonDown(nFlags, point);
}

void CMotionDetectDlg::Close()
{
	KillTimer(1);

	m_DevicesCombo.Clear();
	m_SensitivityCombo.Clear();

	m_mapIServer.clear();

	if (m_pCurServer) m_pCurServer->DisConnect();
	m_pCurServer = NULL;

	CloseTheStream();

	m_nSensitivity = 0;
	m_nMDAreas = (uint64_t)0;
}

void CMotionDetectDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	MOTION_DETECT stMDetect = {0};
	if (!m_pCurServer) goto end;
	
	if (m_CheckBox.GetCheck()) {
		stMDetect.nSensitivity = m_nSensitivity;
		stMDetect.nMDAreas = m_nMDAreas;
	}

	m_pCurServer->SendSetMotionDetectedAreas(stMDetect);

end:
	Close();
	OnOK();
}

void CMotionDetectDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nSensitivity = 0;
	m_nMDAreas = (uint64_t)0;

	Close();
	OnCancel();
}

void CMotionDetectDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_nSensitivity = 0;
	m_nMDAreas = (uint64_t)0;

	Close();
	CDialog::OnClose();
}

void CMotionDetectDlg::OnCbnSelchangeSensitivityCombo()
{
	// TODO: 在此添加控件通知处理程序代码
	m_nSensitivity = m_SensitivityCombo.GetCurSel() + 1;
}
