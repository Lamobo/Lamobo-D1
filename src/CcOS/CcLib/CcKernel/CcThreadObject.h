/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcThreadObject.h
 * @brief    Class CcThreadObject
 */
#ifndef CCTHREADOBJECT_H_
#define CCTHREADOBJECT_H_

#include "CcBase.h"
#include "CcObject.h"

typedef enum{
  CCTHREAD_STARTING,
  CCTHREAD_RUNNING,
  CCTHREAD_STOPPING,
  CCTHREAD_STOPPED,
} eThreadState;

/**
 * @brief Default Class to create a Application
 */
class CcThreadObject {
public:
  CcThreadObject();
  virtual ~CcThreadObject();

  /**
   * @brief Virtual function for Startup-Code
   *        Can be implemnted by inhering Class.
   *        Gets called before thread is starting.
   */
  void start ( void );

  /**
   * @brief Virtual function for Running-Code
   *        Must be implemented by target application.
   */
  virtual void run   ( void ) = 0;

  /**
   * @brief Virtual function for Stop-Code
   *        Can be implemnted by inhering Class.
   *        Gets called after thread has ended.
   */
  void stop  ( void );

  /**
   * @brief Signal to Thread next State;
   * @param State: State to set
   */
  virtual void enterState(eThreadState State);

  /**
  * @brief Get actual State of Thread
  * @return State value
  */
  eThreadState getThreadState(void);
private:
  eThreadState m_State;
};

#endif /* CCTHREADOBJECT_H_ */
