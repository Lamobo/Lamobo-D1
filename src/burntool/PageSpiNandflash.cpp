// PageSpiNandflash.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "PageSpiNandflash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CConfig theConfig;
extern CBurnToolApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPageSpiNandflash property page

IMPLEMENT_DYNCREATE(CPageSpiNandflash, CPropertyPage)

CPageSpiNandflash::CPageSpiNandflash() : CPropertyPage(CPageSpiNandflash::IDD)
{
	//{{AFX_DATA_INIT(CPageSpiNandflash)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPageSpiNandflash::~CPageSpiNandflash()
{
}

void CPageSpiNandflash::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageSpiNandflash)
	DDX_Control(pDX, IDC_LIST_SPINANDFLASH_PARAM, m_spi_nandflash_param_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageSpiNandflash, CPropertyPage)
	//{{AFX_MSG_MAP(CPageSpiNandflash)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageSpiNandflash message handlers
BOOL CPageSpiNandflash::get_config_data(CConfig &config)
{
	CString str;
	int count;
	
	USES_CONVERSION;
	count= m_spi_nandflash_param_list.GetItemCount();
	config.spi_nandflash_parameter_count = count;

	if(config.spi_nandflash_parameter != NULL)
	{
		free(config.spi_nandflash_parameter);
	}

	config.spi_nandflash_parameter = (T_NAND_PHY_INFO_TRANSC* )malloc(sizeof(T_NAND_PHY_INFO_TRANSC) * config.spi_nandflash_parameter_count);
	
	memset(config.spi_nandflash_parameter, 0, sizeof(T_NAND_PHY_INFO_TRANSC) * config.spi_nandflash_parameter_count);

	for(int i = 0; i < count; i++)
	{
		str = m_spi_nandflash_param_list.GetItemText(i, 1);
		config.spi_nandflash_parameter[i].chip_id = config.hex2int(T2A(str));

		str = m_spi_nandflash_param_list.GetItemText(i, 2);
		config.spi_nandflash_parameter[i].page_size = atoi(T2A(str));

		str = m_spi_nandflash_param_list.GetItemText(i, 3);
		config.spi_nandflash_parameter[i].page_per_blk = atoi(T2A(str));

		str = m_spi_nandflash_param_list.GetItemText(i, 4);
		config.spi_nandflash_parameter[i].blk_num = atoi(T2A(str));

		//str = m_spi_nandflash_param_list.GetItemText(i, 5);
		//config.spi_nandflash_parameter[i].group_blk_num = atoi(T2A(str));

		str = m_spi_nandflash_param_list.GetItemText(i, 5);
		config.spi_nandflash_parameter[i].plane_blk_num = atoi(T2A(str));

		str = m_spi_nandflash_param_list.GetItemText(i, 6);
		config.spi_nandflash_parameter[i].spare_size = atoi(T2A(str));

		//str = m_spi_nandflash_param_list.GetItemText(i, 8);
		//config.spi_nandflash_parameter[i].col_cycle = atoi(T2A(str));

		//str = m_spi_nandflash_param_list.GetItemText(i, 9);
		//config.spi_nandflash_parameter[i].lst_col_mask = atoi(T2A(str));

		//str = m_spi_nandflash_param_list.GetItemText(i, 10);
		//config.spi_nandflash_parameter[i].row_cycle = atoi(T2A(str));

		//str = m_spi_nandflash_param_list.GetItemText(i, 11);
		//config.spi_nandflash_parameter[i].last_row_mask = atoi(T2A(str));

		str = m_spi_nandflash_param_list.GetItemText(i, 7);
		config.spi_nandflash_parameter[i].custom_nd = atoi(T2A(str));

		str = m_spi_nandflash_param_list.GetItemText(i, 8);
		config.spi_nandflash_parameter[i].flag = config.hex2int(T2A(str));

		//str = m_spi_nandflash_param_list.GetItemText(i, 14);
		//config.spi_nandflash_parameter[i].cmd_len = config.hex2int(T2A(str));

		str = m_spi_nandflash_param_list.GetItemText(i, 9);
		config.spi_nandflash_parameter[i].data_len = atoi(T2A(str));

		str = m_spi_nandflash_param_list.GetItemText(i, 10);
		strncpy(config.spi_nandflash_parameter[i].des_str, T2A(str), 32);
		
	}

	return TRUE;
}

BOOL CPageSpiNandflash::set_config_item(CConfig &config)
{
	CString strIndex;
	CString str;

	USES_CONVERSION;
	for(UINT i = 0; i < config.spi_nandflash_parameter_count; i++)
	{		
		strIndex.Format(_T("%d"), i+1);
		m_spi_nandflash_param_list.InsertItem(i, strIndex);

		str.Format(_T("0x%x"), config.spi_nandflash_parameter[i].chip_id);//chip_id
		m_spi_nandflash_param_list.SetItemText(i, 1, str);

		str.Format(_T("%d"), config.spi_nandflash_parameter[i].page_size);//page_size
		m_spi_nandflash_param_list.SetItemText(i, 2, str);

		str.Format(_T("%d"), config.spi_nandflash_parameter[i].page_per_blk);//page_per_blk
		m_spi_nandflash_param_list.SetItemText(i, 3, str);

		str.Format(_T("%d"), config.spi_nandflash_parameter[i].blk_num);//blk_num
		m_spi_nandflash_param_list.SetItemText(i, 4, str);

		//str.Format(_T("%d"), config.spi_nandflash_parameter[i].group_blk_num);//group_blk_num
		//m_spi_nandflash_param_list.SetItemText(i, 5, str);

		str.Format(_T("%d"), config.spi_nandflash_parameter[i].plane_blk_num);//plane_blk_num
		m_spi_nandflash_param_list.SetItemText(i, 5, str);

		str.Format(_T("%d"), config.spi_nandflash_parameter[i].spare_size);//spare_size
		m_spi_nandflash_param_list.SetItemText(i, 6, str);

		//str.Format(_T("%d"), config.spi_nandflash_parameter[i].col_cycle);//col_cycle
		//m_spi_nandflash_param_list.SetItemText(i, 8, str);

		//str.Format(_T("%d"), config.spi_nandflash_parameter[i].lst_col_mask);//lst_col_mask
		//m_spi_nandflash_param_list.SetItemText(i, 9, str);

		//str.Format(_T("%d"), config.spi_nandflash_parameter[i].row_cycle);//row_cycle
		//m_spi_nandflash_param_list.SetItemText(i, 10, str);

		//str.Format(_T("%d"), config.spi_nandflash_parameter[i].last_row_mask);//last_row_mask
		//m_spi_nandflash_param_list.SetItemText(i, 11, str);

		str.Format(_T("%d"), config.spi_nandflash_parameter[i].custom_nd);//custom_nd
		m_spi_nandflash_param_list.SetItemText(i, 7, str);

		str.Format(_T("0x%.8X"), config.spi_nandflash_parameter[i].flag);//flag
		m_spi_nandflash_param_list.SetItemText(i, 8, str);
		
		//str.Format(_T("0x%.8X"), config.spi_nandflash_parameter[i].cmd_len);//cmd_len
		//m_spi_nandflash_param_list.SetItemText(i, 14, str);

		str.Format(_T("%d"), config.spi_nandflash_parameter[i].data_len);//data_len
		m_spi_nandflash_param_list.SetItemText(i, 9, str);

		m_spi_nandflash_param_list.SetItemText(i, 10, A2T(config.spi_nandflash_parameter[i].des_str));//des_str
		
	}
	
	return TRUE;
}

void CPageSpiNandflash::SetupDisplay()
{
	CString str;

	str = theApp.GetString(IDS_SPINAND_LIST);
	GetDlgItem(IDC_STATIC_SPI_NANDFLSH_LIST)->SetWindowText(str);//nand列表
}

BOOL CPageSpiNandflash::OnInitDialog() 
{
    CPropertyPage::OnInitDialog();

	SetupDisplay();
	
	ListView_SetExtendedListViewStyle(m_spi_nandflash_param_list.m_hWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT); 
	
	m_spi_nandflash_param_list.InsertColumn(0, _T("N.O."), LVCFMT_LEFT, 40);//序号
	m_spi_nandflash_param_list.InsertColumn(1, _T("ChipID"), LVCFMT_LEFT, 90);//chip id
	m_spi_nandflash_param_list.InsertColumn(2, _T("Page Size"), LVCFMT_LEFT, 90);//页大小
	m_spi_nandflash_param_list.InsertColumn(3, _T("Page per block"), LVCFMT_LEFT, 90);//每个块有多少个页
	m_spi_nandflash_param_list.InsertColumn(4, _T("Total block number"), LVCFMT_LEFT, 90);//块数
	//m_spi_nandflash_param_list.InsertColumn(5, _T("Group block number"), LVCFMT_LEFT, 90);//Group block number
	m_spi_nandflash_param_list.InsertColumn(5, _T("Plane block number"), LVCFMT_LEFT, 90);//Plane block number
	m_spi_nandflash_param_list.InsertColumn(6, _T("Spare size"), LVCFMT_LEFT, 90);//Spare size
	//m_spi_nandflash_param_list.InsertColumn(8, _T("Column address cycle"), LVCFMT_LEFT, 90);//Column address cycle
	//m_spi_nandflash_param_list.InsertColumn(9, _T("Last column addrress cycle mask bit"), LVCFMT_LEFT, 90);//Last column addrress cycle mask bit
	//m_spi_nandflash_param_list.InsertColumn(10, _T("Row address cycle"), LVCFMT_LEFT, 90);//Row address cycle
	//m_spi_nandflash_param_list.InsertColumn(11, _T("Last row address cycle mask bit"), LVCFMT_LEFT, 90);//Last row address cycle mask bit
	m_spi_nandflash_param_list.InsertColumn(7, _T("Custom nandflash"), LVCFMT_LEFT, 90);//Custom
	m_spi_nandflash_param_list.InsertColumn(8, _T("Flag"), LVCFMT_LEFT, 90);//Flag
	//m_spi_nandflash_param_list.InsertColumn(14, _T("Command length"), LVCFMT_LEFT, 90);//Command length
	m_spi_nandflash_param_list.InsertColumn(9, _T("clock"), LVCFMT_LEFT, 90);//Data length
	m_spi_nandflash_param_list.InsertColumn(10, _T("Descriptor string"), LVCFMT_LEFT, 130);//Descriptor
	
	set_config_item(theConfig);//附值
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
