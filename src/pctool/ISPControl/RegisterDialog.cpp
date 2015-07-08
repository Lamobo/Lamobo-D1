// RegisterDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "RegisterDialog.h"
#include "stdlib.h"
#include "errno.h"

// CRegisterDialog 对话框
#define LIMIT_TEXT	4096

IMPLEMENT_DYNAMIC(CRegisterDialog, CDialog)

CRegisterDialog::CRegisterDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CRegisterDialog::IDD, pParent)
{
	m_listRegister.clear();
	m_strText = L"";
	m_iIndex = 0;
}

CRegisterDialog::~CRegisterDialog()
{
	m_listRegister.clear();
	m_strText = L"";
}

void CRegisterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_REFISTERS, m_RegisterEdit);
}


BEGIN_MESSAGE_MAP(CRegisterDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CRegisterDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


// CRegisterDialog 消息处理程序
BOOL CRegisterDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_RegisterEdit.SetLimitText(LIMIT_TEXT);
	return TRUE;
}

BOOL CRegisterDialog::PreTranslateMessage(MSG * pMsg)
{
	// TODO: 在此添加控件通知处理程序代码
	if (pMsg->message == WM_KEYDOWN)
	{
	}

	return CDialog::PreTranslateMessage(pMsg);
}

int CRegisterDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < ((m_strText.GetLength() + 1) * sizeof(TCHAR)))) return -1;

	m_RegisterEdit.GetWindowText(m_strText);
	if (m_strText.GetLength() == 0) {
		return -1;
	}

	nPageID = m_nID;
	nStLen = (m_strText.GetLength() + 1) * sizeof(TCHAR);

	memcpy(pPageInfoSt, m_strText.GetBuffer(), nStLen);

	return 0;
}

int CRegisterDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if (pPageInfoSt == NULL || nStLen <= 0) return -1;

	m_strText.Format(L"%s", (TCHAR*)pPageInfoSt);
	m_RegisterEdit.SetWindowText(m_strText);
	
	return 0;
}

int CRegisterDialog::GetPageInfoStCount()
{
	UpdateRegister();
	if (m_listRegister.empty()) {
		return 0;
	}
	
	m_iIndex = 0;
	return m_listRegister.size();
}

int CRegisterDialog::GetPageInfoSent(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(SENSOR_REG))) return -1;
	
	if (m_iIndex >= m_listRegister.size()) return -1;

	nPageID = m_nID;
	nStLen = sizeof(SENSOR_REG);
	memcpy(pPageInfoSt, &m_listRegister[m_iIndex], nStLen);
	++m_iIndex;

	return 0;
}

BOOL CRegisterDialog::GetPageEnable()
{
	m_RegisterEdit.GetWindowText(m_strText);
	return m_strText.GetLength() > 0 ? TRUE : FALSE;
}

void CRegisterDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_REGISTER, 0);
}

void CRegisterDialog::UpdateRegister()
{
	CString strTemp, strRegister, strValue;
	m_listRegister.clear();
	m_strText = L"";

	m_RegisterEdit.GetWindowText(m_strText);
	int iFind = 0, iLastFind = 0, iFind2 = 0, iSpaceCnt = 0, iRet = 0;

	while(iFind >= 0 && iFind < m_strText.GetLength()) {
		iFind = m_strText.Find(_T(';'), iFind);
		if (iFind < 0) {
			if (m_listRegister.empty())
				AfxMessageBox(L"You did not enter a\';\'for each parameter delimiter, unable to differentiate parameters! Exit");
			return;
		}

		strTemp = m_strText.Mid(iLastFind, iFind - iLastFind);
		iFind2 = strTemp.Find(_T(','));
		if (iFind2 < 0) {
			if (m_listRegister.empty())
				AfxMessageBox(L"You did not enter \',\'as the register and numerical separator, unable to differentiate parameters! Exit");
			return;
		}
		
		SENSOR_REG stReg = {ISP_CID_SET_SENSOR_PARAM, 1, 0, 0};
		
		for (iSpaceCnt = 0; ((strTemp[iSpaceCnt] == _T(' ') || 
							  strTemp[iSpaceCnt] == _T('\n') || 
							  strTemp[iSpaceCnt] == _T('\r')) && 
							  iSpaceCnt < iFind2); ++iSpaceCnt); //忽略前空格

		strRegister = strTemp.Mid(iSpaceCnt, iFind2 - iSpaceCnt);

		stReg.cmd = wcstoul(strRegister.GetBuffer(), NULL, 16);
		if (errno == ERANGE || (stReg.cmd == 0 && 
			(strRegister.Left(4) != L"0x00" && strRegister.Left(2) != L"00" && strRegister.Left(1) != L"0"))) {
			m_RegisterEdit.SetFocus();
			m_RegisterEdit.SetSel(iLastFind, iFind);
			iRet = AfxMessageBox(L"Register address you entered illegally, please enter a hexadecimal number. Ignore this group settings?(Y/N)", MB_YESNO);
			if (iRet == IDNO) return;
			else goto again;
		}
		
		for (iSpaceCnt = iFind2 + 1; ((strTemp[iSpaceCnt] == _T(' ') ||
									  strTemp[iSpaceCnt] == _T('\n') ||
									  strTemp[iSpaceCnt] == _T('\r')) && 
									  iSpaceCnt < strTemp.GetLength()); ++iSpaceCnt); //忽略前空格

		strValue = strTemp.Mid(iSpaceCnt, strTemp.GetLength() - iSpaceCnt);
		stReg.data = wcstoul(strValue.GetBuffer(), NULL, 16);
		if (errno == ERANGE || (stReg.data == 0 && 
			(strValue.Left(4) != L"0x00" && strValue.Left(2) != L"00" && strValue.Left(1) != L"0"))) {
			m_RegisterEdit.SetFocus();
			m_RegisterEdit.SetSel(iLastFind, iFind);
			iRet = AfxMessageBox(L"Register address you entered illegally, please enter a hexadecimal number. Ignore this group settings?(Y/N)", MB_YESNO);
			if (iRet == IDNO) return;
			else goto again;
		}

		m_listRegister.push_back(stReg);

again:
		iFind += 1;
		iLastFind = iFind;
	}
}