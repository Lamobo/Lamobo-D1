#pragma once
#include "afxcmn.h"

#define WM_SCROLL_BEGIN		WM_USER + 500
#define WM_SCROLL_END		WM_USER + 501
#define WM_THUMB_TRACK		WM_USER + 502

class CExactnessSliderCtrl :
	public CSliderCtrl
{

	DECLARE_DYNAMIC(CExactnessSliderCtrl)

public:
	CExactnessSliderCtrl(void);
	~CExactnessSliderCtrl(void);

protected:

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bDrag, m_bLBDown;
};
