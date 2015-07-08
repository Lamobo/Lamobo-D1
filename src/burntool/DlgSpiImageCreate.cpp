// DlgSpiImageCreate.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "DlgSpiImageCreate.h"
#include "SpiImageCreate.h"
#include "SpiImageCreate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CConfig theConfig;
extern CBurnToolApp theApp;
/////////////////////////////////////////////////////////////////////////////
// DlgSpiImageCreate dialog


DlgSpiImageCreate::DlgSpiImageCreate(CWnd* pParent /*=NULL*/)
	: CDialog(DlgSpiImageCreate::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgSpiImageCreate)
		// NOTE: the ClassWizard will add member initialization here
	m_spi_id = _T("");
	//}}AFX_DATA_INIT
}


void DlgSpiImageCreate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgSpiImageCreate)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_COMBO_SPL_TYPE_LIST, m_combo_spi);
	DDX_CBString(pDX, IDC_COMBO_SPL_TYPE_LIST, m_spi_id);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgSpiImageCreate, CDialog)
	//{{AFX_MSG_MAP(DlgSpiImageCreate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgSpiImageCreate message handlers

void DlgSpiImageCreate::OnOK() 
{
	// TODO: Add extra validation here
	CString spiName;
	
	//获取选择的spi 型号
	GetDlgItemText(IDC_COMBO_SPL_TYPE_LIST, spiName);

	TCHAR szFilter[] =	TEXT("BIN Files(*.bin)|*.bin|") \
		TEXT("All Files (*.*)|*.*||") ;
	
	CFileDialog fd(FALSE, _T(".bin"), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
        szFilter, NULL);

	if(IDOK == fd.DoModal())
	{
		//获取 SPI 镜像 保存的路径(含文件名)
		CString strPath = fd.GetPathName();
		
		SpiImageCreate spiimagecreate;
		spiimagecreate.Spi_Enter(spiName, strPath);
	}

	CDialog::OnOK();
}

BOOL DlgSpiImageCreate::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString str;

    str = theApp.GetString(IDS_STATIC_SPI_CHOOSE);
	GetDlgItem(IDC_STATIC_SPI_CHOOSE)->SetWindowText(str);

    str = theApp.GetString(IDS_STATIC_SPINAME);
	GetDlgItem(IDC_STATIC_SPINAME)->SetWindowText(str);
	
	str = theApp.GetString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);

	str = theApp.GetString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);

	str = theApp.GetString(IDS_SPI_IMAGE_TITLE);	
	SetWindowText(str);

	USES_CONVERSION;
	
	// 把参数文件的spi 型号加到 Combo Box 中
	for(UINT i = 0; i < theConfig.spiflash_parameter_count; i++)
	{
		m_combo_spi.AddString(A2T(theConfig.spiflash_parameter[i].des_str));
	}

	m_combo_spi.SetCurSel(0);

	return TRUE;
}

