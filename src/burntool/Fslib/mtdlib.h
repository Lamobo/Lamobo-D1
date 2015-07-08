/*
 * @(#)mtdlib.h
 * @date 2009/06/18
 * @version 1.0
 * @author: aijun.
 * @Leader:xuchuang
 * Copyright 2015 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __MTDLIB_H__
#define __MTDLIB_H__

#include "anyka_types.h"

typedef    struct tag_Medium  T_MEDIUM;
typedef    struct tag_Medium *T_PMEDIUM;

typedef    T_U32 (*F_ReadSector)(T_PMEDIUM medium, T_U8* buf, T_U32 start, T_U32 size);
typedef    T_U32 (*F_WriteSector)(T_PMEDIUM medium, const T_U8 *buf, T_U32 start, T_U32 size);
typedef    T_VOID (*F_DeleteSector)(T_PMEDIUM medium,T_U32 StartSce,T_U32 SecSize);
typedef    T_BOOL (*F_Flush)(T_PMEDIUM medium);

typedef    struct    Object    T_OBJECT;
typedef    struct    Object *T_POBJECT;

typedef    T_VOID    (*F_DESTROY)(T_POBJECT);

typedef enum
{
    MEDIUM_RAM       = 0,
    MEDIUM_ROM       = 1,
    MEDIUM_NANDFLASH = 2,
    MEDIUM_SD        = 3,
    MEDIUM_NORFLASH  = 4,
    MEDIUM_DISKETTE  = 5,
    MEDIUM_FILE      = 6,
    MEDIUM_NANDRES   = 7,
    MEDIUM_USBHOST   = 8,
    MEDIUM_PARTITION = 9,

    MEDIUM_UNKNOWN   = 255
}E_MEDIUM;


/*
    define all object type
    enumeration start with "E_"
*/
typedef enum
{
    TYPE_OBJECT ,
    TYPE_BYTE   ,
    TYPE_WORD   ,
    TYPE_LONG   ,
    TYPE_STRING ,
    TYPE_LIST   ,
    TYPE_LINK   ,
    TYPE_BLINK  ,
    TYPE_CLUSTER,
    TYPE_MEDIUM ,
    TYPE_DRIVER ,
    TYPE_FAT    ,    
    TYPE_FMSG   ,
    TYPE_FILE   ,
    TYPE_ATTR   ,
    TYPE_FID
}E_TYPE;


struct    Object
{
    T_U32 type;
    F_DESTROY destroy;
};

struct tag_Medium
{
    T_OBJECT object;
    T_U8  type;		// Corresponding to E_MEDIUM type.
    T_U8  SecBit;	// 2^n = BytePerSec.
    T_U8  PageBit;	// 2^n = BytePerPage.
    T_U8  SecPerPg;	// PageBit -SecBit .2^n = SecPerPage.
    T_U32 capacity;
    F_ReadSector read;
    F_WriteSector write;
    F_DeleteSector DeleteSec;	// it will output to file system , it is only used in NANDFLASH, it will be ak_null in other medium
    F_Flush flush;
    T_U8* msg;
    T_BOOL readVirtualCpt;
	T_U8 MTDindex;
};

/************************************************************************
 * NAME:        MtdLib_GetVersion
 * FUNCTION  it will return the version of MTD.
 * PARAM:      NONE
 * RETURN:     the version string
**************************************************************************/

T_S8*  MtdLib_GetVersion(T_VOID);

/************************************************************************
 * NAME:        Medium_Destroy
 * FUNCTION    it will destroy the medium, and realease all ram in the medium
                      it will flush before destroying it,the medium can't be used again.
 * PARAM:      T_PMEDIUM obj
 * RETURN:     NONE
**************************************************************************/

T_VOID Medium_Destroy(T_PMEDIUM obj);

#endif  //_MTD_LIB_H_
