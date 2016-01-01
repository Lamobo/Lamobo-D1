/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     WindowsFile.cpp
 * @brief    Implementation of Class WindowsFile
 */
#include "WindowsFile.h"

WindowsFile::WindowsFile(CcStringWin path):
  CcFile(""),
  m_hFile(INVALID_HANDLE_VALUE)
{
  if (path.at(0) == '/')
    m_Path = path.substr(1);
  else
    m_Path = path;
}

WindowsFile::~WindowsFile( void )
{
}

bool WindowsFile::createFile(void){
  return false;
}

size_t WindowsFile::read(char* buffer, size_t size){
  DWORD dwByteRead;
  if (ReadFile(
    m_hFile,           // open file handle
    buffer,      // start of data to write
    size,  // number of bytes to write
    &dwByteRead, // number of bytes that were written
    NULL))            // no overlapped structure
    return dwByteRead;
  else{
    return SIZE_MAX;
  }
}

size_t WindowsFile::size(void){
  DWORD fSize = GetFileSize(m_hFile, NULL);
  return fSize;
}

size_t WindowsFile::write(char* buffer, size_t size){
  DWORD dwBytesWritten;
  if(!WriteFile(
          m_hFile,           // open file handle
          buffer,      // start of data to write
          size,  // number of bytes to write
          &dwBytesWritten, // number of bytes that were written
          NULL))            // no overlapped structure
  {
    printf("%d", GetLastError());
  }
  return dwBytesWritten;
}

bool WindowsFile::open(uint16 flags){
  bool bRet(true);
  DWORD AccessMode=0;
  DWORD CreateNew=0;
  if (flags & Open_Read){
    AccessMode |= GENERIC_READ;
    CreateNew  |= OPEN_EXISTING;
  }
  if (flags & Open_Write){
    AccessMode |= GENERIC_WRITE;
    if (flags & Open_Overwrite)
      CreateNew |= CREATE_ALWAYS;
    else
      CreateNew |= CREATE_NEW;
  }

  if (bRet != false)
  {
    m_hFile = CreateFile(m_Path.getCharString(),                // name of the write
      AccessMode,          // open for writing
      0,                      // do not share
      NULL,                   // default security
      CreateNew,             // create new file only
      FILE_ATTRIBUTE_NORMAL,  // normal file
      NULL);                  // no attr. template
    if (m_hFile != INVALID_HANDLE_VALUE)
      bRet = true;
    else{
      bRet = false;
      printf("%d", GetLastError());
    }
  }
  return bRet;
}

bool WindowsFile::close(void){
  if(CloseHandle(m_hFile))
    return true;
  return false;
}

bool WindowsFile::isFile(void){
  DWORD dwAttrib = GetFileAttributes(m_Path.getCharString());
  if (dwAttrib != INVALID_FILE_ATTRIBUTES &&
    !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    return true;
  return false;
}

bool WindowsFile::setFilePointer(size_t pos){
  bool bRet(false);
  DWORD FilePointer = SetFilePointer(m_hFile, 0, NULL, NULL);
  if (FilePointer != INVALID_SET_FILE_POINTER)
  {
    FilePointer = pos - FilePointer;
    SetFilePointer(m_hFile, FilePointer, NULL, NULL);
    if (FilePointer != INVALID_SET_FILE_POINTER)
    {
      m_filePointer = pos;
      bRet = true;
    }
  }
  return bRet;
}

bool WindowsFile::isDir(void){
  bool bRet(false);
  DWORD ubRet = GetFileAttributes(m_Path.getCharString());
  if (ubRet & FILE_ATTRIBUTE_DIRECTORY && ubRet != INVALID_FILE_ATTRIBUTES){
    bRet = true;
  }
  return bRet;
}

CcStringList WindowsFile::getFileList(char showFlags){
  CcStringList slRet;
  CcString appendData;
  if (isDir()){
    WIN32_FIND_DATA FileData;
    CcString searchPath(m_Path + "/*");
    HANDLE hDir = FindFirstFile(searchPath.getCharString(), &FileData);
    if (hDir != INVALID_HANDLE_VALUE)
    {
      do{
        appendData.clear();
        if (showFlags & SHOW_EXTENDED)
        {
          if (FileData.cFileName[0] != '.' || showFlags & SHOW_HIDDEN)
          {
            if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
              appendData.append('d');
            else
              appendData.append('-');
            appendData.append("rwxrwxrwx  1 windows windows ");
            size_t size = FileData.nFileSizeHigh;
            size = size << 32;
            size += FileData.nFileSizeLow;
            appendData.appendNumber(size);
            appendData.append(" Jan 1 00:00 ");
            CcStringWin temp(FileData.cFileName);
            appendData.append(temp.getCharString());
            appendData.append(" \r\n");
          }
        }
        else{
          if (FileData.cFileName[0] != '.' || showFlags & SHOW_HIDDEN)
          {
            CcStringWin temp(FileData.cFileName);
            temp.append("\r\n");
            appendData.append(temp.getCharString());
          }
        }
        slRet.append(appendData);
      } while (FindNextFile(hDir, &FileData));
      FindClose(hDir);
    }
  }
  return slRet;
}

bool WindowsFile::move(CcString &Path){
  if (MoveFile(
                m_Path.getCharString(),
                Path.getCharString()
                ))
  {
    m_Path = Path;
    return true;
  }
  return false;
}

tm WindowsFile::getLastModified(void){
  FILETIME winTime;
  tm tRet;
  memset(&tRet, 0, sizeof(tm));
  if (GetFileTime(m_hFile, NULL, NULL, &winTime))
  {
    SYSTEMTIME stTime;
    if (FileTimeToSystemTime(&winTime, &stTime)){
      tRet.tm_sec   = stTime.wSecond;
      tRet.tm_min   = stTime.wMinute;
      tRet.tm_hour  = stTime.wHour;
      tRet.tm_mday  = stTime.wDay;
      tRet.tm_mon   = stTime.wMonth;
      tRet.tm_year  = stTime.wYear;
      tRet.tm_wday  = stTime.wDayOfWeek;
      tRet.tm_yday  = 0;
    }
  }
  return tRet;
}
