/**
* @author     Andreas Dirmeier
*
* @copyright  Technische Software Entwicklung Plazotta (c) 2015
*
* @version    1.00
*
* @par        Language   C++ ANSI V3
*
* @par        History
* @n    20.08.2015 / AD
* @n      Module created
*/
/**
*   @file     WinUSBHid.cpp
*   @brief    Access-class to a HID Device
**************************************************************************/
#include "USB/WinUSBHid.h"
#include "CcStringWin.h"
#include <stdio.h>

WinUSBHid::WinUSBHid(void) :
  m_DeviceHandle(NULL)
{
}

WinUSBHid::~WinUSBHid(void)
{
}

void WinUSBHid::GetDeviceCapabilities(void)
{
	PHIDP_PREPARSED_DATA	PreparsedData;
  HIDP_CAPS   hidCaps;          ///< Stored Device-Settings
  CcStringWin strTemp;
  //get informations of HID
	HidD_GetPreparsedData(
    m_DeviceHandle, 
		&PreparsedData);
  //extract important informations of HID
	HidP_GetCaps(
    PreparsedData, 
		&hidCaps);
  //get Usage to identify the type of HID
  m_Info.usage = hidCaps.Usage + hidCaps.UsagePage;
  //save buffer size for in- and output to members
  m_Info.m_uiReportInputSize = hidCaps.InputReportByteLength;
  m_Info.m_uiReportOutputSize = hidCaps.OutputReportByteLength;
  wchar_t sTemp[100]; //temporary Buffer for return values
  // Request product String of HID and save to Member
  HidD_GetProductString     (m_DeviceHandle, sTemp, 100);
  strTemp.append(sTemp);
  m_Info.m_sProductString = strTemp;
  // Request Manufacturer String of HID and save to Member
  HidD_GetManufacturerString(m_DeviceHandle, sTemp,  100);
  strTemp.clear();
  strTemp.append(sTemp);
  m_Info.m_sVendorString = strTemp;
  // Request product Serialnumber of HID and save to Member
  HidD_GetSerialNumberString(m_DeviceHandle, sTemp, 100);
  strTemp.clear();
  strTemp.append(sTemp);
  m_Info.m_sSerialString = strTemp;
	//No need for PreparsedData any more, so free the memory it's using.
	HidD_FreePreparsedData(PreparsedData);
}

bool WinUSBHid::connect(void)
{
  BOOL         bSuccess(FALSE);
  BOOL         LastDevice(FALSE);
  uint16              MemberIndex = 0;
  HIDD_ATTRIBUTES     Attributes;
  DWORD               DeviceUsage;
  DWORD               Length = 0;
  DWORD               Required;
  HANDLE              hDevInfo;

  SP_DEVICE_INTERFACE_DATA          devInfoData;
  PSP_DEVICE_INTERFACE_DETAIL_DATA  detailData=NULL;
  m_DeviceHandle=INVALID_HANDLE_VALUE; //Baustelle
  HidD_GetHidGuid(&m_HidGuid);  
  
  /*
  API function: SetupDiGetClassDevs
  Returns: a handle to a device information set for all installed devices.
  Requires: the GUID returned by GetHidGuid.
  */
  hDevInfo=SetupDiGetClassDevs 
    (&m_HidGuid, 
    NULL, 
    NULL, 
    DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
  devInfoData.cbSize = sizeof(devInfoData);
  
  do
  {
    //Get all Devices Plugged in
    bSuccess = SetupDiEnumDeviceInterfaces
      (hDevInfo, 
      0, 
      &m_HidGuid, 
      MemberIndex, 
      &devInfoData);

    if (bSuccess != FALSE)
    {
      // collect Information from opened device
      bSuccess = SetupDiGetDeviceInterfaceDetail( 
        hDevInfo, 
        &devInfoData, 
        NULL, 
        0, 
        &Length, 
        NULL);
      // allocate memory depending on Length requested from Interface Detail
      detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);
      //Set cbSize in the detailData structure.
      detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
      //Call the function again, this time passing it the returned buffer size.
      bSuccess = SetupDiGetDeviceInterfaceDetail(
        hDevInfo, 
        &devInfoData, 
        detailData, 
        Length, 
        &Required, 
        NULL);
      // Open a handle to the device.
      m_DeviceHandle=CreateFile(
        detailData->DevicePath, 
        0, 
        FILE_SHARE_READ|FILE_SHARE_WRITE, 
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING, 
        0, 
        NULL);
      //Set the Size to the number of bytes in the structure.
      Attributes.Size = sizeof(Attributes);
      bSuccess = HidD_GetAttributes (
        m_DeviceHandle, 
        &Attributes);
      //Is it the desired device?
      bSuccess = false;
      if (Attributes.VendorID == m_Info.vid && Attributes.ProductID == m_Info.pid)
      {
        //Both the Vendor ID and Product ID match.
        bSuccess = TRUE;
        //Collect Data for HID-Communication Settings
        GetDeviceCapabilities();
        if (m_Info.usage != 0)
        {
          DeviceUsage = m_Info.usage;
          // Check if Device is the required one, like Mouse = Mouse
          if (m_Info.usage == DeviceUsage)
          {
            bSuccess = TRUE;
          }
          else
          {
            bSuccess = FALSE;
          }
        }
        if(bSuccess == TRUE)
        {
          //Target Found, create read- and write-handles to it
          m_WriteHandle=CreateFile 
            (detailData->DevicePath, 
            GENERIC_WRITE, 
            FILE_SHARE_READ|FILE_SHARE_WRITE, 
            (LPSECURITY_ATTRIBUTES)NULL,
            OPEN_EXISTING, 
            0, 
            NULL);
          m_ReadHandle=CreateFile 
            (detailData->DevicePath, 
            GENERIC_READ, 
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            (LPSECURITY_ATTRIBUTES)NULL, 
            OPEN_EXISTING, 
            0, 
            NULL);
        }
      } //if (Attributes.ProductID == ProductID && Attributes.VendorID == VendorID)
      else{
        CloseHandle(m_DeviceHandle);
      }
    free(detailData);
    }  //if (Result != 0)
    else
    {
      //SetupDiEnumDeviceInterfaces returned 0, so there are no more devices to check.
      LastDevice=TRUE;
    }
    MemberIndex = MemberIndex + 1;
  } while ((LastDevice == FALSE) && (bSuccess == FALSE));
  //Clean up requested Buffers
  SetupDiDestroyDeviceInfoList(hDevInfo);
  if (bSuccess == FALSE)
    return false;
  else
    return true;
}

size_t WinUSBHid::write(char* cBuffer, uint32 iLength)
{
  BOOL  bSuccess(FALSE);
  DWORD dwBytesWritten;
  //Baustelle chk for invalid_handle
  if(getReportOutputSize() == iLength)
  {
    bSuccess =WriteFile(
        m_WriteHandle,
        cBuffer,
        iLength,
        &dwBytesWritten,
        NULL);
  }
  if (bSuccess == FALSE)
    return false;
  else
    return true;
}

size_t WinUSBHid::read(char* cBuffer, uint32 iLength)
{
  BOOL  bSuccess(FALSE);
  DWORD dwBytesWritten;
  //Baustelle chk for invalid_handle
  if(getReportInputSize() == iLength)
  {
    bSuccess =ReadFile(
        m_ReadHandle,
        cBuffer,
        iLength,
        &dwBytesWritten,
        NULL);
  }
  if (bSuccess == FALSE)
    return false;
  else
    return true;
}
