// PreviewDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "Anyka IP Camera.h"
#include "PreviewDialog.h"

// CPreviewDialog 对话框

IMPLEMENT_DYNAMIC(CPreviewDialog, CDialog)

CPreviewDialog::CPreviewDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CPreviewDialog::IDD, pParent)
{
}

CPreviewDialog::~CPreviewDialog()
{
}

void CPreviewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPreviewDialog, CDialog)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


BOOL CPreviewDialog::OnInitDialog()
{
	CDialog::OnInitDialog();
	return TRUE;
}

// CPreviewDialog 消息处理程序

void CPreviewDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd * pP = GetParent();
	ClientToScreen(&point);
	pP->ScreenToClient(&point);
	pP->SendMessage(WM_LBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));
	CDialog::OnLButtonDown(nFlags, point);
}

void CPreviewDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd * pP = GetParent();
	ClientToScreen(&point);
	pP->ScreenToClient(&point);
	pP->SendMessage(WM_LBUTTONUP, nFlags, MAKELPARAM(point.x, point.y));

	CDialog::OnLButtonUp(nFlags, point);
}

void CPreviewDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd * pP = GetParent();
	ClientToScreen(&point);
	pP->ScreenToClient(&point);
	pP->SendMessage(WM_MOUSEMOVE, nFlags, MAKELPARAM(point.x, point.y));

	CDialog::OnMouseMove(nFlags, point);
}

void CPreviewDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CWnd * pP = GetParent();
	ClientToScreen(&point);
	pP->ScreenToClient(&point);
	pP->SendMessage(WM_RBUTTONDOWN, nFlags, MAKELPARAM(point.x, point.y));

	CDialog::OnRButtonDown(nFlags, point);
}

void CPreviewDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CWnd * pP = GetParent();
	ClientToScreen(&point);
	pP->ScreenToClient(&point);
	pP->SendMessage(WM_LBUTTONDBLCLK, nFlags, MAKELPARAM(point.x, point.y));

	CDialog::OnLButtonDblClk(nFlags, point);
}
