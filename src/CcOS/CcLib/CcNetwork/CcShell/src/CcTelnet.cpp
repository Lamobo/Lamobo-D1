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
