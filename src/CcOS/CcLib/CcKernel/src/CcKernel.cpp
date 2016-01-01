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
 * @file     CcKernel.cpp
 * @brief    Class CcKernel
 */

#include "CcKernel.h"
#include "CcString.h"

#include "TargetConfig.h"
#include CC_TARGET_SYSTEM_INC

CcKernel Kernel;

CcKernel::CcKernel() :
  m_SystemTime(0)
{
  init();
}

CcKernel::~CcKernel() {
  delete m_System;
}

void CcKernel::init(void){
  initDefault();
  m_System = new CC_TARGET_SYSTEM();
  m_System->init();
  CcFileSystem *fsTemp;
  if ((fsTemp = m_System->getFileSystem()) != 0){
    m_FileSystem->setWorkingDir(fsTemp->getWorkingDir());
    CcString Path("/");
    m_FileSystem->addMountPoint(Path, fsTemp);
  }

}
void CcKernel::start(void){
  if(m_SystemStarted == false && m_System != 0)
  {
    m_SystemStarted = true;
    m_System->start();
  }
}

void CcKernel::stop(void){
  m_Threads.closeAll();
  m_AppList.clear();
  m_System->stop();
  delete m_System;
  m_System = 0;
}

void CcKernel::systemReady(){

}

void CcKernel::delayMs(uint32 uiDelay){
  time_t Timeout = uiDelay + getTime();
  m_System->sleep(Timeout);
}

void CcKernel::delayS(uint32 uiDelay){
  delayMs(uiDelay * 1000);
}


bool CcKernel::initGUI(){
  return m_System->initGUI();
}

bool CcKernel::initCLI(){
  return m_System->initCLI();
}

void CcKernel::initFilesystem(void){
  m_FileSystem = new CcFileSystem();
}

void CcKernel::addApp(CcAppCreateFunction appNew, CcString Name){
  sAppListItem newItem;
  newItem.AppNew = appNew;
  newItem.Name = Name;
  m_AppList.append(newItem);
}
CcAppList &CcKernel::getAppList(void){
  return m_AppList;
}
/// @TODO: handling of timer overflow
void CcKernel::addCallDelayed(time_t time, CcObject *Object, uint8 id ){
  uint16 i=0;
  TimerCallbackStruct TCBS;
  TCBS.OH.add(Object, id);
  TCBS.time = time+m_SystemTime;
  for( ; i<TimerCallbackList.size(); i++)
  {
    if(TimerCallbackList[i].time > time)
      break;
  }
  TimerCallbackList.insertAt(i, TCBS);
}

void CcKernel::systemTick( void ){
  m_SystemTime+=10;
  for(uint32 i=0; i<TimerCallbackList.size(); i++){
    if(TimerCallbackList[i].time == m_SystemTime)
    {
      TimerCallbackList[i].OH.call();
      TimerCallbackList.deleteAt(i);
    }
  }
}

bool CcKernel::createThread(CcThreadObject* thread, CcString *sName){
  m_Threads.addThread(thread, sName);
  return m_System->createThread(thread);
}

int CcKernel::createProcess(CcApp *appToStart){
  if (createThread(appToStart))
    return 0;
  else
    return 1;
}

int CcKernel::createProcess(CcProcess &processToStart){
  bool bKernelAppFound = false;
  for (size_t i = 0; i < m_AppList.size(); i++){
    if (m_AppList.at(i).Name == processToStart.m_Name){
      CcApp* app = m_AppList.at(i).AppNew(&processToStart.m_Arguments);
      createThread(app);
      bKernelAppFound = true;
    }
  }
  if (bKernelAppFound){
    return 0;
  }
  else{
    return m_System->createProcess(processToStart);
  }
}

void CcKernel::registerEventReceiver(CcObject *Receiver, uint8 callbackNr){
  m_EventReceiver.add(Receiver, callbackNr);
}

void CcKernel::deleteEventReceiver(CcObject *Receiver){
  m_EventReceiver.remove(Receiver);
}

void CcKernel::emitEvent(CcInputEvent *InputEvent){
  m_EventReceiver.call((void*)InputEvent);
}

CcSystem* CcKernel::getSystem(void){
  return m_System;
}

time_t CcKernel::getTime( void ){
  return m_System->getTime();
}

void CcKernel::setArg(int argc, char **argv){
  delete m_argv;
  m_argc = argc;
  m_argv = argv;
}

void CcKernel::initDefault( void ){
  initFilesystem();
  m_SystemStarted = false;
}

CcIODevice* CcKernel::getDevice(eCcDeviceType Type, uint16 nr){
  uint16 cnt=0;
  for (size_t i = 0; i < m_DeviceList.size(); i++)
  {
    if (m_DeviceList.at(i).Type == Type){
      if (cnt == nr)
        return m_DeviceList.at(i).Device;
      else
        cnt++;
    }
  }
  return 0;
}

CcDeviceList &CcKernel::getDeviceList(void){
  return m_DeviceList;
}

CcSocket* CcKernel::getSocket(eSocketType type){
  return m_System->getSocket(type);
}

CcFileSystem* CcKernel::getFileSystem(void){
  return m_FileSystem;
}

void CcKernel::setDevice(CcIODevice* Device, eCcDeviceType Type){
  sDeviceListItem Item;
  Item.Device = Device;
  Item.Type = Type;
  m_DeviceList.append(Item);
}
