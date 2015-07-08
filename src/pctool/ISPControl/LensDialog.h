#pragma once

#include "BasePage.h"
#include "afxwin.h"

#define  ISP_CID_LENS				0x61		//lens correct

typedef struct isp_lens_correct {
	int type;
	int	enable;					//register table 0x20: offset register 24 [31]
	int lens_coefa[10];			//register table 0x20: start offset register 13 [21:0]
	int lens_coefb[10];
	int lens_coefc[10];
	unsigned int lens_range[10];
	unsigned int lens_xref;		//register table 0x20: offset register 23 [31:22]
	unsigned int lens_yref;		//register table 0x20: offset register 23 [31:22]
}LENSCORRECT;

// CLensDialog 对话框

class CLensDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CLensDialog)

public:
	CLensDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CLensDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_LENS };
	
	int SetLensEnable(BOOL bEnable);
	BOOL GetLensEnable() {return m_bLensEnable;};

	virtual int GetPageEnable() {return m_bLensEnable;};

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual int Clear();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnEnChangeEditXref();
	afx_msg void OnEnChangeEditYref();
	afx_msg void OnEnChangeEditCoefA0();
	afx_msg void OnEnChangeEditCoefB0();
	afx_msg void OnEnChangeEditCoefC0();
	afx_msg void OnEnChangeEditCoefA1();
	afx_msg void OnEnChangeEditCoefB1();
	afx_msg void OnEnChangeEditCoefC1();
	afx_msg void OnEnChangeEditCoefA2();
	afx_msg void OnEnChangeEditCoefB2();
	afx_msg void OnEnChangeEditCoefC2();
	afx_msg void OnEnChangeEditCoefA3();
	afx_msg void OnEnChangeEditCoefB3();
	afx_msg void OnEnChangeEditCoefC3();
	afx_msg void OnEnChangeEditCoefA4();
	afx_msg void OnEnChangeEditCoefB4();
	afx_msg void OnEnChangeEditCoefC4();
	afx_msg void OnEnChangeEditCoefA5();
	afx_msg void OnEnChangeEditCoefB5();
	afx_msg void OnEnChangeEditCoefC5();
	afx_msg void OnEnChangeEditCoefA6();
	afx_msg void OnEnChangeEditCoefB6();
	afx_msg void OnEnChangeEditCoefC6();
	afx_msg void OnEnChangeEditCoefA7();
	afx_msg void OnEnChangeEditCoefB7();
	afx_msg void OnEnChangeEditCoefC7();
	afx_msg void OnEnChangeEditCoefA8();
	afx_msg void OnEnChangeEditCoefB8();
	afx_msg void OnEnChangeEditCoefC8();
	afx_msg void OnEnChangeEditCoefA9();
	afx_msg void OnEnChangeEditCoefB9();
	afx_msg void OnEnChangeEditCoefC9();
	afx_msg void OnEnChangeEditRange0();
	afx_msg void OnEnChangeEditRange1();
	afx_msg void OnEnChangeEditRange2();
	afx_msg void OnEnChangeEditRange3();
	afx_msg void OnEnChangeEditRange4();
	afx_msg void OnEnChangeEditRange5();
	afx_msg void OnEnChangeEditRange6();
	afx_msg void OnEnChangeEditRange7();
	afx_msg void OnEnChangeEditRange8();
	afx_msg void OnEnChangeEditRange9();
	afx_msg void OnBnClickedCheckLens();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButton1();

	int EditBoxNumberStringProcess(CString strEBText, int nEditBoxID);
	inline int EditBoxNumberProcess(int nNumber, int nMin, int nMax);
	void EditBoxEnterProcess(int nEditBoxID, int & nValue, int nMin, int nMax);
	void EditBoxEnterProcess(int nEditBoxID, unsigned int & nValue, int nMin, int nMax);

private:
	LENSCORRECT m_LensCorrect;
	BOOL m_bLensEnable;
	CButton m_LensCheck;
};
