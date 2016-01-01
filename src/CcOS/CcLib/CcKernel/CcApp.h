/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcApp.h
 * @brief    Class CcApp
 */
#ifndef CCAPPLICATION_H_
#define CCAPPLICATION_H_

#include "CcBase.h"
#include "CcThreadObject.h"
#include "CcStringList.h"

class CcApp;

typedef CcApp*(*CcAppCreateFunction)(CcStringList *Arg);

/**
 * @brief Default Class to create a Application
 */
class CcApp : public CcThreadObject {
public:
  CcApp();
  virtual ~CcApp();

  /**
   * @brief Virtual function for Running-Code
   *        Must be implemented by target application.
   */
  virtual void run(void) = 0;
};

#endif /* CCAPPLICATION_H_ */
