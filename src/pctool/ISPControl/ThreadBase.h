#pragma once

#include <string>
#include <windows.h>
#include <process.h>

using namespace std;

class CThreadBase
{
protected:
	CThreadBase(void);
	CThreadBase( const char * ThreadName );
	CThreadBase( string ThreadName );
	virtual ~CThreadBase(void);
	
	/*
	thread run process
	*/
	virtual void Run() = 0;
	
	virtual void Run1();

	/*
	start the thread
	@arg bSuspend: suspend the thread when it start.
	*/
	bool Start( bool bSuspend = false );
	
	/*
	wait for thread stop.
	*/
	void Join( int itimeout = -1 );

	/*
	resume the thread from suspend
	*/
	void Resume();

	/*
	suspend the thread
	*/
	void Suspend();
	
	/*
	terminate the thread
	*/
	bool Terminate( unsigned long ExitCode );


	unsigned int GetThreadID();
	std::string GetThreadName();
	void SetThreadName( string ThreadName );
	void SetThreadName( const char * ThreadName );

private:
	static unsigned int WINAPI thread_begin( void * pParam );

protected:
	HANDLE m_handle;
	unsigned int m_ThreadID;
	string m_ThreadName;
	volatile bool m_bRun;
};
