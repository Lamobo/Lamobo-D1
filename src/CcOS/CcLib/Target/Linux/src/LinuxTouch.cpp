/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxTouch.cpp
 * @brief    Class LinuxTouch
 */
#include "LinuxTouch.h"
#include "CcKernel.h"

LinuxTouch::LinuxTouch() {

}

LinuxTouch::~LinuxTouch() {
}

bool LinuxTouch::open(uint16 flags){
  CC_UNUSED(flags);
  return true;
}

void LinuxTouch::getTouchState(uint16 *x, uint16 *y){
  *x=getXAbsolute();
  *y=getYAbsolute();
}

bool LinuxTouch::getPressState(void){
  return true; 
}
