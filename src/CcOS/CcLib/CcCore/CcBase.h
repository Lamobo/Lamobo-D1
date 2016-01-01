/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcBase.h
 * @brief    To keep code working on different compiler and platforms,
 *            sometimes it is needed to define some datatypes like uchar.
 */
#ifndef CCBASE_H_
#define CCBASE_H_

#include "stddef.h" ///< Import of default types like size_t
#include "stdint.h"
#include "time.h"   ///< Import of types time_t and tm

#ifndef SIZE_MAX
# define SIZE_MAX ~0
#endif

typedef unsigned char       uchar;  ///< define global uchar for bit-save-types
typedef unsigned char       uint8;  ///< define global uint8 for bit-save-types
typedef unsigned short      uint16; ///< define global uint16 for bit-save-types
typedef unsigned long       uint32; ///< define global uint32 for bit-save-types
typedef unsigned long long  uint64; ///< define global uint64 for bit-save-types
typedef signed short        int16;  ///< define global int16 for bit-save-types
typedef signed long         int32;  ///< define global int32 for bit-save-types
typedef signed long long    int64;  ///< define global int64 for bit-save-types
typedef unsigned char       byte;   ///< define global byte for bit-save-types
typedef unsigned int        uint;

#define CC_UNUSED(unused) ((void)unused)
extern class CcKernel Kernel; ///< external definition for Global Kernel Class
#endif /* CCBASE_H_ */
