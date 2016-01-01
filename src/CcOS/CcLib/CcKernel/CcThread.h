/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcThread.h
 * @brief    Class CcThread
 */
#ifndef CCTHREAD_H_
#define CCTHREAD_H_

#include "CcBase.h"
#include "CcObject.h"
#include "CcThreadObject.h"
/**
 * @brief Default Class to create a Application
 */
class CcThread : public CcThreadObject {
public:
  CcThread();
  virtual ~CcThread();

  /**
   * @brief Virtual function for Running-Code
   *        Must be implemented by target application.
   */
  virtual void run   ( void ) = 0;
};

#endif /* CCTHREAD_H_ */
