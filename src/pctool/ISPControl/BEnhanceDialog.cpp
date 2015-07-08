// BEnhanceDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "BEnhanceDialog.h"


// CBEnhanceDialog 对话框

IMPLEMENT_DYNAMIC(CBEnhanceDialog, CDialog)

CBEnhanceDialog::CBEnhanceDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CBEnhanceDialog::IDD, pParent)
{
	m_bBEEnable = FALSE;
	ZeroMemory(&m_BriEnhance, sizeof(BRIENHANCE));
	m_BriEnhance.type = ISP_CID_BRIGHTNESS_ENHANCE;
}

CBEnhanceDialog::~CBEnhanceDialog()
{
}

void CBEnhanceDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_BRI_ENABLE, m_BEnhCheck);
	DDX_Control(pDX, IDC_SLIDER_THRE, m_ThreSlider);
	DDX_Control(pDX, IDC_SLIDER_KREG, m_KregSlider);
	DDX_Control(pDX, IDC_SLIDER_GAIN, m_GainSlider);
}


BEGIN_MESSAGE_MAP(CBEnhanceDialog, CDialog)
	ON_WM_HSCROLL()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_CHECK_BRI_ENABLE, &CBEnhanceDialog::OnBnClickedCheckBriEnable)
	ON_BN_CLICKED(IDC_BUTTON1, &CBEnhanceDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


// CBEnhanceDialog 消息处理程序

BOOL CBEnhanceDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_ThreSlider.SetRange(0, 2048, 0);
	m_KregSlider.SetRange(0, 2048, 0);
	m_GainSlider.SetRange(-63, 63, 0);
	m_GainSlider.SetPos(-63);

	m_BriEnhance.ygain = -63;

	return TRUE;
}

int CBEnhanceDialog::SetBEnhanceEnable(BOOL bEnable)
{
	m_bBEEnable = bEnable;
	return 0;
}

int CBEnhanceDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(BRIENHANCE))) return -1;
	
	nStLen = sizeof(BRIENHANCE);
	m_BriEnhance.enable = m_bBEEnable;
	memcpy(pPageInfoSt, &m_BriEnhance, nStLen);
	nPageID = m_nID;

	return 0;
}

int CBEnhanceDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(BRIENHANCE))) return -1;

	memcpy(&m_BriEnhance, pPageInfoSt, nStLen);

	if (m_BriEnhance.type != ISP_CID_BRIGHTNESS_ENHANCE) return -1;

	m_bBEEnable = m_BriEnhance.enable;
	m_BEnhCheck.SetCheck(m_BriEnhance.enable);

	OnBnClickedCheckBriEnable();

	m_ThreSlider.SetPos(m_BriEnhance.y_thrs);
	m_KregSlider.SetPos(m_BriEnhance.y_edgek);
	m_GainSlider.SetPos(m_BriEnhance.ygain);

	ProcessStaticText(&m_ThreSlider, IDC_STATIC_THRE_VAL);
	ProcessStaticText(&m_KregSlider, IDC_STATIC_KREG_VAL);
	ProcessStaticText(&m_GainSlider, IDC_STATIC_GAIN_VAL);

	return 0;
}

void CBEnhanceDialog::ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID)
{
	CWnd * pText = GetDlgItem(nStaticID);
	CString Val;
	Val.Format(L"[%d]", pSliderBar->GetPos());

	pText->SetWindowText(Val);
}

void CBEnhanceDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)pScrollBar;
	int iID = pSliderCtrl->GetDlgCtrlID();
	int iPos = pSliderCtrl->GetPos();

	if (IDC_SLIDER_THRE == iID) {
		m_BriEnhance.y_thrs = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_THRE_VAL);
	}else if (IDC_SLIDER_KREG == iID) {
		m_BriEnhance.y_edgek = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_KREG_VAL);
	}else if (IDC_SLIDER_GAIN == iID) {
		m_BriEnhance.ygain = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_GAIN_VAL);
	}

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CBEnhanceDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if (bShow)
		m_BEnhCheck.SetCheck(m_bBEEnable);
}

void CBEnhanceDialog::OnBnClickedCheckBriEnable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bBEEnable = m_BEnhCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_BENHANCE, m_bBEEnable);
}

int CBEnhanceDialog::Clear()
{
	ZeroMemory(&m_BriEnhance, sizeof(BRIENHANCE));
	m_bBEEnable = FALSE;
	m_BriEnhance.type = ISP_CID_BRIGHTNESS_ENHANCE;

	m_BEnhCheck.SetCheck(0);

	m_ThreSlider.SetPos(0);
	m_KregSlider.SetPos(0);
	m_GainSlider.SetPos(-63);

	m_BriEnhance.ygain = -63;

	return 0;
}

BOOL CBEnhanceDialog::PreTranslateMessage(MSG * pMsg)
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

void CBEnhanceDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_BENHANCE, 0);
}
