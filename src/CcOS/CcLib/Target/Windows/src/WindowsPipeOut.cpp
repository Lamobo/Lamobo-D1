/**
 * @copyright  Andreas Dirmeier (C) 2015
 *
 * This file is part of CcOS.
 *
 * CcOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CcOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CcOS.  If not, see <http://www.gnu.org/licenses/>.
 **/
/**
 * @author     Andreas Dirmeier
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 * @file     WindowsPipeOut.cpp
 * @brief    Implementation of Class WindowsPipeOut
 */
#include "WindowsPipeOut.h"
#include "stdio.h"

WindowsPipeOut::WindowsPipeOut( CcIODevice* Out ):
  m_Handle(NULL),
  m_hRead(NULL),
  m_IODev(Out)
{
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;
  if (!CreatePipe(&m_hRead, &m_Handle, &saAttr, 0))
    printf(TEXT("StdoutRd CreatePipe"));

  // Ensure the read handle to the pipe for STDOUT is not inherited.

  if (!SetHandleInformation(m_hRead, HANDLE_FLAG_INHERIT, 0))
    printf(TEXT("Stdout SetHandleInformation"));
}

WindowsPipeOut::~WindowsPipeOut( void )
{
  enterState(CCTHREAD_STOPPING);
  CancelIo(m_hRead);
  CancelIo(m_Handle);
  if (m_Handle != 0)
    CloseHandle( m_Handle);
  if (m_hRead != 0)
    CloseHandle( m_hRead);
}

void WindowsPipeOut::run(void){
  char buf[1024];
  DWORD readSize;
  while (getThreadState() == CCTHREAD_RUNNING){
    if (ReadFile(m_hRead,    // open file handle
      buf,        // start of data to write
      1024,       // number of bytes to write
      &readSize,  // number of bytes that were written
      NULL))      // no overlapped structure
    {
      m_IODev->write(buf, readSize);
    }
    else{
      enterState(CCTHREAD_STOPPING);
    }
  }
  enterState(CCTHREAD_STOPPED);
}