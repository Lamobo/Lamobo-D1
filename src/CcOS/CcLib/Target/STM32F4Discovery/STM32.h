/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     STM32.h
 * @brief    Configurations for Target-Device
 **/
#ifndef STM32_H_
#define STM32_H_

// Includes for STM32CubeF4
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_fsmc.h"
#include "stm32f4xx_hal_gpio.h"

/**
 * @addgroup GPIO_Defs GPIO-Definitions
 * @{
 */
#define PORTA       0
#define PORTA_PINS  16
#define PORTB       1
#define PORTB_PINS  16
#define PORTC       2
#define PORTC_PINS  16
#define PORTD       3
#define PORTD_PINS  16
#define PORTE       4
#define PORTE_PINS  16

#define PORT_NUMBER 5
/**
 * @}
 */

#endif /* STM32_H_ */
