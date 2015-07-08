// DemosaicDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "DemosaicDialog.h"


// CDemosaicDialog 对话框

IMPLEMENT_DYNAMIC(CDemosaicDialog, CDialog)

CDemosaicDialog::CDemosaicDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CDemosaicDialog::IDD, pParent)
{
	m_bDEnable = FALSE;
	ZeroMemory(&m_Demosaic, sizeof(DEMOSAIC));
	m_Demosaic.type = ISP_CID_DEMOSAIC;
}

CDemosaicDialog::~CDemosaicDialog()
{
}

void CDemosaicDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_D_ENABLE, m_DCheck);
	DDX_Control(pDX, IDC_SLIDER_THRESOLD, m_DThreSlider);
}


BEGIN_MESSAGE_MAP(CDemosaicDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECK_D_ENABLE, &CDemosaicDialog::OnBnClickedCheckDEnable)
	ON_WM_SHOWWINDOW()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON1, &CDemosaicDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


// CDemosaicDialog 消息处理程序

BOOL CDemosaicDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_DThreSlider.SetRange(0, 4096, 0);

	return TRUE;
}

int CDemosaicDialog::SetDemosaicEnable(BOOL bEnable)
{
	m_bDEnable = bEnable;
	return 0;
}

int CDemosaicDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(DEMOSAIC))) return -1;
	
	nStLen = sizeof(DEMOSAIC);
	m_Demosaic.enable = m_bDEnable;
	memcpy(pPageInfoSt, &m_Demosaic, nStLen);
	nPageID = m_nID;

	return 0;
}

int CDemosaicDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(DEMOSAIC))) return -1;

	memcpy(&m_Demosaic, pPageInfoSt, nStLen);

	if (m_Demosaic.type != ISP_CID_DEMOSAIC) return -1;

	m_bDEnable = m_Demosaic.enable;
	m_DCheck.SetCheck(m_Demosaic.enable);

	OnBnClickedCheckDEnable();

	m_DThreSlider.SetPos(m_Demosaic.threshold);

	ProcessStaticText(&m_DThreSlider, IDC_STATIC_THRESOLD_VAL);

	return 0;
}

void CDemosaicDialog::OnBnClickedCheckDEnable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bDEnable = m_DCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_DEMOSAIC, m_bDEnable);
}

void CDemosaicDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if (bShow)
		m_DCheck.SetCheck(m_bDEnable);
}

void CDemosaicDialog::ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID)
{
	CWnd * pText = GetDlgItem(nStaticID);
	CString Val;
	Val.Format(L"[%d]", pSliderBar->GetPos());

	pText->SetWindowText(Val);
}

void CDemosaicDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)pScrollBar;
	int iID = pSliderCtrl->GetDlgCtrlID();
	int iPos = pSliderCtrl->GetPos();

	if (IDC_SLIDER_THRESOLD == iID) {
		m_Demosaic.threshold = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_THRESOLD_VAL);
	}

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

int CDemosaicDialog::Clear()
{
	ZeroMemory(&m_Demosaic, sizeof(DEMOSAIC));
	m_bDEnable = FALSE;
	m_Demosaic.type = ISP_CID_DEMOSAIC;

	m_DCheck.SetCheck(0);

	m_DThreSlider.SetPos(0);

	return 0;
}

BOOL CDemosaicDialog::PreTranslateMessage(MSG * pMsg)
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

void CDemosaicDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_DEMOSAIC, 0);
}
