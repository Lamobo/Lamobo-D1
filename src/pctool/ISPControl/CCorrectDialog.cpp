// CCorrectDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "CCorrectDialog.h"


// CCCorrectDialog 对话框

IMPLEMENT_DYNAMIC(CCCorrectDialog, CDialog)

CCCorrectDialog::CCCorrectDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CCCorrectDialog::IDD, pParent)
{
	m_bCCEnable = FALSE;
	ZeroMemory(&m_CCorrect, sizeof(CCORRECT));
	m_CCorrect.type = ISP_CID_COLOR;
}

CCCorrectDialog::~CCCorrectDialog()
{
}

void CCCorrectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_CC_ENABLE, m_CCCheck);
	DDX_Control(pDX, IDC_SLIDER_C11, m_C11Slider);
	DDX_Control(pDX, IDC_SLIDER_C12, m_C12Slider);
	DDX_Control(pDX, IDC_SLIDER_C13, m_C13Slider);
	DDX_Control(pDX, IDC_SLIDER_C21, m_C21Slider);
	DDX_Control(pDX, IDC_SLIDER_C22, m_C22Slider);
	DDX_Control(pDX, IDC_SLIDER_C23, m_C23Slider);
	DDX_Control(pDX, IDC_SLIDER_C31, m_C31Slider);
	DDX_Control(pDX, IDC_SLIDER_C32, m_C32Slider);
	DDX_Control(pDX, IDC_SLIDER_C33, m_C33Slider);
	DDX_Control(pDX, IDC_SLIDER_ILOW, m_ILowSlider);
	DDX_Control(pDX, IDC_SLIDER_IHIGH, m_IHighSlider);
}


BEGIN_MESSAGE_MAP(CCCorrectDialog, CDialog)
	ON_BN_CLICKED(IDC_CHECK_CC_ENABLE, &CCCorrectDialog::OnBnClickedCheckCcEnable)
	ON_WM_HSCROLL()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON1, &CCCorrectDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


// CCCorrectDialog 消息处理程序

BOOL CCCorrectDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_C11Slider.SetRange(-2560, 2560, 0);
	m_C11Slider.SetPos(-2560);

	m_C12Slider.SetRange(-2560, 2560, 0);
	m_C12Slider.SetPos(-2560);

	m_C13Slider.SetRange(-2560, 2560, 0);
	m_C13Slider.SetPos(-2560);

	m_C21Slider.SetRange(-2560, 2560, 0);
	m_C21Slider.SetPos(-2560);

	m_C22Slider.SetRange(-2560, 2560, 0);
	m_C22Slider.SetPos(-2560);

	m_C23Slider.SetRange(-2560, 2560, 0);
	m_C23Slider.SetPos(-2560);

	m_C31Slider.SetRange(-2560, 2560, 0);
	m_C31Slider.SetPos(-2560);

	m_C32Slider.SetRange(-2560, 2560, 0);
	m_C32Slider.SetPos(-2560);

	m_C33Slider.SetRange(-2560, 2560, 0);
	m_C33Slider.SetPos(-2560);

	m_ILowSlider.SetRange(0, 2048, 0);
	m_IHighSlider.SetRange(0, 2048, 0);

	m_CCorrect.ccMtrx[0][0] = -2560;
	m_CCorrect.ccMtrx[0][1] = -2560;
	m_CCorrect.ccMtrx[0][2] = -2560;
	m_CCorrect.ccMtrx[1][0] = -2560;
	m_CCorrect.ccMtrx[1][1] = -2560;
	m_CCorrect.ccMtrx[1][2] = -2560;
	m_CCorrect.ccMtrx[2][0] = -2560;
	m_CCorrect.ccMtrx[2][1] = -2560;
	m_CCorrect.ccMtrx[2][2] = -2560;

	return TRUE;
}

int CCCorrectDialog::SetCCorrectEnable(BOOL bEnable)
{
	m_bCCEnable = bEnable;
	return 0;
}

int CCCorrectDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(CCORRECT))) return -1;
	
	nStLen = sizeof(CCORRECT);
	m_CCorrect.enable = m_bCCEnable;
	int nSliderID = -1, nPos = 0;
	if (m_bCCEnable) {
		if (JudgeHighLow(nSliderID, nPos) == 1) return -1;
	}

	memcpy(pPageInfoSt, &m_CCorrect, nStLen);
	nPageID = m_nID;

	return 0;
}

int CCCorrectDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(CCORRECT))) return -1;

	memcpy(&m_CCorrect, pPageInfoSt, nStLen);

	if (m_CCorrect.type != ISP_CID_COLOR) return -1;

	m_bCCEnable = m_CCorrect.enable;
	m_CCCheck.SetCheck(m_CCorrect.enable);

	OnBnClickedCheckCcEnable();

	m_C11Slider.SetPos(m_CCorrect.ccMtrx[0][0]);
	m_C12Slider.SetPos(m_CCorrect.ccMtrx[0][1]);
	m_C13Slider.SetPos(m_CCorrect.ccMtrx[0][2]);

	m_C21Slider.SetPos(m_CCorrect.ccMtrx[1][0]);
	m_C22Slider.SetPos(m_CCorrect.ccMtrx[1][1]);
	m_C23Slider.SetPos(m_CCorrect.ccMtrx[1][2]);

	m_C31Slider.SetPos(m_CCorrect.ccMtrx[2][0]);
	m_C32Slider.SetPos(m_CCorrect.ccMtrx[2][1]);
	m_C33Slider.SetPos(m_CCorrect.ccMtrx[2][2]);

	m_ILowSlider.SetPos(m_CCorrect.cc_thrs_low);
	m_IHighSlider.SetPos(m_CCorrect.cc_thrs_high);

	ProcessStaticText(&m_C11Slider, IDC_STATIC_C11_VAL);
	ProcessStaticText(&m_C12Slider, IDC_STATIC_C12_VAL);
	ProcessStaticText(&m_C13Slider, IDC_STATIC_C13_VAL);
	ProcessStaticText(&m_C21Slider, IDC_STATIC_C21_VAL);
	ProcessStaticText(&m_C22Slider, IDC_STATIC_C22_VAL);
	ProcessStaticText(&m_C23Slider, IDC_STATIC_C23_VAL);
	ProcessStaticText(&m_C31Slider, IDC_STATIC_C31_VAL);
	ProcessStaticText(&m_C32Slider, IDC_STATIC_C32_VAL);
	ProcessStaticText(&m_C33Slider, IDC_STATIC_C33_VAL);
	ProcessStaticText(&m_ILowSlider, IDC_STATIC_ILOW_VAL);
	ProcessStaticText(&m_IHighSlider, IDC_STATIC_IHIGH_VAL);

	return 0;
}

void CCCorrectDialog::OnBnClickedCheckCcEnable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bCCEnable = m_CCCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_CCORRECT, m_bCCEnable);
}

void CCCorrectDialog::ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID)
{
	CWnd * pText = GetDlgItem(nStaticID);
	CString Val;
	Val.Format(L"[%d]", pSliderBar->GetPos());

	pText->SetWindowText(Val);
}

void CCCorrectDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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
	if (IDC_SLIDER_C11 == iID) {
		m_CCorrect.ccMtrx[0][0] = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_C11_VAL);
	}else if (IDC_SLIDER_C12 == iID) {
		m_CCorrect.ccMtrx[0][1] = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_C12_VAL);
	}else if (IDC_SLIDER_C13 == iID) {
		m_CCorrect.ccMtrx[0][2] = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_C13_VAL);
	}else if (IDC_SLIDER_C21 == iID) {
		m_CCorrect.ccMtrx[1][0] = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_C21_VAL);
	}else if (IDC_SLIDER_C22 == iID) {
		m_CCorrect.ccMtrx[1][1] = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_C22_VAL);
	}else if (IDC_SLIDER_C23 == iID) {
		m_CCorrect.ccMtrx[1][2] = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_C23_VAL);
	}else if (IDC_SLIDER_C31 == iID) {
		m_CCorrect.ccMtrx[2][0] = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_C31_VAL);
	}else if (IDC_SLIDER_C32 == iID) {
		m_CCorrect.ccMtrx[2][1] = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_C32_VAL);
	}else if (IDC_SLIDER_C33 == iID) {
		m_CCorrect.ccMtrx[2][2] = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_C33_VAL);
	}else if (IDC_SLIDER_ILOW == iID) {
		m_CCorrect.cc_thrs_low = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_ILOW_VAL);
	}else if (IDC_SLIDER_IHIGH == iID) {
		m_CCorrect.cc_thrs_high = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_IHIGH_VAL);
	}

	if (JudgeHighLow(iID, iPos) == 1) {
		(pSliderCtrl = ((CSliderCtrl *)GetDlgItem(iID)))->SetPos(iPos);
		goto again;
	}

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCCorrectDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if (bShow)
		m_CCCheck.SetCheck(m_bCCEnable);
}

int CCCorrectDialog::Clear()
{
	ZeroMemory(&m_CCorrect, sizeof(CCORRECT));
	m_bCCEnable = FALSE;
	m_CCorrect.type = ISP_CID_COLOR;

	m_CCCheck.SetCheck(0);

	m_C11Slider.SetPos(-2560);
	m_C12Slider.SetPos(-2560);
	m_C13Slider.SetPos(-2560);
	m_C21Slider.SetPos(-2560);
	m_C22Slider.SetPos(-2560);
	m_C23Slider.SetPos(-2560);
	m_C31Slider.SetPos(-2560);
	m_C32Slider.SetPos(-2560);
	m_C33Slider.SetPos(-2560);
	m_ILowSlider.SetPos(0);
	m_IHighSlider.SetPos(0);

	m_CCorrect.ccMtrx[0][0] = -2560;
	m_CCorrect.ccMtrx[0][1] = -2560;
	m_CCorrect.ccMtrx[0][2] = -2560;
	m_CCorrect.ccMtrx[1][0] = -2560;
	m_CCorrect.ccMtrx[1][1] = -2560;
	m_CCorrect.ccMtrx[1][2] = -2560;
	m_CCorrect.ccMtrx[2][0] = -2560;
	m_CCorrect.ccMtrx[2][1] = -2560;
	m_CCorrect.ccMtrx[2][2] = -2560;

	return 0;
}

BOOL CCCorrectDialog::PreTranslateMessage(MSG * pMsg)
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

void CCCorrectDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_CCORRECT, 0);
}

int CCCorrectDialog::JudgeHighLow(int & nSliderID, int & nPos)
{
	if (IDC_SLIDER_ILOW == nSliderID || IDC_SLIDER_IHIGH == nSliderID || -1 == nSliderID) {
		if (m_CCorrect.cc_thrs_high <= m_CCorrect.cc_thrs_low) {
			AfxMessageBox(L"IthreHigh cannot be less than or equal to IthreLow!\n");
			if (m_CCorrect.cc_thrs_low > 2047) {
				m_CCorrect.cc_thrs_low -= 1;
				nSliderID = IDC_SLIDER_ILOW;
				nPos = m_CCorrect.cc_thrs_low;
				return 1;
			}else {
				m_CCorrect.cc_thrs_high = m_CCorrect.cc_thrs_low + 1;
				nSliderID = IDC_SLIDER_IHIGH;
				nPos = m_CCorrect.cc_thrs_high;
				return 1;
			}
		}
	}
	
	return 0;
}