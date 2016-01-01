/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-09
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcPainter.h
 * @brief    Implementation of class CcPainter
 */
#include "CcPainter.h"

CcPainter::CcPainter( CcWindow *Window ):
  m_Window(Window)
{
  setColor(0,0,0);
}

CcPainter::~CcPainter() {
  // TODO Auto-generated destructor stub
}

void CcPainter::setColor(uint8 R, uint8 G, uint8 B){
  m_Color.setColor(R, G, B);
}

void CcPainter::setColor(CcColor Color){
  m_Color = Color;
}

void CcPainter::drawLine(uint16 xStart, uint16 yStart, uint16 xStop, uint16 yStop){
  int16 ySize, yTemp;
  int16 xSize, xTemp;
  int16 PixPerStep;
  if(xStart <= xStop){
    if(yStart <= yStop){
      ySize = 1+ yStop - yStart;
      xSize = xStop - xStart;
    }
    else{
      ySize = yStop - yStart;
      xSize = xStop - xStart;
    }
  }
  else{
    if(yStart <= yStop){
      ySize = yStop - yStart;
      xSize = xStart - xStop;
    }
    else{
      ySize = yStop - yStart;
      xSize = xStart - xStop;
    }
  }
  PixPerStep = (xSize/ySize);
  for(uint16 i = 0; i< ySize; i++){
    yTemp = yStart + i;
    xTemp = xStart + i*PixPerStep;
    m_Window->setPixelWindow(xTemp, yTemp, PixPerStep+1, 1);
    for(uint16 k=0; k<PixPerStep+1; k++)
    {
      m_Window->drawPixel(m_Color.getR(),
                          m_Color.getG(),
                          m_Color.getB());
    }
  }
}

void CcPainter::drawCross(CcPos &position, CcCross &cross)
{
  uint16 startX = position.getX() + ((cross.m_width  - (cross.m_thick/2))/2);
  uint16 startY = position.getY() + ((cross.m_height - (cross.m_thick/2))/2);
  uint16 y = position.getY();
  uint16 x = position.getX();
  for(uint16 i=0; i < cross.m_thick; i++)
  {
    drawLine(x, startY+i, x+cross.m_width, startY+i);
    drawLine(startX+i, y, startX+i, y+cross.m_width);
  }
}
