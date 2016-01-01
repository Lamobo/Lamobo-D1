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
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 * @file     CcSystem.h
 * @brief    Class CcSystem
 */

#ifndef CCSYSTEM_H_
#define CCSYSTEM_H_

#include "CcBase.h"
#include "CcThreadObject.h"
#include "CcString.h"
#include "CcStringList.h"
#include "CcFileSystem.h"
#include "CcProcess.h"
#include "com/CcSocket.h"

class CcSystem {
public:
  CcSystem();
  virtual ~CcSystem();

  /**
   * @brief Initiates the target System.
   *        This function gets called from initiating Kernel.
   *        Must be implemented by Target.
   */
  virtual void init( void )=0;

  /**
   * @brief Starts the target System.
   *        After all Initiations are done, the Kernel calls this function.
   *        In this state, the Target System must implement an own wile(1).
   */

  virtual bool start(void) = 0;
  /**
   * @brief Stops the target System.
   *        This function gets called from Kernel if it is shutting down.
   *        If required for System, it can be implemented by Target.
   */
  virtual void stop(void);

  /**
   * @brief Initialize the Graphical User Interface.
   *        This function can be implemented by System if devices are available to enable
   *        the graphic routines.
   *        If not implemented the function returns false.
   * @return true, if Interface is successfully established
   */
  virtual bool initGUI(void){ return false; }

  /**
   * @brief Initialize the Command Line Interface.
   *        This function can be implemented by System to enable an text output for
   *        Applications.
   *        If not implemented the function returns false
   * @return true, if Interface is successfully established
   */
  virtual bool initCLI( void ){return false;}

  /**
  * @brief Initialize the Application as Service/Daemon.
  *        This function can be implemented by System to enable the ability to start
  *        a Service-Application.
  *        If not implemented the function returns false
  * @param Name: Name of Service it will be registered to System.
  * @return true, if Interface is successfully established
  */
  virtual bool initService(CcString &Name){ CC_UNUSED(Name); return false; }

  /**
   * @brief Initialize the Filesystem if System has one.
   */
  virtual CcFileSystem* getFileSystem(void){return 0;}

  /**
  * @brief Initialize the Filesystem if System has one.
  */
  virtual CcSocket* getSocket(eSocketType type){ CC_UNUSED(type); return 0; }

  /**
   * @brief Order System to timeout for a specific time.
   * @param timoutMs: timeout in milli seconds
   */
  virtual void sleep(time_t timeoutMs) = 0;

  /**
   * @brief Get time in milliseconds system is running.
   *        It has to be implemented by target system.
   * @return System up time.
   */
  virtual time_t getTime( void ){return 0;};

  virtual bool createThread(CcThreadObject* threadObj) = 0;

  virtual int createProcess(CcProcess &processToStart){ CC_UNUSED(processToStart); return -1; }


  volatile bool m_SystemState;
};

#endif /* CCSYSTEM_H_ */
