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
 * @file     CcFileSystem.h
 * @brief    Class CcFileSystem
 */
#ifndef CCFILESYSTEM_H_
#define CCFILESYSTEM_H_

#include "CcBase.h"
#include "CcFile.h"
#include "CcVector.h"

class CcFileSystem;

typedef struct {
  CcString      Path;
  CcFileSystem  *FS;
} sCcFileSystemListItem;

/**
 * @brief Button for GUI Applications
 */
class CcFileSystem {
public:
  /**
   * @brief Constructor
   */
  CcFileSystem(void);

  /**
   * @brief Destructor
   */
  virtual ~CcFileSystem(void);

  virtual CcFile *getFile(CcString &path);

  virtual void addMountPoint(CcString &Path, CcFileSystem* Filesystem);
  virtual CcString &getWorkingDir(void);
  virtual bool mkdir(CcString Path);
  virtual bool del(CcString Path);
  void setWorkingDir(CcString &Path);

protected:
  CcString m_WorkingDir;
private:
  CcVector<sCcFileSystemListItem> m_FSList;
};

#endif /* CcFileSystem_H_ */
