/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcIODevice.cpp
 * @brief    Class CcIODevice
 */

#include "CcIODevice.h"

CcIODevice::CcIODevice() :
  m_IOSettings(0),
  m_IOSettingsSize(0)
{
}

CcIODevice::~CcIODevice() {
}

bool CcIODevice::cancel(){
  return true;
}

size_t CcIODevice::writeSettings(char* buffer, size_t size){
  CC_UNUSED(buffer);
  return size;
}

size_t CcIODevice::readSettings(char* buffer, size_t size){
  for (size_t i = 0; i < size; i++)
    buffer[i] = 0;
  return size;
}