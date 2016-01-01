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
* @date       2015-10
* @par        Language   C++ ANSI V3
* @file     CcStringWin.h
* @brief    Class CcStringWin for handling Strings required for WinAPI
*/
#pragma once

#include "CcString.h"
#include "WindowsGlobals.h"
//#include <atlstr.h>
#include <string>

class CcStringWin : public CcString
{
public:
  CcStringWin();
  CcStringWin(LPWSTR wstr);
  CcStringWin(CcString parent);
  CcStringWin(const char* str) : CcString(str){}
  virtual ~CcStringWin();

  const wchar_t* getCComBSTR( void );
  LPCSTR         getLPCSTR(void);
  LPSTR          getLPSTR(void);
  LPCWSTR        getLPCWSTR ( void );
  std::wstring   getWString(void);

  void append(LPCSTR   str);
  void append(LPCWSTR  str);
  void append(std::wstring str);
};

