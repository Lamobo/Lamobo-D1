/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcTouchTarget.cpp
 * @brief    Class CcTouchTarget
 *    Hardware-Connections:
 *      DIN ->  PA07  -> SPI1_MOSI
 *      DOUT->  PA06  -> SPI1_MISO
 *      CLK ->  PA05  -> SPI1_CLK
 *      CS  ->  PA04  -> SPI1_NSS (Set not in SPI)
 *      PEN ->  PA03  -> Ext. Interrupt 03
 */
#include "CcTouchTarget.h"
#include "CcKernel.h"

CcTouchTarget *g_touchHandle = 0;

extern "C" void EXTI3_IRQHandler( void ){
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET)
  {
    g_touchHandle->onInterrupt();
  }
  HAL_NVIC_GetPendingIRQ(EXTI3_IRQn);
  HAL_NVIC_ClearPendingIRQ(EXTI3_IRQn);
  __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
}

CcTouchTarget::CcTouchTarget() {
  g_touchHandle = this;
  m_CalibMatrix.A   = 31800;
  m_CalibMatrix.B   = -68730;
  m_CalibMatrix.C   = -2749000;
  m_CalibMatrix.D   = -72760;
  m_CalibMatrix.E   = 71510;
  m_CalibMatrix.F   = 7727640;
  m_CalibMatrix.Div = -137716;
}

CcTouchTarget::~CcTouchTarget() {
  // TODO Auto-generated destructor stub
}

void CcTouchTarget::init(void){
  initIO();
}

void CcTouchTarget::initIO(void){
  GPIO_InitTypeDef GPIO_InitStruct;
  /* GPIO Ports Clock Enable */
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin =  GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin =  GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init( GPIOA, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 3);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  __SPI1_CLK_ENABLE();

  m_SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  m_SpiHandle.Init.Mode        = SPI_MODE_MASTER;
  m_SpiHandle.Init.Direction   = SPI_DIRECTION_2LINES;
  m_SpiHandle.Init.CLKPhase    = SPI_PHASE_1EDGE;
  m_SpiHandle.Init.CLKPolarity = SPI_POLARITY_LOW;
  m_SpiHandle.Init.DataSize    = SPI_DATASIZE_8BIT;
  m_SpiHandle.Init.FirstBit    = SPI_FIRSTBIT_MSB;
  m_SpiHandle.Init.NSS         = SPI_NSS_SOFT;
  m_SpiHandle.Init.TIMode      = SPI_TIMODE_DISABLE;
  m_SpiHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  m_SpiHandle.Init.CRCPolynomial  = 7;
  m_SpiHandle.Instance = SPI1;

  HAL_SPI_Init(&m_SpiHandle);

}

void CcTouchTarget::getTouchState(uint16 *x, uint16 *y){
  union toInt{
    uint16 value;
    struct{
      uint8 lower;
      uint8 upper;
    };
  };
  toInt readValue;
  HAL_StatusTypeDef bState;
  uchar readBuf[3]  = {0,0,0};
  uchar writeBuf[3] = {0,0,0};
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  writeBuf[0] = 0x93;
  bState = HAL_SPI_TransmitReceive(&m_SpiHandle,writeBuf, readBuf, sizeof(readBuf), 100);
  switch(bState)
  {
    case HAL_ERROR:
    case HAL_TIMEOUT:
      break;
    default:
      readValue.upper = readBuf[1];
      readValue.lower = readBuf[2];
      *y = 2000-(readValue.value>>4);
      break;
  }

  writeBuf[0] = 0xd0;
  bState = HAL_SPI_TransmitReceive(&m_SpiHandle,writeBuf, readBuf, sizeof(readBuf), 100);
  switch(bState)
  {
    case HAL_ERROR:
    case HAL_TIMEOUT:
      break;
    default:
      readValue.upper = readBuf[1];
      readValue.lower = readBuf[2];
      *x = (readValue.value >> 4);
      break;
  }
}

bool CcTouchTarget::getPressState( void ){
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET)
    return false;
  else
    return true;
}
