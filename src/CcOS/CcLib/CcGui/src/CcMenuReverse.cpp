/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     CcMenuReverse.cpp
* @brief    Class CcMenuReverse
*/
#include "CcMenuReverse.h"

CcMenuReverse::CcMenuReverse(void) :
  m_Pos(0)
{
}

CcMenuReverse::~CcMenuReverse(void)
{
}

void CcMenuReverse::nextPos(void)
{
  m_Pos++;
}

CcMenuItem* CcMenuReverse::getPos(void)
{
  if (m_Pos >= size())
    return 0;
  else
    return at(m_Pos);
}

void CcMenuReverse::resetPos(void)
{
  m_Pos = 0;
}