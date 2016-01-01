/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcDisplay.cpp
 * @brief    Class CcDisplay
 */
#include "dev/CcDisplay.h"
#include "CcWindow.h"

CcDisplay::CcDisplay(uint16 width, uint16 height) :
  m_SizeX(width),
  m_SizeY(height),
  m_CursorX(0),
  m_CursorY(0),
  m_DrawStartX(0),
  m_DrawStartY(0),
  m_DrawSizeX(0),
  m_DrawSizeY(0)
{}

CcDisplay::~CcDisplay() {

  // Auto-generated destructor stub
}

void CcDisplay::fillDisplay( void )
{
  setAddress(0,0,240, 320);
  for(uint32 n=0; n<320*240; n++) {
    drawPixel(0,0,0);
  }
}

void CcDisplay::fillDisplayTest( void )
{
  uchar count=0;
  uint16 xpp = 6;
  uint16 ypp = 8;
  uint32 xXy = ypp*xpp;
  for(uint16 i=0 ; i<240; i+=xpp){
    for(uint16 j=0 ; j<320; j+=ypp){
      setAddress(i,j,xpp, ypp);
      count++;
      for(uint32 n=0; n < xXy; n++) {
        if(count%3==0)
          drawPixel(0xff,0,0);
        else if(count%3==1)
          drawPixel(0xff,0,0);
        else{
          drawPixel(0xff,0,0);
        }
      }
      if(count==2) count=0;
    }
  }
}

CcWindow *CcDisplay::createWindow(void){
  CcWindow *wRet = new CcWindow(this);
  return wRet;
}

uint16 CcDisplay::getSizeX( void ){
  return m_SizeX;
}
uint16 CcDisplay::getSizeY( void ){
  return m_SizeY;
}

void CcDisplay::setSizeX( uint16 x ){
  m_SizeX = x;
}
void CcDisplay::setSizeY( uint16 y ){
  m_SizeY = y;
}

void CcDisplay::resetAddress( void )
{
  setAddress( 0, 0, m_SizeX, m_SizeY);
}

void CcDisplay::nextCursor(void)
{
  if (m_CursorX < m_DrawSizeX - 1){
    m_CursorX++;
  }
  else{
    m_CursorX = m_DrawStartX;
    if (m_CursorY < m_DrawSizeY - 1){
      m_CursorY++;
    }
    else{
      m_CursorY = m_DrawStartY;
    }
  }
}
