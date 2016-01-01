/**
 * @copyright  Andreas Dirmeier (C) 2015
 *
 * This file is part of CcOS.
 *
 * CcOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CcOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CcOS.  If not, see <http://www.gnu.org/licenses/>.
 **/
/**
 * @author     Andreas Dirmeier
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 * @file       CcBase.h
 * @brief      To keep code working on different compiler and platforms,
 *             sometimes it is needed to define some datatypes like uchar.
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
