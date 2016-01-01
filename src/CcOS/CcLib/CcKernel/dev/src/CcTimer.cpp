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
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 * @file     CcTimer.cpp
 * @brief    Class CcTimer
 */

#include "dev/CcTimer.h"

volatile uint32 CcTimer::s_CountDown = 0;

uint32 CcTimer::getCounterState( void ){
  return s_CountDown;
}

CcTimer::CcTimer() {
}

CcTimer::~CcTimer() {
  // nothing to do
}

void CcTimer::tick(){
  if(s_CountDown != 0)
    s_CountDown--;
}

bool CcTimer::open(uint16 flags) {
  CC_UNUSED(flags);
  return false;
}

bool CcTimer::close(void){
  return true;
}

size_t CcTimer::read(char* buffer, size_t size){
  CC_UNUSED(buffer);
  CC_UNUSED(size);
  return 0;
}

size_t CcTimer::write(char* buffer, size_t size){
  CC_UNUSED(buffer);
  CC_UNUSED(size);
  return 0;
}