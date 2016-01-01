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
*   @file     HidDevice.h
*   @brief    Access-class to a HID Device
**************************************************************************/
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
