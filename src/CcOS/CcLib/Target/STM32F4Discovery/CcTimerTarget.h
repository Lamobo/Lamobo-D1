/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcTimerTarget.h
 * @brief    Class CcTimerTarget
 **/
#ifndef CCLIB_CcTimerTarget_H_
#define CCLIB_CcTimerTarget_H_

#include "CcBase.h"
#include "dev/CcTimer.h"

class CcTimerTarget : public CcTimer {
public: //methods
  CcTimerTarget();
  virtual ~CcTimerTarget();
  void tick( void );
  time_t getTime(void);

private: //member
  time_t m_SystemTime;
  time_t m_CountDown;
};

#endif /* CCLIB_CcTimerTarget_H_ */
