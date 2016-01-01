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
 * @file     CcIODevice.h
 * @brief    Class CcIODevice
 */

#ifndef CCIODEVICE_H_
#define CCIODEVICE_H_

#include "CcBase.h"
#include "CcObject.h"

typedef enum {
  eAll = 0,
  eUART,
  eSPI,
  eI2C,
  eDisplay,
  eTouchPanel,
  eEthernet,
  eTimer
}eCcDeviceType;

#define Open_Read   0x0001
#define Open_Write  0x0002
#define Open_RW     0x0003
#define Open_Append 0x0004

#define Open_Overwrite 0x0010
/**
 * @brief Abstract Class for inheriting to every IODevice
 */
class CcIODevice {
public:
  /**
   * @brief Constructor
   */
  CcIODevice();

  /**
   * @brief Destructor
   */
  virtual ~CcIODevice();

  virtual size_t read(char* buffer, size_t size) = 0;
  virtual size_t write(char* buffer, size_t size) = 0;
  virtual bool open(uint16) = 0;
  virtual bool close() = 0;
  virtual bool cancel();

  virtual size_t writeSettings(char* buffer, size_t size);
  virtual size_t readSettings(char* buffer, size_t size);

  void   *m_IOSettings;
  uint16 *m_IOSettingsSize;
};

#endif /* CCIODEVICE_H_ */
