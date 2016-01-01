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
