// DlgConfig.cpp : implementation file
//

#include "stdafx.h"
#include "BurnTool.h"
#include "DlgConfig.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CConfig theConfig;
extern CBurnToolApp theApp;
/////////////////////////////////////////////////////////////////////////////
// CDlgConfig dialog


CDlgConfig::CDlgConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgConfig)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDlgConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgConfig)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgConfig, CDialog)
	//{{AFX_MSG_MAP(CDlgConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgConfig message handlers

BOOL CDlgConfig::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString str;
	
	str = theApp.GetString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);

	str = theApp.GetString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);

	if(theConfig.bUI_ctrl)
	{
		CTreePropSheet::SetPageIcon(&m_page_general, IDI_ICON_PREFERENCE);
		CTreePropSheet::SetPageIcon(&m_page_download, IDI_ICON_PREFERENCE);

		m_sheet.AddPage(&m_page_general);
		m_sheet.AddPage(&m_page_download);
	}
	else
	{
		CTreePropSheet::SetPageIcon(&m_page_general, IDI_ICON_PREFERENCE);
		CTreePropSheet::SetPageIcon(&m_page_download, IDI_ICON_PREFERENCE);
		CTreePropSheet::SetPageIcon(&m_page_hardware, IDI_ICON_PREFERENCE);
		CTreePropSheet::SetPageIcon(&m_page_format, IDI_ICON_PREFERENCE);
		CTreePropSheet::SetPageIcon(&m_page_nandflash, IDI_ICON_PREFERENCE);
		CTreePropSheet::SetPageIcon(&m_page_spinandflash, IDI_ICON_PREFERENCE);
		CTreePropSheet::SetPageIcon(&m_page_spiflash, IDI_ICON_PREFERENCE);
	//	CTreePropSheet::SetPageIcon(&m_page_baseband, IDI_ICON_PREFERENCE);
	

		m_sheet.AddPage(&m_page_general);
		m_sheet.AddPage(&m_page_download);
		m_sheet.AddPage(&m_page_hardware);
		m_sheet.AddPage(&m_page_format);
		m_sheet.AddPage(&m_page_nandflash);
		m_sheet.AddPage(&m_page_spinandflash);
		m_sheet.AddPage(&m_page_spiflash);
	//	m_sheet.AddPage(&m_page_baseband);
	}		

	m_sheet.SetTreeViewMode(TRUE, TRUE, TRUE);
	m_sheet.SetTreeWidth(120);

	m_sheet.Create(this, WS_CHILD | WS_VISIBLE, WS_EX_CONTROLPARENT);
	m_sheet.SetWindowPos(NULL, 1, 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgConfig::OnOK() 
{
	if(theConfig.bUI_ctrl)
	{
		//the general page
		if(NULL != m_page_general.GetSafeHwnd())
		{
			if(!m_page_general.get_config_data(theConfig))
			{
				return;
			}
		}	
		
		//the download page
		if(NULL != m_page_download.GetSafeHwnd())
		{
			m_page_download.get_config_data(theConfig);
		}
	}
	else
	{
		//the general page

		if(NULL != m_page_general.GetSafeHwnd())
		{
			if(!m_page_general.get_config_data(theConfig))
			{
				return;
			}
		}	

		//the download page
		if(NULL != m_page_download.GetSafeHwnd())
		{
			m_page_download.get_config_data(theConfig);
		}

		//the hardware page
		if(NULL != m_page_hardware.GetSafeHwnd())
		{
			m_page_hardware.get_config_data(theConfig);
		}

		//the format page
		if(NULL != m_page_format.GetSafeHwnd())
		{
			if (!m_page_format.get_config_data(theConfig))
			{
				return;
			}
		}


		//the nandflash page
		if(NULL != m_page_nandflash.GetSafeHwnd())
		{
			m_page_nandflash.get_config_data(theConfig);
		}

		//the nandflash page
		if(NULL != m_page_spinandflash.GetSafeHwnd())
		{
			m_page_spinandflash.get_config_data(theConfig);
		}

/*
#if 0
		//the baseband page
		if(NULL != m_page_baseband.GetSafeHwnd())
		{
			m_page_baseband.get_config_data(theConfig);
		}	
#endif
		*/

		//the spiflash page

		if(NULL != m_page_spiflash.GetSafeHwnd())
		{
			m_page_spiflash.get_config_data(theConfig);
		}

		
	}	

	BT_Init(theConfig.device_num,  BurnThread,  BurnProgress);
	CString strTitle;

	strTitle.Format(_T("BurnTool V%d.%d.%02d.%02d [%s]"), MAIN_VERSION, SUB_VERSION0, SUB_VERSION1, SUB_VERSION2, theConfig.project_name);
	
	CMainFrame *pMainFrame = (CMainFrame *)AfxGetMainWnd();

	pMainFrame->SetWindowText(strTitle);

    pMainFrame->RefreshWorkInfo();

	CDialog::OnOK();
}

BOOL CDlgConfig::check_input()
{
	if(NULL != m_page_general.GetSafeHwnd())
	{
		if(!m_page_general.check_input())
		{
			return false;
		}
	}

	if(NULL != m_page_baseband.GetSafeHwnd() && !theConfig.bUI_ctrl)
	{
		if(!m_page_baseband.check_input())
		{
			return false;
		}
	}

	return false;
}

void CDlgConfig::OnCancel() 
{
	//UINT i = 0;

	// TODO: Add extra cleanup here
	USES_CONVERSION;

	_tcsncpy(theConfig.planform_name, theConfig.init_planform_name, MAX_PROJECT_NAME);

	theConfig.chip_type = theConfig.init_chip_type;

	theConfig.ram_param.type = theConfig.init_ram_param.type;

	theConfig.bUsb2 = theConfig.init_bUsb2;
	theConfig.bPLL_Frep_change = theConfig.init_bPLL_Frep_change;

	theConfig.burn_mode = theConfig.init_burn_mode;

	theConfig.bUpdate = theConfig.init_bUpdate;

	theConfig.bUpdateself = theConfig.init_bUpdateself;

	theConfig.bUDiskUpdate = theConfig.init_bUDiskUpdate;

	theConfig.burned_param.burned_mode = theConfig.init_burned_param.burned_mode;

	theConfig.burned_param.bWakup = theConfig.init_burned_param.bWakup;
	
	CDialog::OnCancel();
}
