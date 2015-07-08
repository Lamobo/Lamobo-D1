// SaturationDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "SaturationDialog.h"


// CSaturationDialog 对话框

IMPLEMENT_DYNAMIC(CSaturationDialog, CDialog)

CSaturationDialog::CSaturationDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSaturationDialog::IDD, pParent)
{
	m_bSatEnable = FALSE;
	ZeroMemory(&m_Saturation, sizeof(SATURATION));
	m_Saturation.type = ISP_CID_SATURATION;
}

CSaturationDialog::~CSaturationDialog()
{
}

void CSaturationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_S_ENABLE, m_SaturationCheck);
	DDX_Control(pDX, IDC_SLIDER_KLOW, m_KlowSlider);
	DDX_Control(pDX, IDC_SLIDER_KHIGH, m_KhighSlider);
	DDX_Control(pDX, IDC_SLIDER_CHIGH, m_ChighSlider);
	DDX_Control(pDX, IDC_SLIDER_CLOW, m_ClowSlider);
}


BEGIN_MESSAGE_MAP(CSaturationDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECK_S_ENABLE, &CSaturationDialog::OnBnClickedCheckSEnable)
	ON_WM_SHOWWINDOW()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON1, &CSaturationDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


// CSaturationDialog 消息处理程序

BOOL CSaturationDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_KlowSlider.SetRange(0, 1023, 0);
	m_KhighSlider.SetRange(0, 1023, 0);
	m_ChighSlider.SetRange(0, 255, 0);
	m_ClowSlider.SetRange(0, 255, 0);

	return TRUE;
}

int CSaturationDialog::SetSaturationEnable(BOOL bEnable)
{
	m_bSatEnable = bEnable;
	return 0;
}

int CSaturationDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(SATURATION))) return -1;
	
	m_Saturation.enable = m_bSatEnable;
	nStLen = sizeof(SATURATION);
	int nSliderID = -1, nPos = 0;
	if (m_bSatEnable) {
		if (JudgeHighLow(nSliderID, nPos) == 1) return -1;
	}

	memcpy(pPageInfoSt, &m_Saturation, nStLen);
	nPageID = m_nID;

	return 0;
}

int CSaturationDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(SATURATION))) return -1;

	memcpy(&m_Saturation, pPageInfoSt, nStLen);

	if (m_Saturation.type != ISP_CID_SATURATION) return -1;

	m_bSatEnable = m_Saturation.enable;
	m_SaturationCheck.SetCheck(m_Saturation.enable);

	OnBnClickedCheckSEnable();

	m_KlowSlider.SetPos(m_Saturation.Klow);
	m_KhighSlider.SetPos(m_Saturation.Khigh);
	m_ChighSlider.SetPos(m_Saturation.Chigh);
	m_ClowSlider.SetPos(m_Saturation.Clow);

	ProcessStaticText(&m_KlowSlider, IDC_STATIC_KLOW_VAL);
	ProcessStaticText(&m_KhighSlider, IDC_STATIC_KHIGH_VAL);
	ProcessStaticText(&m_ChighSlider, IDC_STATIC_CHIGH_VAL);
	ProcessStaticText(&m_ClowSlider, IDC_STATIC_CLOW_VAL);

	return 0;
}

void CSaturationDialog::OnBnClickedCheckSEnable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSatEnable = m_SaturationCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_SATURATION, m_bSatEnable);
}

void CSaturationDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if (bShow)
		m_SaturationCheck.SetCheck(m_bSatEnable);
}

void CSaturationDialog::ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID)
{
	CWnd * pText = GetDlgItem(nStaticID);
	CString Val;
	Val.Format(L"[%d]", pSliderBar->GetPos());

	pText->SetWindowText(Val);
}

void CSaturationDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//if (nSBCode != SB_ENDSCROLL) {
		//CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
		//return;
	//}

	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)pScrollBar;
	int iID = pSliderCtrl->GetDlgCtrlID();
	int iPos = pSliderCtrl->GetPos();

again:
	if (IDC_SLIDER_KLOW == iID) {
		m_Saturation.Klow = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_KLOW_VAL);
	}else if (IDC_SLIDER_KHIGH == iID) {
		m_Saturation.Khigh = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_KHIGH_VAL);
	}else if (IDC_SLIDER_CHIGH == iID) {
		m_Saturation.Chigh = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_CHIGH_VAL);
	}else if (IDC_SLIDER_CLOW == iID) {
		m_Saturation.Clow = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_CLOW_VAL);
	}

	if (JudgeHighLow(iID, iPos) == 1) {
		(pSliderCtrl = ((CSliderCtrl *)GetDlgItem(iID)))->SetPos(iPos);
		goto again;
	}

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

int CSaturationDialog::Clear()
{
	ZeroMemory(&m_Saturation, sizeof(SATURATION));
	m_bSatEnable = FALSE;
	m_Saturation.type = ISP_CID_SATURATION;

	m_SaturationCheck.SetCheck(0);

	m_KlowSlider.SetPos(0);
	m_KhighSlider.SetPos(0);
	m_ChighSlider.SetPos(0);
	m_ClowSlider.SetPos(0);

	return 0;
}

BOOL CSaturationDialog::PreTranslateMessage(MSG * pMsg)
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

void CSaturationDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_SATURATION, 0);
}

int CSaturationDialog::JudgeHighLow(int & nSliderID, int & nPos)
{
	if (IDC_SLIDER_KLOW == nSliderID || IDC_SLIDER_KHIGH == nSliderID || -1 == nSliderID) {
		if (m_Saturation.Khigh <= m_Saturation.Klow) {
			AfxMessageBox(L"Khigh cannot be less than or equal to Klow!\n");
			if (m_Saturation.Klow > 1022) {
				m_Saturation.Klow -= 1;
				nSliderID = IDC_SLIDER_KLOW;
				nPos = m_Saturation.Klow;
				return 1;
			}else {
				m_Saturation.Khigh = m_Saturation.Klow + 1;
				nSliderID = IDC_SLIDER_KHIGH;
				nPos = m_Saturation.Khigh;
				return 1;
			}
		}
	}
	
	if (IDC_SLIDER_CLOW == nSliderID || IDC_SLIDER_CHIGH == nSliderID || -1 == nSliderID) {
		if (m_Saturation.Chigh <= m_Saturation.Clow) {
			AfxMessageBox(L"Chigh cannot be less than or equal to Clow!\n");
			if (m_Saturation.Clow > 254) {
				m_Saturation.Clow -= 1;
				nSliderID = IDC_SLIDER_CLOW;
				nPos = m_Saturation.Clow;
				return 1;
			}else {
				m_Saturation.Chigh = m_Saturation.Clow + 1;
				nSliderID = IDC_SLIDER_CHIGH;
				nPos = m_Saturation.Chigh;
				return 1;
			}
		}
	}
	
	return 0;
}

