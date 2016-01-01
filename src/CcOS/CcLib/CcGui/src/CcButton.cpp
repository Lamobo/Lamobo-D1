/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcButton.cpp
 * @brief    Class CcButton
 */
#include "CcButton.h"
#include "CcKernel.h"

#define CCBUTTON_CALLBACK_CLICK     1
#define CCBUTTON_CALLBACK_RELEASE   2

CcButton::CcButton(CcWidget *parent) :
  CcWidget(parent),
  m_Touch(0)
{
  m_ClickWindow.OH.add(this, CCBUTTON_CALLBACK_CLICK);
  m_Clicked = false;
  parent->getEventHandler()->registerOnClick(&m_ClickWindow);
}

CcButton::~CcButton() {
  getParent()->getEventHandler()->deleteOnClick(&m_ClickWindow);
}

void CcButton::registerOnClick(CcObject *object, uint8 nr){
  m_ClickWindow.OH.add(object, nr);
}

void CcButton::deleteOnClick(CcObject *object, uint8 nr){
  m_ClickWindow.OH.remove(object, nr);
}

void CcButton::onClick(CcPos *pos){
  CC_UNUSED(pos);
}

void CcButton::onRelease(CcPos *pos){
  CC_UNUSED(pos);
}

void CcButton::onPositionChanged( void ){
  m_ClickWindow.posXStart = getPosX();
  m_ClickWindow.posXEnd   = getPosX() + (m_sizeX - 1);
  m_ClickWindow.posYStart = getPosY();
  m_ClickWindow.posYEnd   = getPosY() + (m_sizeY - 1);
}

void CcButton::onSizeChanged( void ){
  m_ClickWindow.posXStart = getPosX();
  m_ClickWindow.posXEnd   = getPosX() + (m_sizeX - 1);
  m_ClickWindow.posYStart = getPosY();
  m_ClickWindow.posYEnd   = getPosY() + (m_sizeY - 1);
}

void CcButton::callback(uint8 nr, void *Param){
  switch(nr){
    case CCBUTTON_CALLBACK_CLICK:
      onClick((CcPos*)Param);
      break;
    case CCBUTTON_CALLBACK_RELEASE:
      onRelease((CcPos*)Param);
      break;
  }
}
