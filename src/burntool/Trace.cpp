// Trace.cpp: implementation of the CTrace class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "burntool.h"
#include "Trace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define UNICODE_TXT_HEAD 0xFEFF
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern CBurnToolApp theApp;

CTrace::CTrace()
{
	m_hFile = INVALID_HANDLE_VALUE;
	m_mutex_trace = NULL;//m_mutex_trace
}

CTrace::~CTrace()
{
	if(m_hFile)
	{
		CloseHandle(m_hFile);//CloseHandle
		m_hFile = NULL;
	}

	if(m_mutex_trace)
	{
		CloseHandle(m_mutex_trace);//CloseHandle
		m_mutex_trace = NULL;
	}
}


BOOL CTrace::Init(int id)
{
	SYSTEMTIME st;
	CString str;
	
	if(m_hFile)
	{
		CloseHandle(m_hFile);//CloseHandle
		m_hFile = INVALID_HANDLE_VALUE;//INVALID_HANDLE_VALUE
	}

	GetSystemTime(&st);
	str.Format(_T("Log\\Trace%4d%2d%2d%2d%2d%2d%3d_%2d.txt"), st.wYear, st.wMonth, st.wDay, 
		(st.wHour+8)%24, st.wMinute, st.wSecond, st.wMilliseconds, id);

	str.Replace(' ', '0');

	m_hFile = CreateFile(theApp.ConvertAbsolutePath(str) , GENERIC_WRITE | GENERIC_READ , 
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL , 
		OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);//CreateFile

	if (m_hFile == INVALID_HANDLE_VALUE) //INVALID_HANDLE_VALUE
	{ 
		return FALSE;
	}

	if(!m_mutex_trace)
	{
		m_mutex_trace = CreateMutex(NULL, FALSE, NULL);//CreateMutex
	}

	return TRUE;
}

BOOL CTrace::TraceInfo(LPCSTR strInfo)
{
	SYSTEMTIME st;
	char buffer[1024];
	DWORD write_len;
	
	if(INVALID_HANDLE_VALUE == m_hFile)
	{
		return FALSE;
	}

	GetSystemTime(&st);//GetSystemTime
	sprintf(buffer, "%2d:%2d:%2d:%3d:	%s\r\n", (st.wHour+8)%24, st.wMinute, 
		st.wSecond, st.wMilliseconds, strInfo);

	if(WaitForSingleObject(m_mutex_trace, 5000) != WAIT_OBJECT_0)//WaitForSingleObject
	{
		ReleaseMutex(m_mutex_trace);
		return FALSE;
	}

	WriteFile(m_hFile, buffer, strlen(buffer), &write_len, NULL);//WriteFile

	//release mutex
	ReleaseMutex(m_mutex_trace);//ReleaseMutex
	return TRUE;
}

void CTrace::Close()
{
	if(m_hFile)
	{
		CloseHandle(m_hFile);//CloseHandle
		m_hFile = INVALID_HANDLE_VALUE;//INVALID_HANDLE_VALUE
	}

	if(m_mutex_trace)
	{
		CloseHandle(m_mutex_trace);//CloseHandle
		m_mutex_trace = NULL;//m_mutex_trace
	}
}
