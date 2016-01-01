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
