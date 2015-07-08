#pragma once

#include "BasePage.h"
#include "afxwin.h"
#include <deque>

using namespace std;

#define  ISP_CID_SET_SENSOR_PARAM      0x75    //set sensor parameter

typedef struct isp_config_sensor_reg {
         int type;
         int enable;			//表示使用,　设置为1
         unsigned int cmd;		//命令,　即sensor寄存器地址
         unsigned int data;		//要写的值
}SENSOR_REG;

#define REGISTER_MAX	50

// CRegisterDialog 对话框

class CRegisterDialog : public CDialog, public CBasePage
{
	DECLARE_DYNAMIC(CRegisterDialog)

public:
	CRegisterDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRegisterDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_REGISTER };
	
	int GetPageInfoStCount();
	virtual int GetPageInfoSent(int & nPageID, void * pPageInfoSt, int & nStLen);

	virtual int GetPageInfoSt(int & nPageID, void * pPageInfoSt, int & nStLen);
	virtual int SetPageInfoSt(void * pPageInfoSt, int nStLen);

	virtual BOOL GetPageEnable();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG * pMsg);
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedButton1();

	void UpdateRegister();

	DECLARE_MESSAGE_MAP()

private:
	CEdit m_RegisterEdit;
	CString m_strText;
	deque<SENSOR_REG> m_listRegister;
	int m_iIndex;
};
