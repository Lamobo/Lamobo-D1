/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     CcTaskbarItem.cpp
* @brief    Implementation of Class CcTaskbarItem
*/
#include "CcTaskbarItem.h"
#include "CcTaskbar.h"

CcTaskbarItem::CcTaskbarItem(CcWidget *parent, CcString name) :
  CcMenuItem(parent, name),
  m_Taskbar(0)
{
}

CcTaskbarItem::~CcTaskbarItem() {

}

void CcTaskbarItem::setTaskbar(CcTaskbar *Taskbar){
  m_Taskbar = Taskbar;
}

void CcTaskbarItem::drawMenuTree(void){
  m_Taskbar->drawDummy();
  for (uint16 i = 0; i < size(); i++)
  {
    at(i)->createButton(getButton()->getPosX(),
                        getButton()->getSizeY()*(i+1));
  }
}

void CcTaskbarItem::hideMenuTree(void){
  m_Taskbar->drawDummy();
  for (uint16 i = 0; i < size(); i++)
  {
    at(i)->hideMenuTree();
  }
}
