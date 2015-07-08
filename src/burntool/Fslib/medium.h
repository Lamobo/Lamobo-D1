/*
 * @(#)medium.h
 * @date 2009/06/18
 * @version 1.0
 * @author: aijun.
 * @Leader:xuchuang
 * Copyright 2015 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _MEDIUM_H_
#define        _MEDIUM_H_
#include "nandflash.h"
#include "mtdlib.h"

/******************************************************************************/
#define    MEDIUM_PORTECT_LEVEL_NORMAL      0
#define    MEDIUM_PORTECT_LEVEL_CHECK       1
#define    MEDIUM_PORTECT_LEVEL_READONLY    2


typedef enum tag_E_AKCHIP_MTD
{
    MTD_AK32XX = 0,
    MTD_AK36XX = 1,
    MTD_AK38XX = 2,
    MTD_AK78XX = 3,
    MTD_AK1036 = 4,
    MTD_AK1080 = 5,
    MTD_SUNDANCEIIA = 6,
    MTD_AK98XX = 7,
    MTD_AK37XX = 8,
    MTD_AK11XX = 9, 
    MTD_AK1080L = 10,
    MTD_AKERR  = 255
}E_AKCHIP_MTD;


typedef T_pVOID (*F_MtdRamAlloc)(T_U32 size, T_S8 *filename, T_U32 fileline);
typedef T_pVOID (*F_MtdRamRealloc)(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline); 
typedef T_pVOID (*F_MtdRamFree)(T_pVOID var, T_S8 *filename, T_U32 fileline);


typedef T_pVOID (*F_MtdMemCpy)(T_pVOID dst, T_pCVOID src, T_U32 count);
typedef T_pVOID (*F_MtdMemSet)(T_pVOID buf, T_S32 value, T_U32 count);
typedef T_pVOID (*F_MtdMemMove)(T_pVOID dst, const T_pCVOID src, T_U32 count);
typedef T_S8 (*F_MtdMemCmp)(T_pVOID dst, T_pCVOID src, T_U32 count);

/* Sep.13,07 - Added to support Muti-Task. */
typedef T_S32 (*F_MtdOsCrtSem)(T_U32 initial_count, T_U8 suspend_type, T_S8 *filename, T_U32 fileline);
typedef T_S32 (*F_MtdOsDelSem)(T_S32 semaphore, T_S8 *filename, T_U32 fileline);
typedef T_S32 (*F_MtdOsObtSem)(T_S32 semaphore, T_U32 suspend, T_S8 *filename, T_U32 fileline);
typedef T_S32 (*F_MtdOsRelSem)(T_S32 semaphore, T_S8 *filename, T_U32 fileline);

typedef T_S32 (*F_MtdPrintf)(T_pCSTR s, ...);
typedef T_U32 (*F_MtdGetChipID)(T_VOID);
typedef T_VOID (*F_MtdSysRst)(T_VOID);
typedef T_VOID (*F_MtdRandSeed)(T_VOID);
typedef T_U32 (*F_MtdGetRand)(T_U32);
typedef T_U32   (*ThreadFunPTR)(T_pVOID pData);
typedef T_U32  (*F_CreateThread)(ThreadFunPTR Fun, T_pVOID pData, T_U32 priority);
typedef T_VOID  (*F_KillThead)(T_U32 ThreadHandle);
typedef T_VOID  (*F_SystemSleep)(T_U32 ms);


typedef struct tag_MtdCallBackFunc
{
    /* The folling 3 callback functions are about Ram operation. */
    F_MtdRamAlloc   fRamAlloc;
    F_MtdRamRealloc fRamRealloc;
    F_MtdRamFree    fRamFree;

    F_MtdMemCpy fMemCpy;
    F_MtdMemSet fMemSet;
    F_MtdMemMove fMemMove;
    F_MtdMemCmp fMemCmp;

    /* The following 4 callback functions are used for Multi-Task. If the
       running-enviroment is single-task, set them into empty function. */
    F_MtdOsCrtSem fCrtSem;
    F_MtdOsDelSem fDelSem;
    F_MtdOsObtSem fObtSem;
    F_MtdOsRelSem fRelSem;

    F_MtdPrintf    fPrintf;
    F_MtdGetChipID fGetChip;
    F_MtdSysRst    fMtdSysRst;
    F_MtdRandSeed  fMtdRandSeed;
    F_MtdGetRand   fMtdGetRand;
    F_CreateThread fMtdCreateThread;
    F_KillThead    fMtdKillThread;    
    F_SystemSleep  fSystemSleep; 
}T_MTDCALLBACK, *T_PMTDCALLBACK;



/************************************************************************
 * NAME:        MtdLib_SetCallBack
 * FUNCTION  it will initialize all callback function in mtd.Before we can use mtdlib, 
                      we should first call MtdLib_SetCallBack to initialize it.
 * PARAM:      T_PMTDCALLBACK pMtdConfig
 * RETURN:     NONE
**************************************************************************/
T_VOID MtdLib_SetCallBack(T_PMTDCALLBACK pMtdConfig);

/************************************************************************
 * NAME:        Medium_Destroy
 * FUNCTION    it will destroy the medium, and realease all ram in the medium
                      it will flush before destroying it,the medium can't be used again.
 * PARAM:      T_PMEDIUM obj
 * RETURN:     NONE
**************************************************************************/

T_VOID Medium_Destroy(T_PMEDIUM obj);

/************************************************************************
 * NAME:        Nand_CreateMedium
 * FUNCTION  it will init all small mtd information,
 * PARAM:    T_PNANDFLASH nand
                    T_U32 StartBlk  :the start block in nand
                    T_U32 BlkCnt    :the block number
                    T_U16 *RsvBlkNum     :the resver block table
 * RETURN:     it will return medium information if it success, otherwise return AK_NULL.
**************************************************************************/
T_PMEDIUM Nand_CreateMedium(T_PNANDFLASH nand, T_U32 StartBlk, T_U32 BlkCnt, T_U16 *RsvBlkNum);

/************************************************************************
 * NAME:        Medium_CreatePartition
 * FUNCTION  it will create user's partition form MTD
 * PARAM:    T_PMEDIUM large: the medium which Nand_CreateMedium return.
                    T_U32 StartPage:  the start page of the relative address
                    T_U32 PageCnt     the partition size with page
                    T_U32 SecSize      the sector size, it only is 512,1024,2048,4096
                    T_U32 ProtectLevel  it will display the partition attribute
 * RETURN:     it will return the partition information
**************************************************************************/

T_PMEDIUM Medium_CreatePartition(T_PMEDIUM large, T_U32 StartPage,
                                 T_U32 PageCnt, T_U32 SecSize, T_U32 ProtectLevel);


/************************************************************************
 * NAME:        NandMtd_Format
 * FUNCTION  it will erase BlkCnt blocks from StartBlock, it will use MultiErase block if it can
 * PARAM:      T_PNANDFLASH nand
                    T_U32 StartBlock
                    T_U32 BlkCnt
 * RETURN:     it will return bad block numbers
**************************************************************************/
T_U32 NandMtd_Format(T_PNANDFLASH nand, T_U32 StartBlock, T_U32 BlkCnt);



/************************************************************************
 * NAME:        Medium_GetMTDRsvInfo
 * FUNCTION  it will get the reserve block information of the every small NFTL and be used by burn tool
                    it will count the user's block or count the totle block of the current big NFTL with ArrowFlag
 * PARAM:      T_PNANDFLASH nand:   
                    T_U16 StartBlock:           the start block of the MTD
                    T_U16 TolBlock:              it is the user's block
                    T_U16 RsvBlockPerMTD:   the reserve block in each 2048 blocks
                    T_U16 *RsvBlock:            it will save the reserve block of the every small MTD
                    T_U16 *RsvNum:             it will save the MTD number
 * RETURN:    we will return the totle block of the MTD, 
**************************************************************************/
T_U32 Medium_GetMTDRsvInfo(T_PNANDFLASH nand, T_U32 StartBlock, T_U32 TolBlock, 
                                     T_U16 RsvBlockPerMTD, T_U16 *RsvBlock, T_U16 *RsvNum);

/************************************************************************
 * NAME:       Medium_ConnectFS
 * FUNCTION£ºit will check some data to connect fs
 * PARAM:  T_PMEDIUM medium
            T_U8 data       the fat data
            T_U32 sector    the fat address
            T_U32 size        the fat size
 * RETURN: the release block number
**************************************************************************/

void Medium_ConnectFS(T_PMEDIUM medium,
                                      const T_U8 data[], T_U32 sector,T_U32 size);

T_BOOL Medium_CurrentRWFlag(T_VOID);
T_BOOL Medium_IsSafePoweroff(T_VOID);

#endif

