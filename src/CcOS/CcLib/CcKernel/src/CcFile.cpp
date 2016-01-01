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
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 * @file     CcFile.cpp
 * @brief    Implementation of Class CcFile
 */
#include "CcFile.h"
#include "CcKernel.h"

CcFile::CcFile(CcString path):
  m_Path(path),
  m_filePointer(0)
{
  if (path != ""){
    if (path.begins("/") || path.at(1) == ':')
      m_SystemFile = Kernel.getFileSystem()->getFile(path);
    else{
      m_Path = Kernel.getFileSystem()->getWorkingDir().calcPathAppend(path);
      m_SystemFile = Kernel.getFileSystem()->getFile(m_Path);
    }
  }
  else
    m_SystemFile = 0;
}

CcFile::~CcFile( void )
{
}

size_t CcFile::size(void){
  if(m_SystemFile)
    return m_SystemFile->size();
  return SIZE_MAX;
}

size_t CcFile::read(char* buffer, size_t size){
  if(m_SystemFile)
    return m_SystemFile->read(buffer, size);
  return SIZE_MAX;
}

size_t CcFile::write(char* buffer, size_t size){
  if(m_SystemFile)
    return m_SystemFile->write(buffer, size);
  return SIZE_MAX;
}

bool CcFile::open(uint16 flags){
  if(m_SystemFile)
    return m_SystemFile->open(flags);
  return false;
}

bool CcFile::close(void){
  if(m_SystemFile)
    return m_SystemFile->close();
  return false;
}

bool CcFile::setFilePointer(size_t pos){
  if(m_SystemFile)
    return m_SystemFile->setFilePointer(pos);
  return false;
}

bool CcFile::isFile(void){
  if (m_SystemFile != 0){
    return m_SystemFile->isFile();
  }
  return false;
}

bool CcFile::isDir(void){
  if(m_SystemFile)
    return m_SystemFile->isDir();
  return false;
}

bool CcFile::move(CcString &Path){
  if (m_SystemFile != 0){
    return m_SystemFile->move(Path);
  }
  return false;
}

tm CcFile::getLastModified(void){
  if (m_SystemFile != 0){
    return m_SystemFile->getLastModified();
  }
}

CcStringList CcFile::getFileList(char showFlags){
  if (m_SystemFile != 0)
    return m_SystemFile->getFileList(showFlags);
  return CcStringList();
}

size_t CcFile::write(CcCharArray &charArray, size_t offset, size_t len){
  return write(charArray.getContent(offset, len), charArray.size());
}

size_t CcFile::read(CcCharArray &charArray, size_t offset, size_t len){
  size_t iRet = charArray.size();
  char buf[1024];
  setFilePointer(offset);
  if (len == SIZE_MAX)
    len = size() - offset;
  iRet = len;
  size_t iRead = read(buf, 1024);
  while (iRead != 0 && iRead < len){
    charArray.append(buf, iRead);
    len -= iRead;
    iRead = read(buf, 1024);
  }
  charArray.append(buf, len);
  iRet = charArray.size() - iRet;
  return iRet;
}
