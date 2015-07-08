// BurnToolDoc.h : interface of the CBurnToolDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BURNTOOLDOC_H__0938860F_24B6_4B34_9D48_F2D88D122352__INCLUDED_)
#define AFX_BURNTOOLDOC_H__0938860F_24B6_4B34_9D48_F2D88D122352__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CBurnToolDoc : public CDocument
{
protected: // create from serialization only
	CBurnToolDoc();
	DECLARE_DYNCREATE(CBurnToolDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBurnToolDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBurnToolDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CBurnToolDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BURNTOOLDOC_H__0938860F_24B6_4B34_9D48_F2D88D122352__INCLUDED_)
