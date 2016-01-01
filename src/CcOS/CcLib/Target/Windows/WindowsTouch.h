/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     WindowsTouch.h
 * @brief    Class WindowsTouch
 **/
#ifndef WindowsTouch_H_
#define WindowsTouch_H_

#include "CcBase.h"
#include "dev/CcTouch.h"
class WindowsTouch : public CcTouch
{
public:
  WindowsTouch();
  virtual ~WindowsTouch();

  void init(void);
  void getTouchState(uint16 *x, uint16 *y);
  bool getPressState(void);
};

#endif /* WindowsTouch_H_ */
