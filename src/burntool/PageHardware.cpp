// PageHardware.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "PageHardware.h"
#include "PageGeneral.h"
#include "DlgConfig.h"
#include "TRANSC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CConfig theConfig;
extern CBurnToolApp theApp;
extern HINSTANCE _hInstance;
extern BOOL  ramImportFlag;

extern BOOL import_ram_config(LPCTSTR lpFileName);
extern BOOL export_ram_config(LPCTSTR lpFileName,UINT ui_data);
extern BOOL get_UI_data(UINT *data);


/////////////////////////////////////////////////////////////////////////////
// CPageHardware property page

IMPLEMENT_DYNCREATE(CPageHardware, CPropertyPage)

CPageHardware::CPageHardware() : CPropertyPage(CPageHardware::IDD)
{
	//{{AFX_DATA_INIT(CPageHardware)
	m_feq = 0;
	m_radio_burn_mode = -1;
	initFlag = FALSE;
	//}}AFX_DATA_INIT
}


CPageHardware::~CPageHardware()
{
	int i;
	i=0;
}

void CPageHardware::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageHardware)
	DDX_Control(pDX, IDC_CHIP_TYPE_LIST, m_chip_type_list);
    DDX_Control(pDX, IDC_COMBO_CHIP_SEL, m_chip_sel_list);
	DDX_Text(pDX, IDC_EDIT_FREQ, m_feq);
	DDV_MinMaxUInt(pDX, m_feq, 100, 400);
	DDX_Radio(pDX, IDC_RADIO_NANDFLASH, m_radio_burn_mode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageHardware, CPropertyPage)
	//{{AFX_MSG_MAP(CPageHardware)
	ON_CBN_SELCHANGE(IDC_CHIP_TYPE_LIST, OnSelchangeChipTypeList)
	ON_BN_CLICKED(IDC_RADIO_MCP_MODE, OnRadioMcpMode)
	ON_BN_CLICKED(IDC_RADIO_SDRAM_MODE, OnRadioSdramMode)
	ON_BN_CLICKED(IDC_RADIO_DDR_MODE, OnRadioDdrMode)
	ON_BN_CLICKED(IDC_RADIO_DDR2_16_MODE, OnRadioDdr216Mode)
	ON_BN_CLICKED(IDC_RADIO_DDR2_32_MODE, OnRadioDdr232Mode)
	ON_BN_CLICKED(IDC_RADIO_DDR_MODE32, OnRadioDdrMode32)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_RAM_CONFIG, OnButtonExportRamConfig)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT_RAM_CONFIG, OnButtonImportRamConfig)
    ON_CBN_SELCHANGE(IDC_COMBO_CHIP_SEL, OnChipSelList)
    ON_BN_CLICKED(IDC_BURNED_NONE, OnRadioBurnedNone)
    ON_BN_CLICKED(IDC_BURNED_RESET, OnRadioBurnedReset)
    ON_BN_CLICKED(IDC_BURNED_POWER_OFF, OnRadioBurnedPoweroff)
    ON_BN_CLICKED(IDC_BURNED_GPIO, OnCheckGpio)
    ON_BN_CLICKED(IDC_BURNED_RTC_WAKUP, OnCheckRtcWakup)
    ON_BN_CLICKED(IDC_BURNED_PULLUP,OnRadioBurnPullup)
    ON_BN_CLICKED(IDC_BURNED_PULLDOWN,OnRadioBurnPulldown)
    ON_BN_CLICKED(IDC_CHECK_GPIO_PIN_SELECT,OnCheckLedGpio)
	ON_BN_CLICKED(IDC_RADIO_DDR_16_MOBILE, OnRadioDdr16Mobile)
	ON_BN_CLICKED(IDC_RADIO_DDR_32_MOBILE, OnRadioDdr32Mobile)
	ON_BN_CLICKED(IDC_CHECK_USB2, OnCheckUsb2)
	ON_BN_CLICKED(IDC_CHECK_UDISK_UPDATE, OnCheckUdiskUpdate)
	ON_BN_CLICKED(IDC_CHECK_UPDATE, OnCheckUpdate)
	ON_BN_CLICKED(IDC_RADIO_SFLASH, OnRadioSflash)
	ON_BN_CLICKED(IDC_RADIO_NANDFLASH, OnRadioNandflash)
	ON_BN_CLICKED(IDC_RADIO_SD, OnRadioSd)
	ON_BN_CLICKED(IDC_RADIO_DEBUG, OnRadioDebug)
	ON_BN_CLICKED(IDC_RADIO_JTAG, OnRadioJtag)
	ON_BN_CLICKED(IDC_CHECK_PLLFREP, OnCheckPllfrep)
	ON_BN_CLICKED(IDC_RADIO_SPI_NANDFLASH, OnRadioSpiNandflash)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int h_ctl_array[57] = {IDC_STATIC_SD_RAM,IDC_RADIO_MCP_MODE,IDC_RADIO_DDR_MODE,IDC_RADIO_DDR2_16_MODE,
		IDC_RADIO_DDR_16_MOBILE,IDC_RADIO_SDRAM_MODE,IDC_RADIO_DDR_MODE32,IDC_RADIO_DDR2_32_MODE,IDC_RADIO_DDR_32_MOBILE,
		IDC_STATIC_DDR_Freq,IDC_EDIT_FREQ,IDC_STATIC_Mhz,IDC_STATIC_RAM_SIZE,IDC_RAM_SIZE_LIST,IDC_STATIC_RAM_ROW,
		IDC_RAM_ROW_LIST,IDC_BUTTON_IMPORT_RAM_CONFIG,IDC_STATIC_RAM_BANK,IDC_RAM_BANK_LIST,IDC_STATIC_RAM_COLUMN,
		IDC_RAM_COLUMN_LIST,IDC_BUTTON_EXPORT_RAM_CONFIG,IDC_GROUP_GPIO,IDC_STATIC_CHIP_SELECT,IDC_COMBO_CHIP_SEL,
		IDC_STATIC_CE2_GPIO,IDC_EDIT_GPIO_CE2,IDC_STATIC_CE3_GPIO,IDC_EDIT_GPIO_CE3,IDC_CHECK_GPIO_PIN_SELECT,IDC_EDIT_GPIO_PIN,
		IDC_STATIC_BURN_MODE,IDC_RADIO_NANDFLASH,IDC_RADIO_DEBUG,IDC_RADIO_SD,IDC_RADIO_SFLASH,IDC_RADIO_JTAG,IDC_RADIO_SPI_NANDFLASH,
		IDC_TEST_RAM, IDC_UPDATESELF,IDC_CHECK_UPDATE,IDC_CHECK_UDISK_UPDATE,IDC_STATIC_BURNED,IDC_BURNED_NONE,IDC_BURNED_GPIO,
		IDC_GPIO_PIN_NUM,IDC_BURNED_RTC_WAKUP,IDC_BURNED_RESET,IDC_BURNED_LVH,IDC_BURNED_PULLUP,
		IDC_GPIO_PULLUP_EN,IDC_BURNED_RTC_LVH,IDC_BURNED_POWER_OFF,IDC_BURNED_LVL,IDC_BURNED_PULLDOWN,
		IDC_GPIO_PULLDOWN_EN,IDC_BURNED_RTC_LVL};


/////////////////////////////////////////////////////////////////////////////
// CPageHardware message handlers
BOOL CPageHardware::get_config_data(CConfig &config)
{
	CString str;
	int check;
	
	USES_CONVERSION;

	//get the chip type
	GetDlgItemText(IDC_CHIP_TYPE_LIST, str);
    
	if (str == _T("AK_37XX"))//CHIP_37XX
	{
		config.chip_type = CHIP_37XX;
		config.bUboot = true;
	}
	else if(str == _T("AK_39XX"))//CHIP_39XX
	{
		config.chip_type = CHIP_39XX;
		config.bUboot = true;
	}
	else if(str == _T("AK_780X"))//CHIP_780X
	{
		config.chip_type = CHIP_780X;
		config.bUboot = true;
	}
    else if(str == _T("AK_880X"))//CHIP_880X
	{
		config.chip_type = CHIP_880X;
		config.bUboot = true;
	}
	else if(str == _T("AK_980X"))//CHIP_980X
	{
		config.chip_type = CHIP_980X;
		config.bUboot = true;
	}
	else if(str == _T("AK_11XX"))//CHIP_11XX
	{
		config.chip_type = CHIP_11XX;
		config.bUboot = true;
	}
	else if(str == _T("AK_10XX"))//CHIP_10X6
	{
		config.chip_type = CHIP_10X6;
		config.bUboot = true;
	}
	else//CHIP_980X
	{
		config.chip_type = CHIP_980X;
		config.bUboot = true;
	}

	//get the usb2
	check = ((CButton *)GetDlgItem(IDC_CHECK_USB2))->GetCheck();//bUsb2
	config.bUsb2 = check ? TRUE : FALSE;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_PLLFREP))->GetCheck();//change pll frep
	config.bPLL_Frep_change = check ? TRUE : FALSE;
	

	//RAM TYPE
	if(((CButton *)GetDlgItem(IDC_RADIO_MCP_MODE))->GetCheck())
	{
		config.ram_param.type = RAM_TYPE_MCP;//RAM_TYPE_MCP
	}
	if(((CButton *)GetDlgItem(IDC_RADIO_SDRAM_MODE))->GetCheck())
	{
		config.ram_param.type = RAM_TYPE_SDR;//RAM_TYPE_SDR
	}
	if(((CButton *)GetDlgItem(IDC_RADIO_DDR_MODE))->GetCheck())
	{
		config.ram_param.type = RAM_TYPE_DDR16;//RAM_TYPE_DDR16
	}
	if(((CButton *)GetDlgItem(IDC_RADIO_DDR_MODE32))->GetCheck())
	{
		config.ram_param.type = RAM_TYPE_DDR32;//RAM_TYPE_DDR32
	}
	if(((CButton *)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->GetCheck())
	{
		config.ram_param.type = RAM_TYPE_DDR2_16;//RAM_TYPE_DDR2_16
	}
	if(((CButton *)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->GetCheck())
	{
		config.ram_param.type = RAM_TYPE_DDR2_32;//RAM_TYPE_DDR2_32
	}
	if(((CButton *)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->GetCheck())
	{
		config.ram_param.type = RAM_TYPE_DDR_16_MOBILE;//RAM_TYPE_DDR_16_MOBILE
	}
	if(((CButton *)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->GetCheck())
	{
		config.ram_param.type = RAM_TYPE_DDR_32_MOBILE;//RAM_TYPE_DDR_32_MOBILE
	}

	//ram param
	GetDlgItemText(IDC_RAM_SIZE_LIST, str);
    config.ram_param.size = atoi(T2A(str));//ram_param.size

	GetDlgItemText(IDC_RAM_ROW_LIST, str);
    config.ram_param.row = atoi(T2A(str));//ram_param.row

	GetDlgItemText(IDC_RAM_COLUMN_LIST, str);
    config.ram_param.column = atoi(T2A(str));//column

	GetDlgItemText(IDC_RAM_BANK_LIST, str);
    config.ram_param.banks = atoi(T2A(str));//banks

    GetDlgItemText(IDC_COMBO_CHIP_SEL, str);
    config.chip_select_ctrl.chip_sel_num = atoi(T2A(str));//chip_sel_num
    if (config.chip_select_ctrl.chip_sel_num > 2)
    {
        GetDlgItemText(IDC_EDIT_GPIO_CE2, str);
        if (!str.IsEmpty())
        {
            config.chip_select_ctrl.gpio_chip_2 = atoi(T2A(str));//gpio_chip_2
        }
        else
	    {
		    config.chip_select_ctrl.gpio_chip_2 = INVALID_GPIO;//gpio_chip_2
	    }

        GetDlgItemText(IDC_EDIT_GPIO_CE3, str);
        if (!str.IsEmpty())
        {
            config.chip_select_ctrl.gpio_chip_3 = atoi(T2A(str));//gpio_chip_3
        }
        else
	    {
		    config.chip_select_ctrl.gpio_chip_3 = INVALID_GPIO;//gpio_chip_3
	    }
    }
	
    //select gpio pin
	check = ((CButton *)GetDlgItem(IDC_CHECK_GPIO_PIN_SELECT))->GetCheck();
	config.gpio_pin_select = check ? TRUE : FALSE;

	GetDlgItemText(IDC_EDIT_GPIO_PIN, str);
	if(!str.IsEmpty() && config.gpio_pin_select)//gpio_pin_select
	{
		config.gpio_pin = atoi(T2A(str));//gpio_pin
	}
	else
	{
		config.gpio_pin = INVALID_GPIO;//INVALID_GPIO
	}
	//burn mode

	UpdateData();


	config.m_freq = m_feq;//m_feq

	config.burn_mode = m_radio_burn_mode;//m_radio_burn_mode
	config.bUpdate = ((CButton *)GetDlgItem(IDC_CHECK_UPDATE))->GetCheck();//bUpdate
    //#ifdef SUPPORT_LINUX
	//if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	//{
		config.bUDiskUpdate  = ((CButton *)GetDlgItem(IDC_CHECK_UDISK_UPDATE))->GetCheck();//bUDiskUpdate
	//}
    //#endif

	config.bUpdateself  = ((CButton *)GetDlgItem(IDC_UPDATESELF))->GetCheck();//bUpdateself

	config.bTest_RAM = ((CButton *)GetDlgItem(IDC_TEST_RAM))->GetCheck();//bUpdateself

    //***********************************************
    // get burned param
    
    check = ((CButton *)GetDlgItem(IDC_BURNED_GPIO))->GetCheck();
    if (check)
    {
        GetDlgItemText(IDC_GPIO_PIN_NUM,str);
        if (!str.IsEmpty())
        {
            theConfig.burned_param.pwr_gpio_param.num = atoi(T2A(str));//num
        }
        else
        {
            // fail massage
        }

        check = ((CButton *)GetDlgItem(IDC_BURNED_LVH))->GetCheck();
        if (check)
        {
            theConfig.burned_param.pwr_gpio_param.level = 1;//level = 0
        }
        else 
        {
            theConfig.burned_param.pwr_gpio_param.level = 0;//0
        }
        
        check = ((CButton *)GetDlgItem(IDC_BURNED_PULLUP))->GetCheck();
        if (check)
        {
            check = ((CButton *)GetDlgItem(IDC_GPIO_PULLUP_EN))->GetCheck();
            if (check)
            {
                theConfig.burned_param.pwr_gpio_param.Pullup = 1;
            }
            else
            {
                theConfig.burned_param.pwr_gpio_param.Pullup = 0;
            }
            theConfig.burned_param.pwr_gpio_param.Pulldown = 255;
        }
        else
        {
            check = ((CButton *)GetDlgItem(IDC_GPIO_PULLDOWN_EN))->GetCheck();
            if (check)
            {
                theConfig.burned_param.pwr_gpio_param.Pulldown = 1;
            }
            else
            {
                theConfig.burned_param.pwr_gpio_param.Pulldown = 0;
            }
            theConfig.burned_param.pwr_gpio_param.Pullup = 255;
            
        }
    }

    check = ((CButton *)GetDlgItem(IDC_BURNED_RTC_LVH))->GetCheck();
    if (check)
    {
        theConfig.burned_param.rtc_wakup_level = 1;//rtc_wakup_level
    }
    else 
    {
        theConfig.burned_param.rtc_wakup_level = 0;
    }

	return TRUE;
}

BOOL CPageHardware::set_config_item(CConfig &config)
{
	CString str;
    BOOL check = FALSE;

	USES_CONVERSION;

	config.init_chip_type = config.chip_type;
	//set the chip type
	switch(config.chip_type)
	{
	case CHIP_780X:
		SetDlgItemText(IDC_CHIP_TYPE_LIST, __T("AK_780X"));//CHIP_780X
		break;

	case CHIP_880X:
		SetDlgItemText(IDC_CHIP_TYPE_LIST, __T("AK_880X"));//CHIP_880X
		break;

	case CHIP_980X:
		SetDlgItemText(IDC_CHIP_TYPE_LIST, __T("AK_980X"));//CHIP_980X
		break;

	case CHIP_37XX:
	case CHIP_37XX_L:
		SetDlgItemText(IDC_CHIP_TYPE_LIST, __T("AK_37XX"));//CHIP_37XX
		break;

	case CHIP_39XX:
		SetDlgItemText(IDC_CHIP_TYPE_LIST, __T("AK_39XX"));//CHIP_39XX
		break;

	case CHIP_11XX:
		SetDlgItemText(IDC_CHIP_TYPE_LIST, __T("AK_11XX"));//CHIP_11XX
		break;
	case CHIP_10X6:
	case CHIP_1080A:
	case CHIP_1080L:
	case CHIP_10XXC:
		SetDlgItemText(IDC_CHIP_TYPE_LIST, __T("AK_10XX"));//CHIP_10X6
		break;	
	default:
		SetDlgItemText(IDC_CHIP_TYPE_LIST, __T(""));
		break;
	}

	config.init_bUsb2 = config.bUsb2;
	//set the usb mode
	if(config.bUsb2)
	{
		((CButton *)GetDlgItem(IDC_CHECK_USB2))->SetCheck(BST_CHECKED);
	}

	config.init_bPLL_Frep_change = config.bPLL_Frep_change;
	if(config.bPLL_Frep_change)
	{
		((CButton *)GetDlgItem(IDC_CHECK_PLLFREP))->SetCheck(BST_CHECKED);
	}

	config.init_ram_param.type = config.ram_param.type;//init_ram_param.type
	if (RAM_TYPE_MCP == config.ram_param.type)//RAM_TYPE_MCP
	{
		((CButton*)GetDlgItem(IDC_RADIO_MCP_MODE))->SetCheck(1);//IDC_RADIO_MCP_MODE
		((CButton*)GetDlgItem(IDC_RADIO_SDRAM_MODE))->SetCheck(0);//IDC_RADIO_SDRAM_MODE
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE))->SetCheck(0);//IDC_RADIO_DDR_MODE
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE32))->SetCheck(0);//IDC_RADIO_DDR_MODE32
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->SetCheck(0);//IDC_RADIO_DDR2_16_MODE
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->SetCheck(0);
		GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(FALSE);
		GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
	}
	else if(RAM_TYPE_SDR == config.ram_param.type)//RAM_TYPE_SDR
	{
		((CButton*)GetDlgItem(IDC_RADIO_MCP_MODE))->SetCheck(0);//
		((CButton*)GetDlgItem(IDC_RADIO_SDRAM_MODE))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE32))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->SetCheck(0);
		GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(FALSE);//可用
		GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(FALSE);//可用
		GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
	}
	else if(RAM_TYPE_DDR16 == config.ram_param.type)
	{
		((CButton*)GetDlgItem(IDC_RADIO_MCP_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_SDRAM_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE32))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->SetCheck(0);
		GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);
		if (config.chip_type == CHIP_980X || config.chip_type == CHIP_39XX)//CHIP_980X
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);//可用
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);//可见
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);//可见
		}
		else
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);//不可用
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);//不可用
		}
	}
	else if(RAM_TYPE_DDR32 == config.ram_param.type)//RAM_TYPE_DDR32
	{
		((CButton*)GetDlgItem(IDC_RADIO_MCP_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_SDRAM_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE32))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->SetCheck(0);
		GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);
		if (config.chip_type == CHIP_980X || config.chip_type == CHIP_39XX)//CHIP_980X
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
		}
	}
	else if(RAM_TYPE_DDR2_16 == config.ram_param.type)//RAM_TYPE_DDR2_16
	{
		((CButton*)GetDlgItem(IDC_RADIO_MCP_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_SDRAM_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE32))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->SetCheck(0);
		GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);
		if (config.chip_type == CHIP_980X || config.chip_type == CHIP_39XX)//CHIP_980X
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
		}
	}
	else if(RAM_TYPE_DDR2_32 == config.ram_param.type)//RAM_TYPE_DDR2_32
	{
		((CButton*)GetDlgItem(IDC_RADIO_MCP_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_SDRAM_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE32))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->SetCheck(0);
		GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);
		if (config.chip_type == CHIP_980X || config.chip_type == CHIP_39XX)
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);

			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
		}
	}
	else if(RAM_TYPE_DDR_16_MOBILE == config.ram_param.type)//RAM_TYPE_DDR_16_MOBILE
	{
		((CButton*)GetDlgItem(IDC_RADIO_MCP_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_SDRAM_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE32))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->SetCheck(1);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->SetCheck(0);
		GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);
		if (config.chip_type == CHIP_980X || config.chip_type == CHIP_39XX)
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);

			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
		}
	}
	else if(RAM_TYPE_DDR_32_MOBILE == config.ram_param.type)//RAM_TYPE_DDR_32_MOBILE
	{
		((CButton*)GetDlgItem(IDC_RADIO_MCP_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_SDRAM_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_MODE32))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->SetCheck(0);
		((CButton*)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->SetCheck(1);
		GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
		GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
		GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);
		if (config.chip_type == CHIP_980X || config.chip_type == CHIP_39XX)//CHIP_980X
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);

			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
		}
	}

    str.Format(_T("%d"), config.ram_param.size);//size
    SetDlgItemText(IDC_RAM_SIZE_LIST, str);

    // chip control
    str.Format(_T("%d"), config.chip_select_ctrl.chip_sel_num);//chip_sel_num
    SetDlgItemText(IDC_COMBO_CHIP_SEL, str);
    str.Format(_T("%d"), config.chip_select_ctrl.gpio_chip_2);//gpio_chip_2
    SetDlgItemText(IDC_EDIT_GPIO_CE2, str);
    str.Format(_T("%d"), config.chip_select_ctrl.gpio_chip_3);//gpio_chip_3
    SetDlgItemText(IDC_EDIT_GPIO_CE3, str);
    if (config.chip_select_ctrl.chip_sel_num > 2)//chip_sel_num
    {
        GetDlgItem(IDC_EDIT_GPIO_CE2)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_GPIO_CE3)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_EDIT_GPIO_CE2)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_GPIO_CE3)->EnableWindow(FALSE);
    }
    
	((CButton *)GetDlgItem(IDC_CHECK_GPIO_PIN_SELECT))->SetCheck(config.gpio_pin_select);//gpio_pin_select
	if (config.gpio_pin_select)
    {
        GetDlgItem(IDC_EDIT_GPIO_PIN)->EnableWindow(TRUE);//可用
    }
    else
    {
        GetDlgItem(IDC_EDIT_GPIO_PIN)->EnableWindow(FALSE);//不可用
    }
    //set burn mode

	str.Format(_T("%d"), config.ram_param.size);//size
    SetDlgItemText(IDC_RAM_SIZE_LIST, str);

    str.Format(_T("%d"), config.ram_param.row);//row
    SetDlgItemText(IDC_RAM_ROW_LIST, str);

    str.Format(_T("%d"), config.ram_param.column);//column
    SetDlgItemText(IDC_RAM_COLUMN_LIST, str);

    str.Format(_T("%d"), config.ram_param.banks);//banks
    SetDlgItemText(IDC_RAM_BANK_LIST, str);

	m_feq = config.m_freq;

	str.Format(_T("%d"), config.gpio_pin);//gpio_pin
    SetDlgItemText(IDC_EDIT_GPIO_PIN, str);
    
    // set Burned param ***BEGIN
    str.Format(_T("%d"), theConfig.burned_param.pwr_gpio_param.num);//num
    SetDlgItemText(IDC_GPIO_PIN_NUM, str);

    ((CButton*)GetDlgItem(IDC_BURNED_GPIO))->SetCheck(theConfig.burned_param.bGpio);//bGpio
    if (theConfig.burned_param.pwr_gpio_param.level == 1)
    {
        ((CButton*)GetDlgItem(IDC_BURNED_LVH))->SetCheck(1);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_BURNED_LVL))->SetCheck(1);
    }

    if (theConfig.burned_param.pwr_gpio_param.Pullup == 1)//Pullup
    {
        ((CButton*)GetDlgItem(IDC_BURNED_PULLUP))->SetCheck(1);
        ((CButton*)GetDlgItem(IDC_GPIO_PULLUP_EN))->SetCheck(1);
    }
    else if (theConfig.burned_param.pwr_gpio_param.Pullup == 0)//Pullup
    {
        ((CButton*)GetDlgItem(IDC_BURNED_PULLUP))->SetCheck(1);
        ((CButton*)GetDlgItem(IDC_GPIO_PULLUP_EN))->SetCheck(0);
    }
    else if (theConfig.burned_param.pwr_gpio_param.Pulldown == 1)//Pulldown
    {
        ((CButton*)GetDlgItem(IDC_BURNED_PULLDOWN))->SetCheck(1);
        ((CButton*)GetDlgItem(IDC_GPIO_PULLDOWN_EN))->SetCheck(1);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_BURNED_PULLDOWN))->SetCheck(1);
        ((CButton*)GetDlgItem(IDC_GPIO_PULLDOWN_EN))->SetCheck(0);
    }

	theConfig.init_burned_param.bWakup = theConfig.burned_param.bWakup;//bWakup
    ((CButton*)GetDlgItem(IDC_BURNED_RTC_WAKUP))->SetCheck(theConfig.burned_param.bWakup);
    if (theConfig.burned_param.rtc_wakup_level == 1)
    {
        ((CButton*)GetDlgItem(IDC_BURNED_RTC_LVH))->SetCheck(1);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_BURNED_RTC_LVL))->SetCheck(1);
    }

	theConfig.init_burned_param.burned_mode= theConfig.burned_param.burned_mode;//burned_mode
    if (BURNED_NONE == theConfig.burned_param.burned_mode)//BURNED_NONE
    {
        ((CButton*)GetDlgItem(IDC_BURNED_NONE))->SetCheck(1);
        burned_box_control(FALSE);
    }
    else if (BURNED_RESET == theConfig.burned_param.burned_mode)//BURNED_RESET
    {
        ((CButton*)GetDlgItem(IDC_BURNED_RESET))->SetCheck(1);//IDC_BURNED_RESET
        burned_box_control(FALSE);
    }
    else
    {
        ((CButton*)GetDlgItem(IDC_BURNED_POWER_OFF))->SetCheck(1);//IDC_BURNED_POWER_OFF
        burned_box_control(TRUE);
    }
    // set Burned param ***END

	m_radio_burn_mode = config.burn_mode;//
	config.init_burn_mode = config.burn_mode;//
	config.init_bUpdate = config.bUpdate;//
	((CButton *)GetDlgItem(IDC_CHECK_UPDATE))->SetCheck(config.bUpdate);//
   // #ifdef SUPPORT_LINUX
	if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	{
		//((CButton *)GetDlgItem(IDC_CHECK_UDISK_UPDATE))->SetCheck(config.bUDiskUpdate);
		((CButton *)GetDlgItem(IDC_CHECK_USB2))->ShowWindow(0);
		((CButton *)GetDlgItem(IDC_CHECK_PLLFREP))->ShowWindow(0);
		
	}
	else //#else
	{
		//((CButton *)GetDlgItem(IDC_CHECK_UDISK_UPDATE))->ShowWindow(0);		
		((CButton *)GetDlgItem(IDC_CHECK_USB2))->SetCheck(config.bUsb2);
		((CButton *)GetDlgItem(IDC_CHECK_PLLFREP))->SetCheck(config.bPLL_Frep_change);;
	}

	config.init_bUpdateself = config.bUpdateself;
	if (config.chip_type == CHIP_1080A || config.chip_type == CHIP_1080L 
		|| config.chip_type == CHIP_10XXC || config.chip_type == CHIP_10X6 
		|| config.chip_type == CHIP_11XX)
	{
		((CButton *)GetDlgItem(IDC_UPDATESELF))->ShowWindow(1);// IDC_UPDATESELF
		config.bTest_RAM = FALSE;
		
	} 
	else
	{
		((CButton *)GetDlgItem(IDC_UPDATESELF))->ShowWindow(0);//IDC_UPDATESELF no
		config.bUpdateself = 0;
	}
	((CButton *)GetDlgItem(IDC_TEST_RAM))->SetCheck(config.bTest_RAM);
	((CButton *)GetDlgItem(IDC_UPDATESELF))->SetCheck(config.bUpdateself);
	config.init_bUDiskUpdate = config.bUDiskUpdate;
	((CButton *)GetDlgItem(IDC_CHECK_UDISK_UPDATE))->SetCheck(config.bUDiskUpdate);
    //#endif
	
	GetDlgItemText(IDC_CHIP_TYPE_LIST, str);

	select_change_chip_type(str);

	UpdateData(false);


	return TRUE;
}

BOOL CPageHardware::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	SetupDisplay();
	
	set_config_item(theConfig);
	initFlag = TRUE;
	SetBlgAttr(theConfig.planform_tpye);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPageHardware::select_boot_chip_type()
{
	CDlgConfig* p = (CDlgConfig*)GetParent()->GetParent();
	CPageGeneral *theAppGenral = &p->m_page_general;
	
	//通过在CPageHardware类的控制CPageGeneral一些控制
	theAppGenral->set_nandboot_new_37L();
}

void CPageHardware::OnSelchangeChipTypeList() 
{
	// TODO: Add your control notification handler code here
	int sel;
	CString str;
	
	sel = m_chip_type_list.GetCurSel();
	m_chip_type_list.GetLBText(sel, str);

	if (str == _T("AK_980X"))//CHIP_980X
	{
		theConfig.chip_type = CHIP_980X;
	}
	else if(str == _T("AK_880X"))//CHIP_880X
	{
		theConfig.chip_type = CHIP_880X;
	}
	else if (str == _T("AK_37XX"))//CHIP_37XX
	{
		theConfig.chip_type = CHIP_37XX;
	}
	else if (str == _T("AK_780X"))//CHIP_780X
	{
		theConfig.chip_type = CHIP_780X;
	}
	else if (str == _T("AK_10XX"))//CHIP_10X6
	{
		theConfig.chip_type = CHIP_10X6;
	}
	/*
	else if (str == _T("AK_1080A"))
	{
		theConfig.chip_type = CHIP_1080A;
	}
	*/
	else if (str == _T("AK_11XX"))//CHIP_11XX
	{
		theConfig.chip_type = CHIP_11XX;
	}
	else if (str == _T("AK_39XX"))//CHIP_39XX
	{
		theConfig.chip_type = CHIP_39XX;
	}

	select_change_chip_type(str);
	select_boot_chip_type();
}

void CPageHardware::select_change_chip_type(CString strChipType)
{
	
#if 1	
	if ((strChipType == _T("AK_11XX")) || (strChipType == _T("AK_10XX")))
	{
		for (int i = 0; i < 31; i++)
		{
			GetDlgItem(h_ctl_array[i])->ShowWindow(FALSE);//不可见
		}

		CWnd *pWnd;
		CRect rect;
		CWnd *pWnd_Flag;
		CRect rect_Flag;

		pWnd_Flag = GetDlgItem(IDC_CHIP_TYPE_LIST); //选取一个控件作为标识
		pWnd_Flag->GetWindowRect(&rect_Flag);
		pWnd = GetDlgItem(IDC_STATIC_BURN_MODE); //获取编辑控件指针
		pWnd->GetWindowRect(&rect);

		if ((rect_Flag.top + 59) < rect.top)
		{
			for(int j = 31; j < 57; j++)
			{
				pWnd = GetDlgItem(h_ctl_array[j]); //获取编辑控件指针
				pWnd->GetWindowRect(&rect);
				ScreenToClient(&rect);
				pWnd->SetWindowPos( NULL,rect.left,rect.top - 232,0,0,SWP_NOZORDER | SWP_NOSIZE ); //移动位置
			}
		}
		//当是10或11芯片时，自升级可用
		GetDlgItem(h_ctl_array[39])->ShowWindow(TRUE);
		GetDlgItem(h_ctl_array[38])->ShowWindow(FALSE);
		theConfig.bTest_RAM = 0;
	}
	else
	{
		CWnd *pWnd;
		CRect rect;
		CWnd *pWnd_Flag;
		CRect rect_Flag;

		pWnd_Flag = GetDlgItem(IDC_CHIP_TYPE_LIST); //选取一个控件作为标识
		pWnd_Flag->GetWindowRect(&rect_Flag);
		pWnd = GetDlgItem(IDC_STATIC_BURN_MODE); //获取编辑控件指针
		pWnd->GetWindowRect(&rect);
		//当是10或11芯片时，自升级不可用
		GetDlgItem(h_ctl_array[39])->ShowWindow(FALSE);
		theConfig.bUpdateself = 0;
		GetDlgItem(h_ctl_array[38])->ShowWindow(TRUE);
		

		if ((rect_Flag.top + 59) > rect.top)
		{
			for(int j = 31; j < 57; j++)
			{
				pWnd = GetDlgItem(h_ctl_array[j]); //获取编辑控件指针
				pWnd->GetWindowRect(&rect);
				ScreenToClient(&rect);
				pWnd->SetWindowPos( NULL,rect.left,rect.top + 232,0,0,SWP_NOZORDER | SWP_NOSIZE ); //移动位置
			}
		}

		for (int i = 0; i < 31; i++)
		{
			GetDlgItem(h_ctl_array[i])->ShowWindow(TRUE);
		}

	}
#endif
	if ((RAM_TYPE_MCP == theConfig.ram_param.type) || (RAM_TYPE_SDR== theConfig.ram_param.type))
	{
		GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(FALSE);//IDC_EDIT_FREQ
		GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(FALSE);//IDC_STATIC_DDR_Freq
		GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(FALSE);//IDC_STATIC_Mhz
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);//IDC_BUTTON_IMPORT_RAM_CONFIG
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);//IDC_BUTTON_EXPORT_RAM_CONFIG
	}
	else
	{
		if (strChipType == _T("AK_980X") || strChipType == _T("AK_39XX")) 
		{					
			GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);//FREQ
			GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);//_DDR_Freq
			GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);//IDC_STATIC_Mhz
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);//RAM_CONFIG
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);	//RAM_CONFIG		
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);//RAM_CONFIG
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);//RAM_CONFIG
		}
		else if (strChipType == _T("AK_37XX")) 
		{					
			GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
			GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
			GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);
			GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);//IDC_BUTTON_IMPORT_RAM_CONFIG
			GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);//IDC_BUTTON_EXPORT_RAM_CONFIG
		}
	}

}


void CPageHardware::SetupDisplay()
{
	CString str;

	str = theApp.GetString(IDS_HARDWARE_CHIP_TYPE);
	GetDlgItem(IDC_STATIC_CHIP_TYPE)->SetWindowText(str);//芯片类型

	str = theApp.GetString(IDS_HARDWARE_USB2);
	GetDlgItem(IDC_CHECK_USB2)->SetWindowText(str);//usb2.0 

	str = theApp.GetString(IDS_HARDWARE_PLLFREP);
	GetDlgItem(IDC_CHECK_PLLFREP)->SetWindowText(str);//change pll frep

	str = theApp.GetString(IDS_HARDWARE_RAM_SIZE);
	GetDlgItem(IDC_STATIC_RAM_SIZE)->SetWindowText(str);//ram size 
	
	str = theApp.GetString(IDS_HARDWARE_NAND_CHIP_SEL);
	GetDlgItem(IDC_STATIC_CHIP_SELECT)->SetWindowText(str);//芯片选择
	
	str = theApp.GetString(IDS_STATIC_SD_RAM);
	GetDlgItem(IDC_STATIC_SD_RAM)->SetWindowText(str);//sdram

	str = theApp.GetString(IDS_GROUP_GPIO);
	GetDlgItem(IDC_GROUP_GPIO)->SetWindowText(str);//gpio

	str = theApp.GetString(IDS_STATIC_BURN_MODE);
	GetDlgItem(IDC_STATIC_BURN_MODE)->SetWindowText(str);//烧录模式

	str = theApp.GetString(IDS_STATIC_BURNED);
	GetDlgItem(IDC_STATIC_BURNED)->SetWindowText(str);//

	str = theApp.GetString(IDS_BUTTON_IMPORT_RAM_CONFIG);
	GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->SetWindowText(str);//IDS_BUTTON_IMPORT_RAM_CONFIG

	str = theApp.GetString(IDS_BUTTON_EXPORT_RAM_CONFIG);
	GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->SetWindowText(str);//IDS_BUTTON_EXPORT_RAM_CONFIG

	str = theApp.GetString(IDS_STATIC_RAM_ROW);
	GetDlgItem(IDC_STATIC_RAM_ROW)->SetWindowText(str);//IDS_STATIC_RAM_ROW

	str = theApp.GetString(IDS_STATIC_RAM_COLUMN);
	GetDlgItem(IDC_STATIC_RAM_COLUMN)->SetWindowText(str);//IDS_STATIC_RAM_COLUMN
/*
    if (theConfig.burned_param.burned_mode == BURNED_POWER_OFF)
    {
        burned_box_control(TRUE);
    }
    else
    {
        burned_box_control(FALSE);
    }*/
}

void CPageHardware::OnRadioMcpMode() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(FALSE);
	GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
	GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
	theConfig.ram_param.type = RAM_TYPE_MCP;
}

void CPageHardware::OnRadioSdramMode() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(FALSE);
	GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(FALSE);
	GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
	GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
	theConfig.ram_param.type = RAM_TYPE_SDR;
}

void CPageHardware::OnRadioDdrMode() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
	GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
	GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);
	//GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
	//GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
	if (theConfig.chip_type == CHIP_980X || theConfig.chip_type == CHIP_39XX)
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
	}
	theConfig.ram_param.type = RAM_TYPE_DDR16;
}

void CPageHardware::OnRadioDdr216Mode() 
{


	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
	GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
	GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);

	if (theConfig.chip_type == CHIP_980X || theConfig.chip_type == CHIP_39XX)
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
	}
	theConfig.ram_param.type = RAM_TYPE_DDR2_16;
	
}

void CPageHardware::OnRadioDdr232Mode() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
	GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
	GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);

	if (theConfig.chip_type == CHIP_980X || theConfig.chip_type == CHIP_39XX)
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
	}
	theConfig.ram_param.type = RAM_TYPE_DDR2_32;
}

void CPageHardware::OnRadioDdrMode32() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
	GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
	GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);
	//GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);
	//GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);
	if (theConfig.chip_type == CHIP_980X || theConfig.chip_type == CHIP_39XX)
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);//
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);//
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);//
	}
	else
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);//
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);//
	}
	theConfig.ram_param.type = RAM_TYPE_DDR32;
	
}



static int ram_type_radio_id[] = 
{
	IDC_RADIO_DDR2_32_MODE, IDC_RADIO_DDR_MODE32,IDC_RADIO_MCP_MODE,
	IDC_RADIO_SDRAM_MODE, IDC_RADIO_DDR2_16_MODE,IDC_RADIO_DDR_MODE,
	IDC_RADIO_DDR_16_MOBILE,IDC_RADIO_DDR_32_MOBILE
};


int  CPageHardware::GetComboBoxIndex(int nIDDlgItem)
{
	int				nIndex;
	CComboBox *		pComboBox;
	TCHAR			buf1[64],buf2[64];


	pComboBox = (CComboBox *)GetDlgItem(nIDDlgItem);//
	if(NULL == pComboBox)
	{
		MessageBox(_T("GetComboBoxIndex 1, error"));
		return 0;
	}

	if(0 == GetDlgItemText(nIDDlgItem, buf1,64))
	{
		MessageBox(_T("GetComboBoxIndex 2, error"));
		return 0;
	}

	for(nIndex = 0; nIndex < pComboBox->GetCount();nIndex++)
	{
		pComboBox->GetLBText(nIndex,buf2);   //be careful,not check string len;
		if(!wcsncmp(buf1,buf2,64))
			break;
	}

	return nIndex;
}


void CPageHardware::OnButtonExportRamConfig() 
{
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn;       
	TCHAR szFileName[MAX_PATH] = {0};   
    UINT  ui_data;
	BYTE ram_type;
	CString strTmp;
	UINT  datatmp;
	
    ram_type = theConfig.ram_param.type;

	//假如没有导入ram，查看是否有匹配的ram配置文件并导入
	if (!ramImportFlag)
	{
		if((RAM_TYPE_DDR16 == ram_type) || (RAM_TYPE_DDR32 == ram_type))//RAM_TYPE_DDR16 and RAM_TYPE_DDR32
		{
			strTmp.Format(_T(RAM_CFG_DDR));
		}
		else if ((RAM_TYPE_DDR2_16 == ram_type) || (RAM_TYPE_DDR2_32 == ram_type))//RAM_TYPE_DDR2_16 and RAM_TYPE_DDR2_32
		{
			strTmp.Format(_T(RAM_CFG_DDR2));
		}
		else if ((RAM_TYPE_DDR_16_MOBILE == ram_type) || (RAM_TYPE_DDR_32_MOBILE == ram_type))//RAM_TYPE_DDR_16_MOBILE and RAM_TYPE_DDR_32_MOBILE
		{
			strTmp.Format(_T(RAM_CFG_mDDR));
		}
		else if (RAM_TYPE_MCP == ram_type)//RAM_TYPE_MCP
		{
			strTmp.Format(_T(RAM_CFG_MCP));
		}
		else if (RAM_TYPE_SDR == ram_type)//RAM_TYPE_SDR
		{
			strTmp.Format(_T(RAM_CFG_SDR));
		}

		if(import_ram_config(theApp.ConvertAbsolutePath(strTmp)))
		{
			if(get_UI_data(& datatmp))   //just for 98
			{
				int i;
				for(i = 0;i < sizeof(ram_type_radio_id)/sizeof(ram_type_radio_id[0]); ++i)
				{
					((CButton *)GetDlgItem(ram_type_radio_id[i]))->SetCheck(BST_UNCHECKED);
				}
				
				((CButton *)GetDlgItem(ram_type_radio_id[datatmp & 0x07]))->SetCheck(BST_CHECKED);
				if(((CButton *)GetDlgItem(IDC_RADIO_MCP_MODE))->GetCheck())
				{
					theConfig.ram_param.type = RAM_TYPE_MCP;//RAM_TYPE_MCP
				}
				if(((CButton *)GetDlgItem(IDC_RADIO_SDRAM_MODE))->GetCheck())
				{
					theConfig.ram_param.type = RAM_TYPE_SDR;//RAM_TYPE_SDR
				}
				if(((CButton *)GetDlgItem(IDC_RADIO_DDR_MODE))->GetCheck())
				{
					theConfig.ram_param.type = RAM_TYPE_DDR16;//RAM_TYPE_DDR16
				}
				if(((CButton *)GetDlgItem(IDC_RADIO_DDR_MODE32))->GetCheck())
				{
					theConfig.ram_param.type = RAM_TYPE_DDR32;//RAM_TYPE_DDR32
				}
				if(((CButton *)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->GetCheck())
				{
					theConfig.ram_param.type = RAM_TYPE_DDR2_16;//RAM_TYPE_DDR2_16
				}
				if(((CButton *)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->GetCheck())
				{
					theConfig.ram_param.type = RAM_TYPE_DDR2_32;//RAM_TYPE_DDR2_32
				}
				if(((CButton *)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->GetCheck())
				{
					theConfig.ram_param.type = RAM_TYPE_DDR_16_MOBILE;//RAM_TYPE_DDR_16_MOBILE
				}
				if(((CButton *)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->GetCheck())
				{
					theConfig.ram_param.type = RAM_TYPE_DDR_32_MOBILE;//RAM_TYPE_DDR_32_MOBILE
				}
				GetDlgItem(IDC_EDIT_FREQ)->EnableWindow((datatmp&0x03) != 0x03);

				((CComboBox *)GetDlgItem(IDC_RAM_BANK_LIST))->SetCurSel((datatmp>>9)&0x01);
				((CComboBox *)GetDlgItem(IDC_RAM_ROW_LIST))->SetCurSel((datatmp>>13)&0x07);
				((CComboBox *)GetDlgItem(IDC_RAM_COLUMN_LIST))->SetCurSel((datatmp>>10)&0x07);
			}
		}
	}

	//使导出的文件名默认和匹配文件名一致

	if ((RAM_TYPE_DDR16 == ram_type) || (RAM_TYPE_DDR32 == ram_type))
	{
		_tcscpy(szFileName, _T("ramconfig_ddr.txt"));//RAM_TYPE_DDR16 and RAM_TYPE_DDR32
	}
	else if ((RAM_TYPE_DDR2_16 == ram_type) || (RAM_TYPE_DDR2_32 == ram_type))
	{
		_tcscpy(szFileName, _T("ramconfig_ddr2.txt"));//RAM_TYPE_DDR2_16 and RAM_TYPE_DDR2_32
	}
	else if ((RAM_TYPE_DDR_16_MOBILE == ram_type) || (RAM_TYPE_DDR_32_MOBILE == ram_type))
	{
		_tcscpy(szFileName, _T("ramconfig_m_ddr.txt"));//RAM_TYPE_DDR_16_MOBILE and RAM_TYPE_DDR_32_MOBILE
	}
	else if (RAM_TYPE_MCP == ram_type)
	{
		_tcscpy(szFileName, _T("ramconfig_mcp.txt"));//RAM_TYPE_MCP
	}
	else if (RAM_TYPE_SDR == ram_type)
	{
		_tcscpy(szFileName, _T("ramconfig_sdram.txt"));//RAM_TYPE_SDR
	}

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = _hInstance;
	ofn.hwndOwner = GetSafeHwnd();
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.lpstrFilter = _T("Text\0*.TXT\0All\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	if (GetSaveFileName(&ofn)== FALSE) //获取文件名
	{
		MessageBox(_T("Nothing Export"));
		return;
	}

	for(ui_data=0; \
		ui_data < sizeof(ram_type_radio_id)/sizeof(ram_type_radio_id[0]);	\
		++ui_data)
	{
		if(((CButton *)GetDlgItem(ram_type_radio_id[ui_data]))->GetCheck() == 
			BST_CHECKED)
		{
			break;
		}
	}


	ui_data |= GetComboBoxIndex(IDC_RAM_BANK_LIST) << 9;
	ui_data |= GetComboBoxIndex(IDC_RAM_ROW_LIST) << 13;
	ui_data |= GetComboBoxIndex(IDC_RAM_COLUMN_LIST) <<10;


	if(!export_ram_config(szFileName, ui_data))
	{
		MessageBox(_T("Export file fail, maybe no enough space!"));
		return ;
	}	

}

void CPageHardware::OnButtonImportRamConfig() 
{
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn;       
	TCHAR szFileName[MAX_PATH] = {0};   
	UINT  datatmp;

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = _hInstance;
	ofn.hwndOwner = GetSafeHwnd();
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = sizeof(szFileName);
	ofn.lpstrFilter = _T("Text\0*.TXT\0All\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 

	if (GetOpenFileName(&ofn)== FALSE) 
	{
		MessageBox(_T("Nothing import"));
		return;
	}

//	MessageBox(szFileName);

//	return;

	if(!import_ram_config(szFileName))
	{
		MessageBox(_T("Import file fail,the file is not valid!"));
		return ;
	}
	
	if(get_UI_data(& datatmp))   //just for 98
	{
		int i;
		for(i = 0;i < sizeof(ram_type_radio_id)/sizeof(ram_type_radio_id[0]); ++i)
		{
			((CButton *)GetDlgItem(ram_type_radio_id[i]))->SetCheck(BST_UNCHECKED);
		}
		
		((CButton *)GetDlgItem(ram_type_radio_id[datatmp & 0x07]))->SetCheck(BST_CHECKED);
		if(((CButton *)GetDlgItem(IDC_RADIO_MCP_MODE))->GetCheck())
		{
			theConfig.ram_param.type = RAM_TYPE_MCP;//RAM_TYPE_MCP
		}
		if(((CButton *)GetDlgItem(IDC_RADIO_SDRAM_MODE))->GetCheck())
		{
			theConfig.ram_param.type = RAM_TYPE_SDR;//RAM_TYPE_SDR
		}
		if(((CButton *)GetDlgItem(IDC_RADIO_DDR_MODE))->GetCheck())
		{
			theConfig.ram_param.type = RAM_TYPE_DDR16;//RAM_TYPE_DDR16
		}
		if(((CButton *)GetDlgItem(IDC_RADIO_DDR_MODE32))->GetCheck())
		{
			theConfig.ram_param.type = RAM_TYPE_DDR32;//RAM_TYPE_DDR32
		}
		if(((CButton *)GetDlgItem(IDC_RADIO_DDR2_16_MODE))->GetCheck())
		{
			theConfig.ram_param.type = RAM_TYPE_DDR2_16;//RAM_TYPE_DDR2_16
		}
		if(((CButton *)GetDlgItem(IDC_RADIO_DDR2_32_MODE))->GetCheck())
		{
			theConfig.ram_param.type = RAM_TYPE_DDR2_32;//RAM_TYPE_DDR2_32
		}
		if(((CButton *)GetDlgItem(IDC_RADIO_DDR_16_MOBILE))->GetCheck())
		{
			theConfig.ram_param.type = RAM_TYPE_DDR_16_MOBILE;//RAM_TYPE_DDR_16_MOBILE
		}
		if(((CButton *)GetDlgItem(IDC_RADIO_DDR_32_MOBILE))->GetCheck())
		{
			theConfig.ram_param.type = RAM_TYPE_DDR_32_MOBILE;//RAM_TYPE_DDR_32_MOBILE
		}
		GetDlgItem(IDC_EDIT_FREQ)->EnableWindow((datatmp&0x03) != 0x03);

		((CComboBox *)GetDlgItem(IDC_RAM_BANK_LIST))->SetCurSel((datatmp>>9)&0x01);
		((CComboBox *)GetDlgItem(IDC_RAM_ROW_LIST))->SetCurSel((datatmp>>13)&0x07);
		((CComboBox *)GetDlgItem(IDC_RAM_COLUMN_LIST))->SetCurSel((datatmp>>10)&0x07);
	}
	
	GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);
	ramImportFlag = TRUE;
}

void CPageHardware::OnChipSelList()
{
    int sel;
    int chip_num;
	CString str;
	
    USES_CONVERSION;
    
	sel = m_chip_sel_list.GetCurSel();
	m_chip_sel_list.GetLBText(sel, str);
    chip_num = atoi(T2A(str));

    if (chip_num > 2)
    {
        GetDlgItem(IDC_EDIT_GPIO_CE2)->EnableWindow(TRUE);
        GetDlgItem(IDC_EDIT_GPIO_CE3)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_EDIT_GPIO_CE2)->EnableWindow(FALSE);
        GetDlgItem(IDC_EDIT_GPIO_CE3)->EnableWindow(FALSE);//
    }
}

void CPageHardware::OnCheckLedGpio()
{
    BYTE check;

    check = ((CButton *)GetDlgItem(IDC_CHECK_GPIO_PIN_SELECT))->GetCheck();
    if (check)
    {
        GetDlgItem(IDC_EDIT_GPIO_PIN)->EnableWindow(TRUE);//IDC_EDIT_GPIO_PIN可用
    }
    else 
    {
        GetDlgItem(IDC_EDIT_GPIO_PIN)->EnableWindow(FALSE);//IDC_EDIT_GPIO_PIN不可用
    }
}

void CPageHardware::burned_box_control(BOOL bEnable)
{
    BOOL check;

    if (bEnable)
    {
		GetDlgItem(IDC_BURNED_GPIO)->ShowWindow(TRUE);//
        GetDlgItem(IDC_BURNED_GPIO)->EnableWindow(TRUE);//
        check = ((CButton *)GetDlgItem(IDC_BURNED_GPIO))->GetCheck();//
        if (check)
        {
            GetDlgItem(IDC_GPIO_PIN_NUM)->EnableWindow(TRUE);//
            GetDlgItem(IDC_BURNED_LVH)->EnableWindow(TRUE);//
            GetDlgItem(IDC_BURNED_LVL)->EnableWindow(TRUE);//

            GetDlgItem(IDC_BURNED_PULLUP)->EnableWindow(TRUE);//
            GetDlgItem(IDC_BURNED_PULLDOWN)->EnableWindow(TRUE);//
            check = ((CButton *)GetDlgItem(IDC_BURNED_PULLUP))->GetCheck();//
            if (check)
            {
                GetDlgItem(IDC_GPIO_PULLUP_EN)->ShowWindow(TRUE);//
                GetDlgItem(IDC_GPIO_PULLUP_EN)->EnableWindow(TRUE);//
				GetDlgItem(IDC_GPIO_PULLDOWN_EN)->ShowWindow(TRUE);//
                GetDlgItem(IDC_GPIO_PULLDOWN_EN)->EnableWindow(FALSE);//
            }
            else 
            {
				GetDlgItem(IDC_GPIO_PULLUP_EN)->ShowWindow(TRUE);
                GetDlgItem(IDC_GPIO_PULLUP_EN)->EnableWindow(FALSE);
                GetDlgItem(IDC_GPIO_PULLDOWN_EN)->ShowWindow(TRUE);
                GetDlgItem(IDC_GPIO_PULLDOWN_EN)->EnableWindow(TRUE);
            }
        }
        else
        {
            GetDlgItem(IDC_GPIO_PIN_NUM)->EnableWindow(FALSE);
            GetDlgItem(IDC_BURNED_LVH)->EnableWindow(FALSE);
            GetDlgItem(IDC_BURNED_LVL)->EnableWindow(FALSE);

            GetDlgItem(IDC_BURNED_PULLUP)->EnableWindow(FALSE);
            GetDlgItem(IDC_BURNED_PULLDOWN)->EnableWindow(FALSE);
        }
        GetDlgItem(IDC_BURNED_RTC_WAKUP)->ShowWindow(TRUE);
        GetDlgItem(IDC_BURNED_RTC_WAKUP)->EnableWindow(TRUE);
        check = ((CButton *)GetDlgItem(IDC_BURNED_RTC_WAKUP))->GetCheck();
        if (check)
        {
            GetDlgItem(IDC_BURNED_RTC_LVH)->EnableWindow(TRUE);
            GetDlgItem(IDC_BURNED_RTC_LVL)->EnableWindow(TRUE);
        }
        else
        {
            GetDlgItem(IDC_BURNED_RTC_LVH)->EnableWindow(FALSE);
            GetDlgItem(IDC_BURNED_RTC_LVL)->EnableWindow(FALSE);
        }
    }
    else
    {
        GetDlgItem(IDC_BURNED_GPIO)->EnableWindow(FALSE);
        GetDlgItem(IDC_GPIO_PIN_NUM)->EnableWindow(FALSE);
        GetDlgItem(IDC_BURNED_LVH)->EnableWindow(FALSE);
        GetDlgItem(IDC_BURNED_LVL)->EnableWindow(FALSE);
        GetDlgItem(IDC_BURNED_PULLUP)->EnableWindow(FALSE);
        GetDlgItem(IDC_BURNED_PULLDOWN)->EnableWindow(FALSE);
        GetDlgItem(IDC_GPIO_PULLUP_EN)->EnableWindow(FALSE);
        GetDlgItem(IDC_GPIO_PULLDOWN_EN)->EnableWindow(FALSE);//

        GetDlgItem(IDC_BURNED_RTC_WAKUP)->EnableWindow(FALSE);//
        GetDlgItem(IDC_BURNED_RTC_LVH)->EnableWindow(FALSE);//
        GetDlgItem(IDC_BURNED_RTC_LVL)->EnableWindow(FALSE);//
    }
}

void CPageHardware::OnRadioBurnedNone()
{
    theConfig.burned_param.burned_mode = BURNED_NONE;//BURNED_NONE
    burned_box_control(FALSE);
}

void CPageHardware::OnRadioBurnedReset()
{
    theConfig.burned_param.burned_mode = BURNED_RESET;//BURNED_RESET
    burned_box_control(FALSE);
}

void CPageHardware::OnRadioBurnedPoweroff()
{
    theConfig.burned_param.burned_mode = BURNED_POWER_OFF;//BURNED_POWER_OFF
    burned_box_control(TRUE);
}

void CPageHardware::OnCheckGpio()
{
    BOOL check=FALSE;

    check = ((CButton *)GetDlgItem(IDC_BURNED_GPIO))->GetCheck();//
    theConfig.burned_param.bGpio = check;
    if (check)
    {
        GetDlgItem(IDC_GPIO_PIN_NUM)->EnableWindow(TRUE);//
        GetDlgItem(IDC_BURNED_LVH)->EnableWindow(TRUE);//
        GetDlgItem(IDC_BURNED_LVL)->EnableWindow(TRUE);//

        GetDlgItem(IDC_BURNED_PULLUP)->EnableWindow(TRUE);//
        GetDlgItem(IDC_BURNED_PULLDOWN)->EnableWindow(TRUE);//
        GetDlgItem(IDC_GPIO_PULLUP_EN)->EnableWindow(TRUE);//
        GetDlgItem(IDC_GPIO_PULLDOWN_EN)->EnableWindow(TRUE);//

        check = ((CButton *)GetDlgItem(IDC_BURNED_PULLUP))->GetCheck();//
        if (check)
        {
            GetDlgItem(IDC_GPIO_PULLUP_EN)->ShowWindow(TRUE);//
            GetDlgItem(IDC_GPIO_PULLUP_EN)->EnableWindow(TRUE);//
			GetDlgItem(IDC_GPIO_PULLDOWN_EN)->ShowWindow(TRUE);//
            GetDlgItem(IDC_GPIO_PULLDOWN_EN)->EnableWindow(FALSE);//
        }
        else 
        {
			GetDlgItem(IDC_GPIO_PULLUP_EN)->ShowWindow(TRUE);//
            GetDlgItem(IDC_GPIO_PULLUP_EN)->EnableWindow(FALSE);//
            GetDlgItem(IDC_GPIO_PULLDOWN_EN)->ShowWindow(TRUE);//
            GetDlgItem(IDC_GPIO_PULLDOWN_EN)->EnableWindow(TRUE);//
        }
        
    }
    else
    {
        GetDlgItem(IDC_GPIO_PIN_NUM)->EnableWindow(FALSE);//IDC_GPIO_PIN_NUM
        GetDlgItem(IDC_BURNED_LVH)->EnableWindow(FALSE);//IDC_BURNED_LVH
        GetDlgItem(IDC_BURNED_LVL)->EnableWindow(FALSE);//IDC_BURNED_LVL

        GetDlgItem(IDC_BURNED_PULLUP)->EnableWindow(FALSE);//IDC_BURNED_PULLUP
        GetDlgItem(IDC_BURNED_PULLDOWN)->EnableWindow(FALSE);//IDC_BURNED_PULLDOWN
        GetDlgItem(IDC_GPIO_PULLUP_EN)->EnableWindow(FALSE);//IDC_GPIO_PULLUP_EN
        GetDlgItem(IDC_GPIO_PULLDOWN_EN)->EnableWindow(FALSE);//IDC_GPIO_PULLDOWN_EN
    }
}

void CPageHardware::OnRadioBurnPullup()
{
    BOOL check;

    check = ((CButton *)GetDlgItem(IDC_BURNED_PULLUP))->GetCheck();
    if (check)
    {
        theConfig.burned_param.pwr_gpio_param.Pulldown = 1;
        GetDlgItem(IDC_GPIO_PULLUP_EN)->EnableWindow(TRUE);//可用
        GetDlgItem(IDC_GPIO_PULLDOWN_EN)->EnableWindow(FALSE);//不可用
    }
}

void CPageHardware::OnRadioBurnPulldown()
{
    BOOL check;

    check = ((CButton *)GetDlgItem(IDC_BURNED_PULLDOWN))->GetCheck();
    if (check)
    {
        GetDlgItem(IDC_GPIO_PULLDOWN_EN)->EnableWindow(TRUE);//可用
        GetDlgItem(IDC_GPIO_PULLUP_EN)->EnableWindow(FALSE);//不可用
    }
}

void CPageHardware::OnCheckRtcWakup()
{
    BOOL check = FALSE;

    check = ((CButton *)GetDlgItem(IDC_BURNED_RTC_WAKUP))->GetCheck();
    theConfig.burned_param.bWakup = check;
    if (check)
    {
        GetDlgItem(IDC_BURNED_RTC_LVH)->EnableWindow(TRUE);//可用
        GetDlgItem(IDC_BURNED_RTC_LVL)->EnableWindow(TRUE);//可用
    }
    else
    {
        GetDlgItem(IDC_BURNED_RTC_LVH)->EnableWindow(FALSE);//不可用
        GetDlgItem(IDC_BURNED_RTC_LVL)->EnableWindow(FALSE);//不可用
    }
}

void CPageHardware::OnRadioDdr16Mobile() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);//IDC_EDIT_FREQ可见
	GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);//可用
	GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);//可见
	GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);//可用
	if (theConfig.chip_type == CHIP_980X || theConfig.chip_type == CHIP_39XX)
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);//可用
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);//可用
	}
	else
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);//不可用
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);//不可用
	}
	theConfig.ram_param.type = RAM_TYPE_DDR_16_MOBILE;
}

void CPageHardware::OnRadioDdr32Mobile() 
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EDIT_FREQ)->ShowWindow(TRUE);
	GetDlgItem(IDC_EDIT_FREQ)->EnableWindow(TRUE);
	GetDlgItem(IDC_STATIC_DDR_Freq)->ShowWindow(TRUE);
	GetDlgItem(IDC_STATIC_Mhz)->ShowWindow(TRUE);
	if (theConfig.chip_type == CHIP_980X || theConfig.chip_type == CHIP_39XX)
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->EnableWindow(TRUE);//不可用
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->EnableWindow(TRUE);//不可用
	}
	else
	{
		GetDlgItem(IDC_BUTTON_IMPORT_RAM_CONFIG)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BUTTON_EXPORT_RAM_CONFIG)->ShowWindow(FALSE);//不可见
	}
	theConfig.ram_param.type = RAM_TYPE_DDR_32_MOBILE;//RAM_TYPE_DDR_32_MOBILE
}

void CPageHardware::OnCheckUsb2() 
{
	// TODO: Add your control notification handler code here
	BOOL check = theConfig.bUsb2;
	check = ((CButton *)GetDlgItem(IDC_CHECK_USB2))->GetCheck();
	theConfig.bUsb2 = check ? TRUE : FALSE;//usb2.0
}

void CPageHardware::OnCheckUdiskUpdate() 
{
	// TODO: Add your control notification handler code here
	BOOL check = theConfig.bUDiskUpdate;
	check = ((CButton *)GetDlgItem(IDC_CHECK_UDISK_UPDATE))->GetCheck();
	theConfig.bUDiskUpdate = check ? TRUE : FALSE;
}

void CPageHardware::OnCheckUpdate() 
{
	CDlgConfig* p = (CDlgConfig*)GetParent()->GetParent();
	CPageGeneral *theAppGenral = &p->m_page_general;
	
	// TODO: Add your control notification handler code here
    theConfig.bUpdate = ((CButton *)GetDlgItem(IDC_CHECK_UPDATE))->GetCheck();

	//通过在CPageHardware类的控制CPageGeneral一些控制
    theAppGenral->set_mac_serial_data();
}

void CPageHardware::OnRadioSflash() 
{
	BOOL check = FALSE;

	CDlgConfig* p = (CDlgConfig*)GetParent()->GetParent();
	CPageGeneral *theAppGenral = &p->m_page_general;

	// TODO: Add your control notification handler code here
	check = ((CButton *)GetDlgItem(IDC_RADIO_SFLASH))->GetCheck();
	if (check)
	{
		theConfig.burn_mode = E_CONFIG_SFLASH;//spi模式
	}
	
	//通过在CPageHardware类的控制CPageGeneral一些控制
    theAppGenral->set_mac_serial_data();

	CDlgConfig* q = (CDlgConfig*)GetParent()->GetParent();
	CPageFormat *theAppFormat = &q->m_page_format;

	theAppFormat->ShowLowFormat();
			
}

void CPageHardware::OnRadioNandflash() 
{
	BOOL check = FALSE;
	
	CDlgConfig* p = (CDlgConfig*)GetParent()->GetParent();
	CPageGeneral *theAppGenral = &p->m_page_general;
	
	// TODO: Add your control notification handler code here
	check = ((CButton *)GetDlgItem(IDC_RADIO_NANDFLASH))->GetCheck();
	if (check)
	{
		theConfig.burn_mode = E_CONFIG_NAND;//nand模式
	}
	
	//通过在CPageHardware类的控制CPageGeneral一些控制
    theAppGenral->set_mac_serial_data();
	
	CDlgConfig* q = (CDlgConfig*)GetParent()->GetParent();
	CPageFormat *theAppFormat = &q->m_page_format;

	theAppFormat->ShowLowFormat();
			
}

void CPageHardware::OnRadioSd() 
{
	BOOL check = FALSE;
	
	CDlgConfig* p = (CDlgConfig*)GetParent()->GetParent();
	CPageGeneral *theAppGenral = &p->m_page_general;
	
	// TODO: Add your control notification handler code here
	check = ((CButton *)GetDlgItem(IDC_RADIO_SD))->GetCheck();
	if (check)
	{
		theConfig.burn_mode = E_CONFIG_SD;//sd模式
	}
	
	//通过在CPageHardware类的控制CPageGeneral一些控制
    theAppGenral->set_mac_serial_data();

	CDlgConfig* q = (CDlgConfig*)GetParent()->GetParent();
	CPageFormat *theAppFormat = &q->m_page_format;

	theAppFormat->ShowLowFormat();
			
}

void CPageHardware::OnRadioDebug() 
{
	BOOL check = FALSE;

	// TODO: Add your control notification handler code here
	check = ((CButton *)GetDlgItem(IDC_RADIO_DEBUG))->GetCheck();
	if (check)
	{
		theConfig.burn_mode = E_CONFIG_DEBUG;//debug方式
	}
	
	CDlgConfig* q = (CDlgConfig*)GetParent()->GetParent();
	CPageFormat *theAppFormat = &q->m_page_format;

	theAppFormat->ShowLowFormat();
			
}

void CPageHardware::OnRadioJtag() 
{
	BOOL check = FALSE;

	// TODO: Add your control notification handler code here
	check = ((CButton *)GetDlgItem(IDC_RADIO_JTAG))->GetCheck();
	if (check)
	{
		theConfig.burn_mode = E_CONFIG_JTAG;//JTAG方式
	}
	
	CDlgConfig* q = (CDlgConfig*)GetParent()->GetParent();
	CPageFormat *theAppFormat = &q->m_page_format;

	theAppFormat->ShowLowFormat();
			
}

void CPageHardware::SetBlgAttr(CString planformType) 
{
#if 1
	if (!initFlag)
	{
		return;
	}

	((CButton *)GetDlgItem(IDC_CHECK_UDISK_UPDATE))->ShowWindow(1);	
	((CButton *)GetDlgItem(IDC_CHECK_UDISK_UPDATE))->SetCheck(theConfig.bUDiskUpdate);
	if (planformType == E_ROST_PLANFORM)
	{
		//((CButton *)GetDlgItem(IDC_CHECK_UDISK_UPDATE))->ShowWindow(0);	
		((CButton *)GetDlgItem(IDC_CHECK_USB2))->ShowWindow(1);
		((CButton *)GetDlgItem(IDC_CHECK_USB2))->SetCheck(theConfig.bUsb2);
		
		((CButton *)GetDlgItem(IDC_CHECK_PLLFREP))->ShowWindow(1);
		((CButton *)GetDlgItem(IDC_CHECK_PLLFREP))->SetCheck(theConfig.bPLL_Frep_change);

		GetDlgItem(IDC_STATIC_BURNED)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_NONE)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_RESET)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_POWER_OFF)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_GPIO)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_GPIO_PIN_NUM)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_LVH)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_PULLUP)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_GPIO_PULLUP_EN)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_GPIO_PULLDOWN_EN)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_PULLDOWN)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_LVL)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_RTC_WAKUP)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_RTC_LVH)->ShowWindow(FALSE);//不可见
		GetDlgItem(IDC_BURNED_RTC_LVL)->ShowWindow(FALSE);//不可见
	}
	else if (planformType == E_LINUX_PLANFORM)
	{
		//((CButton *)GetDlgItem(IDC_CHECK_UDISK_UPDATE))->ShowWindow(1);	
		//((CButton *)GetDlgItem(IDC_CHECK_UDISK_UPDATE))->SetCheck(theConfig.bUDiskUpdate);
		((CButton *)GetDlgItem(IDC_CHECK_USB2))->ShowWindow(0);
		((CButton *)GetDlgItem(IDC_CHECK_PLLFREP))->ShowWindow(0);

		GetDlgItem(IDC_STATIC_BURNED)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_NONE)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_RESET)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_POWER_OFF)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_GPIO)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_GPIO_PIN_NUM)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_LVH)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_PULLUP)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_GPIO_PULLUP_EN)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_GPIO_PULLDOWN_EN)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_PULLDOWN)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_LVL)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_RTC_WAKUP)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_RTC_LVH)->ShowWindow(TRUE);//可见
		GetDlgItem(IDC_BURNED_RTC_LVL)->ShowWindow(TRUE);//可见
	}
#endif
}

void CPageHardware::OnCheckPllfrep() 
{
	// TODO: Add your control notification handler code here
	BOOL check = theConfig.bPLL_Frep_change;
	check = ((CButton *)GetDlgItem(IDC_CHECK_PLLFREP))->GetCheck();
	theConfig.bPLL_Frep_change= check ? TRUE : FALSE;// change pll frep
}

void CPageHardware::OnRadioSpiNandflash() 
{
	// TODO: Add your control notification handler code here
	BOOL check = FALSE;
	
	CDlgConfig* p = (CDlgConfig*)GetParent()->GetParent();
	CPageGeneral *theAppGenral = &p->m_page_general;
	
	// TODO: Add your control notification handler code here
	check = ((CButton *)GetDlgItem(IDC_RADIO_SPI_NANDFLASH))->GetCheck();
	if (check)
	{
		theConfig.burn_mode = E_CONFIG_SPI_NAND;//nand模式
	}
	
	//通过在CPageHardware类的控制CPageGeneral一些控制
    theAppGenral->set_mac_serial_data();
	
	CDlgConfig* q = (CDlgConfig*)GetParent()->GetParent();
	CPageFormat *theAppFormat = &q->m_page_format;
	
	theAppFormat->ShowLowFormat();
}
