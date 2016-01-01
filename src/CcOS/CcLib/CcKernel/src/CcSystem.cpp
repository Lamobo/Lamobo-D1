/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcSystem.cpp
 * @brief    Class CcSystem
 */

#include "CcSystem.h"

CcSystem::CcSystem() :
  m_SystemState(0)
{
  // TODO Auto-generated constructor stub

}

CcSystem::~CcSystem() {
  // TODO Auto-generated destructor stub
}

void CcSystem::stop(void)
{
  m_SystemState = false;
}