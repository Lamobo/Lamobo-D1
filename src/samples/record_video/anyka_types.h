/** @file
* @brief Define the global public types for anyka
*
* Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
* @author
* @date 2006-01-16
* @version 1.0
*/

#ifndef __ANYKA_TYPES_H__
#define __ANYKA_TYPES_H__

/** @defgroup GLOBALTYPES global types
*    @ingroup GLOBAL
*/
/*@{*/

/* preliminary type definition for global area */
typedef    unsigned char           T_U8;        /* unsigned 8 bit integer */
typedef    char                    T_CHR;        /* char */
typedef    unsigned short          T_U16;        /* unsigned 16 bit integer */
typedef    unsigned long           T_U32;        /* unsigned 32 bit integer */
#ifdef _MSC_VER
typedef    unsigned __int64        T_U64;        /* unsigned 64 bit integer */
#else
typedef    unsigned long long      T_U64;        /* unsigned 64 bit integer */
#endif
typedef    signed char             T_S8;        /* signed 8 bit integer */
typedef    signed short            T_S16;        /* signed 16 bit integer */
typedef    signed long             T_S32;        /* signed 32 bit integer */
#ifdef _MSC_VER
typedef    __int64                 T_S64;        /* signed 64 bit integer */
#else
typedef    signed long long        T_S64;        /* signed 64 bit integer */
#endif
typedef    void                    T_VOID;        /* void */
typedef    volatile unsigned int   V_UINT32;

#define    T_U8_MAX             ((T_U8)0xff)                // maximum T_U8 value
#define    T_U16_MAX            ((T_U16)0xffff)             // maximum T_U16 value
#define    T_U32_MAX            ((T_U32)0xffffffff)         // maximum T_U32 value
#define    T_U64_MAX            ((T_U64)0xffffffffffffffff) // maximum T_U64 value
#define    T_S8_MIN             ((T_S8)(-127-1))            // minimum T_S8 value
#define    T_S8_MAX             ((T_S8)127)                 // maximum T_S8 value
#define    T_S16_MIN            ((T_S16)(-32767L-1L))       // minimum T_S16 value
#define    T_S16_MAX            ((T_S16)(32767L))           // maximum T_S16 value
#define    T_S32_MIN            ((T_S32)(-2147483647L-1L))  // minimum T_S32 value
#define    T_S32_MAX            ((T_S32)(2147483647L))      // maximum T_S32 value
#define    T_S64_MIN            ((T_S64)(-9223372036854775807LL-1LL))    // minimum T_S64 value
#define    T_S64_MAX            ((T_S64)(9223372036854775807LL))        // maximum T_S64 value

/* basal type definition for global area */
typedef T_U8                    T_BOOL;     /* BOOL type */

typedef T_VOID *                T_pVOID;    /* pointer of void data */
typedef const T_VOID *          T_pCVOID;   /* const pointer of void data */


typedef T_CHR *                 T_pSTR;     /* pointer of string */
typedef const T_CHR *           T_pCSTR;    /* const pointer of string */


typedef T_U16                   T_WCHR;     /**< unicode char */
typedef T_U16 *                 T_pWSTR;    /* pointer of unicode string */
typedef const T_U16 *           T_pCWSTR;   /* const pointer of unicode string */


typedef T_U8 *                  T_pDATA;    /* pointer of data */
typedef const T_U8 *            T_pCDATA;   /* const pointer of data */

typedef T_U32                   T_COLOR;    /* color value */

typedef T_U32                   T_HANDLE;   /* a handle */

typedef T_S32                   T_POS;      /* position type */
typedef T_S32                   T_LEN;      /* length type */

#define         AK_FALSE            0
#define         AK_TRUE             1
#define         AK_NULL             ((T_pVOID)(0))

#define        AK_EMPTY

/*@}*/


#endif
