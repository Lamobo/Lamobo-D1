// BurnToolView.h : interface of the CBurnToolView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BURNTOOLVIEW_H__1B1828DC_A39B_455F_87A0_FEE50D8A9362__INCLUDED_)
#define AFX_BURNTOOLVIEW_H__1B1828DC_A39B_455F_87A0_FEE50D8A9362__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//状态条颜色
#define COLOR_STAT_INITIAL	RGB(100, 100, 100)
#define COLOR_STAT_STANDBY	RGB(244, 244, 19)
#define COLOR_STAT_COMPLETE	RGB(0, 200, 0)
#define COLOR_STAT_FAIL		RGB(255, 0, 0)
#define COLOR_STAT_BURNING	RGB(0, 0, 255)

//进度条颜色
#define COLOR_PROG_BACK		RGB(236, 233, 236)
#define COLOR_PROG_NORMAL	RGB(0, 0, 255)
#define COLOR_PROG_FAIL		RGB(255, 0, 0)
#define COLOR_PROG_COMPLETE	RGB(0, 200, 0)

class CBurnToolView : public CListView
{
protected: // create from serialization only
	CBurnToolView();
	DECLARE_DYNCREATE(CBurnToolView)

// Attributes
public:
	CBurnToolDoc* GetDocument();

// Operations
public:
	int m_cxClient;

	COLORREF m_clrText;
	COLORREF m_clrTextBk;
	COLORREF m_clrBkgnd;	
	COLORREF m_clrHText;
	COLORREF m_clrHBkgnd;
	COLORREF m_clrForeProg;
    COLORREF m_clrBackProg;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBurnToolView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBurnToolView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	void SetupListView(UINT count);

	void SetNumDesp(UINT nItem, CString sNumDesp);
	void SetProgPos(UINT nItem, UINT nProg);
	void SetProgColor(UINT  nItem, COLORREF color);
	void SetStatColor(UINT nItem, COLORREF cColor);
	void SetTime(UINT nItem, UINT nTime);
	void SetStatDesp(UINT nItem, CString sStatDesp);
	void SetCurrentMacAddr(UINT nItem, CString str);
	void SetCurrentSequenceAddr(UINT nItem, CString str);
	void SetupDisplay();

protected:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	

//attibutes
	typedef struct
	{
		UINT		progress;
		COLORREF	color_stat;
		COLORREF	color_prog;
		UINT		time;
	}T_ITEM_PROP;

	UINT		m_item_count;
	CArray<T_ITEM_PROP, T_ITEM_PROP> m_itemprop_arrray;

//operations
	void DrawBkColor(UINT nItem, BOOL bSelected, CDC *pDC);
	void DrawProg(UINT nItem, CRect rcSubItem, BOOL bSelected, CDC *pDC);
	void DrawStatColor(UINT nItem, CRect rcSubItem, CDC *pDC);
	void DrawItemText(UINT nItem, CRect rcSubItem, CString sItem, 
					BOOL bSelected, UINT nJustify, CDC *pDC);

//private
	void GetTimeString(UINT nTime, CString &sTime);

// Generated message map functions
protected:
	//{{AFX_MSG(CBurnToolView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in BurnToolView.cpp
inline CBurnToolDoc* CBurnToolView::GetDocument()
   { return (CBurnToolDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BURNTOOLVIEW_H__1B1828DC_A39B_455F_87A0_FEE50D8A9362__INCLUDED_)
