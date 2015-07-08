// SpecEffDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "SpecEffDialog.h"


// CSpecEffDialog 对话框

IMPLEMENT_DYNAMIC(CSpecEffDialog, CDialog)

CSpecEffDialog::CSpecEffDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSpecEffDialog::IDD, pParent)
{
	m_bYUVEffEnable = m_bSolarEnable = FALSE;
	ZeroMemory(&m_SpecEff, sizeof(SPECIALEFF));
	m_SpecEff.type = ISP_CID_SPECIAL_EFFECT;
}

CSpecEffDialog::~CSpecEffDialog()
{
}

void CSpecEffDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_YUVE_ENABLE, m_EffectEnable);
	DDX_Control(pDX, IDC_CHECK_SOLAR_ENABLE, m_SolarEnable);
	DDX_Control(pDX, IDC_SLIDER_SOLAR_THRE, m_SolarThreSlider);
	DDX_Control(pDX, IDC_SLIDER_COEFA_THRE, m_YEffCoefaSlider);
	DDX_Control(pDX, IDC_SLIDER_COEFB_THRE, m_YEffCoefbSlider);
	DDX_Control(pDX, IDC_SLIDER_UCOEFA_THRE, m_UEffCoefaSlider);
	DDX_Control(pDX, IDC_SLIDER_UCOEFB_THRE, m_UEffCoefbSlider);
	DDX_Control(pDX, IDC_SLIDER_VCOEFA_THRE, m_VEffCoefaSlider);
	DDX_Control(pDX, IDC_SLIDER_VCOEFB_THRE, m_VEffCoefbSlider);
}


BEGIN_MESSAGE_MAP(CSpecEffDialog, CDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_CHECK_YUVE_ENABLE, &CSpecEffDialog::OnBnClickedCheckYuveEnable)
	ON_BN_CLICKED(IDC_CHECK_SOLAR_ENABLE, &CSpecEffDialog::OnBnClickedCheckSolarEnable)
	ON_BN_CLICKED(IDC_BUTTON1, &CSpecEffDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


// CSpecEffDialog 消息处理程序

BOOL CSpecEffDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_SolarThreSlider.SetRange(0, 255, 0);
	m_YEffCoefaSlider.SetRange(-64, 64, 0);
	m_YEffCoefaSlider.SetPos(-64);
	m_SpecEff.y_eff_coefa = -64;

	m_YEffCoefbSlider.SetRange(0, 255, 0);
	m_UEffCoefaSlider.SetRange(-64, 64, 0);
	m_UEffCoefaSlider.SetPos(-64);
	m_SpecEff.u_eff_coefa = -64;

	m_UEffCoefbSlider.SetRange(0, 255, 0);
	m_VEffCoefaSlider.SetRange(-64, 64, 0);
	m_VEffCoefaSlider.SetPos(-64);
	m_SpecEff.v_eff_coefa = -64;

	m_VEffCoefbSlider.SetRange(0, 255, 0);

	return TRUE;
}

int CSpecEffDialog::SetSpecEffEnable(BOOL bEnable)
{
	m_bYUVEffEnable = m_bSolarEnable = bEnable;
	return 0;
}

int CSpecEffDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(SPECIALEFF))) return -1;
	
	nStLen = sizeof(SPECIALEFF);
	m_SpecEff.enable = m_bYUVEffEnable;
	m_SpecEff.solar_enable = m_bSolarEnable;
	memcpy(pPageInfoSt, &m_SpecEff, nStLen);
	nPageID = m_nID;

	return 0;
}

int CSpecEffDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(SPECIALEFF))) return -1;

	memcpy(&m_SpecEff, pPageInfoSt, nStLen);

	if (m_SpecEff.type != ISP_CID_SPECIAL_EFFECT) return -1;

	m_bYUVEffEnable = m_SpecEff.enable;
	m_bSolarEnable = m_SpecEff.solar_enable;
	m_EffectEnable.SetCheck(m_SpecEff.enable);
	m_SolarEnable.SetCheck(m_SpecEff.solar_enable);

	OnBnClickedCheckYuveEnable();
	OnBnClickedCheckSolarEnable();

	m_SolarThreSlider.SetPos(m_SpecEff.solar_thrs);
	m_YEffCoefaSlider.SetPos(m_SpecEff.y_eff_coefa);
	m_YEffCoefbSlider.SetPos(m_SpecEff.y_eff_coefb);
	m_UEffCoefaSlider.SetPos(m_SpecEff.u_eff_coefa);
	m_UEffCoefbSlider.SetPos(m_SpecEff.u_eff_coefb);
	m_VEffCoefaSlider.SetPos(m_SpecEff.v_eff_coefa);
	m_VEffCoefbSlider.SetPos(m_SpecEff.y_eff_coefb);

	ProcessStaticText(&m_SolarThreSlider, IDC_STATIC_SOLAR_VAL);
	ProcessStaticText(&m_YEffCoefaSlider, IDC_STATIC_COEFA_VAL);
	ProcessStaticText(&m_YEffCoefbSlider, IDC_STATIC_COEFB_VAL);
	ProcessStaticText(&m_UEffCoefaSlider, IDC_STATIC_UCOEFA_VAL);
	ProcessStaticText(&m_UEffCoefbSlider, IDC_STATIC_UCOEFB_VAL);
	ProcessStaticText(&m_VEffCoefaSlider, IDC_STATIC_VCOEFA_VAL);
	ProcessStaticText(&m_VEffCoefbSlider, IDC_STATIC_VCOEFB_VAL);

	return 0;
}

void CSpecEffDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if (bShow) {
		m_EffectEnable.SetCheck(m_bYUVEffEnable);
		m_SolarEnable.SetCheck(m_bSolarEnable);
	}
}

void CSpecEffDialog::ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID)
{
	CWnd * pText = GetDlgItem(nStaticID);
	CString Val;
	Val.Format(L"[%d]", pSliderBar->GetPos());

	pText->SetWindowText(Val);
}

void CSpecEffDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)pScrollBar;
	int iID = pSliderCtrl->GetDlgCtrlID();
	int iPos = pSliderCtrl->GetPos();

	if (IDC_SLIDER_SOLAR_THRE == iID) {
		m_SpecEff.solar_thrs = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_SOLAR_VAL);
	}else if (IDC_SLIDER_COEFA_THRE == iID) {
		m_SpecEff.y_eff_coefa = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_COEFA_VAL);
	}else if (IDC_SLIDER_COEFB_THRE == iID) {
		m_SpecEff.y_eff_coefb = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_COEFB_VAL);
	}else if (IDC_SLIDER_UCOEFA_THRE == iID) {
		m_SpecEff.u_eff_coefa = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_UCOEFA_VAL);
	}else if (IDC_SLIDER_UCOEFB_THRE == iID) {
		m_SpecEff.u_eff_coefb = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_UCOEFB_VAL);
	}else if (IDC_SLIDER_VCOEFA_THRE == iID) {
		m_SpecEff.v_eff_coefa = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_VCOEFA_VAL);
	}else if (IDC_SLIDER_VCOEFB_THRE == iID) {
		m_SpecEff.v_eff_coefb = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_VCOEFB_VAL);
	}

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSpecEffDialog::OnBnClickedCheckYuveEnable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bYUVEffEnable = m_EffectEnable.GetCheck();

	if (NULL == m_pMessageWnd) return;

	if (m_bYUVEffEnable)
		CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_SPECEFFEICT, TRUE);
	else if (!m_bSolarEnable)
		CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_SPECEFFEICT, FALSE);
}

void CSpecEffDialog::OnBnClickedCheckSolarEnable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bSolarEnable = m_SolarEnable.GetCheck();

	if (NULL == m_pMessageWnd) return;

	if (m_bSolarEnable)
		CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_SPECEFFEICT, TRUE);
	else if (!m_bYUVEffEnable)
		CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_SPECEFFEICT, FALSE);
}

int CSpecEffDialog::Clear()
{
	ZeroMemory(&m_SpecEff, sizeof(SPECIALEFF));
	m_bYUVEffEnable = m_bSolarEnable = FALSE;
	m_SpecEff.type = ISP_CID_SPECIAL_EFFECT;

	m_EffectEnable.SetCheck(0);
	m_SolarEnable.SetCheck(0);
	
	m_SpecEff.y_eff_coefa = -64;
	m_SpecEff.u_eff_coefa = -64;
	m_SpecEff.v_eff_coefa = -64;

	m_SolarThreSlider.SetPos(0);
	m_YEffCoefaSlider.SetPos(-64);
	m_YEffCoefbSlider.SetPos(0);
	m_UEffCoefaSlider.SetPos(-64);
	m_UEffCoefbSlider.SetPos(0);
	m_VEffCoefaSlider.SetPos(-64);
	m_VEffCoefbSlider.SetPos(0);

	return 0;
}

BOOL CSpecEffDialog::PreTranslateMessage(MSG * pMsg)
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

void CSpecEffDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_SPECEFFEICT, 0);
}
