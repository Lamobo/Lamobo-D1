/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxSystem.h
 * @brief    Class LinuxSystem
 **/

#ifndef LinuxSystem_H_
#define LinuxSystem_H_

#include "CcBase.h"
#include "CcSystem.h"
#include "CcThread.h"
#include "com/CcSocket.h"
#include "CcFileSystem.h"
#include "TargetConfig.h"

class LinuxSystem : public CcSystem{
public:
  LinuxSystem();
  virtual ~LinuxSystem();

  void init(void);
  bool start( void );
  bool initGUI(void);
  bool initCLI(void);
  bool createThread(CcThreadObject* object);
  CcSocket* getSocket(eSocketType type);
  time_t getTime( void );
  virtual void sleep(time_t timeoutMs);

private:
  void initSystem(void);
  void initTimer( void );
  void initDisplay( void );
  void initTouch( void );

  void systemTick( void );

  CcFileSystem *m_Filesystem;
};

#endif /* LinuxSystem_H_ */
