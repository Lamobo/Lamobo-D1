/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     CcStringWin.h
* @brief    Implementation of CcStringWin
*/
#include "CcStringWin.h"
#include <locale>
#include <algorithm>


CcStringWin::CcStringWin()
{
}

CcStringWin::CcStringWin(CcString parent) : 
  CcString(parent)
{
}

CcStringWin::CcStringWin(LPWSTR wstr) :
  CcString("")
{
  append(wstr);
}

CcStringWin::~CcStringWin()
{
}

const wchar_t* CcStringWin::getCComBSTR(void)
{
  return getWString().c_str();
}

LPCSTR   CcStringWin::getLPCSTR(void)
{
  return getStdString().c_str();
}

LPSTR   CcStringWin::getLPSTR(void)
{
  return (char*)getLPCSTR();
}

LPCWSTR  CcStringWin::getLPCWSTR(void)
{
  return getWString().c_str();
}

std::wstring CcStringWin::getWString(void)
{
  const std::ctype<wchar_t>& CType = std::use_facet<std::ctype<wchar_t> >(std::locale());
  std::vector<wchar_t> wideStringBuffer(getStdString().length());
  CType.widen(getStdString().data(), getStdString().data() + getStdString().length(), &wideStringBuffer[0]);
  return std::wstring(&wideStringBuffer[0], wideStringBuffer.size());
}

void CcStringWin::append(LPCSTR   str)
{
  m_String.append(str);
}

void CcStringWin::append(LPCWSTR  str)
{
  std::wstring wstr(str);
  append(wstr);
}

void CcStringWin::append(std::wstring str)
{
  std::string s((const char*)&str[0], sizeof(wchar_t)/sizeof(char)*str.size());
  m_String.append(s);
}
