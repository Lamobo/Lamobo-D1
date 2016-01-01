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
 * @file     CcCharArray.cpp
 * @brief    Class CcCharArray
 **/
#ifndef CCCHARARRAY_H_
#define CCCHARARRAY_H_

#include "CcVector.h"
#include <cstring>

class CcString;

/**
 * @brief Handle list of Strings
 */
class CcCharArray : public CcVector<char>
{
public:
  CcCharArray(const char* toAppend = 0);
  virtual ~CcCharArray();

  void appendConst(const char* toAppend);
};

#endif /* CCCHARARRAY_H_ */
