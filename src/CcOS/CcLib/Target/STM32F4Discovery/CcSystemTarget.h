/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcSystemTarget.h
 * @brief    Class CcSystemTarget
 **/

#ifndef CCSYSTEMTARGET_H_
#define CCSYSTEMTARGET_H_

#include "CcBase.h"
#include "CcSystem.h"
#include "CcTimerTarget.h"
#include "dev/CcTimer.h"
#include "dev/CcGPIO.h"
#include "dev/CcDisplay.h"
#include "dev/CcTouch.h"
#include "CcThread.h"

class CcSystemTarget: public CcSystem {
public:
  CcSystemTarget();
  virtual ~CcSystemTarget();

  void init(void);
  bool start( void );
  bool initGUI(void);
  bool initCLI(void);
  time_t getTime( void );
  bool createThread(CcThread *object);
private:
  void initSystem(void);
  void initTimer( void );
  void initGPIO( void );
  void initDisplay( void );
  void initTouch( void );

  CcGPIO*    m_GPIO;
  CcDisplay* m_Display;
  CcTouch*   m_Touch;
  CcTimerTarget*   m_Timer;
};

#endif /* CCSYSTEMTARGET_H_ */
