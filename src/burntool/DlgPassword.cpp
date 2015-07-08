// DlgPassword.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "DlgPassword.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString strpmid;
extern CBurnToolApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDlgPassword dialog


CDlgPassword::CDlgPassword(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPassword::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgPassword)
	m_password = _T("");
	m_user_id = _T("");
	//}}AFX_DATA_INIT
}


void CDlgPassword::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgPassword)
	DDX_Control(pDX, IDC_USER_ID_TYPE_LIST, m_combo_uid);
	DDX_Text(pDX, IDC_EDIT_ENTER_PASSWORD, m_password);
	DDV_MaxChars(pDX, m_password, 20);
	DDX_CBString(pDX, IDC_USER_ID_TYPE_LIST, m_user_id);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgPassword, CDialog)
	//{{AFX_MSG_MAP(CDlgPassword)
	ON_BN_CLICKED(IDC_BTN_PASSWD_CANCEL, OnBtnPasswdCancel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgPassword message handlers

void CDlgPassword::OnOK() 
{
	// TODO: Add extra validation here		
	GetDlgItemText(IDC_USER_ID_TYPE_LIST, strpmid);	
	
	CDialog::OnOK();
}

void CDlgPassword::OnBtnPasswdCancel() 
{
	// TODO: Add your control notification handler code here
	CDialog::OnCancel();
}

BOOL CDlgPassword::OnInitDialog() 
{
	CDialog::OnInitDialog();
    CString str;
	
	int count = m_combo_uid.GetCount();

	for(int i = 0; i < count; i++)
	{
		m_combo_uid.DeleteString(0);//
	}

	m_combo_uid.AddString(theApp.GetString(IDS_USER_PRODUCER));//生产者
	m_combo_uid.AddString(theApp.GetString(IDS_USER_RESEACHER));//开发者

    str = theApp.GetString(IDS_PASSWORD_TIP);
	GetDlgItem(IDC_STATIC_PASSWORD_TIP)->SetWindowText(str);//

    str = theApp.GetString(IDS_PASSWORD_USER_NAME);
	GetDlgItem(IDC_STATIC_USER_ID)->SetWindowText(str);//用户名

    str = theApp.GetString(IDS_PASSWORD_WORD);
	GetDlgItem(IDC_STATIC_ENTER_PASSWORD)->SetWindowText(str);//密码
	
	str = theApp.GetString(IDS_OK);
	GetDlgItem(IDOK)->SetWindowText(str);//确定

	str = theApp.GetString(IDS_CANCEL);
	GetDlgItem(IDC_BTN_PASSWD_CANCEL)->SetWindowText(str);//取消

	m_combo_uid.SetCurSel(0);

	return TRUE;
}
