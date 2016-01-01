/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcKernel.h
 * @brief    Class CcKernel
 */

#ifndef CCKERNEL_H_
#define CCKERNEL_H_


#include "CcBase.h"
#include "CcSystem.h"
#include "CcVector.h"
#include "CcObject.h"
#include "CcObjectHandler.h"
#include "CcInputEvent.h"
#include "CcIODevice.h"
#include "CcApp.h"
#include "CcThreadObject.h"
#include "CcThreadManager.h"
#include "CcLog.h"
#include "CcFileSystem.h"
#include "CcDeviceList.h"
#include "CcAppList.h"
#include "CcProcess.h"

typedef struct{
  time_t time;
  CcObjectHandler OH;
} TimerCallbackStruct;

class CcKernel  {
public:
  CcKernel();
  virtual ~CcKernel();

  void init( void );
  void start( void );
  void stop(void);
  bool initGUI( void );
  bool initCLI( void );

  void addCallDelayed(time_t time, CcObject *Object, uint8 id);
  void setArg(int arg, char **argv);
  void systemTick( void );
  void systemReady( void );
  void delayMs(uint32 uiDelay);
  void delayS(uint32 uiDelay);
  bool createThread(CcThreadObject *Thread, CcString *sName=0);
  int  createProcess(CcApp *appToStart);
  int  createProcess(CcProcess &processToStart);
  void registerEventReceiver(CcObject *Receiver, uint8 callbackNr);
  void deleteEventReceiver(CcObject *Receiver);
  void emitEvent(CcInputEvent *InputEvent);
  void          addApp(CcAppCreateFunction appFunc, CcString Name);
  CcAppList    &getAppList(void);
  CcSystem     *getSystem(void);
  time_t        getTime(void);
  void          setDevice(CcIODevice* Device, eCcDeviceType Type = eAll);
  CcIODevice*   getDevice(eCcDeviceType Type = eAll, uint16 nr=0);
  CcDeviceList &getDeviceList(void);
  CcSocket*     getSocket(eSocketType type);
  /**
  * @brief Initialize the Filesystem.
  * Check if System has one, otherwise Kernel will create one.
  */
  void initFilesystem(void);
  CcFileSystem* getFileSystem(void);

private:
  void initDefault( void );
  CcSystem  *m_System;
  time_t m_SystemTime;
  CcVector<TimerCallbackStruct> TimerCallbackList;
  CcObjectHandler m_EventReceiver;

private:
  int    m_argc;              ///< Count of Startup Parameters TODO: replace with StringList
  char **m_argv;              ///< Startup parameters TODO: replace with StringList
  bool   m_SystemStarted;     ///< Check if Target-System is started
  CcFileSystem *m_FileSystem; ///< Handle to Filesystem, set by System or Kernel itself.
  CcAppList m_AppList; ///< Applications currently registered to Kernel
  CcThreadManager m_Threads;  ///< Managing all created Threads
  CcDeviceList m_DeviceList; ///< List of Devices registered to Kernel for lowlevel access
  CcLog  m_Log;               ///< Log-Manager to handle Kernel-Output messages
};

#endif /* CCKERNEL_H_ */
