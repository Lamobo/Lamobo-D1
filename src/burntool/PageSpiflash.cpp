// PageSpiflash.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "PageSpiflash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CConfig theConfig;
extern CBurnToolApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPageSpiflash property page

IMPLEMENT_DYNCREATE(CPageSpiflash, CPropertyPage)

CPageSpiflash::CPageSpiflash() : CPropertyPage(CPageSpiflash::IDD)
{
	//{{AFX_DATA_INIT(CPageSpiflash)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPageSpiflash::~CPageSpiflash()
{
}

void CPageSpiflash::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageSpiflash)
	DDX_Control(pDX, IDC_LIST_SPIFLASH_PARAM, m_spiflash_param_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageSpiflash, CPropertyPage)
	//{{AFX_MSG_MAP(CPageSpiflash)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageSpiflash message handlers
BOOL CPageSpiflash::get_config_data(CConfig &config)
{
	CString str;
	int count;
	
	USES_CONVERSION;

	count= m_spiflash_param_list.GetItemCount();
	config.spiflash_parameter_count = count;

	if(config.spiflash_parameter != NULL)
	{
		free(config.spiflash_parameter);
	}

	config.spiflash_parameter = (T_SFLASH_PHY_INFO_TRANSC* )malloc(sizeof(T_SFLASH_PHY_INFO_TRANSC) * config.spiflash_parameter_count);
	
	memset(config.spiflash_parameter, 0, sizeof(T_SFLASH_PHY_INFO_TRANSC) * config.spiflash_parameter_count);
	
	for(int i = 0; i < count; i++)
	{
		str = m_spiflash_param_list.GetItemText(i, 1);
		config.spiflash_parameter[i].chip_id = config.hex2int(T2A(str));

		str = m_spiflash_param_list.GetItemText(i, 2);
		config.spiflash_parameter[i].total_size = atoi(T2A(str));		

		str = m_spiflash_param_list.GetItemText(i, 3);
		config.spiflash_parameter[i].page_size = atoi(T2A(str));

		str = m_spiflash_param_list.GetItemText(i, 4);
		config.spiflash_parameter[i].program_size = atoi(T2A(str));

		str = m_spiflash_param_list.GetItemText(i, 5);
		config.spiflash_parameter[i].erase_size = atoi(T2A(str));

		str = m_spiflash_param_list.GetItemText(i, 6);
		config.spiflash_parameter[i].clock = atoi(T2A(str));

		str = m_spiflash_param_list.GetItemText(i, 7);
		config.spiflash_parameter[i].flag = config.hex2int(T2A(str));

		str = m_spiflash_param_list.GetItemText(i, 8);
		config.spiflash_parameter[i].protect_mask = config.hex2int(T2A(str));

		str = m_spiflash_param_list.GetItemText(i, 9);
		strncpy(config.spiflash_parameter[i].des_str, T2A(str), 31);

		str = m_spiflash_param_list.GetItemText(i, 10);
		config.spiflash_parameter[i].reserved1 = atoi(T2A(str));
		
	}

	return TRUE;
}

BOOL CPageSpiflash::set_config_item(CConfig &config)
{
	CString strIndex;
	CString str;

	USES_CONVERSION;

	for(UINT i = 0; i < config.spiflash_parameter_count; i++)
	{		
		strIndex.Format(_T("%d"), i+1);
		m_spiflash_param_list.InsertItem(i, strIndex);

		str.Format(_T("0x%x"), config.spiflash_parameter[i].chip_id);//chip_id
		m_spiflash_param_list.SetItemText(i, 1, str);

		str.Format(_T("%d"), config.spiflash_parameter[i].total_size);//total_size
		m_spiflash_param_list.SetItemText(i, 2, str);	

		str.Format(_T("%d"), config.spiflash_parameter[i].page_size);//page_size
		m_spiflash_param_list.SetItemText(i, 3, str);

		str.Format(_T("%d"), config.spiflash_parameter[i].program_size);//program_size
		m_spiflash_param_list.SetItemText(i, 4, str);

		str.Format(_T("%d"), config.spiflash_parameter[i].erase_size);//erase_size
		m_spiflash_param_list.SetItemText(i, 5, str);

		str.Format(_T("%d"), config.spiflash_parameter[i].clock);//clock
		m_spiflash_param_list.SetItemText(i, 6, str);	

		str.Format(_T("0x%x"), config.spiflash_parameter[i].flag);//flag
		m_spiflash_param_list.SetItemText(i, 7, str);

		str.Format(_T("0x%x"), config.spiflash_parameter[i].protect_mask);//protect_mask
		m_spiflash_param_list.SetItemText(i, 8, str);	

		m_spiflash_param_list.SetItemText(i, 9, A2T(config.spiflash_parameter[i].des_str));//des_str
		
		str.Format(_T("%d"), config.spiflash_parameter[i].reserved1);//disk erase size
		m_spiflash_param_list.SetItemText(i, 10, str);
	}
	
	return TRUE;
}

BOOL CPageSpiflash::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	SetupDisplay();
	
	ListView_SetExtendedListViewStyle(m_spiflash_param_list.m_hWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT); 
	
	m_spiflash_param_list.InsertColumn(0, _T("N.O."), LVCFMT_LEFT, 40);//序号
	m_spiflash_param_list.InsertColumn(1, _T("ChipID"), LVCFMT_LEFT, 90);//ChipID
	m_spiflash_param_list.InsertColumn(2, _T("Total_Size"), LVCFMT_LEFT, 90);//Total_Size	
	m_spiflash_param_list.InsertColumn(3, _T("Page Size"), LVCFMT_LEFT, 90);//Page Size
	m_spiflash_param_list.InsertColumn(4, _T("Program Size"), LVCFMT_LEFT, 90);//Program Size
	m_spiflash_param_list.InsertColumn(5, _T("Erase Size"), LVCFMT_LEFT, 90);//Erase Size
	m_spiflash_param_list.InsertColumn(6, _T("Clock"), LVCFMT_LEFT, 90);//Clock
	m_spiflash_param_list.InsertColumn(7, _T("Flag"), LVCFMT_LEFT, 90);//Flag
	m_spiflash_param_list.InsertColumn(8, _T("Protect Mask"), LVCFMT_LEFT, 90);//Protect Mask
	m_spiflash_param_list.InsertColumn(9, _T("Descriptor string"), LVCFMT_LEFT, 90);//Descriptor
	m_spiflash_param_list.InsertColumn(10, _T("disk_Esize(K)"), LVCFMT_LEFT, 90);//reserved1  以K对齐 


	set_config_item(theConfig);//附值


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CPageSpiflash::SetupDisplay()
{
    CString str;

	str = theApp.GetString(IDS_SPI_LIST);
	GetDlgItem(IDC_STATIC_SPI_LIST)->SetWindowText(str);//spi列表
}
