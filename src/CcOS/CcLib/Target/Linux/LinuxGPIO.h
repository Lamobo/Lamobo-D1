/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxGPIO.h
 * @brief    Class LinuxGPIO
 */

#ifndef LinuxGPIO_H_
#define LinuxGPIO_H_

#include "dev/CcGPIO.h"

class LinuxGPIO : public CcGPIO {
public:
	LinuxGPIO();
  virtual ~LinuxGPIO();

  void init( void );
};

#endif /* LinuxGPIO_H_ */
