/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxFilesystem.cpp
 * @brief    Implementation of Class LinuxFilesystem
 */
#include "LinuxFilesystem.h"
#include "LinuxFile.h"
#include "sys/stat.h"
#include "stdio.h"
#include "unistd.h"

LinuxFilesystem::LinuxFilesystem( void )
{
}

LinuxFilesystem::~LinuxFilesystem( void )
{
}

CcFile *LinuxFilesystem::getFile(CcString &path){
  CcFile *file = new LinuxFile(path);
  return file;
}

bool LinuxFilesystem::mkdir(CcString Path){
  if( ::mkdir(  Path.getCharString(), S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
                                      S_IWOTH | S_IXUSR | S_IXGRP | S_IXOTH ) )
    return true;
  return false;
}

bool LinuxFilesystem::del(CcString Path){
  if (LinuxFile(Path).isFile()){
    if (remove(Path.getCharString()))
      return true;
  }
  else{
    if (rmdir(Path.getCharString()))
      return true;
  }
  return false;
}

CcString &LinuxFilesystem::getWorkingDir(void){
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL){
    m_WorkingDir = cwd;
    m_WorkingDir.strReplace("\\", "/");
  }
  return m_WorkingDir;
}
