/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     WindowsPipeIn.cpp
* @brief    Implementation of Class WindowsPipeIn
*/
#include "WindowsPipeIn.h"
#include "WindowsSocket.h"

WindowsPipeIn::WindowsPipeIn(CcIODevice* Out) :
m_Handle(NULL),
m_hWrite(NULL),
m_IODev(Out)
{
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;
  if (!CreatePipe(&m_Handle, &m_hWrite, &saAttr, 0))
    printf(TEXT("StdoutRd CreatePipe"));

  // Ensure the read handle to the pipe for STDOUT is not inherited.

  if (!SetHandleInformation(m_hWrite, HANDLE_FLAG_INHERIT, 0))
    printf(TEXT("Stdout SetHandleInformation"));
}

WindowsPipeIn::~WindowsPipeIn(void)
{
  enterState(CCTHREAD_STOPPING);
  CancelIo(m_hWrite);
  CancelIo(m_Handle);
  if (m_Handle != 0)
    CloseHandle( m_Handle);
  if (m_hWrite != 0)
    CloseHandle( m_hWrite);
}

void WindowsPipeIn::run(void){
  char buf[1024];
  DWORD readSize;
  while (getThreadState() == CCTHREAD_RUNNING){
    readSize = ((WindowsSocket*)m_IODev)->readTimeout(buf, 1024);
    WriteFile(m_hWrite,    // open file handle
      buf,        // start of data to write
      1024,       // number of bytes to write
      &readSize,  // number of bytes that were written
      NULL);      // no overlapped structure
  }
  enterState(CCTHREAD_STOPPED);
}