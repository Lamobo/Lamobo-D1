/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
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

