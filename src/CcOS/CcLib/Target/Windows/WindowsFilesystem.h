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
 * @file     WindowsFilesystem.h
 * @brief    Class WindowsFilesystem
 */
#ifndef WINDOWSFILESYSTEM_H_
#define WINDOWSFILESYSTEM_H_

#include "CcBase.h"
#include "CcFileSystem.h"
#include "CcFile.h"

/**
 * @brief Button for GUI Applications
 */
class WindowsFilesystem : public CcFileSystem{
public:
  /**
   * @brief Constructor
   */
  WindowsFilesystem( void );

  /**
   * @brief Destructor
   */
  virtual ~WindowsFilesystem( void );

  CcFile *getFile(CcString &path);
  bool mkdir(CcString Path);
  bool del(CcString Path);
  CcString &getWorkingDir(void);
};

#endif /* WINDOWSFILESYSTEM_H_ */
