/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
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