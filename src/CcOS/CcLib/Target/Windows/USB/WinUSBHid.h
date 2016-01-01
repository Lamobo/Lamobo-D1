/**
* @author     Andreas Dirmeier
*
* @copyright  Technische Software Entwicklung Plazotta (c) 2015
*
* @version    1.00
*
* @par        Language   
*         C++ ANSI V3
*
* @par        History
*         20.08.2015 / AD
*           Module created
*/
/**
*   @file     WinUSBHid.h
*   @brief    Access-class to a HID Device
**************************************************************************/
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
