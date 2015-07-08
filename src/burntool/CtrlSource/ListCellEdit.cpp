// ListCellEdit.cpp : implementation file
//

#include "stdafx.h"
#include "ListCellEdit.h"
#include "ListCtrlEx.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL m_precharchinese;
BOOL m_containchines;

/////////////////////////////////////////////////////////////////////////////
// CListCellEdit

CListCellEdit::CListCellEdit()
{
}

CListCellEdit::	CListCellEdit(int nItem, int nSubItem, CString strInitText)
:m_bEscape (FALSE)
{
    m_nItem = nItem;//二级菜单
    m_nSubItem = nSubItem;//一级菜单
    m_strInitText = strInitText;
	m_left = 0;
	m_precharchinese = FALSE;
	m_containchines = FALSE;
}

CListCellEdit::~CListCellEdit()
{
}


BEGIN_MESSAGE_MAP(CListCellEdit, CEdit)
	//{{AFX_MSG_MAP(CListCellEdit)
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CHAR()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCellEdit message handlers

BOOL CListCellEdit::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class

	if( pMsg->message == WM_KEYDOWN )
	{
		if (pMsg->wParam == VK_DOWN)
		{
			CListCtrlEx *pList = (CListCtrlEx *)GetParent();
			int nIndex = pList->GetItemIndex()+1;
			int nSubIndex = pList->GetSubItemIndex();
			
			if (nIndex >= pList->GetItemCount())
			{
				HWND hWnd = pList->GetParent()->GetSafeHwnd();
//				::SendMessage(hWnd, WM_COMMAND, IDC_BUTTON3, 0);
			}
			pList->EditSubItem(nIndex, nSubIndex);
			return TRUE;
		}
		
		if (pMsg->wParam == VK_UP)
		{
			CListCtrlEx *pList = (CListCtrlEx *)GetParent();
			int nIndex = pList->GetItemIndex()-1;
			int nSubIndex = pList->GetSubItemIndex();
			
			if (nIndex < 0)
				return TRUE;
			pList->EditSubItem(nIndex, nSubIndex);
			return TRUE;
		}
		
		if(pMsg->wParam == VK_RETURN
			|| pMsg->wParam == VK_TAB
			|| pMsg->wParam == VK_BACK
			|| pMsg->wParam == VK_DELETE
			|| pMsg->wParam == VK_ESCAPE
			|| GetKeyState( VK_CONTROL)
			)
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;		    	// DO NOT process further
		}
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}

void CListCellEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	SetListItemText();
	
    DestroyWindow();
	
}

void CListCellEdit::OnNcDestroy() 
{
	CEdit::OnNcDestroy();
	
	// TODO: Add your message handler code here
	delete this;
	
}

void CListCellEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default

	CListCtrlEx *pList = (CListCtrlEx *)GetParent();
	int index = pList->GetSubItemIndex();

	if(m_precharchinese)
	{
		m_precharchinese = FALSE;
		return;
	}

	
	if( nChar == VK_ESCAPE || nChar == VK_RETURN)
	{
		if( nChar == VK_ESCAPE )
			m_bEscape = TRUE;
		pList->SetFocus();		
		return;
	}
	
	if (nChar == VK_TAB)
	{
		pList = (CListCtrlEx *)GetParent();
		pList->MoveNextItem();
		return;
	}


	TCHAR buf[100];
	CString str("");
	//GetWindowText(str);
	GetWindowText(buf , 100);
	str = buf;
	int len = str.GetLength();
	int wlen= _tcslen(buf);



	if (nChar == VK_BACK)
	{
		str = str.Left(str.GetLength()-1);
		
		wlen--;
	}




	if (nChar & 0x80)
	{
        /*
		if (wlen >=4)
		{
			m_precharchinese = TRUE;
			return;
		}
        */
	}
	else
	{
		if(m_containchines)
			if(wlen >= 4)
				return;
	}

	

	
	CWindowDC dc(this);
	CFont *pFont = GetParent()->GetFont();
	CFont *pFontDC = dc.SelectObject( pFont );
	CSize size = dc.GetTextExtent( str );
	dc.SelectObject( pFontDC );
	size.cx += 5;			   	// add some extra buffer
	
	// Get client rect
	CRect rect, parentrect;
	GetClientRect( &rect );
	pList->GetClientRect( &parentrect );
	
	// Transform rect to parent coordinates
	ClientToScreen( &rect );
	pList->ScreenToClient( &rect );
	
	// Check whether control needs to be resized
	// and whether there is space to grow
	if( size.cx > m_width )
	{
		if( size.cx + rect.left < parentrect.right )
			rect.right = m_left + size.cx - 5;
		else
			rect.right = parentrect.right;
		MoveWindow( &rect );
	}
	
	//here we should update the list content after each char input
	SetListItemText();
	
	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

int CListCellEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	CFont* font = GetParent()->GetFont();
	SetFont(font);
	
	SetWindowText( m_strInitText );
	SetFocus();//设置焦点
	SetSel( 0, -1 );
	
	CRect rect;
	GetClientRect( &rect );
	ClientToScreen( &rect );
	GetParent()->ScreenToClient( &rect );
	m_left = rect.left;
	m_width = rect.right - rect.left;
	
	return 0;
}

void CListCellEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default

	if (nChar == VK_DELETE)
	{
		//here we should update the list content after each char input
		CEdit::OnKeyDown( nChar, nRepCnt, nFlags );
		SetListItemText();
		return;
	}
	
	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CListCellEdit::SetListItemText()
{
    CString Text;
    GetWindowText (Text);
	
    // Send Notification to parent of ListView ctrl
    LV_DISPINFO dispinfo;
    dispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
    dispinfo.hdr.idFrom = GetDlgCtrlID();
    dispinfo.hdr.code = LVN_ENDLABELEDIT;
	
    dispinfo.item.mask = LVIF_TEXT;
    dispinfo.item.iItem = m_nItem;//二级
    dispinfo.item.iSubItem = m_nSubItem;//一级
    dispinfo.item.pszText = m_bEscape ? NULL : LPTSTR ((LPCTSTR) Text);
    dispinfo.item.cchTextMax = Text.GetLength();
	
    GetParent()->GetParent()->SendMessage (WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM) &dispinfo);
}

void CListCellEdit::OnChange() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CEdit::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
/*	char str[200];
	this->GetWindowText(str,100);
	strcpy(str,"text");
	this->SetWindowText(str);
	this->SetSel(strlen(str),strlen(str),FALSE);
*/
}

LRESULT CListCellEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	//if(message == WM_PASTE)
	//{
	//	return TRUE;
	//}
	
	return CEdit::WindowProc(message, wParam, lParam);
}
