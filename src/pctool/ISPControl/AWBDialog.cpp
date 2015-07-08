// AWBDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "AWBDialog.h"


// CAWBDialog 对话框

IMPLEMENT_DYNAMIC(CAWBDialog, CDialog)

CAWBDialog::CAWBDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAWBDialog::IDD, pParent)
{
	m_bEnable = FALSE;
	for(int i=0; i<AWBNUM; i++)
	{
		ZeroMemory(&m_stAwbInfo[i], sizeof(AWBINFO));
		m_stAwbInfo[i].type = ISP_CID_AUTO_WHITE_BALANCE;
	}
}

CAWBDialog::~CAWBDialog()
{
}

void CAWBDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_AWB_ENABLE, m_AWBCheck);
	DDX_Control(pDX, IDC_SLIDER_RHIGH, m_SliderRHigh);
	DDX_Control(pDX, IDC_SLIDER_RLOW, m_SliderRLow);
	DDX_Control(pDX, IDC_SLIDER_GHIGH, m_SliderGHigh);
	DDX_Control(pDX, IDC_SLIDER_GLOW, m_SliderGLow);
	DDX_Control(pDX, IDC_SLIDER_BHIGH, m_SliderBHigh);
	DDX_Control(pDX, IDC_SLIDER_BLOW, m_SliderBLow);
	DDX_Control(pDX, IDC_SLIDER_RGBHIGH, m_SliderRGBHigh);
	DDX_Control(pDX, IDC_SLIDER_RGBLOW, m_SliderRGBLow);
	DDX_Control(pDX, IDC_SLIDER_GRHIGH, m_SliderGrHigh);
	DDX_Control(pDX, IDC_SLIDER_GRLOW, m_SliderGrLow);
	DDX_Control(pDX, IDC_SLIDER_GBHIGH, m_SliderGbHigh);
	DDX_Control(pDX, IDC_SLIDER_GBLOW, m_SliderGbLow);
	DDX_Control(pDX, IDC_COMBO1, m_ComGroupValue);
}


BEGIN_MESSAGE_MAP(CAWBDialog, CDialog)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_CHECK_AWB_ENABLE, &CAWBDialog::OnBnClickedCheckAwbEnable)
	ON_NOTIFY(NM_THEMECHANGED, IDC_SLIDER_RHIGH, &CAWBDialog::OnNMThemeChangedSliderRhigh)
	ON_NOTIFY(TRBN_THUMBPOSCHANGING, IDC_SLIDER_RHIGH, &CAWBDialog::OnTRBNThumbPosChangingSliderRhigh)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_RHIGH, &CAWBDialog::OnNMCustomdrawSliderRhigh)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON1, &CAWBDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CAWBDialog::OnBnClickedButton2)
//	ON_CBN_EDITCHANGE(IDC_COMBO1, &CAWBDialog::OnCbnEditchangeCombo1)
ON_CBN_SELCHANGE(IDC_COMBO1, &CAWBDialog::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CAWBDialog 消息处理程序

BOOL CAWBDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	m_SliderRHigh.SetRange(0, 1023, 0);
	m_SliderRLow.SetRange(0, 1023, 0);
	m_SliderGHigh.SetRange(0, 1023, 0);
	m_SliderGLow.SetRange(0, 1023, 0);
	m_SliderBHigh.SetRange(0, 1023, 0);
	m_SliderBLow.SetRange(0, 1023, 0);
	m_SliderRGBHigh.SetRange(0, 1023, 0);
	m_SliderRGBLow.SetRange(0, 1023, 0);
	m_SliderGrHigh.SetRange(0, 1023, 0);
	m_SliderGrLow.SetRange(0, 1023, 0);
	m_SliderGbHigh.SetRange(0, 1023, 0);
	m_SliderGbLow.SetRange(0, 1023, 0);

	m_ComGroupValue.SelectString(0, L"0");
	return TRUE;
}

int CAWBDialog::SetAWBEnable(BOOL bEnable)
{
	m_bEnable = bEnable;
	return 0;
}

int CAWBDialog::SetAwbParamRespond(BYTE * pAwbParam, unsigned int nLen)
{
	if ((nLen != sizeof(struct isp_white_balance)) || (pAwbParam == NULL)) return -1;
	
	struct isp_white_balance * pAWB = (struct isp_white_balance *)pAwbParam;

	CString Val;
	Val.Format(L"%d", pAWB->co_r);
	CWnd * pText = GetDlgItem(IDC_STATIC_TR_VAL);
	pText->SetWindowText(Val);

	Val.Format(L"%d", pAWB->co_g);
	pText = GetDlgItem(IDC_STATIC_TG_VAL);
	pText->SetWindowText(Val);

	Val.Format(L"%d", pAWB->co_b);
	pText = GetDlgItem(IDC_STATIC_TB_VAL);
	pText->SetWindowText(Val);

	return 0;
}

void CAWBDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if (bShow)
		m_AWBCheck.SetCheck(m_bEnable);
}

void CAWBDialog::OnBnClickedCheckAwbEnable()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bEnable = m_AWBCheck.GetCheck();

	int nIndex, num;
	CString GroupValue;

	nIndex = m_ComGroupValue.GetCurSel();
	m_ComGroupValue.GetLBText(nIndex, GroupValue);
	num = _ttoi(GroupValue);

	m_stAwbInfo[num].enable = m_bEnable;

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_AWB, m_bEnable);
}

int CAWBDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AWBINFO))) return -1;
	
	nStLen = sizeof(AWBINFO);
	int nSliderID = -1, nPos = 0; 
	int nIndex, num;
	CString GroupValue;

	nIndex = m_ComGroupValue.GetCurSel();
	m_ComGroupValue.GetLBText(nIndex, GroupValue);
	num = _ttoi(GroupValue);
	if(num < 0 )
		num = 0;
	m_stAwbInfo[num].index = num;

	m_stAwbInfo[num].enable = m_bEnable;
	if (m_bEnable) {
		if (JudgeHighLow(nSliderID, nPos) == 1) return -1;
	}



	memcpy(pPageInfoSt, &m_stAwbInfo[num], nStLen);
	nPageID = m_nID;
	


	return 0;
}

int CAWBDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(AWBINFO))) return -1;

	int nIndex, num;
	CString GroupValue;

//	AfxMessageBox(L"1", 0, 0);

	nIndex = m_ComGroupValue.GetCurSel();
	m_ComGroupValue.GetLBText(nIndex, GroupValue);
	num = _ttoi(GroupValue);

	memcpy(&m_stAwbInfo[num], pPageInfoSt, nStLen);
	
//	AfxMessageBox(L"2", 0, 0);


	if (m_stAwbInfo[num].type != ISP_CID_AUTO_WHITE_BALANCE) return -1;
	
//	AfxMessageBox(L"3", 0, 0);

	m_bEnable = m_stAwbInfo[num].enable;
	m_AWBCheck.SetCheck(m_stAwbInfo[num].enable);

	OnBnClickedCheckAwbEnable();

	m_SliderRHigh.SetPos(m_stAwbInfo[num].r_high);
	m_SliderRLow.SetPos(m_stAwbInfo[num].r_low);
	m_SliderGHigh.SetPos(m_stAwbInfo[num].g_high);
	m_SliderGLow.SetPos(m_stAwbInfo[num].g_low);
	m_SliderBHigh.SetPos(m_stAwbInfo[num].b_high);
	m_SliderBLow.SetPos(m_stAwbInfo[num].b_low);
	m_SliderRGBHigh.SetPos(m_stAwbInfo[num].grb_high);
	m_SliderRGBLow.SetPos(m_stAwbInfo[num].grb_low);
	m_SliderGrHigh.SetPos(m_stAwbInfo[num].gr_high);
	m_SliderGrLow.SetPos(m_stAwbInfo[num].gr_low);
	m_SliderGbHigh.SetPos(m_stAwbInfo[num].gb_high);
	m_SliderGbLow.SetPos(m_stAwbInfo[num].gb_low);

	ProcessStaticText(&m_SliderRHigh, IDC_STATIC_RH_VAL);
	ProcessStaticText(&m_SliderRLow, IDC_STATIC_RL_VAL);
	ProcessStaticText(&m_SliderGHigh, IDC_STATIC_GH_VAL);
	ProcessStaticText(&m_SliderGLow, IDC_STATIC_GL_VAL);
	ProcessStaticText(&m_SliderBHigh, IDC_STATIC_BH_VAL);
	ProcessStaticText(&m_SliderBLow, IDC_STATIC_BL_VAL);
	ProcessStaticText(&m_SliderRGBHigh, IDC_STATIC_RGBH_VAL);
	ProcessStaticText(&m_SliderRGBLow, IDC_STATIC_RGBL_VAL);
	ProcessStaticText(&m_SliderGrHigh, IDC_STATIC_GRH_VAL);
	ProcessStaticText(&m_SliderGrLow, IDC_STATIC_GRL_VAL);
	ProcessStaticText(&m_SliderGbHigh, IDC_STATIC_GBH_VAL);
	ProcessStaticText(&m_SliderGbLow, IDC_STATIC_GBL_VAL);

	return 0;
}

int CAWBDialog::Clear()
{
	for(int i=0; i<AWBNUM; i++)
	{
		ZeroMemory(&m_stAwbInfo[i], sizeof(AWBINFO));
		m_stAwbInfo[i].type = ISP_CID_AUTO_WHITE_BALANCE;
	}

	m_bEnable = FALSE;
	

	m_AWBCheck.SetCheck(0);

	m_SliderRHigh.SetPos(0);
	m_SliderRLow.SetPos(0);
	m_SliderGHigh.SetPos(0);
	m_SliderGLow.SetPos(0);
	m_SliderBHigh.SetPos(0);
	m_SliderBLow.SetPos(0);
	m_SliderRGBHigh.SetPos(0);
	m_SliderRGBLow.SetPos(0);
	m_SliderGrHigh.SetPos(0);
	m_SliderGrLow.SetPos(0);
	m_SliderGbHigh.SetPos(0);
	m_SliderGbLow.SetPos(0);

	return 0;
}

void CAWBDialog::OnNMThemeChangedSliderRhigh(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 该功能要求使用 Windows XP 或更高版本。
	// 符号 _WIN32_WINNT 必须 >= 0x0501。
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CAWBDialog::OnTRBNThumbPosChangingSliderRhigh(NMHDR *pNMHDR, LRESULT *pResult)
{
	// 此功能要求 Windows Vista 或更高版本。
	// _WIN32_WINNT 符号必须 >= 0x0600。
	NMTRBTHUMBPOSCHANGING *pNMTPC = reinterpret_cast<NMTRBTHUMBPOSCHANGING *>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CAWBDialog::OnNMCustomdrawSliderRhigh(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

void CAWBDialog::ProcessStaticText(CSliderCtrl * pSliderBar, int nStaticID)
{
	CWnd * pText = GetDlgItem(nStaticID);
	CString Val;
	Val.Format(L"[%d]", pSliderBar->GetPos());

	pText->SetWindowText(Val);
}

void CAWBDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//if (nSBCode != SB_ENDSCROLL) {
		//CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
		//return;
	//}

	CSliderCtrl * pSliderCtrl = (CSliderCtrl *)pScrollBar;
	int iID = pSliderCtrl->GetDlgCtrlID();
	int iPos = pSliderCtrl->GetPos();


	int nIndex, num;
	CString GroupValue;

	nIndex = m_ComGroupValue.GetCurSel();
	m_ComGroupValue.GetLBText(nIndex, GroupValue);
	num = _ttoi(GroupValue);

again:
	if (IDC_SLIDER_RHIGH == iID) {
		m_stAwbInfo[num].r_high = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_RH_VAL);
	}else if (IDC_SLIDER_RLOW == iID) {
		m_stAwbInfo[num].r_low = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_RL_VAL);
	}else if (IDC_SLIDER_GHIGH == iID) {
		m_stAwbInfo[num].g_high = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_GH_VAL);
	}else if (IDC_SLIDER_GLOW == iID) {
		m_stAwbInfo[num].g_low = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_GL_VAL);
	}else if (IDC_SLIDER_BHIGH == iID) {
		m_stAwbInfo[num].b_high = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_BH_VAL);
	}else if (IDC_SLIDER_BLOW == iID) {
		m_stAwbInfo[num].b_low = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_BL_VAL);
	}else if (IDC_SLIDER_RGBHIGH == iID) {
		m_stAwbInfo[num].grb_high = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_RGBH_VAL);
	}else if (IDC_SLIDER_RGBLOW == iID) {
		m_stAwbInfo[num].grb_low = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_RGBL_VAL);
	}else if (IDC_SLIDER_GRHIGH == iID) {
		m_stAwbInfo[num].gr_high = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_GRH_VAL);
	}else if (IDC_SLIDER_GRLOW == iID) {
		m_stAwbInfo[num].gr_low = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_GRL_VAL);
	}else if (IDC_SLIDER_GBHIGH == iID) {
		m_stAwbInfo[num].gb_high = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_GBH_VAL);
	}else if (IDC_SLIDER_GBLOW == iID) {
		m_stAwbInfo[num].gb_low = iPos;
		ProcessStaticText(pSliderCtrl, IDC_STATIC_GBL_VAL);
	}else {
		fprintf(stderr, "AWB page Unknown slider ID(%d)\n", iID);
	}
	
	if (JudgeHighLow(iID, iPos) == 1) {
		(pSliderCtrl = ((CSliderCtrl *)GetDlgItem(iID)))->SetPos(iPos);
		goto again;
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CAWBDialog::PreTranslateMessage(MSG * pMsg)
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

void CAWBDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_AWB, 0);
}

int CAWBDialog::JudgeHighLow(int & nSliderID, int & nPos)
{
		int nIndex, num;
	CString GroupValue;

	nIndex = m_ComGroupValue.GetCurSel();
	m_ComGroupValue.GetLBText(nIndex, GroupValue);
	num = _ttoi(GroupValue);

	if (IDC_SLIDER_RHIGH == nSliderID || IDC_SLIDER_RLOW == nSliderID || -1 == nSliderID) {
		if (m_stAwbInfo[num].r_high <= m_stAwbInfo[num].r_low) {
			AfxMessageBox(L"r_hight cannot be less than or equal to r_low!\n");
			if (m_stAwbInfo[num].r_low > 1022) {
				m_stAwbInfo[num].r_low -= 1;
				nSliderID = IDC_SLIDER_RLOW;
				nPos = m_stAwbInfo[num].r_low;
				return 1;
			}else {
				m_stAwbInfo[num].r_high = m_stAwbInfo[num].r_low + 1;
				nSliderID = IDC_SLIDER_RHIGH;
				nPos = m_stAwbInfo[num].r_high;
				return 1;
			}
		}
	}
	
	if (IDC_SLIDER_GHIGH == nSliderID || IDC_SLIDER_GLOW == nSliderID || -1 == nSliderID) {
		if (m_stAwbInfo[num].g_high <= m_stAwbInfo[num].g_low) {
			AfxMessageBox(L"g_hight cannot be less than or equal to g_low!\n");
			if (m_stAwbInfo[num].g_low > 1022) {
				m_stAwbInfo[num].g_low -= 1;
				nSliderID = IDC_SLIDER_GLOW;
				nPos = m_stAwbInfo[num].g_low;
				return 1;
			}else {
				m_stAwbInfo[num].g_high = m_stAwbInfo[num].g_low + 1;
				nSliderID = IDC_SLIDER_GHIGH;
				nPos = m_stAwbInfo[num].g_high;
				return 1;
			}
		}
	}
	
	if (IDC_SLIDER_BHIGH == nSliderID || IDC_SLIDER_BLOW == nSliderID || -1 == nSliderID) {
		if (m_stAwbInfo[num].b_high <= m_stAwbInfo[num].b_low) {
			AfxMessageBox(L"b_hight cannot be less than or equal to b_low!\n");
			if (m_stAwbInfo[num].b_low > 1022) {
				m_stAwbInfo[num].b_low -= 1;
				nSliderID = IDC_SLIDER_BLOW;
				nPos = m_stAwbInfo[num].b_low;
				return 1;
			}else {
				m_stAwbInfo[num].b_high = m_stAwbInfo[num].b_low + 1;
				nSliderID = IDC_SLIDER_BHIGH;
				nPos = m_stAwbInfo[num].b_high;
				return 1;
			}
		}
	}
	
	if (IDC_SLIDER_RGBHIGH == nSliderID || IDC_SLIDER_RGBLOW == nSliderID || -1 == nSliderID) {
		if (m_stAwbInfo[num].grb_high <= m_stAwbInfo[num].grb_low) {
			AfxMessageBox(L"rgb_hight cannot be less than or equal to rgb_low!\n");
			if (m_stAwbInfo[num].grb_low > 1022) {
				m_stAwbInfo[num].grb_low -= 1;
				nSliderID = IDC_SLIDER_RGBLOW;
				nPos = m_stAwbInfo[num].grb_low;
				return 1;
			}else {
				m_stAwbInfo[num].grb_high = m_stAwbInfo[num].grb_low + 1;
				nSliderID = IDC_SLIDER_RGBHIGH;
				nPos = m_stAwbInfo[num].grb_high;
				return 1;
			}
		}
	}
	
	if (IDC_SLIDER_GRHIGH == nSliderID || IDC_SLIDER_GRLOW == nSliderID || -1 == nSliderID) {
		if (m_stAwbInfo[num].gr_high <= m_stAwbInfo[num].gr_low) {
			AfxMessageBox(L"gr_hight cannot be less than or equal to gr_low!\n");
			if (m_stAwbInfo[num].gr_low > 1022) {
				m_stAwbInfo[num].gr_low -= 1;
				nSliderID = IDC_SLIDER_GRLOW;
				nPos = m_stAwbInfo[num].gr_low;
				return 1;
			}else {
				m_stAwbInfo[num].gr_high = m_stAwbInfo[num].gr_low + 1;
				nSliderID = IDC_SLIDER_GRHIGH;
				nPos = m_stAwbInfo[num].gr_high;
				return 1;
			}
		}
	}
	
	if (IDC_SLIDER_GBLOW == nSliderID || IDC_SLIDER_GBHIGH == nSliderID || -1 == nSliderID) {
		if (m_stAwbInfo[num].gb_high <= m_stAwbInfo[num].gb_low) {
			AfxMessageBox(L"gb_hight cannot be less than or equal to gb_low!\n");
			if (m_stAwbInfo[num].gb_low > 1022) {
				m_stAwbInfo[num].gb_low -= 1;
				nSliderID = IDC_SLIDER_GBLOW;
				nPos = m_stAwbInfo[num].gb_low;
				return 1;
			}else {
				m_stAwbInfo[num].gb_high = m_stAwbInfo[num].gb_low + 1;
				nSliderID = IDC_SLIDER_GBHIGH;
				nPos = m_stAwbInfo[num].gb_high;
				return 1;
			}
		}
	}
	
	return 0;
}

void CAWBDialog::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_GET_ISP_INFO, ECT_AWB, ISP_CID_AUTO_WHITE_BALANCE);
}

//void CAWBDialog::OnCbnEditchangeCombo1()
//{
//	// TODO: 在此添加控件通知处理程序代码
//	MessageBox(L"test combo1");
//}

void CAWBDialog::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	AWBINFO stAwbInfo;

	int nIndex, num;
	CString GroupValue;

	nIndex = m_ComGroupValue.GetCurSel();
	m_ComGroupValue.GetLBText(nIndex, GroupValue);
	num = _ttoi(GroupValue);

	memcpy(&stAwbInfo, &m_stAwbInfo[num], sizeof(AWBINFO));

	SetPageInfoSt(&stAwbInfo, sizeof(AWBINFO));
}

int CAWBDialog::GetPageInfoStAll(int & nPageID, void * pPageInfoSt, int & nStlen)
{

	if ((pPageInfoSt == NULL) || (nStlen < sizeof(AWBINFO))) return -1;
	
	AWBINFO * pageinfo = (AWBINFO *) pPageInfoSt;

	nStlen = sizeof(AWBINFO);
	int nSliderID = -1, nPos = 0; 
	

	for(int i=0; i<AWBNUM; i++)
	{

		memcpy(pageinfo + i, &m_stAwbInfo[i], nStlen);
		nPageID = m_nID;
	}

	nStlen = nStlen*AWBNUM;

	nPageID = m_nID;
	return AWBNUM;
}

int CAWBDialog::SetPageInfoStAll(void *pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(AWBINFO))) return -1;

	AWBINFO * pageinfo = (AWBINFO *) pPageInfoSt;

	for(int i=0; i<AWBNUM; i++)
	{

		memcpy(&m_stAwbInfo[i], pageinfo + i, sizeof(AWBINFO));

	}

	m_ComGroupValue.SelectString(0, L"0");

	OnCbnSelchangeCombo1();

	return 0;

}