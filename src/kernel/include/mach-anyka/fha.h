#ifndef        _FHA_H_
#define        _FHA_H_

#include "anyka_types.h"
#include "nand_list.h"

#define FHA_SUCCESS 1
#define FHA_FAIL    0

#define VER_NAME_FHA        "FHA"
#define VER_NAME_FS         "FS"
#define VER_NAME_MTD        "MTD"
#define VER_NAME_DRV        "DRV"
#define VER_NAME_MOUNT      "MOUNT"
#define VER_NAME_FSA        "FSA"

typedef enum
{
    FHA_CHIP_880X,      //aspen3
    FHA_CHIP_10XX,      //snowbirds
    FHA_CHIP_980X,      //aspen3s
    FHA_CHIP_37XX,      //Sundance3
    FHA_CHIP_11XX,      //snowbird2
    FHA_CHIP_39XX,      //tmp!!!!!!
}E_FHA_CHIP_TYPE;

#if defined (CONFIG_ARCH_AK98)
#define FHA_CHIP_SET_TYPE      FHA_CHIP_980X
#elif defined (CONFIG_ARCH_AK37)
#define FHA_CHIP_SET_TYPE      FHA_CHIP_37XX
#elif defined (CONFIG_ARCH_AK39)
#define FHA_CHIP_SET_TYPE      FHA_CHIP_39XX
#endif

typedef enum
{
    PLAT_SPOT,          //spot system
    PLAT_SPR,           //spring system    
    PLAT_SWORD,         //sword system
    PLAT_LINUX          //linux system
}E_FHA_PLATFORM_TYPE;

typedef enum
{
    MEDIUM_NAND,                //nand
    MEDIUM_SPIFLASH,            //spiflash
    MEDIUM_EMMC,                //sd, emmc, inand
    MEDIUM_SPI_EMMC,            //data is stored in spiflash and sd
    MEDIUM_EMMC_SPIBOOT,        //data is stored in sd and boot from spiflash
}E_FHA_MEDIUM_TYPE;

typedef enum
{
    MODE_NEWBURN = 1,           //new burn
    MODE_UPDATE,                //update mode
    MODE_UPDATE_SELF,           //update mode self
}E_BURN_MODE;

typedef enum
{
    FHA_DATA_BOOT,
    FHA_DATA_ASA,
    FHA_DATA_BIN,
    FHA_DATA_FS,
    FHA_GET_NAND_PARAM
}E_FHA_DATA_TYPE;

typedef struct
{
    T_U8 lib_name[10];
    T_U8 lib_version[40];
}T_LIB_VER_INFO;


/************************************************************************
 * NAME:     FHA_Erase
 * FUNCTION  callback function, medium erase
 * PARAM:    [in] nChip--meidum chip
 *           [in] nPage--medium page
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
typedef  T_U32 (*FHA_Erase)(T_U32 nChip,  T_U32 nPage); 

/************************************************************************
 * NAME:     FHA_Write
 * FUNCTION  callback function, medium write
 * PARAM:    [in] nChip-----meidum chip
 *           [in] nPage-----medium page
 *           [in] pData-----need to write data pointer addr
 *           [in] nDataLen--need to write data length
 *                          nand(unit byte)
 *                          SD(unit sector count(1sec = 512byte))
 *                          SPI(unit page count, page size in platform define, generally is 256bytes)
 *           [in] pOob------Spare area：Out Of Band, only nand use
 *           [in] nOobLen---Spare area length
 *           [in] eDataType-burn medium data type
 *                          nand -- E_FHA_DATA_TYPE
 *                          SD----- MEDIUM_EMMC
 *                          SPI---- MEDIUM_SPIFLASH
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
typedef T_U32 (*FHA_Write)(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);

/************************************************************************
 * NAME:     FHA_Read
 * FUNCTION  callback function, medium read
 * PARAM:    [in] nChip-----meidum chip
 *           [in] nPage-----medium page
 *           [out]pData-----need to read data pointer addr
 *           [in] nDataLen--need to ren data length
 *                          nand(unit byte)
 *                          SD(unit sector count(1sec = 512byte))
 *                          SPI(unit page count, page size in platform define, generally is 256bytes)
 *           [out]pOob------Spare area：Out Of Band, only nand use
 *           [in] nOobLen---Spare area length
 *           [in] eDataType-burn medium data type
 *                          nand -- E_FHA_DATA_TYPE
 *                          SD----- MEDIUM_EMMC
 *                          SPI---- MEDIUM_SPIFLASH
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
typedef T_U32 (*FHA_Read)(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);

/************************************************************************
 * NAME:     FHA_ReadNandBytes
 * FUNCTION  callback function, nand read no ECC
 * PARAM:    [in] nChip-------meidum chip
 *           [in] rowAddr-----nand physical row addr
 *           [in] columnAddr--nand physical cloumn addr
 *           [out] pData------need to read data pointer addr
 *           [in] nDataLen----need to ren data length
 *                            nand(unit byte)
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
typedef T_U32 (*FHA_ReadNandBytes)(T_U32 nChip, T_U32 rowAddr, T_U32 columnAddr, T_U8 *pData, T_U32 nDataLen);

typedef T_pVOID (*FHA_RamAlloc)(T_U32 size);
typedef T_pVOID (*FHA_RamFree)(T_pVOID var);
typedef T_pVOID (*FHA_MemSet)(T_pVOID pBuf, T_S32 value, T_U32 count);
typedef T_pVOID (*FHA_MemCpy)(T_pVOID dst, T_pCVOID src, T_U32 count);
typedef T_S32   (*FHA_MemCmp)(T_pCVOID pbuf1, T_pCVOID pbuf2, T_U32 count);
typedef T_pVOID (*FHA_MemMov)(T_pVOID dst, const T_pCVOID src, T_U32 count);
typedef T_S32   (*FHA_Printf)(T_pCSTR s, ...);

typedef struct tag_FHA_LibCallback
{
    FHA_Erase Erase;
    FHA_Write Write;
    FHA_Read  Read;
    FHA_ReadNandBytes ReadNandBytes;
    FHA_RamAlloc RamAlloc;
    FHA_RamFree  RamFree;
    FHA_MemSet   MemSet;
    FHA_MemCpy   MemCpy;
    FHA_MemCmp   MemCmp;
    FHA_MemMov   MemMov;
    FHA_Printf   Printf;
}T_FHA_LIB_CALLBACK, *T_PFHA_LIB_CALLBACK;

typedef struct tag_FHA_Init_Info
{
    T_U32 nChipCnt;                //片选数
    T_U32 nBlockStep;            //nand block step值
    E_FHA_CHIP_TYPE     eAKChip;    //AK芯片类型
    E_FHA_PLATFORM_TYPE ePlatform;  //系统类型
    E_FHA_MEDIUM_TYPE   eMedium;    //存储介质类型
    E_BURN_MODE     eMode;        //烧录模式
}T_FHA_INIT_INFO, *T_PFHA_INIT_INFO;

typedef struct tag_FHABinParam
{
    T_U32   data_length;    //数据长度
    T_U32   ld_addr;        //运行地址
    T_U8    file_name[16];  //文件名称
    T_BOOL  bBackup;        //是否备份
    T_BOOL  bCheck;         //是否校验
    T_BOOL  bUpdateSelf;    //spotlight自升级用，会给每个BIN预留同样多的空间    
}T_FHA_BIN_PARAM, *T_PFHA_BIN_PARAM;

typedef struct
{
    T_U32 BinPageStart; /*bin data start addr*/
    T_U32 PageSize; /*spi page size*/
    T_U32 PagesPerBlock;/*page per block*/
}T_SPI_INIT_INFO, *T_PSPI_INIT_INFO;

/************************************************************************
 * NAME:     FHA_burn_init
 * FUNCTION  Initial FHA callback function, init fha init info para
 * PARAM:    [in] pInit----burn platform init struct pointer
 *           [in] pCB------callback function struct pointer
 *           [in] pPhyInfo-input medium struct pointer
 *                         NAND-------T_NAND_PHY_INFO*
 *                         SPIFLASH---T_PSPI_INIT_INFO
 *                         EMMC-------AK_NULL
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_burn_init(T_PFHA_INIT_INFO pInit, T_PFHA_LIB_CALLBACK pCB, T_pVOID pPhyInfo);

/************************************************************************
 * NAME:     FHA_set_resv_zone_info
 * FUNCTION  set reserve area
 * PARAM:    [in] nSize--set reserve area size(unit Mbyte)
 *           [in] bErase-if == 1 ,erase reserve area, or else not erase 
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32 FHA_set_resv_zone_info(T_U32  nSize, T_BOOL bErase);

/************************************************************************
 * NAME:     FHA_set_bin_resv_size
 * FUNCTION  set write bin reserve size(unit byte)
 * PARAM:    [in] bin_size--reserve bin size
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_set_bin_resv_size(T_U32 bin_size);

/************************************************************************
 * NAME:     FHA_write_bin_begin
 * FUNCTION  set write bin start init para
 * PARAM:    [in] bin_param--Bin file info struct
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_write_bin_begin(T_PFHA_BIN_PARAM bin_param);

/************************************************************************
 * NAME:     FHA_write_bin
 * FUNCTION  write bin to medium
 * PARAM:    [in] pData-----need to write bin data pointer addr
 *           [in] data_len--need to write bin data length
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_write_bin(const T_U8 * pData,  T_U32 data_len);

/************************************************************************
 * NAME:     FHA_write_boot_begin
 * FUNCTION  set write boot start init para
 * PARAM:    [in] bin_len--boot length
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_write_boot_begin(T_U32 bin_len);

/************************************************************************
 * NAME:     FHA_write_boot
 * FUNCTION  write boot to medium
 * PARAM:    [in] pData-----need to write boot data pointer addr
 *           [in] data_len--need to write boot data length
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_write_boot(const T_U8 *pData,  T_U32 data_len);

/************************************************************************
 * NAME:     FHA_get_last_pos
 * FUNCTION  get fs start position
 * PARAM:    NULL
 * RETURN:   success return fs start block, fail retuen 0
**************************************************************************/
T_U32  FHA_get_last_pos(void);

/************************************************************************
 * NAME:     FHA_set_fs_part
 * FUNCTION  set fs partition info to medium 
 * PARAM:    [in] pInfoBuf-----need to write fs info data pointer addr
 *           [in] data_len--need to write fs info data length
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_set_fs_part(const T_U8 *pInfoBuf, T_U32 buf_len);

/************************************************************************
 * NAME:     FHA_close
 * FUNCTION  flush all need to save data to medium, and free all fha malloc ram
 * PARAM:    NULL
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_close(T_VOID);

/************************************************************************
 * NAME:     FHA_mount
 * FUNCTION  Initial FHA mount callback function, init fha mount init info para
 * PARAM:    [in] pInit----burn platform init struct pointer
 *           [in] pCB------callback function struct pointer
 *           [in] pPhyInfo-input medium struct pointer
 *                         NAND-------T_NAND_PHY_INFO*, linux platform == AK_NULL
 *                         SPIFLASH---T_PSPI_INIT_INFO
 *                         EMMC-------AK_NULL
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_mount(T_PFHA_INIT_INFO pInit, T_PFHA_LIB_CALLBACK pCB, T_pVOID pPhyInfo);

/************************************************************************
 * NAME:     FHA_read_bin_begin
 * FUNCTION  set read bin start init para
 * PARAM:    [in] bin_param--Bin file info struct
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_read_bin_begin(T_PFHA_BIN_PARAM bin_param);

/************************************************************************
 * NAME:     FHA_read_bin
 * FUNCTION  read bin to buf from medium
 * PARAM:    [out]pData-----need to read bin data buf pointer addr
 *           [in] data_len--need to read bin data length
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_read_bin(T_U8 *pData,  T_U32 data_len);

/************************************************************************
 * NAME:     FHA_get_maplist
 * FUNCTION  get block address map of bin. (only nand)
 * PARAM:    [in] file_name---need to get bin's file name
 *           [out]map_data----need to get bin's block map buf pointer addr
 *           [out]file_len----need to get bin's file length 
 *           [in] bBackup-----if AK_TRUE == bBackup, get backup block map, or else get origin block map
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_get_maplist(T_U8 file_name[], T_U16 *map_data, T_U32 *file_len, T_BOOL bBackup);

/************************************************************************
 * NAME:     FHA_get_nand_para
 * FUNCTION  get nand para
 * PARAM:    [out] pNandPhyInfo--nand info struct pointer
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_get_nand_para(T_NAND_PHY_INFO *pNandPhyInfo);

/************************************************************************
 * NAME:     FHA_get_fs_part
 * FUNCTION  get fs partition info
 * PARAM:    [out]pInfoBuf---need to get fs info data pointer addr
 *           [in] data_len---need to get fs info data length
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_get_fs_part(T_U8 *pInfoBuf , T_U32 buf_len);

/************************************************************************
 * NAME:     FHA_get_resv_zone_info
 * FUNCTION  get reserve area
 * PARAM:    [out] start_block--get reserve area start block
 *           [out] block_cnt----get reserve area block count
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32 FHA_get_resv_zone_info(T_U16 *start_block, T_U16 *block_cnt);

/************************************************************************
 * NAME:     FHA_get_bin_num
 * FUNCTION  get bin file number
 *           [out] cnt----bin file count in medium
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_get_bin_num(T_U32 *cnt);

/************************************************************************
 * NAME:     FHA_set_lib_version
 * FUNCTION  set burn all lib version
 * PARAM:    [in] lib_info--all lib version struct pointer addr
 *           [in] lib_cnt---lib count
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_set_lib_version(T_LIB_VER_INFO *lib_info,  T_U32 lib_cnt);

/************************************************************************
 * NAME:     FHA_get_lib_verison
 * FUNCTION  get burn lib version by input lib_info->lib_name
 * PARAM:    [in-out] lib_info--input lib_info->lib_name, output lib_info->lib_version
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32  FHA_get_lib_verison(T_LIB_VER_INFO *lib_info);

/************************************************************************
 * NAME:     FHA_get_version
 * FUNCTION  get fha version
 * PARAM:    NULL
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U8 *FHA_get_version(void);

/************************************************************************
 * NAME:     FHA_check_lib_version
 * FUNCTION  check burn lib version
 * PARAM:    [in] lib_info lib name and verison
 * RETURN:   success return FHA_SUCCESS, fail retuen FHA_ FAIL
**************************************************************************/
T_U32 FHA_check_lib_version(T_LIB_VER_INFO *lib_info);


#endif      //_FHA_BINBURN_H_


