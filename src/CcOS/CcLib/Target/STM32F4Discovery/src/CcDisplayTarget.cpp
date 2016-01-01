/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcDisplayTarget.cpp
 * @brief    Class CcDisplayTarget
 *  Hardware-Connections:
 *    LCD-D00  -> PD14 -> FSMC_A00
 *    LCD-D01  -> PD15 -> FSMC_A01
 *    LCD-D02  -> PD00 -> FSMC_A02
 *    LCD-D03  -> PD01 -> FSMC_A03
 *    LCD-D04  -> PE07 -> FSMC_A04
 *    LCD-D05  -> PE08 -> FSMC_A05
 *    LCD-D06  -> PE09 -> FSMC_A06
 *    LCD-D07  -> PE10 -> FSMC_A07
 *    LCD-D08  -> PE11 -> FSMC_A08
 *    LCD-D09  -> PE12 -> FSMC_A09
 *    LCD-D10  -> PE13 -> FSMC_A10
 *    LCD-D11  -> PE14 -> FSMC_A11
 *    LCD-D12  -> PE15 -> FSMC_A12
 *    LCD-D13  -> PD08 -> FSMC_A13
 *    LCD-D14  -> PD09 -> FSMC_A14
 *    LCD-D15  -> PD10 -> FSMC_A15
 *    LCD-CS   -> PD07 -> FSMC_NE1
 *    LCD-RnW  -> PD05 -> FSMC_NWE
 *    LCD-DnC  -> PD04 -> FSMC_NOE
 *    LCD-LED  -> PC14
 *    LCD-RES  -> PC15
 */

#include "CcDisplayTarget.h"
#include "CcKernel.h"

#define LCD_BASE   (0x60000000UL)          // LCD base address

#define LCD_REG16  (*((volatile unsigned short *) 0x60000000)) // RS = 0
#define LCD_DAT16  (*((volatile unsigned short *) 0x60100000)) // RS = 1

CcDisplayTarget::CcDisplayTarget() :
  CcDisplay(240, 320)
{
  setSizeX(240);
  setSizeY(320);
  init();
}

CcDisplayTarget::~CcDisplayTarget() {

}

void CcDisplayTarget::init( void ){
  initIO();
  initBus();
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET); //trigger reset-pin
  Kernel.delayMs(5);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET); //trigger reset-pin
  Kernel.delayMs(50); /* Delay 50 ms                        */

  uint16 driverCode = readReg(0x00);

  if( driverCode == 0x1289 || driverCode == 0x8989 ){
    //trace_printf("Found Display: SSD1289\n");
    writeCommandData(0x0000,0x0001);   /* Enable LCD Oscillator */
    writeCommandData(0x0003,0xA8A4);

    writeCommandData(0x000C,0x0000);
    writeCommandData(0x000D,0x080C);
    writeCommandData(0x000E,0x2B00);
    writeCommandData(0x001E,0x00B7);
    writeCommandData(0x0001,0x2B3F);   /* 320*240 0x2B3F */
    writeCommandData(0x0002,0x0600);
    writeCommandData(0x0010,0x0000);
    writeCommandData(0x0011,0x6070);
    writeCommandData(0x0005,0x0000);
    writeCommandData(0x0006,0x0000);
    writeCommandData(0x0016,0xEF1C);
    writeCommandData(0x0017,0x0003);
    writeCommandData(0x0007,0x0233);
    writeCommandData(0x000B,0x0000);
    writeCommandData(0x000F,0x0000);
    writeCommandData(0x0041,0x0000);
    writeCommandData(0x0042,0x0000);
    writeCommandData(0x0048,0x0000);
    writeCommandData(0x0049,0x013F);
    writeCommandData(0x004A,0x0000);
    writeCommandData(0x004B,0x0000);
    writeCommandData(0x0044,0xEF00);
    writeCommandData(0x0045,0x0000);
    writeCommandData(0x0046,0x013F);
    writeCommandData(0x0030,0x0707);
    writeCommandData(0x0031,0x0204);
    writeCommandData(0x0032,0x0204);
    writeCommandData(0x0033,0x0502);
    writeCommandData(0x0034,0x0507);
    writeCommandData(0x0035,0x0204);
    writeCommandData(0x0036,0x0204);
    writeCommandData(0x0037,0x0502);
    writeCommandData(0x003A,0x0302);
    writeCommandData(0x003B,0x0302);
    writeCommandData(0x0023,0x0000);
    writeCommandData(0x0024,0x0000);
    writeCommandData(0x0025,0x8000);
    writeCommandData(0x004e,0x0000);
    writeCommandData(0x004f,0x0000);
    writeCommand(0x0022);
    fillDisplay();
  }
  //else
    //trace_printf("No Display found\n");
}

void CcDisplayTarget::initIO( void ){
  GPIO_InitTypeDef GPIO_InitStruct;
  /* GPIO Ports Clock Enable */
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOC,&GPIO_InitStruct);
  setBackgroundLed(true);

  //Enable Port D
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  GPIO_InitStruct.Pin =
              GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_4  |
              GPIO_PIN_5  | GPIO_PIN_7  | GPIO_PIN_8  | GPIO_PIN_9  |
              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_14 |
              GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init(GPIOD,&GPIO_InitStruct);

  //Enable Port E
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
  GPIO_InitStruct.Pin = GPIO_PIN_3 |
              GPIO_PIN_7  | GPIO_PIN_8  | GPIO_PIN_9  |
              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 |
              GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FSMC;
  HAL_GPIO_Init( GPIOE, &GPIO_InitStruct);

}

void CcDisplayTarget::initBus( void ){
  FSMC_NORSRAM_TimingTypeDef Timing;
  static SRAM_HandleTypeDef hSram;
  __FSMC_CLK_ENABLE();

  /* SRAM4 memory initialization */
  hSram.Instance = FSMC_NORSRAM_DEVICE;
  hSram.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;

  hSram.Init.NSBank = FSMC_NORSRAM_BANK1;
  hSram.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hSram.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hSram.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hSram.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hSram.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hSram.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hSram.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hSram.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hSram.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hSram.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hSram.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hSram.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;

  Timing.AddressSetupTime = 7;
  Timing.AddressHoldTime = 1;
  Timing.DataSetupTime = 9;
  Timing.BusTurnAroundDuration = 0;
  Timing.CLKDivision = 4;
  Timing.DataLatency = 2;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;

  HAL_SRAM_Init(&hSram, &Timing, &Timing);
  //if(HAL_SRAM_Init(&hSram, &Timing, &Timing) != HAL_OK)
    //trace_printf("LCD HAL Init error\n");

}

void CcDisplayTarget::writeData(uint16 uiData){
  LCD_DAT16 = uiData;
}

void CcDisplayTarget::drawPixel(uchar R, uchar G, uchar B){
  uint16 uiData;
  uiData  = 0x001F & B>>3;
  uiData |= 0x07E0 & ( G << 2 );
  uiData |= 0xf800 & ( R << 9 );
  LCD_DAT16 = uiData;
}

uint16 CcDisplayTarget::readData( void ){
  return LCD_DAT16;
}

uint16 CcDisplayTarget::readReg(uint16 uiAddr){
  LCD_REG16 = uiAddr;
  return LCD_DAT16;
}

void CcDisplayTarget::writeCommand(uint16 uiCommand){
  LCD_REG16 = uiCommand;
}

void CcDisplayTarget::writeCommandData(uint16 uiCommand, uint16 uiData){
  writeCommand(uiCommand);
  writeData(uiData);
}

void CcDisplayTarget::setAddress(uint16 x, uint16 y, uint16 xSize, uint16 ySize){
  uint16 calcX, calcY;
  calcX = xSize + x;
  calcX--;
  calcY = ySize + y;
  calcY--;
  writeCommandData(0x0044, x | calcX << 8);  // horizontal start + end address
  writeCommandData(0x0045, y);      // vertical start-address
  writeCommandData(0x0046, calcY);     // vertical end-address
  writeCommandData(0x004E, x);
  writeCommandData(0x004F, y);
  writeCommand(0x0022);             // Apply cooridnates
}


void CcDisplayTarget::setBackgroundLed( bool bState )
{
  if(bState)
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET); //turn on Backlight
  else
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET); //turn on Backlight
}

void CcDisplayTarget::setCursor( uint16 x, uint16 y )
{
  writeCommandData(0x004E, x);
  writeCommandData(0x004F, y);
  writeCommand(0x0022);
}
