/*
 * CcCalibrationButton.cpp
 *
 *  Created on: 13.09.2015
 *      Author: Andreas
 */

#include "CcCalibrationButton.h"
#include "CcPainter.h"
#include "CcString.h"
#include "CcKernel.h"

CcCalibrationButton::CcCalibrationButton(CcWidget *parent) :
  CcButton(parent),
  m_TextWidget(parent),
  m_cross(20, 20, 3)
{
  m_ClickWindow.posXStart = 0;
  m_ClickWindow.posYStart = 0;
  m_ClickWindow.posXEnd   = 0xffff;
  m_ClickWindow.posYEnd   = 0xffff;
  fillCalibData();
  m_buttonNr = 0;
  m_TextWidget.setFontColor(0x00, 0xff, 0x00);
  m_TextWidget.setPos(30, 10);
  m_TextWidget.setSize(200, 200);
  m_PosAbsolute.setPos(0,0);
  m_PosRelative.setPos(0,0);
  m_Done=false;
}

CcCalibrationButton::~CcCalibrationButton() {

}

void CcCalibrationButton::draw(){
  drawBackground();
  drawText();
  drawButton();
}
void CcCalibrationButton::drawButton(){
  CcPainter Painter(getWindow());
  Painter.setColor(0xff, 0, 0);
  Painter.drawCross(Pos1, m_cross);
}

void CcCalibrationButton::drawText(){
  CcString sDisplayText("Calibrate the Touchpanel\n");
  sDisplayText.append("Press on Position where crosses\n");
  sDisplayText.append("are getting Displayed\n\n");
  sDisplayText.append("Pos1: ( ");
  sDisplayText.appendNumber(m_calibData.touch.X1);
  sDisplayText.append(", ");
  sDisplayText.appendNumber(m_calibData.touch.Y1);
  sDisplayText.append(" )\n");
  sDisplayText.append("Pos2: ( ");
  sDisplayText.appendNumber(m_calibData.touch.X2);
  sDisplayText.append(", ");
  sDisplayText.appendNumber(m_calibData.touch.Y2);
  sDisplayText.append(" )\n");
  sDisplayText.append("Pos3: ( ");
  sDisplayText.appendNumber(m_calibData.touch.X3);
  sDisplayText.append(", ");
  sDisplayText.appendNumber(m_calibData.touch.Y3);
  sDisplayText.append(" )\n");
  if(m_Done){
    sDisplayText.append("\n\n\n");
    sDisplayText.append("Matrix->A:   ");
    sDisplayText.appendNumber(m_CalibMatrix.A);
    sDisplayText.append("\n");
    sDisplayText.append("Matrix->B:   ");
    sDisplayText.appendNumber(m_CalibMatrix.B);
    sDisplayText.append("\n");
    sDisplayText.append("Matrix->C:   ");
    sDisplayText.appendNumber(m_CalibMatrix.C);
    sDisplayText.append("\n");
    sDisplayText.append("Matrix->D:   ");
    sDisplayText.appendNumber(m_CalibMatrix.D);
    sDisplayText.append("\n");
    sDisplayText.append("Matrix->E:   ");
    sDisplayText.appendNumber(m_CalibMatrix.E);
    sDisplayText.append("\n");
    sDisplayText.append("Matrix->F:   ");
    sDisplayText.appendNumber(m_CalibMatrix.F);
    sDisplayText.append("\n");
    sDisplayText.append("Matrix->Div: ");
    sDisplayText.appendNumber(m_CalibMatrix.Div);
    if (m_PosAbsolute.getX() > 0 || m_PosAbsolute.getY() > 0)
    {
      sDisplayText.append("\n\n");
      sDisplayText.append("Absolute: ( ");
      sDisplayText.appendNumber(m_PosAbsolute.getX());
      sDisplayText.append(", ");
      sDisplayText.appendNumber(m_PosAbsolute.getY());
      sDisplayText.append(" )\n");
      sDisplayText.append("Relative: ( ");
      sDisplayText.appendNumber(m_PosRelative.getX());
      sDisplayText.append(", ");
      sDisplayText.appendNumber(m_PosRelative.getY());
      sDisplayText.append(" )\n");
    }
  }
  m_TextWidget.setString(sDisplayText);
  m_TextWidget.drawString();
}

void CcCalibrationButton::onClick(CcPos *pos){
  //simulate wrong values
  //pos->setPos(pos->getX() * 5, pos->getY() * 2);
  CcPainter Painter(getWindow());
  Painter.setColor(0xff, 0, 0);
  if(m_buttonNr == 0){
    m_calibData.touch.X1 = pos->getX();
    m_calibData.touch.Y1 = pos->getY();
    drawBackground();
    drawText();
    Painter.drawCross(Pos2, m_cross);
    m_buttonNr++;
  }
  else if(m_buttonNr == 1){
    m_calibData.touch.X2 = pos->getX();
    m_calibData.touch.Y2 = pos->getY();
    drawBackground();
    drawText();
    Painter.drawCross(Pos3, m_cross);
    m_buttonNr++;
  }
  else if(m_buttonNr == 2){
    m_buttonNr++;
    m_Done=true;
    m_calibData.touch.X3 = pos->getX();
    m_calibData.touch.Y3 = pos->getY();
    // All values connected start calculation
    calcCalibration();
    /*m_onDone.call();
    m_buttonNr++;*/
    drawBackground();
    drawText();

    m_ClickWindow.posXStart = getParent()->getPosX();
    m_ClickWindow.posYStart = getParent()->getPosY();
    m_ClickWindow.posXEnd = getParent()->getSizeX();
    m_ClickWindow.posYEnd = getParent()->getSizeY();
    m_onDone.call();
  }
  else{
    m_PosAbsolute.setPos(pos->getX(), pos->getY());
    CcPos sim = simulateCalibration(*pos);
    m_PosRelative.setPos(sim.getX(), sim.getY());
    drawBackground();
    drawText();
  }
}

void CcCalibrationButton::fillCalibData( void ){
  uint16 xSize = getWindow()->getSizeX();
  uint16 ySize = getWindow()->getSizeY();
  uint32 temp32;
  uint16 temp16X, temp16Y;
  memset(&m_calibData, 0, sizeof(m_calibData));
  //generate a Point up left
  Pos1.setPos( 30, 30);
  m_calibData.display.X1 = 30 + (m_cross.m_width  / 2) + getParent()->getPosX();
  m_calibData.display.Y1 = 30 + (m_cross.m_height / 2) + getParent()->getPosY();
  //generate a Point right middle
  temp32 = (xSize * 48);
  temp16X = (temp32 / 64) & 0xffff;
  temp16Y = (ySize / 2);
  Pos2.setPos(temp16X, temp16Y);
  m_calibData.display.X2 = temp16X + (m_cross.m_width  / 2) + getParent()->getPosX();
  m_calibData.display.Y2 = temp16Y + (m_cross.m_height / 2) + getParent()->getPosY();
  //generate a Point down middle
  temp16X = (xSize / 2);
  temp32 = (ySize * 48);
  temp16Y = (temp32 / 64) & 0xffff;
  Pos3.setPos(temp16X, temp16Y);
  m_calibData.display.X3 = temp16X + (m_cross.m_width  / 2) + getParent()->getPosX();
  m_calibData.display.Y3 = temp16Y + (m_cross.m_height / 2) + getParent()->getPosY();
}

void CcCalibrationButton::calcCalibration( void ){
  int32 temp1 = ((m_calibData.touch.X1 - m_calibData.touch.X3) * (m_calibData.touch.Y2 - m_calibData.touch.Y3));
  int32 temp2 = ((m_calibData.touch.X2 - m_calibData.touch.X3) * (m_calibData.touch.Y1 - m_calibData.touch.Y3));
  m_CalibMatrix.Div = temp1-temp2;

  if(m_CalibMatrix.Div != 0){
    m_CalibMatrix.A = ((m_calibData.display.X1 - m_calibData.display.X3) * (m_calibData.touch.Y2   - m_calibData.touch.Y3   )) -
                      ((m_calibData.display.X2 - m_calibData.display.X3) * (m_calibData.touch.Y1   - m_calibData.touch.Y3   ));

    m_CalibMatrix.B = ((m_calibData.touch.X1   - m_calibData.touch.X3  ) * (m_calibData.display.X2 - m_calibData.display.X3 )) -
                      ((m_calibData.display.X1 - m_calibData.display.X3) * (m_calibData.touch.X2   - m_calibData.touch.X3   ));

    m_CalibMatrix.C = (((m_calibData.touch.X3 * m_calibData.display.X2) - (m_calibData.touch.X2 * m_calibData.display.X3)) * m_calibData.touch.Y1) +
                      (((m_calibData.touch.X1 * m_calibData.display.X3) - (m_calibData.touch.X3 * m_calibData.display.X1)) * m_calibData.touch.Y2) +
                      (((m_calibData.touch.X2 * m_calibData.display.X1) - (m_calibData.touch.X1 * m_calibData.display.X2)) * m_calibData.touch.Y3) ;

    m_CalibMatrix.D = ((m_calibData.display.Y1 - m_calibData.display.Y3) * (m_calibData.touch.Y2   - m_calibData.touch.Y3)) -
                      ((m_calibData.display.Y2 - m_calibData.display.Y3) * (m_calibData.touch.Y1   - m_calibData.touch.Y3)) ;

    m_CalibMatrix.E = ((m_calibData.touch.X1   - m_calibData.touch.X3  ) * (m_calibData.display.Y2 - m_calibData.display.Y3)) -
                      ((m_calibData.display.Y1 - m_calibData.display.Y3) * (m_calibData.touch.X2   - m_calibData.touch.X3  )) ;

    m_CalibMatrix.F = (((m_calibData.touch.X3   * m_calibData.display.Y2) - (m_calibData.touch.X2 * m_calibData.display.Y3)) * m_calibData.touch.Y1) +
                      (((m_calibData.touch.X1   * m_calibData.display.Y3) - (m_calibData.touch.X3 * m_calibData.display.Y1)) * m_calibData.touch.Y2) +
                      (((m_calibData.touch.X2   * m_calibData.display.Y1) - (m_calibData.touch.X1 * m_calibData.display.Y2)) * m_calibData.touch.Y3) ;
  }
}

void CcCalibrationButton::registerOnDone(CcObject *object, uint8 nr){
  m_onDone.add(object, nr);
}

CcPos CcCalibrationButton::simulateCalibration(CcPos input){
  CcPos Ret;
  uint16 x, y;
  x = (uint16)(((m_CalibMatrix.A * input.getX()) +
                (m_CalibMatrix.B * input.getY()) +
                 m_CalibMatrix.C) / m_CalibMatrix.Div);
  y = (uint16)(((m_CalibMatrix.D * input.getX()) +
                (m_CalibMatrix.E * input.getY()) +
                 m_CalibMatrix.F) / m_CalibMatrix.Div);
  Ret.setPos(x, y);
  return Ret;
}
