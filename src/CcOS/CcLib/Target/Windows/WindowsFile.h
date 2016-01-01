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
 * @file     WindowsFile.h
 * @brief    Class WindowsFile
 */
#ifndef WindowsFile_H_
#define WindowsFile_H_

#include "CcBase.h"
#include "CcStringWin.h"
#include "WindowsGlobals.h"
#include "CcFile.h"

/**
 * @brief Button for GUI Applications
 */
class WindowsFile : public CcFile {
public:
  /**
   * @brief Constructor
   */
  WindowsFile(CcStringWin path = "");

  /**
   * @brief Destructor
   */
  virtual ~WindowsFile( void );

  size_t size(void);
  size_t read(char* buffer, size_t size);
  size_t write(char* buffer, size_t size);
  bool open(uint16 flags);
  bool close(void);
  bool isFile(void);
  bool isDir(void);
  bool move(CcString &Path);
  tm getLastModified(void);

  virtual bool setFilePointer(size_t pos);

  bool createFile();

  CcStringList getFileList(char showFlags = 0);

private:
  HANDLE m_hFile;
};

#endif /* WindowsFile_H_ */
