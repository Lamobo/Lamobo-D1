/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxTouch.h
 * @brief    Class LinuxTouch
 **/
#ifndef LinuxTouch_H_
#define LinuxTouch_H_

#include "CcBase.h"
#include "dev/CcTouch.h"
class LinuxTouch : public CcTouch
{
public:
  LinuxTouch();
  virtual ~LinuxTouch();

  bool open(uint16 flags=0);
  bool close(void){return true;}
  void getTouchState(uint16 *x, uint16 *y);
  bool getPressState(void);
};

#endif /* LinuxTouch_H_ */
