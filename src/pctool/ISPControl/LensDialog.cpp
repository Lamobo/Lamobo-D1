// LensDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "ISPControl.h"
#include "LensDialog.h"


// CLensDialog 对话框

IMPLEMENT_DYNAMIC(CLensDialog, CDialog)

CLensDialog::CLensDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CLensDialog::IDD, pParent)
{
	ZeroMemory(&m_LensCorrect, sizeof(LENSCORRECT));
	m_LensCorrect.type = ISP_CID_LENS;
	m_bLensEnable = FALSE;
}

CLensDialog::~CLensDialog()
{
}

void CLensDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_LENS, m_LensCheck);
}


BEGIN_MESSAGE_MAP(CLensDialog, CDialog)
	ON_EN_CHANGE(IDC_EDIT_XREF, &CLensDialog::OnEnChangeEditXref)
	ON_EN_CHANGE(IDC_EDIT_YREF, &CLensDialog::OnEnChangeEditYref)
	ON_EN_CHANGE(IDC_EDIT_COEF_A0, &CLensDialog::OnEnChangeEditCoefA0)
	ON_EN_CHANGE(IDC_EDIT_COEF_B0, &CLensDialog::OnEnChangeEditCoefB0)
	ON_EN_CHANGE(IDC_EDIT_COEF_C0, &CLensDialog::OnEnChangeEditCoefC0)
	ON_EN_CHANGE(IDC_EDIT_COEF_A1, &CLensDialog::OnEnChangeEditCoefA1)
	ON_EN_CHANGE(IDC_EDIT_COEF_B1, &CLensDialog::OnEnChangeEditCoefB1)
	ON_EN_CHANGE(IDC_EDIT_COEF_C1, &CLensDialog::OnEnChangeEditCoefC1)
	ON_EN_CHANGE(IDC_EDIT_COEF_A2, &CLensDialog::OnEnChangeEditCoefA2)
	ON_EN_CHANGE(IDC_EDIT_COEF_B2, &CLensDialog::OnEnChangeEditCoefB2)
	ON_EN_CHANGE(IDC_EDIT_COEF_C2, &CLensDialog::OnEnChangeEditCoefC2)
	ON_EN_CHANGE(IDC_EDIT_COEF_A3, &CLensDialog::OnEnChangeEditCoefA3)
	ON_EN_CHANGE(IDC_EDIT_COEF_B3, &CLensDialog::OnEnChangeEditCoefB3)
	ON_EN_CHANGE(IDC_EDIT_COEF_C3, &CLensDialog::OnEnChangeEditCoefC3)
	ON_EN_CHANGE(IDC_EDIT_COEF_A4, &CLensDialog::OnEnChangeEditCoefA4)
	ON_EN_CHANGE(IDC_EDIT_COEF_B4, &CLensDialog::OnEnChangeEditCoefB4)
	ON_EN_CHANGE(IDC_EDIT_COEF_C4, &CLensDialog::OnEnChangeEditCoefC4)
	ON_EN_CHANGE(IDC_EDIT_COEF_A5, &CLensDialog::OnEnChangeEditCoefA5)
	ON_EN_CHANGE(IDC_EDIT_COEF_B5, &CLensDialog::OnEnChangeEditCoefB5)
	ON_EN_CHANGE(IDC_EDIT_COEF_C5, &CLensDialog::OnEnChangeEditCoefC5)
	ON_EN_CHANGE(IDC_EDIT_COEF_A6, &CLensDialog::OnEnChangeEditCoefA6)
	ON_EN_CHANGE(IDC_EDIT_COEF_B6, &CLensDialog::OnEnChangeEditCoefB6)
	ON_EN_CHANGE(IDC_EDIT_COEF_C6, &CLensDialog::OnEnChangeEditCoefC6)
	ON_EN_CHANGE(IDC_EDIT_COEF_A7, &CLensDialog::OnEnChangeEditCoefA7)
	ON_EN_CHANGE(IDC_EDIT_COEF_B7, &CLensDialog::OnEnChangeEditCoefB7)
	ON_EN_CHANGE(IDC_EDIT_COEF_C7, &CLensDialog::OnEnChangeEditCoefC7)
	ON_EN_CHANGE(IDC_EDIT_COEF_A8, &CLensDialog::OnEnChangeEditCoefA8)
	ON_EN_CHANGE(IDC_EDIT_COEF_B8, &CLensDialog::OnEnChangeEditCoefB8)
	ON_EN_CHANGE(IDC_EDIT_COEF_C8, &CLensDialog::OnEnChangeEditCoefC8)
	ON_EN_CHANGE(IDC_EDIT_COEF_A9, &CLensDialog::OnEnChangeEditCoefA9)
	ON_EN_CHANGE(IDC_EDIT_COEF_B9, &CLensDialog::OnEnChangeEditCoefB9)
	ON_EN_CHANGE(IDC_EDIT_COEF_C9, &CLensDialog::OnEnChangeEditCoefC9)
	ON_EN_CHANGE(IDC_EDIT_RANGE0, &CLensDialog::OnEnChangeEditRange0)
	ON_EN_CHANGE(IDC_EDIT_RANGE1, &CLensDialog::OnEnChangeEditRange1)
	ON_EN_CHANGE(IDC_EDIT_RANGE2, &CLensDialog::OnEnChangeEditRange2)
	ON_EN_CHANGE(IDC_EDIT_RANGE3, &CLensDialog::OnEnChangeEditRange3)
	ON_EN_CHANGE(IDC_EDIT_RANGE4, &CLensDialog::OnEnChangeEditRange4)
	ON_EN_CHANGE(IDC_EDIT_RANGE5, &CLensDialog::OnEnChangeEditRange5)
	ON_EN_CHANGE(IDC_EDIT_RANGE6, &CLensDialog::OnEnChangeEditRange6)
	ON_EN_CHANGE(IDC_EDIT_RANGE7, &CLensDialog::OnEnChangeEditRange7)
	ON_EN_CHANGE(IDC_EDIT_RANGE8, &CLensDialog::OnEnChangeEditRange8)
	ON_EN_CHANGE(IDC_EDIT_RANGE9, &CLensDialog::OnEnChangeEditRange9)
	ON_BN_CLICKED(IDC_CHECK_LENS, &CLensDialog::OnBnClickedCheckLens)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON1, &CLensDialog::OnBnClickedButton1)
END_MESSAGE_MAP()


// CLensDialog 消息处理程序


int CLensDialog::EditBoxNumberStringProcess(CString strEBText, int nEditBoxID)
{
	int strLen = strEBText.GetLength();

	if (strLen < 0) return -1;
	
	TCHAR cNumber = 0;
	for (int i = 0; i < strLen; ++i) {
		cNumber = strEBText.GetAt(i);
		if ((cNumber > _T('9') || cNumber < _T('0')) && (cNumber != _T('-'))) {
			AfxMessageBox(L"Please enter the numbers 0~9!\n");
			strEBText.Delete(i);
			SetDlgItemText(nEditBoxID, strEBText);
		}
	}

	return 0;
}

int CLensDialog::EditBoxNumberProcess(int nNumber, int nMin, int nMax)
{
	if (nNumber > nMax) return nMax;
	if (nNumber < nMin) return nMin;

	return nNumber;
}

void CLensDialog::EditBoxEnterProcess(int nEditBoxID, int & nValue, int nMin, int nMax)
{
	CString strText;
	GetDlgItemText(nEditBoxID, strText);
	if (strText.GetLength() == 0) return;
	
	if (EditBoxNumberStringProcess(strText, nEditBoxID) < 0) return;

	nValue = EditBoxNumberProcess(_ttoi(strText), nMin, nMax);
	
	if (nValue != (unsigned int)_ttoi(strText)) {
		strText.Format(L"%d", nValue);
		SetDlgItemText(nEditBoxID, strText);
	}
}

void CLensDialog::EditBoxEnterProcess(int nEditBoxID, unsigned int & nValue, int nMin, int nMax)
{
	CString strText;
	GetDlgItemText(nEditBoxID, strText);
	if (strText.GetLength() == 0) return;
	
	if (EditBoxNumberStringProcess(strText, nEditBoxID) < 0) return;

	nValue = (unsigned int)(EditBoxNumberProcess(_ttoi(strText), nMin, nMax));
	
	if (nValue != (unsigned int)_ttoi(strText)) {
		strText.Format(L"%d", nValue);
		SetDlgItemText(nEditBoxID, strText);
	}
}

void CLensDialog::OnEnChangeEditXref()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_XREF, m_LensCorrect.lens_xref, 0, 8192);
}

void CLensDialog::OnEnChangeEditYref()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_YREF, m_LensCorrect.lens_yref, 0, 8192);
}

void CLensDialog::OnEnChangeEditCoefA0()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_A0, m_LensCorrect.lens_coefa[0], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefB0()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_B0, m_LensCorrect.lens_coefb[0], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefC0()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_C0, m_LensCorrect.lens_coefc[0], -511, 511);
}

void CLensDialog::OnEnChangeEditCoefA1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_A1, m_LensCorrect.lens_coefa[1], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefB1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_B1, m_LensCorrect.lens_coefb[1], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefC1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_C1, m_LensCorrect.lens_coefc[1], -511, 511);
}

void CLensDialog::OnEnChangeEditCoefA2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_A2, m_LensCorrect.lens_coefa[2], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefB2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_B2, m_LensCorrect.lens_coefb[2], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefC2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_C2, m_LensCorrect.lens_coefc[2], -511, 511);
}

void CLensDialog::OnEnChangeEditCoefA3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_A3, m_LensCorrect.lens_coefa[3], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefB3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_B3, m_LensCorrect.lens_coefb[3], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefC3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	
	EditBoxEnterProcess(IDC_EDIT_COEF_C3, m_LensCorrect.lens_coefc[3], -511, 511);
}

void CLensDialog::OnEnChangeEditCoefA4()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_A4, m_LensCorrect.lens_coefa[4], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefB4()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_B4, m_LensCorrect.lens_coefb[4], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefC4()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_C4, m_LensCorrect.lens_coefc[4], -511, 511);
}

void CLensDialog::OnEnChangeEditCoefA5()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_A5, m_LensCorrect.lens_coefa[5], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefB5()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_B5, m_LensCorrect.lens_coefb[5], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefC5()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_C5, m_LensCorrect.lens_coefc[5], -511, 511);
}

void CLensDialog::OnEnChangeEditCoefA6()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_A6, m_LensCorrect.lens_coefa[6], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefB6()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_B6, m_LensCorrect.lens_coefb[6], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefC6()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_C6, m_LensCorrect.lens_coefc[6], -511, 511);
}

void CLensDialog::OnEnChangeEditCoefA7()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_A7, m_LensCorrect.lens_coefa[7], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefB7()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_B7, m_LensCorrect.lens_coefb[7], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefC7()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_C7, m_LensCorrect.lens_coefc[7], -511, 511);
}

void CLensDialog::OnEnChangeEditCoefA8()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_A8, m_LensCorrect.lens_coefa[8], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefB8()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_B8, m_LensCorrect.lens_coefb[8], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefC8()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_C8, m_LensCorrect.lens_coefc[8], -511, 511);
}

void CLensDialog::OnEnChangeEditCoefA9()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_A9, m_LensCorrect.lens_coefa[9], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefB9()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_B9, m_LensCorrect.lens_coefb[9], -1023, 1023);
}

void CLensDialog::OnEnChangeEditCoefC9()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_COEF_C9, m_LensCorrect.lens_coefc[9], -511, 511);
}

void CLensDialog::OnEnChangeEditRange0()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_RANGE0, m_LensCorrect.lens_range[0], 0, 2097152);
}

void CLensDialog::OnEnChangeEditRange1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_RANGE1, m_LensCorrect.lens_range[1], 0, 2097152);
}

void CLensDialog::OnEnChangeEditRange2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_RANGE2, m_LensCorrect.lens_range[2], 0, 2097152);
}

void CLensDialog::OnEnChangeEditRange3()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_RANGE3, m_LensCorrect.lens_range[3], 0, 2097152);
}

void CLensDialog::OnEnChangeEditRange4()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_RANGE4, m_LensCorrect.lens_range[4], 0, 2097152);
}

void CLensDialog::OnEnChangeEditRange5()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_RANGE5, m_LensCorrect.lens_range[5], 0, 2097152);
}

void CLensDialog::OnEnChangeEditRange6()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_RANGE6, m_LensCorrect.lens_range[6], 0, 2097152);
}

void CLensDialog::OnEnChangeEditRange7()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_RANGE7, m_LensCorrect.lens_range[7], 0, 2097152);
}

void CLensDialog::OnEnChangeEditRange8()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_RANGE8, m_LensCorrect.lens_range[8], 0, 2097152);
}

void CLensDialog::OnEnChangeEditRange9()
{
	// TODO:  如果该控件是 RICHEDIT 控件，则它将不会
	// 发送该通知，除非重写 __super::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

	EditBoxEnterProcess(IDC_EDIT_RANGE9, m_LensCorrect.lens_range[9], 0, 2097152);
}

void CLensDialog::OnBnClickedCheckLens()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bLensEnable = m_LensCheck.GetCheck();

	if (NULL == m_pMessageWnd) return;

	CBasePage::SendPageMessage(m_pMessageWnd, WM_ENABLE_CHANGE, ECT_LENS, m_bLensEnable);
}

int CLensDialog::SetLensEnable(BOOL bEnable)
{
	m_bLensEnable = bEnable;
	return 0;
}

int CLensDialog::GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen < sizeof(LENSCORRECT))) return -1;
	
	m_LensCorrect.enable = m_bLensEnable;
	nStLen = sizeof(LENSCORRECT);
	memcpy(pPageInfoSt, &m_LensCorrect, sizeof(LENSCORRECT));
	nPageID = m_nID;

	return 0;
}

int CLensDialog::SetPageInfoSt(void * pPageInfoSt, int nStLen)
{
	if ((pPageInfoSt == NULL) || (nStLen != sizeof(LENSCORRECT))) return -1;

	memcpy(&m_LensCorrect, pPageInfoSt, nStLen);

	if (m_LensCorrect.type != ISP_CID_LENS) return -1;

	m_bLensEnable = m_LensCorrect.enable;
	m_LensCheck.SetCheck(m_LensCorrect.enable);

	OnBnClickedCheckLens();
	
	CString strText;
	strText.Format(L"%d", m_LensCorrect.lens_xref);
	GetDlgItem(IDC_EDIT_XREF)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_yref);
	GetDlgItem(IDC_EDIT_YREF)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefa[0]);
	GetDlgItem(IDC_EDIT_COEF_A0)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefa[1]);
	GetDlgItem(IDC_EDIT_COEF_A1)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefa[2]);
	GetDlgItem(IDC_EDIT_COEF_A2)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefa[3]);
	GetDlgItem(IDC_EDIT_COEF_A3)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefa[4]);
	GetDlgItem(IDC_EDIT_COEF_A4)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefa[5]);
	GetDlgItem(IDC_EDIT_COEF_A5)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefa[6]);
	GetDlgItem(IDC_EDIT_COEF_A6)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefa[7]);
	GetDlgItem(IDC_EDIT_COEF_A7)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefa[8]);
	GetDlgItem(IDC_EDIT_COEF_A8)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefa[9]);
	GetDlgItem(IDC_EDIT_COEF_A9)->SetWindowText(strText);
	
	strText.Format(L"%d", m_LensCorrect.lens_coefb[0]);
	GetDlgItem(IDC_EDIT_COEF_B0)->SetWindowText(strText);
	
	strText.Format(L"%d", m_LensCorrect.lens_coefb[1]);
	GetDlgItem(IDC_EDIT_COEF_B1)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefb[2]);
	GetDlgItem(IDC_EDIT_COEF_B2)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefb[3]);
	GetDlgItem(IDC_EDIT_COEF_B3)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefb[4]);
	GetDlgItem(IDC_EDIT_COEF_B4)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefb[5]);
	GetDlgItem(IDC_EDIT_COEF_B5)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefb[6]);
	GetDlgItem(IDC_EDIT_COEF_B6)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefb[7]);
	GetDlgItem(IDC_EDIT_COEF_B7)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefb[8]);
	GetDlgItem(IDC_EDIT_COEF_B8)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefb[9]);
	GetDlgItem(IDC_EDIT_COEF_B9)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefc[0]);
	GetDlgItem(IDC_EDIT_COEF_C0)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefc[1]);
	GetDlgItem(IDC_EDIT_COEF_C1)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefc[2]);
	GetDlgItem(IDC_EDIT_COEF_C2)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefc[3]);
	GetDlgItem(IDC_EDIT_COEF_C3)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefc[4]);
	GetDlgItem(IDC_EDIT_COEF_C4)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefc[5]);
	GetDlgItem(IDC_EDIT_COEF_C5)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefc[6]);
	GetDlgItem(IDC_EDIT_COEF_C6)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefc[7]);
	GetDlgItem(IDC_EDIT_COEF_C7)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefc[8]);
	GetDlgItem(IDC_EDIT_COEF_C8)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_coefc[9]);
	GetDlgItem(IDC_EDIT_COEF_C9)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_range[0]);
	GetDlgItem(IDC_EDIT_RANGE0)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_range[1]);
	GetDlgItem(IDC_EDIT_RANGE1)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_range[2]);
	GetDlgItem(IDC_EDIT_RANGE2)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_range[3]);
	GetDlgItem(IDC_EDIT_RANGE3)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_range[4]);
	GetDlgItem(IDC_EDIT_RANGE4)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_range[5]);
	GetDlgItem(IDC_EDIT_RANGE5)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_range[6]);
	GetDlgItem(IDC_EDIT_RANGE6)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_range[7]);
	GetDlgItem(IDC_EDIT_RANGE7)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_range[8]);
	GetDlgItem(IDC_EDIT_RANGE8)->SetWindowText(strText);

	strText.Format(L"%d", m_LensCorrect.lens_range[9]);
	GetDlgItem(IDC_EDIT_RANGE9)->SetWindowText(strText);
	
	return 0;
}

int CLensDialog::Clear()
{
	m_bLensEnable = FALSE;
	ZeroMemory(&m_LensCorrect, sizeof(LENSCORRECT));
	m_LensCorrect.type = ISP_CID_LENS;

	m_LensCheck.SetCheck(0);
	return 0;
}

void CLensDialog::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码
	if (bShow)
		m_LensCheck.SetCheck(m_bLensEnable);
}

BOOL CLensDialog::PreTranslateMessage(MSG * pMsg)
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

void CLensDialog::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (NULL == m_pMessageWnd) return;
	CBasePage::SendPageMessage(m_pMessageWnd, WM_SUBMISSION, ECT_LENS, 0);
}
