// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "BurnTool.h"

#include "MainFrm.h"

#include "BurnToolDoc.h"
#include "BurnToolView.h"

#include "DlgPassword.h"
#include "DlgPasswordChange.h"
#include "DlgConfig.h"

#include "MyEdit.h"

#include <initguid.h>
#include <setupapi.h>
#include "dbt.h"
#include "TRANSC.h"

#include "burn.h"
#include "updatebase.h"
#include "DlgImageCreate.h"
#include "DlgSpiImageCreate.h"
#include "AKFS.h"
#include "Update.h"
#include "CheckExportDlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////

#define ID_VIEW_LANG1 1001
#define ID_VIEW_LANG2 1002

#define UDISK_NULL 0
#define UDISK_OPEN 1
#define UDISK_CHECK 2

#define	TIMER_BURN		1
#define TIMER_UDISK		2

#define MENU_BASE_ID 1200

DEFINE_GUID(M3USB_GUID, 0x6e7ac6a7, 0x1a4c, 0x4a72, 0x83, 0x54, 0x89, 0x95, 0x11, 0xf8, 0x13, 0x51);

DEFINE_GUID(ANYKA_USB_GUID,
    0xB367BEE1, 0xCCF3, 0x471B, 0xBC, 0xA9, 0x62, 0x92, 0x83, 0x80, 0xFB, 0x78);

//////////////////////////////////////////////////////////////////////////


CString g_arrive_name[MAX_DEVICE_NUM];
CString g_remove_name[MAX_DEVICE_NUM];

extern CConfig theConfig;
extern CString strpmid;
extern CString strrdid;

BOOL  g_bUploadbinMode = FALSE;
BOOL  g_bUpload_spialldata = FALSE;
BOOL  ramImportFlag = FALSE;		//ram参数导入标识
BOOL  m_budisk_burn = FALSE;
BOOL  m_budisk_burn_startflag = FALSE;
BOOL  m_budisk_getUSBnum = FALSE;
BOOL  m_budisk_burnstart = FALSE;
BOOL  USB_attachflag = FALSE;
int  m_budisk_current_udisknum = 0;


//HANDLE mutex_usb;
HANDLE mutex_time;
HANDLE mutex_com;
HANDLE usb_init_event;
//HANDLE image_event[MAX_DEVICE_NUM];
HANDLE image_event;
HANDLE udiskburn_event;
HANDLE capacity_event;
//HANDLE usb_link_event;    //usb个数和已连接的个数事件
HANDLE ResetDevice_event;


UINT g_download_nand_count = 0;
T_DOWNLOAD_NAND g_download_nand[MAX_DOWNLOAD_NAND];
UINT g_download_udisk_count = 0;
T_DOWNLOAD_UDISK g_download_udisk[MAX_DOWNLOAD_FILES];
UINT g_download_mtd_count = 0;
T_DOWNLOAD_MTD g_download_mtd[MAX_DOWNLOAD_MTD];
UINT m_worknum = 0;
UINT g_workTotalnum  = 0;
UINT g_workThreadnum = 0;
UINT g_timer_counter = 0;
volatile UINT g_udisk_burnnum = 0;
UINT  m_USBnum = 0;
CHAR burn_detel_usb_flag[MAX_DEVICE_NUM] = {0}; //
CHAR burn_auto[MAX_DEVICE_NUM] = {0}; //当勾选自动烧录时，那么通过此变量定每一个通道都是否都为1，才开始全部烧录
extern UINT g_capacity_size[MAX_DEVICE_NUM];  //各nand 和 sd卡的容量
extern BOOL g_capacity_flag;
extern UINT g_capacity_burnnum;


extern CBurnToolApp theApp;

UINT g_disk_count = 0;
//UINT g_img_stat[MAX_DEVICE_NUM];
UINT g_img_stat;
//T_DISK_INFO g_disk_info[16];
T_nID_DISK_INFO g_nID_disk_info[MAX_DEVICE_NUM] = {0};
CRITICAL_SECTION image_cs;

BOOL    g_bEraseMode = FALSE;
char g_diskname_No[MAX_DEVICE_NUM] = {0};
char g_download_mtd_flag[MAX_DEVICE_NUM] = {0};
HANDLE g_handle = NULL;   //信号量

UINT get_udiskfile_drivernum(UINT filenum);


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_TBTN_START, OnTbtnStart)
	ON_COMMAND(ID_TBTN_SETTING, OnTbtnSetting)
	ON_COMMAND(ID_TBTN_IMPORT, OnTbtnImport)
	ON_COMMAND(ID_TBTN_EXPORT, OnTbtnExport)
	ON_COMMAND(ID_VIEW_COM, OnViewCom)
	ON_UPDATE_COMMAND_UI(ID_VIEW_COM, OnUpdateViewCom)
	ON_COMMAND(ID_SETTING_CHANGE_PASSWD, OnSettingChangePasswd)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_ACTIVATE()
	ON_UPDATE_COMMAND_UI(ID_SETTING_CHANGE_PASSWD, OnUpdateSettingChangePasswd)
	ON_UPDATE_COMMAND_UI(ID_APP_HELP, OnUpdateAppHelp)
	ON_COMMAND(ID_APP_HELP, OnAppHelp)
	ON_COMMAND(ID_TOOL_IMAGE_MAKER, OnToolImageMaker)
	ON_COMMAND(ID_SPI_IMAGE_MAKER, OnSpiImageMaker)
	ON_COMMAND(ID_BIN_READ_BACK, OnBinUpload)
	ON_BN_CLICKED(IDC_CHECK_ONLINE_IMAGE_MAKE, OnOnlineImageMake)
	ON_BN_CLICKED(IDC_CHECK_AUTO_BURN, OnAutoBurn)
	ON_COMMAND(ID_UPDATE_IMPORT, OnTbtnImport)
	ON_COMMAND(ID_UPDATE_EXPORT, OnTbtnExport)
	ON_COMMAND(ID_CONFIG_SETTING, OnTbtnSetting)
	ON_WM_CLOSE()
    ON_COMMAND(IDC_BUTTON_CLR_REC, onTbtbClrRec)
	ON_BN_CLICKED(IDC_CHECK_UDISK_BURN, OnCheckUdiskBurn)
	//}}AFX_MSG_MAP
	//ON_MESSAGE(ON_COM_RECEIVE, On_Receive)
	ON_MESSAGE(ON_BURNFLASH_MESSAGE, On_Message)
	ON_MESSAGE(ON_BURNFLASH_PROCESS, On_Process)
    ON_MESSAGE(ON_BURNFLASH_DEVICE_ARRIVE, On_DeviceArrive)
    ON_MESSAGE(ON_BURNFLASH_DEVICE_REMOVE, On_DeviceRemove)
    
	ON_COMMAND(ID_MENU_CONFIG0+0, OnChangeConfig0)
	ON_COMMAND(ID_MENU_CONFIG0+1, OnChangeConfig1)
	ON_COMMAND(ID_MENU_CONFIG0+2, OnChangeConfig2)
	ON_COMMAND(ID_MENU_CONFIG0+3, OnChangeConfig3)
	ON_COMMAND(ID_MENU_CONFIG0+4, OnChangeConfig4)
	ON_COMMAND(ID_MENU_CONFIG0+5, OnChangeConfig5)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	CAKFS cAK;

	cAK.Init();
	// TODO: add member initialization code here
	m_pEdit = NULL;

	m_device_num = 0;

	download_length_data = 0;
	download_length_mtd = 0;
	download_length_udisk = 0;
	download_length_new_mtd = 0;
	bin_pos = 0;

	m_main_timer = 0;
	m_time_count = 0;	
    
    m_cur_link = 0;
    m_cur_useful = theConfig.device_num;
	m_total_num = 0;
	m_total_pass = 0;
    m_total_fail = 0;
    
	m_update_udisk_cnt = 0;
    m_bScan = TRUE;

	CreateDirectory(theApp.ConvertAbsolutePath(_T("log")), NULL);//创建文件夹
    frmLogfile.SetFileName(_T("log\\frm_log.txt"));
    frmLogfile.CheckFileSize(512*1024);  // delete the log file if it is larger than 512K
    
    frmLogfile.InitFile();
    frmLogfile.WriteLogFile(0,"*********************************************************************************\r\n");
    frmLogfile.WriteLogFile(LOG_LINE_TIME | LOG_LINE_DATE,"Open burntool.exe\r\n");

	memset(m_udisk_state, 0, sizeof(m_udisk_state));
	memset(m_burn_time, 0, sizeof(m_burn_time));
}

CMainFrame::~CMainFrame()
{
	if(m_pEdit)
	{
		delete[] m_pEdit;
		m_pEdit =NULL;
	}
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~(LONG) FWS_ADDTOTITLE;

	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	UINT i = 0;

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
    
    if(!m_DlgBar.Create(this, IDD_BAR_BURN_INFO,CBRS_BOTTOM | CBRS_TOOLTIPS | CBRS_FLYBY, IDD_BAR_BURN_INFO))
	{
		TRACE0("Failed to create Dialogbar\n");
		return -1;      // fail to create
	}
    m_DlgBar.SetDlgItemText(IDC_STATIC_CUR_NUM, theApp.GetString(IDS_MAINVIEW_CUR_NUM));
    m_DlgBar.SetDlgItemText(IDC_STATIC_CUR_USEFUL,theApp.GetString(IDS_MAINVIEW_CUR_USEFUL));
    m_DlgBar.SetDlgItemText(IDC_STATIC_CUR_PASS,theApp.GetString(IDS_MAINVIEW_CUR_PASS));
    m_DlgBar.SetDlgItemText(IDC_STATIC_TOTAL_PASS,theApp.GetString(IDS_MAINVIEW_TOTAL_PASS));
    m_DlgBar.SetDlgItemText(IDC_STATIC_TOTAL_FAIL,theApp.GetString(IDS_MAINVIEW_TOTAL_FAIL));
    m_DlgBar.SetDlgItemText(IDC_BUTTON_CLR_REC,theApp.GetString(IDS_MAINVIEW_CLR_REC));
	m_DlgBar.SetDlgItemText(IDC_STATIC_STATE,theApp.GetString(IDS_STATIC_STATE));
	m_DlgBar.SetDlgItemText(IDC_STATIC_NOW_UDISK,theApp.GetString(IDS_STATIC_NOW_UDISK));
	m_DlgBar.SetDlgItemText(IDC_STATIC_UDISK,theApp.GetString(IDS_STATIC_UDISK_NUM));
	m_DlgBar.SetDlgItemText(IDC_CHECK_AUTO_BURN,theApp.GetString(IDS_AUTO_BURN));
	m_DlgBar.SetDlgItemText(IDC_CHECK_UDISK_BURN,theApp.GetString(IDS_UDISK_BURN));
	m_DlgBar.SetDlgItemText(IDC_CHECK_ONLINE_IMAGE_MAKE,theApp.GetString(IDS_ONLINE_IMAGE_MAKE));

	// Create CoolBar for COM data
	if(!m_ComBar.Create(_T("COM Bar"), this, CSize(200, 150), TRUE, ID_COM_BAR))
	{
		TRACE0("Failed to create com bar\n");
		return -1;
	}

    //Build Com Bar
	// Open COM
	if(!SetupCOM(theConfig.com_count))
	{
		TRACE0("Failed to build com bar\n");
		return -1;
	}

	// Create Tool Bar
	if (!m_ToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_ToolBar.LoadToolBar(IDR_TLB_BURN))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	m_ToolBar.SetButtonText(0, theApp.GetString(IDS_TOOLBAR_START));
	m_ToolBar.SetButtonText(1, theApp.GetString(IDS_TOOLBAR_SETTING));
	m_ToolBar.SetButtonText(2, theApp.GetString(IDS_TOOLBAR_IMPORT));
	m_ToolBar.SetButtonText(3, theApp.GetString(IDS_TOOLBAR_EXPORT));
	//m_ToolBar.SetButtonText(4, theApp.GetString(IDS_TOOLBAR_FORMAT));

	/////////////调整工具条/////////////////
	CRect rc(0, 0, 0, 0);
	CSize sizeMax(0, 0);
	CToolBarCtrl& bar = m_ToolBar.GetToolBarCtrl();
	for (int nIndex = bar.GetButtonCount() - 1; nIndex >= 0; nIndex--)
	{
		bar.GetItemRect(nIndex, rc);

		rc.NormalizeRect();
		sizeMax.cx = __max(rc.Size().cx, sizeMax.cx);
		sizeMax.cy = __max(rc.Size().cy, sizeMax.cy);
	}
	sizeMax.cx += 10;
	sizeMax.cy += 6;
	m_ToolBar.SetSizes(sizeMax, CSize(32,36));

	//create com bar
	m_ComBar.SetBarStyle(m_ComBar.GetBarStyle() | 
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	m_ComBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_ComBar, AFX_IDW_DOCKBAR_BOTTOM);

	//change window size
	MoveWindow(0, 0, 800, 600);	

	//Initialize m_pBurn
	SetupBurn(theConfig.device_num);

	//get password
	theConfig.GetPassword();

	//set window title
	set_window_title();

	//setup download
	SetupDownload();

    RefreshWorkInfo();

	//create mutex
	//mutex_usb = CreateMutex(NULL, FALSE, NULL);
	mutex_time = CreateMutex(NULL, FALSE, NULL);
	mutex_com = CreateMutex(NULL, FALSE, NULL);
	usb_init_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	//for (i = 0; i < MAX_DEVICE_NUM; i++)
	//{
	image_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	udiskburn_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	//usb_link_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	ResetDevice_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	capacity_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	g_handle = CreateSemaphore(NULL, 1, 1, NULL);
	//}
	m_budisk_burn = FALSE;
	
    SetConfigMenu();
	
	SetupDisplay();
    
    ClearLogFile();
	//start timer
	StartTimer();

	//register m3bios message handler
	DoRegisterDeviceInterface();

    ::SetProp(m_hWnd, theApp.m_Open_prop, (HANDLE)1);
	return 0;
}


void CMainFrame::OnTbtnStart() 
{
	UINT i = 0, j = 0;

	// TODO: Add your command handler code here
	CBurnToolView *pView = (CBurnToolView *)GetActiveView();

	ResetEvent(image_event);
	ResetEvent(udiskburn_event);
	ResetEvent(capacity_event);
	m_budisk_burn = TRUE;
	USB_attachflag = TRUE; //当按开始后，不再进行查找usb设备个数，这样是降低连接的时间

	frmLogfile.WriteLogFile(0,"BT_Start \n");
	BYTE check = ((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_AUTO_BURN))->GetCheck();
	if ((g_workTotalnum = BT_Start()) != 0)
	{
        m_total_num += g_workTotalnum;
        g_workThreadnum = 0;
		g_capacity_burnnum = 0;
	    //if(m_worknum > 0)
	    //{
		//    disable_control();
	    //}
		frmLogfile.WriteLogFile(0,"BT_Start_g_workTotalnum :%d \n", g_workTotalnum);
    }
	else if ((theConfig.bUDiskUpdate == TRUE || check == TRUE ) && (m_budisk_burn_startflag == FALSE))
	{
		CUpdateBase update;
		char diskname[32];
		TCHAR strDisk[4];
		UINT flag = 0;
		UINT idex = 0;
		//int update_auto = m_update_cnt; // 在之前找到的自动升级设备（block0被擦除）
		
		//m_update_cnt = 0;
		memset(g_diskname_No, 0, 32);
		m_update_udisk_cnt = 0;
		g_udisk_burnnum = 0;  //对于u盘烧录时，进行初始化

		m_update_udisk_cnt = update.GetUDisk(diskname, 32);
		m_budisk_current_udisknum = m_update_udisk_cnt;
		m_budisk_getUSBnum = TRUE;
		m_budisk_burnstart = TRUE;
		//g_udisk_burnnum = 0;
		
		//获取挂起u盘时的设备个数
		//g_udisk_burnnum = BT_GetUSB_Num(theConfig.PID, theConfig.VID);

		//在ontimer已获取到个数，所以这里重附值就可以了
		g_udisk_burnnum = m_USBnum;
		frmLogfile.WriteLogFile(0,"get the usb num = %d \n",g_udisk_burnnum);
		//g_udisk_burnnum = BT_GetUSB_Num(57601, 1238);

		if (m_update_udisk_cnt != 0 && theConfig.bUDiskUpdate && m_worknum == 0 && g_udisk_burnnum != 0)
		{
			
			for (i=0; i< (UINT)m_update_udisk_cnt; i++)
			{
				strDisk[0] = diskname[i];
				strDisk[1] = ':';
				strDisk[2] = 0;
				
				//判断此移动磁盘是不是我们公司的产品
				flag = BT_DriverName_IsCurrentFlag(strDisk, theConfig.PID, theConfig.VID);
				if(flag == 1)	
				{
					g_diskname_No[idex] = diskname[i] - 'A';
					idex++;
				}
			}

			for (i=0; i< (UINT)m_update_udisk_cnt; i++)
			{
				strDisk[0] = diskname[i];
				strDisk[1] = ':';
				strDisk[2] = 0;

				//判断此移动磁盘是不是我们公司的产品
				//flag = BT_DriverName_IsCurrentFlag(strDisk, theConfig.PID, theConfig.VID);
				//如果磁盘是非公司的产品，那么该盘不做如下操作
				if (g_diskname_No[0] != 0)
				{
					for (j = 0; j < 32; j++)
					{
						if (g_diskname_No[j] != 0 && g_diskname_No[j] == (int)i)
						{
							break;
						}
					}
				}
				else
				{
					j = 32;  //说明此时并没有U盘是非公司的
				}
				
				if (j == 32)//flag != 1)
				{
					PostMessage(ON_BURNFLASH_MESSAGE, diskname[i], LPARAM(MESSAGE_SEND_CMD_ANYKA));
				}
				//else
				//{
				//	g_diskname_No[idex] = diskname[i] - 'A';
				//	idex++;
				//}
			}
        }
		else
		{
			USB_attachflag = FALSE;
		}
	}
	m_budisk_burn_startflag = FALSE;

		/*

		m_bScan = FALSE;

		for (int i=0; i< m_update_udisk_cnt; i++)
		{
			if(update.OpenDisk(diskname[i]))
			{
				update.SendCommand();
				
				update.CloseDisk();					
			}
			Sleep(150);	
		}

		m_bScan = TRUE;
        
	}*/
    RefreshWorkInfo();
}

void CMainFrame::OnTbtnSetting() 
{
	// TODO: Add your command handler code here
	CBurnToolView *pView = (CBurnToolView *)GetActiveView();

	CDlgPassword dlgPasswd;
    CString str;

	dlgPasswd.m_user_id = theApp.GetString(IDS_USER_PRODUCER);

	if(dlgPasswd.DoModal() == IDOK)
	{	
		if(strpmid == theApp.GetString(IDS_USER_PRODUCER))
		{
			theConfig.bUI_ctrl = TRUE;

			if(dlgPasswd.m_password != theConfig.passwd_ctrl.pm_password)
			{
                str = theApp.GetString(IDS_PASSWORD_ERROR);
				MessageBox(str, NULL, MB_ICONERROR | MB_OK);
				return;
			}
		}
		else
		{		
			theConfig.bUI_ctrl = FALSE;

			if(dlgPasswd.m_password != theConfig.passwd_ctrl.rd_password)
			{
                str = theApp.GetString(IDS_PASSWORD_ERROR);
				MessageBox(str, NULL, MB_ICONERROR | MB_OK);
				return;
			}
		}			
	}
	else
	{
		return;
	}

	CDlgConfig dlg_Cfg;
	if(dlg_Cfg.DoModal())
	{
		//setup com
		SetupCOM(theConfig.com_count);

		//setup burn
		SetupBurn(theConfig.device_num);
		
		//setup download
		SetupDownload();

		//setup list view
		pView->SetupListView(theConfig.device_num);

		//set window title
		set_window_title();

		//write config

		theConfig.WriteConfig(theApp.m_config_file[theApp.m_config_index]);
		
	}
}

BOOL CMainFrame::SetupCOM(int com_count)
{
	CString str;
	int i;

	if(m_pEdit)
	{
		m_pEdit->DestroyWindow();
		delete[] m_pEdit;
		m_pEdit = NULL;
	}

	if(!theConfig.bOpenCOM || com_count <= 0)
	{

		ShowControlBar(&m_ComBar, FALSE, FALSE);
		return TRUE;
	}


	m_pEdit = new CMyEdit[com_count];
	for(i = 0; i < com_count; i++)
	{
		m_pEdit->SetReadOnly();
		m_pEdit[i].ModifyStyleEx(0, WS_EX_STATICEDGE);

		str.Format(_T(" COM %u "), i + theConfig.com_base);
	}


	m_pEdit[0].ShowWindow(SW_SHOW);

	ShowControlBar(&m_ComBar, TRUE, FALSE);

	return TRUE;
}

BOOL CMainFrame::SetupBurn(int device_num)
{
//	int i;

	m_device_num = device_num;

	return true;
}

void CMainFrame::RefreshWorkInfo()
{
	CString str;
	BYTE check = 0;
	CMenu *pMainMenu = GetMenu();
	CMenu *pSubMenu;
	
	str.Format(_T("%d"),m_cur_link);
	m_DlgBar.SetDlgItemText(IDC_CUR_NUM,str);
    str.Format(_T("%d"), theConfig.device_num - m_cur_link);
	m_DlgBar.SetDlgItemText(IDC_CUR_USEFUL,str);
    str.Format(_T("%d"),m_total_num);
    m_DlgBar.SetDlgItemText(IDC_TOTAL_NUM,str);
    str.Format(_T("%d"),m_total_pass);
    m_DlgBar.SetDlgItemText(IDC_TOTAL_PASS,str);
    str.Format(_T("%d"), m_total_fail);
	m_DlgBar.SetDlgItemText(IDC_TOTAL_FAIL,str);

	pSubMenu = pMainMenu->GetSubMenu(2);

	if ((TRUE == theConfig.ShowImageMaker_flag) && (theConfig.planform_tpye == E_LINUX_PLANFORM))
	{
		pSubMenu->DeleteMenu(ID_TOOL_IMAGE_MAKER, MF_BYCOMMAND);
		theConfig.ShowImageMaker_flag = FALSE;
	}
	else if ((FALSE == theConfig.ShowImageMaker_flag) && (theConfig.planform_tpye == E_ROST_PLANFORM))
	{
		//先删除
		pSubMenu->DeleteMenu(ID_TOOL_IMAGE_MAKER, MF_BYCOMMAND);
		pSubMenu->DeleteMenu(ID_SPI_IMAGE_MAKER, MF_BYCOMMAND);
		pSubMenu->DeleteMenu(ID_BIN_READ_BACK, MF_BYCOMMAND);
		//再增加
		pSubMenu->AppendMenu(MF_STRING, ID_TOOL_IMAGE_MAKER, theApp.GetString(IDS_MENU_CREATIMAGE_TOOL));
		theConfig.ShowImageMaker_flag = TRUE;
		pSubMenu->AppendMenu(MF_STRING, ID_SPI_IMAGE_MAKER, theApp.GetString(IDS_SPI_IMAGE_MAKER));
		theConfig.ShowImageMaker_flag = TRUE;
		pSubMenu->AppendMenu(MF_STRING, ID_BIN_READ_BACK, theApp.GetString(IDS_BIN_READ_BACK));
		theConfig.ShowImageMaker_flag = TRUE;
	}
	//切换界面时进行重设置
	((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_UDISK_BURN))->SetCheck(theConfig.bUDiskUpdate);
	//因为10和11芯片有支持烧完后重启功能，所以以防重复烧录，此些平台不提供自动烧录功能
	if (TRUE == theConfig.bUDiskUpdate && (theConfig.chip_type == CHIP_1080A
		|| theConfig.chip_type == CHIP_1080L || theConfig.chip_type == CHIP_11XX
		|| theConfig.chip_type == CHIP_10X6 || theConfig.chip_type == CHIP_10XXC))
	{
		((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_AUTO_BURN))->EnableWindow(FALSE);
		((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_AUTO_BURN))->SetCheck(0);
	} 
	else
	{
		((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_AUTO_BURN))->EnableWindow(TRUE);
	}

	if (CHIP_10X6 == theConfig.chip_type || CHIP_10XXC == theConfig.chip_type || CHIP_1080L == theConfig.chip_type)
	{
		((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_ONLINE_IMAGE_MAKE))->ShowWindow(FALSE);
	}
	else
	{
		((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_ONLINE_IMAGE_MAKE))->ShowWindow(TRUE);
		check = ((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_ONLINE_IMAGE_MAKE))->GetCheck();
		if (check)
		{
			theConfig.bonline_make_image = TRUE;
		}
		else 
		{
			theConfig.bonline_make_image = FALSE;
		}
		
	}
/*
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && (theConfig.chip_type == CHIP_11XX))
	{
		((CButton *)m_DlgBar.GetDlgItem(IDC_UPDATE_EXPORT))->ShowWindow(TRUE);
		((CButton *)m_DlgBar.GetDlgItem(IDC_UPDATE_IMPORT))->ShowWindow(TRUE);
	}
	else
	{
		((CButton *)m_DlgBar.GetDlgItem(IDC_UPDATE_EXPORT))->ShowWindow(FALSE);
		((CButton *)m_DlgBar.GetDlgItem(IDC_UPDATE_IMPORT))->ShowWindow(FALSE);
	}
    */
}

void CMainFrame::onTbtbClrRec()
{
    m_total_num = 0;
    m_total_pass = 0;
    m_total_fail = 0;
    RefreshWorkInfo();
}

void CMainFrame::OnViewCom() 
{
	// TODO: Add your command handler code here
	BOOL bVisible = ((m_ComBar.GetStyle() & WS_VISIBLE) != 0);
	
	ShowControlBar(&m_ComBar, !bVisible, FALSE);
}


int CMainFrame::GetUDisk(char pDiskName[], int len)
{
	DWORD	MaxDriveSet;
	DWORD	drive;
	TCHAR	szDrvName[33];
	int		count = 0;

	MaxDriveSet = GetLogicalDrives();

	for (drive = 0; drive < 32; drive++)  
	{
		if ( MaxDriveSet & (1 << drive) )  
		{
			DWORD temp = 1<<drive;
			_stprintf(szDrvName, _T("%c:\\"), 'A'+drive);
			
			if(GetDriveType(szDrvName)== DRIVE_REMOVABLE && (drive > 1))
			{
				pDiskName[count++] = (char)('A'+drive);
				if(count >= len)
				{
					break;
				}
			}
		}
	}

	return count;
}

void CMainFrame::ScanUDisk()
{
	char diskName[32] = {0};
	int i, j;
	TCHAR strDisk[4] = {0};
	UINT vol = 0, index, times = 0;;
	BYTE check;
	int attach_flag = 0;


	if(!m_bScan)
	{
		//Sleep(1000);
		frmLogfile.WriteLogFile(0,"m_bScan == ak_false\r\n");
		return;
	}
	
	if (theConfig.bUDiskUpdate && m_budisk_burnstart && g_udisk_burnnum > 1)
	{
		frmLogfile.WriteLogFile(0,"wait the first disk mount\r\n");
		Sleep(1000);
		m_budisk_burnstart = AK_FALSE;
	}
	

	int udisk_cnt = GetUDisk(diskName, 32);
	frmLogfile.WriteLogFile(0,"udisk_cnt i = %d\n", udisk_cnt);
	for(i = 0; i < udisk_cnt; i++)
	{
		frmLogfile.WriteLogFile(0, "BT_AttachUSB diskName = %c\n", diskName[i]);
	}

	for(i = 0; i < 32; i++)
	{
		//如果磁盘是非公司的产品，那么该盘不做如下操作
		if (g_diskname_No[0] != 0)
		{
			for (j = 0; j < 32; j++)
			{
				if (g_diskname_No[j] != 0 && g_diskname_No[j] == i)
				{
					break;
				}
			}
		}
		else
		{
			j = 32;  //说明此时并没有U盘是非公司的
		}

		//j == 32 表示没有非本公司的磁盘
		if(UDISK_OPEN == m_udisk_state[i] && j == 32)
		{
			m_udisk_state[i] = UDISK_CHECK;
            frmLogfile.WriteLogFile(0,"ScanUDisk i = %d m_udisk_state %d\n", i, m_udisk_state[i]);
		}
	}
	

	for(i = 0; i < udisk_cnt; i++)
	{
		attach_flag = 0;
		if(diskName[i] >= 'A' && diskName[i] <= 'Z')
		{
			vol = diskName[i] - 'A';

           frmLogfile.WriteLogFile(0,"BT_AttachUSB vol = %d diskName = %c m_udisk_state %d\n",vol, diskName[i], m_udisk_state[vol]);
			if(UDISK_NULL == m_udisk_state[vol])
			{
				strDisk[0] = diskName[i];
				strDisk[1] = ':';
				strDisk[2] = 0;
				
				index = BT_AttachUSB(strDisk, USB_MASS);
				check = ((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_AUTO_BURN))->GetCheck();

				//
				if (index == 0)
				{
					//多台烧录时，如果读资源失败了，那么下一次也会进行再连接一次
					attach_flag = BT_Get_AttachUSB_flag(strDisk);
				}
				frmLogfile.WriteLogFile(0,"BT_AttachUSB2 %d, attach_flag:%d\n", index, attach_flag);

				if (attach_flag != 1)
				{
					if(index > 0)
					{
						m_cur_link++;
						//g_udisk_burnnum++;
						burn_auto[index-1] = 1;
						//PostMessage(ON_BURNFLASH_DEVICE_ARRIVE, (WPARAM)index, 0);
						PostMessage(ON_BURNFLASH_MESSAGE, index+100, LPARAM(MESSAGE_SET_STANDBY));
					}
					else if (check == TRUE && m_worknum == 0 && theConfig.bUDiskUpdate) // if (theConfig.bUDiskUpdate && m_worknum == 0)
					{
						//当勾选自动烧录，以及是刚开始的
						PostMessage(ON_BURNFLASH_MESSAGE, diskName[i], LPARAM(MESSAGE_SEND_CMD_ANYKA));
					}
					m_udisk_state[vol] = UDISK_OPEN;
				}
			}
			else if(UDISK_CHECK == m_udisk_state[vol])
			{
				m_udisk_state[vol] = UDISK_OPEN;
			}
		}
	}

	for(i = 0; i < 32; i++)
	{

		if(UDISK_CHECK == m_udisk_state[i])
		{
			strDisk[0] = i + 'A';
			strDisk[1] = ':';
			strDisk[2] = 0;
			
			index = BT_DetachUSB(strDisk, USB_MASS);
 			if(index > 0)
			{
                frmLogfile.WriteLogFile(0,"!!udisk\n");
                if (m_cur_link)
                {
                    m_cur_link--;
                }
				burn_auto[index-1] = 0;

				if (burn_detel_usb_flag[index-1] != 1)
				{
					PostMessage(ON_BURNFLASH_MESSAGE, index+100, LPARAM(MESSAGE_RESET_STANDBY));
				}
				burn_detel_usb_flag[index-1] = 0;
			}
            frmLogfile.WriteLogFile(0,"BT_DetachUSB i = %d diskName = %c index = %d\n", i, strDisk[0], index);
			
			m_udisk_state[i] = UDISK_NULL;
		}
	}

    RefreshWorkInfo();
}

LRESULT CMainFrame::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{//插入设备都在这里
	// TODO: Add your specialized code here and/or call the base class
   PDEV_BROADCAST_DEVICEINTERFACE hDev_Broadcast_Deviceinterface;

   E_USB_TYPE usbType;

    if(message == WM_DEVICECHANGE)
    {
		frmLogfile.WriteLogFile(0,"WindowProc-->wParam: %x\n", wParam);
        if(wParam == DBT_DEVICEARRIVAL)
		{
            PDEV_BROADCAST_HDR hDev_Broadcast_HDR = (PDEV_BROADCAST_HDR)lParam;

			frmLogfile.WriteLogFile(0,"hDev_Broadcast_HDR->dbch_devicetype: %x\n", hDev_Broadcast_HDR->dbch_devicetype);
            switch(hDev_Broadcast_HDR->dbch_devicetype)
            {

            case DBT_DEVTYP_DEVICEINTERFACE:
                frmLogfile.WriteLogFile(0,"DBT_DEVTYP_DEVICEINTERFACE");
                
                hDev_Broadcast_Deviceinterface = 
                    (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

               if( hDev_Broadcast_Deviceinterface->dbcc_classguid == M3USB_GUID 
					|| hDev_Broadcast_Deviceinterface->dbcc_classguid == ANYKA_USB_GUID)
                {
					UINT index;

					if (hDev_Broadcast_Deviceinterface->dbcc_classguid == M3USB_GUID)
					{
						usbType = USB_M3USB;
					}
					else
					{
						usbType = USB_AP3USB;
					}

					index = BT_AttachUSB(hDev_Broadcast_Deviceinterface->dbcc_name, usbType);
					
					if(index > 0)
					 {
		                 PostMessage(ON_BURNFLASH_MESSAGE, index+100, LPARAM(MESSAGE_SET_STANDBY));
					 }
                }
                break;
			  case DBT_DEVTYP_VOLUME:
                  frmLogfile.WriteLogFile(0,"DBT_DEVTYP_VOLUME\n");
				
				ScanUDisk();
				break;
            default:
                frmLogfile.WriteLogFile(0,"KKKKKKKKKKKKKKKK\n");
				
                break;
            }
		}
        else if(wParam == DBT_DEVICEREMOVECOMPLETE)
        {

            PDEV_BROADCAST_HDR hDev_Broadcast_HDR = (PDEV_BROADCAST_HDR)lParam;

            switch(hDev_Broadcast_HDR->dbch_devicetype)
            {
            case DBT_DEVTYP_DEVICEINTERFACE:
                frmLogfile.WriteLogFile(0,"DBT_DEVTYP_DEVICEINTERFACE2\n");
                
                hDev_Broadcast_Deviceinterface = 
                    (PDEV_BROADCAST_DEVICEINTERFACE)lParam;

                if( hDev_Broadcast_Deviceinterface->dbcc_classguid == M3USB_GUID 
					|| hDev_Broadcast_Deviceinterface->dbcc_classguid == ANYKA_USB_GUID)
                {
					UINT index;

					if (hDev_Broadcast_Deviceinterface->dbcc_classguid == M3USB_GUID)
					{
						usbType = USB_M3USB;
					}
					else
					{
						usbType = USB_AP3USB;
					}

                    index = BT_DetachUSB(hDev_Broadcast_Deviceinterface->dbcc_name, usbType);

 					if(index > 0)
					{
						PostMessage(ON_BURNFLASH_MESSAGE, index+100, LPARAM(MESSAGE_RESET_STANDBY));
					}
                }
                break;
			case DBT_DEVTYP_VOLUME:
                frmLogfile.WriteLogFile(0,"DBT_DEVTYP_VOLUME!2\n");
				
				ScanUDisk();
				break;
            default:
                frmLogfile.WriteLogFile(0,"YYYYYYYYYYYYYYYY\n");
				
                break;
            }
            
        }
    }
	return CFrameWnd::WindowProc(message, wParam, lParam);
}

BOOL CMainFrame::DoRegisterDeviceInterface()
{
    BOOL ret = 1;

	BT_RegisterDevice(GetSafeHwnd(), USB_AP3USB);
	BT_RegisterDevice(GetSafeHwnd(), USB_M3USB);
	
    return ret;
}

void CMainFrame::fail_in_module_burn(int id, LPCTSTR str)
{
	CBurnToolView *pView = (CBurnToolView *)GetActiveView();

	pView->SetStatDesp(id, str);
	pView->SetStatColor(id, COLOR_STAT_FAIL);

	pView->SetProgColor(id, COLOR_PROG_FAIL);
    if (m_worknum != 0)
    {
        if ( m_cur_link != 0)
        {
            m_cur_link--;
        }
		burn_auto[id] = 0;
	    m_worknum--;
		if (g_capacity_burnnum == m_worknum)
		{
			SetEvent(capacity_event);
		}
        if (g_workTotalnum != 0)
            g_workTotalnum--;
        
	    if(0 == m_worknum)
	    {
		    enable_control();
	    }
    }
}

//Module burn regular message handler
LRESULT CMainFrame::On_ModuleBurnMessage(WPARAM wp, LPARAM lp)
{
	CBurnToolView *pView = (CBurnToolView *)GetActiveView();
	int channel = wp - 100;
//	int i;
	int burn_index = -1;

	if(burn_index < 0)
	{
		return 0;
	}

	return 0;
}

//Module burn process message handler
LRESULT CMainFrame::On_ModuleBurnProgress(WPARAM wp, LPARAM lp)
{
//	CBurnToolView *pView = (CBurnToolView *)GetActiveView();
//	int channel = wp - 100;
//	int progress = (int)lp;
//	int i;
	
	return 0;
}

LRESULT CMainFrame::On_Process(WPARAM wp, LPARAM lp)
{
	CBurnToolView *pView = (CBurnToolView *)GetActiveView();
	CString str;
	int port = wp - 100 - 1;
	int length = lp;
	
	UINT total_length;
	UINT pos;
	UINT parttion_length = 0, i = 0;

	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)))
	{
		if(theConfig.bUpdate)
		{
			total_length = download_length_data;
			pos = (int)(100*((double)length/total_length));
		}
		else
		{
			if (g_download_mtd_flag[port] == 0)
			{
				total_length = download_length_data + download_length_mtd;
				pos = (int)(20*((double)length/total_length));
				bin_pos = pos;
			}
			else
			{
				total_length = download_length_new_mtd;
				pos = bin_pos + (int)(80*((double)(length-download_length_data)/total_length));
			}
		}
	}
	else
	{
		if (theConfig.planform_tpye == E_LINUX_PLANFORM && theConfig.burn_mode == E_CONFIG_SFLASH)
		{
			for (i = 0; i < theConfig.format_count; i++)
			{
				parttion_length += theConfig.spi_format_data[i].Size;
			}
			parttion_length = parttion_length*1024*1024;
			
		}
		else
		{
			parttion_length = 0;
		}
		total_length = download_length_data + download_length_mtd + download_length_udisk + parttion_length;
		pos = (int)(100*((double)length/total_length));
	}


    if(pos > 100)
    {
        pos = 100;
    }
        
    pView->SetProgPos(port, pos);

	return 0;
}

void CMainFrame::OnChangeConfig0()
{
    theApp.m_config_index = 0;
    OnChangeConfig();
}
void CMainFrame::OnChangeConfig1()
{
    theApp.m_config_index = 1;
    OnChangeConfig();
}
void CMainFrame::OnChangeConfig2()
{
    theApp.m_config_index = 2;
    OnChangeConfig();
}
void CMainFrame::OnChangeConfig3()
{
    theApp.m_config_index = 3;
    OnChangeConfig();
}
void CMainFrame::OnChangeConfig4()
{
    theApp.m_config_index = 4;
    OnChangeConfig();
}
void CMainFrame::OnChangeConfig5()
{
    theApp.m_config_index = 5;
    OnChangeConfig();
}

void CMainFrame::OnChangeConfig()
{
    CMenu  *pSubMenu = GetMenu()->GetSubMenu(3);
    UINT   state;
    UINT   i;

    state = pSubMenu->GetMenuState(ID_MENU_CONFIG0+theApp.m_config_index, MF_CHECKED);
    if (MF_CHECKED == state)    // not choose the same configuration
    {
        return;
    }

    
    for (i=0; i<theApp.m_config_file_cnt; i++)  // remove all check flag
    {
        pSubMenu->CheckMenuItem(ID_MENU_CONFIG0+i, MF_UNCHECKED);
    }
    pSubMenu->CheckMenuItem(ID_MENU_CONFIG0+theApp.m_config_index, MF_CHECKED); // make the new check flag

    theConfig.ReadConfig(theApp.m_config_file[theApp.m_config_index]);  // load the new configuration
    
    CBurnToolView *pView = (CBurnToolView *)GetActiveView();

    // refresh configurations and UI
    {
        //setup download
		SetupDownload();
		
		//setup list view
		pView->SetupListView(theConfig.device_num);
		
		//set window title
		set_window_title();

		//m_now_link = 0;
		//m_now_pass = 0;
		//DisplayWorkNO();

		BT_Init(theConfig.device_num,  BurnThread,  BurnProgress);
    }
}

LRESULT CMainFrame::On_Message(WPARAM wp, LPARAM lp)
{
	CBurnToolView *pView = (CBurnToolView *)GetActiveView();
	CString str;
	T_MODE_CONTROL ModeCtrl;
	int port = wp - 100 - 1;	
	BYTE check = 0;
	UINT i = 0;
	BOOL falg = FALSE;

	switch(lp)
	{
	case MESSAGE_SET_STANDBY:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_READY));
		pView->SetStatColor(port, COLOR_STAT_STANDBY);
		pView->SetProgPos(port, 0);
		pView->SetProgColor(port, COLOR_PROG_NORMAL);
		pView->SetTime(port, 0);
		pView->SetCurrentSequenceAddr(port, _T(""));
		pView->SetCurrentMacAddr(port, _T(""));
		
		//如果是自动烧录的功能
		check = ((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_AUTO_BURN))->GetCheck();
		if ((check))// && (0 ==(theConfig.device_num - m_cur_link)))
		{
			for (i = 0; i < theConfig.device_num; i++)
			{
				if (burn_auto[i] == 1)
				{
					falg = TRUE;
				}
				else
				{
					falg = FALSE;
					break;
				}
			}
			//AfxMessageBox(_T("自动烧录"));
			if (falg == TRUE)
			{
				OnTbtnStart();
			}
		}
		else
		{
			frmLogfile.WriteLogFile(0,"m_budisk_burn:%d\n", m_budisk_burn);
			if (theConfig.bUDiskUpdate == TRUE && m_budisk_burn == TRUE)
			{
				//Sleep(500);
				/*
				if (m_cur_link == m_USBnum) //当连接的个数等于USB设备的个数时激活事件
				{
					SetEvent(usb_link_event);
				}
				//wait creat image
				WaitForSingleObject(usb_link_event, 3600000);
				*/
				//OnTbtnStart_bychannelID(port);
				frmLogfile.WriteLogFile(0,"OnTbtnStart \n");
				m_budisk_burn_startflag = TRUE;
				OnTbtnStart();
			}
		}

		break;

	case MESSAGE_RESET_STANDBY:
		pView->SetStatDesp(port, _T(""));
		pView->SetStatColor(port, COLOR_STAT_INITIAL);
		pView->SetProgPos(port, 0);
		pView->SetProgColor(port, COLOR_PROG_BACK);
	//	pView->SetTime(port, 0);
		break;
	case MESSAGE_START_BURN:
		pView->SetStatColor(port, COLOR_STAT_BURNING);
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START));
		break;

	case MESSAGE_START_SET_REGISTER:
		pView->SetStatColor(port, COLOR_STAT_BURNING);
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_SET_REGISTERS));	
		break;
		
	case MESSAGE_START_SET_RAMPARAM:
		pView->SetStatColor(port, COLOR_STAT_BURNING);
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_SET_RAMPARAM));	
		break;

	case MESSAGE_SET_RAMPARAM_FAIL:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_SET_RAMPARAM_FAIL));
		terminate_mission(port, theApp.GetString(IDS_BURN_SET_RAMPARAM_FAIL));
		break;

	case MESSAGE_SET_REGISTER_FAIL:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_SET_REGISTERS_FAIL));
		terminate_mission(port, theApp.GetString(IDS_BURN_SET_REGISTERS_FAIL));
		break;
		
	case MESSAGE_GET_REGVALUE_FAIL:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_GET_REGISTERS_FAIL));
		terminate_mission(port, theApp.GetString(IDS_BURN_GET_REGISTERS_FAIL));
		break;

	case MESSAGE_SET_REGISTER_SUCCESS:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_SET_REGISTERS_SUCCESS));
		break;

	case MESSAGE_BURN_MAC_ADDR_SHOW:
		//烧录完成后显示当前的MAC地址
		OnMacAddr(port, TRUE);
		break;

	case MESSAGE_BURN_SERIAL_ADDR_SHOW:
		//烧录完成后显示当前序列号
		OnSequenceAddr(port, TRUE); 
		break;

	case MESSAGE_START_INIT_USB:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_INIT_USB));
		break;
		
	case MESSAGE_INIT_USB_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_INIT_USB_FAIL));
		break;

	case MESSAGE_TASK_COMPLETE:
		pView->SetStatColor(port, COLOR_STAT_COMPLETE);
		if (g_bUploadbinMode == TRUE)
		{
			pView->SetStatDesp(port, theApp.GetString(IDS_BIN_UPLOAD_COMPLETE));
		}
		else if (g_bUpload_spialldata)
		{
			pView->SetStatDesp(port, theApp.GetString(IDS_SPIFLASH_UPLOAD_COMPLETE));
		}
		else
		{
			pView->SetStatDesp(port, theApp.GetString(IDS_BURN_COMPLETE));
		}
		
		pView->SetProgColor(port, COLOR_PROG_COMPLETE);
        pView->SetProgPos(port, 100);


        // 烧录完成连接数减1
        frmLogfile.WriteLogFile(0,"!!sucesss\n");
		
        if (m_cur_link != 0)
        {
            m_cur_link--;
        }
		burn_auto[port] = 0;
		
        m_total_pass++;
        RefreshWorkInfo(); 

		//set window title
		//set_window_title();

		break;
		
	case MESSAGE_DOWNLOAD_AND_TESTRAM_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_DOWNLOAD_AND_TESTRAM));
		break;

	case MESSAGE_DOWNLOAD_AND_TESTRAM_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_DOWNLOAD_AND_TESTRAM_FAIL));
		break;

	case MESSAGE_DOWNLOAD_PRODUCER_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_DOWNLOAD_PRODUCER));
		break;

	case MESSAGE_DOWNLOAD_PRODUCER_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_DOWNLOAD_PRODUCER_FAIL));
		break;

    case MESSAGE_DOWNLOAD_PRODUCER_USB_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_DOWNLOAD_PRODUCER_USB_FAIL));
		break;

    case MESSAGE_DOWNLOAD_PRODUCER_TIMEOUT_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_DOWNLOAD_PRODUCER_TIMEOUT_FAIL));
		break;

	case MESSAGE_DOWNLOAD_PRODUCER_SUCCESS:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_DOWNLOAD_PRODUCER_SUCESS));
		break;

	case MESSAGE_START_TEST_TRANSC:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_TEST));
		break;

	case MESSAGE_TEST_TRANSC_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_TEST_FAIL));
		break;

	case MESSAGE_GET_CHIP_PARA:		
		theConfig.ChangeBurnMode(&ModeCtrl);
		pView->SetStatDesp(port, theApp.GetString(
			TRANSC_MEDIUM_SPIFLASH == ModeCtrl.eMedium ? \
			IDS_BURN_START_GET_SPIFLASH_CHIP_ID : \
			IDS_BURN_START_GET_NAND_CHIP_ID));
		break;

	case MESSAGE_GET_HIGHID_FAIL:
		terminate_mission(port, theApp.GetString(IDS_GET_HIGHID));
		break;

	case MESSAGE_GET_CHIP_PARA_FAIL:
		theConfig.ChangeBurnMode(&ModeCtrl);
		terminate_mission(port, theApp.GetString(
			TRANSC_MEDIUM_SPIFLASH == ModeCtrl.eMedium ? \
			IDS_BURN_GET_SPIFLASH_CHIP_ID_FAIL :  \
			IDS_BURN_GET_NAND_CHIP_ID_FAIL));
		break;

	case MESSAGE_SET_CHIP_PARA:
		theConfig.ChangeBurnMode(&ModeCtrl);
		pView->SetStatDesp(port, theApp.GetString(
			TRANSC_MEDIUM_SPIFLASH == ModeCtrl.eMedium ? \
			IDS_BURN_START_SET_SPIFLASH_PARAM :   \
			IDS_BURN_START_SET_NAND_PARAM));
		break;

	case MESSAGE_SET_CHIP_PARA_FAIL:
		theConfig.ChangeBurnMode(&ModeCtrl);
		terminate_mission(port, theApp.GetString(
			TRANSC_MEDIUM_SPIFLASH == ModeCtrl.eMedium ? \
			IDS_BURN_SET_SPIFLASH_PARAM_FAIL :    \
			IDS_BURN_SET_NAND_PARAM_FAIL));
		break;

	case MESSAGE_START_ERASE_TRANSC:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_ERASE));
		break;

	case MESSAGE_ERASE_TRANSC_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_ERASE_FAIL));
		break;

	case MESSAGE_START_FORMAT_TRANSC:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_FORMAT));
		break;

	case MESSAGE_FORMAT_TRANSC_FAIAL:
		terminate_mission(port, theApp.GetString(IDS_BURN_FORMAT_FAIL));
		break;

	case MESSAGE_START_DOWNLOAD_FILE:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_DOWNLOAD_FILE));
		break;
		
	case MESSAGE_DOWNLOAD_FILE_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_DOWNLOAD_FILE_FAIL));
		break;

	case MESSAGE_DOWNLOAD_FILE_LEN:
		{
		}
		break;

	case MESSAGE_BEGIN_GET_AID:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_GET_AID));
		break;

	case MESSAGE_GET_AID_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_GET_AID_FAIL));
		break;

		
	case MESSAGE_UPLOAD_SPIFLASH_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_UPLOAD_SPIFLASH_START));
		break;

	case MESSAGE_DOWNLOAD_BIN_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_DOWNLOAD_BIN_START));
		break;

	case MESSAGE_DOWNLOAD_BIN_FAIL:
		terminate_mission(port, theApp.GetString(IDS_DOWNLOAD_BIN_FAIL));
		break;

	case MESSAGE_DOWNLOAD_BIN_SUCCESS:
		//pView->SetStatDesp(port, theApp.GetString(IDS_BURN_DOWNLOAD_FILE_SUCESS));
		break;
	case MESSAGE_DOWNLOAD_IMG_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_DOWNLOAD_IMG_START));
		break;

	case MESSAGE_DOWNLOAD_IMG_FAIL:
		terminate_mission(port, theApp.GetString(IDS_DOWNLOAD_IMG_FAIL));
		break;

	case MESSAGE_DOWNLOAD_IMG_SUCCESS:
		//pView->SetStatDesp(port, theApp.GetString(IDS_BURN_DOWNLOAD_PRODUCER_SUCESS));
		break;
	case MESSAGE_DOWNLOAD_BOOT_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_DOWNLOAD_BOOT_START));
		break;

	case MESSAGE_DOWNLOAD_BOOT_FAIL:
		terminate_mission(port, theApp.GetString(IDS_DOWNLOAD_BOOT_FAIL));
		break;

	case MESSAGE_DOWNLOAD_BOOT_SUCCESS:
		//pView->SetStatDesp(port, theApp.GetString(IDS_BURN_DOWNLOAD_PRODUCER_SUCESS));
		break;
		
	case MESSAGE_PARTTION_INFORMATION_IS_NULL:
		terminate_mission(port, theApp.GetString(IDS_PARTTION_INFORMATION_IS_NULL));
		break;

	case MESSAGE_GET_MEDIUM_DATAINFO_FAIL:
		terminate_mission(port, theApp.GetString(IDS_GET_MEDIUM_DATAINFO_FAIL));
		break;

	case MESSAGE_GET_FREE_BLOCK_FAIL:
		terminate_mission(port, theApp.GetString(IDS_GET_FREE_BLOCK_FAIL));
		break;

	case MESSAGE_LOW_FORMAT_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_LOW_FORMAT_START));
		break;

	case MESSAGE_LOW_FORMAT_FAIL:
		terminate_mission(port, theApp.GetString(IDS_LOW_FORMAT_FAIL));
		break;
		
	case MESSAGE_MALLOC_MEDIUM_FAIL:
		terminate_mission(port, theApp.GetString(IDS_MALLOC_MEDIUM_FAIL));
		break;
	
	case MESSAGE_WRITE_MAC_ADDR_ASA_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_WRITE_MAC_ADDR_ASA_START));
		break;

	case MESSAGE_WRITE_MAC_ADDR_ASA_FAIL:
		terminate_mission(port, theApp.GetString(IDS_WRITE_MAC_ADDR_ASA_FAIL));
		break;

	case MESSAGE_WRITE_SERIAL_ADDR_ASA_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_WRITE_SERIAL_ADDR_ASA_START));
		break;

	case MESSAGE_WRITE_SERIAL_ADDR_ASA_FAIL:
		terminate_mission(port, theApp.GetString(IDS_WRITE_SERIAL_ADDR_ASA_FAIL));
		break;

 	case MESSAGE_DOWNLOAD_CHANGE_CLK_START:

        //#ifdef SUPPORT_LINUX
		if (1)//theConfig.planform_tpye == E_LINUX_PLANFORM)
		{
		     pView->SetStatColor(port, COLOR_STAT_BURNING);
		}
        //#endif
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_START_DOWNLOAD_CHANGE_CLK));
		break;

	case MESSAGE_DOWNLOAD_CHANGE_CLK_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_DOWNLOAD_CHANGE_CLK_FAIL));
		break;

	case MESSAGE_DOWNLOAD_CHANGECLK_TIMEOUT_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_DOWNLOAD_CHANGE_CLK_TIMEOUT_FAIL));
		break;
    
	case MESSAGE_DOWNLOAD_CHANGE_CLK_USB_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_DOWNLOAD_CHANGE_CLK_USB_FAIL));
		break;

	case MESSAGE_BASEBAND_GPIO_SETTING:
		pView->SetStatDesp(port, theApp.GetString(IDS_BASEBAND_GPIO_SETTING));
		break;

	case MESSAGE_BASEBAND_GPIO_SETTING_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BASEBAND_GPIO_SETTING_FAIL));
		break;
		
	case MESSAGE_SET_PARAM_TO_PRODUCER_FAIL:
		terminate_mission(port, theApp.GetString(IDS_SET_PARAM_TO_PRODUCER_FAIL));
		break;   
    
	case MESSAGE_SET_MODE_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_SET_MODE_START));
		break;
    case MESSAGE_SET_MODE_FAIL:
		terminate_mission(port, theApp.GetString(IDS_SET_MODE_FAIL));
		break;
	case MESSAGE_SET_ERASEMODE_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_SET_ERASEMODE_START));
		break;
    case MESSAGE_SET_ERASEMODE_FAIL:
		terminate_mission(port, theApp.GetString(IDS_SET_ERASEMODE_FAIL));
		break;
    case MESSAGE_SET_NAND_PARA_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_SET_NAND_PARA_START));
		break;
    case MESSAGE_SET_NAND_PARA_FAIL:
		terminate_mission(port, theApp.GetString(IDS_SET_NAND_PARA_FAIL));
		break;
	case MESSAGE_GET_ALL_FREE_BLOCK:
		pView->SetStatDesp(port, theApp.GetString(IDS_GET_ALL_FREE_BLOCK));
		break;  
    case MESSAGE_SET_SEC_AREA_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_SET_SEC_AREA_START));
		break;        
    case MESSAGE_SET_SEC_AREA_FAIL:
		terminate_mission(port, theApp.GetString(IDS_SET_SEC_AREA_FAIL));
		break;
    case MESSAGE_BIN_UPLOAD_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BIN_UPLOAD_FAIL));
		break;
	case MESSAGE_GET_BAD_BLOCK_FAIL:
		terminate_mission(port, theApp.GetString(IDS_GET_BAD_BLOCK_FAIL));
		break;

	case MESSAGE_BURN_MAC_ADDR_READ:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_MAC_ADDR_READ));
		break;
		
	case MESSAGE_BURN_MAC_ADDR_READ_ERROR:
		terminate_mission(port, theApp.GetString(IDS_BURN_MAC_ADDR_READ_ERROR));
		break;

	case MESSAGE_BURN_SERIAL_ADDR_READ:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_SERIAL_ADDR_READ));
		break;
		
	case MESSAGE_BURN_SERIAL_ADDR_READ_ERROR:
		terminate_mission(port, theApp.GetString(IDS_BURN_SERIAL_ADDR_READ_ERROR));
		break;

	case MESSAGE_BURN_MAC_ADDR_COMPARE:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_MAC_ADDR_COMPARE));
		break;
		
	case MESSAGE_BURN_MAC_ADDR_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_MAC_ADDR_FAIL));
		break;
		
	case MESSAGE_BURN_SERIAL_ADDR_COMPARE:
		pView->SetStatDesp(port, theApp.GetString(IDS_BURN_SERIAL_ADDR_COMPARE));
		break;
		
	case MESSAGE_BURN_SERIAL_ADDR_FAIL:
		terminate_mission(port, theApp.GetString(IDS_BURN_SERIAL_ADDR_FAIL));
		break;

    case MESSAGE_SET_RESV_AREA_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_SET_RESV_AREA_START));
		break;
    case MESSAGE_SET_RESV_AREA_FAIL:
		terminate_mission(port, theApp.GetString(IDS_SET_RESV_AREA_FAIL));
		break;
    case MESSAGE_CREATE_PARTITION_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_CREATE_PARTITION_START));
		break;        
    case MESSAGE_CREATE_PARTITION_FAIL:
		terminate_mission(port, theApp.GetString(IDS_CREATE_PARTITION_FAIL));
		break;
    case MESSAGE_CLOSE_START:
		pView->SetStatDesp(port, theApp.GetString(IDS_CLOSE_START));
		break;
    case MESSAGE_CLOSE_FAIL:
		terminate_mission(port, theApp.GetString(IDS_CLOSE_FAIL));
		break;
	case MESSAGE_SET_NAND_GPIOCE_FAIL:
		terminate_mission(port, theApp.GetString(IDS_SET_NAND_GPIOCE_FAIL));
		break;
	case MESSAGE_DOWNLOAD_CHANNELID_FAIL:
		terminate_mission(port, theApp.GetString(IDS_DOWNLOAD_CHANNELID_FAIL));
		break;
	case MESSAGE_SET_COMMODE_FAIL:
		terminate_mission(port, theApp.GetString(IDS_SET_COMMODE_FAIL));
		break;
	case MESSAGE_DOWNLOAD_COMADDR_FAIL:
		terminate_mission(port, theApp.GetString(IDS_DOWNLOAD_COMADDR_FAIL));
		break;

		//设置通道的地址
	case MESSAGE_START_SET_CHANNELID:
	//	#ifdef SUPPORT_LINUX
		if (theConfig.planform_tpye == E_LINUX_PLANFORM)
		{
			pView->SetStatColor(port, COLOR_STAT_BURNING);
		}
        //#endif
		pView->SetStatDesp(port, theApp.GetString(IDS_SET_CHANNELID));
		break;

		//通道的地址错误
	case MESSAGE_START_CHANNELID_FAIL:
		terminate_mission(port, theApp.GetString(IDS_CHANNELID_FAIL));
		break; 

    case MESSAGE_SEND_CMD_ANYKA:
        SendCmdToResetUSB(wp);
        break;

	//在线进行镜像制作
	case MESSAGE_START_IMAGE_CREATE:
		pView->SetStatDesp(port, theApp.GetString(IDS_START_IMAGE_CREATE));
		if(g_img_stat == E_IMG_INIT)
		{
			DWORD tid;
			g_img_stat = E_IMG_CREATING;
			m_image_create.nID = port+1;//由于上面已减1，为了配对之前的用法，所以这里加1
			if (theConfig.planform_tpye == E_ROST_PLANFORM && ((theConfig.chip_type == CHIP_1080L) || (theConfig.chip_type == CHIP_10XXC)))
			{
				CreateThread(NULL, 0, online_image_create_thread_snowdirdL, &m_image_create, 0, &tid);
			} 
			else
			{
				CreateThread(NULL, 0, online_image_create_thread, &m_image_create, 0, &tid);
			}
			
		}
		break;
		
	case MESSAGE_IMAGE_CREATE_SUCCESS:
		pView->SetStatDesp(port, theApp.GetString(IDS_IMAGE_CREATE_SUCCESS));
		g_img_stat = E_IMG_SUCCESS;
		SetEvent(image_event);
		break;
	
	case MMESSAGE_MEDIUM_CAPACITY_CHECK:
		pView->SetStatDesp(port, theApp.GetString(IDS_MEDIUM_CAPACITY_CHECK));
		break;
	
	case MESSAGE_UPLAOD_SPIFLASH_FAIL:
		terminate_mission(port, theApp.GetString(IDS_SPIFLASH_UPLAOD_FAIL));
		break;

	case MESSAGE_MEDIUM_CAPACITY_FAIL:
		terminate_mission(port, theApp.GetString(IDS_MEDIUM_CAPACITY_FAIL));
		break;
		
	case MESSAGE_IMAGE_CREATE_FAIL:
		terminate_mission(port, theApp.GetString(IDS_IMAGE_CREATE_FAIL));
		//pView->SetStatDesp(port, theApp.GetString(IDS_IMAGE_CREATE_FAIL));
		g_img_stat = E_IMG_FAIL;
		SetEvent(image_event);
		break;

	case MESSAGE_IMAGE_CREATE_RESET:
		pView->SetStatDesp(port, theApp.GetString(IDS_IMAGE_CREATE_RESET));
		 ((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_ONLINE_IMAGE_MAKE))->SetCheck(FALSE);
		break;

	default:
		break;
	}

	return 0;
}

LRESULT CMainFrame::On_DeviceArrive(WPARAM wp, LPARAM lp)
{
	CBurnToolView *pView = (CBurnToolView *)GetActiveView();
    
    m_cur_link++;
    RefreshWorkInfo();

    if (!theConfig.bUDiskUpdate)
	 {
		PostMessage(ON_BURNFLASH_MESSAGE, wp+100, LPARAM(MESSAGE_SET_STANDBY));
	 }
	 else
	 {
        /*变颜色 还没有测试验证过*/
 		 //SetupDownload();
/*
		 pView->SetStatDesp(wp-1, theApp.GetString(IDS_BURN_UDISK_UPDATE_READY));
		 pView->SetStatColor(wp-1, COLOR_STAT_STANDBY);
		 pView->SetProgPos(wp-1, 0);
		 pView->SetProgColor(wp-1, COLOR_PROG_NORMAL);
		 pView->SetTime(wp-1, 0);

         Sleep(100);
		 
		 if ((g_workTotalnum = BT_Start()) != 0)
         {
             g_workThreadnum = 0;
         }
*/
	 }

    /*
	if(0 == m_worknum && theConfig.bUboot)
	{
//		int i;
		CString dbcc_name = g_arrive_name[wp];
		g_arrive_name[wp].Empty();
	
	}
	else
	{
		if(m_worknum > 0)
		{
			SetEvent(usb_init_event);
		}
	}
	*/
    
    return 0;
}

LRESULT CMainFrame::On_DeviceRemove(WPARAM wp, LPARAM lp)
{
	CString dbcc_name = g_remove_name[wp];
	g_remove_name[wp].Empty();

//	int i;
    // if (m_cur_link > 0) m_cur_link--;
	return 0;
}

LRESULT CMainFrame::On_Receive(WPARAM wp, LPARAM lp)
{
//	int port, len;
//	int receive_len;

//	port = wp - theConfig.com_base - 100;

	USES_CONVERSION;

	return 0;
}
/*
void CMainFrame::DisplayWorkNO()
{
	CString str;
	str.Format(_T("%d"),m_now_link);
	m_DlgBar.SetDlgItemText(IDC_EDIT_NOW_LINK,str);
	str.Format(_T("%d"),m_total_link);
	m_DlgBar.SetDlgItemText(IDC_EDIT_TOTAL_LINK,str);
	str.Format(_T("%d"),m_now_pass);
	m_DlgBar.SetDlgItemText(IDC_EDIT_NOW_PASS,str);
	str.Format(_T("%d"),m_total_pass);
	m_DlgBar.SetDlgItemText(IDC_EDIT_TOTAL_PASS,str);
}
*/
void CMainFrame::OnUpdateViewCom(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	BOOL bVisible = ((m_ComBar.GetStyle() & WS_VISIBLE) != 0);
	pCmdUI->SetCheck(bVisible);
}

void CMainFrame::ActivateFrame(int nCmdShow) 
{
	// TODO: Add your specialized code here and/or call the base class
	nCmdShow = SW_SHOWMAXIMIZED;
	
	CFrameWnd::ActivateFrame(nCmdShow);
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		ShowControlBar(&m_ComBar, FALSE, FALSE);
	}
	return CFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnSettingChangePasswd() 
{
	// TODO: Add your command handler code here
	CDlgPasswordChange dlgChngPasswd;
    CString str;

	USES_CONVERSION;

	dlgChngPasswd.m_user_id = theApp.GetString(IDS_USER_PRODUCER);
	if(dlgChngPasswd.DoModal() == IDOK)
	{
		if(dlgChngPasswd.m_password_new == dlgChngPasswd.m_password_confirm)
		{
			if(strrdid == theApp.GetString(IDS_USER_PRODUCER))
			{
				if(dlgChngPasswd.m_password_old == theConfig.passwd_ctrl.pm_password)
				{
					_tcsncpy(theConfig.passwd_ctrl.pm_password, dlgChngPasswd.m_password_new, MAX_PASSWD_LENGTH);

					if(theConfig.StorePassword())
					{
                        str = theApp.GetString(IDS_PASSWORD_MODIFY_SUCCESS);
						MessageBox(str);
					}
					else
					{
						_tcsncpy(theConfig.passwd_ctrl.pm_password, dlgChngPasswd.m_password_old, MAX_PASSWD_LENGTH);
                        
						MessageBox(_T("Error In Storing Password!"), NULL, MB_ICONERROR|MB_OK);
					}
				}
				else
				{
                    str = theApp.GetString(IDS_PASSWORD_ERROR);
					MessageBox(str, NULL, MB_ICONERROR|MB_OK);
				}
			}
			else
			{
				if(dlgChngPasswd.m_password_old == theConfig.passwd_ctrl.rd_password)
				{
					_tcsncpy(theConfig.passwd_ctrl.rd_password, dlgChngPasswd.m_password_new, MAX_PASSWD_LENGTH);
					if(theConfig.StorePassword())
					{
                        str = theApp.GetString(IDS_PASSWORD_MODIFY_SUCCESS);
						MessageBox(str);
					}
					else
					{
						_tcsncpy(theConfig.passwd_ctrl.rd_password, dlgChngPasswd.m_password_old, MAX_PASSWD_LENGTH);
						MessageBox(_T("Error In Storing Password!"), NULL, MB_ICONERROR|MB_OK);
					}
				}
				else
				{
                    str = theApp.GetString(IDS_PASSWORD_ERROR);

					MessageBox(str, NULL, MB_ICONERROR|MB_OK);
				}
			}			
		}
		else
		{
            str = theApp.GetString(IDS_PASSWORD_CONFIRM_ERROR);
			MessageBox(str, NULL, MB_ICONERROR|MB_OK);
		}
	}
}

void CMainFrame::find_resource(LPCTSTR resource_path, LPCTSTR dest_path, BOOL compare)
{
	if(resource_path == NULL || dest_path == NULL)
		return;

	CString path = resource_path;

	if(path.Right(1) != '*')
	{
		if(path.Right(1) != '\\')
			path += __T("\\");
		path += __T("*");
	}

	WIN32_FIND_DATA find_data;
	HANDLE fp = FindFirstFile(path, &find_data ); 
	if(fp != INVALID_HANDLE_VALUE)
	{
		while(FindNextFile(fp, &find_data ) != 0)
		{
			if(g_download_udisk_count >= MAX_DOWNLOAD_FILES)
			{
				FindClose(fp);
				return;
			}			

			if(_tcscmp(find_data.cFileName, _T("..")) == 0 || _tcscmp(find_data.cFileName, _T(".")) == 0)
				continue;

			g_download_udisk[g_download_udisk_count].bCompare = compare;

			_tcscpy(g_download_udisk[g_download_udisk_count].pc_path, resource_path);
			if(resource_path[_tcslen(resource_path) - 1] != '\\')
				_tcscat(g_download_udisk[g_download_udisk_count].pc_path, _T("\\"));
			
			_tcscat(g_download_udisk[g_download_udisk_count].pc_path, find_data.cFileName);

			_tcscpy(g_download_udisk[g_download_udisk_count].udisk_path, dest_path);
			if(g_download_udisk[g_download_udisk_count].udisk_path[_tcslen(g_download_udisk[g_download_udisk_count].udisk_path) - 1] != '/')
				_tcscat(g_download_udisk[g_download_udisk_count].udisk_path, _T("/"));

			//char *path = strstr(download_resource[download_resource_count].file_pc_path, original_path);

			//if(path != NULL)
			_tcscat(g_download_udisk[g_download_udisk_count].udisk_path, find_data.cFileName);

			g_download_udisk_count++;

			if(g_download_udisk_count >= MAX_DOWNLOAD_FILES)
			{
				FindClose(fp);
				return;
			}

			if((find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				find_resource(g_download_udisk[g_download_udisk_count - 1].pc_path,
					g_download_udisk[g_download_udisk_count - 1].udisk_path, compare);
			}
		}
		FindClose(fp);
	}
}

void CMainFrame::SetupDownload()
{
	UINT dwAttribute;
	UINT i;
	CString str;

	//reset
	g_download_nand_count = 0;
	g_download_udisk_count = 0;

	//setup download nand
	g_download_nand_count = theConfig.download_nand_count;
	memcpy(g_download_nand, theConfig.download_nand_data, g_download_nand_count*sizeof(T_DOWNLOAD_NAND));

	//setup download mtd
	g_download_mtd_count = theConfig.download_mtd_count;
	memcpy(g_download_mtd, theConfig.download_mtd_data, g_download_mtd_count*sizeof(T_DOWNLOAD_MTD));

	//setup download udisk
	for(i = 0; i < theConfig.download_udisk_count; i++)
	{
		dwAttribute = GetFileAttributes(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[i].pc_path));
		if(0xFFFFFFFF == dwAttribute)
		{
			str.Format(_T("Cannot find the path: %s"), theApp.ConvertAbsolutePath(theConfig.download_udisk_data[i].pc_path));
			MessageBox(str);
			break;
		}
		else
		{
			if((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				find_resource(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[i].pc_path), 
					theConfig.download_udisk_data[i].udisk_path, 
					theConfig.download_udisk_data[i].bCompare);
			}
			else
			{
				g_download_udisk[g_download_udisk_count++] = theConfig.download_udisk_data[i];
			}
		}
	}

	// maybe need to count file size
	// will add it later
	CountDownloadLength();
	
	theConfig.ConfigNandbootCmdline(!(theConfig.cmdLine.bCmdLine));
}


void CMainFrame::CountDownloadLength()
{
	UINT i;
	HANDLE hFile;
	DWORD dwSize;
	DWORD dwAttribute;
	CString str;

	if(E_CONFIG_JTAG == theConfig.burn_mode)
	{
		return;
	}

	//init
	download_length_data = 0;
	download_length_mtd = 0;
	download_length_udisk = 0;
	download_length_new_mtd = 0;
	bin_pos = 0;

	//add produce length
    hFile = CreateFile(theApp.ConvertAbsolutePath(theConfig.path_producer), GENERIC_READ , FILE_SHARE_READ , NULL , 
					OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		dwSize = GetFileSize(hFile, NULL);
		if(dwSize != 0xFFFFFFFF)
		{
			download_length_data += dwSize;
		}
		else
		{
			str.Format(_T("Cannot find file: %s"), theConfig.path_producer);
			MessageBox(str);

			return;
		}

		
		DWORD read_len;
		BYTE buf[4] = {0};
		SetFilePointer(hFile, PRODUCE_VERSION_CKECK_OFFSET, NULL, FILE_BEGIN);
		ReadFile(hFile, buf, 4, &read_len, NULL);
		if(buf[0] == VER_CHECK0 && buf[1] == VER_CHECK1 && buf[2] == VER_CHECK2 && buf[3] == VER_CHECK3)
		{
			memset(&theConfig.version_ctrl, 0, sizeof(T_VERSION_DATA));
			SetFilePointer(hFile, PRODUCE_VERSION_DATA_OFFSET, NULL, FILE_BEGIN);
			ReadFile(hFile, &theConfig.version_ctrl, sizeof(T_VERSION_DATA), &read_len, NULL);	
		}
		else
		{
			memset(&theConfig.version_ctrl, 0, sizeof(T_VERSION_DATA));
		}

		CloseHandle(hFile);
	}
	else
	{
		str.Format(_T("Cannot find file: %s"), theConfig.path_producer);
		MessageBox(str);

		return;
	}

	if(E_CONFIG_DEBUG == theConfig.burn_mode)
	{
		return;
	}

	//add nandboot size
	hFile = CreateFile(theApp.ConvertAbsolutePath(theConfig.path_nandboot_new), GENERIC_READ , FILE_SHARE_READ , NULL , 
					OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		dwSize = GetFileSize(hFile, NULL);
		if(dwSize != 0xFFFFFFFF)
		{
			download_length_data += dwSize;
		}
		else
		{
			str.Format(_T("Cannot find file: %s"), theConfig.path_nandboot_new);
			MessageBox(str);

			return;
		}

		CloseHandle(hFile);
	}
	else
	{
		hFile = CreateFile(theApp.ConvertAbsolutePath(theConfig.path_nandboot), GENERIC_READ , FILE_SHARE_READ , NULL , 
			OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				download_length_data += dwSize;
			}
			else
			{
				str.Format(_T("Cannot find file: %s"), theConfig.path_nandboot);
				MessageBox(str);
				
				return;
			}
			
			CloseHandle(hFile);
		}
		else
		{
			str.Format(_T("Cannot find file: %s"), theConfig.path_nandboot);
			MessageBox(str);	
			return;
		}
	}
	
	//add bios size
/*	hFile = CreateFile(theApp.ConvertAbsolutePath(theConfig.path_bios), GENERIC_READ , FILE_SHARE_READ , NULL , 
					OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		dwSize = GetFileSize(hFile, NULL);
		if(dwSize != 0xFFFFFFFF)
		{
			total_download_length += dwSize*2;
		}
		else
		{
			str.Format(_T("Cannot find file: %s"), theConfig.path_bios);
			MessageBox(str);

			return;
		}

		CloseHandle(hFile);
	}
	else
	{
		str.Format(_T("Cannot find file: %s"), theConfig.path_bios);
		MessageBox(str);

		return;
	}
*/
	//add size of files download to nand
	for(i = 0; i < g_download_nand_count; i++)
	{
		hFile = CreateFile(theApp.ConvertAbsolutePath(g_download_nand[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
						OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				download_length_data += dwSize;
			}
			else
			{
				str.Format(_T("Cannot find file: %s"), g_download_nand[i].pc_path);
				MessageBox(str);

				return;
			}

			CloseHandle(hFile);
		}
		else
		{
			str.Format(_T("Cannot find file: %s"), g_download_nand[i].pc_path);
			MessageBox(str);

			return;
		}
	}
	
	//add size of files download to mtd
	for(i = 0; i < g_download_mtd_count; i++)
	{
		dwAttribute = GetFileAttributes(theApp.ConvertAbsolutePath(g_download_mtd[i].pc_path));
		if(dwAttribute == 0xFFFFFFFF)
		{
			//if (theConfig.chip_type != CHIP_11XX && theConfig.planform_tpye != E_ROST_PLANFORM)
			//{
				str.Format(_T("Cannot find file: %s"), g_download_mtd[i].pc_path);
				MessageBox(str);
				return;
			//}
			//else
			//{
			//	break;//由于11平台是在线制作镜像，每一个通道的镜像名不相同，所以此处不需要再进行判断镜像文件是否存在
			//}
			
		}

		if((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
            DWORD FileCnt = 0, FileSize = 0, MinSize = (DWORD)(-1);

            if (!GetFileInfoInDir(theApp.ConvertAbsolutePath(g_download_mtd[i].pc_path), &FileCnt, &FileSize, &MinSize))
                return;
            /*由于文件夹不确定是哪个镜像文件，所以这里只找一个字节最小的文件来模糊的统计总容量*/
            if (FileCnt)
            {
                download_length_mtd += MinSize;
            }

			continue;
		}

		hFile = CreateFile(theApp.ConvertAbsolutePath(g_download_mtd[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
						OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
        
		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				download_length_mtd += dwSize;
			}
			else
			{
				str.Format(_T("Cannot find file: %s"), g_download_mtd[i].pc_path);
				MessageBox(str);

				return;
			}

			CloseHandle(hFile);
		}
		else
		{
			str.Format(_T("Cannot find file: %s"), g_download_mtd[i].pc_path);
			MessageBox(str);

			return;
		}
	}

	//add size of files download to udisk
	for(i = 0; i < g_download_udisk_count; i++)
	{
		dwAttribute = GetFileAttributes(theApp.ConvertAbsolutePath(g_download_udisk[i].pc_path));
		if(dwAttribute == 0xFFFFFFFF)
		{
			str.Format(_T("Cannot find file: %s"), g_download_udisk[i].pc_path);
			MessageBox(str);
			
			return;
		}

		if((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
            DWORD FileCnt = 0, FileSize = 0;

            if (!GetFileInfoInDir(theApp.ConvertAbsolutePath(g_download_mtd[i].pc_path), &FileCnt, &FileSize, NULL))
                return;

            if (FileCnt)
            {
                download_length_udisk += FileSize;
            }
            
			continue;
		}

		hFile = CreateFile(theApp.ConvertAbsolutePath(g_download_udisk[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
						OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				download_length_udisk += dwSize;
			}
			else
			{
				str.Format(_T("Cannot find file: %s"), g_download_udisk[i].pc_path);
				MessageBox(str);

				return;
			}

			CloseHandle(hFile);
		}
		else
		{
			str.Format(_T("Cannot find file: %s"), g_download_udisk[i].pc_path);
			MessageBox(str);

			return;
		}
	}

	//check fls and eep file
	if(theConfig.DownloadMode != E_DOWNLOAD_AK_ONLY)
	{
		//check fls
		dwAttribute = GetFileAttributes(theApp.ConvertAbsolutePath(theConfig.path_fls));
		if(dwAttribute == 0xFFFFFFFF)
		{
			str.Format(_T("Cannot find fls file: %s"), theConfig.path_fls);
			MessageBox(str);
			
			return;
		}

		//check eep
		dwAttribute = GetFileAttributes(theApp.ConvertAbsolutePath(theConfig.path_eep));
		if(dwAttribute == 0xFFFFFFFF)
		{
			str.Format(_T("Cannot find eep file: %s"), theConfig.path_eep);
			MessageBox(str);
			
			return;
		}	
	}
}

void CMainFrame::OnMacAddr(int port, BOOL falg) 
{
	UINT tempmac = 0;
	CString str;
	TCHAR buf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	CBurnToolView *pView = (CBurnToolView *)GetActiveView();

	_tcsncpy(buf, theConfig.mac_current_high, MAX_MAC_SEQU_ADDR_COUNT);
	_tcscat(buf, _T(":"));
	_tcscat(buf, theConfig.g_mac_show_current_low[port]);

	pView->SetCurrentMacAddr(port, buf);
	
}

void CMainFrame::OnSequenceAddr(int port, BOOL falg) 
{
	UINT tempsequence = 0;
	TCHAR buf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	
	
	USES_CONVERSION;

	CBurnToolView *pView = (CBurnToolView *)GetActiveView();
	_tcsncpy(buf, theConfig.sequence_current_high, MAX_MAC_SEQU_ADDR_COUNT);
	_tcscat(buf, theConfig.g_sequence_show_current_low[port]);
	
	pView->SetCurrentSequenceAddr(port, buf);
	
}


//中断
void CMainFrame::terminate_mission(int id, LPCTSTR str)
{
	CBurnToolView *pView = (CBurnToolView *)GetActiveView();

	pView->SetStatDesp(id, str);
	pView->SetStatColor(id, COLOR_STAT_FAIL);

	pView->SetProgColor(id, COLOR_PROG_FAIL);

	g_capacity_size[id - 1] = 0;

    m_total_fail++;
    if (m_worknum != 0)
    {
        frmLogfile.WriteLogFile(0,"!!terminate\n");
		
        if ( m_cur_link != 0)
        {
            m_cur_link--;
        }
		burn_auto[id] = 0;
		
        m_worknum--;
		if (g_capacity_burnnum == m_worknum)
		{
			SetEvent(capacity_event);
		}
        if (g_workTotalnum != 0)
            g_workTotalnum--;

        m_burn_time[id].nID = 0;    // close timer
        if (0 == m_worknum)
        {
			g_bEraseMode = FALSE;
			g_bUploadbinMode = FALSE;   // upload bin mode not effect!
			g_bUpload_spialldata = FALSE;
			m_budisk_burn = FALSE;      //if no intern udiskburn
			m_budisk_getUSBnum = FALSE;
			USB_attachflag = FALSE;    //恢复可以查找usb设备的标志
			g_capacity_flag = AK_TRUE;
    		enable_control();
        }
    }
    // m_cur_useful = theConfig.device_num - m_cur_link;
    RefreshWorkInfo();

	//set window title
	set_window_title();

	if(!theConfig.bUboot)
	{
	}
}

void CMainFrame::OnDestroy() 
{
	CAKFS cAK;

	CFrameWnd::OnDestroy();
	
	// TODO: Add your message handler code here
//	int i;

//	for(i = 0; i < m_device_num; i++)
//	{
//	}


	StopTimer();

	theApp.m_lang.WriteString();
	WritePrivateProfileString(_T("Lang"), _T("Lang"),  
				theApp.m_lang.m_langName[theApp.m_lang.m_activeLang], 
				theApp.ConvertAbsolutePath(_T("burn.ini")));
    WritePrivateProfileString(_T("Open"),_T("Open"),_T("N"),theApp.ConvertAbsolutePath(_T("burn.ini")));
    WritePrivateProfileString(_T("Config"),_T("File Name"),theApp.m_config_file[theApp.m_config_index],
                theApp.ConvertAbsolutePath(_T("burn.ini")));
	theConfig.WriteConfig(theApp.m_config_file[theApp.m_config_index]);

	theConfig.ConfigNandbootCmdline(TRUE);
    if (INVALID_HANDLE_VALUE != theConfig.m_hMutex)
    {
        CloseHandle(theConfig.m_hMutex);
        theConfig.m_hMutex = INVALID_HANDLE_VALUE;
    }
	if (INVALID_HANDLE_VALUE != theConfig.m_hMutGetAddr)
    {
        CloseHandle(theConfig.m_hMutGetAddr);
        theConfig.m_hMutGetAddr = INVALID_HANDLE_VALUE;
    }

    ::RemoveProp(m_hWnd, theApp.m_Open_prop);

	cAK.Destroy();
}

void CMainFrame::disable_control()
{
    m_DlgBar.EnableWindow(FALSE);
	m_ToolBar.EnableWindow(FALSE);
}

void CMainFrame::enable_control()
{
	//int i;

	g_img_stat = E_IMG_INIT;
	ResetEvent(image_event);
	ResetEvent(udiskburn_event);
	ResetEvent(ResetDevice_event);
	ResetEvent(capacity_event);
	//ResetEvent(image_event);
	download_length_new_mtd = 0;
    m_DlgBar.EnableWindow(TRUE);
	m_ToolBar.EnableWindow(TRUE);
	memset(m_burn_time, 0, sizeof(m_burn_time));
	memset(g_diskname_No, 0, 32);
}

void CMainFrame::StartTimer()
{
	m_main_timer = SetTimer(TIMER_BURN, 1000, 0);
	m_udisk_timer = SetTimer(TIMER_UDISK, 1000, 0);
	memset(m_burn_time, 0, sizeof(m_burn_time));
}

void CMainFrame::StopTimer()
{
	KillTimer(m_main_timer);
	KillTimer(m_udisk_timer);
	m_time_count = 0;
}


void CMainFrame::OnTimer(UINT nIDEvent) 
{
	CBurnToolView *pView = (CBurnToolView *)GetActiveView();
	//烧录时间
	if (TIMER_BURN == nIDEvent)
	{
		g_timer_counter++;
		for(UINT i = 0; i < theConfig.device_num; i++)
		{
			if(0 != m_burn_time[i].nID)
			{
				m_burn_time[m_burn_time[i].nID-1].time_count++;
				pView->SetTime(m_burn_time[i].nID-1, m_burn_time[m_burn_time[i].nID-1].time_count);
			}
		}
	}
	//u盘事件
	if (TIMER_UDISK == nIDEvent)
	{
		CUpdateBase update;
		char diskName[32];
		CString str_udisk;
		CString str_usb;
		int udisk_cnt =0;
		int usb_cnt = 0;
		//UINT i = 0;
		//TCHAR strDisk[4];
		//UINT flag = 0;
		//UINT idex = 0;

		//这里是获取U盘的个数
		udisk_cnt = update.GetUDisk(diskName, 32);

		str_udisk.Format(_T("%d"),udisk_cnt);
		m_DlgBar.SetDlgItemText(IDC_EDIT_UDISK, str_udisk);

		//这里是获取USB设备的个数
		if (FALSE == USB_attachflag)
		{
			usb_cnt = BT_GetUSB_Num(theConfig.PID, theConfig.VID);
			m_USBnum = usb_cnt; 
		}
		
		//str.Format(_T("%d"),udisk_cnt);
		str_usb.Format(_T("%d"),m_USBnum);
		m_DlgBar.SetDlgItemText(IDC_EDIT_UDISK_NUM,str_usb);
	}

	CFrameWnd::OnTimer(nIDEvent);
}

//烧录工具的标题修改
void CMainFrame::set_window_title()
{
	CString str;
	CString strSub;

	if (theConfig.planform_tpye == E_ROST_PLANFORM)
	{
		str.Format(_T("%s V%d.%d.%02d.%02d P:V%d.%d.%02d"), theApp.GetString(IDS_TITLE), MAIN_VERSION, SUB_VERSION0, SUB_VERSION1, SUB_VERSION2, 
			theConfig.version_ctrl.main_version, theConfig.version_ctrl.sub_version1, theConfig.version_ctrl.sub_version2);
	}
	if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	{
		str.Format(_T("%s V%d.%d.%02d.%02d"), theApp.GetString(IDS_TITLE), MAIN_VERSION, SUB_VERSION0, SUB_VERSION1, SUB_VERSION2);
	}
	
	switch (theConfig.burn_mode)
	{
		case E_CONFIG_NAND:
			str += _T("<NAND BURN>");//nand
			break;
		case E_CONFIG_SPI_NAND:
			str += _T("<spi NAND BURN>");//nand
			break;
		case E_CONFIG_SFLASH:
			str += _T("<SFlash BURN>");//spi
			break;
		case E_CONFIG_SD:
			str += _T("<SD BURN>");//sd
			break;
		case E_CONFIG_DEBUG:
			str += _T("<DEBUG>");//debug
			break;
		case E_CONFIG_JTAG:
			str += _T("<JTAG>");//jtag
			break;
	}

	if (theConfig.bUpdate)
	{
		str += _T("update");//bUpdate
	}

	if (g_bUploadbinMode)
	{
		str += _T("bin upload");// bin 回读
	}

	if (g_bUpload_spialldata)
	{
		str += _T("upload spi data");//  
	}

//#ifdef SUPPORT_LINUX
	//if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	{
		if (theConfig.bUDiskUpdate)
		{
			str += _T("--UDiskBurn");//bUDiskUpdate
		}
	}
//#endif
	
	SetWindowText(str);		
}

void CMainFrame::SetupDisplay()
{
	int nCurPos = 0;
	CMenu *pMainMenu = GetMenu();
	CMenu *pSubMenu;
#if 1
	//int iUserMode = theApp.GetUserMode();

	pSubMenu = pMainMenu->GetSubMenu(0);
	pMainMenu->ModifyMenu(0, MF_BYPOSITION, 0, theApp.GetString(IDS_MENU_UPDATE));//IDS_MENU_UPDATE
	pSubMenu->ModifyMenu(ID_UPDATE_EXPORT, MF_BYCOMMAND, 
		ID_UPDATE_EXPORT, theApp.GetString(IDS_MENU_UPDATE_EXPORT));//IDS_MENU_UPDATE_EXPORT
	pSubMenu->ModifyMenu(ID_UPDATE_IMPORT, MF_BYCOMMAND, 
		ID_UPDATE_IMPORT, theApp.GetString(IDS_MENU_UPDATE_IMPORT));//IDS_MENU_UPDATE_IMPORT

	pSubMenu = pMainMenu->GetSubMenu(1);
	pMainMenu->ModifyMenu(1, MF_BYPOSITION, 1, theApp.GetString(IDS_MENU_SETTING));//IDS_MENU_SETTING
	pSubMenu->ModifyMenu(0, MF_BYPOSITION, 0, theApp.GetString(IDS_MENU_LANGUAGE));//IDS_MENU_LANGUAGE
	pSubMenu->ModifyMenu(ID_CONFIG_SETTING, MF_BYCOMMAND, 
		ID_CONFIG_SETTING, theApp.GetString(IDS_CONFIG_SETTING));//IDS_CONFIG_SETTING
	pSubMenu->ModifyMenu(ID_SETTING_CHANGE_PASSWD, MF_BYCOMMAND, 
		ID_SETTING_CHANGE_PASSWD, theApp.GetString(IDS_MENU_MODIFY_PASSWD));//IDS_MENU_MODIFY_PASSWD

	pSubMenu = pMainMenu->GetSubMenu(2);
	pMainMenu->ModifyMenu(2, MF_BYPOSITION, 2, theApp.GetString(IDS_MENU_TOOL));//IDS_MENU_TOOL
	pSubMenu->ModifyMenu(ID_TOOL_IMAGE_MAKER, MF_BYCOMMAND, 
		ID_TOOL_IMAGE_MAKER, theApp.GetString(IDS_MENU_CREATIMAGE_TOOL));//IDS_MENU_CREATIMAGE_TOOL
	pSubMenu->ModifyMenu(ID_SPI_IMAGE_MAKER, MF_BYCOMMAND, 
		ID_SPI_IMAGE_MAKER, theApp.GetString(IDS_SPI_IMAGE_MAKER));//IDS_SPI_IMAGE_MAKER
	pSubMenu->ModifyMenu(ID_BIN_READ_BACK, MF_BYCOMMAND, 
		ID_BIN_READ_BACK, theApp.GetString(IDS_BIN_READ_BACK));//IDS_BIN_READ_BACK
//#ifdef  SUPPORT_LINUX
	if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	{
		pSubMenu->DeleteMenu(ID_TOOL_IMAGE_MAKER, MF_BYCOMMAND);//ID_TOOL_IMAGE_MAKER
	}
//#endif

	pSubMenu = pMainMenu->GetSubMenu(3);
	pMainMenu->ModifyMenu(3, MF_BYPOSITION, 3, theApp.GetString(IDS_MENU_CONFIG));//IDS_MENU_CONFIG

	pSubMenu = pMainMenu->GetSubMenu(4);
	pMainMenu->ModifyMenu(4, MF_BYPOSITION, 4, theApp.GetString(IDS_MENU_HELP));//IDS_MENU_HELP
	pSubMenu->ModifyMenu(ID_APP_ABOUT, MF_BYCOMMAND, 
		ID_APP_ABOUT, theApp.GetString(IDS_MENU_ABOUT));

	pSubMenu->ModifyMenu(ID_APP_HELP, MF_BYCOMMAND, 
		ID_APP_HELP, theApp.GetString(IDS_MENU_HELP));
#endif

	DrawMenuBar();
    
    m_DlgBar.SetDlgItemText(IDC_STATIC_CUR_NUM, theApp.GetString(IDS_MAINVIEW_CUR_NUM));//当前个数
    m_DlgBar.SetDlgItemText(IDC_STATIC_CUR_USEFUL,theApp.GetString(IDS_MAINVIEW_CUR_USEFUL));//当前使用个数
    m_DlgBar.SetDlgItemText(IDC_STATIC_CUR_PASS,theApp.GetString(IDS_MAINVIEW_CUR_PASS));//当前通过个数
    m_DlgBar.SetDlgItemText(IDC_STATIC_TOTAL_PASS,theApp.GetString(IDS_MAINVIEW_TOTAL_PASS));//总数
    m_DlgBar.SetDlgItemText(IDC_STATIC_TOTAL_FAIL,theApp.GetString(IDS_MAINVIEW_TOTAL_FAIL));//失败个数
    m_DlgBar.SetDlgItemText(IDC_BUTTON_CLR_REC,theApp.GetString(IDS_MAINVIEW_CLR_REC));
	m_DlgBar.SetDlgItemText(IDC_STATIC_STATE,theApp.GetString(IDS_STATIC_STATE));//状态
	m_DlgBar.SetDlgItemText(IDC_STATIC_NOW_UDISK,theApp.GetString(IDS_STATIC_NOW_UDISK));//USB
	m_DlgBar.SetDlgItemText(IDC_STATIC_UDISK,theApp.GetString(IDS_STATIC_UDISK_NUM));//U盘
	m_DlgBar.SetDlgItemText(IDC_CHECK_AUTO_BURN,theApp.GetString(IDS_AUTO_BURN));//自动烧录
	m_DlgBar.SetDlgItemText(IDC_CHECK_UDISK_BURN,theApp.GetString(IDS_UDISK_BURN));//U盘烧录
	m_DlgBar.SetDlgItemText(IDC_CHECK_ONLINE_IMAGE_MAKE,theApp.GetString(IDS_ONLINE_IMAGE_MAKE));//在制作

	m_ToolBar.SetButtonText(0, theApp.GetString(IDS_TOOLBAR_START));
	m_ToolBar.SetButtonText(1, theApp.GetString(IDS_TOOLBAR_SETTING));
	m_ToolBar.SetButtonText(2, theApp.GetString(IDS_TOOLBAR_IMPORT));
	m_ToolBar.SetButtonText(3, theApp.GetString(IDS_TOOLBAR_EXPORT));
	//m_ToolBar.SetButtonText(4, theApp.GetString(IDS_TOOLBAR_FORMAT));

	m_ToolBar.SetSizes(CSize(60,60), CSize(32,36));	
	Invalidate();

	CBurnToolView *pView = (CBurnToolView *)GetActiveView();
	if(pView)
	{
		pView->SetupDisplay();
	}

	set_window_title();
}

void CMainFrame::SetConfigMenu()
{
    CMenu  *pSubMenu = GetMenu()->GetSubMenu(3);
    UINT   i;
    
    pSubMenu->ModifyMenu(ID_MENU_CONFIG0, MF_CHECKED, ID_MENU_CONFIG0, theApp.m_config_file[0]);
    pSubMenu->CheckMenuItem(ID_MENU_CONFIG0, MF_UNCHECKED);
        
    for (i=1; i<theApp.m_config_file_cnt; i++)
    {
        pSubMenu->AppendMenu(MF_CHECKED, ID_MENU_CONFIG0+i, theApp.m_config_file[i]);
        pSubMenu->CheckMenuItem(ID_MENU_CONFIG0+i, MF_UNCHECKED);
    }
    pSubMenu->CheckMenuItem(ID_MENU_CONFIG0+theApp.m_config_index, MF_CHECKED);
}

void CMainFrame::RefreshLangMenu()
{
	CMenu *pMainMenu = GetMenu();
	CMenu *pViewMenu = pMainMenu->GetSubMenu(1);
	CMenu *pLangMenu = pViewMenu->GetSubMenu(0);

	ASSERT(pLangMenu != NULL);

	//delete all the item before
	for (int iPos = pLangMenu->GetMenuItemCount()-1; iPos >= 0; iPos--)
		pLangMenu->DeleteMenu(iPos, MF_BYPOSITION);

	UINT i;
	for(i = 0; i < theApp.m_lang.m_langCount; i++)
	{
		pLangMenu->AppendMenu(MF_STRING, MENU_BASE_ID+i, theApp.m_lang.m_langName[i]);
	}
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	CFrameWnd::OnActivate(nState, pWndOther, bMinimized);
	
	// TODO: Add your message handler code here
	RefreshLangMenu();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pHandlerInfo == NULL)
	{
		for (UINT i = 0; i < theApp.m_lang.m_langCount; i++)
		{
			if (nID == MENU_BASE_ID+i)
			{
				if(nCode == CN_COMMAND)
				{
					if(theApp.m_lang.ChangeLang(i))
					{
						SetupDisplay();

						WritePrivateProfileString(_T("Lang"), _T("Lang"),  
							theApp.m_lang.m_langName[theApp.m_lang.m_activeLang], 
							theApp.ConvertAbsolutePath(_T("burn.ini")));
					}
				}
				else if(nCode == (int)CN_UPDATE_COMMAND_UI)
				{
					CCmdUI *pCmdUI = (CCmdUI*)pExtra;

					pCmdUI->SetCheck(theApp.m_lang.m_activeLang == i);
					
					if(m_worknum == 0)
					{
						pCmdUI->Enable(TRUE);
					}
					else
					{
						pCmdUI->Enable(FALSE);
					}
				}
				return TRUE;
			}
		}
	}
	
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnUpdateSettingChangePasswd(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if(m_worknum > 0)
	{
		pCmdUI->Enable(FALSE);
	}
	else
	{
		pCmdUI->Enable(TRUE);
	}
}


//此接口只提供给11平台的160K的烧录工具使用
DWORD WINAPI CMainFrame::online_image_create_thread_snowdirdL(LPVOID para)
{
	CImageCreate *pImage = (CImageCreate *)para;
	CMainFrame *pMF = (CMainFrame *)AfxGetMainWnd();
	UINT i, j, m = 0, n = 0, nID,drvnum = 0;
	DWORD dwAttr, dwSize;
	CString str;
	HANDLE hFile;
	LONGLONG capacity_M = 0;
    unsigned char buf_volume_label[15] = {0};
	char disk[2] = "Z";
	T_U8 DriverID = 0;
	TCHAR udisk_path_temp[MAX_PATH] = {0};
	TCHAR udisk_temp[MAX_PATH] = {0};
	T_U8 ID = 0;
	BOOL mtd_flag = AK_FALSE;
	
    USES_CONVERSION;
	pImage->ExitFlag = FALSE;
	nID = pImage->nID; //获取通过的id
	CreateDirectory(theApp.ConvertAbsolutePath(_T("Image")), NULL);//创建文件夹

	pImage->fslib_init();

	for(i = 0; i < g_disk_count; i++)
	{
		mtd_flag = AK_FALSE;
		//根据下载镜像判断是否要进行制作镜像
		for (n = 0; n < theConfig.download_mtd_count; n++)
		{
			if (theConfig.download_mtd_data[n].disk_name[0] == theConfig.format_data[i].Disk_Name)
			{
				mtd_flag = AK_TRUE;  
				break;
			}
		}
		//如果mtd_flag是AK_TRUE，那么说明不需要制作此盘的镜像文件,否则要制作
		if (AK_TRUE == mtd_flag)
		{
			continue;
		}
		

		ID = (T_U8)(disk[0] - (nID - 1) - 'A');//umount driver id

		//制作镜像名
		str.Format(_T("Image\\burn%c.img"), theConfig.format_data[i].Disk_Name);

		//由于当扇区个数小于8K，那么就是exfat系统，
		//同时由于exfat系统最新的扇区数是128，所以小于这个都出错
		if (g_nID_disk_info[nID-1].g_disk_info[i].sectorCount <= 128 )
		{
			pImage->img_destroy();
			::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
			return -1;
		}

		capacity_M = g_nID_disk_info[nID-1].g_disk_info[i].sectorCount;
		capacity_M *= g_nID_disk_info[nID-1].g_disk_info[i].sectorSize;
		capacity_M /=  1024; //(1024*1024);  //以K为单位
		

        if(!pImage->img_create(theApp.ConvertAbsolutePath(str), 
            (T_U32)capacity_M, g_nID_disk_info[nID-1].g_disk_info[i].sectorSize, g_nID_disk_info[nID-1].g_disk_info[i].PageSize, theConfig.burn_mode, disk[0] - (nID-1)))
		{
			FS_UnMountMemDev(ID);//FS_UnMountMemDev
			pImage->img_destroy();
			::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
			return -1;
		}
        
        if (strlen((const char *)theConfig.pVolumeLable[i].volume_lable) != 0) // 添加卷标
        {
            buf_volume_label[0] = disk[0] - (nID-1); //disk[0] - DriverID; //'A'+i;
            buf_volume_label[1] = ':';
            buf_volume_label[2] = '\\';
            memcpy(buf_volume_label+3,theConfig.pVolumeLable[i].volume_lable,11);       // most 11 byte
            if (!pImage->img_add_volume_lable(buf_volume_label))
			{
				FS_UnMountMemDev(ID);//FS_UnMountMemDev
				pImage->img_destroy();
				::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
                return -1;
			}
        }

		g_nID_disk_info[nID-1].g_disk_info[i].bWritten = FALSE;

		for(j = 0; j < theConfig.download_udisk_count; j++)
		{
			if (theConfig.udisk_info[i] == theConfig.download_udisk_data[j].udisk_path[0])
			{
				//由于11平台上使用的多线程时，镜像文件名是不一样的，所以需要区分
				//drvnum = get_udiskfile_drivernum(j);//获取文件所在的盘符
				_tcscpy(udisk_temp , theConfig.download_udisk_data[j].udisk_path);
				udisk_temp[0] = g_nID_disk_info[nID-1].g_disk_info[i].diskName;
				
				dwAttr = GetFileAttributes(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[j].pc_path));
				if(0xFFFFFFFF == dwAttr)
				{
					continue;
				}
				
				//Change to 
				if(':' == udisk_temp[1])
				{
					if(udisk_temp[0] >= 'a' && udisk_temp[0] <= 'z')
					{
						udisk_temp[0] = udisk_temp[0] - 'a' + 'A';
					}
				}
				
				if(':' == theConfig.download_udisk_data[j].udisk_path[1])
				{
					if(theConfig.download_udisk_data[j].udisk_path[0] >= 'a' &&
						theConfig.download_udisk_data[j].udisk_path[0] <= 'z')
					{
						theConfig.download_udisk_data[j].udisk_path[0] = theConfig.download_udisk_data[j].udisk_path[0] - 'a' + 'A';
					}
				}
				
				if ((0 != (theConfig.download_udisk_data[j].pc_path)[0]) 
					&& (0 != (theConfig.download_udisk_data[j].udisk_path)[0]))
				{
					//镜像的路径
					_tcscpy(udisk_path_temp , theConfig.download_udisk_data[j].udisk_path);
					//DriverID = udisk_temp[0] - 'A';
					udisk_path_temp[0] = disk[0] - (nID - 1); //disk[0] - DriverID;
					
					
					if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
					{
						if(!pImage->img_add_dir(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[j].pc_path), 
							T2A(udisk_path_temp)))//udisk_path_temp替换theConfig.download_udisk_data[i].udisk_path
						{
							FS_UnMountMemDev(ID);
							pImage->img_destroy();
							::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
							return -1;
						}
					}
					else
					{
						if(!pImage->img_add_file(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[j].pc_path), 
							T2A(udisk_path_temp))) //udisk_path_temp替换theConfig.download_udisk_data[i].udisk_path
						{
							FS_UnMountMemDev(ID);
							pImage->img_destroy();
							::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
							return -1;
						}
					}
				}
				for(m = 0; m < g_disk_count; m++)
					{
					if(g_nID_disk_info[nID-1].g_disk_info[m].diskName == udisk_temp[0])
					{
						g_nID_disk_info[nID-1].g_disk_info[m].bWritten = TRUE;
						
						if (theConfig.download_udisk_data[m].bCompare)
						{
							g_nID_disk_info[nID-1].g_disk_info[m].bCompare = TRUE;
							break;
						}
					}
				}
			} 
		}

		//去掉driver
		if(AK_FALSE == FS_UnMountMemDev(ID))
		{
			pImage->img_destroy();
			::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
			return -1;
		}
	}

	pImage->img_destroy();


	g_download_mtd_count = 0; 
    pMF->download_length_new_mtd = 0;
	disk[0] = 'Z';
	DriverID = 0;

	for(i = 0; i < g_disk_count; i++)
	{
		//DriverID = g_nID_disk_info[nID-1].g_disk_info[i].diskName - 'A';
		//str.Format(_T("Image\\burn%c.img"), g_nID_disk_info[nID-1].g_disk_info[i].diskName);
		//str.Format(_T("Image\\burn%c.img"), g_download_mtd[i].disk_name[0]);
		str.Format(_T("Image\\burn%c.img"), theConfig.format_data[i].Disk_Name);
		
		
		hFile = CreateFile(theApp.ConvertAbsolutePath(str), GENERIC_READ , FILE_SHARE_READ , NULL , 
				OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pMF->download_length_new_mtd += dwSize;
			}

			CloseHandle(hFile);
		}
		//
		//ID = (T_U8)(disk[0] - g_nID_disk_info[nID-1].g_disk_info[i].diskName);
		//if(AK_FALSE == FS_UnMountMemDev(ID))
		//{
		//	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
		//	return -1;
	//	}

	}

    for(i = 0; i < theConfig.download_mtd_count; i++)
    {
		g_download_mtd[g_download_mtd_count].bCompare = theConfig.download_mtd_data[i].bCompare;
		_tcscpy(g_download_mtd[g_download_mtd_count].pc_path, theConfig.download_mtd_data[i].pc_path);
		g_download_mtd[g_download_mtd_count].disk_name[0] = theConfig.download_mtd_data[i].disk_name[0];
		g_download_mtd[g_download_mtd_count].disk_name[1] = 0;
		g_download_mtd_count++;	
		
		//打开文件
		hFile = CreateFile(theApp.ConvertAbsolutePath(theConfig.download_mtd_data[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
				OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pMF->download_length_new_mtd += dwSize;
			}

			CloseHandle(hFile);
		}
    }
    
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_SUCCESS));
/*
	if (theConfig.download_udisk_count > 0)
	{
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, 0, LPARAM(MESSAGE_IMAGE_CREATE_SUCCESS));
	}
	else
	{
		SetEvent(image_event);
	}
*/

	return 0;
}

DWORD WINAPI CMainFrame::online_image_create_thread(LPVOID para)
{
	CImageCreate *pImage = (CImageCreate *)para;
	CMainFrame *pMF = (CMainFrame *)AfxGetMainWnd();
	UINT i, j, n;
	DWORD dwAttr, dwSize;
	CString str;
	HANDLE hFile;
	LONGLONG capacity_M = 0;
    unsigned char buf_volume_label[15] = {0};
	UINT nID = pImage->nID; //获取通过的id
	T_U8  DriverID = 0;
	
    USES_CONVERSION;
	pImage->ExitFlag = FALSE;

	CreateDirectory(theApp.ConvertAbsolutePath(_T("Image")), NULL);//创建文件夹

	pImage->fslib_init();

	for(i = 0; i < g_disk_count; i++)
	{
		str.Format(_T("Image\\burn%c.img"), g_nID_disk_info[nID-1].g_disk_info[i].diskName);


		capacity_M = g_nID_disk_info[nID-1].g_disk_info[i].sectorCount;
		capacity_M *= g_nID_disk_info[nID-1].g_disk_info[i].sectorSize;
		//capacity_M /= (1024*1024);
		capacity_M /=  1024; //(1024*1024);  //以K为单位

		//创建镜像
        if(!pImage->img_create(theApp.ConvertAbsolutePath(str), 
            (T_U32)capacity_M, g_nID_disk_info[nID-1].g_disk_info[i].sectorSize, g_nID_disk_info[nID-1].g_disk_info[i].PageSize, theConfig.burn_mode, g_nID_disk_info[nID-1].g_disk_info[i].diskName))
		{
			for (n = 0; n <= i; n++)
			{
				DriverID = (T_U8)(g_nID_disk_info[nID-1].g_disk_info[n].diskName - 'A');//
				FS_UnMountMemDev(DriverID);
			}
			pImage->img_destroy();
			::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
			return -1;
		}
        
        if (strlen((const char *)theConfig.pVolumeLable[i].volume_lable) != 0) // 添加卷标
        {
            buf_volume_label[0] = 'A'+i;
            buf_volume_label[1] = ':';
            buf_volume_label[2] = '\\';
            memcpy(buf_volume_label+3,theConfig.pVolumeLable[i].volume_lable,11);       // most 11 byte
            if (!pImage->img_add_volume_lable(buf_volume_label))//卷标
			{
				for (n = 0; n <= i; n++)
				{
					DriverID = (T_U8)(g_nID_disk_info[nID-1].g_disk_info[n].diskName - 'A');//
					FS_UnMountMemDev(DriverID);
				}
				pImage->img_destroy();
				::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
                return -1;
			}
        }

		g_nID_disk_info[nID-1].g_disk_info[i].bWritten = FALSE;
	}

	for(i = 0; i < theConfig.download_udisk_count; i++)
	{
		//文件属性
		dwAttr = GetFileAttributes(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[i].pc_path));
		if(0xFFFFFFFF == dwAttr)
		{
			continue;
		}

		//Change to 
		if(':' == theConfig.download_udisk_data[i].udisk_path[1])
		{
			if(theConfig.download_udisk_data[i].udisk_path[0] >= 'a' &&
				theConfig.download_udisk_data[i].udisk_path[0] <= 'z')
			{
				theConfig.download_udisk_data[i].udisk_path[0] = 
					theConfig.download_udisk_data[i].udisk_path[0] - 'a' + 'A';
			}
		}

		//针对linux
		for(j = 0; j < g_disk_count; j++)
		{
			if (theConfig.download_udisk_data[i].udisk_path[0] == g_nID_disk_info[nID-1].g_disk_info[j].diskName)
			{
				break;
			}
		}
		if ( j == g_disk_count)
		{
			continue;
		}


		if ((0 != (theConfig.download_udisk_data[i].pc_path)[0]) 
			&& (0 != (theConfig.download_udisk_data[i].udisk_path)[0]))
		{
				if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				{
					//加文件夹
					if(!pImage->img_add_dir(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[i].pc_path), 
						T2A(theConfig.download_udisk_data[i].udisk_path)))
					{
						for (n = 0; n <= i; n++)
						{
							DriverID = (T_U8)(g_nID_disk_info[nID-1].g_disk_info[n].diskName - 'A');//
							FS_UnMountMemDev(DriverID);
						}
						pImage->img_destroy();
						::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
						return -1;
					}
				}
				else
				{
					//加文件
					if(!pImage->img_add_file(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[i].pc_path), 
						T2A(theConfig.download_udisk_data[i].udisk_path)))
					{
						for (n = 0; n <= i; n++)
						{
							DriverID = (T_U8)(g_nID_disk_info[nID-1].g_disk_info[n].diskName - 'A');//
							FS_UnMountMemDev(DriverID);
						}
						pImage->img_destroy();
						::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
						return -1;
					}
				}
		}

	
		for(j = 0; j < g_disk_count; j++)
		{
			if(g_nID_disk_info[nID-1].g_disk_info[j].diskName == theConfig.download_udisk_data[i].udisk_path[0])
			{
				g_nID_disk_info[nID-1].g_disk_info[j].bWritten = TRUE;
				
				if (theConfig.download_udisk_data[i].bCompare)
				{
					g_nID_disk_info[nID-1].g_disk_info[j].bCompare = TRUE;
					break;
				}
			}
		}
	}
	
	for(i = 0; i < g_disk_count; i++)
	{
		DriverID = (T_U8)(g_nID_disk_info[nID-1].g_disk_info[i].diskName - 'A');//
	
		//去掉driver
		if(AK_FALSE == FS_UnMountMemDev(DriverID))
		{
			pImage->img_destroy();
			::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
			return -1;
		}
	}

	pImage->img_destroy();

	g_download_mtd_count = 0; 
    pMF->download_length_new_mtd = 0;

	for(i = 0; i < g_disk_count; i++)
	{
		str.Format(_T("Image\\burn%c.img"), g_nID_disk_info[nID-1].g_disk_info[i].diskName);

		hFile = CreateFile(theApp.ConvertAbsolutePath(str), GENERIC_READ , FILE_SHARE_READ , NULL , 
				OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pMF->download_length_new_mtd += dwSize;
			}

			CloseHandle(hFile);
		}
	}

    for(i = 0; i < theConfig.download_mtd_count; i++)
    {
		g_download_mtd[g_download_mtd_count].bCompare = theConfig.download_mtd_data[i].bCompare;
		_tcscpy(g_download_mtd[g_download_mtd_count].pc_path, theConfig.download_mtd_data[i].pc_path);
		g_download_mtd[g_download_mtd_count].disk_name[0] = theConfig.download_mtd_data[i].disk_name[0];
		g_download_mtd[g_download_mtd_count].disk_name[1] = 0;
		g_download_mtd_count++;	

		hFile = CreateFile(theApp.ConvertAbsolutePath(theConfig.download_mtd_data[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
				OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pMF->download_length_new_mtd += dwSize;
			}

			CloseHandle(hFile);
		}
    }
    
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_SUCCESS));
/*
	if (theConfig.download_udisk_count > 0)
	{
		::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, 0, LPARAM(MESSAGE_IMAGE_CREATE_SUCCESS));
	}
	else
	{
		SetEvent(image_event);
	}
*/

	return 0;
}

/*
#if 0
DWORD WINAPI CMainFrame::online_image_create_thread(LPVOID para)
{
	CImageCreate *pImage = (CImageCreate *)para;
	CMainFrame *pMF = (CMainFrame *)AfxGetMainWnd();
	UINT i, j, nID,drvnum = 0;
	DWORD dwAttr, dwSize;
	CString str;
	HANDLE hFile;
	LONGLONG capacity_M = 0;
    unsigned char buf_volume_label[15] = {0};
	char disk[2] = "Z";
	T_U8 DriverID = 0;
	TCHAR udisk_path_temp[MAX_PATH] = {0};
	TCHAR udisk_temp[MAX_PATH] = {0};
	
    USES_CONVERSION;
	pImage->ExitFlag = FALSE;
	nID = pImage->nID; //获取通过的id
	CreateDirectory(theApp.ConvertAbsolutePath(_T("Image")), NULL);

	pImage->fslib_init();

	for(i = 0; i < g_disk_count; i++)
	{
		//str.Format(_T("Image\\burn%c.img"), g_disk_info[i].diskName);
		DriverID = g_nID_disk_info[nID-1].g_disk_info[i].diskName - 'A';
		str.Format(_T("Image\\burn%d_%c.img"), nID-1, disk[0] - DriverID);


		capacity_M = g_nID_disk_info[nID-1].g_disk_info[i].sectorCount;
		capacity_M *= g_nID_disk_info[nID-1].g_disk_info[i].sectorSize;
		capacity_M /= (1024*1024);

        if(!pImage->img_create(theApp.ConvertAbsolutePath(str), 
            (T_U32)capacity_M, g_nID_disk_info[nID-1].g_disk_info[i].sectorSize, g_nID_disk_info[nID-1].g_disk_info[i].PageSize, theConfig.burn_mode, disk[0]-DriverID))
		{
			::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
			return -1;
		}
        
        if (strlen((const char *)theConfig.pVolumeLable[i].volume_lable) != 0) // 添加卷标
        {
            buf_volume_label[0] = disk[0] - DriverID; //'A'+i;
            buf_volume_label[1] = ':';
            buf_volume_label[2] = '\\';
            memcpy(buf_volume_label+3,theConfig.pVolumeLable[i].volume_lable,11);       // most 11 byte
            if (!pImage->img_add_volume_lable(buf_volume_label))
                return -1;
        }

		g_nID_disk_info[nID-1].g_disk_info[i].bWritten = FALSE;
	}

	for(i = 0; i < theConfig.download_udisk_count; i++)
	{
		//由于11平台上使用的多线程时，镜像文件名是不一样的，所以需要区分
		if (theConfig.planform_tpye == E_ROST_PLANFORM && theConfig.chip_type == CHIP_11XX)
		{
			drvnum = get_udiskfile_drivernum(i);//获取文件所在的盘符
			_tcscpy(udisk_temp , theConfig.download_udisk_data[i].udisk_path);
			udisk_temp[0] = g_nID_disk_info[nID-1].g_disk_info[drvnum].diskName;
		} 
		else
		{
			_tcscpy(udisk_temp , theConfig.download_udisk_data[i].udisk_path);
		}


		dwAttr = GetFileAttributes(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[i].pc_path));
		if(0xFFFFFFFF == dwAttr)
		{
			continue;
		}

		//Change to 
		if(':' == udisk_temp[1])
		{
			if(udisk_temp[0] >= 'a' && udisk_temp[0] <= 'z')
			{
				udisk_temp[0] = udisk_temp[0] - 'a' + 'A';
			}
		}

		if(':' == theConfig.download_udisk_data[i].udisk_path[1])
		{
			if(theConfig.download_udisk_data[i].udisk_path[0] >= 'a' &&
				theConfig.download_udisk_data[i].udisk_path[0] <= 'z')
			{
				theConfig.download_udisk_data[i].udisk_path[0] = theConfig.download_udisk_data[i].udisk_path[0] - 'a' + 'A';
			}
		}

		if ((0 != (theConfig.download_udisk_data[i].pc_path)[0]) 
			&& (0 != (theConfig.download_udisk_data[i].udisk_path)[0]))
		{
				//镜像的路径
				_tcscpy(udisk_path_temp , theConfig.download_udisk_data[i].udisk_path);
				DriverID = udisk_temp[0] - 'A';
				udisk_path_temp[0] = disk[0] - DriverID;


				if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
				{
					if(!pImage->img_add_dir(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[i].pc_path), 
						T2A(udisk_path_temp)))//udisk_path_temp替换theConfig.download_udisk_data[i].udisk_path
					{
						::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
						return -1;
					}
				}
				else
				{
					if(!pImage->img_add_file(theApp.ConvertAbsolutePath(theConfig.download_udisk_data[i].pc_path), 
						T2A(udisk_path_temp))) //udisk_path_temp替换theConfig.download_udisk_data[i].udisk_path
					{
						::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
						return -1;
					}
				}
		}
		for(j = 0; j < g_disk_count; j++)
		{
			if(g_nID_disk_info[nID-1].g_disk_info[j].diskName == udisk_temp[0])
			{
				g_nID_disk_info[nID-1].g_disk_info[j].bWritten = TRUE;
				
				if (theConfig.download_udisk_data[i].bCompare)
				{
					g_nID_disk_info[nID-1].g_disk_info[j].bCompare = TRUE;
					break;
				}
			}
		}
	}

	pImage->img_destroy();


	g_download_mtd_count = 0; 
    pMF->download_length_new_mtd = 0;
	disk[0] = 'Z';
	DriverID = 0;

	for(i = 0; i < g_disk_count; i++)
	{
		T_U8 ID = 0;
		DriverID = g_nID_disk_info[nID-1].g_disk_info[i].diskName - 'A';
		str.Format(_T("Image\\burn%d_%c.img"), nID-1,  disk[0]-DriverID);

		hFile = CreateFile(theApp.ConvertAbsolutePath(str), GENERIC_READ , FILE_SHARE_READ , NULL , 
				OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pMF->download_length_new_mtd += dwSize;
			}

			CloseHandle(hFile);
		}
		//
		ID = (T_U8)(disk[0] - g_nID_disk_info[nID-1].g_disk_info[i].diskName);
		if(AK_FALSE == FS_UnMountMemDev(ID))
		{
			::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
			return -1;
		}

	}

    for(i = 0; i < theConfig.download_mtd_count; i++)
    {
		g_download_mtd[g_download_mtd_count].bCompare = theConfig.download_mtd_data[i].bCompare;
		_tcscpy(g_download_mtd[g_download_mtd_count].pc_path, theConfig.download_mtd_data[i].pc_path);
		g_download_mtd[g_download_mtd_count].disk_name[0] = theConfig.download_mtd_data[i].disk_name[0];
		g_download_mtd[g_download_mtd_count].disk_name[1] = 0;
		g_download_mtd_count++;	

		hFile = CreateFile(theApp.ConvertAbsolutePath(theConfig.download_mtd_data[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
				OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);

		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pMF->download_length_new_mtd += dwSize;
			}

			CloseHandle(hFile);
		}
    }
    
	::PostMessage(AfxGetMainWnd()->GetSafeHwnd(), ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_SUCCESS));

	return 0;
}
#endif
*/

void CMainFrame::StartCountTime(UINT nID)
{
	m_burn_time[nID-1].nID = nID; 
	m_burn_time[nID-1].time_count = 0;
}
void CMainFrame::StopCountTime(UINT nID)
{
	m_burn_time[nID-1].nID = 0; 
}

void CMainFrame::OnUpdateAppHelp(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here

}

void CMainFrame::OnAppHelp() 
{
	// TODO: Add your command handler code here
    // T_U32  retVal = 0;
    if ((long)ShellExecute(NULL, _T("open"), theApp.ConvertAbsolutePath(_T("help.doc")), NULL, NULL, SW_SHOWNORMAL) <= 32)
    {
        MessageBox(theApp.GetString(IDS_HELP_FILE_ERROR));
    }
}

/*
功能：递归查找文件夹中的文件个数和总文件大小，及最小字节的文件
说明：文件夹的内容总容量不能大于4g，要么总文件大小出错的 
作者：luqiliu
输入：pathFloder--文件夹的总路径
输出：FileCnt-----文件的个数 
      FileSize----文件的总容量(unit byte)
      MinSize-----最小的文件
返回：成功找到一个以上的文件返回TRUE， 否则返回FALSE
*/
BOOL CMainFrame::GetFileInfoInDir(LPTSTR pathFloder, DWORD *FileCnt, DWORD *FileSize, DWORD *MinSize)
{
	WIN32_FIND_DATA fd;
	HANDLE hSearch;
	TCHAR searchPath[MAX_PATH+1];
    TCHAR tmpPCPath[MAX_PATH+1];
    HANDLE hFile;
    DWORD  dwSize;
    CString str;
 
    if (pathFloder == NULL)
    {
        return FALSE;
    }
    
    _tcsncpy(searchPath, pathFloder, MAX_PATH);
	_tcscat(searchPath, _T("\\*"));

	hSearch = FindFirstFile(searchPath, &fd);
	if(INVALID_HANDLE_VALUE == hSearch)
	{
		return FALSE;
	}
	
	USES_CONVERSION;
	
	do
	{
		if((0 != _tcscmp(fd.cFileName, _T("."))) && (0 != _tcscmp(fd.cFileName, _T(".."))))
		{
			_tcscpy(tmpPCPath, pathFloder);
            _tcscat(tmpPCPath, _T("\\"));
			_tcscat(tmpPCPath, fd.cFileName);

			/*如果镜像，只查找当前的目录*/
            if(MinSize == NULL && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
                if (!GetFileInfoInDir(tmpPCPath, FileCnt, FileSize, MinSize))
                {
                    FindClose(hSearch);

                    return FALSE;
                }
			}
            else
            {
	            //add produce length
                hFile = CreateFile(tmpPCPath, GENERIC_READ , FILE_SHARE_READ , NULL , 
					            OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	            if(hFile != INVALID_HANDLE_VALUE)
	            {
		            dwSize = GetFileSize(hFile, NULL);
                    CloseHandle(hFile);
		            if(dwSize != 0xFFFFFFFF)
		            {
                        if (FileSize != NULL)
                        {
                            *FileSize += dwSize;
                        }
                        if (FileCnt != NULL)
                            (*FileCnt)++;

                        /*为了自动匹配镜像，查找最小字节的镜像文件*/
                        if (MinSize != NULL)
                        {
                            if (*MinSize > dwSize)
                                *MinSize = dwSize;
                        }

                    }
		            else
		            {
                        FindClose(hSearch);
			            str.Format(_T("GetFileInfoInDir Cannot find file: %s"), tmpPCPath);
			            MessageBox(str);
                    
			            return FALSE;
		            }
                }
            }
		}
	}
	while(FindNextFile(hSearch, &fd));

	FindClose(hSearch);
    
	return TRUE;	
}

BOOL CMainFrame::ClearLogFile(VOID)
{
    WIN32_FIND_DATA  FindFileData; 
    
    HANDLE  hSearch; 
    BOOL  fFinished = FALSE; 

    USES_CONVERSION;
    
    
    // Start searching for .TXT files in the current directory. 
    hSearch = FindFirstFile(_T("*.txt"), &FindFileData); 
    if (hSearch == INVALID_HANDLE_VALUE) 
    {
        
        return FALSE;
    } 

    frmLogfile.WriteLogFile(0, "--------------------------------------------------\r\n");
    frmLogfile.WriteLogFile(LOG_LINE_TIME, "Clear history logfiles:\r\n");

    while (!fFinished) 
    {
        if ((FindFileData.cFileName[0]=='c') &&
            (FindFileData.cFileName[1]=='h') &&
            (FindFileData.cFileName[6]=='l') &&
            (FindFileData.cFileName[7]=='o') &&
            (FindFileData.cFileName[8]=='g'))           // 日志文件
        {
            SYSTEMTIME  logSystemTime;
            SYSTEMTIME pcSystemTime;
            DWORD   dwAttrs; 
            DWORD log_date = 0;
            DWORD pc_date;
            
            frmLogfile.WriteLogFile(0, "<%s>\r\n",T2A(FindFileData.cFileName));
            dwAttrs = GetFileAttributes(FindFileData.cFileName);
            if (dwAttrs & FILE_ATTRIBUTE_READONLY != 0) // 只读文件
            {
                // 去掉只读属性
                SetFileAttributes(FindFileData.cFileName, dwAttrs & (!FILE_ATTRIBUTE_READONLY));            
            }
            
            // 文件时间
            FileTimeToSystemTime(&FindFileData.ftLastWriteTime, &logSystemTime);
            frmLogfile.WriteLogFile(0, "last modify time: %d-%d-%d\r\n",logSystemTime.wYear,logSystemTime.wMonth,logSystemTime.wDay);
            log_date = logSystemTime.wYear*365 + logSystemTime.wMonth*30 + logSystemTime.wDay;
            
            // 系统时间
            GetLocalTime(&pcSystemTime);
            pc_date = pcSystemTime.wYear*365 + pcSystemTime.wMonth*30 + pcSystemTime.wDay;
            
            if (pc_date - log_date > 6)
            {
                frmLogfile.WriteLogFile(0, "delete...\r\n");
                DeleteFile(FindFileData.cFileName);
            }
            frmLogfile.WriteLogFile(0,".\r\n");
        }
        
        if (!FindNextFile(hSearch, &FindFileData)) //找下一个
        {
            if (ERROR_NO_MORE_FILES == GetLastError()) 
            { 
                frmLogfile.WriteLogFile(LOG_LINE_TIME, "End!\r\n");
                fFinished = TRUE; 
            } 
        }
    }

    // Close the search handle. 
    FindClose(hSearch);
    
    frmLogfile.WriteLogFile(0, "--------------------------------------------------\r\n");
    return TRUE;
}

VOID CMainFrame::SendCmdToResetUSB(char diskname)
{
		CUpdateBase update;
		TCHAR strDisk[4] = {0};
		UINT  vol;
		

		m_bScan = FALSE;

		if(update.OpenDisk(diskname))//打开USB
		{
			update.SendCommand();//发CC命令
			//Sleep(150);
			update.CloseDisk();		
		}
		m_budisk_current_udisknum--;

		vol = diskname - 'A';
		frmLogfile.WriteLogFile(0,"vol:%d, diskname:%c, m_budisk_current_udisknum:%d\r\n", vol, diskname, m_budisk_current_udisknum);
		m_udisk_state[vol] = UDISK_NULL;

		if (0 == m_budisk_current_udisknum)
		{
			Sleep(150);	
			frmLogfile.WriteLogFile(0,"m_budisk_current_udisknum:%d\r\n", m_budisk_current_udisknum);
			m_bScan = TRUE;
		}
}

void CMainFrame::OnToolImageMaker() 
{
	// TODO: Add your command handler code here
	DlgImageCreate dlgImageCreate;
    
    dlgImageCreate.DoModal();
	
}

void CMainFrame::OnSpiImageMaker() 
{
	// TODO: Add your command handler code here
#if 0
    //判断是否有spi参数
	if (0 < theConfig.spiflash_parameter_count)
	{
		DlgSpiImageCreate dlgSpiImageCreate;
		dlgSpiImageCreate.DoModal();
	}
	else
	{
		AfxMessageBox(theApp.GetString(IDS_NO_SPI));
	}
#endif

	if(IDOK == MessageBox(theApp.GetString(IDS_SPIFLASH_UPLOAD),NULL,MB_OKCANCEL|MB_ICONEXCLAMATION))
	{
		g_bUpload_spialldata = TRUE;
		g_bUploadbinMode = FALSE;
	}
	else
	{
		g_bUpload_spialldata = FALSE;
	}
	
	//set window title
	set_window_title();

}

void CMainFrame::OnBinUpload() 
{
	// TODO: Add your command handler code here
	if(IDOK == MessageBox(theApp.GetString(IDS_BIN_UPLOAD),NULL,MB_OKCANCEL|MB_ICONEXCLAMATION))
	{
		g_bUploadbinMode = TRUE;
		g_bUpload_spialldata = FALSE;
	}
	else
	{
		g_bUploadbinMode = FALSE;
	}

	//set window title
	set_window_title();

	/*	
    if (!BT_Start())
    {
        g_bUploadbinMode = FALSE;
    }
	*/
}

void CMainFrame::OnClose() 
{
	// TODO: Add your command handler code here MB_OKCANCEL|MB_ICONEXCLAMATION
	if (IDYES == MessageBox(theApp.GetString(IDS_CLOSE_OR_NO), NULL, MB_YESNO))
	{
		CFrameWnd::OnClose();
	}
	
}

void CMainFrame::OnOnlineImageMake() 
{
	// TODO: Add your control notification handler code here
	BYTE check = 0;
	if (CHIP_10X6 == theConfig.chip_type || CHIP_10XXC == theConfig.chip_type || CHIP_1080L == theConfig.chip_type)
	{
		((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_ONLINE_IMAGE_MAKE))->ShowWindow(FALSE);
	}
	else
	{
		((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_ONLINE_IMAGE_MAKE))->ShowWindow(TRUE);
		check = ((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_ONLINE_IMAGE_MAKE))->GetCheck();
		if (check)
		{
			theConfig.bonline_make_image = TRUE;//在线制作
		}
		else 
		{
			theConfig.bonline_make_image = FALSE;
		}
	}
	
}

void CMainFrame::OnAutoBurn() 
{
	// TODO: Add your control notification handler code here
	
}



//获取文件所在的盘
UINT get_udiskfile_drivernum(UINT filenum)
{
	UINT numID = 0;
	UINT i = 0;

	for (numID = 0; numID < theConfig.udiskfile_drivernum; numID++)
	{
		if (theConfig.download_udisk_data[filenum].udisk_path[0] == theConfig.udisk_info[numID])
		{
			break;
		}
	}

	return numID;
}
//保存文件所在的盘，
//镜像文件创建时使用
void Save_udiskfile_drivernum(void)
{
    UINT i;
	UINT m = 0, n;
	UINT j = 0;
	BOOL flag = TRUE;
	TCHAR udisk_temp;
	TCHAR udisk[MAX_PATH] = {0};

	memset(theConfig.udisk_info, 0, MAX_PATH);

	for(i = 0; i < theConfig.format_count; i++)
	{
		udisk[i] = theConfig.format_data[i].Disk_Name;
		j++;
	}

	//记录多少个盘存放文件
	theConfig.udiskfile_drivernum = j;

	//对文件的盘符进行排列
	for (i = 1; i < j && udisk[i] != 0; i++)
	{
		for (n = i; n > 0; n--)
		{
			if (udisk[n] < udisk[n-1])
			{
				udisk_temp = udisk[n];
				udisk[n] = udisk[n-1];
				udisk[n-1] = udisk_temp;
			}
		}
	}
	//_tcscpy(theConfig.udisk_info, udisk);
	memcpy(theConfig.udisk_info, udisk, (theConfig.udiskfile_drivernum+1)*2);
}

//创建镜像的文件时需要用
void SetPartInfo(UINT nID, T_DRIVER_INFO *pDirverInfo, UINT num, UINT SectorPerPage, UINT SectorSize, UINT nDISKB_Enlarge)
{
    UINT i;
	
    if(NULL == pDirverInfo || num  == 0)
    {
        return;
    }
	
    //EnterCriticalSection(&image_cs);
	
    g_disk_count = num; //theConfig.format_count;
    for(i = 0; i < g_disk_count; i++)
    {
        g_nID_disk_info[nID-1].g_disk_info[i].diskName = pDirverInfo[i].DiskName;
        g_nID_disk_info[nID-1].g_disk_info[i].sectorCount = pDirverInfo[i].PageCnt * SectorPerPage;
        g_nID_disk_info[nID-1].g_disk_info[i].sectorSize  = SectorSize;
        g_nID_disk_info[nID-1].g_disk_info[i].PageSize    = SectorSize * SectorPerPage;
		
		
        // if (('B' == g_disk_info[i].diskName) && (0 != nDISKB_Enlarge))
        if ((theConfig.format_data[i].bOpenZone) && (0 != nDISKB_Enlarge))  // 用户盘
        {
            UINT tmp_sec;
			
            tmp_sec = nDISKB_Enlarge * (1024 * 1024 / SectorSize);
            if (tmp_sec > g_nID_disk_info[nID-1].g_disk_info[i].sectorCount)   // 如果扩容值小于实际容量值则不改变
            {
                g_nID_disk_info[nID-1].g_disk_info[i].sectorCount = tmp_sec;
            }
        }
        g_nID_disk_info[nID-1].g_disk_info[i].bWritten = FALSE;
        g_nID_disk_info[nID-1].g_disk_info[i].bCompare = FALSE;
    }
	
   //LeaveCriticalSection(&image_cs);
}

void CMainFrame::OnTbtnExport() 
{
	// TODO: Add your control notification handler code here
	//if ((theConfig.planform_tpye == E_ROST_PLANFORM) && (theConfig.chip_type == CHIP_11XX))
	{
		//((CButton *)m_DlgBar.GetDlgItem(IDC_UPDATE_EXPORT))->ShowWindow(TRUE);

	#ifdef CRC_CHECK
		SetupDownload();
	#endif
		
		CCheckExportDlg dlg;
		
		dlg.DoModal();
	}
	//else
	{
		//((CButton *)m_DlgBar.GetDlgItem(IDC_UPDATE_EXPORT))->ShowWindow(FALSE);
	}
}

void CMainFrame::ImportUptFile()
{
	TCHAR szFilter[] =	TEXT("Update Files(*.upd)|*.upd|") \
		TEXT("All Files (*.*)|*.*||") ;
	
	CFileDialog fd(AK_TRUE, _T(".upd"), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
        szFilter, NULL);
	
    if(IDOK == fd.DoModal())
    {
		CBurnToolView *pView = (CBurnToolView *)GetActiveView();
		
		CString strPath = fd.GetPathName();
        CUpdate update;
		
		HWND hDlg = CreateWindow(_T("BUTTON"), 
			theApp.GetString(IDS_MSG_WAIT),
			WS_VISIBLE, 
			GetSystemMetrics(SM_CXSCREEN)/2 - 100, 
			GetSystemMetrics(SM_CYSCREEN)/2 - 50 ,
			200, 
			NULL, 
			m_hWnd,
			NULL,
			AfxGetInstanceHandle(), 
			NULL);
		
		if(update.ImportUpdateFile(&theConfig, strPath))//导入
        {
			
			SetupDownload();
			
			::DestroyWindow(hDlg);
            
			AfxMessageBox(theApp.GetString(IDS_MSG_UPD_IMPORT_SUCCESS));
        }
        else
        {
			::DestroyWindow(hDlg);
			
			AfxMessageBox(theApp.GetString(IDS_MSG_UPD_IMPORT_FAIL));
        }
		
		pView->SetupListView(theConfig.device_num);
    }
}

void CMainFrame::OnTbtnImport() 
{
	// TODO: Add your control notification handler code here
	//if ((theConfig.planform_tpye == E_ROST_PLANFORM)  && (theConfig.chip_type == CHIP_11XX))
	{
		//((CButton *)m_DlgBar.GetDlgItem(ID_TBTN_IMPORT))->ShowWindow(TRUE);
		ImportUptFile();
	}
	//else
	{
		//((CButton *)m_DlgBar.GetDlgItem(ID_TBTN_IMPORT))->ShowWindow(FALSE);
	}
}
#if 0
void CMainFrame::OnTbtnFormat() 
{
	// TODO: Add your control notification handler code here

	CUpdateBase update;

	CMainFrame *pMF = (CMainFrame *)AfxGetMainWnd();

	//((CButton*)GetDlgItem(ID_TBTN_FORMAT))->EnableWindow(FALSE);//格式化中，不让再次点击
	
	if(IDYES == MessageBox(theApp.GetString(IDS_PHYSICAL_FORMAT),NULL,MB_YESNO))
	{
		g_bEraseMode = TRUE;
		BT_Start();
		/*
		if (!BT_Start())
		{
            g_bEraseMode = FALSE;
		}
		*/
	}
	
	//((CButton*)GetDlgItem(ID_TBTN_FORMAT))->EnableWindow(TRUE);         //格式化完毕，允许点击
	
}

#endif

void CMainFrame::OnCheckUdiskBurn() 
{
	// TODO: Add your control notification handler code here
	BOOL check = theConfig.bUDiskUpdate;
	check = ((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_UDISK_BURN))->GetCheck();
	theConfig.bUDiskUpdate = check ? TRUE : FALSE;
	//因为10和11芯片有支持烧完后重启功能，所以以防重复烧录，此些平台不提供自动烧录功能
	if (TRUE == theConfig.bUDiskUpdate && (theConfig.chip_type == CHIP_1080A
		|| theConfig.chip_type == CHIP_1080L || theConfig.chip_type == CHIP_11XX
		|| theConfig.chip_type == CHIP_10X6 || theConfig.chip_type == CHIP_10XXC))
	{
		((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_AUTO_BURN))->EnableWindow(FALSE);//自动烧录不可用
		((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_AUTO_BURN))->SetCheck(0);
	} 
	else
	{
		((CButton *)m_DlgBar.GetDlgItem(IDC_CHECK_AUTO_BURN))->EnableWindow(TRUE);//自动烧录可用
	}
	//set window title
	set_window_title();
}
