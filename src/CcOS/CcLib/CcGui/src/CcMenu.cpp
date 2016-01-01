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
 * @file     CcMenu.cpp
 * @brief    Implementation of Class CcMenu
 */
#include "CcMenu.h"
#include "CcMenuItem.h"

CcMenu::CcMenu(CcWidget *parentWidget, CcTaskbar *parentTaskbar) :
  m_parentWidget(parentWidget),
  m_Taskbar(parentTaskbar)
{

}

CcMenu::~CcMenu() {
}

void CcMenu::addItem(CcTaskbarItem *entry){
  m_MenuTree.append(entry);
}

bool CcMenu::delItem(CcTaskbarItem *toDel){
  m_MenuTree.deleteItem(toDel);
  return true;
}

CcMenuReverse *CcMenu::getReverseList(void)
{
  return &m_RevList;
}

CcTaskbarItem *CcMenu::createItem(CcString name){
  CcTaskbarItem *newItem = new CcTaskbarItem(m_parentWidget, name);
  newItem->setReverseList(&m_RevList);
  newItem->setTaskbar(m_Taskbar);
  m_MenuTree.append(newItem);
  return newItem;
}

size_t CcMenu::size(void){
  return m_MenuTree.size();
}

CcTaskbarItem* CcMenu::at(uint16 pos){
  return m_MenuTree.at(pos);
}