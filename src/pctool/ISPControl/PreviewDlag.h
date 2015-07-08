#pragma once


// CPreviewDlag 对话框

class CPreviewDlag : public CDialog
{
	DECLARE_DYNAMIC(CPreviewDlag)

public:
	CPreviewDlag(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CPreviewDlag();

// 对话框数据
	enum { IDD = IDD_DIALOG_PREVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
