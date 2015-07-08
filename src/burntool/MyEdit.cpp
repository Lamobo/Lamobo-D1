// MyEdit.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "MyEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyEdit

CMyEdit::CMyEdit()
{
}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	//{{AFX_MSG_MAP(CMyEdit)
//	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyEdit message handlers

/*HBRUSH CMyEdit::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	// TODO: Change any attributes of the DC here
//	CFont font;
//	font.CreatePointFont(90, _T("Arial"));
//	pDC->SelectObject(&font);

//	pDC->SetTextColor(RGB(0,0,0));

//	HBRUSH newHbr = CreateSolidBrush(RGB(255,255,255));

	// TODO: Return a non-NULL brush if the parent's handler should not be called
//	return newHbr;
}*/
