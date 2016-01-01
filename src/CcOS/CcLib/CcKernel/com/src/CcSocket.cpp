/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     CcSocket.cpp
* @brief    Class CcSocket
*/
#include "com/CcSocket.h"

CcSocket::CcSocket(eSocketType type):
  m_SockType(type)
{
}

CcSocket::~CcSocket( void )
{
}

bool CcSocket::connect(CcString &hostName, CcString &hostPort)
{
  bool bRet;
  ipv4_t ip;
  bRet = getHostByName(hostName, &ip);
  if (bRet != false)
    bRet = connect(ip, hostPort.toUint16());
  return bRet;
}