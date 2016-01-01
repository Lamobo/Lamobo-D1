/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcHttpServer.cpp
 * @brief    Implementation of Class CcHttpServer
 *           Protocol: http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
 */
#include "CcKernel.h"
#include "CcThread.h"
#include "CcHttpServer.h"

CcApp* CcHttpServer::main(CcStringList *Arg)
{
  CcApp* ret = new CcHttpServer(Arg);
  return ret;
}

CcHttpServer::CcHttpServer( uint16 Port ) :
  m_Port(Port)
{
}

CcHttpServer::CcHttpServer(CcStringList *Arg) :
  m_Port(80)
{
  CC_UNUSED(Arg);
}

CcHttpServer::~CcHttpServer( void )
{
}

void CcHttpServer::run(void){
  m_Socket = Kernel.getSocket(eTCP);
  ipv4_t localhost = { 127, 0, 0, 1 };
  m_Socket->bind(localhost, m_Port);
  m_Socket->listen();
  CcSocket *temp;
  while (getThreadState() == CCTHREAD_RUNNING){
    temp = m_Socket->accept();
    CcHttpServerWorker *worker = new CcHttpServerWorker(temp);
    worker->start();
  }
}
