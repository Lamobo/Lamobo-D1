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
 * @file     CcMenuItem.cpp
 * @brief    Implementation of Class CcMenuItem
 */
#include "CcMenuItem.h"

CcMenuItem::CcMenuItem(CcWidget *Parent, CcString name) :
  m_parentWidget(Parent),
  m_Button(0),
  m_Name(name),
  m_ReverseList(0)
{
}

CcMenuItem::~CcMenuItem() {
  hideMenuTree();
  clear();
}

CcMenuItem* CcMenuItem::createItem(CcString name){
  CcMenuItem *ret = new CcMenuItem( m_parentWidget, name);
  ret->setReverseList(m_ReverseList);
  addSubTree(ret);
  return ret;
}

void CcMenuItem::addItem(CcMenuItem *toAdd){
  addSubTree(toAdd);
}

void CcMenuItem::delItem(CcMenuItem *toDel){
  delSubTree(toDel);
}

void CcMenuItem::setValue(CcString &toSet){
  m_Name = toSet;
}

CcMenuItem* CcMenuItem::at(uint16 pos){
  return (CcMenuItem*)getAt(pos);
}

CcString* CcMenuItem::getValue(void){
  return &m_Name;
}

void CcMenuItem::setReverseList(CcMenuReverse *list){
  m_ReverseList = list;
}

CcMenuReverse* CcMenuItem::getReverseList(void){
  return m_ReverseList;
}

CcPushButton* CcMenuItem::createButton(uint16 startX, uint16 startY){
  m_Button = new CcPushButton(m_parentWidget);
  m_Button->setText(m_Name);
  m_Button->setPos(startX, startY);
  m_Button->setBorder(0x2d, 0x2d, 0x30, 1);
  m_Button->setBackgroundColor(0x33, 0x33, 0x33);
  m_Button->setBorder(0, 0, 0, 0);
  m_Button->setSize(100, 20);
  m_Button->draw();
  m_Button->registerOnClick(this, 0);
  return m_Button;
}

CcPushButton* CcMenuItem::getButton(void){
  return m_Button;
}

void CcMenuItem::drawMenuTree(void){
  for (uint16 i = 0; i < size(); i++)
  {
    at(i)->createButton(100 + m_Button->getPosX(), m_Button->getPosY() + m_Button->getSizeY()*i);
  }
}

void CcMenuItem::hideMenuTree(void){
  for (uint16 i = 0; i < size(); i++)
  {
    at(i)->hideMenuTree();
  }
  delete m_Button;
  m_Button = 0;
}

void CcMenuItem::callback(uint8 nr, void *Param){
  CC_UNUSED(Param);
  if (nr == 0){
    drawMenuTree();
  }
  else{
    // TODO: exec CcApp
  }
}
