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
 **/
#ifndef CCDISPLAYTARGET_H_
#define CCDISPLAYTARGET_H_

#include "CcBase.h"
#include "STM32.h"
#include "dev/CcDisplay.h"
#include "dev/CcTimer.h"

class CcDisplayTarget : public CcDisplay
{
public:
  CcDisplayTarget();
  virtual ~CcDisplayTarget();

  void init( void );
  void setCursor( uint16 x, uint16 y );
  void setAddress( uint16 x, uint16 y, uint16 xSize, uint16 ySize);
  void setBackgroundLed( bool bState );
  void initIO( void );
  void initBus( void );
  void drawPixel(uchar R, uchar G, uchar B);
  /**
   * @brief Write data to LCD
   * @param uiData: data to write
   */
  void writeData(uint16 uiData);
  /**
   * @brief Write command to LCD
   * @param uiData: command to write
   */
  void writeCommand(uint16 uiCmd);

  /**
   * @brief Write a command with one data at once;
   * @param uiCommand: command to write
   * @param uiData: data to write
   */
  void writeCommandData(uint16 uiCommand, uint16 Data);
  uint16 readReg( uint16 uiAddr );
  uint16 readData( void );

private: //member
  GPIO_InitTypeDef m_typeDefD, m_typeDefC, m_typeDefE;

  FSMC_NORSRAM_InitTypeDef m_sramInitStructure;
  FSMC_NORSRAM_TimingTypeDef m_sramTimeStruct;
  SRAM_HandleTypeDef m_hSram;
};

#endif /* CCDISPLAYTARGET_H_ */
