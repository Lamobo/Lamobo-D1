// WBDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "WBDialog.h"


// CWBDialog 对话框

IMPLEMENT_DYNAMIC(CWBDialog, CDialog)

CWBDialog::CWBDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CWBDialog::IDD, pParent)
{
	m_bWBEnable = FALSE;
	ZeroMemory(&m_WBalance, sizeof(WBALANCE));
	m_WBalance.type = ISP_CID_WHITE_BALANCE;
}

CWBDialog::~CWBDialog()
{
}

void CWBDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_WB, m_WBCheck);
	DDX_Control(pDX, IDC_SLIDER_WBR, m_WBRSlider);
	DDX_Control(pDX, IDC_SLIDER_WBG, m_WBGSlider);
	DDX_Control(pDX, IDC_SLIDER_WBB, m_WBBSlider);
}


BEGIN_MESSAGE_MAP(CWBDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECK_WB, &CWBDialog::OnBnClickedCheckWb)
	ON_WM_SHOWWINDOW()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON1, &CWBDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


// CWBDialog 消息处理程序

BOOL CWBDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_WBRSlider.SetRange(0, 2560, 0);
	m_WBGSlider.SetRange(0, 2560, 0);
	m_WBBSlider.SetRange(0, 2560, 0);

	return TRUE;
}

int CWBDialog::SetWBEnable(BOOL bEnable)
{
	m_bWBEnable = bEnable;
	return 0;
}

int CWBDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(WBALANCE))) return -1;
	
	nStLen = sizeof(WBALANCE);
	m_WBalance.enable = m_bWBEnable;
	memcpy(pPageInfoSt, &m_WBalance, nStLen);
	nPageID = m_nID;

	return 0;
}

int CWBDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(WBALANCE))) return -1;

	memcpy(&m_WBalance, pPageInfoSt, nStLen);

	if (m_WBalance.type != ISP_CID_WHITE_BALANCE) return -1;

	m_bWBEnable = m_WBalance.enable;
	m_WBCheck.SetCheck(m_WBalance.enable);

	OnBnClickedCheckWb();

	m_WBRSlider.SetPos(m_WBalance.co_r);
	m_WBGSlider.SetPos(m_WBalance.co_g);
	m_WBBSlider.SetPos(m_WBalance.co_b);

	ProcessStaticText(&m_WBRSlider, IDC_STATIC_RVAL);
	ProcessStaticText(&m_WBGSlider, IDC_STATIC_GVAL);
	ProcessStaticText(&m_WBBSlider, IDC_STATIC_BVAL);

	return 0;
}

void CWBDialog::OnBnClickedCheckWb()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bWBEnable = m_WBCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_WB, m_bWBEnable);
}

void CWBDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if (bShow)
		m_WBCheck.SetCheck(m_bWBEnable);
}

void CWBDialog::ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID)
{
	CWnd * pText = GetDlgItem(nStaticID);
	CString Val;
	Val.Format(L"[%d]", pSliderBar->GetPos());

	pText->SetWindowText(Val);
}

void CWBDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)pScrollBar;
	int iID = pSliderCtrl->GetDlgCtrlID();
	int iPos = pSliderCtrl->GetPos();

	if (IDC_SLIDER_WBR == iID) {
		m_WBalance.co_r = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_RVAL);
	}else if (IDC_SLIDER_WBG == iID) {
		m_WBalance.co_g = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_GVAL);
	}else if (IDC_SLIDER_WBB == iID) {
		m_WBalance.co_b = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_BVAL);
	}

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

int CWBDialog::Clear()
{
	ZeroMemory(&m_WBalance, sizeof(WBALANCE));
	m_bWBEnable = FALSE;
	m_WBalance.type = ISP_CID_WHITE_BALANCE;

	m_WBCheck.SetCheck(0);

	m_WBRSlider.SetPos(0);
	m_WBGSlider.SetPos(0);
	m_WBBSlider.SetPos(0);

	return 0;
}

BOOL CWBDialog::PreTranslateMessage(MSG * pMsg)
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

void CWBDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_WB, 0);
}
