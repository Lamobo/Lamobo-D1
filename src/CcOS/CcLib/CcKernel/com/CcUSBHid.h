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
 * @file     CcUSBHid.h
 * @brief    Create Access to an USB-Hid-Device
 */
#pragma once

#include "CcBase.h"
#include "CcString.h"
#include "CcIODevice.h"

typedef struct {
  uint32    vid;   ///< Vendor-ID of HID-Device
  uint32    pid;   ///< Product-ID of HID-Device
  uint32    usage; ///< Usb-Usage+Page for type of HID
  uint32    m_uiReportInputSize; ///< Size of Buffer for Sending Data
  uint32    m_uiReportOutputSize;///< Size of Buffer for Receiving Data
  CcString  m_sVendorString;     ///< Vendor String read from Device
  CcString  m_sProductString;    ///< Product String read from Device
  CcString  m_sSerialString;     ///< Serial-Number read from Device
} CcUSBHidInfo;

/**
 * @brief Class for communication with a USB-HIDevice
 */
class CcUSBHid : private CcIODevice
{
public:
  /**
   * @brief Constructor
   */
  CcUSBHid( void );
  /**
   * @brief Destructor
   */
  virtual ~CcUSBHid( void );

  /**
   * @brief Set VID & PID of Device for connecting to
   * @param iVid:       Vendor-ID of HID-Device
   * @param iPid:       Product-ID of HID-Device
   * @param iUsbUsage:  USB-Page and Usage in Format 0xPPUU,
   *                    default: 0 (parameter is ignored)
   * @return true if connection successfully, false if device not available 
   */
  bool setDevice(uint32 vid, uint32 pid, uint32 usage = 0);

  /**
   * @brief Write Buffer to Device
   * @param[out] cBuffer: Buffer with containing data for writing to device
   * @param[in] iLength: Size of Buffer
   */
  virtual size_t write(char* cBuffer, uint32 iLength);
  
  /**
   * @brief Read Buffer from Device
   * @param[in] cBuffer: Buffer get filled with data of device
   * @param[in] iLength: Max read size to Buffer
   */
  virtual size_t read(char* cBuffer, uint32 iLength);

  /**
   * @brief Readbuffer size for device;
   * @return Returns the Size in Bytes, have to be read from device
   */
  uint32 getReportInputSize(void);
  
  /**
   * @brief Writebuffer size for device;
   * @return Returns the Size in Bytes, have to be written to device
   */
  uint32 getReportOutputSize(void);

protected:
  /**
   * @brief start connecting to device previously set.
   * @return true if connection successfully established
   */
  virtual bool connect(void) = 0;

  CcUSBHidInfo m_Info; ///< Info of connected Device
};
