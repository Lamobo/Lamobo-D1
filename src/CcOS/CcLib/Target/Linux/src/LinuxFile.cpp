/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxFile.cpp
 * @brief    Implementation of Class LinuxFile
 */
#include "LinuxFile.h"
#include "unistd.h"
#include "stdio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dirent.h"

LinuxFile::LinuxFile(CcString path):
  m_hFile(0)
{
  m_Path = path;
}

LinuxFile::~LinuxFile( void )
{
  if(m_hFile != 0)
    fclose(m_hFile);
}

bool LinuxFile::createFile(void){
  return false;
}

size_t LinuxFile::read(char* buffer, size_t size){
  ssize_t dwByteRead;
  if ((dwByteRead = ::fread(buffer, sizeof(char), size, m_hFile)) <0)
    return SIZE_MAX;
  else{
    return dwByteRead;
  }
}

size_t LinuxFile::size(void){
  struct stat st;
  if (stat(m_Path.getCharString(), &st) == 0)
      return st.st_size;
  return -1;
}

size_t LinuxFile::write(char* buffer, size_t size){
  ssize_t dwByteWritten;
  if ((dwByteWritten = ::fwrite(buffer, sizeof(char), size, m_hFile)) <0)
    return SIZE_MAX;
  else{
    return dwByteWritten;
  }
}

bool LinuxFile::open(uint16 flags){
  bool bRet(true);
  char flag[3];
  if (flags & Open_Read){
    flag[0] = 'r';
    flag[1] = '\0';
  }
  if (flags & Open_Write){
    flag[0] = 'w';
    flag[1] = '\0';
  }
  m_hFile = fopen(m_Path.getCharString(), flag);
  if (m_hFile != 0)
    bRet = true;
  else{
    bRet = false;
  }
  return bRet;
}

bool LinuxFile::close(void){
  if(m_hFile != 0)
    if(fclose(m_hFile))
      return true;
  return false;
}

bool LinuxFile::isFile(void){
  struct stat sStat;
  stat(m_Path.getCharString(), &sStat);
  if(S_ISREG(sStat.st_mode ))
     return true;
  return false;
}

bool LinuxFile::setFilePointer(size_t pos){
  bool bRet(false);
  //TODO: setting filepointer
  return bRet;
}

bool LinuxFile::isDir(void){
  struct stat sStat;
  stat(m_Path.getCharString(), &sStat);
  if(S_ISDIR(sStat.st_mode ))
     return true;
  return false;
}

CcStringList LinuxFile::getFileList(char showFlags){
  CcStringList slRet;
  CcString appendData;
  if (isDir()){
    DIR           *d;
    struct dirent *dir;
    d = opendir(m_Path.getCharString());
    if (d)
    {
      while ((dir = readdir(d)) != NULL)
      {
        appendData.clear();
        if (showFlags & SHOW_EXTENDED)
        {
          if (dir->d_name[0] != '.')
          {
            if (dir->d_type & DT_DIR)
              appendData.append('d');
            else if (dir->d_type & DT_LNK)
              appendData.append('-');
            else
              appendData.append('-');
            appendData.append("rwxrwxrwx  1 Linux Linux ");
            CcString sTempPath = CcString(dir->d_name);
            sTempPath = m_Path.calcPathAppend(sTempPath);
            size_t sSize = LinuxFile(sTempPath).size();
            appendData.appendNumber(sSize);
            appendData.append(" Jan 1 00:00 ");
            appendData.append(dir->d_name);
            appendData.append(" \r\n");
            slRet.append(appendData);
          }
        }
      }
      closedir(d);
    }
  }
  return slRet;
}

bool LinuxFile::move(CcString &Path){
  if (rename(
                m_Path.getCharString(),
                Path.getCharString()
                ))
  {
    m_Path = Path;
    return true;
  }
  return false;
}

tm LinuxFile::getLastModified(void){
  struct stat sStat;
  tm tRet;
  if (stat(m_Path.getCharString(), &sStat))
  {
    time_t time = sStat.st_mtime;
    localtime_r(&time, &tRet);
  }
  return tRet;
}
