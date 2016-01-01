/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcGPIO.cpp
 * @brief    Class CcGPIO
 */

#include "CcGPIOTarget.h"
#include "STM32.h"

CcGPIOPortA::CcGPIOPortA() {
  m_PinList = new sPin[PORTA_PINS];
  for(uint8 i=0; i < PORTA_PINS; i++)
  {
    m_PinList[i].PinFunction          = PIN_TO_INPUT;
    m_PinList[i].PinFunctionAvailable = 0;
  }
  init();
}

CcGPIOPortA::~CcGPIOPortA() {
  // TODO Auto-generated destructor stub
}

void CcGPIOPortA::init( void ){
  /*GPIO_InitTypeDef m_GPIO_InitStruct;
  m_GPIO_InitStruct.Pin = GPIO_PIN_All;
  m_GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  m_GPIO_InitStruct.Pull = GPIO_NOPULL;
  m_GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init( GPIOA, &m_GPIO_InitStruct);*/
}
