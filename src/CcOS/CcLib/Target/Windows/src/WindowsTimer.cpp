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
 * @file     WindowsTimer.cpp
 * @brief    Class WindowsTimer
 **/
#include "WindowsTimer.h"


WindowsTimer::WindowsTimer() {
}

WindowsTimer::~WindowsTimer() {
  // nothing to do
}

void WindowsTimer::delayMs(uint32 uiDelay){

}

void WindowsTimer::delayS(uint32 uiDelay){

}

void WindowsTimer::tick( void ){

}

bool WindowsTimer::open(uint16 flags) {
  CC_UNUSED(flags);
  return false;
}

bool WindowsTimer::close(void){
  return true;
}

size_t WindowsTimer::read(char* buffer, size_t size){
  CC_UNUSED(buffer);
  CC_UNUSED(size);
  return 0;
}

size_t WindowsTimer::write(char* buffer, size_t size){
  CC_UNUSED(buffer);
  CC_UNUSED(size);
  return 0;
}