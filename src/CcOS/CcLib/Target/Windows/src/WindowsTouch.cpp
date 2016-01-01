/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     WindowsTouch.cpp
 * @brief    Class WindowsTouch
 */
#include "WindowsTouch.h"
#include "CcKernel.h"

WindowsTouch::WindowsTouch() {

}

WindowsTouch::~WindowsTouch() {
}

void WindowsTouch::init(void){
}

void WindowsTouch::getTouchState(uint16 *x, uint16 *y){
  *x=getXAbsolute();
  *y=getYAbsolute();
}

bool WindowsTouch::getPressState(void){ 
  return true; 
}