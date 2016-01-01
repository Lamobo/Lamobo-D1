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
  while (m_State != CCTHREAD_STOPPED) Kernel.delayMs(10);
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
