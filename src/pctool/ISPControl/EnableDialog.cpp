// EnableDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "EnableDialog.h"


// CEnableDialog 对话框

IMPLEMENT_DYNAMIC(CEnableDialog, CDialog)

CEnableDialog::CEnableDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CEnableDialog::IDD, pParent)
{
	ZeroMemory(&m_stEnable, sizeof(m_stEnable));
}

CEnableDialog::~CEnableDialog()
{
}

void CEnableDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_BB, m_BBCheck);
	DDX_Control(pDX, IDC_CHECK_LENS_ADJUST, m_LensCheck);
	DDX_Control(pDX, IDC_CHECK_DEMOSAIC, m_DemosaicCheck);
	DDX_Control(pDX, IDC_CHECK_RGB_FILTER, m_RGBFCheck);
	DDX_Control(pDX, IDC_CHECK_UV_FILTER, m_UVFCheck);
	DDX_Control(pDX, IDC_CHECK_DEFECT_PIXEL, m_DPixelCheck);
	DDX_Control(pDX, IDC_CHECK_WB, m_WBCheck);
	DDX_Control(pDX, IDC_CHECK_AWB, m_AWBCheck);
	DDX_Control(pDX, IDC_CHECK_CCORRECT, m_CCorrectCheck);
	DDX_Control(pDX, IDC_CHECK_GAMMA, m_GammaCheck);
	DDX_Control(pDX, IDC_CHECK_BENHANCE, m_BEnhanceCheck);
	DDX_Control(pDX, IDC_CHECK_SATURATIOn, m_SaturCheck);
	DDX_Control(pDX, IDC_CHECK_SPECIAL, m_SpecEffCheck);
}


BEGIN_MESSAGE_MAP(CEnableDialog, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_BB, &CEnableDialog::OnBnClickedCheckBb)
	ON_BN_CLICKED(IDC_CHECK_LENS_ADJUST, &CEnableDialog::OnBnClickedCheckLensAdjust)
	ON_BN_CLICKED(IDC_CHECK_DEMOSAIC, &CEnableDialog::OnBnClickedCheckDemosaic)
	ON_BN_CLICKED(IDC_CHECK_RGB_FILTER, &CEnableDialog::OnBnClickedCheckRgbFilter)
	ON_BN_CLICKED(IDC_CHECK_UV_FILTER, &CEnableDialog::OnBnClickedCheckUvFilter)
	ON_BN_CLICKED(IDC_CHECK_DEFECT_PIXEL, &CEnableDialog::OnBnClickedCheckDefectPixel)
	ON_BN_CLICKED(IDC_CHECK_WB, &CEnableDialog::OnBnClickedCheckWb)
	ON_BN_CLICKED(IDC_CHECK_AWB, &CEnableDialog::OnBnClickedCheckAwb)
	ON_BN_CLICKED(IDC_CHECK_CCORRECT, &CEnableDialog::OnBnClickedCheckCcorrect)
	ON_BN_CLICKED(IDC_CHECK_GAMMA, &CEnableDialog::OnBnClickedCheckGamma)
	ON_BN_CLICKED(IDC_CHECK_BENHANCE, &CEnableDialog::OnBnClickedCheckBenhance)
	ON_BN_CLICKED(IDC_CHECK_SATURATIOn, &CEnableDialog::OnBnClickedCheckSaturation)
	ON_BN_CLICKED(IDC_CHECK_SPECIAL, &CEnableDialog::OnBnClickedCheckSpecial)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CEnableDialog 消息处理程序

void CEnableDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()
}

void CEnableDialog::OnBnClickedCheckBb()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_BBCheck.GetCheck())
		m_stEnable.bBBEnable = TRUE;
	else
		m_stEnable.bBBEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_BB, m_stEnable.bBBEnable);
}

void CEnableDialog::OnBnClickedCheckLensAdjust()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_LensCheck.GetCheck())
		m_stEnable.bLensEnable = TRUE;
	else
		m_stEnable.bLensEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_LENS, m_stEnable.bLensEnable);
}

void CEnableDialog::OnBnClickedCheckDemosaic()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_DemosaicCheck.GetCheck())
		m_stEnable.bDemosaicEnable = TRUE;
	else
		m_stEnable.bDemosaicEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_DEMOSAIC, m_stEnable.bDemosaicEnable);
}

void CEnableDialog::OnBnClickedCheckRgbFilter()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_RGBFCheck.GetCheck())
		m_stEnable.bRGBFilterEnable = TRUE;
	else
		m_stEnable.bRGBFilterEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_RGBFILTER, m_stEnable.bRGBFilterEnable);
}

void CEnableDialog::OnBnClickedCheckUvFilter()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_UVFCheck.GetCheck())
		m_stEnable.bUVFilterEnable = TRUE;
	else
		m_stEnable.bUVFilterEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_UVFILTER, m_stEnable.bUVFilterEnable);
}

void CEnableDialog::OnBnClickedCheckDefectPixel()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_DPixelCheck.GetCheck())
		m_stEnable.bDefectPixelEnable = TRUE;
	else
		m_stEnable.bDefectPixelEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_DEFECTPIXEL, m_stEnable.bDefectPixelEnable);
}

void CEnableDialog::OnBnClickedCheckWb()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_WBCheck.GetCheck())
		m_stEnable.bWBEnable = TRUE;
	else
		m_stEnable.bWBEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_WB, m_stEnable.bWBEnable);
}

void CEnableDialog::OnBnClickedCheckAwb()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_AWBCheck.GetCheck())
		m_stEnable.bAWBEnable = TRUE;
	else
		m_stEnable.bAWBEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_AWB, m_stEnable.bAWBEnable);
}

void CEnableDialog::OnBnClickedCheckCcorrect()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_CCorrectCheck.GetCheck())
		m_stEnable.bCCorrectEnable = TRUE;
	else
		m_stEnable.bCCorrectEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_CCORRECT, m_stEnable.bCCorrectEnable);
}

void CEnableDialog::OnBnClickedCheckGamma()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_GammaCheck.GetCheck())
		m_stEnable.bGammaEnable = TRUE;
	else
		m_stEnable.bGammaEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_GAMMA, m_stEnable.bGammaEnable);
}

void CEnableDialog::OnBnClickedCheckBenhance()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_BEnhanceCheck.GetCheck())
		m_stEnable.bBEnhanceEnable = TRUE;
	else
		m_stEnable.bBEnhanceEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_BENHANCE, m_stEnable.bBEnhanceEnable);
}

void CEnableDialog::OnBnClickedCheckSaturation()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_SaturCheck.GetCheck())
		m_stEnable.bSaturationEnable = TRUE;
	else
		m_stEnable.bSaturationEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_SATURATION, m_stEnable.bSaturationEnable);
}

void CEnableDialog::OnBnClickedCheckSpecial()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_SpecEffCheck.GetCheck())
		m_stEnable.bSpecEffEnable = TRUE;
	else
		m_stEnable.bSpecEffEnable = FALSE;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_SPECEFFEICT, m_stEnable.bSpecEffEnable);
}

int CEnableDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(m_stEnable))) return -1;
	
	nStLen = sizeof(m_stEnable);
	memcpy(pPageInfoSt, &m_stEnable, nStLen);
	nPageID = m_nID;

	return 0;
}

int CEnableDialog::SetEnable(int nFlag, BOOL bEnable)
{
	if (ECT_BB == nFlag) m_stEnable.bBBEnable = bEnable;
	else if (ECT_LENS == nFlag) m_stEnable.bLensEnable = bEnable;
	else if (ECT_DEMOSAIC == nFlag) m_stEnable.bDemosaicEnable = bEnable;
	else if (ECT_RGBFILTER == nFlag) m_stEnable.bRGBFilterEnable = bEnable;
	else if (ECT_UVFILTER == nFlag) m_stEnable.bUVFilterEnable = bEnable;
	else if (ECT_DEFECTPIXEL == nFlag) m_stEnable.bDefectPixelEnable = bEnable;
	else if (ECT_WB == nFlag) m_stEnable.bWBEnable = bEnable;
	else if (ECT_AWB == nFlag) m_stEnable.bAWBEnable = bEnable;
	else if (ECT_CCORRECT == nFlag) m_stEnable.bCCorrectEnable = bEnable;
	else if (ECT_GAMMA == nFlag) m_stEnable.bGammaEnable = bEnable;
	else if (ECT_BENHANCE == nFlag) m_stEnable.bBEnhanceEnable = bEnable;
	else if (ECT_SATURATION == nFlag) m_stEnable.bSaturationEnable = bEnable;
	else if (ECT_SPECEFFEICT == nFlag) m_stEnable.bSpecEffEnable = bEnable;
	else return -1;

	m_BBCheck.SetCheck(m_stEnable.bBBEnable);
	m_LensCheck.SetCheck(m_stEnable.bLensEnable);
	m_DemosaicCheck.SetCheck(m_stEnable.bDemosaicEnable);
	m_RGBFCheck.SetCheck(m_stEnable.bRGBFilterEnable);
	m_UVFCheck.SetCheck(m_stEnable.bUVFilterEnable);
	m_DPixelCheck.SetCheck(m_stEnable.bDefectPixelEnable);
	m_WBCheck.SetCheck(m_stEnable.bWBEnable);
	m_AWBCheck.SetCheck(m_stEnable.bAWBEnable);
	m_CCorrectCheck.SetCheck(m_stEnable.bCCorrectEnable);
	m_GammaCheck.SetCheck(m_stEnable.bGammaEnable);
	m_BEnhanceCheck.SetCheck(m_stEnable.bBEnhanceEnable);
	m_SaturCheck.SetCheck(m_stEnable.bSaturationEnable);
	m_SpecEffCheck.SetCheck(m_stEnable.bSpecEffEnable);	

	return 0;
}

int CEnableDialog::Clear()
{
	ZeroMemory(&m_stEnable, sizeof(m_stEnable));

	m_BBCheck.SetCheck(0);
	m_LensCheck.SetCheck(0);
	m_DemosaicCheck.SetCheck(0);
	m_RGBFCheck.SetCheck(0);
	m_UVFCheck.SetCheck(0);
	m_DPixelCheck.SetCheck(0);
	m_WBCheck.SetCheck(0);
	m_AWBCheck.SetCheck(0);
	m_CCorrectCheck.SetCheck(0);
	m_GammaCheck.SetCheck(0);
	m_BEnhanceCheck.SetCheck(0);
	m_SaturCheck.SetCheck(0);
	m_SpecEffCheck.SetCheck(0);
	return 0;
}
void CEnableDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码

	if (bShow) {
		m_BBCheck.SetCheck(m_stEnable.bBBEnable);
		m_LensCheck.SetCheck(m_stEnable.bLensEnable);
		m_DemosaicCheck.SetCheck(m_stEnable.bDemosaicEnable);
		m_RGBFCheck.SetCheck(m_stEnable.bRGBFilterEnable);
		m_UVFCheck.SetCheck(m_stEnable.bUVFilterEnable);
		m_DPixelCheck.SetCheck(m_stEnable.bDefectPixelEnable);
		m_WBCheck.SetCheck(m_stEnable.bWBEnable);
		m_AWBCheck.SetCheck(m_stEnable.bAWBEnable);
		m_CCorrectCheck.SetCheck(m_stEnable.bCCorrectEnable);
		m_GammaCheck.SetCheck(m_stEnable.bGammaEnable);
		m_BEnhanceCheck.SetCheck(m_stEnable.bBEnhanceEnable);
		m_SaturCheck.SetCheck(m_stEnable.bSaturationEnable);
		m_SpecEffCheck.SetCheck(m_stEnable.bSpecEffEnable);	
	}
}

BOOL CEnableDialog::PreTranslateMessage(MSG * pMsg)
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
