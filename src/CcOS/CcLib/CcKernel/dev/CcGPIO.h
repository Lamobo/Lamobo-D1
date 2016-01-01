/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcGPIO.h
 * @brief    Class CcGPIO
 */

#ifndef CCGPIO_H_
#define CCGPIO_H_

#include "CcBase.h"

#define PIN_TO_SPI        0x0001
#define PIN_TO_UART       0x0002
#define PIN_TO_I2C        0x0004
#define PIN_TO_FSMC       0x0008
#define PIN_TO_INPUT      0x0010
#define PIN_TO_OUTPUT     0x0011
#define PIN_TO_INTERRUPT  0x0012

/**
 * @brief Control the Input and Outputports on device
 */
class CcGPIO {
public:
  /**
   * @brief Structure holding Pin Settings
   */
  typedef struct{
    uint16 PinFunction;
    uint16 PinFunctionAvailable;
  } sPin;

  /**
   * @brief Constructor
   */
  CcGPIO();

  /**
   * @brief Destructor
   */
  virtual ~CcGPIO();

  /**
   * @brief Initialize basic settings for General Purpose Input Output
   */
  virtual void init( void ) = 0;

  sPin *m_PinList; ///< List of all existing Pins available in this Class.
};

#endif /* CCGPIO_H_ */
