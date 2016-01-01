/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
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