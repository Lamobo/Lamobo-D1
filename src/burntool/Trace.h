// Trace.h: interface for the CTrace class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TRACE_H__AFD24C7D_31FD_4E07_BF57_39B6BC1A3D63__INCLUDED_)
#define AFX_TRACE_H__AFD24C7D_31FD_4E07_BF57_39B6BC1A3D63__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTrace  
{
public:
	CTrace();
	virtual ~CTrace();

protected:
	HANDLE m_hFile;

	HANDLE m_mutex_trace;

public:
	BOOL Init(int id);
	void Close();

	BOOL TraceInfo(LPCSTR strInfo);

};

#endif // !defined(AFX_TRACE_H__AFD24C7D_31FD_4E07_BF57_39B6BC1A3D63__INCLUDED_)
