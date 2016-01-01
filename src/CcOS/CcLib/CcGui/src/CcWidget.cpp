/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcWidget.cpp
 * @brief    Class CcWidget
 */

#include "CcWidget.h"
#include "CcKernel.h"
#include "CcPainter.h"
#include "CcInputEvent.h"

CcWidget::CcWidget( CcWidget *parent ):
  m_Parent(parent),
  m_uiBorderSize(0),
  m_BackgroundColor(0, 0, 0),
  m_BorderColor(0, 0, 0),
  m_posX(0),
  m_posY(0)
{
  setWindow(parent->getWindow());
  m_posX = parent->getPosX();
  m_posY = parent->getPosY();
  m_sizeX = parent->getSizeX();
  m_sizeY = parent->getSizeY();
}

CcWidget::CcWidget(CcDisplay *Display) :
  m_Parent(0),
  m_uiBorderSize(0),
  m_BackgroundColor(0, 0, 0),
  m_BorderColor(0, 0, 0),
  m_posX(0),
  m_posY(0)
{
  if (Display == 0)
  {
    Display = (CcDisplay*)Kernel.getDevice(eDisplay);
  }
  if (Display != 0)
  {
    m_Window = Display->createWindow();
    m_Window->init();
    m_sizeX = m_Window->getSizeX();
    m_sizeY = m_Window->getSizeY();
  }
}

CcWidget::~CcWidget() {

}

void CcWidget::setWindow(CcWindow *Window){
  m_Window = Window;
  setSize(Window->getSizeX(), Window->getSizeY());
}

void CcWidget::setBackgroundColor(uchar R, uchar G, uchar B){
  if (m_Parent == 0)
  {
    m_Window->setBackgroundColor(R, G, B);
  }
  else
  {
    m_BackgroundColor.setColor(R, G, B);
  }
  onBackgroundChanged();
}

CcColor* CcWidget::getBackgroundColor(void)
{
  return &m_BackgroundColor;
}

void CcWidget::setBorder(uchar R, uchar G, uchar B, uint8 uiSize)
{
  if (m_Parent == 0)
  { 
    m_Window->setBorder(R, G, B, uiSize);
  }
  else
  {
    m_BorderColor.setColor(R, G, B);
  }
  m_uiBorderSize = uiSize;
}

uint16 CcWidget::getPosX( void ){
  return m_posX;
}

uint16 CcWidget::getPosY(void){
  return m_posY;
}

void CcWidget::setPos(uint16 x, uint16 y){
  if (getParent() != 0){
    m_posX = getParent()->getPosX() + x;
    m_posY = getParent()->getPosY() + y;
  }
  else{
    m_Window->setPos(x, y);
  }
  onPositionChanged();
}

void CcWidget::setSize(uint16 x, uint16 y){
  m_sizeX = x;
  m_sizeY = y;
  onSizeChanged();
}

void CcWidget::setPixelWindow(uint16 xWindowPos, uint16 yWindowPos, uint16 xSize, uint16 ySize){
  uint16 xVal = xWindowPos + m_posX;
  uint16 yVal = yWindowPos + m_posY;
  m_Window->setPixelWindow(xVal, yVal, xSize, ySize);
}

CcWindowEvents* CcWidget::getEventHandler(void){
  if (getParent() == 0)
    return m_Window->getEventHandler();
  else
    return getParent()->getEventHandler();
}

void CcWidget::drawBorder(void ){
  uint16 uiTemp1;
  uint16 uiTemp2;
  uint16 uiTemp3;
  if(m_uiBorderSize > 0){
    CcPainter Painter(m_Window);
    Painter.setColor(m_BorderColor);
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

void CcWidget::draw( void )
{
  if (m_Parent == 0)
    m_Window->draw();
  drawBackground();
}

void CcWidget::drawBackground( void ){
  uint16 xTemp = m_sizeX-(2*m_uiBorderSize);
  uint16 yTemp = m_sizeY-(2*m_uiBorderSize);
  uint32 uiTemp = xTemp*yTemp;
  setPixelWindow(m_uiBorderSize, m_uiBorderSize, xTemp, yTemp);
  for (uint32 i = 0; i< uiTemp; i++)
    drawPixel(m_BackgroundColor.m_R, m_BackgroundColor.m_G, m_BackgroundColor.m_B);
}

void CcWidget::drawPixel(uchar R, uchar G, uchar B){
  m_Window->drawPixel(R,G,B);
}

CcWindow *CcWidget::getWindow(void){ 
  return m_Window;
}

CcWidget *CcWidget::getParent(void){
  return m_Parent;
}
