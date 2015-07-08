// transc.h : main header file for the TRANSC DLL
//

#if !defined(AFX_TRANSC_H__4FCECEDD_1D9F_488E_9AF1_0FFDDCFBD854__INCLUDED_)
#define AFX_TRANSC_H__4FCECEDD_1D9F_488E_9AF1_0FFDDCFBD854__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "USBTransc.h"
/////////////////////////////////////////////////////////////////////////////
// CTranscApp
// See transc.cpp for the implementation of this class
//

class CTranscApp : public CWinApp
{
public:
	CTranscApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTranscApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTranscApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifdef TRANSC_EXPORTS
#define TRANSC_API __declspec(dllexport)
#else
#define TRANSC_API __declspec(dllimport)
#endif

#ifndef BT_FAIL
    #define BT_FAIL                 0
#endif
#ifndef BT_SUCCESS
    #define BT_SUCCESS              1
#endif
#define BT_DL_FILE_OPEN_FAIL        2
#define BT_DL_FILE_READ_FAIL        3
#define BT_DL_FILE_DOWNLOAD_FAIL    4
#define BT_SECAREA_FORMAT_ERR       5
#define BT_SECAREA_MOUNT_ERR        6
#define BT_DL_COMPARE_FAIL          7
#define BT_DL_UPDATE_BIN_OVER       8
#define BT_PARTION_PARAM_ERR        9
#define BT_PARTION_MEDIUM_ERR       10
#define BT_IMG_MATCH_ERR            11
#define BT_IMG_WRITE_ERR            12
#define BT_FILE_CREATE_FAIL         13
#define BT_FILE_WRITE_FAIL          14 
#define BT_UPDATE_BIN_OVER_RANGE    15

#define ASA_MODE_OPEN		0
#define ASA_MODE_CREATE		1

#define    DOWNLOAD_BIN_FILENAME_SIZE	    (15+1)
/*
typedef enum
{
    MODE_NEWBURN = 1,
    MODE_UPDATE,
}E_BURN_MODE;
*/
typedef enum
{
    E_LINUX_PLANFORM = 1,
	E_ROST_PLANFORM ,
}E_PLANFORM_TYPE;

typedef enum
{
    TRANSC_MEDIUM_NAND,                //nand
    TRANSC_MEDIUM_SPIFLASH,            //spiflash
    TRANSC_MEDIUM_EMMC,                //sd, emmc, inand
    TRANSC_MEDIUM_SPI_EMMC,            //data is stored in spiflash and sd
    TRANSC_MEDIUM_EMMC_SPIBOOT,        //data is stored in sd and boot from spiflash
	TRANSC_MEDIUM_SPI_NAND,            //spi nand
}E_TRANSC_MEDIUM_TYPE;

typedef struct
{
    USHORT eMedium;
    USHORT burn_mode;
}T_MODE_CONTROL;

typedef struct tag_Nand_Phy_Info
{
    UINT32  chip_id;			//芯片ID号
    USHORT  page_size;			//Page大小
    USHORT  page_per_blk;		//一个block的总Page数
    USHORT  blk_num;			//总block数目
    USHORT  group_blk_num;		//
    USHORT  plane_blk_num;		//
    UCHAR   spare_size;			//spare size
    UCHAR   col_cycle;			//column address cycle
    UCHAR   lst_col_mask;		//last column address cycle mask bit
    UCHAR   row_cycle;			//row address cycle
    UCHAR   last_row_mask;		//last row address cycle mask bit
    UCHAR   custom_nd;			//initial bad block type
    UINT32	flag;					//
    UINT32  cmd_len;			//nandflash command length
    UINT32  data_len;			//nandflash data length
    CHAR   des_str[32];		   //描述符
}T_NAND_PHY_INFO;

typedef struct 
{
	UINT32 boot_start_addr;      //nandboot run address
	UINT32 randomize_enable;     //randomize
} nand_boot_info_t;

typedef struct
{
    UINT	chip_id;
    UINT    total_size;             ///< flash total size in bytes
    UINT	page_size;       ///< total bytes per page
    UINT	program_size;    ///< program size at 02h command
    UINT	erase_size;      ///< erase size at d8h command 
    UINT	clock;           ///< spi clock, 0 means use default clock 
    
    //chip character bits:
    //bit 0: under_protect flag, the serial flash under protection or not when power on
    //bit 1: fast read flag    
    BYTE	flag;            ///< chip character bits
    BYTE	protect_mask;    ///< protect mask bits in status register:BIT2:BP0, 
                             //BIT3:BP1, BIT4:BP2, BIT5:BP3, BIT7:BPL
    BYTE    reserved1;
    BYTE    reserved2;
    CHAR   des_str[32];		   //描述符                                    
}T_SFLASH_PHY_INFO;

typedef struct
{
    BOOL  bCompare;
    BOOL  bBackup;
    BOOL  bUpdateSelf;
    TCHAR pc_path[MAX_PATH+1];
    UINT  ld_addr;
    CHAR  file_name[MAX_PATH];
}T_DOWNLOAD_BIN;

typedef struct
{
	BOOL  bCompare;
	TCHAR pc_path[MAX_PATH];
	CHAR  driver_name;
}T_DOWNLOAD_IMG;

typedef struct
{
    BOOL  bCompare;
    TCHAR pc_path[MAX_PATH+1];
    char  udisk_path[MAX_PATH+1];
} T_DOWNLOAD_FILE;

#pragma pack(1)

typedef struct
{
	BYTE Disk_Name;				//盘符名
    BYTE bOpenZone;				//
    BYTE ProtectType;			//	
    BYTE ZoneType;				//
	UINT Size;
	UINT EnlargeSize;         // set this value if enlarge capacity,otherwise set 0 
    UINT HideStartBlock;      //hide disk start
    UINT FSType;
    UINT resv[1];				
}T_PARTION_INFO;

//此结构体是专门为linux平台的spi分区烧录使用的
typedef struct
{
	BYTE Disk_Name;				//盘符名
    BYTE bOpenZone;				//
    BYTE ProtectType;			//	
    BYTE ZoneType;				//
	FLOAT Size;
	UINT EnlargeSize;         // set this value if enlarge capacity,otherwise set 0 
    UINT HideStartBlock;      //hide disk start
    UINT FSType;
    UINT resv[1];				
}T_SPIFLASH_PARTION_INFO;

#pragma pack()


typedef enum
{
    BT_USB_STATUS_IDLE = 0, //idle
    BT_USB_STATUS_WORK,     //Attach usb, start to work
	BT_USB_STATUS_RUNNING,	//the burn thread get event to run or start to run after switch usb
    BT_USB_STATUS_SWITCH,   //only hit switching 1.1 to 2.0 at this time, set status for BT_USB_STATUS_RUNNING after switch
	BT_USB_STATUS_RESET_IDLE,  //between detaching usb1.1 and attaching usb2.0 when this status 
}E_USB_STATUS;

typedef enum
{
    CHIP_3224, //AK_3224
    CHIP_322L, //AK_322L
    CHIP_36XX, //Sundance
    CHIP_780X, //Aspen
    CHIP_880X, //Aspen2
    CHIP_10X6, //Snowbird,snowbirdsA~D
    CHIP_3631, //Sundance2
    CHIP_3671, //Sundance2A
	CHIP_980X,	//aspen3s
	CHIP_3671A,	//sundance2a V6
	CHIP_1080A,	//snowbirdsE
	CHIP_37XX,  //sundance3
	CHIP_11XX,  //AK11
	CHIP_1080L,	//snowbirdL
	CHIP_10XXC,	//10c
	CHIP_39XX,  //AK39
	CHIP_37XX_L,  //37L
    CHIP_RESERVER,
}E_CHIP_TYPE;

typedef struct
{
    UINT reg_addr;
    UINT reg_value;
}T_RAM_REG;

#pragma pack(1)
typedef struct
{
    UINT	num;
    BYTE	dir;
    BYTE	level;
	BYTE	Pullup;
	BYTE	Pulldown;
} T_GPIO_PARAM;
#pragma pack()

typedef enum
{
	E_GPIO_DIR_OUT = 0,
	E_GPIO_DIR_IN,
	E_GPIO_LEVEL_HIGH,
    E_GPIO_LEVEL_LOW,
	E_GPIO_PULL_ABLE,
    E_GPIO_PULL_DISABLE,
}E_GPIO;

typedef struct
{
	UINT nSize_enlarge; 
	CHAR driver_Name;
	CHAR pstrVolumeLable[12];
}T_FORMAT_DRIVER;


typedef struct
{
    CHAR  file_name[DOWNLOAD_BIN_FILENAME_SIZE];
	TCHAR pc_path[MAX_PATH+1];
}T_UPLOAD_BIN;

typedef struct
{
    BYTE DiskName;		
	BYTE Rsv1[3];			
    UINT PageCnt;          
    UINT Rsv2;
    UINT Rsv3;
}T_DRIVER_INFO;

#pragma pack(1)
typedef struct  
{
    UINT  burned_mode;
    UINT  bGpio;
    T_GPIO_PARAM  pwr_gpio_param;
    UINT  bWakup;
    UINT  rtc_wakup_level;
}T_BURNED_PARAM;
#pragma pack()

//片选信息
#pragma pack(1)
typedef struct 
{
	BYTE chip_sel_num;		//片选数
	BYTE gpio_chip_2;					//片选2 ce
	BYTE gpio_chip_3;					//片选3 ce
}T_CHIP_SELECT_DATA;
#pragma pack()

typedef struct
{
    UINT time;	   //set event waitting time(unit second)
    UINT planform;	   //
    UINT ChipType;	   //
    UINT Rsv[18];  //reserve
}T_TRANSC_PARA;


typedef struct
{
    //T_PNANDFLASH nand;  /*nand的结构体*/
    UINT chip;            /*片选*/   
    UINT plane_num;      /*第几个plane*/ 
    UINT block;          /*第几个block*/
    UINT page;           /*block 的page偏移*/
    BYTE *data;           /*需要读写数据buf*/
    BYTE *spare_tbl;       /*需要处理的spare 数据buf*/
    UINT oob_len;        /*oob的长度*/
	UINT page_num;      /*需要写入page个数*/  
	UINT lib_type;      /*0是nftl，1是exnftl*/  
    UINT REV[4];       /*保留之后扩展*/
}T_MEDIUM_RW_SECTOR_INFO, *T_PMEDIUM_RW_SECTOR_INFO;

/**
 * @brief get version of this libarary.
 *
 * @author luqiliu
 * @date 2009-09-23
 * @param[out] MainVer main version.
 * @param[out] SubVer  subsidiary version.
 * @return void
 */
TRANSC_API void   BT_GetVersion(UINT *MainVersion, UINT *SubVersion1, UINT *SubVersion2);

/**
 * @brief initial the transclib.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nBurrnObject counts of burn object.
 * @param[in] BurnProcess  the burn thread callback function.
 * @param[in] BurnProgress  the callback function of rate of burn progress.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_Init(UINT nBurrnObject,  BOOL(* BurnProcess)( UINT nID), VOID (*BurnProgress)(UINT nID, UINT  nDataLen));

/**
 * @brief attach a usb device.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] pstrUSBName name of usb device.
 * @param[in] usbType  type of usb device.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_AttachUSB(TCHAR *pstrUSBName, E_USB_TYPE usbType);

/**
 * @brief detach a usb device.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] pstrUSBName name of usb device.
 * @param[in] usbType  type of usb device.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_DetachUSB(TCHAR *pstrUSBName, E_USB_TYPE usbType);

/**
 * @brief start to burn.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 * @retval modify by luqiliu: 2011-11-22, return burn thread number
 */
TRANSC_API UINT   BT_Start(VOID);

/**
 * @brief set ram register parameter, eg, SDRAM, DDR, DDR2.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] RamReg  the struct array of ram register and value of ram register.
 * @param[in] nNumReg counts of register.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SetRamParam(UINT nID, T_RAM_REG RamReg[], UINT nNumReg);

/**
 * @brief download the file of producer.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] pstrPath  the path of producer.
 * @param[in] runAddr run address of producer.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_DownloadProducer(UINT nID, TCHAR *pstrPath, UINT runAddr);

/**
 * @brief switch usb1.1 to usb2.0.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SwitchUSBHighSpeed(UINT nID);

/**
 * @brief switch all usb device from usb1.1 to usb2.0 parallel.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SwitchUSBHighSpeedAsyn(UINT nID);

/**
 * @brief test whether usb connection is OK.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_TestConnection(UINT nID);
/**
 * @brief set burn mode.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] mode burn mode.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SetMode(UINT nID, T_MODE_CONTROL ModeCtrl);
/**
 * @brief get flash ID(nandflash or spiflash).
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] ce2_gpio gpio number connecting nand CE2.
 * @param[in] ce2_gpio gpio number connecting nand CE3.
 * @param[out] FlashID flash ID(nandflash or spiflash).
 * @param[out] nChipCnt counts of chip.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT	  BT_GetFlashID(UINT nID, UINT ce2_gpio, UINT ce3_gpio, UINT *FlashID,  UINT *nChipCnt);

/**
 * @brief set nandflash parameter.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] NandPhyInfo nandflash phisic parameter info.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SetNandParam(UINT nID, T_NAND_PHY_INFO *NandPhyInfo);

/**
 * @brief initial secure area.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] type type.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_InitSecArea(UINT nID, UINT type);

/**
 * @brief set size of reserve area(unit:MB).
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] nSize size of reserve area.
 * @param[in] bErase whether to erase this area(nanflash).
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SetResvAreaSize(UINT nID, UINT nSize, BOOL bErase);

/**
 * @brief download bin file.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] pDowloadBin parameter of bin file.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_DownloadBin(UINT nID, T_DOWNLOAD_BIN *pDowloadBin);

/**
 * @brief create partition of file systerm.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] PartionInfo partition info.
 * @param[in] nNumPartion partition numbers.
 * @param[in] resvSize file systerm reserve size(unit block counts).
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_CreatePartion(UINT nID, T_PARTION_INFO PartionInfo[], UINT nNumPartion, UINT resvSize);

/**
 * @brief format a partition of file system.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] formatDriver partition info.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_FormatDriver(UINT nID, T_FORMAT_DRIVER *formatDriver);

/**
 * @brief mount partition of file system.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] driverName array of partition name(eg:'A', 'B').
 * @param[in] nDriverNum counts of driver
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_MountDriver(UINT nID, char driverName[], UINT nDriverNum);

/**
 * @brief download boot file.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] pstrPath path of boot file.
 * @param[in] ChipType type of chip(E_CHIP_TYPE).
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_DownloadBoot(UINT nID, TCHAR *pstrPath,  E_CHIP_TYPE ChipType);

/**
 * @brief download image file.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] pDowloadImg parameter of image file.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_DownloadImg(UINT nID, T_DOWNLOAD_IMG *pDowloadImg);

/**
 * @brief download u disk file.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] pDowloadFile parameter of file.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_DownloadFile(UINT nID, T_DOWNLOAD_FILE *pDowloadFile);

/**
 * @brief finish burn task.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_Close(UINT nID);

/**
 * @brief set GPIO
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] the param for GPIO.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SetGPIO(UINT nID, T_GPIO_PARAM *GpioParam);

/**
 * @brief set param to producer manage burn finished status
 *
 * @author wu_haiquan
 * @date 2012-02-01
 * @param[in] nID ----------index of burn channel.
 * @param[in] BurnedParam --the param for producer deal with burn finished status.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SetBurnedParam(UINT nID, T_BURNED_PARAM *BurnedParam);

/**
 * @brief send param to producer set multi nandflash Ce gpio
 *
 * @author wu_haiquan
 * @date 2012-02-19
 * @param[in] nID ----------index of burn channel.
 * @param[in] chip_ce_ctrl --the param for producer deal with nandflash CE gpio pin
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SetNandGpioCe(UINT nID, T_CHIP_SELECT_DATA *chip_ce_ctrl);
/**
 * @brief reset device.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_ResetDevice(UINT nID);

/**
 * @brief get register value by rom.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] regAddr register address.
 * @param[out] regValue register value.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_GetRegValue(UINT nID, UINT regAddr, UINT *regValue);

/**
 * @brief get disk info.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[out] DriverNum counts of driver.
 * @param[out] pDriverInfo info of driver.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_GetDiskInfo(UINT nID,  UINT *DriverNum, T_DRIVER_INFO* pDriverInfo);

/**
 * @brief upload bin file.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[out] pUploadBin parameter of bin file.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT  BT_UploadBin(UINT nID, T_UPLOAD_BIN* pUploadBin);

/**
 * @brief upload bin file for comp data.
 *
 * @author guochunlai
 * @date 2013-10-10
 * @param[in] nID index of burn channel.
 * @param[out] pUploadBin parameter of bin file.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT  BT_UploadBinForComp(UINT nID, T_UPLOAD_BIN* pUploadBin);

TRANSC_API UINT  BT_GetFsStartPos(UINT nID, UINT* pSatrtPos);

TRANSC_API UINT  BT_UploadImgForComp(UINT nID, T_UPLOAD_BIN* pUploadBin, UINT startPos);

/**
 * @brief upload boot file.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] pstrPath path of boot file.
 * @param[in] fileLen length of boot file.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT  BT_UploadBoot(UINT nID, TCHAR* pstrPath, UINT fileLen);

/**
 * @brief upload boot file & comp data.
 *
 * @author guochunlai
 * @date 2013-10-10
 * @param[in] nID index of burn channel.
 * @param[in] pstrPath path of boot file.
 * @param[in] fileLen length of boot file.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */TRANSC_API UINT  BT_UploadBootForComp(UINT nID, TCHAR* pstrPath, UINT fileLen);

/**
 * @brief write secure area file.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] fileName file name.
 * @param[in] pBuf data buffer of file.
 * @param[in] bufSize buffer size.
 * @param[in] mode mode(ASA_MODE_OPEN or ASA_MODE_CREATE).
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT  BT_WriteASAFile(UINT nID, char* fileName, BYTE* pBuf, UINT bufSize, UINT mode);

/**
 * @brief read secure area file.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] fileName file name.
 * @param[in] pBuf data buffer of file.
 * @param[out] bufSize buffer size, MAX size 508.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT  BT_ReadASAFile(UINT nID, char* fileName, BYTE* pBuf, UINT *bufSize);

/**
 * @brief dowload boot file(download file to boot area directly, not to modify boot parameter of file).
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] pstrPath file name.
 * @param[in] nBootLen data buffer of file.
 * @param[in] ChipType chip type(E_CHIP_TYPE).
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_DownloadClientBoot(UINT nID, TCHAR *pstrPath,  UINT nBootLen, E_CHIP_TYPE ChipType);

/**
 * @brief set spiflash parameter.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] SPIPhyInfo spiflash phisic parameter info.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SetSPIParam(UINT nID, T_SFLASH_PHY_INFO *SPIPhyInfo);

/**
 * @brief get ram value by rom.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] AddrStart start address.
 * @param[in] pValueBuf buffer.
 * @param[in] LenRead buffer length.
 * @param[in] bFirst whether to read for the first time.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_GetRamValue(UINT nID, UINT AddrStart, BYTE *pValueBuf, UINT LenRead, BOOL bFirst);

/**
 * @brief download data of a file to appointed address.
 *
 * @author luqiliu
 * @date 2009-09-17
 * @param[in] nID index of burn channel.
 * @param[in] pstrPath  the path of file.
 * @param[in] downloadAddr appointed addresss.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_DownloadRam(UINT nID, TCHAR *pstrPath, UINT downloadAddr);

/**
 * @brief wait for USB attach, after download producer or download change clock program.
 *
 * @author luqiliu
 * @date 2012-02-01
 * @param[in] nID index of burn channel.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_WaitUSBAttach(UINT nID);

/**
 * @brief set some transc control param.
 *
 * @author wu_haiquan
 * @date 2012-02-01
 * @param[in] param struct pointer of transc.
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SetTranscParam(T_TRANSC_PARA *para);

/**
 * @brief download data to ram
 *
 * @author luqiliu
 * @date 2012-02-01
 * @param[in] nID index of burn channel.
 * @param[in] ramAddr--ram addr to write data.
 * @param[in] data_buf -- data buffer
 * @param[in] data_len -- data buffer length
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_DownloadDataToRam(UINT nID, UINT ramAddr, PBYTE data_buf, UINT data_len);

/**
 * @brief get medium(nandflash, SD) data struct info
 *
 * @author luqiliu
 * @date 2012-08-28
 * @param[out] nID index of burn channel.
 * @param[in] pBuf--medium(nandflash, SD) data struct info
 * @param[in] data_buf -- data buffer
 * @param[in] bufSize  -- data buffer length
 * @return UINT
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT  BT_GetMediumStruct(UINT nID,   BYTE *pBuf,  UINT bufSize, UINT *StartBlock);


TRANSC_API UINT  BT_MediumWriteSector(UINT nID, T_PMEDIUM_RW_SECTOR_INFO functionInfo, UINT writeSize, UINT *retVal);

TRANSC_API UINT  BT_MediumReadSector(UINT nID,  T_PMEDIUM_RW_SECTOR_INFO functionInfo,  UINT readSize, UINT *retVal);

TRANSC_API UINT  BT_MediumReadFlag(UINT nID,  BYTE *pBuf, T_PMEDIUM_RW_SECTOR_INFO functionInfo, UINT readSize, UINT *retVal, UINT nNum);

TRANSC_API UINT  BT_MediumEraseBlock(UINT nID, T_PMEDIUM_RW_SECTOR_INFO functionInfo, UINT *retVal, UINT nNum);

TRANSC_API UINT  BT_MediumSetBadBlock(UINT nID,  T_PMEDIUM_RW_SECTOR_INFO functionInfo, UINT *retVal);

TRANSC_API UINT  BT_MediumIsBadBlock(UINT nID,  T_PMEDIUM_RW_SECTOR_INFO functionInfo, UINT *retVal);

TRANSC_API UINT  BT_Medium_Get_BadBlockBuf(UINT nID, UCHAR *buf, UINT buflen);

TRANSC_API UINT  BT_UploadSDPage(UINT nID,  UINT startPage,  PBYTE pBuf,  UINT PageNum, UINT *retVal);

/**
 * @brief read the all freeblock in buf.
 *
 * @author lixingjian
 * @date 2012-11-10
 * @param[in] nID index of burn channel.
 * @param[in] the mode of erase.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT BT_Medium_Read_FreeBlockBuf(UINT nID, BYTE *pBuf, T_PMEDIUM_RW_SECTOR_INFO functionInfo, UINT ooblen, BYTE *ret_val, UINT BlockNum);

/**
 * @brief set erase mode.
 *
 * @author lixingjian
 * @date 2012-11-10
 * @param[in] nID index of burn channel.
 * @param[in] the mode of erase.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_SetEraseMode(UINT nID, UINT erasemode);


/**
 * @brief set size of bin reserve area(unit:MB).
 *
 * @author lixingjian
 * @date 2012-11-21
 * @param[in] nID index of burn channel.
 * @param[in] nSize size of bin reserve area.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_Set_BinResvSize(UINT nID, UINT nSize);


/**
 * @brief create partition of linux spiflash burn
 *
 * @author lixingjian
 * @date 2012-12-17
 * @param[in] nID index of burn channel.
 * @param[in] PartionInfo partition info.
 * @param[in] nNumPartion partition numbers.
 * @param[in] resvSize file systerm reserve size(unit block counts).
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_CreatePartion_SpiFlash(UINT nID, T_SPIFLASH_PARTION_INFO PartionInfo[], UINT nNumPartion, UINT resvSize);


/**
 * @brief udisk burn to get the usb num
 *
 * @author lixingjian
 * @date 2013-1-18
 * @param[in] PID
 * @param[in] VID
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT   BT_GetUSB_Num(UINT PID, UINT VID);


TRANSC_API UINT   BT_DriverName_IsCurrentFlag(TCHAR *pstrUSBName, UINT PID, UINT VID);


/**
 * @brief BT_Get_AttachUSB_flag
 *
 * @author lixingjian
 * @date 2013-1-18
 * @param[in] TCHAR *pstrUSBName
 * @return UINT.
 * @retval 0 or 1
 */
TRANSC_API UINT   BT_Get_AttachUSB_flag(TCHAR *pstrUSBName);

/////////////////////////////////////////////////////////////////////////////

/**
 * @brief BT_DownloadImg_length
 *
 * @author lixingjian
 * @date 2013-1-18
 * @param[in] TCHAR *pstrUSBName
 * @return UINT.
 * @retval 0 or 1
 */
TRANSC_API UINT BT_DownloadImg_length(UINT nID, UINT size);

/**
 * @brief BT_GetFlash_HighID
 *
 * @author lixingjian
 * @date 2013-1-18
 * @param[in] TCHAR *pstrUSBName
 * @return UINT.
 * @retval 0 or 1
 */
TRANSC_API UINT   BT_GetFlash_HighID(UINT nID, UINT ce2_gpio, UINT ce3_gpio, UINT *HighID);


TRANSC_API UINT   BT_Download_and_TestRam(UINT nID, TCHAR *pstrPath, UINT runAddr, UINT RAM_Size);



/**
 * @brief uploadSPI DATA.
 *
 * @author LIXINGJIAN
 * @date 2013-12-31
 * @param[in] nID index of burn channel.
 * @param[out] pUploadBin parameter of bin file.
 * @return UINT.
 * @retval BT_SUCCESS or BT_FAIL
 */
TRANSC_API UINT  BT_Upload_SpiAlldata(UINT nID, TCHAR *pc_path, UINT file_len);


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSC_H__4FCECEDD_1D9F_488E_9AF1_0FFDDCFBD854__INCLUDED_)

