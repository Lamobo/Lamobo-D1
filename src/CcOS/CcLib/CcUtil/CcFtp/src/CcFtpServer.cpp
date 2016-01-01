/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
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
