/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
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
