/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcThread.cpp
 * @brief    Class CcThread
 */
#include "CcThread.h"
#include "CcKernel.h"

CcThread::CcThread()
{
  Kernel.createThread(this);
}

CcThread::~CcThread() {
  // TODO Auto-generated destructor stub
}
