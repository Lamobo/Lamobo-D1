/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxSystem.cpp
 * @brief    Class LinuxSystem
 **/
#include "LinuxSystem.h"
#include "LinuxTimer.h"
#include "LinuxDisplay.h"
#include "LinuxTouch.h"
#include "LinuxFilesystem.h"
#include "LinuxSocket.h"
#include "CcKernel.h"
#include "time.h"
#include "pthread.h"
#include "unistd.h"

LinuxSystem::LinuxSystem() {
}

LinuxSystem::~LinuxSystem() {
}

void LinuxSystem::init(void){
  initSystem();
  initTimer();
}

bool LinuxSystem::start( void ){
  m_SystemState=true;
  //start the main loop
  while(m_SystemState){
    sleep(1);
  }
  return true;
}

bool LinuxSystem::initGUI(void){
  initDisplay();
  initTouch();
  return true;
}

bool LinuxSystem::initCLI(void){
  return false;
}

void LinuxSystem::initSystem(void)
{
  m_Filesystem = new LinuxFilesystem();
  CcString str("/");
  Kernel.getFileSystem()->addMountPoint(str, m_Filesystem);
  Kernel.getFileSystem()->setWorkingDir(m_Filesystem->getWorkingDir());
}

void LinuxSystem::initTimer( void ){
  CcTimer *Timer = new LinuxTimer();
  Kernel.setDevice(Timer, eTimer);
}

void LinuxSystem::initDisplay( void ){
#if (CC_USE_GUI > 0)
  m_Display = new LinuxDisplay(500, 500);
  Kernel.setDevice(m_Display, eDisplay);
  m_Display->open();
#endif
}

void LinuxSystem::initTouch( void ){
}

void LinuxSystem::systemTick( void ){
  Kernel.systemTick();
}

void *threadFunction(void *Param){
  CcThread *pThreadObject = static_cast<CcThread *>(Param);
  pThreadObject->enterState(CCTHREAD_RUNNING);
  pThreadObject->run();
  pThreadObject->enterState(CCTHREAD_STOPPED);
  return 0;
}

bool LinuxSystem::createThread(CcThreadObject *Object){
  pthread_t threadId;
  if (0 == pthread_create(&threadId, 0, threadFunction, (void*)Object))
    return true;
  else
    return false;
}

CcSocket* LinuxSystem::getSocket(eSocketType type){
  CcSocket *temp= new LinuxSocket(type);
  return temp;
}

int32 LinuxSystem::getTime( void ){
  return time(0);
}

void LinuxSystem::sleep(time_t timeoutMs){
  usleep(1000 * timeoutMs);
}
