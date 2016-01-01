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