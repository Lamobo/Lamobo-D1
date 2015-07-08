// ISPControlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "ISPControlDlg.h"
#include "FFScale.h"
#include "yuv2rgb.h"
#include <math.h>
#include <gdiplus.h>
#include <Gdiplusgraphics.h>
#include "yuv2rgb.h"
#include "HeadFile.h"
#include "ServerInfo.h"

#define ATTEMPT_OPEN_MAX	3
#define CHAR_SEPARATOR		'/'

#define WM_CALC_RGBAVG				WM_USER + 100
#define WM_SERVER_RET_INFO			WM_USER + 302

using namespace Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static int av_lock_manager_cb(void ** mutex, enum AVLockOp lockOp)
{
	switch(lockOp) {
	case AV_LOCK_CREATE:
		*mutex = (void*)CreateMutex(NULL, false, NULL);
		break;
	case AV_LOCK_DESTROY:
		CloseHandle((HANDLE)*mutex);
		*mutex = NULL;
		break;
	case AV_LOCK_OBTAIN:
		WaitForSingleObject((HANDLE)*mutex, INFINITE);
		break;
	case AV_LOCK_RELEASE:
		ReleaseMutex((HANDLE)*mutex);
		break;
	}

	return 0;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CISPControlDlg 对话框

#define WINDOW_HEIGHT			690
#define WINDOW_WIDTH			910
#define GRAY_PIC_WIDTH			300

#define WIDGET_APART_WIDTH		2
#define WIDGET_APART_HEIGHT		2
#define TREE_WIDTH_SCALE		6

#define VIDEO_MODE_NAME_720P	L"720P"
#define VIDEO_MODE_NAME_VGA		L"VGA"
#define VIDEO_MODE_NAME_QVGA	L"QVGA"

#define RTSP_PREFIX				"rtsp://"
#define SEPARATOR				"/"
#define PORT_PREFIX				":"
#define TREE_ROOT_ITEM_NAME		L"Device Lists:"
#define MAX_RTSP_URL_LEN		(MAX_PATH + 24)

#define PAGE_ID_ENABLE			0
#define PAGE_ID_BB				1
#define PAGE_ID_LENS			2
#define PAGE_ID_DEMOSAIC		3
#define PAGE_ID_FILTER			4
#define PAGE_ID_WB				5
#define PAGE_ID_AWB				6
#define PAGE_ID_CCORRECT		7
#define PAGE_ID_GAMMA			8
#define PAGE_ID_BENHANCE		9
#define PAGE_ID_SATURATION		10
#define PAGE_ID_SPECEFF			11
#define PAGE_ID_REGISTER		12
#define PAGE_ID_RGBAVG			13

unsigned int CISPControlDlg::thread_begin( void * pParam )
{
	CISPControlDlg * pDlg = static_cast<CISPControlDlg *>(pParam);
	pDlg->Monitor();
	return 0;
}

void CISPControlDlg::OnClientFinish(void * pLParam, void * pRParam)
{
	CISPControlDlg * pthis = (CISPControlDlg *)pLParam;
	CAimer39RTSPClient * pClient = (CAimer39RTSPClient *)pRParam;
	
	if (pthis->m_pClient != pClient) {
		AfxMessageBox( L"WARN! a client no under control finish\n", 0, 0 );
		return;
	}

	pthis->m_Preview.Invalidate();
}

void CISPControlDlg::OnClientDisConnect(void * pLParam, void * pRParam)
{
	CISPControlDlg * pthis = (CISPControlDlg *)pLParam;
	CAimer39RTSPClient * pClient = (CAimer39RTSPClient *)pRParam;

#ifdef WARN_ERROR_OUT
	fprintf(stderr, "WARN::####Disconnet we will do the reconnect operate, because we didn't rec any video data in last 4 second####\n");
#endif
	
	if (pthis->m_pClient != pClient) {
		AfxMessageBox( L"WARN! a client no under control disconnect\n", 0, 0 );
		return;
	}
	
	if (pthis->m_pServerPreview)
		pthis->m_pServerPreview->DisConnect();
}

void CISPControlDlg::OnIspInfoParamRespond(IServer * pIServer, BYTE * pInfoParam, unsigned int nlen, void * pClassParam)
{
	CISPControlDlg * pthis = (CISPControlDlg *)pClassParam;

	if (pthis == NULL || (pthis->m_pServerPreview != pIServer)) return;
	
	int nInfoType = *((int *)pInfoParam);

	if (nInfoType == ISP_CID_AUTO_WHITE_BALANCE)
		pthis->m_pageAWB.SetAwbParamRespond(pInfoParam, nlen);
	else {
		fprintf(stderr, "a isp info responed param type unknown!\n");
	}
}

void CISPControlDlg::OnServerReturnInfo(IServer * pIServer, RETINFO * pstRetInfo, void * pClassParam)
{
	CISPControlDlg * pthis = (CISPControlDlg *)pClassParam;

	{
		CAutoLock lock(&(pthis->m_csForRet));
		memcpy(&(pthis->m_stRetInfo), pstRetInfo, sizeof(RETINFO));
	}

	pthis->PostMessage(WM_SERVER_RET_INFO, 0, (LPARAM)pIServer);
}


CISPControlDlg::CISPControlDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CISPControlDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_pClient = NULL;
	m_videoDecoder = NULL;
	m_videoRender = NULL;
	m_pServerPreview = NULL;

	m_bIsSearch = FALSE;
	m_runThreadFlag = FALSE;
	m_bNeedJudgeDisConnWork = TRUE;
	m_bDrawEnable = FALSE;

	ZeroMemory(m_YCnt, sizeof(m_YCnt));
	m_YMaxNum = 0;

	m_bNeedUpdateRGBAvg = FALSE;
	m_bLogImage = FALSE;

	m_strURL.clear();

	GdiplusStartupInput gdiplusStartupInput;
	
    // Initialize GDI+.
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
}

CISPControlDlg::~CISPControlDlg()
{
	if (m_MemDC.m_hDC) {
		m_MemDC.SelectObject(m_pOldMemBitmap);
		m_MemDC.DeleteDC();
		m_MemBitmap.DeleteObject();
	}
	
	if (m_bDrawEnable)
		m_GrayBarBitmap.DeleteObject();

	GdiplusShutdown(m_gdiplusToken);
}

void CISPControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE1, m_TreeCtrl);
	DDX_Control(pDX, IDC_TAB1, m_TabCtrl);
}

BEGIN_MESSAGE_MAP(CISPControlDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CISPControlDlg::OnBnClickedButtonOut)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CISPControlDlg::OnTcnSelchangeTab1)
	ON_WM_CTLCOLOR()
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_DRAWITEM()
	ON_WM_MOVE()
	ON_MESSAGE(WM_ENABLE_CHANGE, OnPageEnableChanegeMessage)
	ON_MESSAGE(WM_SUBMISSION, OnPageSubmissionMsg)
	ON_MESSAGE(WM_GET_ISP_INFO, OnPageGetInfoMsg)
	ON_NOTIFY(NM_RCLICK, IDC_TREE1, &CISPControlDlg::OnNMRClickTree1)
	ON_WM_TIMER()
	ON_COMMAND(ID_SEARCH_DEVICE, &CISPControlDlg::OnSearchDevice)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE1, &CISPControlDlg::OnNMDblclkTree1)
	ON_BN_CLICKED(IDC_BUTTON3, &CISPControlDlg::OnBnClickedButtonSubmission)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON2, &CISPControlDlg::OnBnClickedButtonIn)
	ON_WM_SYSKEYDOWN()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_IMAGECALCTYPE_log, &CISPControlDlg::OnImageCalcTypeLog)
	ON_COMMAND(ID_IMAGECALCTYPE_NORMAL, &CISPControlDlg::OnImageCalcTypeNormal)
	ON_WM_RBUTTONDOWN()
	ON_MESSAGE(WM_CALC_RGBAVG, OnCalcRGBAvgMsg)
	ON_BN_CLICKED(IDC_BUTTON4, &CISPControlDlg::OnBnClickedButton4)
	ON_MESSAGE(WM_SERVER_RET_INFO, &CISPControlDlg::OnServerRetInfo)
END_MESSAGE_MAP()

// CISPControlDlg 消息处理程序

void CISPControlDlg::InitPreviewWindows(BOOL bNeedCreate)
{
	CRect cWindowRect;

	GetClientRect(&cWindowRect);
	
	int nAssumeTreeWidth = cWindowRect.Width() / TREE_WIDTH_SCALE;

	int nPreviewWidth = cWindowRect.Width() - nAssumeTreeWidth - GRAY_PIC_WIDTH - 3 * WIDGET_APART_WIDTH;
	int nPreviewHeight = nPreviewWidth / 16 * 9; //16:9
	
	int nXpos = cWindowRect.left + nAssumeTreeWidth + 2 * WIDGET_APART_WIDTH;
	int nYpos = cWindowRect.top + WIDGET_APART_HEIGHT;
	
	if (bNeedCreate)
		m_Preview.Create(IDD_DIALOG_PREVIEW, this);

	m_Preview.MoveWindow(nXpos, nYpos, nPreviewWidth, nPreviewHeight);

	if (bNeedCreate)
		m_Preview.ShowWindow(SW_SHOW);
}

void CISPControlDlg::InitTreeCtrlPosition()
{
	CRect cWindowRect, cPreviewRect;
	
	GetClientRect(&cWindowRect);
	m_Preview.GetWindowRect(&cPreviewRect);
	ScreenToClient(&cPreviewRect);

	m_TreeCtrl.MoveWindow(cWindowRect.left + WIDGET_APART_WIDTH, cWindowRect.top + WIDGET_APART_HEIGHT, 
						cWindowRect.Width() / TREE_WIDTH_SCALE, cPreviewRect.Height() * 2 / 3);
}

void CISPControlDlg::InitTabCtrlPosition(BOOL bNeedInsert)
{
	CRect cWindowRect, cPreviewRect;
	GetClientRect(&cWindowRect);

	m_Preview.GetWindowRect(&cPreviewRect);
	ScreenToClient(&cPreviewRect);

	int nXpos = cWindowRect.left + WIDGET_APART_WIDTH;
	int nYpos = cPreviewRect.bottom + WIDGET_APART_HEIGHT;
	int nWidth = cWindowRect.Width() - 2 * WIDGET_APART_WIDTH;
	int nHeight = cWindowRect.Height() - cPreviewRect.Height() - 3 * WIDGET_APART_HEIGHT;

	m_TabCtrl.MoveWindow(nXpos, nYpos, nWidth, nHeight);

	if (!bNeedInsert) return;

	long lStyle = GetWindowLong(m_TabCtrl.GetSafeHwnd(), GWL_STYLE);
	lStyle = SetWindowLong(m_TabCtrl.GetSafeHwnd(), GWL_STYLE, lStyle | WS_CLIPCHILDREN);

	m_TabCtrl.InsertItem(PAGE_ID_ENABLE,	L"Enabled");
	m_TabCtrl.InsertItem(PAGE_ID_BB,		L"Black Balance");
	m_TabCtrl.InsertItem(PAGE_ID_LENS,		L"Lens Calibration");
	m_TabCtrl.InsertItem(PAGE_ID_DEMOSAIC,  L"Interpolation");
	m_TabCtrl.InsertItem(PAGE_ID_FILTER,	L"Filter");
	m_TabCtrl.InsertItem(PAGE_ID_WB,		L"White Balance");
	m_TabCtrl.InsertItem(PAGE_ID_AWB,		L"AWB Parameters Calculation");
	m_TabCtrl.InsertItem(PAGE_ID_CCORRECT,  L"Color Correction");
	m_TabCtrl.InsertItem(PAGE_ID_GAMMA,		L"Gamma");
	m_TabCtrl.InsertItem(PAGE_ID_BENHANCE,  L"Enhanced Border");
	m_TabCtrl.InsertItem(PAGE_ID_SATURATION,L"Saturation");
	m_TabCtrl.InsertItem(PAGE_ID_SPECEFF,	L"Effects Processing");
	m_TabCtrl.InsertItem(PAGE_ID_REGISTER,	L"Register Settings");
	m_TabCtrl.InsertItem(PAGE_ID_RGBAVG,	L"RGB Average Value Calculation");
}

void CISPControlDlg::InitButtonPosition()
{
	CRect cTreeRect, cPreviewRect;

	m_TreeCtrl.GetWindowRect(&cTreeRect);
	ScreenToClient(&cTreeRect);

	m_Preview.GetWindowRect(&cPreviewRect);
	ScreenToClient(&cPreviewRect);
	
	int nHeightMid = (cPreviewRect.bottom - cTreeRect.bottom) / 2;
	int nButtonHeight = (cPreviewRect.bottom - cTreeRect.bottom - 4 * WIDGET_APART_HEIGHT) / 3;
	int nButtonWidth = cTreeRect.Width();

	CButton * pButton1 =  (CButton *)GetDlgItem(IDC_BUTTON1);
	CButton * pButton2 =  (CButton *)GetDlgItem(IDC_BUTTON2);
	CButton * pButton3 =  (CButton *)GetDlgItem(IDC_BUTTON3);
	CButton * pButton4 =  (CButton *)GetDlgItem(IDC_BUTTON4);

	int nXpos = cTreeRect.left;
	int nYpos = cTreeRect.bottom + nHeightMid - nButtonHeight / 2;
	pButton2->MoveWindow(nXpos, nYpos, nButtonWidth, nButtonHeight);

	nYpos = nYpos - WIDGET_APART_HEIGHT - nButtonHeight;
	pButton1->MoveWindow(nXpos, nYpos, nButtonWidth, nButtonHeight);

	nYpos = nYpos + 2 * (nButtonHeight +  WIDGET_APART_HEIGHT);
	pButton3->MoveWindow(nXpos, nYpos, (nButtonWidth - WIDGET_APART_WIDTH) / 2 , nButtonHeight);
	pButton4->MoveWindow(nXpos + ((nButtonWidth + WIDGET_APART_WIDTH) / 2), nYpos, (nButtonWidth - WIDGET_APART_HEIGHT) / 2, nButtonHeight);
}

void CISPControlDlg::InitTabPage(BOOL bNeedCreate)
{
	if (bNeedCreate) {
		m_pageLens.Create(IDD_DIALOG_LENS, &m_TabCtrl);
		m_pageLens.SetPageID(PAGE_ID_LENS);
		m_pageLens.SetMessageWindow(this);

		m_pageEnable.Create(IDD_DIALOG_Enable, &m_TabCtrl);
		m_pageEnable.SetPageID(PAGE_ID_ENABLE);
		m_pageEnable.SetMessageWindow(this);

		m_pageAWB.Create(IDD_DIALOG_AWB, &m_TabCtrl);
		m_pageAWB.SetPageID(PAGE_ID_AWB);
		m_pageAWB.SetMessageWindow(this);

		m_pageBB.Create(IDD_DIALOG_BB, &m_TabCtrl);
		m_pageBB.SetPageID(PAGE_ID_BB);
		m_pageBB.SetMessageWindow(this);

		m_pageBEnhance.Create(IDD_DIALOG_BRI_ENHANCE, &m_TabCtrl);
		m_pageBEnhance.SetPageID(PAGE_ID_BENHANCE);
		m_pageBEnhance.SetMessageWindow(this);

		m_pageCCorrect.Create(IDD_DIALOG_COLOR_CORRECT, &m_TabCtrl);
		m_pageCCorrect.SetPageID(PAGE_ID_CCORRECT);
		m_pageCCorrect.SetMessageWindow(this);

		m_pageDemosaic.Create(IDD_DIALOG_DEMOSAIC, &m_TabCtrl);
		m_pageDemosaic.SetPageID(PAGE_ID_DEMOSAIC);
		m_pageDemosaic.SetMessageWindow(this);

		m_pageFilter.Create(IDD_DIALOG_FILTER, &m_TabCtrl);
		m_pageFilter.SetPageID(PAGE_ID_FILTER);
		m_pageFilter.SetMessageWindow(this);

		m_pageGamma.Create(IDD_DIALOG_GAMMA, &m_TabCtrl);
		m_pageGamma.SetPageID(PAGE_ID_GAMMA);
		m_pageGamma.SetMessageWindow(this);

		m_pageSaturation.Create(IDD_DIALOG_SATURATION, &m_TabCtrl);
		m_pageSaturation.SetPageID(PAGE_ID_SATURATION);
		m_pageSaturation.SetMessageWindow(this);

		m_pageSpecEff.Create(IDD_DIALOG_SPECIAL_EFFECT, &m_TabCtrl);
		m_pageSpecEff.SetPageID(PAGE_ID_SPECEFF);
		m_pageSpecEff.SetMessageWindow(this);

		m_pageWB.Create(IDD_DIALOG_WB, &m_TabCtrl);
		m_pageWB.SetPageID(PAGE_ID_WB);
		m_pageWB.SetMessageWindow(this);

		m_pageRegister.Create(IDD_DIALOG_REGISTER, &m_TabCtrl);
		m_pageRegister.SetPageID(PAGE_ID_REGISTER);
		m_pageRegister.SetMessageWindow(this);

		m_pageRGBAvg.Create(IDD_DIALOG_SHOW_RGB_AVG, &m_TabCtrl);
		m_pageRGBAvg.SetPageID(PAGE_ID_RGBAVG);
		m_pageRGBAvg.SetMessageWindow(this);
	}

	CRect rect;
	m_TabCtrl.GetClientRect(&rect);

	rect.top += 23;
	rect.left += 2;
	rect.right -= 3;
	rect.bottom -= 3;

	m_pageLens.MoveWindow(&rect);
	m_pageEnable.MoveWindow(&rect);
	m_pageAWB.MoveWindow(&rect);
	m_pageBB.MoveWindow(&rect);
	m_pageBEnhance.MoveWindow(&rect);
	m_pageCCorrect.MoveWindow(&rect);
	m_pageDemosaic.MoveWindow(&rect);
	m_pageFilter.MoveWindow(&rect);
	m_pageGamma.MoveWindow(&rect);
	m_pageSaturation.MoveWindow(&rect);
	m_pageSpecEff.MoveWindow(&rect);
	m_pageWB.MoveWindow(&rect);
	m_pageRegister.MoveWindow(&rect);
	m_pageRGBAvg.MoveWindow(&rect);

	m_pageEnable.EnableWindow(TRUE);
	m_pageEnable.ShowWindow(SW_SHOW);

	m_TabCtrl.SetCurSel(0);
}

#define MIN_GRAY_WINDOW_WIDTH	260
#define MIN_GRAY_WINDOW_HEIGHT	236
#define GRAY_IMAGE_WIDTH		256
#define GRAY_IMAGE_HEIGHT		180
#define GRAY_IMAGE_BAR_HEIGHT	10

void CISPControlDlg::InitGrayImangeWindowRect()
{
	CRect rect, windowRect;
	m_Preview.GetWindowRect(rect);
	ScreenToClient(rect);

	GetClientRect(windowRect);

	m_GrayImageWindowRect.left = rect.right + WIDGET_APART_WIDTH;
	m_GrayImageWindowRect.right = windowRect.right - WIDGET_APART_WIDTH;
	m_GrayImageWindowRect.top = rect.top;
	m_GrayImageWindowRect.bottom = rect.bottom;
	
	m_bDrawEnable = TRUE;

	if (m_GrayImageWindowRect.Width() < MIN_GRAY_WINDOW_WIDTH || 
		m_GrayImageWindowRect.Height() < MIN_GRAY_WINDOW_HEIGHT) {
		m_bDrawEnable = FALSE;
	}

	int nWidthMid = m_GrayImageWindowRect.left + m_GrayImageWindowRect.Width() / 2;

	m_GrayImageRect.left = nWidthMid - GRAY_IMAGE_WIDTH / 2;
	m_GrayImageRect.right = nWidthMid + GRAY_IMAGE_WIDTH / 2;
	m_GrayImageRect.top = m_GrayImageWindowRect.top + 3 * WIDGET_APART_HEIGHT;
	m_GrayImageRect.bottom = m_GrayImageRect.top + GRAY_IMAGE_HEIGHT;

	m_GrayImageBarRect.left = m_GrayImageRect.left;
	m_GrayImageBarRect.right = m_GrayImageRect.right;
	m_GrayImageBarRect.top = m_GrayImageRect.bottom + 3 * WIDGET_APART_HEIGHT;
	m_GrayImageBarRect.bottom = m_GrayImageBarRect.top + GRAY_IMAGE_BAR_HEIGHT;
	
	CWnd * pText1 = GetDlgItem(IDC_STATIC_AVERAGE);
	CWnd * pText2 = GetDlgItem(IDC_STATIC_WARP);
	CWnd * pText3 = GetDlgItem(IDC_STATIC_AVERAGE_VAL);
	CWnd * pText4 = GetDlgItem(IDC_STATIC_WARP_VAL);

	CRect textRect;
	pText1->GetClientRect(textRect);

	int nHeightRemain = m_GrayImageWindowRect.bottom -  m_GrayImageBarRect.bottom;
	if (nHeightRemain < ((textRect.Height() + WIDGET_APART_HEIGHT) * 2)) {
		pText1->EnableWindow(FALSE);
		pText1->ShowWindow(SW_HIDE);

		pText2->EnableWindow(FALSE);
		pText2->ShowWindow(SW_HIDE);

		pText3->EnableWindow(FALSE);
		pText3->ShowWindow(SW_HIDE);

		pText4->EnableWindow(FALSE);
		pText4->ShowWindow(SW_HIDE);
		return;
	}
	
	int nHeightMid = nHeightRemain / 2 + m_GrayImageBarRect.bottom;

	int x = m_GrayImageRect.left + 2 * WIDGET_APART_WIDTH;
	int y = nHeightMid - textRect.Height() - WIDGET_APART_HEIGHT;
	pText1->MoveWindow(x, y, textRect.Width(), textRect.Height());

	pText2->GetClientRect(textRect);
	y = nHeightMid + WIDGET_APART_HEIGHT;
	pText2->MoveWindow(x, y, textRect.Width(), textRect.Height());
	
	CRect textRect1;
	pText1->GetWindowRect(textRect);
	ScreenToClient(textRect);
	pText3->GetClientRect(textRect1);
	x = textRect.right + WIDGET_APART_WIDTH;
	y = textRect.top;
	pText3->MoveWindow(x, y, textRect1.Width(), textRect1.Height());

	pText4->GetClientRect(textRect1);
	pText2->GetWindowRect(textRect);
	ScreenToClient(textRect);
	x = textRect.right + WIDGET_APART_WIDTH;
	y = textRect.top;
	pText4->MoveWindow(x, y, textRect1.Width(), textRect1.Height());
}

void CISPControlDlg::InitGrayBarBitmap()
{
	if (!m_bDrawEnable) return;
	
	CFFScale ffScale;
	ffScale.SetAttribute(SWS_PF_YUV420P, SWS_PF_BGRA);

	BYTE * pYuv = new BYTE[m_GrayImageBarRect.Height() * m_GrayImageBarRect.Width() * 3 / 2];
	
	memset(pYuv, 128, m_GrayImageBarRect.Height() * m_GrayImageBarRect.Width() * 3 / 2);

	for (int i = 0; i < m_GrayImageBarRect.Height(); ++i) {
		for (int j = 0; j < m_GrayImageBarRect.Width(); ++j)
			pYuv[i * m_GrayImageBarRect.Width() + j] = j;
	}

	BYTE * pRGB = new BYTE[m_GrayImageBarRect.Height() * m_GrayImageBarRect.Width() * 4];
	BOOL bRet = ffScale.Scale(pYuv, m_GrayImageBarRect.Width(), m_GrayImageBarRect.Height(), 0, pRGB, 
		m_GrayImageBarRect.Width(), m_GrayImageBarRect.Height(), m_GrayImageBarRect.Width() * 4);
	
	bRet = m_GrayBarBitmap.CreateBitmap(m_GrayImageBarRect.Width(), m_GrayImageBarRect.Height(), 1, 32, pRGB);
	delete[] pRGB;
}

BOOL CISPControlDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CDC* pDC = GetDC();
	m_MemDC.CreateCompatibleDC(pDC);
	
	m_MemBitmap.CreateCompatibleBitmap(pDC, 500, 500);
	m_pOldMemBitmap = m_MemDC.SelectObject(&m_MemBitmap);
	ReleaseDC(pDC);

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	avcodec_register_all();
	av_lockmgr_register(av_lock_manager_cb);
	
	//start the monitor thread
	m_runThreadFlag = TRUE;
	m_bNeedJudgeDisConnWork = TRUE;
	m_MonitorThread = (HANDLE)_beginthreadex(NULL, 0, thread_begin, (LPVOID)this, 0, NULL);

	time_t t = time(0);
	struct tm * ptNow = NULL;

	char logInfoName[MAX_PATH] = {0};
	
	ptNow = localtime(&t);
	
	sprintf(logInfoName, "ISPinfo_%04d_%02d_%02d,%02d,%02d,%02d.log", 
		ptNow->tm_year + 1900, ptNow->tm_mon + 1, ptNow->tm_mday, ptNow->tm_hour, ptNow->tm_min, ptNow->tm_sec);

	freopen(logInfoName, "w+t", stderr);
	
	CRect WindowRect;
	GetWindowRect(&WindowRect);
	MoveWindow(WindowRect.left, WindowRect.top, WINDOW_WIDTH, WINDOW_HEIGHT);
	
	InitPreviewWindows();

	InitTreeCtrlPosition();
	HTREEITEM hRoot = m_TreeCtrl.InsertItem(TREE_ROOT_ITEM_NAME, TVI_ROOT, TVI_LAST);

	InitTabCtrlPosition();

	InitButtonPosition();

	InitTabPage();

	InitGrayImangeWindowRect();
	InitGrayBarBitmap();

	SetTimer(1, 1000, NULL);
	SetTimer(2, 1020, NULL);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CISPControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CISPControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		DrawGrayImageWindow();
		//CDialog::OnPaint();
	}
}

void CISPControlDlg::DrawGrayImageWindow()
{
	CPaintDC dc(this); // 用于绘制的设备上下文

	if (!m_bDrawEnable) return;

	//draw frame
	dc.DrawEdge(m_GrayImageWindowRect, EDGE_BUMP, BF_RECT);
	dc.Draw3dRect(m_GrayImageRect.left - 1, m_GrayImageRect.top - 1, 
		m_GrayImageRect.Width() + 2, m_GrayImageRect.Height() + 2, RGB(0, 0, 0), RGB(0, 0, 0));
	dc.Draw3dRect(m_GrayImageBarRect.left - 1, m_GrayImageBarRect.top - 1, 
		m_GrayImageBarRect.Width() + 2, m_GrayImageBarRect.Height() + 2, RGB(0, 0, 0), RGB(0, 0, 0));

	m_MemDC.FillSolidRect(0, 0, m_GrayImageRect.Width(), m_GrayImageRect.Height(), GetSysColor(COLOR_3DFACE));
	int oldBkMode = m_MemDC.SetBkMode(TRANSPARENT);
	
	//draw gray image
	Graphics graphics(m_MemDC);
	Pen gdiPen(Color(64, 64, 64));
	
	float yratio = 0.0;
	int ybase = m_GrayImageRect.Height();

	if (m_bLogImage) {
		yratio = m_GrayImageRect.Height() / (float)log10((float)(m_YMaxNum + 1));
	}else {
		yratio = m_GrayImageRect.Height() / (float)(m_YMaxNum + 1);
	}

	for (int i = 0; i < 256; ++i) {
		if (m_bLogImage) {
			graphics.DrawLine(&gdiPen, Point(i, m_GrayImageRect.Height()), 
								Point(i, (int)(ybase - log10((float)(1 + m_YCnt[i])) * yratio)));
		}else {
			graphics.DrawLine(&gdiPen, Point(i, m_GrayImageRect.Height()), 
								Point(i, (int)(ybase - m_YCnt[i] * yratio)));
		}
	}
	
	dc.BitBlt(m_GrayImageRect.left, m_GrayImageRect.top, m_GrayImageRect.Width(), m_GrayImageRect.Height(), &m_MemDC, 0, 0, SRCCOPY);
	
	//draw GrayBar bitmap
	CBitmap* pOldMemBitmap = m_MemDC.SelectObject(&m_GrayBarBitmap);
	dc.BitBlt(m_GrayImageBarRect.left, m_GrayImageBarRect.top, m_GrayImageBarRect.Width(), m_GrayImageBarRect.Height(), &m_MemDC, 0, 0, SRCCOPY);
	m_MemDC.SelectObject(pOldMemBitmap);
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CISPControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int CISPControlDlg::ServerDisConnect()
{
	if (m_pServerPreview == NULL) return -1;
	
	if (m_pServerPreview->IsDisConnect()) {
		int iRet = m_pServerPreview->Connect();
		if (iRet < 0) {
#ifdef WARN_ERROR_OUT
			char strIPAddr[MAX_IP_LEN] = {0};
			unsigned int nLen = MAX_IP_LEN;

			m_pServerPreview->GetServerIp(strIPAddr, &nLen);
			fprintf(stderr, "WARN::####Disconnet server : %s connect failed####\n", strIPAddr);
#endif
			m_pServerPreview->DisConnect();
			return 0;
		}

		iRet = m_pServerPreview->SendGetServerInfo();
		if (iRet < 0) {
#ifdef WARN_ERROR_OUT
			char strIPAddr[MAX_IP_LEN] = {0};
			unsigned int nLen = MAX_IP_LEN;

			m_pServerPreview->GetServerIp(strIPAddr, &nLen);
			fprintf(stderr, "WARN::####Disconnet server : %s Send Get Server Info failed####\n", strIPAddr);
#endif
			m_pServerPreview->DisConnect();
			return 0;
		}

		BOOL bIsRespond = FALSE;
		m_pServerPreview->GetServerRespondComplete(bIsRespond);
		int iAttemptOpenCnt = 0;
		
		while(!bIsRespond && iAttemptOpenCnt < ATTEMPT_OPEN_MAX) {
			Sleep(200);
			m_pServerPreview->GetServerRespondComplete(bIsRespond);
			++iAttemptOpenCnt;
		}

		if (iAttemptOpenCnt >= ATTEMPT_OPEN_MAX && !bIsRespond) {
#ifdef WARN_ERROR_OUT
			char strIPAddr[MAX_IP_LEN] = {0};
			unsigned int nLen = MAX_IP_LEN;

			m_pServerPreview->GetServerIp(strIPAddr, &nLen);
			fprintf(stderr, "WARN::####Disconnet server : %s Send Get Server Info time out####\n", strIPAddr);
#endif
			
			goto Next;
		}else if (bIsRespond) {
			IMAGE_SET stImageSet = {0};
			m_pServerPreview->GetServerImageSet(stImageSet);
			if ((stImageSet.nBrightness == 255) && 
				(stImageSet.nContrast == 255) && (stImageSet.nSaturation == 255)) { // the server don't want us to connect to it, because the server is connect limit was reached.
				AfxMessageBox(L"Number of the server connections has reached the upper limit, banned from the server connection!", 0, 0);
				
				CloseTheStream();

				m_pServerPreview->DisConnect();
				m_pServerPreview = NULL;
				m_strURL.clear();

				return -1;
			}
		}
	}
	
	//if (m_pClient == NULL) {
		//return 0;
	//}

	//string cstrURL;
	//m_pClient->GetURL(cstrURL);
	//if (cstrURL.empty()) {
Next:
	if (m_strURL.empty()) {
		AfxMessageBox(L"Do not achieve RTSP address is in the air from the server, this error will lead to reconnect failed and unable to play the video stream.", 0, 0);
#ifdef WARN_ERROR_OUT
		fprintf(stderr, "WARN::####can't get the play url from server, can't play the stream####\n");
#endif
		return -1;
	}

	//if (OpenTheStream(cstrURL.c_str(), FALSE) < 0) {
	if (OpenTheStream(m_strURL.c_str(), FALSE) < 0) {
#ifdef WARN_ERROR_OUT
		fprintf(stderr, "WARN::####open stream error!####\n");
#endif
		m_pServerPreview->DisConnect();
		return 0;	
	}

#ifdef WARN_ERROR_OUT
	char strIPAddr[MAX_IP_LEN] = {0};
	unsigned int nLen1 = MAX_IP_LEN;

	m_pServerPreview->GetServerIp(strIPAddr, &nLen1);
	fprintf(stderr, "WARN::####Disconnet server : %s connect success####\n", strIPAddr);
#endif

	return 0;
}

void CISPControlDlg::Monitor()
{
	BOOL bIsContinue = FALSE;

	while(TRUE) {
		if (!m_runThreadFlag) break;

		if (!m_bNeedJudgeDisConnWork) goto again;
		
		{//断线重连
			CAutoLock lock(&m_csForServerConnect);

			if (m_pServerPreview && m_pServerPreview->IsDisConnect()) {
#ifdef WARN_ERROR_OUT
				char strIPAddr[MAX_IP_LEN] = {0};
				unsigned int nLen = MAX_IP_LEN;

				m_pServerPreview->GetServerIp(strIPAddr, &nLen);
				fprintf(stderr, "WARN::####Disconnet server : %s start reconnect####\n", strIPAddr);
#endif
				ServerDisConnect();
			}
		}

again:
		Sleep(1000);//1 second
	}
}

void CISPControlDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	int nCruSel = m_TabCtrl.GetCurSel();
	ShowPage(nCruSel);
}

HBRUSH CISPControlDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}

void CISPControlDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
}

void CISPControlDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
}

void CISPControlDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CISPControlDlg::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);

	// TODO: 在此处添加消息处理程序代码
}

void CISPControlDlg::ShowPage(int iSelect)
{
	CDialog * aPage[TAB_PAGE_CNT] = {NULL};
	int i = 0;

	for (i = 0; i < TAB_PAGE_CNT; ++i) {
		if (i == 0) aPage[i] = (CDialog*)&m_pageEnable;
		else if (i == 1) aPage[i] = (CDialog*)&m_pageBB;
		else if (i == 2) aPage[i] = (CDialog*)&m_pageLens;
		else if (i == 3) aPage[i] = (CDialog*)&m_pageDemosaic;
		else if (i == 4) aPage[i] = (CDialog*)&m_pageFilter;
		else if (i == 5) aPage[i] = (CDialog*)&m_pageWB;
		else if (i == 6) aPage[i] = (CDialog*)&m_pageAWB;
		else if (i == 7) aPage[i] = (CDialog*)&m_pageCCorrect;
		else if (i == 8) aPage[i] = (CDialog*)&m_pageGamma;
		else if (i == 9) aPage[i] = (CDialog*)&m_pageBEnhance;
		else if (i == 10) aPage[i] = (CDialog*)&m_pageSaturation;
		else if (i == 11) aPage[i] = (CDialog*)&m_pageSpecEff;
		else if (i == 12) aPage[i] = (CDialog*)&m_pageRegister;
		else aPage[i] = (CDialog*)&m_pageRGBAvg;
	}
	
	for (i = 0; i < TAB_PAGE_CNT; ++i) {
		if (i == iSelect) {
			aPage[i]->EnableWindow(TRUE);
			aPage[i]->ShowWindow(SW_SHOW);
			continue;
		}

		aPage[i]->EnableWindow(FALSE);
		aPage[i]->ShowWindow(SW_HIDE);
	}
}

LRESULT CISPControlDlg::OnPageEnableChanegeMessage(WPARAM wParam, LPARAM lParam)
{
	int nPageID = GETMESSAGEID(lParam);
	BOOL bEnable = GETMESSAGEINFO(lParam);
	
	if (nPageID == PAGE_ID_ENABLE) {
		if (ECT_BB == wParam) m_pageBB.SetBBEnable(bEnable);
		else if (ECT_LENS == wParam) m_pageLens.SetLensEnable(bEnable);
		else if (ECT_DEMOSAIC == wParam) m_pageDemosaic.SetDemosaicEnable(bEnable);
		else if (ECT_RGBFILTER == wParam) m_pageFilter.SetRGBFEnable(bEnable);
		else if (ECT_UVFILTER == wParam) m_pageFilter.SetUVFEnable(bEnable);
		else if (ECT_DEFECTPIXEL == wParam) m_pageFilter.SetDFPEnable(bEnable);
		else if (ECT_WB == wParam) m_pageWB.SetWBEnable(bEnable);
		else if (ECT_AWB == wParam) m_pageAWB.SetAWBEnable(bEnable);
		else if (ECT_CCORRECT == wParam) m_pageCCorrect.SetCCorrectEnable(bEnable);
		else if (ECT_GAMMA == wParam) m_pageGamma.SetGammaEnable(bEnable);
		else if (ECT_BENHANCE == wParam)m_pageBEnhance.SetBEnhanceEnable(bEnable);
		else if (ECT_SATURATION == wParam) m_pageSaturation.SetSaturationEnable(bEnable);
		else if (ECT_SPECEFFEICT == wParam) m_pageSpecEff.SetSpecEffEnable(bEnable);
		else{
			fprintf(stderr, "Page Message is flag value unknown!\n");
			return -1;
		}
	}else {
		if (m_pageEnable.SetEnable(wParam, bEnable) < 0)
			fprintf(stderr, "Page Message from function page can't set to Enable page!\n");
	}

	return 0;
}

LRESULT CISPControlDlg::OnPageSubmissionMsg(WPARAM wParam, LPARAM lParam)
{
	int nPageID = GETMESSAGEID(lParam);
	UINT msg = GETMESSAGEINFO(lParam);

	SendIspParam(TRUE, nPageID, wParam);
	return 0;
}

void CISPControlDlg::OnNMRClickTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	POINT pos;
	*pResult = -1;

	CMenu menu, *pm;
	if (!menu.LoadMenu(IDR_MENU1)) {
		AfxMessageBox( L"Unable to load the menu!\n", 0, 0 );
		return;
	}

	pm = menu.GetSubMenu(0);
	GetCursorPos( &pos );
	pm->TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, this);

	*pResult = 0;
}

#define DEVICE_PREFIX	L"Device%d:"

void CISPControlDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	USES_CONVERSION;
	static int SearchCnt = 0;

	if (nIDEvent == 1) {
		if (m_bIsSearch && (SearchCnt < 3)) {
			m_Search.Search();
			++SearchCnt;
		}
	
		if (SearchCnt == 3) {
			SearchCnt = 0;
			m_bIsSearch = FALSE;
			int nCount = 0;
			if (!(nCount = m_Search.GetServerCount())) {
				AfxMessageBox(L"Not search any server!", 0, 0);
			}else {
				char strServerID[MAX_ID_LEN] = {0};
				unsigned int len = MAX_ID_LEN;
				STREAMMODE mode = STREAM_MODE_MAX;
				BOOL bFind = FALSE;
				HTREEITEM hRoot = m_TreeCtrl.GetRootItem();
				CString strServerIDShow;

				for (int i = 0; i < nCount; ++i) {
					IServer * pIServer;
					m_Search.GetServer(i, &pIServer);
					
					len = MAX_ID_LEN;
					ZeroMemory(strServerID, len);
					pIServer->GetServerID(strServerID, &len);
					
					strServerIDShow.Format(DEVICE_PREFIX, i);
					strServerIDShow.Append(A2W(strServerID));

					HTREEITEM hDevice = m_TreeCtrl.InsertItem(strServerIDShow, hRoot);
					m_TreeCtrl.SetItemData(hDevice, (DWORD_PTR)pIServer);
				}//for (int i = 0; i < nCount; ++i)
				
				AfxMessageBox(L"Success!", 0, 0);
				m_TreeCtrl.Expand(m_TreeCtrl.GetRootItem(), TVE_EXPAND);
			}//if (!(nCount = m_Search.GetServerCount())) else
		}//SearchCnt == 3
	}

	if (nIDEvent == 2) {
		CalcYuvCntFormeDisplayFrame();
		if (m_videoRender && m_bNeedUpdateRGBAvg) PostMessage(WM_CALC_RGBAVG);
	}

	CDialog::OnTimer(nIDEvent);
}

void CISPControlDlg::CalcYuvCntFormeDisplayFrame()
{
	if (!m_videoRender) return;

	int nYImageLen = 0, nYLineSize = 0, nYHeight = 0;

	m_videoRender->GetCurrentYuv420ImageY(NULL, nYImageLen, nYHeight, nYLineSize);
	
	if (!nYImageLen || !nYHeight || !nYLineSize) return;

	uint8_t * pYImage = new uint8_t[nYImageLen];
	if (!pYImage) return;

	if (m_videoRender->GetCurrentYuv420ImageY(pYImage, nYImageLen, nYHeight, nYLineSize) < 0) {
		delete[] pYImage;
		return;
	}

	ZeroMemory(m_YCnt, sizeof(m_YCnt));
	
	//int nTemp = 0;
	for (int i = 0; i < nYImageLen; ++i){
		//nTemp = (pYImage[i] - 128) * 1.164 + 128;
		
		//if (nTemp < 0) nTemp = 0;
		//if (nTemp > 255) nTemp = 255;

		m_YCnt[pYImage[i]] += 1;
	}
	
	m_YMaxNum = 0;
	for (int i = 0; i < 256; ++i) {
		if (m_YMaxNum < m_YCnt[i]) m_YMaxNum = m_YCnt[i];
	}

	delete[] pYImage;

	InvalidateRect(m_GrayImageRect, FALSE);
	
	double dAverage = 0.0;
	double p[256] = {0.0};
	for (int i = 0; i < 256; ++i) {
		p[i] = ((double)m_YCnt[i] / (double)(nYHeight * nYLineSize));
		dAverage += (i * p[i]);
	}
	
	double dWarp = 0.0;
	for (int i = 1; i < 256; ++i) {
		dWarp += (((double)i - dAverage) * ((double)i - dAverage) * p[i]);
	}
	
	CString strText;
	strText.Format(L"%0.7f", dAverage);
	GetDlgItem(IDC_STATIC_AVERAGE_VAL)->SetWindowText(strText);

	strText.Format(L"%0.7f", dWarp);
	GetDlgItem(IDC_STATIC_WARP_VAL)->SetWindowText(strText);
}

void CISPControlDlg::CalcRGBAvgInRect()
{
	if (!m_videoRender || !m_bNeedUpdateRGBAvg) return;
	
	int nYImageLen = 0, nYLineSize = 0, nYHeight = 0;
	int nUImageLen = 0, nULineSize = 0, nUHeight = 0;
	int nVImageLen = 0, nVLineSize = 0, nVHeight = 0;
	uint8_t * pYImage = NULL, * pUImage = NULL, * pVImage = NULL,
		* pRectRGB = NULL, *pDestRGB = NULL, * pPos = NULL;
	CRect rectTemp = m_DrawRect, rectPreview;
	
	m_videoRender->GetCurrentYuv420ImageYuv(NULL, nYImageLen, nYHeight, nYLineSize,
						NULL, nVImageLen, nVHeight, nVLineSize, NULL, nUImageLen, nUHeight, nULineSize);
	if (!nYImageLen || !nYHeight || !nYLineSize) return;
	if (!nVImageLen || !nVHeight || !nVLineSize) return;
	if (!nUImageLen || !nUHeight || !nULineSize) return;

	pYImage = new uint8_t[nYImageLen];
	if (!pYImage) goto end;

	pUImage = new uint8_t[nUImageLen];
	if (!pUImage) goto end;

	pVImage = new uint8_t[nVImageLen];
	if (!pVImage) goto end;
	
	if (m_videoRender->GetCurrentYuv420ImageYuv(pYImage, nYImageLen, nYHeight, nYLineSize, 
		pUImage, nUImageLen, nUHeight, nULineSize, pVImage, nVImageLen, nVHeight, nVLineSize) < 0) goto end;

	pDestRGB = new uint8_t[nYLineSize * nYHeight * 4];
	if (!pDestRGB) goto end;
	ZeroMemory(pDestRGB, nYLineSize * nYHeight * 4);

	YUV_TO_RGB32_s(pYImage, nYLineSize, pUImage, pVImage, nVLineSize, pDestRGB,
				 nYLineSize, nYHeight, nYLineSize * 4);
	
	m_Preview.GetWindowRect(&rectPreview);
	
	float wration =  nYLineSize / (float)rectPreview.Width();
	float hration = nYHeight / (float)rectPreview.Height();
		
	rectTemp.left *= wration;
	rectTemp.right *= wration;
	rectTemp.top *= hration;
	rectTemp.bottom *= hration;

	pRectRGB = new uint8_t[rectTemp.Height() * rectTemp.Width() * 4];
	if (!pRectRGB) goto end;

	ZeroMemory(pRectRGB, rectTemp.Height() * rectTemp.Width() * 4);
	
	int i = 0;
	for (; i < rectTemp.Height(); ++i) {
		memcpy(pRectRGB + (i * rectTemp.Width()) * 4, 
				pDestRGB + (((rectTemp.top + i) * nYLineSize) + rectTemp.left) * 4, rectTemp.Width() * 4);
	}
				 
	m_pageRGBAvg.SetWH(rectTemp.Width(), rectTemp.Height());
	m_pageRGBAvg.SetPageInfoSt(pRectRGB, rectTemp.Height() * rectTemp.Width() * 4);
	
end:
	if (pYImage) delete[] pYImage;
	if (pUImage) delete[] pUImage;
	if (pVImage) delete[] pVImage;
	if (pDestRGB) delete[] pDestRGB;
	if (pRectRGB) delete[] pRectRGB;
}

void CISPControlDlg::OnSearchDevice()
{
	// TODO: 在此添加命令处理程序代码
	m_TreeCtrl.DeleteAllItems();
	HTREEITEM hRoot = m_TreeCtrl.InsertItem( TREE_ROOT_ITEM_NAME, TVI_ROOT, TVI_LAST );

	{
		CAutoLock lock(&m_csForServerConnect);
		m_Search.DeleteAllServer();
		m_pServerPreview = NULL;
	}

	CloseTheStream();

	m_Search.Search();
	m_bIsSearch = TRUE;
}

void CISPControlDlg::OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = -1;
	
	POINT pos;
	if (!GetCursorPos( &pos ))
		return;
	
	m_TreeCtrl.ScreenToClient( &pos );

	UINT nFlag;
	HTREEITEM hItem = m_TreeCtrl.HitTest( pos, &nFlag );
	
	if ((hItem != NULL) && (TVHT_ONITEM & nFlag)) {
		m_TreeCtrl.Select( hItem, TVGN_CARET );
		if (!m_TreeCtrl.GetChildItem(hItem) && (TREE_ROOT_ITEM_NAME != m_TreeCtrl.GetItemText(hItem))) { // is the server node
			IServer * pIServer = (IServer *)m_TreeCtrl.GetItemData(hItem);
			const char * strURL = MakeRTSPUrl(pIServer);
			
			if (RegisterThePreviewServer(pIServer, strURL) < 0) return;

			if (OpenTheStream(strURL) < 0)
				UnregisterThePreviewServer(pIServer);
		}
	}

	*pResult = 0;
}

int CISPControlDlg::RegisterThePreviewServer(IServer * pIServer, const char * strURL)
{
	USES_CONVERSION;
	if (NULL == pIServer) return -1;
	
	if (m_pServerPreview == pIServer) {
		if (strcmp((m_strURL.c_str()), strURL)) {
			m_strURL.clear();
			m_strURL = strURL;
		}

		return 0; // already registered
	}
	
	CAutoLock lock(&m_csForServerConnect);

	if (m_pServerPreview) m_pServerPreview->DisConnect();

	if (pIServer->Connect() < 0) {
		WCHAR astrMsg[100] = {0};
		char strIPAddr[MAX_IP_LEN] = {0};
		unsigned int nLen = MAX_IP_LEN;

		pIServer->GetServerIp(strIPAddr, &nLen);

		_sntprintf(astrMsg, 100, L"Unable to connect to server%s", A2W(strIPAddr));
		AfxMessageBox( astrMsg, 0, 0 );
		return -1;
	}

	int iRet = pIServer->SendGetServerInfo();
	if (iRet < 0) {
		pIServer->DisConnect();
		return -1;
	}

	BOOL bIsRespond = FALSE;
	pIServer->GetServerRespondComplete(bIsRespond);
	int iAttemptOpenCnt = 0;
	
	while(!bIsRespond && iAttemptOpenCnt < ATTEMPT_OPEN_MAX) {
		Sleep(200);
		pIServer->GetServerRespondComplete(bIsRespond);
		++iAttemptOpenCnt;
	}

	 if (bIsRespond) {
		IMAGE_SET stImageSet = {0};
		pIServer->GetServerImageSet(stImageSet);
		if ((stImageSet.nBrightness == 255) && 
			(stImageSet.nContrast == 255) && (stImageSet.nSaturation == 255)) { // the server don't want us to connect to it, because the server is connect limit was reached.
			WCHAR astrMsg[100] = {0};
			char strIPAddr[MAX_IP_LEN] = {0};
			unsigned int nLen = MAX_IP_LEN;

			pIServer->GetServerIp(strIPAddr, &nLen);

			_sntprintf(astrMsg, 100, L"Number of the server connections has reached the upper limit, banned from the server connection!", A2W(strIPAddr));
			AfxMessageBox(astrMsg, 0, 0);
			return -1;
		}
	}
	
	m_pServerPreview = pIServer;
	m_pServerPreview->SetIspInfoParamCallBack(OnIspInfoParamRespond, this);
	m_pServerPreview->SetServerRetCallBack(OnServerReturnInfo, this);
	m_strURL.clear();
	m_strURL = strURL;

	return 0;
}

int CISPControlDlg::UnregisterThePreviewServer(IServer * pIServer)
{
	if (NULL == pIServer) return -1;

	if (m_pServerPreview == NULL) return 0; // already unregistered

	CAutoLock lock(&m_csForServerConnect);
	
	m_pServerPreview->DisConnect();
	
	m_pServerPreview->SetServerRetCallBack(NULL, NULL);
	m_pServerPreview->SetIspInfoParamCallBack(NULL, NULL);
	m_pServerPreview = NULL;
	m_strURL.clear();
	
	return 0;
}

char * CISPControlDlg::MakeRTSPUrl(IServer * pIServer)
{
	static char strURL[MAX_RTSP_URL_LEN] = { 0 };

	if (pIServer == NULL) return NULL;

	unsigned int iStreamSelect = 0, nCnt = 0, nPort = 0;

	pIServer->GetServerStreamCnt(nCnt);

	if (nCnt < 1) {
		AfxMessageBox( L"Current server without any available video stream!", 0, 0 );
		return NULL;
	}
	
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
	pIServer->GetServerStreamName(iStreamSelect, strStreamName, &len);
	strncat(strURL, strStreamName, len);

	return strURL;
}

int CISPControlDlg::CloseTheStream()
{
	CAutoLock lock(&m_csForOpenCloseStream);

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

#define MAX_WAIT_CNT	20

int CISPControlDlg::OpenTheStream(const char * strURL, BOOL bNeedENotify)
{
	USES_CONVERSION;

	WCHAR astrMsg[100] = {0};
	int iErrorCode = 0;
	
	if (strURL == NULL) {
		if (bNeedENotify)
			AfxMessageBox( L"Unable to open an empty RTSP address!", 0, 0 );

		return 0;
	}
	
	unsigned int iStreamChoose = 0, iSCnt = 0, len = MAX_STREAM_NAME;
	int iFps = 0;

	const char * pWhere = NULL;
	
	m_pServerPreview->GetServerStreamCnt(iSCnt);

	pWhere = strrchr(strURL, CHAR_SEPARATOR);
	if (pWhere == NULL) 
		iSCnt = 0;

	pWhere += 1;
	char strStreamName[MAX_STREAM_NAME] = {0};

	for (iStreamChoose = 0; iStreamChoose < iSCnt; ++iStreamChoose) {
		len = MAX_STREAM_NAME;
		ZeroMemory(strStreamName, MAX_STREAM_NAME * sizeof(char));

		if (m_pServerPreview->GetServerStreamName(iStreamChoose, strStreamName, &len) < 0)
			continue;
		
		if (strcmp(pWhere, strStreamName) == 0)
			break;
	}
	
	if (iStreamChoose < iSCnt)
		m_pServerPreview->GetStreamFps(iStreamChoose, iFps);
	else
		iFps = 30;

	CAutoLock lock(&m_csForOpenCloseStream);
	
	if (m_pClient != NULL)	CloseTheStream();

	m_pClient = CAimer39RTSPClient::CreateNew();
	if (NULL == m_pClient) {
		if (bNeedENotify)
			AfxMessageBox( L"Unable to get device ... serious internal error!", 0, 0 );
		return -1;
	}

	m_pClient->RegisterFinishCallback(OnClientFinish, this);
	m_pClient->RegisterDisConnCallback(OnClientDisConnect, this);

	iErrorCode = m_pClient->OpenURL(strURL);
	if (iErrorCode < 0) {
		if (bNeedENotify) {
			_sntprintf(astrMsg, 100, L"OpenURL %s error! error = %s", A2W(strURL), A2W(m_pClient->GetLastError()));
			AfxMessageBox( astrMsg, 0, 0 );
		}
		return -1;
	}
	
	int nWaitCnt = 0;
	bool isPrepare = false;
	while ( !isPrepare ) {
		iErrorCode = m_pClient->IsPrepare( isPrepare );
		if ((iErrorCode != 0) || (nWaitCnt >= MAX_WAIT_CNT)) {
			if (bNeedENotify) {
				if ((iErrorCode == 0) && (nWaitCnt >= MAX_WAIT_CNT)) 
					_sntprintf(astrMsg, 100, L"Connect to the server % s, timeout!", A2W(strURL));
				else
					_sntprintf(astrMsg, 100, L"OpenURL %s error! error = %s", A2W(strURL), A2W(m_pClient->GetLastError()));
				
				AfxMessageBox( astrMsg, 0, 0 );
			}

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

			m_videoRender->OpenRender(m_Preview.m_hWnd);
			m_videoRender->SetFillMode(KeepAspectRatio);
			m_videoRender->SetServerStreamFps(iFps);
			m_videoDecoder->OpenFfmpeg();
			
			m_pClient->RegisterSink(type, m_videoDecoder);
			m_videoDecoder->RegisterSink(m_videoRender, SINK_VIDEO);
		}
	}
	
	m_pClient->Play();
	
	if (m_videoDecoder)
		m_videoDecoder->Start();

	return 0;
}

#define PAGE_INFO_MAX_LEN	8192
#define COMMAND_SLEEP_MAX	100

void CISPControlDlg::OnBnClickedButtonSubmission()
{
	// TODO: 在此添加控件通知处理程序代码
	SendIspParam();
}

void CISPControlDlg::SendIspParam(BOOL bNeedJudge, int nPageID, int nFlag)
{
	if (m_pServerPreview == NULL || m_pServerPreview->IsDisConnect()) {
		AfxMessageBox(L"No connection with any device, or the connection has been disconnected, unable to set the ISP parameters!\n", 0, 0);
		return;
	}
	
	BYTE aPageInfo[PAGE_INFO_MAX_LEN] = {0};
	int nLen = PAGE_INFO_MAX_LEN, ret = 0;
	
	while ((bNeedJudge && (nPageID == PAGE_ID_LENS)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageLens.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Parameter Settings in the camera calibration error, do not send it to the server.");
			break;
		}

		ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
		if (ret < 0) fprintf(stderr, "WARN! send lens page is info error! ret = %d\n", ret);
		Sleep(COMMAND_SLEEP_MAX);
		break;
	}
	
	while ((bNeedJudge && (nPageID == PAGE_ID_AWB)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageAWB.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Automatic white balance setting parameter error, do not send it to the server.");
			break;	
		}

		ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
		if (ret < 0) fprintf(stderr, "WARN! send AWB page is info error! ret = %d\n", ret);
		Sleep(COMMAND_SLEEP_MAX);
		break;
	}
	
	while ((bNeedJudge && (nPageID == PAGE_ID_BB)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageBB.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Automatic white balance setting parameter error, do not send it to the server.");
			break;			
		}

		ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
		if (ret < 0) fprintf(stderr, "WARN! send BB page is info error! ret = %d\n", ret);
		Sleep(COMMAND_SLEEP_MAX);
		break;	
	}
	
	while ((bNeedJudge && (nPageID == PAGE_ID_BENHANCE)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageBEnhance.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Boundary enhanced setting parameter error, do not send it to the server.");
			break;		
		}

		ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
		if (ret < 0) fprintf(stderr, "WARN! send BEnhance page is info error! ret = %d\n", ret);
		Sleep(COMMAND_SLEEP_MAX);
		break;
	}
	
	while ((bNeedJudge && (nPageID == PAGE_ID_CCORRECT)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageCCorrect.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"颜色校正中设置的参数错误，不发送此设置到服务器端。");
			break;			
		}

		ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
		if (ret < 0) fprintf(stderr, "WARN! send CCorrect page is info error! ret = %d\n", ret);
		Sleep(COMMAND_SLEEP_MAX);
		break;
	}
	
	while ((bNeedJudge && (nPageID == PAGE_ID_DEMOSAIC)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageDemosaic.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Color correction parameter settings error, do not send it to the server.");
			break;			
		}

		ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
		if (ret < 0) fprintf(stderr, "WARN! send Demosaic page is info error! ret = %d\n", ret);
		Sleep(COMMAND_SLEEP_MAX);
		break;
	}
	
	while ((bNeedJudge && (nPageID == PAGE_ID_FILTER)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));

		while ((bNeedJudge && (nFlag == ECT_RGBFILTER)) || !bNeedJudge) {
			if (m_pageFilter.GetPageInfoByIndex(nPageID, aPageInfo, nLen, PAGE_INFO_RGBFILTER) < 0) {
				AfxMessageBox(L"RGB Filter parameter settings error, do not send it to the server.");
				break;	
			}

			ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
			if (ret < 0) fprintf(stderr, "WARN! send Filter page is rgb info error! ret = %d\n", ret);
			Sleep(COMMAND_SLEEP_MAX);
			break;
		}

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		while ((bNeedJudge && (nFlag == ECT_UVFILTER)) || !bNeedJudge) {
			if (m_pageFilter.GetPageInfoByIndex(nPageID, aPageInfo, nLen, PAGE_INFO_UVFILTER) < 0) {
				AfxMessageBox(L"UV Filter parameter settings error, do not send it to the server.");
				break;		
			}

			ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
			if (ret < 0) fprintf(stderr, "WARN! send Filter page is uv info error! ret = %d\n", ret);
			Sleep(COMMAND_SLEEP_MAX);
			break;
		}

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		while ((bNeedJudge && (nFlag == ECT_DEFECTPIXEL)) || !bNeedJudge) {
			if (m_pageFilter.GetPageInfoByIndex(nPageID, aPageInfo, nLen, PAGE_INFO_DFPDEFECT) < 0) {
				AfxMessageBox(L"Dead pixel detection parameter setting error, do not send it to the server.");
				break;
			}

			ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
			if (ret < 0) fprintf(stderr, "WARN! send Filter page is Dfp defect info error! ret = %d\n", ret);
			Sleep(COMMAND_SLEEP_MAX);
			break;
		}

		break;
	}
	
	if ((bNeedJudge && (nPageID == PAGE_ID_GAMMA)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		for(int i = 0; i < 2; ++i) {
			if (m_pageGamma.GetPageInfoStIndex(nPageID, aPageInfo, nLen, i) < 0) {
				AfxMessageBox(L"Gamma parameter setting error, do not send it to the server.");
				break;
			}

			ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
			if (ret < 0) fprintf(stderr, "WARN! send Gamma page is info error!index = %d, ret = %d\n", i, ret);	
			Sleep(COMMAND_SLEEP_MAX);
		}
	}
	
	while ((bNeedJudge && (nPageID == PAGE_ID_SATURATION)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageSaturation.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Saturation parameter setting error, do not send it to the server.");
			break;			
		}

		ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
		if (ret < 0) fprintf(stderr, "WARN! send Saturation page is info error! ret = %d\n", ret);
		Sleep(COMMAND_SLEEP_MAX);
		break;
	}
	
	while ((bNeedJudge && (nPageID == PAGE_ID_SPECEFF)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageSpecEff.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Effects Processing parameter setting error, do not send it to the server.");
			break;		
		}

		ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
		if (ret < 0) fprintf(stderr, "WARN! send SpecEff page is info error! ret = %d\n", ret);
		Sleep(COMMAND_SLEEP_MAX);
		break;
	}
	
	while ((bNeedJudge && (nPageID == PAGE_ID_WB)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageWB.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Black Balance parameter setting error, do not send it to the server.");
			break;			
		}

		ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
		if (ret < 0) fprintf(stderr, "WARN! send WB page is info error! ret = %d\n", ret);
		break;
	}

	while ((bNeedJudge && (nPageID == PAGE_ID_REGISTER)) || !bNeedJudge){
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		int nCount = m_pageRegister.GetPageInfoStCount();
		for (int i = 0; i < nCount; ++i) {
			if (m_pageRegister.GetPageInfoSent(nPageID, aPageInfo, nLen) < 0) {
				AfxMessageBox(L"Sensor Register parameter setting error, do not send it to the server.");
				break;
			}

			ret = m_pServerPreview->SendISPCommand(aPageInfo, nLen);
			if (ret < 0) fprintf(stderr, "WARN! send lens page is info error! ret = %d\n", ret);
			Sleep(COMMAND_SLEEP_MAX);
		}

		break;
	}
}

void CISPControlDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_runThreadFlag = FALSE;
	WiatForMonitorThreadEnd();

	CloseTheStream();

	CoUninitialize();

	av_lockmgr_register(NULL);
	KillTimer(1);
	KillTimer(2);

	WSACleanup();

	CDialog::OnClose();
}

#define ISP_FILE_HEADER_LEN		8
#define ISP_INDEXS_LEN			132

typedef struct ISPFileIndex
{
	int enable;
	int nDataLen;
	int nIndex; //file index
}ISPINDEX;

void CISPControlDlg::WriteIspFileHeader(CFile & IspFile)
{
	DWORD dwTag = MAKE_SYSTEM_TAG(SYSTEM_TAG);
	IspFile.Write(&dwTag, sizeof(DWORD));
	
	DWORD dwIndexCnt = SAVE_PAGE_CNT;
	IspFile.Write(&dwIndexCnt, 4);
	
	ISPINDEX astIndex[SAVE_PAGE_CNT] = {0}; //init the Index
	
	IspFile.Write(astIndex, sizeof(astIndex));
}

void CISPControlDlg::WriteIspIndexContent(CFile & IspFile, int nIndex, int enable, int nLen, int nFileIndex)
{
	int Position = IspFile.GetPosition();
	IspFile.Seek((ISP_FILE_HEADER_LEN + nIndex * sizeof(ISPINDEX)), CFile::begin);

	ISPINDEX stIndex = {-1};
	stIndex.enable = 1;
	stIndex.nDataLen = nLen;
	stIndex.nIndex = nFileIndex;

	IspFile.Write(&stIndex, sizeof(ISPINDEX));
	IspFile.Seek(Position, CFile::begin);
}

void CISPControlDlg::WriteEnablePageInfo2IspFile(CFile & IspFile)
{
	int nIndex = 0;
	BYTE aPageInfo[PAGE_INFO_MAX_LEN] = {0};
	int nLen = PAGE_INFO_MAX_LEN, nPageID = PAGE_ID_BB, ret = 0;

	CBasePage * aPage[SAVE_PAGE_CNT] = {NULL};
	int i = 0;

	for (; i < SAVE_PAGE_CNT; ++i) {
		if (i == 0) aPage[i] = (CBasePage*)&m_pageBB;
		else if (i == 1) aPage[i] = (CBasePage*)&m_pageLens;
		else if (i == 2) aPage[i] = (CBasePage*)&m_pageDemosaic;
		else if (i == 3) aPage[i] = (CBasePage*)&m_pageFilter;
		else if (i == 4) aPage[i] = (CBasePage*)&m_pageWB;
		else if (i == 5) aPage[i] = (CBasePage*)&m_pageAWB;
		else if (i == 6) aPage[i] = (CBasePage*)&m_pageCCorrect;
		else if (i == 7) aPage[i] = (CBasePage*)&m_pageGamma;
		else if (i == 8) aPage[i] = (CBasePage*)&m_pageBEnhance;
		else if (i == 9) aPage[i] = (CBasePage*)&m_pageSaturation;
		else if (i == 10) aPage[i] = (CBasePage*)&m_pageSpecEff;
		else if (i == 11) aPage[i] = (CBasePage*)&m_pageRegister;
		else aPage[i] = NULL;
	}

	for (i = 0; i < SAVE_PAGE_CNT; ++i) {
		if (!aPage[i]) continue;

		if (aPage[i]->GetPageEnable()) {
			nLen = PAGE_INFO_MAX_LEN;

			if (i == 7) {
				FILE * file;
				file = fopen("Gamma.log", "w");
				if (file == NULL) goto again;

				for (int j = 0; j < 2; ++j) {
					m_pageGamma.GetPageInfoStIndex(nPageID, aPageInfo, nLen, j);
					GAMMACALC * pSt = (GAMMACALC *)aPageInfo;
					for (int k = 0; k < 32; ++k) {
						if (!(k % 8)) fprintf(file, "\n");
						fprintf(file, "0x%08x, ", pSt->gamma[k]);
					}
				}

				fclose(file);
				nLen = PAGE_INFO_MAX_LEN;
			}

again:
			if(i == 5)
				aPage[i]->GetPageInfoStAll(nPageID, aPageInfo, nLen);
			else
				aPage[i]->GetPageInfoSt(nPageID, aPageInfo, nLen);
			
			int Positon = IspFile.GetPosition(); // from file begin;
			IspFile.Write(&nPageID, sizeof(int));
			nLen += sizeof(int);
			IspFile.Write(aPageInfo, nLen);
			
			WriteIspIndexContent(IspFile, nIndex, 1, nLen, Positon);
			++nIndex;
		}
	}
}

void CISPControlDlg::OnBnClickedButtonOut()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strIspFile, strError;

	CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"ISP Files (*.iif)|*.iif||", NULL);

	if (dlg.DoModal() == IDOK) {
		strIspFile = dlg.GetPathName();
	}else return;

	CString strTemp = strIspFile.Right(4);
	if (strTemp != L".iif")
		strIspFile.Append(L".iif");
	
	CFileException fileException;
	CFile file;
	if (!file.Open(strIspFile, 
			CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary | CFile::shareExclusive, 
			&fileException)) {
		strError.Format(L"Error % u!Unable to open file % s!", fileException.m_cause, strIspFile);
		AfxMessageBox(strError, 0, 0);
		return;
	}

	WriteIspFileHeader(file);
	WriteEnablePageInfo2IspFile(file);
	file.Flush();
	file.Close();
}

int CISPControlDlg::ReadIspFileHeader(CFile & IspFile, DWORD & dwTag, DWORD & dwIndexCnt)
{
	IspFile.Seek(0, CFile::begin);

	IspFile.Read(&dwTag, sizeof(DWORD));
	IspFile.Read(&dwIndexCnt, sizeof(DWORD));

	return 0;
}

int CISPControlDlg::ReadIspIndex(CFile & IspFile, int nIndex, int & enable, int & nLen, int & nFileIndex)
{
	IspFile.Seek(ISP_FILE_HEADER_LEN + nIndex * sizeof(ISPINDEX), CFile::begin);

	ISPINDEX stIndex = {0};

	IspFile.Read(&stIndex, sizeof(ISPINDEX));

	enable = stIndex.enable;
	nLen = stIndex.nDataLen;
	nFileIndex = stIndex.nIndex;
	
	return 0;
}

int CISPControlDlg::ReadIspContent(CFile & IspFile, int nFileIndex, int nLen, int & nPageID, void * aPageStInfo, int & nInfoLen)
{
	//ID is int type.
	if ((aPageStInfo == NULL) || (nInfoLen < (nLen - sizeof(int)))) return -1;

	IspFile.Seek(nFileIndex, CFile::begin);
	IspFile.Read(&nPageID, sizeof(int));

	IspFile.Read(aPageStInfo, nLen - sizeof(int));
	nInfoLen = nLen - sizeof(int);

	return 0;
}

void CISPControlDlg::OnBnClickedButtonIn()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strIspFile, strError;

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"ISP Files (*.iif)|*.iif||", NULL);

	if (dlg.DoModal() == IDOK) {
		strIspFile = dlg.GetPathName();
	}else return;

	CFileException fileException;
	CFile file;
	if (!file.Open(strIspFile, CFile::modeRead | CFile::typeBinary | CFile::shareExclusive, &fileException)) {
		strError.Format(L"Error % u!Unable to open file % s!", fileException.m_cause, strIspFile);
		AfxMessageBox(strError, 0, 0);
		return;
	}
	
	DWORD dwTag = 0, dwIndexCnt = 0;
	ReadIspFileHeader(file, dwTag, dwIndexCnt);

	if (dwTag != MAKE_SYSTEM_TAG(SYSTEM_TAG) || dwIndexCnt != SAVE_PAGE_CNT) {
		strError.Format(L"The file % s, not the ISP configuration file!", strIspFile);
		AfxMessageBox(strError, 0, 0);

		file.Close();
		return;
	}

	CBasePage * aPage[SAVE_PAGE_CNT] = {NULL};
	int i = 0;

	for (; i < SAVE_PAGE_CNT; ++i) { //array index = PAGE_ID_* - 1;
		if (i == 0) aPage[i] = (CBasePage*)&m_pageBB;
		else if (i == 1) aPage[i] = (CBasePage*)&m_pageLens;
		else if (i == 2) aPage[i] = (CBasePage*)&m_pageDemosaic;
		else if (i == 3) aPage[i] = (CBasePage*)&m_pageFilter;
		else if (i == 4) aPage[i] = (CBasePage*)&m_pageWB;
		else if (i == 5) aPage[i] = (CBasePage*)&m_pageAWB;
		else if (i == 6) aPage[i] = (CBasePage*)&m_pageCCorrect;
		else if (i == 7) aPage[i] = (CBasePage*)&m_pageGamma;
		else if (i == 8) aPage[i] = (CBasePage*)&m_pageBEnhance;
		else if (i == 9) aPage[i] = (CBasePage*)&m_pageSaturation;
		else if (i == 10) aPage[i] = (CBasePage*)&m_pageSpecEff;
		else if (i == 11) aPage[i] = (CBasePage*)&m_pageRegister;
		else aPage[i] = NULL;
	}
	
	int enable = 0, nLen = 0, nFileIndex = 0, nDataLen = 0, nInfoLen = 0, nPageID= 1;
	BYTE aPageInfo[PAGE_INFO_MAX_LEN] = {0};

	for (int i = 0; i < SAVE_PAGE_CNT; ++i) 
	{
		if (!aPage[i]) continue;

		ReadIspIndex(file, i, enable, nLen, nFileIndex);

		if (enable && nLen && (nFileIndex >= (ISP_FILE_HEADER_LEN + ISP_INDEXS_LEN))) 
		{
			nInfoLen = PAGE_INFO_MAX_LEN;
			ReadIspContent(file, nFileIndex, nLen, nPageID, aPageInfo, nInfoLen);

			if(nPageID == 6)
			{
				
				if (aPage[nPageID - 1]->SetPageInfoStAll((void*)aPageInfo, nInfoLen) < 0) {
				strError.Format(L"Settings Page (PageID =% d), error!", strIspFile);
				AfxMessageBox(strError, 0, 0);
				}
				
			}
			else
			{
				if (aPage[nPageID - 1]->SetPageInfoSt((void*)aPageInfo, nInfoLen) < 0) {
					strError.Format(L"Settings Page (PageID =% d), error!", strIspFile);
					AfxMessageBox(strError, 0, 0);
				}
			}
		}
	}

	file.Close();
}

void CISPControlDlg::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CISPControlDlg::PreTranslateMessage(MSG * pMsg)
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

void CISPControlDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CISPControlDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect;
	m_Preview.GetWindowRect(&rect);
	ScreenToClient(&rect);
	
	do {
		if (!rect.PtInRect(point)) break;

		FullScreenProcess(TRUE);
	}while (0);

	CDialog::OnLButtonDblClk(nFlags, point);
}

void CISPControlDlg::FullScreenProcess(BOOL bIsFullScreen)
{
	CAutoLock lock(&m_csForServerConnect);
	
	if (m_videoRender == NULL) return;

	if (bIsFullScreen) {
		if (m_pServerPreview->IsDisConnect()) return;

		int ret = m_videoRender->FullScreen(TRUE, OnFullScreenMessage, (void *)(this));
		if (ret < 0) return;
	}else{
		m_videoRender->FullScreen(FALSE, NULL, NULL);
	}
}

void CISPControlDlg::OnFullScreenMessage(UINT message, WPARAM wParam, LPARAM lParam, void * pClassParam)
{
	CISPControlDlg * pthis = (CISPControlDlg *)pClassParam;

	if (message == WM_LBUTTONDBLCLK ||
		(message == WM_KEYUP && wParam == VK_ESCAPE)) {
		pthis->FullScreenProcess(FALSE);
	}
}

#define DEFAULT_COLOR	D3DCOLOR_XRGB(8, 46, 84)
#define LINE_WIDTH		2

void CISPControlDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect = 0;
	m_Preview.GetWindowRect(&rect);
	ScreenToClient(&rect);

	if (rect.PtInRect(point)) {
		m_bNeedUpdateRGBAvg = FALSE;
		ClientToScreen(&point);
		m_Preview.ScreenToClient(&point);
		m_DrawRect.left = point.x;
		m_DrawRect.top = point.y;
		m_bStartDraw = TRUE;
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CISPControlDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect = 0;
	m_Preview.GetWindowRect(&rect);
	ScreenToClient(&rect);

	if (rect.PtInRect(point) && m_bStartDraw && m_videoRender) { 
		ClientToScreen(&point);
		m_Preview.ScreenToClient(&point);		
		m_DrawRect.bottom = point.y;
		m_DrawRect.right = point.x;

		m_DrawRect.NormalizeRect();

		if (m_DrawRect.Height() % 2 != 0) m_DrawRect.bottom += 1;
		if (m_DrawRect.Width() % 2 != 0) m_DrawRect.right += 1;

		if (rect.bottom < m_DrawRect.bottom) m_DrawRect.bottom = rect.bottom;
		if (rect.right < m_DrawRect.right) m_DrawRect.right = rect.right;
		
		m_videoRender->DrawRectangle(0, m_DrawRect, LINE_WIDTH, DEFAULT_COLOR);

		if (!m_DrawRect.IsRectEmpty()) m_bNeedUpdateRGBAvg = TRUE;
		else m_bNeedUpdateRGBAvg = FALSE;

		m_bStartDraw = FALSE;
	}

	CDialog::OnLButtonUp(nFlags, point);
}

void CISPControlDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rect = 0;
	m_Preview.GetWindowRect(&rect);
	ScreenToClient(&rect);

	if (nFlags == 1) { // Left button down
		if (m_videoRender && m_bStartDraw && rect.PtInRect(point)) {
			m_bNeedUpdateRGBAvg = FALSE;
			ClientToScreen(&point);
			m_Preview.ScreenToClient(&point);
			
			m_DrawRect.bottom = point.y;
			m_DrawRect.right = point.x;
			
			m_videoRender->DrawRectangle(0, m_DrawRect, LINE_WIDTH, DEFAULT_COLOR);
		}else if (m_videoRender && m_bStartDraw && (!rect.PtInRect(point))) {
			ClientToScreen(&point);
			m_Preview.ScreenToClient(&point);

			m_Preview.GetWindowRect(&rect);
			m_Preview.ScreenToClient(&rect);

			if (point.y < 0) point.y = 0;
			if (point.x < 0) point.x = 0;

			if (point.x >= rect.right - 2) point.x = rect.right;
			if (point.y >= rect.bottom - 2) point.y = rect.bottom;

			m_DrawRect.bottom = point.y;
			m_DrawRect.right = point.x;
			
			m_DrawRect.NormalizeRect();

			if (m_DrawRect.Height() % 2 != 0) m_DrawRect.bottom += 1;
			if (m_DrawRect.Width() % 2 != 0) m_DrawRect.right += 1;

			if (rect.bottom < m_DrawRect.bottom) m_DrawRect.bottom = rect.bottom;
			if (rect.right < m_DrawRect.right) m_DrawRect.right = rect.right;
			
			m_videoRender->DrawRectangle(0, m_DrawRect, LINE_WIDTH, DEFAULT_COLOR);

			if (!m_DrawRect.IsRectEmpty()) m_bNeedUpdateRGBAvg = TRUE;
			else m_bNeedUpdateRGBAvg = FALSE;

			m_bStartDraw = FALSE;
		}
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CISPControlDlg::OnImageCalcTypeLog()
{
	// TODO: 在此添加命令处理程序代码
	m_bLogImage = TRUE;
}

void CISPControlDlg::OnImageCalcTypeNormal()
{
	// TODO: 在此添加命令处理程序代码
	m_bLogImage = FALSE;
}

void CISPControlDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (m_GrayImageWindowRect.PtInRect(point)) {
		CMenu menu, *pm;
		if (!menu.LoadMenu(IDR_MENU2)) {
			AfxMessageBox( L"Unable to load the menu!\n", 0, 0 );
			return;
		}

		pm = menu.GetSubMenu(0);
		if (m_bLogImage) {
			pm->CheckMenuItem(ID_IMAGECALCTYPE_log, MF_BYCOMMAND | MF_CHECKED);
			pm->CheckMenuItem(ID_IMAGECALCTYPE_NORMAL, MF_BYCOMMAND | MF_UNCHECKED);
		}else {
			pm->CheckMenuItem(ID_IMAGECALCTYPE_NORMAL, MF_BYCOMMAND | MF_CHECKED);
			pm->CheckMenuItem(ID_IMAGECALCTYPE_log, MF_BYCOMMAND | MF_UNCHECKED);
		}
		
		ClientToScreen(&point);
		pm->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
		ScreenToClient(&point);
	}

	CDialog::OnRButtonDown(nFlags, point);
}

LRESULT CISPControlDlg::OnCalcRGBAvgMsg(WPARAM wParam, LPARAM lParam)
{
	int nCruSel = m_TabCtrl.GetCurSel();
	if (nCruSel != PAGE_ID_RGBAVG) return 0;

	CalcRGBAvgInRect();

	return 0;
}

LRESULT CISPControlDlg::OnPageGetInfoMsg(WPARAM wParam, LPARAM lParam)
{
	int nPageID = GETMESSAGEID(lParam);
	UINT msg = GETMESSAGEINFO(lParam);

	if (m_pServerPreview == NULL || m_pServerPreview->IsDisConnect()) {
		AfxMessageBox(L"No connection with any device, or the connection has been disconnected, unable to set the SP parameters!\n", 0, 0);
		return -1;
	}
	
	int ret = m_pServerPreview->SendGetIspInfo(msg);

	return 0;
}

void CISPControlDlg::WiatForMonitorThreadEnd()
{
	DWORD result;
	MSG msg;

	while(TRUE) {
		result = MsgWaitForMultipleObjects(1, &m_MonitorThread, FALSE, INFINITE, QS_ALLINPUT);
		if (result == WAIT_OBJECT_0)
			break;
		else {
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			DispatchMessage(&msg);
		}
	}
}

void CISPControlDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码生成头文件
	CString strHFile, strError;
	
	CString strDefName = L"isp_param.h";
	CFileDialog dlg(FALSE, NULL, strDefName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		L"ISP Files (*.h)|*.h||", NULL);

	if (dlg.DoModal() == IDOK) {
		strHFile = dlg.GetPathName();
	}else return;

	CString strTemp = strHFile.Right(2);
	if (strTemp != L".h")
		strHFile.Append(L".h");

	if (strDefName != strHFile.Right(strDefName.GetLength())) {
		AfxMessageBox(L"The header file name is not isp_param.h, the kernel uses only isp_param.h, and the kernel needs to manually modify the isp_param.h!", 0, 0);
	}

	CFileException fileException;
	CFile file;
	if (!file.Open(strHFile, 
		CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive, 
			&fileException)) {
		strError.Format(L"Error % u!Unable to open file % s!", fileException.m_cause, strHFile);
		AfxMessageBox(strError, 0, 0);
		return;
	}
	
	BYTE aPageInfo[PAGE_INFO_MAX_LEN] = {0};
	int nLen = PAGE_INFO_MAX_LEN, ret = 0;
	int nPageID = 0;

	while (TRUE) {
		char * pStr = NULL;
		GenerateHFileHead(&pStr);
		if (pStr == NULL) break;
		
		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;
		
		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		/*
		if (m_pageAWB.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"自动白平衡中设置的参数错误。");
			break;	
		}
		*/
		if (m_pageAWB.GetPageInfoStAll(nPageID, aPageInfo, nLen) <= 0) {
			AfxMessageBox(L"Automatic white balance setting parameter error.");
			break;	
		}

		GenerateAWBHFileDefine((AWBINFO *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageBB.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"white balance setting parameter error.");
			break;			
		}

		GenerateBlackBalanceHFileDefine((BLACKBALANCE *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageBEnhance.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Boundary enhanced setting parameter error.");
			break;		
		}

		GenerateBrightnessEnhHFileDefine((BRIENHANCE *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageCCorrect.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Color Correction setting parameter error.");
			break;			
		}
		GenerateColorCorrectHFileDefine((CCORRECT *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageDemosaic.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Color correction parameter settings error.");
			break;			
		}

		GenerateDemosaicHFileDefine((DEMOSAIC *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageFilter.GetPageInfoByIndex(nPageID, aPageInfo, nLen, PAGE_INFO_RGBFILTER) < 0) {
			AfxMessageBox(L"RGB Filter parameter settings error.");
			break;	
		}

		GenerateRGBFilterHFileDefine((RGBFILTER *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageFilter.GetPageInfoByIndex(nPageID, aPageInfo, nLen, PAGE_INFO_UVFILTER) < 0) {
			AfxMessageBox(L"UV Filter parameter settings error");
			break;		
		}

		GenerateUVFilterHFileDefine((UVFILTER *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageFilter.GetPageInfoByIndex(nPageID, aPageInfo, nLen, PAGE_INFO_DFPDEFECT) < 0) {
			AfxMessageBox(L"Dead pixel detection parameter setting error.");
			break;
		}

		GenerateDFPDefectHFileDefine((DFPDEFECT *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		GAMMACALC gcInfo[2];
		GAMMACALC * pgcInfo[2] = {&gcInfo[0], &gcInfo[1]};

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		for(int i = 0; i < 2; ++i) {
			if (m_pageGamma.GetPageInfoStIndex(nPageID, aPageInfo, nLen, i) < 0) {
				AfxMessageBox(L"Gamma parameter setting error.");
				break;
			}

			memcpy(&gcInfo[i], aPageInfo, sizeof(GAMMACALC));
		}

		GenerateGammaHFileDefine(pgcInfo, 2, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageLens.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Lens calibration parameter setting error.");
			break;
		}

		GenerateLensHFileDefine((LENSCORRECT *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageSaturation.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Saturation parameter setting error.");
			break;			
		}

		GenerateSaturationHFileDefine((SATURATION *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageSpecEff.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Effects Processing parameter settings error, do not send it to the server.");
			break;		
		}

		GenerateSpecEffectHFileDefine((SPECIALEFF *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		nLen = PAGE_INFO_MAX_LEN;
		ZeroMemory(aPageInfo, sizeof(aPageInfo));
		if (m_pageWB.GetPageInfoSt(nPageID, aPageInfo, nLen) < 0) {
			AfxMessageBox(L"Black Balance parameter settings error, do not send it to the server.");
			break;			
		}

		GenerateWhiteBalanceHFileDefine((WBALANCE *)aPageInfo, &pStr);
		if (pStr == NULL) break;

		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		GenerateHeadFileEnd(&pStr);
		if (pStr == NULL) break;
		
		file.Write(pStr, strlen(pStr) * sizeof(char));
		delete[] pStr;

		break;
	}

	file.Close();
}

LRESULT CISPControlDlg::OnServerRetInfo(WPARAM wParam, LPARAM lParam)
{
	USES_CONVERSION;

	if (lParam == NULL) return 0;

	IServer * pRetInfoServer = (IServer *)lParam;
	
	if (pRetInfoServer != m_pServerPreview) //this return info server current no preview 
		return 0;
	
	unsigned int nLen = MAX_ID_LEN;
	char strServerID[MAX_ID_LEN] = {0};
	pRetInfoServer->GetServerID(strServerID, &nLen);

	char strIPAddr[MAX_IP_LEN] = {0};
	nLen = MAX_IP_LEN;
	pRetInfoServer->GetServerIp(strIPAddr, &nLen);

	CString strInfo, strServerInfo;
	strInfo.Format(L"Server%s(%s)Back:");
	
	RETINFO stInfo = {0};

	{
		CAutoLock lock(&m_csForRet);
		memcpy(&stInfo, &m_stRetInfo, sizeof(RETINFO));
	}

	GetStringFromRetInfo(m_stRetInfo, strServerInfo);
	
	if (strServerInfo.GetLength() <= 0)
		return 0;

	strInfo.Append(strServerInfo);
	
	AfxMessageBox(strServerInfo);

	return 0;
}
