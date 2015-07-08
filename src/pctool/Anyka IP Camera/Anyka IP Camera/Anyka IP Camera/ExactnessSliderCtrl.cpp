#include "StdAfx.h"
#include "ExactnessSliderCtrl.h"

IMPLEMENT_DYNAMIC(CExactnessSliderCtrl, CSliderCtrl) 

CExactnessSliderCtrl::CExactnessSliderCtrl(void)
{
	m_bLBDown = FALSE;
	m_bDrag = FALSE;
}

CExactnessSliderCtrl::~CExactnessSliderCtrl(void)
{
}

BEGIN_MESSAGE_MAP(CExactnessSliderCtrl, CSliderCtrl)
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CExactnessSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{	
	CRect rect;
	GetThumbRect(rect);

	if (rect.PtInRect(point)) {
		m_bDrag = TRUE;
		return;
	}

	GetChannelRect(&rect);

	if (!rect.PtInRect(point)) return;

	m_bLBDown = TRUE;

	CWnd * pP = GetParent();
	pP->SendMessage(WM_SCROLL_BEGIN, 0, (LPARAM)this);

	int nMax = GetRangeMax();
	float fInterval = float(rect.Width()) / nMax;
	float fPos = (point.x - rect.left + fInterval - 1) / fInterval;

	CSliderCtrl::SetPos((int)fPos);
}

void CExactnessSliderCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_bDrag || m_bLBDown) {
		CWnd * pP = GetParent();
		pP->SendMessage(WM_SCROLL_END, 0, (LPARAM)this);
	}

	m_bDrag = FALSE;
	m_bLBDown = FALSE;
}

void CExactnessSliderCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_bDrag && (nFlags == MK_LBUTTON)) {
		CWnd * pP = GetParent();
		CRect rect;
		GetClientRect(&rect);

		if (!rect.PtInRect(point)) {
			m_bDrag = FALSE;
			m_bLBDown = FALSE;
			pP->SendMessage(WM_SCROLL_END, 0, (LPARAM)this);
			return;
		}

		int nMax = GetRangeMax();
		float fInterval = float(rect.Width()) / nMax;
		float fPos = (point.x - rect.left + fInterval - 1) / fInterval;

		CSliderCtrl::SetPos((int)fPos);

		pP->SendMessage(WM_THUMB_TRACK, (int)fPos, (LPARAM)this);
	}
}