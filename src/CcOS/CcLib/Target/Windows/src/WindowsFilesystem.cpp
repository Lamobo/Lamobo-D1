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
 * @file     WindowsFilesystem.cpp
 * @brief    Implementation of Class WindowsFilesystem
 */
#include "WindowsFilesystem.h"
#include "WindowsFile.h"

WindowsFilesystem::WindowsFilesystem( void )
{
}

WindowsFilesystem::~WindowsFilesystem( void )
{
}

CcFile *WindowsFilesystem::getFile(CcString &path){
  CcFile *file = new WindowsFile(path);
  return file;
}

bool WindowsFilesystem::mkdir(CcString Path){
  if( CreateDirectory(  Path.getCharString(),
                        NULL ) )
    return true;
  return false;
}

bool WindowsFilesystem::del(CcString Path){
  if (WindowsFile(Path).isFile()){
    if (DeleteFile(Path.getCharString()))
      return true;
  }
  else{
    CcString winStr = Path;
    winStr.strReplace("/", "\\");
    char *pszFrom = new char[winStr.length() + 2];
    memcpy(pszFrom, winStr.getCharString(), winStr.length());
    pszFrom[winStr.length()] = 0;
    pszFrom[winStr.length() + 1] = 0;


    SHFILEOPSTRUCT fileop;
    fileop.hwnd = NULL;			// no status display
    fileop.wFunc = FO_DELETE;	// delete operation
    fileop.pFrom = pszFrom;		// source file name as double null terminated string
    fileop.pTo = NULL;			// no destination needed
    fileop.fFlags = FOF_NOCONFIRMATION | FOF_SILENT;	// do not prompt the user

    //if (!noRecycleBin)
    //  fileop.fFlags |= FOF_ALLOWUNDO;

    fileop.fAnyOperationsAborted = FALSE;
    fileop.lpszProgressTitle = NULL;
    fileop.hNameMappings = NULL;

    int ret = SHFileOperation(&fileop);

    delete[] pszFrom;

    return (ret == 0);
  }
  return false;
}

CcString &WindowsFilesystem::getWorkingDir(void){
  char buffer[MAX_PATH];
  GetCurrentDirectory(MAX_PATH, buffer);
  m_WorkingDir = buffer;
  m_WorkingDir.strReplace("\\", "/");
  return m_WorkingDir;
}
