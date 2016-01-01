/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcTelnet.cpp
 * @brief    Implementation of Class CcTelnet
 */
#include "CcTelnet.h"
#include "CcKernel.h"

CcTelnet::CcTelnet(uint16 Port) :
m_Port(Port)
{
}

CcTelnet::~CcTelnet( void )
{
}

void CcTelnet::run(void){
  CcSocket *temp;
  bool bSuccess = false;
  m_Socket = Kernel.getSocket(eTCP);
  bSuccess = m_Socket->bind({ 127, 0, 0, 1 }, m_Port);
  bSuccess = m_Socket->listen();
  temp = m_Socket->accept();
  m_Socket->close();
  m_Shell = new CcShell(temp, temp);
  m_Socket = temp;
  Kernel.createProcess(m_Shell);
}
