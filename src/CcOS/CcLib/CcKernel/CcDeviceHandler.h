/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcDeviceHandler.h
 * @brief    Class CcDeviceHandler
 */
#ifndef CCDEVICEHANDLER_H_
#define CCDEVICEHANDLER_H_

#include "CcBase.h"

/**
 * @brief Handles all devices and Interfaces connected to Kernel
 */
class CcDeviceHandler {
public:
  /**
   * @brief Constructor
   */
  CcDeviceHandler(void);

  /**
   * @brief Destructor
   */
  virtual ~CcDeviceHandler(void);
};

#endif /* CCDEVICEHANDLER_H_ */
