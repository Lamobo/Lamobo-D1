/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcThreadObject.cpp
 * @brief    Class CcThreadObject
 */
#include "CcThreadObject.h"
#include "CcKernel.h"

CcThreadObject::CcThreadObject() :
  m_State(CCTHREAD_STOPPED)
{
  // TODO Auto-generated constructor stub

}

CcThreadObject::~CcThreadObject() {
}

void CcThreadObject::start( void ) {
  Kernel.createThread(this);
}

void CcThreadObject::stop(void) {
  enterState(CCTHREAD_STOPPING);
}

eThreadState CcThreadObject::getThreadState(void){
  return m_State;
}

void CcThreadObject::enterState(eThreadState State){
  m_State = State;
}
