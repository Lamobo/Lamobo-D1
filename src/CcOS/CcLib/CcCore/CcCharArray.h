/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
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
