/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcWorker.cpp
 * @brief    Implementation of Class CcWorker
 */
#include "CcWorker.h"

CcWorker::CcWorker( void )
{
}

CcWorker::~CcWorker( void )
{
}

void CcWorker::enterState(eThreadState State) {
  if (State != CCTHREAD_STOPPED)
  { 
    CcThreadObject::enterState(State);
  }
  else delete this;
}
