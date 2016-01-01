/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcCharArray.cpp
 * @brief    Implemtation of class CcCharArray
 */

#include "CcCharArray.h"
#include <cstring>

CcCharArray::CcCharArray(const char* toAppend)
{
  if (toAppend != 0)
  {
    CcVector<char>::append((char*)toAppend, strlen(toAppend));
  }
}

CcCharArray::~CcCharArray()
{
}

void CcCharArray::appendConst(const char* toAppend){
  if (toAppend != 0)
  {
    CcVector<char>::append((char*)toAppend, strlen(toAppend));
  }
}