/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
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