// BBDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "BBDialog.h"


// CBBDialog 对话框

IMPLEMENT_DYNAMIC(CBBDialog, CDialog)

CBBDialog::CBBDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBBDialog::IDD, pParent)
{
	m_bBBEnable = FALSE;
	ZeroMemory(&m_BackBalance, sizeof(BLACKBALANCE));
	m_BackBalance.type = ISP_CID_BLACK_BALANCE;
}

CBBDialog::~CBBDialog()
{
}

void CBBDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_BB_ENABLE, m_EnableCheck);
	DDX_Control(pDX, IDC_SLIDER_R_OFFSET, m_ROffsetSlider);
	DDX_Control(pDX, IDC_SLIDER_G_OFFSET, m_GOffsetSlider);
	DDX_Control(pDX, IDC_SLIDER_B_OFFSET, m_BOffsetSlider);
}


BEGIN_MESSAGE_MAP(CBBDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECK_BB_ENABLE, &CBBDialog::OnBnClickedCheckBbEnable)
	ON_WM_SHOWWINDOW()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON1, &CBBDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


// CBBDialog 消息处理程序

BOOL CBBDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_ROffsetSlider.SetRange(0, 1023, 0);
	m_GOffsetSlider.SetRange(0, 1023, 0);
	m_BOffsetSlider.SetRange(0, 1023, 0);

	return TRUE;
}

int CBBDialog::SetBBEnable(BOOL bEnable)
{
	m_bBBEnable = bEnable;
	return 0;
}

void CBBDialog::OnBnClickedCheckBbEnable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bBBEnable = m_EnableCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_BB, m_bBBEnable);
}

int CBBDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(BLACKBALANCE))) return -1;
	
	nStLen = sizeof(BLACKBALANCE);
	m_BackBalance.enable = m_bBBEnable;
	memcpy(pPageInfoSt, &m_BackBalance, nStLen);
	nPageID = m_nID;

	return 0;
}

int CBBDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(BLACKBALANCE))) return -1;

	memcpy(&m_BackBalance, pPageInfoSt, nStLen);

	if (m_BackBalance.type != ISP_CID_BLACK_BALANCE) return -1;

	m_bBBEnable = m_BackBalance.enable;
	m_EnableCheck.SetCheck(m_BackBalance.enable);

	OnBnClickedCheckBbEnable();

	m_ROffsetSlider.SetPos(m_BackBalance.r_offset);
	m_GOffsetSlider.SetPos(m_BackBalance.g_offset);
	m_BOffsetSlider.SetPos(m_BackBalance.b_offset);

	ProcessStaticText(&m_ROffsetSlider, IDC_STATIC_RVALUE);
	ProcessStaticText(&m_GOffsetSlider, IDC_STATIC_GVALUE);
	ProcessStaticText(&m_BOffsetSlider, IDC_STATIC_BVALUE);

	return 0;
}

int CBBDialog::Clear()
{
	ZeroMemory(&m_BackBalance, sizeof(BLACKBALANCE));
	m_bBBEnable = FALSE;
	m_BackBalance.type = ISP_CID_BLACK_BALANCE;

	m_EnableCheck.SetCheck(0);

	m_ROffsetSlider.SetPos(0);
	m_GOffsetSlider.SetPos(0);
	m_BOffsetSlider.SetPos(0);

	return 0;
}

void CBBDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if (bShow)
		m_EnableCheck.SetCheck(m_bBBEnable);
}

void CBBDialog::ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID)
{
	CWnd * pText = GetDlgItem(nStaticID);
	CString Val;
	Val.Format(L"[%d]", pSliderBar->GetPos());

	pText->SetWindowText(Val);
}

void CBBDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)pScrollBar;
	int iID = pSliderCtrl->GetDlgCtrlID();
	int iPos = pSliderCtrl->GetPos();

	if (IDC_SLIDER_R_OFFSET == iID) {
		m_BackBalance.r_offset = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_RVALUE);
	}else if (IDC_SLIDER_G_OFFSET == iID) {
		m_BackBalance.g_offset = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_GVALUE);
	}else if (IDC_SLIDER_B_OFFSET == iID) {
		m_BackBalance.b_offset = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_BVALUE);
	}

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CBBDialog::PreTranslateMessage(MSG * pMsg)
{
	// TODO: 在此添加控件通知处理程序代码
	if (pMsg->message == WM_KEYDOWN)
	{
		switch(pMsg->wParam) 
		{
		case VK_RETURN:
			return TRUE;
		case VK_ESCAPE:
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CBBDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_BB, 0);
}
