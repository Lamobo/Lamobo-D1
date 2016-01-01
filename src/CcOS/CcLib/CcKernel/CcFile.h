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
 * @file     CcFile.h
 * @brief    Class CcFile
 */
#ifndef CcFile_H_
#define CcFile_H_

#include "CcBase.h"
#include "CcIODevice.h"
#include "CcString.h"
#include "CcCharArray.h"
#include "CcStringList.h"

#define SHOW_HIDDEN   0x01
#define SHOW_EXTENDED 0x02

/**
 * @brief Button for GUI Applications
 */
class CcFile : public CcIODevice {
public:
  /**
   * @brief Constructor
   */
  CcFile( CcString path = "");


  /**
   * @brief Destructor
   */
  virtual ~CcFile( void );

  virtual size_t size(void);
  virtual size_t read(char* buffer, size_t size);
  virtual size_t write(char* buffer, size_t size);
  virtual bool open(uint16 flags);
  virtual bool close();
  virtual bool setFilePointer(size_t pos);
  virtual bool isFile(void);
  virtual bool isDir(void);
  virtual bool move(CcString &Path);
  virtual tm getLastModified(void);

  virtual CcStringList getFileList(char showFlags = 0);

  size_t write(CcCharArray &charArray, size_t offset=0, size_t len = SIZE_MAX);
  size_t read(CcCharArray &charArray, size_t offset = 0, size_t len = SIZE_MAX);

protected: //Variables
  CcString m_Path;
  size_t m_filePointer;
private:
  CcFile *m_SystemFile;
};

#endif /* CcFile_H_ */
