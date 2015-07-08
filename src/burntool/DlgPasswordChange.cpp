// DlgPasswordChange.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "DlgPasswordChange.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString strrdid;
extern CBurnToolApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDlgPasswordChange dialog


CDlgPasswordChange::CDlgPasswordChange(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPasswordChange::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgPasswordChange)
	m_password_confirm = _T("");
	m_password_new = _T("");
	m_password_old = _T("");
	//}}AFX_DATA_INIT
}


void CDlgPasswordChange::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPasswordChange)
	DDX_Control(pDX, IDC_USER_ID_TYPE_LIST_CHG, m_combo_uid);
	DDX_Text(pDX, IDC_EDIT_CONFIRM_PASSWORD, m_password_confirm);
	DDV_MaxChars(pDX, m_password_confirm, 20);
	DDX_Text(pDX, IDC_EDIT_NEW_PASSWORD, m_password_new);
	DDV_MaxChars(pDX, m_password_new, 20);
	DDX_Text(pDX, IDC_EDIT_OLD_PASSWORD, m_password_old);
	DDV_MaxChars(pDX, m_password_old, 20);
	DDX_CBString(pDX, IDC_USER_ID_TYPE_LIST_CHG, m_user_id);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgPasswordChange, CDialog)
	//{{AFX_MSG_MAP(CDlgPasswordChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPasswordChange message handlers

void CDlgPasswordChange::OnOK() 
{
	// TODO: Add extra validation here
	GetDlgItemText(IDC_USER_ID_TYPE_LIST_CHG, strrdid);	

	CDialog::OnOK();
}

BOOL CDlgPasswordChange::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CString str;
	int count = m_combo_uid.GetCount();

	for(int i = 0; i < count; i++)
	{
		m_combo_uid.DeleteString(0);
	}

	m_combo_uid.AddString(theApp.GetString(IDS_USER_PRODUCER));//生产者
	m_combo_uid.AddString(theApp.GetString(IDS_USER_RESEACHER));//开发者

    str = theApp.GetString(IDS_PASSWORD_USER);
	GetDlgItem(IDC_STATIC_USER_ID_CHG)->SetWindowText(str);//

    str = theApp.GetString(IDS_PASSWORD_OLDWORD);
	GetDlgItem(IDC_STATIC_OLD_PASSWORD)->SetWindowText(str);//

    str = theApp.GetString(IDS_PASSWORD_NEWWORD);
	GetDlgItem(IDC_STATIC_NEW_PASSWORD)->SetWindowText(str);//

    str = theApp.GetString(IDS_PASSWORD_CONFIRM);
	GetDlgItem(IDC_STATIC_CONFIRM_PASSWORD)->SetWindowText(str);//
	
	str = theApp.GetString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);

	str = theApp.GetString(IDS_CANCEL);
	GetDlgItem(IDCANCEL)->SetWindowText(str);
	
	str =  theApp.GetString(IDS_PASSWORD_MODIFY_TITLE);	
	SetWindowText(str);

	m_combo_uid.SetCurSel(0);
	
	return TRUE;
}
