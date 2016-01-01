/**
* @author     Andreas Dirmeier
*
* @copyright  Technische Software Entwicklung Plazotta (c) 2015
*
* @version    1.00
*
* @par        Language   C++ ANSI V3
*
* @par        History
* @n    20.08.2015 / AD
* @n      Module created
*/
/**
*   @file     HidDevice.cpp
*   @brief    Access-class to a HID Device
**************************************************************************/

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
