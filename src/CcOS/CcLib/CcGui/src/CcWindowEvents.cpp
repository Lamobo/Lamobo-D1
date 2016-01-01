/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
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
