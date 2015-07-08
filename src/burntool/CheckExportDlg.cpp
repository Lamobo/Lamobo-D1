// CheckExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "CheckExportDlg.h"
#include "Update.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCheckExportDlg dialog
extern CConfig theConfig;
extern CBurnToolApp theApp;

CCheckExportDlg::CCheckExportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCheckExportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCheckExportDlg)
		// NOTE: the ClassWizard will add member initialization here
		m_str_check_export = _T("");
	//}}AFX_DATA_INIT
}


void CCheckExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCheckExportDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Text(pDX, IDC_EDIT_CHECK_STRING, m_str_check_export);
	DDV_MaxChars(pDX, m_str_check_export, 10);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCheckExportDlg, CDialog)
	//{{AFX_MSG_MAP(CCheckExportDlg)
	ON_EN_CHANGE(IDC_EDIT_CHECK_STRING, OnChangeEditCheckString)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCheckExportDlg message handlers

void CCheckExportDlg::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData();
	
	TCHAR szFilter[] =	TEXT("Update Files(*.upd)|*.upd|") \
		TEXT("All Files (*.*)|*.*||") ;
	
	CFileDialog fd(FALSE, _T(".upd"), NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
        szFilter, NULL);
	
    if(IDOK == fd.DoModal())
    {
		CString strPath = fd.GetPathName();
        CUpdate update;
		
		HWND hDlg = CreateWindow(_T("BUTTON"), 
			theApp.GetString(IDS_MSG_WAIT), 
			WS_VISIBLE|BS_PUSHBUTTON, 
			GetSystemMetrics(SM_CXSCREEN)/2 - 100, 
			GetSystemMetrics(SM_CYSCREEN)/2 - 50 ,
			200, 
			NULL, 
			GetSafeHwnd(),
			NULL,
			AfxGetInstanceHandle(), 
			NULL);
		
        if(update.ExportUpdateFile(&theConfig, strPath, m_str_check_export))
        {
			::DestroyWindow(hDlg);
            AfxMessageBox(theApp.GetString(IDS_MSG_UPD_EXPORT_SUCCESS));
        }
        else
        {
			CFileFind fd_p;
			if(fd_p.FindFile(theApp.ConvertAbsolutePath(strPath)))
			{
				DeleteFile(theApp.ConvertAbsolutePath(strPath));
			}
			::DestroyWindow(hDlg);
            AfxMessageBox(theApp.GetString(IDS_MSG_UPD_EXPORT_FAIL));
        }
	}
	
	CDialog::OnOK();
}

void CCheckExportDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CCheckExportDlg::OnChangeEditCheckString() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

BOOL CCheckExportDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CString strTitle;
	CString str;
	
	str = theApp.GetString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);

	str = theApp.GetString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);

	strTitle =  theApp.GetString(IDS_CHECK_EXPORT_DIALOG);	
	SetWindowText(strTitle);	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
