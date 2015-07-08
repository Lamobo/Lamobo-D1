// PageBaseband.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "PageBaseband.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CConfig theConfig;
extern CBurnToolApp theApp;
extern HINSTANCE _hInstance;
/////////////////////////////////////////////////////////////////////////////
// CPageBaseband property page

IMPLEMENT_DYNCREATE(CPageBaseband, CPropertyPage)

CPageBaseband::CPageBaseband() : CPropertyPage(CPageBaseband::IDD)
{
	//{{AFX_DATA_INIT(CPageBaseband)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPageBaseband::~CPageBaseband()
{
}

void CPageBaseband::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageBaseband)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageBaseband, CPropertyPage)
	//{{AFX_MSG_MAP(CPageBaseband)
	ON_BN_CLICKED(IDC_CHECK_DOWNLOAD_FLS, OnCheckDownloadFls)
	ON_BN_CLICKED(IDC_CHECK_DOWNLOAD_EEP, OnCheckDownloadEep)
	ON_BN_CLICKED(IDC_BTN_BROWSE_FLS, OnBtnBrowseFls)
	ON_BN_CLICKED(IDC_BTN_BROWSE_EEP, OnBtnBrowseEep)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageBaseband message handlers

BOOL CPageBaseband::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// TODO: Add extra initialization here

	SetupDisplay();
	set_config_item(theConfig);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPageBaseband::get_config_data(CConfig &config)
{
	CString str;
	int check;

	USES_CONVERSION;
	
	//check box
	config.DownloadMode = E_DOWNLOAD_BOTH;
	check = ((CButton *)GetDlgItem(IDC_RADIO_DOWNLOAD_AK))->GetCheck();
	if(check)
	{
		config.DownloadMode = E_DOWNLOAD_AK_ONLY;//E_DOWNLOAD_AK_ONLY
	}
	check = ((CButton *)GetDlgItem(IDC_RADIO_DOWNLOAD_INFINEON))->GetCheck();
	if(check)
	{
		config.DownloadMode = E_DOWNLOAD_IFX_ONLY;//E_DOWNLOAD_IFX_ONLY
	}

	//baudrate
	GetDlgItemText(IDC_COMBO_MOUDLE_BURN_BUADRATE, str);//²¨ÌØÂÊ
	config.baudrate_module_burn = atoi(T2A(str));

	//GPIO
	GetDlgItemText(IDC_EDIT_GPIO_DTR, str);// gpio
	config.gpio_dtr = atoi(T2A(str));

	GetDlgItemText(IDC_EDIT_GPIO_MODULE_IGT, str);//gpio_module_igt
	config.gpio_module_igt = atoi(T2A(str));
	
	GetDlgItemText(IDC_EDIT_GPIO_MODULE_RESET, str);//gpio_module_reset
	config.gpio_module_reset = atoi(T2A(str));

	check = ((CButton *)GetDlgItem(IDC_CHECK_DOWNLOAD_FLS))->GetCheck();//bDownloadFLS
	config.bDownloadFLS = check ? TRUE : FALSE;

	check = ((CButton *)GetDlgItem(IDC_CHECK_DOWNLOAD_EEP))->GetCheck();//bDownloadEEP
	config.bDownloadEEP = check ? TRUE : FALSE;
	
	//path
	GetDlgItemText(IDC_EDIT_PATH_FLS, str);//path_fls
	_tcsncpy(config.path_fls, str, MAX_PATH);

	GetDlgItemText(IDC_EDIT_PATH_EEP, str);//path_eep
	_tcsncpy(config.path_eep, str, MAX_PATH);

	return TRUE;
}

BOOL CPageBaseband::set_config_item(CConfig &config)
{
	CString str;
	
	//check box
	if(E_DOWNLOAD_AK_ONLY == config.DownloadMode)//E_DOWNLOAD_AK_ONLY
	{
		((CButton *)GetDlgItem(IDC_RADIO_DOWNLOAD_AK))->SetCheck(BST_CHECKED);
	}
	else if(E_DOWNLOAD_IFX_ONLY == config.DownloadMode)//E_DOWNLOAD_IFX_ONLY
	{
		((CButton *)GetDlgItem(IDC_RADIO_DOWNLOAD_INFINEON))->SetCheck(BST_CHECKED);
	}
	else
	{
		((CButton *)GetDlgItem(IDC_RADIO_DOWNLOAD_BOTH))->SetCheck(BST_CHECKED);
	}

	//baud rate
	str.Format(_T("%u"), config.baudrate_module_burn);//baudrate_module_burn
	SetDlgItemText(IDC_COMBO_MOUDLE_BURN_BUADRATE, str);

	//GPIO
	str.Format(_T("%u"), config.gpio_dtr);//gpio_dtr
	SetDlgItemText(IDC_EDIT_GPIO_DTR, str);

	str.Format(_T("%u"), config.gpio_module_igt);//gpio_module_igt
	SetDlgItemText(IDC_EDIT_GPIO_MODULE_IGT, str);

	str.Format(_T("%u"), config.gpio_module_reset);//gpio_module_reset
	SetDlgItemText(IDC_EDIT_GPIO_MODULE_RESET, str);

	//fls
	if(config.bDownloadFLS)
	{
		((CButton *)GetDlgItem(IDC_CHECK_DOWNLOAD_FLS))->SetCheck(BST_CHECKED);//bDownloadFLS
	}

	//eep
	if(config.bDownloadEEP)
	{
		((CButton *)GetDlgItem(IDC_CHECK_DOWNLOAD_EEP))->SetCheck(BST_CHECKED);//bDownloadEEP
	}


	//path
	SetDlgItemText(IDC_EDIT_PATH_FLS, config.path_fls);//path_fls
	SetDlgItemText(IDC_EDIT_PATH_EEP, config.path_eep);//path_eep

	check_download();
	
	return TRUE;
}

void CPageBaseband::SetupDisplay()
{
	CString str;

	str = theApp.GetString(IDS_BASEBAND_DOWNLOAD_MODE);
	GetDlgItem(IDC_STATIC_DOWNLOAD_MODE)->SetWindowText(str);//IDS_BASEBAND_DOWNLOAD_MODE

	str = theApp.GetString(IDS_BASEBAND_DOWNLOAD_AK_ONLY);//IDS_BASEBAND_DOWNLOAD_AK_ONLY
	GetDlgItem(IDC_RADIO_DOWNLOAD_AK)->SetWindowText(str);

	str = theApp.GetString(IDS_BASEBAND_DOWNLOAD_IFX_ONLY);
	GetDlgItem(IDC_RADIO_DOWNLOAD_INFINEON)->SetWindowText(str);//IDC_RADIO_DOWNLOAD_INFINEON

	str = theApp.GetString(IDS_BASEBAND_DOWNLOAD_BOTH);
	GetDlgItem(IDC_RADIO_DOWNLOAD_BOTH)->SetWindowText(str);//GetDlgItem(IDC_RADIO_DOWNLOAD_BOTH
	
	str = theApp.GetString(IDS_BASEBAND_BURN_SETTING);
	GetDlgItem(IDC_STATIC_MODULE_BURN_SETTING)->SetWindowText(str);//IDC_STATIC_MODULE_BURN_SETTING

	str = theApp.GetString(IDS_BASEBAND_BAUDRATE);
	GetDlgItem(IDC_STATIC_MODULE_BURN_BAUDRATE)->SetWindowText(str);//GetDlgItem(IDC_STATIC_MODULE_BURN_BAUDRATE

	str = theApp.GetString(IDS_BASEBAND_DOWNLOAD_FLS);
	GetDlgItem(IDC_CHECK_DOWNLOAD_FLS)->SetWindowText(str);//IDC_CHECK_DOWNLOAD_FLS

	str = theApp.GetString(IDS_BASEBAND_DOWNLOAD_EEP);
	GetDlgItem(IDC_CHECK_DOWNLOAD_EEP)->SetWindowText(str);//IDC_CHECK_DOWNLOAD_EEP

	str = theApp.GetString(IDS_BASEBAND_BROWSE);
	GetDlgItem(IDC_BTN_BROWSE_FLS)->SetWindowText(str);//IDC_BTN_BROWSE_FLS

	str = theApp.GetString(IDS_BASEBAND_BROWSE);
	GetDlgItem(IDC_BTN_BROWSE_EEP)->SetWindowText(str);//IDC_BTN_BROWSE_EEP

	str = theApp.GetString(IDS_BASEBAND_GPIO_SETTING);
	GetDlgItem(IDC_STATIC_MODULE_GPIO_SETTING)->SetWindowText(str);//IDC_STATIC_MODULE_GPIO_SETTING

}

BOOL CPageBaseband::check_input()
{
	int check;
	CString str;
	
	check = ((CButton *)GetDlgItem(IDC_CHECK_DOWNLOAD_FLS))->GetCheck();
	if(check)
	{
		//check fls
		GetDlgItemText(IDC_EDIT_PATH_FLS, str);
		if(str.Right(3) != _T("fls"))
		{
			MessageBox(_T("wrong fls file!!!"));
			return FALSE;
		}
	}

	check = ((CButton *)GetDlgItem(IDC_CHECK_DOWNLOAD_EEP))->GetCheck();//IDC_CHECK_DOWNLOAD_EEP
	if(check)
	{
		//check eep
		GetDlgItemText(IDC_EDIT_PATH_EEP, str);
		if(str.Right(3) != _T("eep"))
		{
			MessageBox(_T("wrong eep file!!!"));
			return FALSE;
		}	
	}

	return TRUE;
}

void CPageBaseband::check_download()
{
	int check;

	//fls
	check = ((CButton *)GetDlgItem(IDC_CHECK_DOWNLOAD_FLS))->GetCheck();
	if(check)
	{
		GetDlgItem(IDC_EDIT_PATH_FLS)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_BROWSE_FLS)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_EDIT_PATH_FLS)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_BROWSE_FLS)->EnableWindow(FALSE);
	}

	//eep
	check = ((CButton *)GetDlgItem(IDC_CHECK_DOWNLOAD_EEP))->GetCheck();
	if(check)
	{
		GetDlgItem(IDC_EDIT_PATH_EEP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BTN_BROWSE_EEP)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_EDIT_PATH_EEP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BTN_BROWSE_EEP)->EnableWindow(FALSE);
	}
}

void CPageBaseband::OnCheckDownloadFls() 
{
	// TODO: Add your control notification handler code here
	check_download();
	
}

void CPageBaseband::OnCheckDownloadEep() 
{
	// TODO: Add your control notification handler code here
	check_download();
}

void CPageBaseband::OnBtnBrowseFls() 
{
	// TODO: Add your control notification handler code here
		OPENFILENAME ofn;
	TCHAR pstrFileName[260] = {0}, pstrTitleName[260] = {0};
	
	TCHAR szFilter[] =	TEXT ("FLS Files (*.fls)\0*.fls;\0\0");
	
	memset ( &ofn, 0, sizeof ( ofn ) );
	
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hInstance         = _hInstance ;
	ofn.hwndOwner         = GetSafeHwnd();
	ofn.lpstrFilter       = szFilter;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrDefExt       = TEXT ("fls") ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.Flags             = OFN_FILEMUSTEXIST; 
	
	
	if(GetOpenFileName (&ofn))
	{
		TCHAR * relative_path = pstrFileName;

		if((relative_path = _tcsstr(pstrFileName, theConfig.path_module)) != NULL)
		{
			relative_path = pstrFileName + _tcslen(theConfig.path_module);
			SetDlgItemText(IDC_EDIT_PATH_FLS, relative_path);
		}
		else
		{
			SetDlgItemText(IDC_EDIT_PATH_FLS, pstrFileName);	
		}
	}	
	
}

void CPageBaseband::OnBtnBrowseEep() 
{
	// TODO: Add your control notification handler code here
	OPENFILENAME ofn;
	TCHAR pstrFileName[260] = {0}, pstrTitleName[260] = {0};
	
	TCHAR szFilter[] =	TEXT ("EEP Files (*.eep)\0*.eep;\0\0");
	
	memset ( &ofn, 0, sizeof ( ofn ) );
	
	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hInstance         = _hInstance ;
	ofn.hwndOwner         = GetSafeHwnd();
	ofn.lpstrFilter       = szFilter;
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrDefExt       = TEXT ("fls") ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.Flags             = OFN_FILEMUSTEXIST; 
	
	
	if(GetOpenFileName (&ofn))
	{
		TCHAR * relative_path = pstrFileName;

		if((relative_path = _tcsstr(pstrFileName, theConfig.path_module)) != NULL)
		{
			relative_path = pstrFileName + _tcslen(theConfig.path_module);
			SetDlgItemText(IDC_EDIT_PATH_EEP, relative_path);
		}
		else
		{
			SetDlgItemText(IDC_EDIT_PATH_EEP, pstrFileName);	
		}
	}		
	
}
