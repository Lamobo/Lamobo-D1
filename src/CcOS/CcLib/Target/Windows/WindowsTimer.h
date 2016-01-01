/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     WindowsTimer.h
 * @brief    Class WindowsTimer
 **/
#ifndef WindowsTimer_H_
#define WindowsTimer_H_

#include "CcBase.h"
#include "dev/CcTimer.h"

class WindowsTimer : public CcTimer {
public: //methods
  WindowsTimer();
  virtual ~WindowsTimer();


  static void delayMs(uint32 uiDelay);
  void delayS(uint32 uiDelay);

  static void tick(void);
  bool open(uint16 flags);
  bool close(void);
  size_t read(char* buffer, size_t size);
  size_t write(char* buffer, size_t size);

private: //methods
  static uint32 getCounterState(void);

private: //member
  static uint32 s_CountDown;
};

#endif /* WindowsTimer_H_ */
