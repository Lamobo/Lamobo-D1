#pragma once


// CPreviewDialog 对话框

class CPreviewDialog : public CDialog
{
	DECLARE_DYNAMIC(CPreviewDialog)

public:
	CPreviewDialog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPreviewDialog();

// 对话框数据
	enum { IDD = IDD_DIALOG_PREVIEW1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};
