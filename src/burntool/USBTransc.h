
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the USBTRANSC_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// USBTRANSC_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifndef __USB_TRANSC_H_
#define __USB_TRANSC_H_

#ifdef USBTRANSC_EXPORTS
#define USBTRANSC_API __declspec(dllexport)
#else
#define USBTRANSC_API __declspec(dllimport)
#endif

#define USB_MAINVERSION 1
#define USB_SUBVERSION  10

#define BT_FAIL 0				//return fail
#define BT_SUCCESS 1			//return success	

typedef enum
{
	USB_M3USB = 0,				//usb device for sundance2, aspen3
	USB_NBUSB,					//usb device for snowbird
	USB_MASS,					//umass device
	USB_AP3USB,					//usb device for sundance3,aspen3s
	USB_SNOWB,					//usb device for snowbirds
	USB_NULL
}
E_USB_TYPE;

typedef enum
{
	DATA_STAGE_NONE,			//no data stage, only send command
	DATA_STAGE_IN,				//data input stage for PC
	DATA_STAGE_OUT				//data output stage for PC
}
E_DATA_STAGE;

typedef struct
{
	BYTE  cmd;					//command
	E_DATA_STAGE data_stage;	//data stage of E_DATA_STAGE
	UINT cmd_param[2];			//parameter
	
	PBYTE data_buffer;			//data buffer
	UINT data_length;			//the length of data buffer

	UINT ack_status;			
}
T_USB_TRANSC;

/**
 * @brief open usb device depend on usbType .
 *
 * @author liaozhijun
 * @date 2009-09-23
 * @param[in] strUSBName usb device name.
 * @param[in] usbType usb device type.
 * @return HANDLE
 * @retval handle of usb device
 */
USBTRANSC_API HANDLE BT_OpenUSB(LPTSTR strUSBName, E_USB_TYPE usbType);

/**
 * @brief close usb device .
 *
 * @author liaozhijun
 * @date 2009-09-23
 * @param[in] hUSB handle of usb device.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
USBTRANSC_API UINT BT_CloseUSB(HANDLE hUSB);

/**
 * @brief send usb transc .
 *
 * @author liaozhijun
 * @date 2009-09-23
 * @param[in] hUSB handle of usb device.
 * @param[in] pTransc parameter of usb transc.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
USBTRANSC_API UINT BT_CommitTransc(HANDLE hUSB, T_USB_TRANSC *pTransc);

/**
 * @brief register usb device to window for not umass device.
 *
 * @author liaozhijun
 * @date 2009-09-23
 * @param[in] hWnd handle of the window.
 * @param[in] usbType usb device type.
 * @return BOOL
 * @retval TRUE or FALSE
 */
USBTRANSC_API BOOL BT_RegisterDevice(HWND hWnd, E_USB_TYPE usbType);

/**
 * @brief get version of this libarary.
 *
 * @author liaozhijun
 * @date 2009-09-23
 * @param[out] MainVer main version.
 * @param[out] SubVer  subsidiary version.
 * @return void
 */
USBTRANSC_API void BT_GetUSBLibVersion(UINT *MainVer, UINT *SubVer);

/**
 * @brief read data from anyka usb device(not umass device).
 *
 * @author liaozhijun
 * @date 2009-09-23
 * @param[in] dataBuf data buffer.
 * @param[in] dwLen  buffer length.
 * @return DWORD
 * @retval read length
 */
USBTRANSC_API DWORD BT_ReadData(HANDLE hUSB, BYTE dataBuf[], DWORD dwLen);

/**
 * @brief write data to anyka usb device(not umass device).
 *
 * @author liaozhijun
 * @date 2009-09-23
 * @param[in] dataBuf data buffer.
 * @param[in] dwLen  buffer length.
 * @return DWORD
 * @retval writter length
 */
USBTRANSC_API DWORD BT_WriteData(HANDLE hUSB, BYTE dataBuf[], DWORD dwLen);

/**
 * @brief write data to anyka usb device(not umass device).
 *
 * @author luqiliu
 * @date 2012-06-20
 * @param[in] hUSB    USB handle
 * @param[in] SCSI    expand SCSI
 * @param[in] dataBuf data buffer.
 * @param[in] dwLen   buffer length MAX 5 BYTE.
 * @return DWORD
 * @retval BT_SUCCESS & BT_FAIL
 */
USBTRANSC_API DWORD BT_SendExSCSICommand(HANDLE hUSB, BYTE SCSI, BYTE dataBuf[], DWORD dwLen);


#endif