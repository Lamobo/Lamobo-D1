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
 * @file     CcInputEvent.cpp
 * @brief    Class CcInputEvent
 */
#include "CcInputEvent.h"

CcInputEvent::CcInputEvent() :
  m_type(0),
  m_code(0),
  m_value(0)
{
  //Nothing to do
}

CcInputEvent::~CcInputEvent() {
  //Nothing to do
}

void CcInputEvent::setMouseEvent(uint16 type, uint16 code, uint16 x, uint16 y){
  m_type = type;
  m_code = code;
  mouse_convert conv;
  conv.coord.x = x;
  conv.coord.y = y;
  m_value = conv.value;
}

sCcMouseEvent CcInputEvent::getMouseEvent(void)  {
  sCcMouseEvent ev;
  mouse_convert conv;
  conv.value = m_value;
  ev.x = conv.coord.x;
  ev.y = conv.coord.y;
  ev.type = m_type;
  ev.code = m_code;
  return ev;
}
