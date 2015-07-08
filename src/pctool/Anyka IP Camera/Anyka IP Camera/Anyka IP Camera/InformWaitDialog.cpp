// InformWaitDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "Anyka IP Camera.h"
#include "InformWaitDialog.h"


// CInformWaitDialog 对话框

IMPLEMENT_DYNAMIC(CInformWaitDialog, CDialog)

CInformWaitDialog::CInformWaitDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CInformWaitDialog::IDD, pParent)
{

}

CInformWaitDialog::~CInformWaitDialog()
{
}

void CInformWaitDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInformWaitDialog, CDialog)
END_MESSAGE_MAP()


// CInformWaitDialog 消息处理程序
