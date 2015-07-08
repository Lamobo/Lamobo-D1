#pragma once


// CInformWaitDialog 对话框

class CInformWaitDialog : public CDialog
{
	DECLARE_DYNAMIC(CInformWaitDialog)

public:
	CInformWaitDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CInformWaitDialog();

// 对话框数据
	enum { IDD = IDD_WAITDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
