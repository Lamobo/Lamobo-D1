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
 * @date       2015-09
 * @par        Language   C++ ANSI V3
 * @file     CcUSBHid.cpp
 * @brief    Create Access to an USB-Hid-Device
 */

#include "com/CcUSBHid.h"

CcUSBHid::CcUSBHid( void )
{
}

CcUSBHid::~CcUSBHid( void )
{
}

bool CcUSBHid::setDevice( uint32 vid, uint32 pid, uint32 usage )
{
  bool bRet(false);
  m_Info.vid = vid;
  m_Info.pid = pid;
  m_Info.usage = usage;
  bRet = connect();
  return bRet;
}

uint32 CcUSBHid::getReportInputSize(void){
  return m_Info.m_uiReportInputSize;
}

uint32 CcUSBHid::getReportOutputSize(void){
  return m_Info.m_uiReportOutputSize;
}
