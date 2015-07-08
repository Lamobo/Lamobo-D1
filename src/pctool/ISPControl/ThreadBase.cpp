#include "StdAfx.h"
#include "ThreadBase.h"

unsigned int CThreadBase::thread_begin( void * pParam )
{
	CThreadBase * pThread = static_cast<CThreadBase *>(pParam);
	pThread->Run1();
	return 0;
}

CThreadBase::CThreadBase(void)
: m_handle(NULL)
, m_ThreadName( "" )
, m_bRun( false )
{	
}

CThreadBase::CThreadBase( const char * ThreadName )
: m_handle(NULL)
, m_ThreadName( ThreadName )
, m_bRun( false )
{
}

CThreadBase::CThreadBase( string ThreadName )
: m_handle(NULL)
, m_ThreadName( ThreadName )
, m_bRun( false )
{
}

CThreadBase::~CThreadBase(void)
{
	if (m_handle)
		::CloseHandle( m_handle );
}

bool CThreadBase::Start( bool bSuspend )
{
	if ( m_bRun )
		return true;
	
	if ( bSuspend )
		m_handle = (HANDLE)_beginthreadex( NULL, THREAD_STACK_SIZE, thread_begin, this, CREATE_SUSPENDED, &m_ThreadID );
	else
		m_handle = (HANDLE)_beginthreadex( NULL, THREAD_STACK_SIZE, thread_begin, this, 0, &m_ThreadID );

	m_bRun = (NULL != m_handle);

	return m_bRun;
}

void CThreadBase::Run1()
{
	Run();
	m_bRun = false;
}

void CThreadBase::Join( int itimeout )
{
	if ( (NULL == m_handle) || !m_bRun )
		return;

	if ( itimeout < 0 )
		itimeout = INFINITE;

	::WaitForSingleObject( m_handle, itimeout );

	//DWORD result;
	//MSG msg;

	//while(TRUE) {
		//result = MsgWaitForMultipleObjects(1, &m_handle, FALSE, INFINITE, QS_ALLINPUT);
		//if (result == WAIT_OBJECT_0)
			//break;
		//else {
			//PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			//DispatchMessage(&msg);
		//}
	//}
}

void CThreadBase::Resume()
{
	if ( (NULL == m_handle) || !m_bRun )
		return;

	::ResumeThread( m_handle );
}

void CThreadBase::Suspend()
{
	if ( (NULL == m_handle) || !m_bRun )
		return;

	::SuspendThread( m_handle );
}

bool CThreadBase::Terminate( unsigned long ExitCode )
{
	if ( (NULL == m_handle) || !m_bRun )
		return true;

	if ( ::TerminateThread( m_handle, ExitCode ) ) {
		::CloseHandle( m_handle );
		m_handle = NULL;
		return true;
	}

	return false;
}

unsigned int CThreadBase::GetThreadID()
{
	return m_ThreadID;
}

void CThreadBase::SetThreadName( const char *ThreadName )
{
	if ( NULL == ThreadName )
		m_ThreadName = "";
	else
		m_ThreadName = ThreadName;
}

void CThreadBase::SetThreadName( string ThreadName )
{
	m_ThreadName = ThreadName;
}
