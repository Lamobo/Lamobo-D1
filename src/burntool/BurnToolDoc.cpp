// BurnToolDoc.cpp : implementation of the CBurnToolDoc class
//

#include "stdafx.h"
#include "BurnTool.h"

#include "BurnToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBurnToolDoc

IMPLEMENT_DYNCREATE(CBurnToolDoc, CDocument)

BEGIN_MESSAGE_MAP(CBurnToolDoc, CDocument)
	//{{AFX_MSG_MAP(CBurnToolDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBurnToolDoc construction/destruction

CBurnToolDoc::CBurnToolDoc()
{
	// TODO: add one-time construction code here

}

CBurnToolDoc::~CBurnToolDoc()
{
}

BOOL CBurnToolDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CBurnToolDoc serialization

void CBurnToolDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CBurnToolDoc diagnostics

#ifdef _DEBUG
void CBurnToolDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CBurnToolDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBurnToolDoc commands
