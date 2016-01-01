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
* @date       2015-09
* @par        Language   C++ ANSI V3
* @file     WinUSBHid.h
* @brief    Create Access to an USB-Hid-Device through Windows API
*/
#pragma once

#include "CcBase.h"
#include "Com/CcUSBHid.h"
#include "WindowsGlobals.h"
#include <atlstr.h>
#include <setupapi.h>
#include <hidsdi.h>

#define HIDDEVICE_DEFAULT_STRING_SIZE  32 ///< Default Stringsize for HID

/**
 * @brief Class for communication with a USB-HIDevice
 */
class WinUSBHid : private CcUSBHid
{
public:
  /**
   * @brief Constructor
   */
  WinUSBHid(void);
  /**
   * @brief Destructor
   */
  virtual ~WinUSBHid(void);

  bool connect( void );
  
  /**
   *  @brief Get Device-Settings from Preparsed Data
   */
  void GetDeviceCapabilities( void );

  /**
   * @brief Write Buffer to USBDevice.
   *    Implementation of abstract function form CcIODevice
   * @param[out] cBuffer: Buffer with containing data for writing to device
   * @param[in] iLength: Size of Buffer
   */
  size_t write(char* cBuffer, uint32 iLength);
  
  /**
   * @brief Read Buffer from Device
   *    Implementation of abstract function form CcIODevice
   * @param[in] cBuffer: Buffer get filled with data of device
   * @param[in] iLength: Max read size to Buffer
   */
  size_t read(char* cBuffer, uint32 iLength);

private:
  HANDLE      m_DeviceHandle; ///< Handle to connected HID-Device
  HANDLE      m_WriteHandle;  ///< Write-Handle to write to device
  HANDLE      m_ReadHandle;   ///< Read--Handle to read from device
  GUID        m_HidGuid;      ///< GUID for Comunication with HID-Devices
};
