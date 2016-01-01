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
 * @file     CcWindowEvents.h
 * @brief    Class CcWindowEvents
 */
#ifndef CcWindowEvents_H_
#define CcWindowEvents_H_

#include "CcBase.h"
#include "CcObject.h"
#include "CcObjectHandler.h"
#include "CcVector.h"

#define CB_EVENT      0

/**
* @brief Register a Callback, if click recieved within this defined window.
*/
typedef struct {
  uint16 posXStart;
  uint16 posYStart;
  uint16 posXEnd;
  uint16 posYEnd;
  CcObjectHandler OH;
} sClickWindow;


class CcWindowEvents : public CcObject {
public:
  CcWindowEvents( void );
  virtual ~CcWindowEvents( void );
  void callback(uint8 nr, void *Param = 0);



  void registerOnClick(sClickWindow *window);
  void deleteOnClick(sClickWindow *window);

private:
  bool findOnClick(uint16 posX, uint16 posY);

  CcVector<sClickWindow*> clickList;
};

#endif /* CcWindowEvents_H_ */
