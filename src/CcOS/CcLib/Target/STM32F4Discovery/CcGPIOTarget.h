/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcGPIOTarget.h
 * @brief    Class CcGPIO A-J
 */

#ifndef CCGPIOTARGET_H_
#define CCGPIOTARGET_H_

#include "CcBase.h"
#include "dev/CcGPIO.h"

class CcGPIOPortA : public CcGPIO {
public:
  CcGPIOPortA();
  virtual ~CcGPIOPortA();

  void init( void );


private:

};

#endif /* CCGPIOTARGET_H_ */
