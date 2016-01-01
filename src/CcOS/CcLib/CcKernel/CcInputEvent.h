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
 * @file     CcInputEvent.h
 * @brief    Class CcInputEvent
 */
#ifndef CcInputEvent_H_
#define CcInputEvent_H_

#include "CcBase.h"
#include "CcString.h"
#include "CcInputEvent.defs.h"

typedef struct {
  uint16 type;
  uint16 code;
  uint16 x;
  uint16 y;
}sCcMouseEvent;

/**
 * @brief Default Class for Input Events like Keyboard, Mouse
 */
class CcInputEvent {
public:
  CcInputEvent();
  virtual ~CcInputEvent();

  void setMouseEvent(uint16 type, uint16 code, uint16 x, uint16 y);
  sCcMouseEvent getMouseEvent(void);

  typedef union{
    struct {
      uint16 x;
      uint16 y;
    } coord;
    uint32 value;
  } mouse_convert;

  uint16 m_type;
  uint16 m_code;
  uint32 m_value;
};

#endif /* CcInputEvent_H_ */
