/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcFileSystem.cpp
 * @brief    Implementation of Class CcFileSystem
 */
#include "CcFileSystem.h"
#include "CcKernel.h"

CcFileSystem::CcFileSystem(void)
{
}

CcFileSystem::~CcFileSystem(void){
}

CcFile *CcFileSystem::getFile(CcString &path){
  CcFile* fileRet=0;
  if (path.begins("/CcBin/"))
  {
    CcString sTemp = path.substr(7);
    if (sTemp.begins("CcDisplay")){
      sTemp = sTemp.substr(sizeof("CcDisplay"));
      uint16 uiTemp = sTemp.toUint16();
      fileRet = (CcFile*)Kernel.getDevice(eDisplay, uiTemp);
    }
    else if (sTemp.begins("CcTouch")){
      sTemp = sTemp.substr(sizeof("CcTouch"));
      uint16 uiTemp = sTemp.toUint16();
      fileRet = (CcFile*)Kernel.getDevice(eTouchPanel, uiTemp);
    }
    else if (sTemp.begins("CcEthernet")){
      sTemp = sTemp.substr(sizeof("CcEthernet"));
      uint16 uiTemp = sTemp.toUint16();
      fileRet = (CcFile*)Kernel.getDevice(eEthernet, uiTemp);
    }
  }
  else if ((path.at(0) == '/' || path.length() >1 || path.at(1) == ':')){
    fileRet = m_FSList.at(0).FS->getFile(path);
  }
  else{
    for (size_t i = 0; i < m_FSList.size(); i++){
      if (path.begins(m_FSList.at(i).Path)){
        fileRet = m_FSList.at(i).FS->getFile(path);
      }
    }
  }
  return fileRet;
}

bool CcFileSystem::mkdir(CcString Path){
  bool bRet = false;
  if ((Path.at(0) == '/') || Path.at(1) == ':'){
    bRet = m_FSList.at(0).FS->mkdir(Path);
  }
  else{
    for (size_t i = 0; i < m_FSList.size(); i++){
      if (Path.begins(m_FSList.at(i).Path)){
        bRet = m_FSList.at(i).FS->mkdir(Path);
      }
    }
  }
  return bRet;
}

bool CcFileSystem::del(CcString Path){
  bool bRet = false;
  if (Path.at(1) == ':' || (Path.at(2) == ':')){
    bRet = m_FSList.at(0).FS->del(Path);
  }
  else{
    for (size_t i = 0; i < m_FSList.size(); i++){
      if (Path.begins(m_FSList.at(i).Path)){
        bRet = m_FSList.at(i).FS->del(Path);
      }
    }
  }
  return bRet;
}


CcString &CcFileSystem::getWorkingDir(void){
  return m_WorkingDir;
}

void CcFileSystem::setWorkingDir(CcString &Path){
  //TODO: check for existense
  m_WorkingDir = Path;
}

void CcFileSystem::addMountPoint(CcString &Path, CcFileSystem* Filesystem){
  sCcFileSystemListItem newItem;
  newItem.FS = Filesystem;
  newItem.Path = Path;
  m_FSList.append(newItem);
}
