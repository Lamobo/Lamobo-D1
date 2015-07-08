// PageNandflash.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "PageNandflash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CConfig theConfig;
extern CBurnToolApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPageNandflash property page

IMPLEMENT_DYNCREATE(CPageNandflash, CPropertyPage)

CPageNandflash::CPageNandflash() : CPropertyPage(CPageNandflash::IDD)
{
	//{{AFX_DATA_INIT(CPageNandflash)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPageNandflash::~CPageNandflash()
{
}

void CPageNandflash::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageNandflash)
	DDX_Control(pDX, IDC_LIST_NANDFLASH_PARAM, m_nandflash_param_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageNandflash, CPropertyPage)
	//{{AFX_MSG_MAP(CPageNandflash)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageNandflash message handlers
BOOL CPageNandflash::get_config_data(CConfig &config)
{
	CString str;
	int count;
	
	USES_CONVERSION;
	count= m_nandflash_param_list.GetItemCount();
	config.nandflash_parameter_count = count;

	if(config.nandflash_parameter != NULL)
	{
		free(config.nandflash_parameter);
	}

	config.nandflash_parameter = (T_NAND_PHY_INFO_TRANSC* )malloc(sizeof(T_NAND_PHY_INFO_TRANSC) * config.nandflash_parameter_count);
	
	memset(config.nandflash_parameter, 0, sizeof(T_NAND_PHY_INFO_TRANSC) * config.nandflash_parameter_count);
	
	for(int i = 0; i < count; i++)
	{
		str = m_nandflash_param_list.GetItemText(i, 1);
		config.nandflash_parameter[i].chip_id = config.hex2int(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 2);
		config.nandflash_parameter[i].page_size = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 3);
		config.nandflash_parameter[i].page_per_blk = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 4);
		config.nandflash_parameter[i].blk_num = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 5);
		config.nandflash_parameter[i].group_blk_num = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 6);
		config.nandflash_parameter[i].plane_blk_num = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 7);
		config.nandflash_parameter[i].spare_size = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 8);
		config.nandflash_parameter[i].col_cycle = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 9);
		config.nandflash_parameter[i].lst_col_mask = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 10);
		config.nandflash_parameter[i].row_cycle = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 11);
		config.nandflash_parameter[i].last_row_mask = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 12);
		config.nandflash_parameter[i].custom_nd = atoi(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 13);
		config.nandflash_parameter[i].flag = config.hex2int(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 14);
		config.nandflash_parameter[i].cmd_len = config.hex2int(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 15);
		config.nandflash_parameter[i].data_len = config.hex2int(T2A(str));

		str = m_nandflash_param_list.GetItemText(i, 16);
		strncpy(config.nandflash_parameter[i].des_str, T2A(str), 32);
		
	}

	return TRUE;
}

BOOL CPageNandflash::set_config_item(CConfig &config)
{
	CString strIndex;
	CString str;

	USES_CONVERSION;
	for(UINT i = 0; i < config.nandflash_parameter_count; i++)
	{		
		strIndex.Format(_T("%d"), i+1);
		m_nandflash_param_list.InsertItem(i, strIndex);

		str.Format(_T("0x%x"), config.nandflash_parameter[i].chip_id);//chip_id
		m_nandflash_param_list.SetItemText(i, 1, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].page_size);//page_size
		m_nandflash_param_list.SetItemText(i, 2, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].page_per_blk);//page_per_blk
		m_nandflash_param_list.SetItemText(i, 3, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].blk_num);//blk_num
		m_nandflash_param_list.SetItemText(i, 4, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].group_blk_num);//group_blk_num
		m_nandflash_param_list.SetItemText(i, 5, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].plane_blk_num);//plane_blk_num
		m_nandflash_param_list.SetItemText(i, 6, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].spare_size);//spare_size
		m_nandflash_param_list.SetItemText(i, 7, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].col_cycle);//col_cycle
		m_nandflash_param_list.SetItemText(i, 8, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].lst_col_mask);//lst_col_mask
		m_nandflash_param_list.SetItemText(i, 9, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].row_cycle);//row_cycle
		m_nandflash_param_list.SetItemText(i, 10, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].last_row_mask);//last_row_mask
		m_nandflash_param_list.SetItemText(i, 11, str);

		str.Format(_T("%d"), config.nandflash_parameter[i].custom_nd);//custom_nd
		m_nandflash_param_list.SetItemText(i, 12, str);

		str.Format(_T("0x%.8X"), config.nandflash_parameter[i].flag);//flag
		m_nandflash_param_list.SetItemText(i, 13, str);
		
		str.Format(_T("0x%.8X"), config.nandflash_parameter[i].cmd_len);//cmd_len
		m_nandflash_param_list.SetItemText(i, 14, str);

		str.Format(_T("0x%.8X"), config.nandflash_parameter[i].data_len);//data_len
		m_nandflash_param_list.SetItemText(i, 15, str);

		m_nandflash_param_list.SetItemText(i, 16, A2T(config.nandflash_parameter[i].des_str));//des_str
		
	}
	
	return TRUE;
}

BOOL CPageNandflash::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	SetupDisplay();
	
	ListView_SetExtendedListViewStyle(m_nandflash_param_list.m_hWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT); 
	
	m_nandflash_param_list.InsertColumn(0, _T("N.O."), LVCFMT_LEFT, 40);//序号
	m_nandflash_param_list.InsertColumn(1, _T("ChipID"), LVCFMT_LEFT, 90);//chip id
	m_nandflash_param_list.InsertColumn(2, _T("Page Size"), LVCFMT_LEFT, 90);//页大小
	m_nandflash_param_list.InsertColumn(3, _T("Page per block"), LVCFMT_LEFT, 90);//每个块有多少个页
	m_nandflash_param_list.InsertColumn(4, _T("Total block number"), LVCFMT_LEFT, 90);//块数
	m_nandflash_param_list.InsertColumn(5, _T("Group block number"), LVCFMT_LEFT, 90);//Group block number
	m_nandflash_param_list.InsertColumn(6, _T("Plane block number"), LVCFMT_LEFT, 90);//Plane block number
	m_nandflash_param_list.InsertColumn(7, _T("Spare size"), LVCFMT_LEFT, 90);//Spare size
	m_nandflash_param_list.InsertColumn(8, _T("Column address cycle"), LVCFMT_LEFT, 90);//Column address cycle
	m_nandflash_param_list.InsertColumn(9, _T("Last column addrress cycle mask bit"), LVCFMT_LEFT, 90);//Last column addrress cycle mask bit
	m_nandflash_param_list.InsertColumn(10, _T("Row address cycle"), LVCFMT_LEFT, 90);//Row address cycle
	m_nandflash_param_list.InsertColumn(11, _T("Last row address cycle mask bit"), LVCFMT_LEFT, 90);//Last row address cycle mask bit
	m_nandflash_param_list.InsertColumn(12, _T("Custom nandflash"), LVCFMT_LEFT, 90);//Custom
	m_nandflash_param_list.InsertColumn(13, _T("Flag"), LVCFMT_LEFT, 90);//Flag
	m_nandflash_param_list.InsertColumn(14, _T("Command length"), LVCFMT_LEFT, 90);//Command length
	m_nandflash_param_list.InsertColumn(15, _T("Data length"), LVCFMT_LEFT, 90);//Data length
	m_nandflash_param_list.InsertColumn(16, _T("Descriptor string"), LVCFMT_LEFT, 130);//Descriptor

	set_config_item(theConfig);//附值

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CPageNandflash::SetupDisplay()
{
	CString str;

	str = theApp.GetString(IDS_NAND_LIST);
	GetDlgItem(IDC_STATIC_NAND_LIST)->SetWindowText(str);//nand列表
}
