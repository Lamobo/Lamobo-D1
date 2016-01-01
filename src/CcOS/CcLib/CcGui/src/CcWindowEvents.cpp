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
 * @file     CcWindowEvents.cpp
 * @brief    Class CcWindowEvents
 */

#include "CcWindowEvents.h"
#include "CcKernel.h"
#include "CcPos.h"

CcWindowEvents::CcWindowEvents()
{
  Kernel.registerEventReceiver(this, 0);
}

CcWindowEvents::~CcWindowEvents()
{
}

void CcWindowEvents::callback(uint8 nr, void *Param)
{
  switch(nr){
    case CB_EVENT:
    {
      CcInputEvent *event = (CcInputEvent*)(Param);
      switch (event->m_type)
      {
        case EVT_MOUSE:
        {
          sCcMouseEvent mouseEvent = event->getMouseEvent();
          findOnClick(mouseEvent.x, mouseEvent.y);
          break;
        }
        default:
          break;
      }
    }
  }
}

void CcWindowEvents::registerOnClick(sClickWindow *window){
  clickList.insertAt(0, window);
}

void CcWindowEvents::deleteOnClick(sClickWindow *window){
  for (uint16 i = 0; i<clickList.size(); i++)
  {
    if (clickList.at(i) == window){
      clickList.deleteAt(i);
      return;
    }
  }
}

bool CcWindowEvents::findOnClick(uint16 posX, uint16 posY){
  for (uint16 i = 0; i<clickList.size(); i++)
  {
    if (clickList[i]->posXStart <= posX &&
      clickList[i]->posYStart <= posY &&
      clickList[i]->posXEnd >= posX &&
      clickList[i]->posYEnd >= posY)
    {
      CcPos pos;
      pos.setPos(posX, posY);
      clickList[i]->OH.call(&pos);
      return true;
    }
  }
  return false;
}
