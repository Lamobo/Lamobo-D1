/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcTimerTarget.cpp
 * @brief    Class CcTimerTarget
 **/
#include <CcTimerTarget.h>
#include "STM32.h"
#include "CcKernel.h"

CcTimerTarget *g_SystemTimer;
extern "C" void SysTick_Handler(void) {
  g_SystemTimer->tick();
  //Trigger Hardware for timeouts
  HAL_IncTick();
}

CcTimerTarget::CcTimerTarget(void):
  m_SystemTime(0)
{
  g_SystemTimer = this;
  SysTick_Config(SystemCoreClock / 1000); //Tick every 1ms
  m_CountDown = 0;
}

CcTimerTarget::~CcTimerTarget(void ){

}

time_t CcTimerTarget::getTime(void) {
  return m_SystemTime;
}

void CcTimerTarget::tick(void){
  m_SystemTime++;
}

