/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcTouch.cpp
 * @brief    Class CcTouch
 */

#include "dev/CcTouch.h"
#include "CcKernel.h"
#include "CcInputEvent.h"

#include "CcBase.h"
CcTouch::CcTouch() :
  m_AbsoluteX(0),
  m_AbsoluteY(0),
  m_NextPoll(0)
{
  setPosition(0,0);
  m_CalibMatrix.A=1;
  m_CalibMatrix.B=0;
  m_CalibMatrix.C=0;
  m_CalibMatrix.D=1;
  m_CalibMatrix.E=0;
  m_CalibMatrix.F=0;
  m_CalibMatrix.Div=1;
}

CcTouch::~CcTouch()
{
  //No implementation yet, do it in TargetClass
}

void CcTouch::onInterrupt( void ){
  if( m_NextPoll < Kernel.getTime() &&
      getPressState())
  {
    m_NextPoll = Kernel.getTime() + 100;
    uint16 x,y;
    getTouchState(&x,&y);
    setPosition(x,y);
    startConversion();
    //Send Kernel Event
    CcInputEvent Event;
    Event.setMouseEvent(EVT_MOUSE, EVC_MOUSE_L_DOWN, m_X, m_Y );
    Kernel.emitEvent(&Event);
  }
}

void CcTouch::startConversion(void ){
  m_X = (uint16)( ( (m_CalibMatrix.A * m_AbsoluteX) +
                    (m_CalibMatrix.B * m_AbsoluteY) +
                     m_CalibMatrix.C ) / m_CalibMatrix.Div);
  m_Y = (uint16)( ( (m_CalibMatrix.D * m_AbsoluteX) +
                    (m_CalibMatrix.E * m_AbsoluteY) +
                     m_CalibMatrix.F ) / m_CalibMatrix.Div);
}

void CcTouch::startPolling( void ){
}

void CcTouch::setPosition(uint16 x, uint16 y){
  m_AbsoluteX = x;
  m_AbsoluteY = y;
}

bool CcTouch::setCalibration(sCcTouchMatrix Matrix){
  bool bRet = false;
  if(Matrix.Div!=0)
  {
    m_CalibMatrix = Matrix;
    bRet=true;
  }
  return bRet;
}
