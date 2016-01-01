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