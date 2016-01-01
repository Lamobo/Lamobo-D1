/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     TargetConfig.h
 * @brief    Configurations for Target-Device
 **/
#ifndef STM32F4DISCOVERY_TARGETCONFIG_H_
#define STM32F4DISCOVERY_TARGETCONFIG_H_

#define CC_TARGET_SYSTEM     CcSystemTarget
#define CC_TARGET_SYSTEM_INC "CcSystemTarget.h"
#define CC_TARGET_TIMER      CcTimerTarget
#define CC_TARGET_TIMER_INC  "CcTimerTarget.h"
#define CC_TARGET_DISPLAY
#define CC_TARGET_TOUCH

#endif /* STM32F4DISCOVERY_TARGETCONFIG_H_ */
