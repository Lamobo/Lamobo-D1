/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcTimer.h
 * @brief    Class CcTimer
 */

#ifndef CCTIMER_H_
#define CCTIMER_H_

#include "CcBase.h"
#include "CcIODevice.h"

class CcTimer : public CcIODevice{
public: //methods
  CcTimer();
  virtual ~CcTimer();

  static void delayMs(uint32 uiDelay);
  void delayS(uint32 uiDelay);

  static void tick( void );

  virtual bool open(uint16 flags);
  virtual bool close(void);
  virtual size_t read(char* buffer, size_t size);
  virtual size_t write(char* buffer, size_t size);

private: //methods
  static uint32 getCounterState(void);

private: //member
  static volatile uint32 s_CountDown;
};

#endif /* CCTIMER_H_ */
