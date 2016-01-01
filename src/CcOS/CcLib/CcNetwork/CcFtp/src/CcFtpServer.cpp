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
 * @file     CcFtpServer.cpp
 * @brief    Implementation of Class CcFtpServer
 *           Protocol: http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
 */
#include "CcKernel.h"
#include "CcThread.h"
#include "CcFtpServer.h"
#include "CcFtpTypes.h"

CcApp* CcFtpServer::main(CcStringList *Arg)
{
  CcApp* ret = new CcFtpServer(Arg);
  return ret;
}

CcFtpServer::CcFtpServer( uint16 Port ) :
  m_Port(Port)
{
}

CcFtpServer::CcFtpServer(CcStringList *Arg) :
  m_Port(27521)
{
  CC_UNUSED(Arg);
}

CcFtpServer::~CcFtpServer( void )
{
}

void CcFtpServer::run(void){
  m_Socket = Kernel.getSocket(eTCP);
  ipv4_t localhost = { 127, 0, 0, 1 };
  m_Socket->bind(localhost, m_Port);
  m_Socket->listen();
  CcSocket *temp;
  while (getThreadState() == CCTHREAD_RUNNING){
    temp = m_Socket->accept();
    CcFtpServerWorker *worker = new CcFtpServerWorker(temp);
    Kernel.createThread(worker);
  }
}
