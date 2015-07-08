// PageNormal.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "PageGeneral.h"
#include "PageHardware.h"
#include "DlgConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CConfig theConfig;
extern CBurnToolApp theApp;
extern HINSTANCE _hInstance;

//extern char Current_DirectoryBuffer[MAX_PATH];

/////////////////////////////////////////////////////////////////////////////
// CPageGeneral property page

IMPLEMENT_DYNCREATE(CPageGeneral, CPropertyPage)

CPageGeneral::CPageGeneral() : CPropertyPage(CPageGeneral::IDD)
{
	//{{AFX_DATA_INIT(CPageGeneral)
	m_edit_mac_start_addr_high = _T("");
	m_edit_mac_start_addr_low = _T("");
	m_edit_mac_end_addr_high = _T("");
	m_edit_mac_end_addr_low = _T("");
	m_edit_sequence_end_addr_high = _T("");
	m_edit_sequence_end_addr_low = _T("");
	m_edit_sequence_start_addr_high = _T("");
	m_edit_sequence_start_addr_low = _T("");
	//}}AFX_DATA_INIT
}

CPageGeneral::~CPageGeneral()
{
}

void CPageGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageGeneral)
	DDX_Text(pDX, IDC_EDIT_MAC_START_ADDR_HIGH, m_edit_mac_start_addr_high);
	DDV_MaxChars(pDX, m_edit_mac_start_addr_high, 8);
	DDX_Text(pDX, IDC_EDIT_MAC_START_ADDR_LOW, m_edit_mac_start_addr_low);
	DDV_MaxChars(pDX, m_edit_mac_start_addr_low, 8);
	DDX_Text(pDX, IDC_EDIT_MAC_END_ADDR_HIGH, m_edit_mac_end_addr_high);
	DDV_MaxChars(pDX, m_edit_mac_end_addr_high, 8);
	DDX_Text(pDX, IDC_EDIT_MAC_END_ADDR_LOW, m_edit_mac_end_addr_low);
	DDV_MaxChars(pDX, m_edit_mac_end_addr_low, 8);
	DDX_Text(pDX, IDC_EDIT_SEQUENCE_END_ADDR_HIGH, m_edit_sequence_end_addr_high);
	DDV_MaxChars(pDX, m_edit_sequence_end_addr_high, 10);
	DDX_Text(pDX, IDC_EDIT_SEQUENCE_END_ADDR_LOW, m_edit_sequence_end_addr_low);
	DDV_MaxChars(pDX, m_edit_sequence_end_addr_low, 6);
	DDX_Text(pDX, IDC_EDIT_SEQUENCE_START_ADDR_HIGH, m_edit_sequence_start_addr_high);
	DDV_MaxChars(pDX, m_edit_sequence_start_addr_high, 10);
	DDX_Text(pDX, IDC_EDIT_SEQUENCE_START_ADDR_LOW, m_edit_sequence_start_addr_low);
	DDV_MaxChars(pDX, m_edit_sequence_start_addr_low, 6);
	//}}AFX_DATA_MAP
}

int ctl_array[42] = {IDC_STATIC_k1,IDC_CHECK_MAC_ADDR,IDC_CHECK_FORCE_WRITE_MAC_ADDR,IDC_MACADDR_RESET,
					IDC_STATIC_MAC_START_NAME,IDC_EDIT_MAC_START_ADDR_HIGH,IDC_STATIC_MAC_START_ADDR,
					IDC_EDIT_MAC_START_ADDR_LOW,IDC_STATIC_MAC_END_NAME,IDC_EDIT_MAC_END_ADDR_HIGH,
					IDC_STATIC_MAC_END_ADDR,IDC_EDIT_MAC_END_ADDR_LOW,
					IDC_STATIC_k2,IDC_CHECK_SEQUENCE_ADDR,IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR,
					IDC_SEQUENCE_ADDR_RESET,IDC_STATIC_SERIAL_START_NAME,IDC_EDIT_SEQUENCE_START_ADDR_HIGH,
					IDC_STATIC_SEQUENCE_START,IDC_EDIT_SEQUENCE_START_ADDR_LOW,IDC_STATIC_SERIAL_END_NAME,
					IDC_EDIT_SEQUENCE_END_ADDR_HIGH,IDC_STATIC_SEQUENCE_END,IDC_EDIT_SEQUENCE_END_ADDR_LOW,
					IDC_STATIC_a1,IDC_STATIC_a2,IDC_EDIT_PROD_RUN_ADDR,IDC_STATIC_PATH_PRODUCER,
					IDC_EDIT_PATH_PRODUCER,IDC_BTN_BROWSE_PRODUCER,IDC_STATIC_PATH_NANDBOOT,IDC_EDIT_PATH_NANDBOOT,
					IDC_BTN_BROWSE_NANDBOOT,IDC_STATIC_PATH_NANDBOOT_NEW,IDC_EDIT_PATH_NANDBOOT_NEW,IDC_BTN_BROWSE_NANDBOOT_NEW,
					IDC_STATIC_a3,IDC_EVENT_TIME, IDC_STATIC_PID,IDC_EDIT_PID,IDC_STATIC_VID,IDC_EDIT_VID};

BEGIN_MESSAGE_MAP(CPageGeneral, CPropertyPage)
	//{{AFX_MSG_MAP(CPageGeneral)
	ON_BN_CLICKED(IDC_BTN_BROWSE_PRODUCER, OnBtnBrowseProducer)
	ON_BN_CLICKED(IDC_BTN_BROWSE_NANDBOOT, OnBtnBrowseNandboot)
	ON_BN_CLICKED(IDC_CHECK_OPEN_COM, OnCheckOpenCom)
	ON_EN_CHANGE(IDC_EDIT_PROD_RUN_ADDR, OnChangeEditProdRunAddr)
	ON_BN_CLICKED(IDC_CHK_CONFIG_CMDLINE, OnChkConfigCmdline)
	ON_EN_CHANGE(IDC_EDIT_CMDLINE_ADDR, OnChangeEditCmdlineAddr)
	ON_EN_CHANGE(IDC_EDIT_CMDLINE_DATA, OnChangeEditCmdlineData)
	ON_BN_CLICKED(IDC_CHECK_MAC_ADDR, OnCheckMacAddr)
	ON_BN_CLICKED(IDC_CHECK_FORCE_WRITE_MAC_ADDR, OnCheckForceWriteMacAddr)
	ON_BN_CLICKED(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR, OnCheckForceWriteSequenceAddr)
	ON_BN_CLICKED(IDC_CHECK_SEQUENCE_ADDR, OnCheckSequenceAddr)
	ON_EN_CHANGE(IDC_EDIT_MAC_START_ADDR_LOW, OnChangeEditMacStartAddrLow)
	ON_EN_CHANGE(IDC_EDIT_MAC_START_ADDR_HIGH, OnChangeEditMacStartAddrHigh)
	ON_EN_CHANGE(IDC_EDIT_MAC_END_ADDR_HIGH, OnChangeEditMacEndAddrHigh)
	ON_EN_CHANGE(IDC_EDIT_MAC_END_ADDR_LOW, OnChangeEditMacEndAddrLow)
	ON_EN_CHANGE(IDC_EDIT_SEQUENCE_START_ADDR_HIGH, OnChangeEditSequenceStartAddrHigh)
	ON_EN_CHANGE(IDC_EDIT_SEQUENCE_START_ADDR_LOW, OnChangeEditSequenceStartAddrLow)
	ON_EN_CHANGE(IDC_EDIT_SEQUENCE_END_ADDR_HIGH, OnChangeEditSequenceEndAddrHigh)
	ON_EN_CHANGE(IDC_EDIT_SEQUENCE_END_ADDR_LOW, OnChangeEditSequenceEndAddrLow)
	ON_CBN_SELCHANGE(IDC_COMBO_PALNFORM_TYPE, OnSelchangeComboPalnformType)
	ON_BN_CLICKED(IDC_SEQUENCE_ADDR_RESET, OnSequenceAddrReset)
	ON_BN_CLICKED(IDC_MACADDR_RESET, OnMacaddrReset)
	ON_EN_CHANGE(IDC_EDIT_PID, OnChangeEditPid)
	ON_EN_CHANGE(IDC_EDIT_VID, OnChangeEditVid)
	ON_BN_CLICKED(IDC_BTN_BROWSE_NANDBOOT_NEW, OnBtnBrowseNandbootNew)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageGeneral message handlers
void CPageGeneral::set_nandboot_new_37L()
{
	if (theConfig.chip_type != CHIP_37XX && theConfig.chip_type != CHIP_37XX_L)
	{
		GetDlgItem(IDC_STATIC_PATH_NANDBOOT_NEW)->ShowWindow(FALSE);//不可用
		GetDlgItem(IDC_EDIT_PATH_NANDBOOT_NEW)->ShowWindow(FALSE);//不可用
		GetDlgItem(IDC_BTN_BROWSE_NANDBOOT_NEW)->ShowWindow(FALSE);//不可用
	}
	else
	{
		GetDlgItem(IDC_STATIC_PATH_NANDBOOT_NEW)->ShowWindow(TRUE);//不可用
		GetDlgItem(IDC_EDIT_PATH_NANDBOOT_NEW)->ShowWindow(TRUE);//不可用
		GetDlgItem(IDC_BTN_BROWSE_NANDBOOT_NEW)->ShowWindow(TRUE);//不可用
	}
}

void CPageGeneral::set_mac_serial_data()
{
	int check;

	USES_CONVERSION;
	/*
#if 0
	//如果是升级模式或spi烧录，那么MAC和序列号为不可用
	if (TRUE == theConfig.bUpdate || theConfig.burn_mode == E_CONFIG_SFLASH)
	{
		//配置命令行
		GetDlgItem(IDC_CHK_CONFIG_CMDLINE)->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_CHK_CONFIG_CMDLINE))->SetCheck(FALSE);

		//mac地址
		GetDlgItem(IDC_CHECK_MAC_ADDR)->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_MAC_ADDR))->SetCheck(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR))->SetCheck(FALSE);
		
		//序列号地址
		GetDlgItem(IDC_CHECK_SEQUENCE_ADDR)->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_SEQUENCE_ADDR))->SetCheck(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR))->SetCheck(FALSE);
	} 
	else
#endif
*/
	{	
		if (theConfig.planform_tpye == E_LINUX_PLANFORM)
		{
			GetDlgItem(IDC_CHK_CONFIG_CMDLINE)->ShowWindow(TRUE);//			
			GetDlgItem(IDC_CHK_CONFIG_CMDLINE)->EnableWindow(TRUE);
			GetDlgItem(IDC_GERNERAL_CMD_ADDR)->ShowWindow(TRUE);
			GetDlgItem(IDC_STATIC_0x)->ShowWindow(TRUE);
			GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->ShowWindow(TRUE);
			GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->EnableWindow(TRUE);
			GetDlgItem(IDC_GERNERAL_CMD_DATA)->ShowWindow(TRUE);
			GetDlgItem(IDC_EDIT_CMDLINE_DATA)->ShowWindow(TRUE);
			GetDlgItem(IDC_EDIT_CMDLINE_DATA)->EnableWindow(TRUE);
			GetDlgItem(IDC_STATIC_L2)->ShowWindow(TRUE);	
			GetDlgItem(IDC_STATIC_L1)->ShowWindow(TRUE);

			GetDlgItem(IDC_EDIT_MAC_START_ADDR_HIGH)->ShowWindow(TRUE);//mac start high
			GetDlgItem(IDC_EDIT_MAC_START_ADDR_LOW)->ShowWindow(TRUE);//mac start low
			GetDlgItem(IDC_EDIT_MAC_END_ADDR_HIGH)->ShowWindow(TRUE);//mac end high
			GetDlgItem(IDC_EDIT_MAC_END_ADDR_LOW)->ShowWindow(TRUE);//mac end low
			GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR)->ShowWindow(TRUE);//mac force write
			GetDlgItem(IDC_MACADDR_RESET)->ShowWindow(TRUE);//reset
			
			GetDlgItem(IDC_STATIC_MAC_START_NAME)->ShowWindow(TRUE);//静态标志
			GetDlgItem(IDC_STATIC_MAC_START_ADDR)->ShowWindow(TRUE);//
			GetDlgItem(IDC_STATIC_MAC_END_NAME)->ShowWindow(TRUE);//
			GetDlgItem(IDC_STATIC_MAC_END_ADDR)->ShowWindow(TRUE);//

			GetDlgItem(IDC_CHECK_MAC_ADDR)->ShowWindow(TRUE);//
			GetDlgItem(IDC_CHECK_MAC_ADDR)->EnableWindow(TRUE);	//		
			GetDlgItem(IDC_CHECK_SEQUENCE_ADDR)->ShowWindow(TRUE);//
			GetDlgItem(IDC_CHECK_SEQUENCE_ADDR)->EnableWindow(TRUE);//
			GetDlgItem(IDC_STATIC_k1)->ShowWindow(TRUE);
            
			CDlgConfig* p = (CDlgConfig*)GetParent()->GetParent();
			CPageHardware *theAppGenral = &p->m_page_hardware;

			theAppGenral->SetBlgAttr(theConfig.planform_tpye);
			//CPageHardware* p = (CPageHardware*)GetParent()->GetParent();

			//p->GetDlgItem(IDC_STATIC_BURNED)->ShowWindow(FALSE);
			
			CWnd *pWnd;
			CRect rect;
			CWnd *pWnd_Flag;
			CRect rect_Flag;

			pWnd_Flag = GetDlgItem(IDC_STATIC_L1); //选取一个控件作为标识
			pWnd_Flag->GetWindowRect(&rect_Flag);
			pWnd = GetDlgItem(IDC_STATIC_k2); //获取编辑控件指针
			pWnd->GetWindowRect(&rect);

			if ((rect_Flag.top + 59) > rect.top)
			{
				for(int aa = 12; aa < 42;aa++)
				{
					pWnd = GetDlgItem(ctl_array[aa]); //获取编辑控件指针
					pWnd->GetWindowRect(&rect);
					ScreenToClient(&rect);
					pWnd->SetWindowPos( NULL,rect.left,rect.top + 210,0,0,SWP_NOZORDER | SWP_NOSIZE ); //移动位置
				}
			}			
		}
		if (theConfig.planform_tpye == E_ROST_PLANFORM)
		{		
			GetDlgItem(IDC_CHK_CONFIG_CMDLINE)->ShowWindow(FALSE);
			GetDlgItem(IDC_GERNERAL_CMD_ADDR)->ShowWindow(FALSE);
			GetDlgItem(IDC_STATIC_0x)->ShowWindow(FALSE);
			GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->ShowWindow(FALSE);
			GetDlgItem(IDC_GERNERAL_CMD_DATA)->ShowWindow(FALSE);
			GetDlgItem(IDC_EDIT_CMDLINE_DATA)->ShowWindow(FALSE);
			GetDlgItem(IDC_STATIC_L2)->ShowWindow(FALSE);	
			GetDlgItem(IDC_STATIC_L1)->ShowWindow(FALSE);
			
			GetDlgItem(IDC_CHECK_MAC_ADDR)->ShowWindow(FALSE);
			GetDlgItem(IDC_STATIC_k1)->ShowWindow(FALSE);
			GetDlgItem(IDC_EDIT_MAC_START_ADDR_HIGH)->ShowWindow(FALSE);
			GetDlgItem(IDC_EDIT_MAC_START_ADDR_LOW)->ShowWindow(FALSE);
			GetDlgItem(IDC_EDIT_MAC_END_ADDR_HIGH)->ShowWindow(FALSE);
			GetDlgItem(IDC_EDIT_MAC_END_ADDR_LOW)->ShowWindow(FALSE);
			GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR)->ShowWindow(FALSE);
			GetDlgItem(IDC_MACADDR_RESET)->ShowWindow(FALSE);
			
			GetDlgItem(IDC_STATIC_MAC_START_NAME)->ShowWindow(FALSE);
			GetDlgItem(IDC_STATIC_MAC_START_ADDR)->ShowWindow(FALSE);
			GetDlgItem(IDC_STATIC_MAC_END_NAME)->ShowWindow(FALSE);
			GetDlgItem(IDC_STATIC_MAC_END_ADDR)->ShowWindow(FALSE);

			((CButton *)GetDlgItem(IDC_CHECK_MAC_ADDR))->SetCheck(FALSE);
			((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR))->SetCheck(FALSE);

			CDlgConfig* p = (CDlgConfig*)GetParent()->GetParent();
			CPageHardware *theAppGenral = &p->m_page_hardware;

			theAppGenral->SetBlgAttr(theConfig.planform_tpye);
			
			CWnd *pWnd;
			CRect rect;
			CWnd *pWnd_Flag;
			CRect rect_Flag;

			pWnd_Flag = GetDlgItem(IDC_STATIC_L1); //选取一个控件作为标识
			pWnd_Flag->GetWindowRect(&rect_Flag);
			
			pWnd = GetDlgItem(IDC_STATIC_k2); //获取编辑控件指针
			pWnd->GetWindowRect(&rect);
			
			if (((rect_Flag.top + 59) < rect.top))
			{
				for(int aa = 12; aa < 42;aa++)
				{
					pWnd = GetDlgItem(ctl_array[aa]); //获取编辑控件指针
					pWnd->GetWindowRect(&rect);
					ScreenToClient(&rect);
					pWnd->SetWindowPos( NULL,rect.left,rect.top - 210,0,0,SWP_NOZORDER | SWP_NOSIZE ); //移动位置
				}
			}
			
			GetDlgItem(IDC_CHECK_SEQUENCE_ADDR)->ShowWindow(TRUE);//序列号显示可用
			GetDlgItem(IDC_CHECK_SEQUENCE_ADDR)->EnableWindow(TRUE);//可见
		}
	}

	//如果是升级模式或spi烧录，那么序列号为不可用
	if ((theConfig.planform_tpye != E_LINUX_PLANFORM && TRUE == theConfig.bUpdate
		&& theConfig.burn_mode != E_CONFIG_SFLASH) 
		|| (theConfig.burn_mode == E_CONFIG_SFLASH 
		&& theConfig.planform_tpye == E_ROST_PLANFORM))
	{	
		//序列号地址
		GetDlgItem(IDC_CHECK_SEQUENCE_ADDR)->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_SEQUENCE_ADDR))->SetCheck(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR))->SetCheck(FALSE);

		//MAC地址
		GetDlgItem(IDC_CHECK_MAC_ADDR)->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_MAC_ADDR))->SetCheck(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR))->SetCheck(FALSE);
	} 
	
	//配置命令行
	check = ((CButton *)GetDlgItem(IDC_CHK_CONFIG_CMDLINE))->GetCheck();	
    if (check)
    {
		GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->EnableWindow(TRUE);//
        GetDlgItem(IDC_EDIT_CMDLINE_DATA)->EnableWindow(TRUE);//
    }
    else
    {
		GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->EnableWindow(FALSE);//
        GetDlgItem(IDC_EDIT_CMDLINE_DATA)->EnableWindow(FALSE);//
	}

	//mac地址
	check = ((CButton *)GetDlgItem(IDC_CHECK_MAC_ADDR))->GetCheck();	
    if (check)
    {
		GetDlgItem(IDC_EDIT_MAC_START_ADDR_HIGH)->EnableWindow(TRUE); //mac start high
        GetDlgItem(IDC_EDIT_MAC_START_ADDR_LOW)->EnableWindow(TRUE); //mac start low
		GetDlgItem(IDC_EDIT_MAC_END_ADDR_HIGH)->EnableWindow(TRUE);//mac end high
        GetDlgItem(IDC_EDIT_MAC_END_ADDR_LOW)->EnableWindow(TRUE);//mac end loe
		GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR)->EnableWindow(TRUE);//force write
		GetDlgItem(IDC_MACADDR_RESET)->EnableWindow(TRUE);//reset
    }
    else
    {
		GetDlgItem(IDC_EDIT_MAC_START_ADDR_HIGH)->EnableWindow(FALSE); //mac start high
        GetDlgItem(IDC_EDIT_MAC_START_ADDR_LOW)->EnableWindow(FALSE);//mac start low
		GetDlgItem(IDC_EDIT_MAC_END_ADDR_HIGH)->EnableWindow(FALSE);//mac end high
        GetDlgItem(IDC_EDIT_MAC_END_ADDR_LOW)->EnableWindow(FALSE);//mac endlow
		GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR)->EnableWindow(FALSE);//force write
		GetDlgItem(IDC_MACADDR_RESET)->EnableWindow(FALSE);//reset
	}

	//序列号地址
	check = ((CButton *)GetDlgItem(IDC_CHECK_SEQUENCE_ADDR))->GetCheck();	
    if (check)
    {
		GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_HIGH)->EnableWindow(TRUE);//序列号
        GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_LOW)->EnableWindow(TRUE);//
		GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_HIGH)->EnableWindow(TRUE);//
        GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_LOW)->EnableWindow(TRUE);//
		GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR)->EnableWindow(TRUE);//
		GetDlgItem(IDC_SEQUENCE_ADDR_RESET)->EnableWindow(TRUE);//
    }
    else
    {	
		GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_HIGH)->EnableWindow(FALSE);//
        GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_LOW)->EnableWindow(FALSE);//
		GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_HIGH)->EnableWindow(FALSE);//
        GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_LOW)->EnableWindow(FALSE);//
		GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR)->EnableWindow(FALSE);//
		GetDlgItem(IDC_SEQUENCE_ADDR_RESET)->EnableWindow(FALSE);//
    }
}



BOOL CPageGeneral::get_config_data(CConfig &config)
{
	CString str;
	int check;
	TCHAR dstBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR surBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	if(!check_input())//检查正确性
	{
		return FALSE;
	}

	USES_CONVERSION;

	//get project name
	GetDlgItemText(IDC_EDIT_PROJECT_NAME, str);
	_tcsncpy(config.project_name, str, MAX_PROJECT_NAME);//工程名

	//get device number
	GetDlgItemText(IDC_EDIT_DEVICE_NUM, str);
	config.device_num = atoi(T2A(str));	//通道数

	if(config.device_num > MAX_DEVICE_NUM)
	{
		config.device_num = MAX_DEVICE_NUM;//超过最大通道数，就用最大的
	}

	if(0 == config.device_num)
	{
		config.device_num = 1;//最少是1个
	}


	GetDlgItemText(IDC_COMBO_PALNFORM_TYPE, str);
	_tcsncpy(config.planform_name, str, MAX_PROJECT_NAME);//工程名

	
	//get com data
	check = ((CButton *)GetDlgItem(IDC_CHECK_OPEN_COM))->GetCheck();
	config.bOpenCOM = check ? TRUE : FALSE;

	if(check)
	{
		GetDlgItemText(IDC_EDIT_COM_NUM, str);
		config.com_count = atoi(T2A(str));

		if(config.com_count > MAX_DEVICE_NUM)
		{
			config.com_count = MAX_DEVICE_NUM;
		}

		if(config.com_count == 0)
		{
			config.com_count = 1;
		}

		GetDlgItemText(IDC_EDIT_COM_BASE, str);
		config.com_base = atoi(T2A(str));//com_base

		GetDlgItemText(IDC_EDIT_BAUDRATE, str);
		config.com_baud_rate = atoi(T2A(str));//com_baud_rate
	}
	
	//get path
	GetDlgItemText(IDC_EDIT_PATH_PRODUCER, str);
	_tcsncpy(config.path_producer, str, MAX_PATH);//produce

	GetDlgItemText(IDC_EDIT_PATH_NANDBOOT, str);
	_tcsncpy(config.path_nandboot, str, MAX_PATH);//boot

	GetDlgItemText(IDC_EDIT_PATH_NANDBOOT_NEW, str);
	_tcsncpy(config.path_nandboot_new, str, MAX_PATH);//boot

	GetDlgItemText(IDC_EDIT_PROD_RUN_ADDR, str);
	config.producer_run_addr = config.hex2int(T2A(str));//produce run addr

//	GetDlgItemText(IDC_EDIT_PATH_BIOS, str);
//	_tcsncpy(config.path_bios, str, MAX_PATH);
	GetDlgItemText(IDC_EDIT_CMDLINE_ADDR, str);
	config.cmdLine.cmdAddr = config.hex2int(T2A(str));//cmdAddr

	GetDlgItemText(IDC_EDIT_CMDLINE_DATA, str);
	strncpy(config.cmdLine.CmdData, T2A(str), CMDLINE_LEN);

	if (_tcscmp(config.planform_name, A2T("LINUX")) == 0)
	{
		config.planform_tpye = E_LINUX_PLANFORM;
		
		GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->ShowWindow(TRUE);//可用
		GetDlgItem(IDC_EDIT_CMDLINE_DATA)->ShowWindow(TRUE);//可用
		GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->EnableWindow(TRUE);//可见
		GetDlgItem(IDC_EDIT_CMDLINE_DATA)->EnableWindow(TRUE);//可见
	}
	else if (_tcscmp(config.planform_name, A2T("RTOS")) == 0)
	{
		config.planform_tpye = E_ROST_PLANFORM;
		GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->ShowWindow(FALSE);//不可用
		GetDlgItem(IDC_EDIT_CMDLINE_DATA)->ShowWindow(FALSE);//不可用
	}
	else//
	{
		str.Format(_T("planform tpye is error,please check config planform tpye"));
		AfxMessageBox(str, MB_OK);
	}

	//如果是升级模式，那么MAC和序列号为不可用
	set_mac_serial_data();
		
		
	GetDlgItemText(IDC_EVENT_TIME, str);
    config.event_wait_time = atoi(T2A(str));//等待时间

	GetDlgItemText(IDC_EDIT_PID, str);
    config.PID = config.hex2int(T2A(str));//pid

	GetDlgItemText(IDC_EDIT_VID, str);
    config.VID = config.hex2int(T2A(str));//vid

	//get MAC ADDR
	GetDlgItemText(IDC_EDIT_MAC_START_ADDR_HIGH, str);
	_tcsncpy(surBuf, str, MAX_MAC_SEQU_ADDR_COUNT);
	//把读出mac_start_high地址转换出大写
	theConfig.lower_to_upper(surBuf, dstBuf);
	_tcscpy(config.mac_start_high, dstBuf);
	memset(surBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
	memset(dstBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
	GetDlgItemText(IDC_EDIT_MAC_START_ADDR_LOW, str);
	_tcsncpy(surBuf, str, MAX_MAC_SEQU_ADDR_COUNT);
	//把读出mac_start_low地址转换出大写
	theConfig.lower_to_upper(surBuf, dstBuf);
	_tcscpy(config.mac_start_low, dstBuf);
	memset(surBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
	memset(dstBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);

	GetDlgItemText(IDC_EDIT_MAC_END_ADDR_HIGH, str);
	_tcsncpy(surBuf, str, MAX_MAC_SEQU_ADDR_COUNT);
	//把读出mac_end_high地址转换出大写
	theConfig.lower_to_upper(surBuf, dstBuf);
	_tcscpy(config.mac_end_high, dstBuf);
	memset(surBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
	memset(dstBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);

	GetDlgItemText(IDC_EDIT_MAC_END_ADDR_LOW, str);
	_tcsncpy(surBuf, str, MAX_MAC_SEQU_ADDR_COUNT);
	//把读出mac_end_low地址转换出大写
	theConfig.lower_to_upper(surBuf, dstBuf);
	_tcscpy(config.mac_end_low, dstBuf);
	memset(surBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
	memset(dstBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);

	_tcsncpy(config.mac_current_high, config.mac_start_high, MAX_MAC_SEQU_ADDR_COUNT);//当前高位
	
	if (_tcscmp (config.mac_start_low, config.mac_current_low) >= 0)
	{
		_tcsncpy(config.mac_current_low, config.mac_start_low, MAX_MAC_SEQU_ADDR_COUNT);//当前低位
	}

	//get SEQUENCE ADDR
	GetDlgItemText(IDC_EDIT_SEQUENCE_START_ADDR_HIGH, str);
	_tcsncpy(surBuf, str, MAX_MAC_SEQU_ADDR_COUNT);

	//把读出sequence_start_high地址转换出大写
	theConfig.lower_to_upper(surBuf, dstBuf);
	_tcscpy(config.sequence_start_high, dstBuf);
	memset(surBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
	memset(dstBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
	GetDlgItemText(IDC_EDIT_SEQUENCE_START_ADDR_LOW, str);

	_tcsncpy(config.sequence_start_low, str, MAX_MAC_SEQU_ADDR_COUNT);
	GetDlgItemText(IDC_EDIT_SEQUENCE_END_ADDR_HIGH, str);
	_tcsncpy(surBuf, str, MAX_MAC_SEQU_ADDR_COUNT);

	//把读出sequence_end_high地址转换出大写
	theConfig.lower_to_upper(surBuf, dstBuf);
	_tcscpy(config.sequence_end_high, dstBuf);
	memset(surBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
	memset(dstBuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
	GetDlgItemText(IDC_EDIT_SEQUENCE_END_ADDR_LOW, str);

	_tcsncpy(config.sequence_end_low, str, MAX_MAC_SEQU_ADDR_COUNT);
	_tcsncpy(config.sequence_current_high, config.sequence_start_high, MAX_MAC_SEQU_ADDR_COUNT);
	
	if (_tcscmp (config.sequence_start_low, config.sequence_current_low) >= 0)//比较
	{
		_tcsncpy(config.sequence_current_low, config.sequence_start_low, MAX_MAC_SEQU_ADDR_COUNT);
	}



	check = ((CButton *)GetDlgItem(IDC_CHECK_MAC_ADDR))->GetCheck();	
    if (check)
    {
		theConfig.macaddr_flag = TRUE;
		check = ((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR))->GetCheck();	
		if (check)
		{
			theConfig.fore_write_mac_addr = TRUE;//强制
		}
		else
		{
			theConfig.fore_write_mac_addr = FALSE;
		}
    }
    else
    {
		theConfig.macaddr_flag = FALSE;
		theConfig.fore_write_mac_addr = FALSE;
    }

	check = ((CButton *)GetDlgItem(IDC_CHECK_SEQUENCE_ADDR))->GetCheck();	
    if (check)
    {
		theConfig.sequenceaddr_flag = TRUE;
		check = ((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR))->GetCheck();	
		if (check)
		{
			theConfig.fore_write_serial_addr = TRUE;//强制写
		}
		else
		{
			theConfig.fore_write_serial_addr = FALSE;//非
		}
    }
    else
    {
		theConfig.sequenceaddr_flag = FALSE;//非
		theConfig.fore_write_serial_addr = FALSE;//非
    }

	set_nandboot_new_37L();

	return TRUE;
}

BOOL CPageGeneral::set_config_item(CConfig &config)
{
	CString str;

	USES_CONVERSION;

	//set project name
	SetDlgItemText(IDC_EDIT_PROJECT_NAME, config.project_name);//工程名

	//set device number
	str.Format(_T("%u"), config.device_num);//通道数
	SetDlgItemText(IDC_EDIT_DEVICE_NUM, str);
	
	//planform type
	_tcsncpy(config.init_planform_name, config.planform_name, MAX_PROJECT_NAME);
	SetDlgItemText(IDC_COMBO_PALNFORM_TYPE, config.planform_name);//平台类型
	
	//set com data
	check_com(config.bOpenCOM);
	
	//set path
	SetDlgItemText(IDC_EDIT_PATH_PRODUCER, config.path_producer);//path_producer
	SetDlgItemText(IDC_EDIT_PATH_NANDBOOT, config.path_nandboot);//path_nandboot
	SetDlgItemText(IDC_EDIT_PATH_NANDBOOT_NEW, config.path_nandboot_new);
	
	str.Format(_T("%08x"), theConfig.producer_run_addr);//producer_run_addr
	GetDlgItem(IDC_EDIT_PROD_RUN_ADDR)->SetWindowText(str);

	str.Format(_T("%x"), theConfig.cmdLine.cmdAddr);//cmdAddr
	GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->SetWindowText(str);

	str.Format(_T("%s"), A2T(theConfig.cmdLine.CmdData));//CmdData
	GetDlgItem(IDC_EDIT_CMDLINE_DATA)->SetWindowText(str);

	((CButton *)GetDlgItem(IDC_CHK_CONFIG_CMDLINE))->SetCheck(theConfig.cmdLine.bCmdLine);
	GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->EnableWindow(theConfig.cmdLine.bCmdLine);
	GetDlgItem(IDC_EDIT_CMDLINE_DATA)->EnableWindow(theConfig.cmdLine.bCmdLine);


	if (_tcscmp(config.planform_name, A2T("LINUX")) == 0)
	{
		config.planform_tpye = E_LINUX_PLANFORM;
		
		GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->ShowWindow(TRUE);//可用
		GetDlgItem(IDC_EDIT_CMDLINE_DATA)->ShowWindow(TRUE);//可用
		GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->EnableWindow(TRUE);//可见
		GetDlgItem(IDC_EDIT_CMDLINE_DATA)->EnableWindow(TRUE);//可见
	}
	else if (_tcscmp(config.planform_name, A2T("RTOS")) == 0)
	{
		config.planform_tpye = E_ROST_PLANFORM;
		GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->ShowWindow(FALSE);//不可用
		GetDlgItem(IDC_EDIT_CMDLINE_DATA)->ShowWindow(FALSE);//不可用
	}
	else//
	{
		str.Format(_T("planform tpye is error,please check config planform tpye"));
		AfxMessageBox(str, MB_OK);
	}

	//如果是升级模式，那么MAC和序列号为不可用
	set_mac_serial_data();
	
	//
	/*
	GetDlgItem(IDC_CHK_CONFIG_CMDLINE)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_CMDLINE_DATA)->EnableWindow(FALSE);
*/
    str.Format(_T("%d"), theConfig.event_wait_time);
    GetDlgItem(IDC_EVENT_TIME)->SetWindowText(str);

	((CButton *)GetDlgItem(IDC_CHECK_MAC_ADDR))->SetCheck(theConfig.macaddr_flag);
	if (theConfig.macaddr_flag)
	{
		GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR)->EnableWindow(TRUE);//可见
		((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR))->SetCheck(theConfig.fore_write_mac_addr);
	}
	else
	{
		GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR)->EnableWindow(FALSE);//不可见
		((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR))->SetCheck(FALSE);
	}
	
	
	GetDlgItem(IDC_EDIT_MAC_START_ADDR_HIGH)->EnableWindow(theConfig.macaddr_flag);//开始高
	GetDlgItem(IDC_EDIT_MAC_START_ADDR_LOW)->EnableWindow(theConfig.macaddr_flag);//开始低
	GetDlgItem(IDC_EDIT_MAC_END_ADDR_HIGH)->EnableWindow(theConfig.macaddr_flag);
	GetDlgItem(IDC_EDIT_MAC_END_ADDR_LOW)->EnableWindow(theConfig.macaddr_flag);
	GetDlgItem(IDC_MACADDR_RESET)->EnableWindow(theConfig.macaddr_flag);

	str.Format(_T("%s"), theConfig.mac_start_high);
	GetDlgItem(IDC_EDIT_MAC_START_ADDR_HIGH)->SetWindowText(str);
	str.Format(_T("%s"), theConfig.mac_start_low);
	GetDlgItem(IDC_EDIT_MAC_START_ADDR_LOW)->SetWindowText(str);
	str.Format(_T("%s"), theConfig.mac_end_high);
	GetDlgItem(IDC_EDIT_MAC_END_ADDR_HIGH)->SetWindowText(str);
	str.Format(_T("%s"), theConfig.mac_end_low);
	GetDlgItem(IDC_EDIT_MAC_END_ADDR_LOW)->SetWindowText(str);

	((CButton *)GetDlgItem(IDC_CHECK_SEQUENCE_ADDR))->SetCheck(theConfig.sequenceaddr_flag);
	if (theConfig.sequenceaddr_flag)
	{
		GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR)->EnableWindow(TRUE);
		((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR))->SetCheck(theConfig.fore_write_serial_addr);
	}
	else
	{
		GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR)->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR))->SetCheck(FALSE);
	}
	
	GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_HIGH)->EnableWindow(theConfig.sequenceaddr_flag);//
	GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_LOW)->EnableWindow(theConfig.sequenceaddr_flag);//
	GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_HIGH)->EnableWindow(theConfig.sequenceaddr_flag);//
	GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_LOW)->EnableWindow(theConfig.sequenceaddr_flag);//
	GetDlgItem(IDC_SEQUENCE_ADDR_RESET)->EnableWindow(theConfig.sequenceaddr_flag);//
	
	str.Format(_T("%s"), theConfig.sequence_start_high);
	GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_HIGH)->SetWindowText(str);//
	str.Format(_T("%s"), theConfig.sequence_start_low);
	GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_LOW)->SetWindowText(str);//
	str.Format(_T("%s"), theConfig.sequence_end_high);
	GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_HIGH)->SetWindowText(str);//
	str.Format(_T("%s"), theConfig.sequence_end_low);
	GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_LOW)->SetWindowText(str);//

	set_nandboot_new_37L();

	str.Format(_T("%04x"), theConfig.PID);
	GetDlgItem(IDC_EDIT_PID)->SetWindowText(str);
	str.Format(_T("%04x"), theConfig.VID);
	GetDlgItem(IDC_EDIT_VID)->SetWindowText(str);
	
	return TRUE;
}

BOOL CPageGeneral::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// TODO: Add extra initialization here
	SetupDisplay();

	set_config_item(theConfig);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPageGeneral::OnBtnBrowseProducer() 
{
	OPENFILENAME ofn;
	TCHAR pstrFileName[260] = {0}, pstrTitleName[260] = {0};
	
	TCHAR szFilter[] =	TEXT ("BIN Files (*.bin;*.nb0)\0*.bin;*.nb0\0")  \
		TEXT ("All Files (*.*)\0*.*\0\0") ;
	
	memset ( &ofn, 0, sizeof ( ofn ) );
	
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hInstance         = _hInstance ;
	ofn.hwndOwner         = GetSafeHwnd();
	ofn.lpstrFilter       = szFilter;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrDefExt       = TEXT ("bin") ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.Flags             = OFN_FILEMUSTEXIST; 
	
	
	if(GetOpenFileName (&ofn))
	{
		TCHAR * relative_path = pstrFileName;

		if((relative_path = _tcsstr(pstrFileName, theConfig.path_module)) != NULL)
		{
			relative_path = pstrFileName + _tcslen(theConfig.path_module);
			SetDlgItemText(IDC_EDIT_PATH_PRODUCER, relative_path);
		}
		else
		{
			SetDlgItemText(IDC_EDIT_PATH_PRODUCER, pstrFileName);
		}
	}
}

void CPageGeneral::OnBtnBrowseNandboot() 
{	
	OPENFILENAME ofn;
	TCHAR pstrFileName[260] = {0}, pstrTitleName[260] = {0};
	
	TCHAR szFilter[] =	TEXT ("BIN Files (*.bin;*.nb0)\0*.bin;*.nb0\0")  \
		TEXT ("All Files (*.*)\0*.*\0\0") ;
	
	memset ( &ofn, 0, sizeof ( ofn ) );
	
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hInstance         = _hInstance ;
	ofn.hwndOwner         = GetSafeHwnd();
	ofn.lpstrFilter       = szFilter;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrDefExt       = TEXT ("bin") ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.Flags             = OFN_FILEMUSTEXIST; 
	
	
	if(GetOpenFileName (&ofn))
	{
		TCHAR * relative_path = pstrFileName;

		if((relative_path = _tcsstr(pstrFileName, theConfig.path_module)) != NULL)
		{
			relative_path = pstrFileName + _tcslen(theConfig.path_module);
			SetDlgItemText(IDC_EDIT_PATH_NANDBOOT, relative_path);
		}
		else
		{
			SetDlgItemText(IDC_EDIT_PATH_NANDBOOT, pstrFileName);
		}
	}
	
}

void CPageGeneral::check_com(BOOL bCheck)
{
	CString str;

	if(bCheck)
	{
		//open check
		((CButton *)GetDlgItem(IDC_CHECK_OPEN_COM))->SetCheck(BST_CHECKED);

		//set text
		str.Format(_T("%u"), theConfig.com_count);//com_count
		SetDlgItemText(IDC_EDIT_COM_NUM, str);

		str.Format(_T("%u"), theConfig.com_base);//com_base
		SetDlgItemText(IDC_EDIT_COM_BASE, str);

		str.Format(_T("%u"), theConfig.com_baud_rate);//com_baud_rate
		SetDlgItemText(IDC_EDIT_BAUDRATE, str);

	    //set window enable	    
		GetDlgItem(IDC_EDIT_COM_NUM)->ShowWindow(TRUE);//
        GetDlgItem(IDC_EDIT_COM_BASE)->ShowWindow(TRUE);//
		GetDlgItem(IDC_EDIT_BAUDRATE)->ShowWindow(TRUE);//
		GetDlgItem(IDC_EDIT_COM_NUM)->EnableWindow(TRUE);//
        GetDlgItem(IDC_EDIT_COM_BASE)->EnableWindow(TRUE);//
		GetDlgItem(IDC_EDIT_BAUDRATE)->EnableWindow(TRUE);//
	}
	else
	{
		((CButton *)GetDlgItem(IDC_CHECK_OPEN_COM))->SetCheck(BST_UNCHECKED);

		//set text
		str.Format(_T("%u"), theConfig.com_count);//com_count
		SetDlgItemText(IDC_EDIT_COM_NUM, str);

		str.Format(_T("%u"), theConfig.com_base);//com_base
		SetDlgItemText(IDC_EDIT_COM_BASE, str);

		str.Format(_T("%u"), theConfig.com_baud_rate);//com_baud_rate
		SetDlgItemText(IDC_EDIT_BAUDRATE, str);

		GetDlgItem(IDC_EDIT_COM_NUM)->ShowWindow(FALSE);//
        GetDlgItem(IDC_EDIT_COM_BASE)->ShowWindow(FALSE);//
		GetDlgItem(IDC_EDIT_BAUDRATE)->ShowWindow(FALSE);	//	
	}
}

void CPageGeneral::OnCheckOpenCom() 
{
	// TODO: Add your control notification handler code here
	int check;

	check = ((CButton *)GetDlgItem(IDC_CHECK_OPEN_COM))->GetCheck();
	check_com(check);
}

//判断是否全0值
BOOL CPageGeneral::is_zero_ether_addr(TCHAR *addr)
{
	BOOL flag = FALSE;

	//0表示48
	if ((addr[0] == 48 && addr[1] == 48 && addr[3] == 48 
		&& addr[4] == 48 && addr[6] == 48 && addr[7] == 48))
	{
		flag = TRUE;//如果是0就为true
	}

	return flag;
}

//判断是否multicast
BOOL CPageGeneral::is_multicast_ether_addr(TCHAR *addr)
{
	UINT n = 0;
	CHAR num[2] = {0};

	USES_CONVERSION;//

	memcpy(num, T2A(addr+1), 1);//字符转整
	sscanf(num, "%x", &n);//

	return 0x01 & n;
}


BOOL CPageGeneral::check_input()
{
	UINT device_num, com_num;
	CString str;
	int check = 0;
	UINT len= 0;
	TCHAR surtempbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR dsttempbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	USES_CONVERSION;

	GetDlgItemText(IDC_EDIT_DEVICE_NUM, str);
	device_num = atoi(T2A(str));	

	if(device_num > MAX_DEVICE_NUM)
	{
		str.Format(_T("device number is large than max device num [%d]"), MAX_DEVICE_NUM);
		AfxMessageBox(str, MB_OK);

		return FALSE;
	}

	check = ((CButton *)GetDlgItem(IDC_CHECK_OPEN_COM))->GetCheck();
	if(check)
	{
		GetDlgItemText(IDC_EDIT_COM_NUM, str);
		com_num = atoi(T2A(str));//com_num
		
		if(com_num > device_num)
		{
			str.Format(_T("the com number is large than max device num"));
			AfxMessageBox(str, MB_OK);

			return FALSE;
		}

		if(com_num == 0)
		{
			str.Format(_T("the com number is 0!!"));
			AfxMessageBox(str, MB_OK);

			return FALSE;
		}
	}

	check = ((CButton *)GetDlgItem(IDC_CHECK_MAC_ADDR))->GetCheck();
	if(check)
	{

		GetDlgItemText(IDC_EDIT_MAC_START_ADDR_HIGH, str);	
		if (str.GetLength() < 8)
		{
			str.Format(theApp.GetString(IDS_MACADDR_START_HIGHLEN_8));
			//str.Format(_T("mac start addr high Get str Length less than 8 !!"));
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}
		
		GetDlgItemText(IDC_EDIT_MAC_START_ADDR_LOW, str);	
		if (str.GetLength() < 8)
		{
			str.Format(theApp.GetString(IDS_MACADDR_START_LOWLEN_8));
			//str.Format(_T("mac start addr low Get str Length less than 8 !!"));
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}
		
		GetDlgItemText(IDC_EDIT_MAC_END_ADDR_HIGH, str);	
		if (str.GetLength() < 8)
		{
			str.Format(theApp.GetString(IDS_MACADDR_END_HIGHLEN_8));
			//str.Format(_T("mac end addr high Get str Length less than 8 !!"));
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}
		GetDlgItemText(IDC_EDIT_MAC_END_ADDR_LOW, str);	
		if (str.GetLength() < 8)
		{
			str.Format(theApp.GetString(IDS_MACADDR_END_LOWLEN_8));
			//str.Format(_T("mac end addr low Get str Length less than 8 !!"));
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}

		memset(surtempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
		memset(dsttempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
		GetDlgItemText(IDC_EDIT_MAC_START_ADDR_HIGH, str);
		_tcsncpy(surtempbuf, str, MAX_MAC_SEQU_ADDR_COUNT);

		GetDlgItemText(IDC_EDIT_MAC_END_ADDR_HIGH, str);
		_tcsncpy(dsttempbuf, str, MAX_MAC_SEQU_ADDR_COUNT);

		if(_tcscmp(surtempbuf, dsttempbuf) != 0)
		{
			GetDlgItemText(IDC_EDIT_MAC_START_ADDR_HIGH, str);
			str.Format(theApp.GetString(IDS_MACADDR_START_END_HIGH_DIFFRENT));
			//str.Format(_T("mac start addr high and end addr high is differnt!!"));
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}


		//判断一下高位是否第2个字符的二进制中最后一位是否是1，如果是提示出错
		if(is_multicast_ether_addr(surtempbuf))//判断结束的高位
		{
			GetDlgItemText(IDC_EDIT_MAC_START_ADDR_HIGH, str);
			str.Format(theApp.GetString(IDS_MACADDR_HIGH_ONE));
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}
		//如果是全0，也提示出错
		if(is_zero_ether_addr(surtempbuf))
		{
			GetDlgItemText(IDC_EDIT_MAC_START_ADDR_HIGH, str);
			str.Format(theApp.GetString(IDS_MACADDR_HIGH_ZERO));
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}

		//判断结束的高位
		if(is_multicast_ether_addr(dsttempbuf))
		{
			GetDlgItemText(IDC_EDIT_MAC_END_ADDR_HIGH, str);
			str.Format(theApp.GetString(IDS_MACADDR_HIGH_ONE));
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}
		
		//如果是全0，也提示出错
		if(is_zero_ether_addr(dsttempbuf))
		{
			GetDlgItemText(IDC_EDIT_MAC_END_ADDR_HIGH, str);
			str.Format(theApp.GetString(IDS_MACADDR_HIGH_ZERO));
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}



		memset(surtempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
		memset(dsttempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
		GetDlgItemText(IDC_EDIT_MAC_START_ADDR_LOW, str);
		_tcsncpy(surtempbuf, str, MAX_MAC_SEQU_ADDR_COUNT);
		GetDlgItemText(IDC_EDIT_MAC_END_ADDR_LOW, str);
		_tcsncpy(dsttempbuf, str, MAX_MAC_SEQU_ADDR_COUNT);

		if(_tcscmp(surtempbuf, dsttempbuf) > 0)
		{
			str.Format(theApp.GetString(IDS_MACADDR_START_END_LOW_MOER));
			//str.Format(_T("mac start addr low is more than mac end addr low !!"));
			AfxMessageBox(str, MB_OK);
			return FALSE;
		}
	}

	check = ((CButton *)GetDlgItem(IDC_CHECK_SEQUENCE_ADDR))->GetCheck();
	if(check)
	{
		GetDlgItemText(IDC_EDIT_SEQUENCE_START_ADDR_HIGH, str);	
		if (str.GetLength() < 10)
		{
			str.Format(theApp.GetString(IDS_SERIALADDR_START_HIGHLEN_10));
			//str.Format(_T("serial start addr high Get str Length less than 10 !!"));
			AfxMessageBox(str, MB_OK);
			
			return FALSE;
		}
		GetDlgItemText(IDC_EDIT_SEQUENCE_START_ADDR_LOW, str);	
		if (str.GetLength() < 6)
		{
			str.Format(theApp.GetString(IDS_SERIALADDR_START_LOWLEN_6));
			//str.Format(_T("serial start addr low Get str Length less than 6 !!"));
			AfxMessageBox(str, MB_OK);
			
			return FALSE;
		}
		GetDlgItemText(IDC_EDIT_SEQUENCE_END_ADDR_HIGH, str);	
		if (str.GetLength() < 10)
		{
			str.Format(theApp.GetString(IDS_SERIALADDR_END_HIGHLEN_10));
			//str.Format(_T("serial end addr high Get str Length less than 10 !!"));
			AfxMessageBox(str, MB_OK);
			
			return FALSE;
		}
		GetDlgItemText(IDC_EDIT_SEQUENCE_END_ADDR_LOW, str);	
		if (str.GetLength() < 6)
		{
			str.Format(theApp.GetString(IDS_SERIALADDR_END_LOWLEN_6));
			//str.Format(_T("serial end addr low Get str Length less than 6 !!"));
			AfxMessageBox(str, MB_OK);
			
			return FALSE;
		}

		memset(surtempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
		memset(dsttempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
		GetDlgItemText(IDC_EDIT_SEQUENCE_START_ADDR_HIGH, str);
		_tcsncpy(surtempbuf, str, MAX_MAC_SEQU_ADDR_COUNT);
		GetDlgItemText(IDC_EDIT_SEQUENCE_END_ADDR_HIGH, str);
		_tcsncpy(dsttempbuf, str, MAX_MAC_SEQU_ADDR_COUNT);
		

		if(_tcscmp(surtempbuf, dsttempbuf) != 0)
		{
			str.Format(theApp.GetString(IDS_SERIALADDR_START_END_HIGH_DIFFRENT));
			//str.Format(_T("serial start addr high and end addr high is differnt!!"));
			AfxMessageBox(str, MB_OK);
			
			return FALSE;
		}

		memset(surtempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
		memset(dsttempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
		GetDlgItemText(IDC_EDIT_SEQUENCE_START_ADDR_LOW, str);
		_tcsncpy(surtempbuf, str, MAX_MAC_SEQU_ADDR_COUNT);
		GetDlgItemText(IDC_EDIT_SEQUENCE_END_ADDR_LOW, str);
		_tcsncpy(dsttempbuf, str, MAX_MAC_SEQU_ADDR_COUNT);
		if(_tcscmp(surtempbuf, dsttempbuf) > 0)
		{
			str.Format(theApp.GetString(IDS_SERIALADDR_START_END_LOW_MOER));
			//str.Format(_T("serial start addr low is more than end addr low !!"));
			AfxMessageBox(str, MB_OK);
			
			return FALSE;
		}
	}

	return TRUE;
}

void CPageGeneral::SetupDisplay()
{
	CString str;

	str = theApp.GetString(IDS_GENERAL_PROJECT_NAME);
	GetDlgItem(IDC_STATIC_PROJECT_NAME)->SetWindowText(str);

	str = theApp.GetString(IDS_GENERAL_CHANNEL_NUM);
	GetDlgItem(IDC_STATIC_DEVICE_NUM)->SetWindowText(str);
	
	str = theApp.GetString(IDS_PLANFORM_TYPE);
	GetDlgItem(IDC_PLANFORM_TYPE)->SetWindowText(str);

	str = theApp.GetString(IDS_GENERAL_OPEN_COM);
	GetDlgItem(IDC_CHECK_OPEN_COM)->SetWindowText(str);

	str = theApp.GetString(IDS_GENERAL_COM_NUM);
	GetDlgItem(IDC_STATIC_COM_NUM)->SetWindowText(str);

	str = theApp.GetString(IDS_GENERAL_COM_BASE);
	GetDlgItem(IDC_STATIC_COM_BASE)->SetWindowText(str);
	
	str = theApp.GetString(IDS_GENERAL_COM_BAUDRATE);
	GetDlgItem(IDC_STATIC_BAUDRATE)->SetWindowText(str);

    str = theApp.GetString(IDS_GENERAL_CONFIG_CMDLINE);
	GetDlgItem(IDC_CHK_CONFIG_CMDLINE)->SetWindowText(str);
	
    str = theApp.GetString(IDS_GENERAL_CMD_ADDR);
	GetDlgItem(IDC_GERNERAL_CMD_ADDR)->SetWindowText(str);

    str = theApp.GetString(IDS_GENERAL_CMD_DATA);
	GetDlgItem(IDC_GERNERAL_CMD_DATA)->SetWindowText(str);

	str = theApp.GetString(IDS_GENERAL_BURN_MAC_ADDR);
	GetDlgItem(IDC_CHECK_MAC_ADDR)->SetWindowText(str);
	str = theApp.GetString(IDS_GENERAL_FORCE_WRITE_MAC_ADDR);
	GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR)->SetWindowText(str);
	str = theApp.GetString(IDS_MACADDR_RESET);
	GetDlgItem(IDC_MACADDR_RESET)->SetWindowText(str);

	str = theApp.GetString(IDS_GENERAL_MAC_START_ADDR);
	GetDlgItem(IDC_STATIC_MAC_START_ADDR)->SetWindowText(str);
	str = theApp.GetString(IDS_GENERAL_MAC_END_ADDR);
	GetDlgItem(IDC_STATIC_MAC_END_ADDR)->SetWindowText(str);
	str = theApp.GetString(IDS_GENERAL_BURN_SEQUNCE);
	GetDlgItem(IDC_CHECK_SEQUENCE_ADDR)->SetWindowText(str);
	str = theApp.GetString(IDS_GENERAL_FORCE_WRITE_SEQUNCE);
	GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR)->SetWindowText(str);
	str = theApp.GetString(IDS_SEQUENCE_ADDR_RESET);
	GetDlgItem(IDC_SEQUENCE_ADDR_RESET)->SetWindowText(str);
	str = theApp.GetString(IDS_GENERAL_SEQUNCE_START);
	GetDlgItem(IDC_STATIC_SEQUENCE_START)->SetWindowText(str);
	str = theApp.GetString(IDS_GENERAL_SEQUNCE_END);
	GetDlgItem(IDC_STATIC_SEQUENCE_END)->SetWindowText(str);

	str = theApp.GetString(IDS_GENERAL_MAC_START_NAME);
	GetDlgItem(IDC_STATIC_MAC_START_NAME)->SetWindowText(str);
	str = theApp.GetString(IDS_GENERAL_MAC_END_NAME);
	GetDlgItem(IDC_STATIC_MAC_END_NAME)->SetWindowText(str);
	str = theApp.GetString(IDS_GENERAL_SERIAL_START_NAME);
	GetDlgItem(IDC_STATIC_SERIAL_START_NAME)->SetWindowText(str);
	str = theApp.GetString(IDS_GENERAL_SERIAL_END_NAME);
	GetDlgItem(IDC_STATIC_SERIAL_END_NAME)->SetWindowText(str);


	str = theApp.GetString(IDS_GENERAL_PATH_PRODUCER);
	GetDlgItem(IDC_STATIC_PATH_PRODUCER)->SetWindowText(str);
	
	str = theApp.GetString(IDS_GENERAL_PATH_NANDBOOT);
	GetDlgItem(IDC_STATIC_PATH_NANDBOOT)->SetWindowText(str);

	str = theApp.GetString(IDS_GENERAL_PATH_NANDBOOT_NEW);
	GetDlgItem(IDC_STATIC_PATH_NANDBOOT_NEW)->SetWindowText(str);

//	str = theApp.GetString(IDS_GENERAL_PATH_BIOS);
//	GetDlgItem(IDC_STATIC_PATH_BIOS)->SetWindowText(str);
	
	str = theApp.GetString(IDS_GENERAL_EXPLORER);
	GetDlgItem(IDC_BTN_BROWSE_PRODUCER)->SetWindowText(str);

	str = theApp.GetString(IDS_GENERAL_EXPLORER);
	GetDlgItem(IDC_BTN_BROWSE_NANDBOOT)->SetWindowText(str);

	str = theApp.GetString(IDS_GENERAL_EXPLORER);
	GetDlgItem(IDC_BTN_BROWSE_NANDBOOT_NEW)->SetWindowText(str);

//	str = theApp.GetString(IDS_GENERAL_EXPLORER);
//	GetDlgItem(IDC_BTN_BROWSE_BIOS)->SetWindowText(str);

}

void CPageGeneral::OnChangeEditProdRunAddr() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

	limit_addr_input(IDC_EDIT_PROD_RUN_ADDR);
}

void CPageGeneral::OnChkConfigCmdline() 
{
	// TODO: Add your control notification handler code here

	//theConfig.cmdLine.bCmdLine = !theConfig.cmdLine.bCmdLine;

	//GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->EnableWindow(theConfig.cmdLine.bCmdLine);
	//GetDlgItem(IDC_EDIT_CMDLINE_DATA)->EnableWindow(theConfig.cmdLine.bCmdLine);
	
	BOOL check = FALSE;

    check = ((CButton *)GetDlgItem(IDC_CHK_CONFIG_CMDLINE))->GetCheck();

    if (check)
    {
        GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_CMDLINE_DATA)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_EDIT_CMDLINE_ADDR)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_CMDLINE_DATA)->EnableWindow(FALSE);
    }
}

void CPageGeneral::limit_addr_input(UINT nID)
{
	CString str;

	GetDlgItemText(nID, str);

	if (str.GetLength() > 8)
	{
		str = str.Left(8);
		GetDlgItem(nID)->SetWindowText(str);
		GetDlgItem(nID)->SetFocus();
	}

	if (!str.IsEmpty())
	{
		UINT nLen = str.GetLength();
		if (((str.GetAt(nLen-1) >= '0') && (str.GetAt(nLen-1) <= '9'))
			|| ((str.GetAt(nLen-1) >= 'a') && (str.GetAt(nLen-1) <= 'f'))
			|| ((str.GetAt(nLen-1) >= 'A') && (str.GetAt(nLen-1) <= 'F')))
		{
		}
		else
		{
			str = str.Left(nLen-1);
			GetDlgItem(nID)->SetWindowText(str);
			GetDlgItem(IDC_STATIC)->SetFocus();
		}
	}
}

void CPageGeneral::OnChangeEditCmdlineAddr() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	limit_addr_input(IDC_EDIT_CMDLINE_ADDR);
}

void CPageGeneral::OnChangeEditCmdlineData() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	CString str;

	GetDlgItemText(IDC_EDIT_CMDLINE_DATA, str);

	if (str.GetLength() > (CMDLINE_LEN - 10))
	{
		str = str.Left(CMDLINE_LEN - 10);
		GetDlgItem(IDC_EDIT_CMDLINE_DATA)->SetWindowText(str);
	}
}

void CPageGeneral::OnCheckMacAddr() 
{	
	// TODO: Add your control notification handler code here
	CString str;
	BOOL check = FALSE;
	UINT i = 0;

    check = ((CButton *)GetDlgItem(IDC_CHECK_MAC_ADDR))->GetCheck();

    if (check)
    {
        GetDlgItem(IDC_EDIT_MAC_START_ADDR_HIGH)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_MAC_START_ADDR_LOW)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_MAC_END_ADDR_HIGH)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_MAC_END_ADDR_LOW)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR)->EnableWindow(TRUE);
		GetDlgItem(IDC_MACADDR_RESET)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_EDIT_MAC_START_ADDR_HIGH)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_MAC_START_ADDR_LOW)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_MAC_END_ADDR_HIGH)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_MAC_END_ADDR_LOW)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR)->EnableWindow(FALSE);
		GetDlgItem(IDC_MACADDR_RESET)->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_MAC_ADDR))->SetCheck(FALSE);
    }
	
	//show the MAC START AND END ADDR
}

void CPageGeneral::OnCheckForceWriteMacAddr() 
{
	// TODO: Add your control notification handler code here
	
}

void CPageGeneral::OnCheckForceWriteSequenceAddr() 
{
	// TODO: Add your control notification handler code here
}

void CPageGeneral::OnCheckSequenceAddr() 
{
	// TODO: Add your control notification handler code here
	BOOL check = FALSE;
	CString str;
	
    check = ((CButton *)GetDlgItem(IDC_CHECK_SEQUENCE_ADDR))->GetCheck();
	
    if (check)
    {
        GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_HIGH)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_LOW)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_HIGH)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_LOW)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR)->EnableWindow(TRUE);
		GetDlgItem(IDC_SEQUENCE_ADDR_RESET)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_HIGH)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_SEQUENCE_START_ADDR_LOW)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_HIGH)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_SEQUENCE_END_ADDR_LOW)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR)->EnableWindow(FALSE);
		GetDlgItem(IDC_SEQUENCE_ADDR_RESET)->EnableWindow(FALSE);
		((CButton *)GetDlgItem(IDC_CHECK_FORCE_WRITE_SEQUENCE_ADDR))->SetCheck(FALSE);
    }
	//show the MAC START AND END ADDR
}

BOOL CPageGeneral::Serial_iserror(UINT nID, UINT len, CString str, TCHAR *pBuf)
{
	UINT i = 0;
	UINT s = 0;
	BOOL falg = TRUE;
	
	for (i = 0; i < len; i++)
	{
		if (((str.GetAt(i) < '0') || ((str.GetAt(i) > '9')
			&& (str.GetAt(i) < 'A')) || ((str.GetAt(i) > 'Z')
			&& (str.GetAt(i) < 'a')) || (str.GetAt(i) > 'z')))
		{
			str.SetAt(i, pBuf[i]);
			SetDlgItemText(nID, str);
			((CEdit*)GetDlgItem(nID))->SetSel( i, i+1, FALSE );
			falg = FALSE;
			break;
		}
		if (islower(str.GetAt(i)))
		{	
			str.SetAt(i, toupper(str.GetAt(i)));
			GetDlgItem(nID)->SetWindowText(str);
			((CEdit*)GetDlgItem(nID))->SetSel( i+1, i+1, FALSE );
		}
	}
	return falg;
}

void CPageGeneral::limit_serial_addr_input(UINT nID, UINT len, TCHAR *pBuf)
{
	CString str;
	BOOL falg = TRUE;
	
	GetDlgItemText(nID, str);
	
	if (str.GetLength() > (int)len)
	{
		//str = str.Left(len);
		//GetDlgItem(nID)->SetWindowText(str);
		//GetDlgItem(nID)->SetFocus();
	}
	
	if (!str.IsEmpty())
	{
		UINT nLen = str.GetLength();

		if (!Serial_iserror( nID, nLen, str,  pBuf))
		{
			falg = FALSE;
		}
		if(falg == TRUE)
		{
			if (((str.GetAt(nLen-1) >= '0') && (str.GetAt(nLen-1) <= '9'))
				|| ((str.GetAt(nLen-1) >= 'a') && (str.GetAt(nLen-1) <= 'z'))
				|| ((str.GetAt(nLen-1) >= 'A') && (str.GetAt(nLen-1) <= 'Z')))
			{
			}
			else
			{
				((CEdit*)GetDlgItem(nID))->SetSel( nLen-1, nLen, FALSE );
				
			}
		}
	}
}

void CPageGeneral::limit_serial_addr_input_low(UINT nID, UINT len)
{
	CString str;
	
	GetDlgItemText(nID, str);
	
	if (str.GetLength() > (int)len)
	{
		//str = str.Left(len);
		//GetDlgItem(nID)->SetWindowText(str);
		//GetDlgItem(nID)->SetFocus();
	}
	
	if (!str.IsEmpty())
	{
		UINT nLen = str.GetLength();
		if ((str.GetAt(nLen-1) >= '0') && (str.GetAt(nLen-1) <= '9'))
		{
		}
		else
		{
			GetDlgItemText(nID, str);	
			str = str.Left(nLen-1);
		    GetDlgItem(nID)->SetWindowText(str);
			((CEdit*)GetDlgItem(nID))->SetSel( nLen-1, nLen-1, FALSE );
		}
	}
}

BOOL CPageGeneral::Mac_iserror(UINT nID, UINT len, CString str, TCHAR *pBuf)
{
	UINT i = 0;
	UINT s = 0;
	BOOL falg = TRUE;

	for (i = 0; i < len; i++)
	{
		if (((i != 2) && (i != 5)) && ((str.GetAt(i) < '0') || ((str.GetAt(i) > '9')
			&& (str.GetAt(i) < 'A')) || ((str.GetAt(i) > 'F')
			&& (str.GetAt(i) < 'a')) || (str.GetAt(i) > 'f')))
		{
			str.SetAt(i, pBuf[i]);
			SetDlgItemText(nID, str);
			((CEdit*)GetDlgItem(nID))->SetSel( i, i+1, FALSE);
			falg = FALSE;
			break;
		}
		if ((i == 2 && (str.GetAt(i) != ':')) || (i == 5 && str.GetAt(i) != ':'))
		{
			GetDlgItemText(nID, str);	
			AfxMessageBox(theApp.GetString(IDS_MACADDR_FORMAT), MB_OK);			
			str.SetAt(i, pBuf[i]);
			SetDlgItemText(nID, str);
			((CEdit*)GetDlgItem(nID))->SetSel( i+1, i+1, FALSE);
			falg = FALSE;
			break;
		}
		if (islower(str.GetAt(i)))
		{	
			str.SetAt(i, toupper(str.GetAt(i)));
			GetDlgItem(nID)->SetWindowText(str);
			((CEdit*)GetDlgItem(nID))->SetSel( i+1, i+1, FALSE );
		}
		
	}
	return falg;
}

void CPageGeneral::limit_mac_addr_input(UINT nID, UINT len, TCHAR *pBuf)
{
	CString str;
	UINT i = 0;
	UINT s = 0;
	BOOL falg = TRUE;

	GetDlgItemText(nID, str);
	
	if (str.GetLength() > (int)len)
	{
		//str = str.Left(len);
		//GetDlgItem(nID)->SetWindowText(str);
		//GetDlgItem(nID)->SetFocus();
	}
	
	if (!str.IsEmpty())
	{		
		UINT nLen = str.GetLength();

		if (!Mac_iserror( nID, nLen, str,  pBuf))
		{
			falg = FALSE;
		}

		if(falg == TRUE)
		{
			if ((nLen == 3 && (str.GetAt(nLen-1) != ':')) || (nLen == 6 && str.GetAt(nLen-1) != ':'))
			{
				GetDlgItemText(nID, str);	
				AfxMessageBox(theApp.GetString(IDS_MACADDR_FORMAT), MB_OK);

				str.SetAt(nLen-1, ':');
				GetDlgItem(nID)->SetWindowText(str);
				((CEdit*)GetDlgItem(nID))->SetSel( nLen-1, nLen-1, FALSE );
			}	
			else if (((nLen != 3) && (nLen != 6)) && (((str.GetAt(nLen-1) >= '0') && (str.GetAt(nLen-1) <= '9'))
				|| ((str.GetAt(nLen-1) >= 'a') && (str.GetAt(nLen-1) <= 'f'))
				|| ((str.GetAt(nLen-1) >= 'A') && (str.GetAt(nLen-1) <= 'F'))))
			{
			}
			else if ((nLen == 3 && (str.GetAt(nLen-1) == ':')) || (nLen == 6 && str.GetAt(nLen-1) == ':'))
			{
			}
			else
			{
				GetDlgItemText(nID, str);	
				str = str.Left(nLen-1);
				GetDlgItem(nID)->SetWindowText(str);
				((CEdit*)GetDlgItem(nID))->SetSel( nLen-1, nLen-1, FALSE );
			}
		}
	}
}

void CPageGeneral::OnChangeEditMacStartAddrLow() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here

	limit_mac_addr_input(IDC_EDIT_MAC_START_ADDR_LOW, 8, theConfig.mac_start_low);
}

void CPageGeneral::OnChangeEditMacStartAddrHigh() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	limit_mac_addr_input(IDC_EDIT_MAC_START_ADDR_HIGH, 8, theConfig.mac_start_high);
	
}

void CPageGeneral::OnChangeEditMacEndAddrHigh() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	limit_mac_addr_input(IDC_EDIT_MAC_END_ADDR_HIGH, 8, theConfig.mac_end_high);

}

void CPageGeneral::OnChangeEditMacEndAddrLow() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	limit_mac_addr_input(IDC_EDIT_MAC_END_ADDR_LOW, 8, theConfig.mac_end_low);

	
}

void CPageGeneral::OnChangeEditSequenceStartAddrHigh() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	limit_serial_addr_input(IDC_EDIT_SEQUENCE_START_ADDR_HIGH, 10, theConfig.sequence_start_high);
}

void CPageGeneral::OnChangeEditSequenceStartAddrLow() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	limit_serial_addr_input_low(IDC_EDIT_SEQUENCE_START_ADDR_LOW, 6);
}

void CPageGeneral::OnChangeEditSequenceEndAddrHigh() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	limit_serial_addr_input(IDC_EDIT_SEQUENCE_END_ADDR_HIGH, 10, theConfig.sequence_end_high);
}

void CPageGeneral::OnChangeEditSequenceEndAddrLow() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	limit_serial_addr_input_low(IDC_EDIT_SEQUENCE_END_ADDR_LOW, 6);
}

void CPageGeneral::OnSelchangeComboPalnformType() 
{
	// TODO: Add your control notification handler code here
	CString get_str;
	
	CComboBox* cbo = (CComboBox*)GetDlgItem(IDC_COMBO_PALNFORM_TYPE);
    USES_CONVERSION;
	cbo->GetLBText(cbo->GetCurSel(), get_str);
	if (_tcscmp(get_str, A2T("LINUX")) == 0)
	{
		theConfig.planform_tpye = E_LINUX_PLANFORM;
	}
	if (_tcscmp(get_str, A2T("RTOS")) == 0)
	{
		theConfig.planform_tpye = E_ROST_PLANFORM;
	}
	_tcsncpy(theConfig.planform_name, get_str, MAX_PROJECT_NAME);

	//如果是升级模式，那么MAC和序列号为不可用
	set_mac_serial_data();
}

void CPageGeneral::OnSequenceAddrReset() 
{
	CString str;
	UINT nID = 0;
	TCHAR tmpBufFlag[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR tmpBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};


	// TODO: Add your control notification handler code here
	GetDlgItemText(IDC_EDIT_SEQUENCE_START_ADDR_LOW, str);
	_tcsncpy(theConfig.sequence_start_low, str, MAX_MAC_SEQU_ADDR_COUNT);
	_tcsncpy(theConfig.sequence_current_low, theConfig.sequence_start_low, MAX_MAC_SEQU_ADDR_COUNT);

	for (nID = 0; nID < 32; nID++)
	{
		swprintf(tmpBufFlag,_T("%s_%d_F"), sequencecurrentlow, nID);
		GetPrivateProfileString(_T("config_addr"), tmpBufFlag, NULL, tmpBuf, MAX_MAC_SEQU_ADDR_COUNT, theApp.ConvertAbsolutePath(CONFIG_ADDR_FILE_NAME));
		if (tmpBuf[0] == 0)
		{
			continue;
		}
		else
		{
			theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, _T(""), _T(""), nID);
		}
	}
	str.Format(theApp.GetString(IDS_SEQUENCE_RESET_SUCCEED));
	AfxMessageBox(str, MB_OK);
}

void CPageGeneral::OnMacaddrReset() 
{
	CString str;
	UINT nID = 0;
	TCHAR tmpBufFlag[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR tmpBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};


	// TODO: Add your control notification handler code here
	GetDlgItemText(IDC_EDIT_MAC_START_ADDR_LOW, str);
	_tcsncpy(theConfig.mac_start_low, str, MAX_MAC_SEQU_ADDR_COUNT);
	_tcsncpy(theConfig.mac_current_low, theConfig.mac_start_low, MAX_MAC_SEQU_ADDR_COUNT);

	for (nID = 0; nID < 32; nID++)
	{
		swprintf(tmpBufFlag,_T("%s_%d_F"), maccurrentlow, nID);
		GetPrivateProfileString(_T("config_addr"), tmpBufFlag, NULL, tmpBuf, MAX_MAC_SEQU_ADDR_COUNT, theApp.ConvertAbsolutePath(CONFIG_ADDR_FILE_NAME));
		if (tmpBuf[0] == 0)
		{
			continue;
		}
		else
		{
			theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, _T(""), _T(""), nID);
		}
	}

	str.Format(theApp.GetString(IDS_MACADDR_RESET_SUCCEED));
	AfxMessageBox(str, MB_OK);
}

//
void CPageGeneral::limit_pid_vid_input(UINT nID, UINT len)
{
	CString str;
	
	//A获取值
	GetDlgItemText(nID, str);
	
	//控制长度不超过4个
	if (str.GetLength() > 4)
	{
		AfxMessageBox(_T("lenght more than 4"), MB_OK);	
		str = str.Left(4);
		GetDlgItem(nID)->SetWindowText(str);
		GetDlgItem(nID)->SetFocus();
	}
	
	if (!str.IsEmpty())
	{
		UINT nLen = str.GetLength();
		if (((str.GetAt(nLen-1) >= '0') && (str.GetAt(nLen-1) <= '9'))
			|| ((str.GetAt(nLen-1) >= 'a') && (str.GetAt(nLen-1) <= 'f'))
			|| ((str.GetAt(nLen-1) >= 'A') && (str.GetAt(nLen-1) <= 'F')))
		{
		}
		else
		{
			str = str.Left(nLen-1);
			GetDlgItem(nID)->SetWindowText(str);
			GetDlgItem(nID)->SetFocus();
		}
	}
}

void CPageGeneral::OnChangeEditPid() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	limit_pid_vid_input(IDC_EDIT_PID, 4);

	
}

void CPageGeneral::OnChangeEditVid() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CPropertyPage::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	limit_pid_vid_input(IDC_EDIT_VID, 4);
}

void CPageGeneral::OnBtnBrowseNandbootNew() 
{
	// TODO: Add your control notification handler code here	
	OPENFILENAME ofn;
	TCHAR pstrFileName[260] = {0}, pstrTitleName[260] = {0};
	
	TCHAR szFilter[] =	TEXT ("BIN Files (*.bin;*.nb0)\0*.bin;*.nb0\0")  \
		TEXT ("All Files (*.*)\0*.*\0\0") ;
	
	memset ( &ofn, 0, sizeof ( ofn ) );
	
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hInstance         = _hInstance ;
	ofn.hwndOwner         = GetSafeHwnd();
	ofn.lpstrFilter       = szFilter;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrDefExt       = TEXT ("bin") ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.Flags             = OFN_FILEMUSTEXIST; 
	
	
	if(GetOpenFileName (&ofn))
	{
		TCHAR * relative_path = pstrFileName;
		
		if((relative_path = _tcsstr(pstrFileName, theConfig.path_module)) != NULL)
		{
			relative_path = pstrFileName + _tcslen(theConfig.path_module);
			SetDlgItemText(IDC_EDIT_PATH_NANDBOOT_NEW, relative_path);
		}
		else
		{
			SetDlgItemText(IDC_EDIT_PATH_NANDBOOT_NEW, pstrFileName);
		}
	}
}
