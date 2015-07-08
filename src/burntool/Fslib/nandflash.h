/*
 * @(#)nandflash.h
 * @date 2009/06/18
 * @version 1.0
 * @author: aijun.
 * @Leader:xuchuang
 * Copyright 2015 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifndef __NANDFLASH_H__
#define __NANDFLASH_H__

#include "anyka_types.h"

/* Corresponding to ChipCharacterBits */
#define NANDFLASH_MAPPING_HORIZONTAL    0x10000000	// Horizontal-mapping mode

//nand的特性
#define NANDFLASH_RANDOMISER        17
#define NANDFLASH_COPYBACK          16
#define NANDFLASH_MULTI_COPYBACK    15
#define NANDFLASH_MULTI_READ        14
#define NANDFLASH_MULTI_ERASE       13
#define NANDFLASH_MULTI_WRITE       12
#define NANDFLASH_CACHE_WRITE        1
#define NANDFLASH_CACHE_READ         2

//define the error code of nandflash
typedef enum tag_SNandErrorCode
{
    NF_SUCCESS        =    ((T_U16)1),
    NF_FAIL           =    ((T_U16)0),
    NF_WEAK_DANGER    =    ((T_U16)-1),    			//successfully read, but a little dangerous.
    NF_STRONG_DANGER    =      ((T_U16)-2),			//successfully read, but very dangerous, forbidden to use again.
}E_NANDERRORCODE;

typedef struct SNandflash  T_NANDFLASH;
typedef struct SNandflash *T_PNANDFLASH;


typedef E_NANDERRORCODE (*fNand_WriteSector)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page, const T_U8 data[], T_U8* oob,T_U32 oob_len);
                              
typedef E_NANDERRORCODE (*fNand_ReadSector)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page, T_U8 data[], T_U8* oob,T_U32 oob_len);

                              
typedef E_NANDERRORCODE (*fNand_WriteFlag)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len);
                              
typedef E_NANDERRORCODE (*fNand_ReadFlag)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len);
                              
typedef E_NANDERRORCODE (*fNand_CopyBack)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 SourceBlock, T_U32 DestBlock, T_U32 page);
                              
typedef E_NANDERRORCODE (*fNand_MultiCopyBack)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 PlaneNum, T_U32 SourceBlock, T_U32 DestBlock, T_U32 page);
                              
typedef E_NANDERRORCODE (*fNand_MultiReadSector)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 PlaneNum, T_U32 block, T_U32 page,T_U8 data[], T_U8* SpareTbl,T_U32 oob_len);	//SpareTbl将是包含MutiPlaneNum个T_MTDOOB缓冲的指针
                              
typedef E_NANDERRORCODE (*fNand_MultiWriteSector)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 PlaneNum, T_U32 block, T_U32 page,const T_U8 data[], T_U8* SpareTbl,T_U32 oob_len);	//SpareTbl将是包含MutiPlaneNum个T_MTDOOB缓冲的指针
                              
typedef E_NANDERRORCODE (*fNand_EraseBlock)(T_PNANDFLASH nand, T_U32 chip, T_U32 block);

typedef E_NANDERRORCODE (*fNand_MultiEraseBlock)(T_PNANDFLASH nand, T_U32 chip, T_U32 planeNum, T_U32 block);

typedef T_BOOL (*fNand_IsBadBlock)(T_PNANDFLASH nand, T_U32 chip, T_U32 block);

typedef T_BOOL (*fNand_SetBadBlock)(T_PNANDFLASH nand, T_U32 chip, T_U32 block);

typedef T_U32  (*fNand_Fake2Real)(T_PNANDFLASH nand, T_U32 plane,
                                  T_U32 FakePhyAddr, T_U32 *chip);


//ExNFTL接口函数                        
typedef E_NANDERRORCODE (*fNand_ExReadFlag)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len);
                              
typedef E_NANDERRORCODE (*fNand_ExReadSector)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 plane_num, T_U32 block, T_U32 page,T_U8 data[], T_U8* spare_tbl,T_U32 oob_len, T_U32 page_num);//SpareTbl将是包含MutiPlaneNum个T_MTDOOB缓冲的指针
                              
typedef E_NANDERRORCODE (*fNand_ExWriteSector)(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 plane_num, T_U32 block, T_U32 page,const T_U8 data[], T_U8* spare_tbl,T_U32 oob_len, T_U32 page_num);    //SpareTbl将是包含MutiPlaneNum个T_MTDOOB缓冲的指针
   
typedef E_NANDERRORCODE (*fNand_ExEraseBlock)(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block);

typedef T_BOOL (*fNand_ExIsBadBlock)(T_PNANDFLASH nand, T_U32 chip, T_U32 block);

typedef T_BOOL (*fNand_ExSetBadBlock)(T_PNANDFLASH nand, T_U32 chip, T_U32 block);

struct SNandflash			//介质结构体说明
{
     T_U32   NandType;
    /* character bits, 最高4位表示plane属性，最低位表示是否需要block内顺序写page
       bit31表示是否有copyback，1表示有copyback
       bit30表示是否只有一个plane，1表示只有一个plane
       bit29表示是否前后plane，1表示有前后plane
       bit28表示是否奇偶plane，1表示有奇偶plane
       bit0表示在同一个block内是否需要顺序写page，1表示需要按顺序写，即该nand为MLC
    注意: 如果(bit29和bit28)为'11'，则表示该chip包括4个plane，既有奇偶也有前后plane */
    T_U32   ChipCharacterBits;
    T_U32   PlaneCnt;
    T_U32   PlanePerChip;
    T_U32   BlockPerPlane;
    T_U32   PagePerBlock;
    T_U32   SectorPerPage;
    T_U32   BytesPerSector;
    fNand_WriteSector   WriteSector;
    fNand_ReadSector    ReadSector;
    fNand_WriteFlag     WriteFlag;
    fNand_ReadFlag      ReadFlag;
    fNand_EraseBlock    EraseBlock;
    fNand_CopyBack      CopyBack;
    fNand_IsBadBlock    IsBadBlock;  
    fNand_SetBadBlock    SetBadBlock;
    fNand_Fake2Real     Fake2Real;	// the func of phy addr converting
    fNand_MultiReadSector    MultiRead;
    fNand_MultiWriteSector    MultiWrite;
    fNand_MultiCopyBack        MultiCopyBack;
    fNand_MultiEraseBlock     MultiEraseBlock;
    T_U32     BufStart[1];
	
    //ExNFTL接口函数
    fNand_ExReadFlag      ExReadFlag;
    fNand_ExIsBadBlock    ExIsBadBlock;  
    fNand_ExSetBadBlock   ExSetBadBlock;
    fNand_ExReadSector    ExRead;
    fNand_ExWriteSector   ExWrite;
    fNand_ExEraseBlock    ExEraseBlock;
};
#endif

