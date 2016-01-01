/**
 * @copyright  Andreas Dirmeier (C) 2015
 *
 * This file is part of CcOS.
 *
 * CcOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CcOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CcOS.  If not, see <http://www.gnu.org/licenses/>.
 **/
/**
 * @author     Andreas Dirmeier
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
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
