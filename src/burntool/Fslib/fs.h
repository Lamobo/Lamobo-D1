/*
 * @(#)fs.h
 * @date 2010/07/15
 * @version 1.0
 * @author Lu_Qiliu.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FS_H_
#define        _FS_H_

#ifndef SD_BURNER
#include "nandflash.h"
#include "mtdlib.h"
#include "file.h"
#else
#include "medium.h"
#endif

#ifndef SD_BURNER
typedef enum tag_E_AKCHIP_FS
{
    FS_AK32XX = 0,
    FS_AK36XX = 1,
    FS_AK38XX = 2,
    FS_AK78XX = 3,
    FS_AK1036 = 4,
    FS_AK1080 = 5,
    FS_SUNDANCEIIA = 6,
    FS_AK98XX = 7,
    FS_AK37XX = 8,
    FS_AK11XX = 9,
    FS_AK1080L = 10,
    FS_AK10XXC = 11,
    FS_AKERR  = 255
}E_AKCHIP_FS;
#endif

typedef T_VOID (*F_OutStream)(T_U16 ch);
typedef T_U8 (*F_Instream)(T_VOID);
typedef T_U32 (*F_GetSecond)(T_VOID);
typedef T_VOID (*F_SetSecond)(T_U32 seconds);
/* Jan.10,07 - Modified to support Multi-Language */
typedef T_S32 (*F_UniToAsc)(const T_U16 *pUniStr, T_U32 UniStrLen,
                            T_pSTR pAnsibuf, T_U32 AnsiBufLen, T_U32 code);
typedef T_S32 (*F_AscToUni)(const T_pSTR pAnsiStr, T_U32 AnsiStrLen,
                            T_U16 *pUniBuf, T_U32 UniBufLen, T_U32 code);

typedef T_pVOID    (*F_RamAlloc)(T_U32 size, T_S8 *filename, T_U32 fileline);
typedef T_pVOID    (*F_RamRealloc)(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline); 
typedef T_pVOID (*F_RamFree)(T_pVOID var, T_S8 *filename, T_U32 fileline);

/* Sep.13,07 - Added to support Muti-Task. */
typedef T_S32 (*F_OsCrtSem)(T_U32 initial_count, T_U8 suspend_type, T_S8 *filename, T_U32 fileline);
typedef T_S32 (*F_OsDelSem)(T_S32 semaphore, T_S8 *filename, T_U32 fileline);
typedef T_S32 (*F_OsObtSem)(T_S32 semaphore, T_U32 suspend, T_S8 *filename, T_U32 fileline);
typedef T_S32 (*F_OsRelSem)(T_S32 semaphore, T_S8 *filename, T_U32 fileline);

typedef T_U32 (*F_GetChipID)(T_VOID);

typedef T_pVOID (*F_MemCpy)(T_pVOID dst, T_pCVOID src, T_U32 count);
typedef T_pVOID (*F_MemSet)(T_pVOID buf, T_S32 value, T_U32 count);
typedef T_pVOID (*F_MemMov)(T_pVOID dst, T_pCVOID src, T_U32 count);
typedef T_S32   (*F_MemCmp)(T_pCVOID buf1, T_pCVOID buf2, T_U32 count);
typedef T_S32   (*F_Printf)(T_pCSTR s, ...);

typedef T_U32   (*ThreadFunPTR)(T_pVOID pData);
typedef T_U32  (*F_MountThead)(ThreadFunPTR Fun, T_pVOID pData, T_U32 priority);
typedef T_VOID  (*F_KillThead)(T_U32 ThreadHandle);
typedef T_VOID  (*F_SystemSleep)(T_U32 ms);
typedef void F_ChkDskCallback(T_VOID *pData);

typedef T_VOID (*F_MtdSysRst1)(T_VOID);
typedef T_VOID (*F_MtdRandSeed1)(T_VOID);
typedef T_U32 (*F_MtdGetRand1)(T_U32);


typedef struct tag_FsCallback
{
    F_OutStream out;
    F_Instream  in;
    F_GetSecond fGetSecond;
    F_SetSecond fSetSecond;
    F_UniToAsc  fUniToAsc;
    F_AscToUni  fAscToUni;
    F_RamAlloc  fRamAlloc;
    F_RamRealloc fRamRealloc;
    F_RamFree  fRamFree;
    F_OsCrtSem fCrtSem;
    F_OsDelSem fDelSem;
    F_OsObtSem fObtSem;
    F_OsRelSem fRelSem;

    F_MemCpy  fMemCpy;
    F_MemSet  fMemSet;
    F_MemMov  fMemMov;
    F_MemCmp  fMemCmp;
    F_Printf  fPrintf;

    F_GetChipID fGetChipId;

    F_MtdSysRst1    fSysRst;
    F_MtdRandSeed1  fRandSeed;
    F_MtdGetRand1   fGetRand; 
    F_MountThead   fMountThead;   
    F_KillThead   fKillThead;  
    F_SystemSleep  fSystemSleep;
}T_FSCallback, *T_PFSCallback;

typedef T_BOOL (*F_GetDriverCallback)(T_VOID);
typedef T_U32 (*F_DRIVER_Read)  (T_PMEDIUM medium, T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt); 
typedef T_U32 (*F_DRIVER_Write) (T_PMEDIUM medium, const T_U8 *buf,T_U32 BlkAddr, T_U32 BlkCnt); 
typedef 
#ifndef WIN32
__packed
#endif
struct _DRIVER_INFO_
{
    T_U8        DriverID;          
    T_U8        nMainType;		// the value of E_MEDIUM
    T_U8        nSubType;		// the value of E_SUB_MEDIUM
    T_U32       nBlkSize;
    T_U32       nBlkCnt;
    T_PMEDIUM   medium;
    F_DRIVER_Read   fRead;
    F_DRIVER_Write  fWrite;
} DRIVER_INFO, *PDRIVER_INFO;

typedef struct
{
    T_U8  Disk_Name;   //盘符名
    T_U8  bOpenZone;   //E_SUB_MEDIUM--用户 系统盘
    T_U8  ProtectType; //MEDIUM_PORTECT_LEVEL_NORMAL(CHECK, READONLY)        
    T_U8  ZoneType;    //E_FS_ZT ------分区的类型
    T_U32 Size;        //real partiton capacity(Mbit)
    T_U32 EnlargeSize;         //enlarge capacity(Mbit)
    T_U32 HideStartBlock;      //reserve hide disk start 
    T_U32 FSType;              //E_FATFS --FS type
    T_U32 resv[1];             //reserve
}T_FS_PARTITION_INFO;

typedef struct _FOEMAT_INFO_
{
    T_U8  MediumType; /*medium type*/
    T_U32 obj; /*if nand obj = T_PNANDFLASH , else SD obj= PDRIVER_INFO*/
} FORMAT_INFO, *PFORMAT_INFO;

typedef enum 
{
    FS_NAND,   
    FS_SD,     
}E_FS_MEDIUM;


typedef enum 
{
    SYSTEM_PARTITION,   //系统分区,一般是保留分区,用户不可见
    USER_PARTITION,     //用户分区,此分区为U盘可见分区,
}E_SUB_MEDIUM;

typedef enum 
{
    ZT_PRIMARY = 0,
    ZT_SLAVE,
}E_FS_ZT;

/************************************************************************
 * NAME:        FS_InitCallBack
 * FUNCTION  Initial FS callback function
 * PARAM:      [in] fsInitInfo------ callback function struct pointer
                      [in]PagesPerFSBuf--fs cache capacity, unit page
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_InitCallBack(T_PFSCallback fsInitInfo, T_U16 PagesPerFSBuf);

#ifndef SD_BURNER
/************************************************************************
 * NAME:        FS_MountNandFlash
 * FUNCTION  fs mount nandflash, register nand's para
 * PARAM:      [in] gNand------ nand's  para struct pointer
                    [in]StartBlock----fs's start block relative to this medium start position
                    [out]DriverList-----get dirver ID array, must no less than driver's count, max is 26 byte
                    [out]DriverCnt--  driver's count
 * RETURN:    success driver start ID, fail return T_U8_MAX
**************************************************************************/
T_U8 FS_MountNandFlash(T_PNANDFLASH gNand, T_U32 StartBlock, T_U8 DriverList[], T_U8 *DriverCnt);

/************************************************************************
 * NAME:        FS_MountMemDev
 * FUNCTION   fs mount enable remove device, register remove device's para
 * PARAM:      [in] pDevInfo---- need mount's remove device's  para struct pointer
                      [out]DriverCnt--  driver's count
                      [in]StartID--  driver id, system will automatically malloc one id if it is 0xFF
 * RETURN:     success driver start ID, fail return T_U8_MAX
**************************************************************************/
T_U8 FS_MountMemDev(PDRIVER_INFO pDevInfo, T_U8 *DriverCnt, T_U8 StartID);

/************************************************************************
 * NAME:        FS_UnMountMemDev
 * FUNCTION   fs unmount enable remove device
 * PARAM:      [in] DriverID------ need unmount's remove device's driverID

 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_UnMountMemDev(T_U8 DriverID);

/************************************************************************
 * NAME:        FS_Destroy
 * FUNCTION   fs destroy free all fs malloced memory
 * PARAM:      no

 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_Destroy(T_VOID);

/************************************************************************
 * NAME:        FS_UnInstallDriver
 * FUNCTION   fs uninstall driver
 * PARAM:       [in] DriverID------ need remove device's driverID start
                        [in] DriverCnt-----need remove device's driverID count
 * RETURN:     success return be uninstalled driver ID count , fail retuen 0
**************************************************************************/
T_U8 FS_UnInstallDriver(T_U8 DriverID, T_U8 DriverCnt);

/************************************************************************
 * NAME:        FS_InstallDriver
 * FUNCTION   fs install driver, notice: only insert  driver ID to install queue, need wait for thead auto install
 * PARAM:       [in] DriverID------ need install device's driverID start
                        [in] DriverCnt-----need install device's driverID count
 * RETURN:     success return be installed driver ID count , fail retuen 0
**************************************************************************/
T_U8 FS_InstallDriver(T_U8 DriverID, T_U8 DriverCnt);


/************************************************************************
 * NAME:        FS_InstallDriver_CallBack
 * FUNCTION   fs install driver, notice: only insert  driver ID to install queue, need wait for thead auto install
 * PARAM:       [in] DriverID------ need install device's driverID start
                        [in] DriverCnt-----need install device's driverID count
                        [in] F_GetDriverCallback pGet_Griver    
 * RETURN:     success return be installed driver ID count , fail retuen 0
**************************************************************************/
T_U8 FS_InstallDriver_CallBack(T_U8 DriverID, T_U8 DriverCnt, F_GetDriverCallback pGet_Griver);


/************************************************************************
 * NAME:        FS_GetDriver
 * FUNCTION   fs get driver info by driverID
 * PARAM:       [out] pDriverInfo---dirver info
                        [in] DriverID-----need get driver's ID
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_GetDriver(PDRIVER_INFO pDriverInfo, T_U8 DriverID);

/************************************************************************
 * NAME:        FS_GetFakeMedium
 * FUNCTION   fs get the medium of the driver by driverID
 * PARAM:       [in] DriverID-----need get driver's ID
 * RETURN:     return the medium point
**************************************************************************/

T_PMEDIUM FS_GetFakeMedium(T_U8 DriverID);


/************************************************************************
 * NAME:        FS_GetFirstDriver
 * FUNCTION   fs get first driverID's driver info 
 * PARAM:       [out] pDriverInfo---dirver info
                        
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_GetFirstDriver(PDRIVER_INFO pDriverInfo);

/************************************************************************
 * NAME:        FS_GetNextDriver
 * FUNCTION   fs get next driverID's driver info 
 * PARAM:       [out] pDriverInfo---dirver info
                        
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_GetNextDriver(PDRIVER_INFO pDriverInfo);

/************************************************************************
 * NAME:        FS_ChkDsk
 * FUNCTION   fs check disk error and  repair fat link
 * PARAM:       [in] DriverID---need check's dirverID
                       [in]pCallBack--callback function,   chkdsk process index
                       [in]CallbackData-callback function's para
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_ChkDsk(T_U8 DriverID, F_ChkDskCallback pCallBack, T_VOID *CallbackData);

/************************************************************************
 * NAME:        FS_FlushDriver
 * FUNCTION   fs flush driver , flush all data to medium
 * PARAM:       [in] DriverID---need flush's dirverID

 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_FlushDriver(T_U8 DriverID);

/************************************************************************
 * NAME:        FS_CheckInstallDriver
 * FUNCTION   fs check driver is not install
 * PARAM:       [in] DriverID---need check's DriverName("A","B","C"......"Z")

 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_CheckInstallDriver(T_U8 DriverName);

/************************************************************************
 * NAME:        FS_CreateCache
 * FUNCTION   fs create cache for medium
 * PARAM:       [in] medium---need create's medium
                        [in] CacheSize-cache size
 * RETURN:     success return cache medium, fail retuen AK_NULL
**************************************************************************/
T_PMEDIUM FS_CreateCache(T_PMEDIUM medium, T_U32 CacheSize);

/************************************************************************
 * NAME:        FS_DestroyCache
 * FUNCTION   fs destroy cache for medium
 * PARAM:       [in] CacheMedium---need destroy's medium
 * RETURN:     no
**************************************************************************/
T_VOID FS_DestroyCache(T_PMEDIUM CacheMedium);

/************************************************************************
 * NAME:        FS_AsynFlush
 * FUNCTION   fs flush all asynchronism data to device
 * PARAM:       [in] NONE
 * RETURN:     NONE
**************************************************************************/

T_VOID FS_AsynFlush(void);

/************************************************************************
 * NAME:        FS_SetAsynWriteBufSize
 * FUNCTION   fs create cache for medium, if user want to use asyn operator to other driver,
                        user must call the function with other id, it will flush all asyn data with current id,
                        and start new dirver asyn buffer.
 * PARAM:       [in] BufSize---the asyn buffer size
                       [in] DriverID---the asyn operator driver id
 * RETURN:     NONE
**************************************************************************/
T_BOOL FS_SetAsynWriteBufSize(T_U32 BufSize, T_U8 DriverID);

/************************************************************************
 * NAME:        FS_QuickFormatDriver
 * FUNCTION   it will format the dirver with id,it don't change the type of the file system
 * PARAM:       [in] DriverID---the format operator driver id
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/

T_BOOL FS_QuickFormatDriver(T_U8 DriverID);

/************************************************************************
 * NAME:        FS_SetAsynWriteBufSize
 * FUNCTION   fs create cache for medium, if user want to use asyn operator to other driver,
                        user must call the function with other id, it will flush all asyn data with current id,
                        and start new dirver asyn buffer.
 * PARAM:       [in] DriverID---the asyn operator driver id
                       [in] FsType---the type of the format driver
                       
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/

T_BOOL FS_FormatDriver(T_U8 DriverID, E_FATFS FsType);


/************************************************************************
 * NAME:        FS_GetDriverCapacity
 * FUNCTION   it will get the capacity of the driver
 * PARAM:       [in] DriverID---the asyn operator driver id
                       [in] high---the high 32 bit of the driver capacity
                       
 * RETURN:     low 32 bit of the driver capacity
**************************************************************************/

T_U32 FS_GetDriverCapacity(T_U8 DriverID, T_U32 *high);


/************************************************************************
 * NAME:        FS_GetDriverUsedSize
 * FUNCTION   it will get the use size of the driver
 * PARAM:       [in] DriverID---the asyn operator driver id
                       [in] high---the high 32 bit of the driver use size
                       
 * RETURN:     low 32 bit of the driver use size
**************************************************************************/

T_U32 FS_GetDriverUsedSize(T_U8 DriverID, T_U32 *high);



/************************************************************************
 * NAME:        FS_GetDriverFreeSize
 * FUNCTION   it will get the capacity of the driver
 * PARAM:       [in] DriverID---the asyn operator driver id
                       [in] high---the high 32 bit of the driver free size
                       
 * RETURN:     low 32 bit of the driver free size
**************************************************************************/

T_U32 FS_GetDriverFreeSize(T_U8 DriverID, T_U32 *high);

/************************************************************************
 * NAME:       FS_SpeedupUsbNand
 * FUNCTION：it will check some data to connect fs in the mode of usb nand
 * PARAM:  T_PMEDIUM medium
            T_U8 data       the sector data
            T_U32 sector    the sector address
            T_U32 size        the sector size
 * RETURN: none
**************************************************************************/

T_VOID FS_SpeedupUSBNand(T_PMEDIUM medium, const T_U8 data [ ], T_U32 sector, T_U32 size);

/************************************************************************
 * NAME:        FS_ClearAsyn
 * FUNCTION  it will clear some asyn buffer data
 * PARAM:      T_PMEDIUM medium
                       T_U32 sector
                       T_U32 size                      
 * RETURN:      NONE
**************************************************************************/
T_VOID FS_ClearAsyn(T_PMEDIUM medium, T_U32 sector, T_U32 size);

/************************************************************************
 * NAME:       FS_GetVersion
 * FUNCTION：get fs mount version info
 * PARAM: T_VOID
 * RETURN: version info
**************************************************************************/
T_U8 *FS_GetVersion(T_VOID);
#else
/************************************************************************
 * NAME:     FS_GetOldFsMedium
 * FUNCTION：get old fs medium by driver ID
 * PARAM:    [in] DriverID--the fs dirver ID 
 * RETURN:   success return Medium Addr, fail retuen 0
**************************************************************************/
T_U32 FS_GetOldFsMedium(T_U8 DriverID);
#endif

/************************************************************************
* NAME:        FS_FormatFSPartitionInfo
* FUNCTION   Format fs partition and management-self by input partition info. (for burn)

* PARAM:       [in] PartitionInfo--need to managing meidum partition info array
                    [in] nNumPartion--partition info array's number
                    [in ]resvSize------mtd's reserve area size(unit is block)
                    [in]StartBlock-----fs's start block relative to this medium start position
                    [in]pFormatInfo---format require's part of para
* RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_LowFormat(T_FS_PARTITION_INFO PartitionInfo[], 
    T_U32 nNumPartion, 
    T_U32 resvSize, 
    T_U32  StartBlock, 
    PFORMAT_INFO pFormatInfo);

/************************************************************************
 * NAME:       FS_GetDriverCapacity_SecCnt
 * FUNCTION：get driver format real Capacity
 * PARAM:    [in] driverID--driver ID
 * RETURN:   driver format real Capacity, if no driver return 0;
**************************************************************************/
T_U32  FS_GetDriverCapacity_SecCnt(T_U8 DriverID);

/************************************************************************
 * NAME:     FS_GetDriverAttr
 * FUNCTION：get driver Attribute
 * PARAM:    [in] driverID--driver ID
 * RETURN:   success return MEDIUM_PORTECT_LEVEL_NORMAL(CHECK, READONLY), fail return T_U32_MAX;
**************************************************************************/
T_U32  FS_GetDriverAttr(T_U8 DriverID);

/************************************************************************
 * NAME:      FS_MountCacheToDriver
 * FUNCTION   fs mount cache medium for driver
 * PARAM:     [in] medium---the cache medium for driver 
 * RETURN:    success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_MountCacheToDriver(T_PMEDIUM medium);

/************************************************************************
 * NAME:      FS_MountCacheToDriver
 * FUNCTION   fs unmount cache medium for driver
 * PARAM:     [in] medium---the cache medium for driver 
 * RETURN:    success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_UnMountCacheToDriver(T_PMEDIUM medium);

/************************************************************************
 * NAME:     FS_GetAsynBufInfo
 * FUNCTION  check which file when asyn wirte sector error .
 * PARAM:    UseSize--asyn buffer has used size(unit byte)
 *           BufSize--asyn buffer capacity size(unit byte)
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_GetAsynBufInfo(T_U32 *UseSize, T_U32 *BufSize);


/************************************************************************
 * NAME:     FS_UnMountnNandflash
 * FUNCTION  fs unmount nand medium for driver
 * PARAM:    DriverID--driver id
 * RETURN:     success return AK_TRUE, fail retuen AK_FALSE
**************************************************************************/
T_BOOL FS_UnMountnNandflash(T_U8 DriverID);
#ifdef NFTL_RW_FLAG
/************************************************************************
 * NAME:     FS_CurrentRWFlag
 * FUNCTION  Get current NFTL read&write status flag (only apply to AK11 platform).
 *                  
 * PARAM:    file handle; 
 *
 * RETURN:     success return AK_TRUE,  R&W is busy
 *                   fail retuen AK_FALSE, R&W is idle
**************************************************************************/
T_BOOL FS_CurrentRWFlag(T_U32 file);
#endif

#endif //_FS_H_

