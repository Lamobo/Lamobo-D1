/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcTaskbar.cpp
 * @brief    Implementation of Class CcTaskbar
 */
#include "CcTaskbar.h"
#include "CcText.h"
#include "CcPushButton.h"

CcTaskbar::CcTaskbar(CcWidget *parent) :
  CcWidget(parent),
  m_DummyCenter(0),
  m_Menu(0)
{
  if (parent != 0){
    setSize(parent->getSizeX(), 20);
    setBackgroundColor(0x2d, 0x2d, 0x30);
    setPos(0, 0);
  }
  m_Center=0;
}

CcTaskbar::~CcTaskbar() {
  hideDummy();
}

CcMenu* CcTaskbar::createMenu( void ){
  m_Menu = new CcMenu(getParent(), this);
  return m_Menu;
}

void CcTaskbar::drawMenu(void){
  for (uint16 i = 0; i < m_Menu->size(); i++)
  {
    CcTaskbarItem *temp = m_Menu->at(i);
    temp->createButton(100 * i, 0);
  }
}

void CcTaskbar::hideMenu(void){
  for (uint16 i = 0; i < m_Menu->size(); i++)
  {
    m_Menu->at(i)->hideMenuTree();
  }
  m_Center->draw();
}

void CcTaskbar::drawDummy(void){
  delete m_DummyCenter;
  m_DummyCenter = 0;
  m_DummyCenter = new CcButton(getParent());
  m_DummyCenter->setPos(m_Center->getPosX(), m_Center->getPosY());
  m_DummyCenter->setSize(m_Center->getSizeX(), m_Center->getSizeY());
  m_DummyCenter->registerOnClick(this, 0);
}

void CcTaskbar::hideDummy(void){
  delete m_DummyCenter;
  m_DummyCenter = 0;
}

void CcTaskbar::drawTray(void){

}

void CcTaskbar::setCenterWindow(CcWidget* center)
{
  m_Center = center;
}

void CcTaskbar::callback(uint8 nr, void *Param){
  CC_UNUSED(Param);
  switch (nr)
  {
  case 0://Taskbar Item clicked
    cb_DeleteDraws();
  }
}

void CcTaskbar::cb_DeleteDraws(void){
  if (m_Menu) m_Menu->getReverseList()->clear();
  hideDummy();
  hideMenu();
}
