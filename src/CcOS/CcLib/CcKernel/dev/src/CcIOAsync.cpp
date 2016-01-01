/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcIOAsync.cpp
 * @brief    Class CcIOAsync
 */

#include "CcIODevice.h"
#include "dev/CcIOAsync.h"


CcIOAsync::CcIOAsync(CcIODevice *device) :
  m_Device(device)
{
}

CcIOAsync::~CcIOAsync() {
}

bool CcIOAsync::read(char* buffer, size_t size){
  size_t szRet = read(buffer, size);
  onReadDone(szRet);
  return true;
}

bool CcIOAsync::write(char* buffer, size_t size){
  size_t szRet = write(buffer, size);
  onWriteDone(szRet);
  return true;
}

bool CcIOAsync::onReadDone(size_t size){
  if (size > 0)
    return true;
  else
    return false;
}

bool CcIOAsync::onWriteDone(size_t size){
  if (size > 0)
    return true;
  else 
    return false;
}

void CcIOAsync::callback(uint8 nr, void *Param){
  CC_UNUSED(Param);
  switch (nr)
  { 
    case CCIOASYNC_CB_READDONE:
      onReadDone(0);
      break;
    case CCIOASYNC_CB_WRITEDONE:
      onWriteDone(0);
      break;
  }
}
