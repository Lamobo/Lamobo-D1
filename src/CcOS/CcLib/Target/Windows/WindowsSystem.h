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
 * @file     WindowsSystem.h
 * @brief    Class WindowsSystem
 **/

#ifndef WindowsSystem_H_
#define WindowsSystem_H_

#include "CcBase.h"
#include "CcSystem.h"
#include "TargetConfig.h"
#include "WindowsSocket.h"
#include "WindowsTimer.h"
#include "WindowsDisplay.h"
#include "WindowsTouch.h"
#include "WindowsService.h"

class WindowsSystem : public CcSystem{
public:
  WindowsSystem();
  virtual ~WindowsSystem();

  void init(void);
  bool initGUI(void);
  bool initCLI(void);
  bool initService(void);
  bool start(void);
  void stop(void);
  bool createThread(CcThreadObject* threadObj);
  int createProcess(CcProcess &processToStart);
  void loadModule(CcString &Path);
  time_t getTime(void);
  void sleep(time_t timeoutMs);
  CcFileSystem* getFileSystem(void);
  CcSocket* getSocket(eSocketType type);

private:
  void initSystem(void);
  void initTimer( void );
  void initDisplay( void );
  void initTouch( void );
  void initFilesystem( void );

  void systemTick( void );
  WindowsDisplay*  m_Display;
  WindowsTimer*    m_Timer;
  WindowsService*  m_Service;
  CcFileSystem*    m_Filesystem;
  bool m_GuiInitialized;
};

#endif /* WindowsSystem_H_ */
