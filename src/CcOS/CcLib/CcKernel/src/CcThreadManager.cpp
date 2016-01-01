/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcThreadManager.cpp
 * @brief    Class CcThreadManager
 */
#include "CcThreadManager.h"
#include "CcKernel.h"

CcThreadManager::CcThreadManager( void )
{

}

CcThreadManager::~CcThreadManager() {

}

void CcThreadManager::addThread(CcThreadObject *thread, CcString *Name){
  sThreadState newItem;
  newItem.thread = thread;
  newItem.sNname = Name;
  m_ThreadList.append(newItem);
}

void CcThreadManager::closeAll(void){
  for (size_t i = 0; i < m_ThreadList.size(); i++)
  {
    CcThreadObject *thread = m_ThreadList.at(i).thread;
    if (thread->getThreadState() != CCTHREAD_STOPPED)
    thread->enterState(CCTHREAD_STOPPING);
    while (thread->getThreadState() != CCTHREAD_STOPPED);
  }
}