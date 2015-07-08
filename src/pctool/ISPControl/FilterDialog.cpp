// FilterDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "FilterDialog.h"


// CFilterDialog 对话框

IMPLEMENT_DYNAMIC(CFilterDialog, CDialog)

CFilterDialog::CFilterDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFilterDialog::IDD, pParent)
{
	m_bRGBFEnable = m_bDFPEnable = m_bUVFEnable = FALSE;
	ZeroMemory(&m_RGBFilter, sizeof(RGBFILTER));
	ZeroMemory(&m_UVFilter, sizeof(UVFILTER));
	ZeroMemory(&m_DFPDefect, sizeof(DFPDEFECT));

	m_RGBFilter.type = ISP_CID_RGB_FILTER;
	m_UVFilter.type = ISP_CID_UV_FILTER;
	m_DFPDefect.type = ISP_CID_DEFECT_PIXEL;
}

CFilterDialog::~CFilterDialog()
{
}

void CFilterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_RGBENABLE, m_RGBFCheck);
	DDX_Control(pDX, IDC_CHECK_DFPENABLE, m_DfpCheck);
	DDX_Control(pDX, IDC_CHECK_UVENABLE, m_UVFCheck);
	DDX_Control(pDX, IDC_SLIDER_RGBFILTER, m_RGBFSlider);
	DDX_Control(pDX, IDC_SLIDER_DFP, m_DfpSlider);
}


BEGIN_MESSAGE_MAP(CFilterDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECK_RGBENABLE, &CFilterDialog::OnBnClickedCheckRgbenable)
	ON_BN_CLICKED(IDC_CHECK_DFPENABLE, &CFilterDialog::OnBnClickedCheckDfpenable)
	ON_BN_CLICKED(IDC_CHECK_UVENABLE, &CFilterDialog::OnBnClickedCheckUvenable)
	ON_WM_SHOWWINDOW()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON1, &CFilterDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CFilterDialog::OnBnClickedButtonDfp)
	ON_BN_CLICKED(IDC_BUTTON3, &CFilterDialog::OnBnClickedButtonRGB)
END_MESSAGE_MAP()


// CFilterDialog 消息处理程序

BOOL CFilterDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_RGBFSlider.SetRange(0, 1023, 0);
	m_DfpSlider.SetRange(0, 3, 0);

	return TRUE;
}

int CFilterDialog::SetRGBFEnable(BOOL bEnable)
{
	m_bRGBFEnable = bEnable;
	return 0;
}

int CFilterDialog::SetUVFEnable(BOOL bEnable)
{
	m_bUVFEnable = bEnable;
	return 0;
}

int CFilterDialog::SetDFPEnable(BOOL bEnable)
{
	m_bDFPEnable = bEnable;
	return 0;
}

int CFilterDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < (sizeof(RGBFILTER) + sizeof(UVFILTER) + sizeof(DFPDEFECT)))) return -1;
	
	nStLen = sizeof(RGBFILTER) + sizeof(UVFILTER) + sizeof(DFPDEFECT);
	
	m_RGBFilter.enable = m_bRGBFEnable;
	m_UVFilter.enable = m_bUVFEnable;
	m_DFPDefect.enable = m_bDFPEnable;

	BYTE * pos = (BYTE *)pPageInfoSt;
	memcpy(pos, &m_RGBFilter, sizeof(RGBFILTER));
	pos += sizeof(RGBFILTER);
	memcpy(pos, &m_UVFilter, sizeof(UVFILTER));
	pos += sizeof(UVFILTER);
	memcpy(pos, &m_DFPDefect, sizeof(DFPDEFECT));
	nPageID = m_nID;

	return 0;
}

int CFilterDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != (sizeof(RGBFILTER) + sizeof(UVFILTER) + sizeof(DFPDEFECT)))) return -1;
	
	BYTE * pos = (BYTE *)pPageInfoSt;
	memcpy(&m_RGBFilter, pos, sizeof(RGBFILTER));
	pos += sizeof(RGBFILTER);
	memcpy(&m_UVFilter, pos, sizeof(UVFILTER));
	pos += sizeof(UVFILTER);
	memcpy(&m_DFPDefect, pos, sizeof(DFPDEFECT));

	if ((m_RGBFilter.type != ISP_CID_RGB_FILTER) || 
		(m_UVFilter.type != ISP_CID_UV_FILTER) || 
		(m_DFPDefect.type != ISP_CID_DEFECT_PIXEL)) return -1;

	m_bRGBFEnable = m_RGBFilter.enable;
	m_bDFPEnable = m_DFPDefect.enable;
	m_bUVFEnable = m_UVFilter.enable;

	m_RGBFCheck.SetCheck(m_RGBFilter.enable);
	m_DfpCheck.SetCheck(m_DFPDefect.enable);
	m_UVFCheck.SetCheck(m_UVFilter.enable);

	OnBnClickedCheckRgbenable();
	OnBnClickedCheckDfpenable();
	OnBnClickedCheckUvenable();

	m_RGBFSlider.SetPos(m_RGBFilter.threshold);
	m_DfpSlider.SetPos(m_DFPDefect.threshold);

	ProcessStaticText(&m_RGBFSlider, IDC_STATIC_RGBF_VAL);
	ProcessStaticText(&m_DfpSlider, IDC_STATIC_DFPS_VAL);

	return 0;
}

int CFilterDialog::GetPageInfoByIndex(int & nPageID, void * pPageInfoSt, int & nStLen, int iIndex)
{
	if (iIndex == PAGE_INFO_RGBFILTER) {
		if ((pPageInfoSt == NULL) || (nStLen < sizeof(RGBFILTER))) return -1;

		m_RGBFilter.enable = m_bRGBFEnable;
		nStLen = sizeof(RGBFILTER);
		memcpy(pPageInfoSt, &m_RGBFilter, sizeof(RGBFILTER));
	}else if (iIndex == PAGE_INFO_UVFILTER) {
		if ((pPageInfoSt == NULL) || (nStLen < sizeof(UVFILTER))) return -1;
		
		m_UVFilter.enable = m_bUVFEnable;
		nStLen = sizeof(UVFILTER);
		memcpy(pPageInfoSt, &m_UVFilter, sizeof(UVFILTER));
	}else if (iIndex == PAGE_INFO_DFPDEFECT) {
		if ((pPageInfoSt == NULL) || (nStLen < sizeof(DFPDEFECT))) return -1;
		
		m_DFPDefect.enable = m_bDFPEnable;
		nStLen = sizeof(DFPDEFECT);
		memcpy(pPageInfoSt, &m_DFPDefect, sizeof(DFPDEFECT));
	}else{
		return -1;
	}

	nPageID = m_nID;
	return 0;
}

void CFilterDialog::OnBnClickedCheckRgbenable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bRGBFEnable = m_RGBFCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_RGBFILTER, m_bRGBFEnable);
}

void CFilterDialog::OnBnClickedCheckDfpenable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bDFPEnable = m_DfpCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_DEFECTPIXEL, m_bDFPEnable);
}

void CFilterDialog::OnBnClickedCheckUvenable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bUVFEnable = m_UVFCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_UVFILTER, m_bUVFEnable);
}

void CFilterDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if (bShow){
		m_RGBFCheck.SetCheck(m_bRGBFEnable);
		m_DfpCheck.SetCheck(m_bDFPEnable);
		m_UVFCheck.SetCheck(m_bUVFEnable);
	}
}

void CFilterDialog::ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID)
{
	CWnd * pText = GetDlgItem(nStaticID);
	CString Val;
	Val.Format(L"[%d]", pSliderBar->GetPos());

	pText->SetWindowText(Val);
}

void CFilterDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)pScrollBar;
	int iID = pSliderCtrl->GetDlgCtrlID();
	int iPos = pSliderCtrl->GetPos();

	if (IDC_SLIDER_RGBFILTER == iID) {
		m_RGBFilter.threshold = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_RGBF_VAL);
	}else if (IDC_SLIDER_DFP == iID) {
		m_DFPDefect.threshold = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_DFPS_VAL);
	}

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

int CFilterDialog::Clear()
{
	m_bRGBFEnable = m_bDFPEnable = m_bUVFEnable = FALSE;

	ZeroMemory(&m_RGBFilter, sizeof(RGBFILTER));
	ZeroMemory(&m_UVFilter, sizeof(UVFILTER));
	ZeroMemory(&m_DFPDefect, sizeof(DFPDEFECT));

	m_RGBFilter.type = ISP_CID_RGB_FILTER;
	m_UVFilter.type = ISP_CID_UV_FILTER;
	m_DFPDefect.type = ISP_CID_DEFECT_PIXEL;

	m_RGBFCheck.SetCheck(0);
	m_DfpCheck.SetCheck(0);
	m_UVFCheck.SetCheck(0);

	m_RGBFSlider.SetPos(0);
	m_DfpSlider.SetPos(0);

	return 0;
}

BOOL CFilterDialog::PreTranslateMessage(MSG * pMsg)
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

void CFilterDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_UVFILTER, 0);
}

void CFilterDialog::OnBnClickedButtonDfp()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_DEFECTPIXEL, 0);
}

void CFilterDialog::OnBnClickedButtonRGB()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_RGBFILTER, 0);
}
