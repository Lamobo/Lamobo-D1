// BurnToolView.cpp : implementation of the CBurnToolView class
//

#include "stdafx.h"
#include "BurnTool.h"

#include "BurnToolDoc.h"
#include "BurnToolView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define OFFSET_FIRST    2
#define OFFSET_OTHER    6

extern CConfig theConfig;
extern CBurnToolApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CBurnToolView

IMPLEMENT_DYNCREATE(CBurnToolView, CListView)

BEGIN_MESSAGE_MAP(CBurnToolView, CListView)
	//{{AFX_MSG_MAP(CBurnToolView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_WM_MEASUREITEM_REFLECT()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBurnToolView construction/destruction

CBurnToolView::CBurnToolView()
{
	// TODO: add construction code here
	m_cxClient = 0;
	m_clrText = ::GetSysColor(COLOR_WINDOWTEXT);
	m_clrTextBk = ::GetSysColor(COLOR_WINDOW);
	m_clrBkgnd = ::GetSysColor(COLOR_WINDOW);
	m_clrHText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	m_clrHBkgnd = ::GetSysColor(COLOR_HIGHLIGHT);
//	m_clrForeProg = ::GetSysColor(COLOR_HOTLIGHT);
	m_clrBackProg = ::GetSysColor(COLOR_BTNFACE);
}

CBurnToolView::~CBurnToolView()
{
}

BOOL CBurnToolView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= LVS_REPORT | LVS_OWNERDRAWFIXED;
	
	return CListView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CBurnToolView drawing
void CBurnToolView::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	lpMeasureItemStruct->itemHeight = 32;
}

void CBurnToolView::OnDraw(CDC* pDC)
{
	CBurnToolDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

void CBurnToolView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();


	// TODO: You may populate your ListView with items by directly accessing
	//  its list control through a call to GetListCtrl().

	CListCtrl& ListCtrl = GetListCtrl();
	ListView_SetExtendedListViewStyle(ListCtrl.GetSafeHwnd(),  LVS_EX_FULLROWSELECT);

	ListCtrl.InsertColumn(0, theApp.GetString(IDS_MAINVIEW_STATUS_IMAGE), LVCFMT_LEFT, 50);//状态图
	ListCtrl.InsertColumn(1, theApp.GetString(IDS_MAINVIEW_NUMBER), LVCFMT_LEFT, 80);//个数
	ListCtrl.InsertColumn(2, theApp.GetString(IDS_MAINVIEW_PROGRESS), LVCFMT_LEFT, 200);//进程
	ListCtrl.InsertColumn(3, theApp.GetString(IDS_MAINVIEW_STATUS_TEXT), LVCFMT_LEFT, 250);
	ListCtrl.InsertColumn(4, theApp.GetString(IDS_MAINVIEW_TIMER), LVCFMT_LEFT, 80);//时间
	ListCtrl.InsertColumn(5, theApp.GetString(IDS_MAINVIEW_SEQUENCE), LVCFMT_LEFT, 150);//序列号
	ListCtrl.InsertColumn(6, theApp.GetString(IDS_MAINVIEW_MAC_ADDR), LVCFMT_LEFT, 150);//MAC

	m_itemprop_arrray.SetSize(16, 16);//大小

	SetupListView(theConfig.device_num);//通道设置
}

/////////////////////////////////////////////////////////////////////////////
// CBurnToolView diagnostics

#ifdef _DEBUG
void CBurnToolView::AssertValid() const
{
	CListView::AssertValid();
}

void CBurnToolView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CBurnToolDoc* CBurnToolView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBurnToolDoc)));
	return (CBurnToolDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBurnToolView message handlers
void CBurnToolView::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CListCtrl& ListCtrl=GetListCtrl();
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	UINT nItem = lpDrawItemStruct->itemID;
	CRect rcItem(lpDrawItemStruct->rcItem);
	BOOL bFocus = (GetFocus() == this);

	static _TCHAR szBuff[MAX_PATH];
	
	LV_ITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = nItem;
	lvi.iSubItem = 0;
	lvi.pszText = szBuff;
	lvi.cchTextMax = sizeof(szBuff);
	lvi.stateMask = 0xFFFF;     // get all state flags
	ListCtrl.GetItem(&lvi);

	BOOL bSelected = (bFocus || (GetStyle() & LVS_SHOWSELALWAYS)) && lvi.state & LVIS_SELECTED;
	bSelected = bSelected || (lvi.state & LVIS_DROPHILITED);
	
// set colors if item is selected
	DrawBkColor(nItem, bSelected, pDC);

// Draw StatColor
	CRect rcLabel;
	ListCtrl.GetItemRect(nItem, rcLabel, LVIR_LABEL);
	DrawStatColor(nItem, rcLabel, pDC);

//Draw other Item
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH;

	CRect rcSubItem = rcLabel;

	for(int nColumn = 1; ListCtrl.GetColumn(nColumn, &lvc); nColumn++)
	{
		rcSubItem.left = rcSubItem.right;
		rcSubItem.right += lvc.cx;

		if(2 == nColumn)
		{
			DrawProg(nItem, rcSubItem, bSelected, pDC);
		}
		else
		{
			CString sItem = ListCtrl.GetItemText(nItem, nColumn);
			UINT nJustify = DT_LEFT;

			if(sItem.IsEmpty())
			{
				continue;
			}

			switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
			{
				case LVCFMT_RIGHT:
					nJustify = DT_RIGHT;
					break;
				case LVCFMT_CENTER:
					nJustify = DT_CENTER;
					break;
				default:
					break;
			}
			///
			DrawItemText(nItem, rcSubItem, sItem, bSelected, nJustify, pDC);
		}
	}
}
//画色
void CBurnToolView::DrawBkColor(UINT nItem, BOOL bSelected, CDC *pDC)
{
	CListCtrl &ListCtrl = GetListCtrl();
	COLORREF clrBkSave;
	
	CRect rcAllLabels;
	ListCtrl.GetItemRect(nItem, rcAllLabels, LVIR_BOUNDS);

//	CRect rcLabel;
//	ListCtrl.GetItemRect(nItem, rcLabel, LVIR_LABEL);

//	rcAllLabels.left = rcLabel.left;
	if (rcAllLabels.right<m_cxClient)
	{
		rcAllLabels.right = m_cxClient;
	}

	if (bSelected)
	{	
		clrBkSave = pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));

		pDC->FillRect(rcAllLabels, &CBrush(::GetSysColor(COLOR_HIGHLIGHT)));
		//SetBkColor
		pDC->SetBkColor(clrBkSave);
	}
	else
	{
		//FillRect
		pDC->FillRect(rcAllLabels, &CBrush(m_clrTextBk));
	}
}

//画图
void CBurnToolView::DrawProg(UINT nItem, CRect rcSubItem, BOOL bSelected, CDC *pDC)
{
	CRect rcProg = rcSubItem;
	T_ITEM_PROP item_prop = m_itemprop_arrray[nItem];
	CString sItem;
	
	sItem.Format(_T("%d%%"), item_prop.progress);

	//shrink two sides of the rect
	rcProg.DeflateRect(5, 5);

	//Draw outside border of progress bar
	pDC->Rectangle(rcProg);
	//DeflateRect
	rcProg.DeflateRect(1, 1);

	//calulate the size of two parts of progress bar
	CRect rcLeft, rcRight;
	rcLeft = rcRight = rcProg;

	//right and  left
	rcLeft.right = rcLeft.left + MulDiv(item_prop.progress, rcProg.Width(), 100);
	rcRight.left = rcLeft.right;

	if(bSelected)
	{
		pDC->FillSolidRect(rcLeft, RGB(200,200,200)); //FillSolidRect
		pDC->FillSolidRect(rcRight, m_clrBkgnd); //FillSolidRect
		pDC->SetTextColor(m_clrText); //SetTextColor
		pDC->DrawText(sItem, rcProg, DT_VCENTER|DT_CENTER|DT_SINGLELINE);
	}
	else
	{
		pDC->FillSolidRect(rcLeft, item_prop.color_prog);
		pDC->FillSolidRect(rcRight, COLOR_PROG_BACK);
		
		CRgn rgn;
		rgn.CreateRectRgnIndirect(rcLeft);
		pDC->SelectClipRgn(&rgn); // SelectClipRgn
		pDC->SetTextColor(m_clrBkgnd); // SetTextColor
		pDC->DrawText(sItem, rcProg, DT_VCENTER|DT_CENTER|DT_SINGLELINE);

		rgn.SetRectRgn(rcRight);
		pDC->SelectClipRgn(&rgn);//SelectClipRgn
		pDC->SetTextColor(m_clrText);//SetTextColor
		pDC->DrawText(sItem, rcProg, DT_VCENTER|DT_CENTER|DT_SINGLELINE);
		pDC->SelectClipRgn(NULL); //SelectClipRgn
	}
}

void CBurnToolView::DrawStatColor(UINT nItem, CRect rcSubItem, CDC *pDC)
{
	CRect rcColor = rcSubItem;
	rcColor.DeflateRect(10, 5); //DeflateRect

	T_ITEM_PROP item_prop;
	item_prop = m_itemprop_arrray[nItem]; //m_itemprop_arrray

//	HICON hicon = AfxGetApp()->LoadIcon(IDI_ICON_RIGHT);
//	pDC->DrawIcon(rcSubItem.left, rcSubItem.top, hicon);

	CBrush brush(item_prop.color_stat);
	pDC->FillRect(&rcColor, &brush);
}

void CBurnToolView::DrawItemText(UINT nItem, CRect rcSubItem, CString sItem, 
								 BOOL bSelected, UINT nJustify, CDC *pDC)
{
	CRect rcLabel = rcSubItem;
	rcLabel.left += OFFSET_OTHER;
	rcLabel.right -= OFFSET_OTHER;

	COLORREF clrTextSave = 0;
	
	if(bSelected)
	{
		//SetTextColor
		clrTextSave = pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
	}

	pDC->DrawText(sItem, rcLabel,
				nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);

	if(bSelected)
	{
		//SetTextColor
		pDC->SetTextColor(clrTextSave);
	}
}

void CBurnToolView::SetupListView(UINT count)
{
	CListCtrl &ListCtrl = GetListCtrl();

	ListCtrl.DeleteAllItems(); //DeleteAllItems
	m_itemprop_arrray.RemoveAll();// RemoveAll

	TCHAR macbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR serialbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	T_ITEM_PROP item_prop;
	item_prop.color_stat = COLOR_STAT_INITIAL;
	item_prop.color_prog = COLOR_PROG_NORMAL;
	item_prop.progress = 0;
	item_prop.time = 0;

	CString str;
	for(UINT i = 0; i < count; i++)
	{
		//插入
		ListCtrl.InsertItem(i, NULL);
		
		str.Format(_T("%s%d"), theApp.GetString(IDS_MAINVIEW_DEVICE), i);
		ListCtrl.SetItemText(i, 1, str);

		GetTimeString(item_prop.time, str);
		ListCtrl.SetItemText(i, 4, str);
		//add 
		m_itemprop_arrray.Add(item_prop);
		//第5
		ListCtrl.SetItemText(i, 5, _T(""));
		//第6
		ListCtrl.SetItemText(i, 6, _T(""));
	}
}

void CBurnToolView::OnSize(UINT nType, int cx, int cy) 
{
	CListView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	m_cxClient = cx;
}
//设置进程位置
void CBurnToolView::SetProgPos(UINT nItem, UINT nProg)
{
	UINT count  = m_itemprop_arrray.GetSize();

	if(nItem >= count)
	{
		return;
	}

	if(m_itemprop_arrray[nItem].progress != nProg)
	{
		m_itemprop_arrray[nItem].progress = nProg;

		//For Update ... do not know why
		CListCtrl &ListCtrl = GetListCtrl();
		ListCtrl.SetItemText(nItem, 2, _T(""));
	}
}
//进程变色
void CBurnToolView::SetProgColor(UINT nItem, COLORREF color)
{
	m_itemprop_arrray[nItem].color_prog = color;

	//For Update ... do not know why
	CListCtrl &ListCtrl = GetListCtrl();
	ListCtrl.SetItemText(nItem, 2, _T(""));
}
//开始变色
void CBurnToolView::SetStatColor(UINT nItem, COLORREF cColor)
{
	UINT count = m_itemprop_arrray.GetSize();

	if(nItem >= count)
	{
		return;
	}

	m_itemprop_arrray[nItem].color_stat = cColor;
	
	//For Update ... do not know why
	CListCtrl &ListCtrl = GetListCtrl();
	ListCtrl.SetItemText(nItem, 0, _T(""));
}

void CBurnToolView::SetStatDesp(UINT nItem, CString sStatDesp)
{
	UINT count = m_itemprop_arrray.GetSize();

	if(nItem >= count)
	{
		return;
	}
	
	CListCtrl &ListCtrl = GetListCtrl();
	ListCtrl.SetItemText(nItem, 3, sStatDesp);
}
//设置时间
void CBurnToolView::SetTime(UINT nItem, UINT nTime)
{
	UINT count = m_itemprop_arrray.GetSize();

	if(nItem >= count)
	{
		return;
	}

	m_itemprop_arrray[nItem].time = nTime;
	
	//For Update ... do not know why
	CListCtrl &ListCtrl = GetListCtrl();

	CString sTime;
	GetTimeString(nTime, sTime);
	ListCtrl.SetItemText(nItem, 4, sTime);
}
//获取时间格式
void CBurnToolView::GetTimeString(UINT nTime, CString &sTime)
{
	UINT hour, minute, second;
	
	hour = nTime / 3600;
	minute = (nTime - hour*3600)/60;
	second = nTime - hour*3600 - minute*60;

	sTime.Format(_T("%d:%2d:%2d"), hour, minute, second);
	sTime.Replace(' ', '0');
}

//序列号
void CBurnToolView::SetCurrentSequenceAddr(UINT nItem, CString str)
{
	CListCtrl &ListCtrl = GetListCtrl();

	ListCtrl.SetItemText(nItem, 5, str);
}
//mac
void CBurnToolView::SetCurrentMacAddr(UINT nItem, CString str)
{
	CListCtrl &ListCtrl = GetListCtrl();

	ListCtrl.SetItemText(nItem, 6, str);
}


void CBurnToolView::SetupDisplay()
{
	CListCtrl &ListCtrl = GetListCtrl();

	int i;

	ListCtrl.DeleteAllItems();
	
	for(i = 0; i < 7; i++)
	{
		ListCtrl.DeleteColumn(0);
	}
	//烧录状态
	ListCtrl.InsertColumn(0, theApp.GetString(IDS_MAINVIEW_STATUS_IMAGE), LVCFMT_LEFT, 50);
	ListCtrl.InsertColumn(1, theApp.GetString(IDS_MAINVIEW_NUMBER), LVCFMT_LEFT, 80);//烧录个数
	ListCtrl.InsertColumn(2, theApp.GetString(IDS_MAINVIEW_PROGRESS), LVCFMT_LEFT, 200);//进程
	ListCtrl.InsertColumn(3, theApp.GetString(IDS_MAINVIEW_STATUS_TEXT), LVCFMT_LEFT, 250);
	ListCtrl.InsertColumn(4, theApp.GetString(IDS_MAINVIEW_TIMER), LVCFMT_LEFT, 80);//时间
	ListCtrl.InsertColumn(5, theApp.GetString(IDS_MAINVIEW_SEQUENCE), LVCFMT_LEFT, 150);//序列号
	ListCtrl.InsertColumn(6, theApp.GetString(IDS_MAINVIEW_MAC_ADDR), LVCFMT_LEFT, 150);//MAC

	m_itemprop_arrray.SetSize(16, 16);

	SetupListView(theConfig.device_num);

	ListCtrl.Invalidate();
}