/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxTimer.h
 * @brief    Class LinuxTimer
 **/
#ifndef LinuxTimer_H_
#define LinuxTimer_H_

#include "CcBase.h"
#include "dev/CcTimer.h"

class LinuxTimer : public CcTimer {
public: //methods
  LinuxTimer();
  virtual ~LinuxTimer();

  static void delayMs(uint32 uiDelay);
  void delayS(uint32 uiDelay);

  static void tick( void );

private: //methods
  static uint32 getCounterState(void);

private: //member
  static uint32 s_CountDown;
};

#endif /* LinuxTimer_H_ */
