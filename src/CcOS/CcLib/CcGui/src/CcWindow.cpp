/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcWindow.cpp
 * @brief    Class CcWindow
 */

#include "CcWindow.h"
#include "CcKernel.h"
#include "CcPainter.h"
#include "CcInputEvent.h"

CcWindow::CcWindow(uint16 sizeX, uint16 sizeY, CcDisplay *display) :
  m_Display(display),
  m_uiBorderSize(0),
  m_sizeX(sizeX),
  m_sizeY(sizeY)
{
  if (m_Display == 0){
    m_Display = (CcDisplay*)Kernel.getDevice(eDisplay);
    m_Events = new CcWindowEvents();
  }
  else{
    setDisplay(display);
  }
  m_posX = 0;
  m_posY = 0;
  setBackgroundColor(0x00, 0x00, 0x00);
}

CcWindow::CcWindow(CcDisplay *display) :
  m_Display(display),
  m_uiBorderSize(0)
{
  if (m_Display == 0){
    m_Display = (CcDisplay*)Kernel.getDevice(eDisplay);
    if (m_Display != 0)
    {
      m_sizeX = m_Display->getSizeX();
      m_sizeY = m_Display->getSizeY();
    }
  }
  else{
    m_sizeX = m_Display->getSizeX();
    m_sizeY = m_Display->getSizeY();
  }
  m_Events = new CcWindowEvents();
  m_posX = 0;
  m_posY = 0;
  setBackgroundColor(0x00, 0x00, 0x00);
}

CcWindow::~CcWindow() {

}

void CcWindow::init(){

}

void CcWindow::setDisplay(CcDisplay *Display){
  m_Display = Display;
  setSize(Display->getSizeX(), Display->getSizeY());
}

void CcWindow::setBackgroundColor(uchar R, uchar G, uchar B){
  if(R == 0)
    m_cBackgroundR = 0x08;
  else
    m_cBackgroundR = R;
  if(G == 0)
    m_cBackgroundG = 0x08;
  else
    m_cBackgroundG = G;
  if(B == 0)
    m_cBackgroundB = 0x08;
  else
    m_cBackgroundB = B;
  onColorChanged();
}

void CcWindow::setBorder(uchar R, uchar G, uchar B, uint8 uiSize)
{
  if(m_cBorderR == 0) m_cBorderR = 0x08;
  else m_cBorderR = R;
  if(m_cBorderG == 0) m_cBorderG = 0x08;
  else m_cBorderG = G;
  if(m_cBorderB == 0) m_cBorderB = 0x08;
  else m_cBorderB = B;
  m_uiBorderSize = uiSize;
  onColorChanged();
}

uint16 CcWindow::getPosX( void ){
  return m_posX;
}

uint16 CcWindow::getPosY(void){
  return m_posY;
}

void CcWindow::setPos(uint16 x, uint16 y){
  m_posX = x;
  m_posY = y;
  onPositionChanged();
}

void CcWindow::setSize(uint16 x, uint16 y){
  m_sizeX = x;
  m_sizeY = y;
  onSizeChanged();
}

void CcWindow::setPixelWindow(uint16 xWindowPos, uint16 yWindowPos, uint16 xSize, uint16 ySize){
  uint16 xVal = xWindowPos + m_posX;
  uint16 yVal = yWindowPos + m_posY;
  m_Display->setAddress(xVal, yVal, xSize, ySize);
}

CcWindowEvents* CcWindow::getEventHandler(void){
  return m_Events;
}

void CcWindow::drawBorder(void ){
  uint16 uiTemp1;
  uint16 uiTemp2;
  uint16 uiTemp3;
  if(m_uiBorderSize > 0){
    CcPainter Painter(this);
    Painter.setColor(m_cBorderR, m_cBorderG, m_cBorderB);
    //write top border
    Painter.drawLine(0,0,m_sizeX-1,0);
    //write bottom border
    uiTemp1=m_sizeY-m_uiBorderSize;
    Painter.drawLine(0,uiTemp1,m_sizeX-1,uiTemp1);
    //write left border
    uiTemp1=m_sizeY-(m_uiBorderSize*2);
    Painter.drawLine(0,m_uiBorderSize,0,uiTemp1);
    //write right border
    uiTemp1=m_sizeX-1;
    uiTemp2=m_uiBorderSize;
    uiTemp3=m_sizeY-(m_uiBorderSize*2);
    Painter.drawLine(uiTemp1,uiTemp2,uiTemp1,uiTemp3);
  }
}

void CcWindow::draw( void )
{
  drawBackground();
}

void CcWindow::drawBackground( void ){
  uint16 xTemp = m_sizeX-(2*m_uiBorderSize);
  uint16 yTemp = m_sizeY-(2*m_uiBorderSize);
  uint32 uiTemp = xTemp*yTemp;
  setPixelWindow(m_uiBorderSize, m_uiBorderSize, xTemp, yTemp);
  for(uint32 i=0; i< uiTemp; i++)
    drawPixel(m_cBackgroundR, m_cBackgroundG, m_cBackgroundB);
}

void CcWindow::drawPixel(uchar R, uchar G, uchar B){
  m_Display->drawPixel(R,G,B);
}

CcDisplay *CcWindow::getDisplay(void){ 
  return m_Display;
}
