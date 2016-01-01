/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcDisplay.h
 * @brief    Class CcDisplay
 */
#ifndef CCDISPLAY_H_
#define CCDISPLAY_H_

#include "CcBase.h"
#include "CcIODevice.h"

class CcWindow;

class CcDisplay : public CcIODevice {
public:
  CcDisplay(uint16 width = 0, uint16 height = 0);
  virtual ~CcDisplay();

  virtual size_t read(char*, size_t size){
    return size;
  }
  virtual size_t write(char*, size_t size){
    return size;
  }
  virtual bool open( uint16 flags = 0) = 0;
  virtual void setBackgroundLed( bool bState ) = 0;
  virtual void setCursor( uint16 x, uint16 y ) = 0;
  virtual void setAddress( uint16 x, uint16 y, uint16 xSize, uint16 ySize) = 0;
  virtual void drawPixel(uchar R, uchar G, uchar B) = 0;

  virtual CcWindow* createWindow(void);

  uint16 getSizeX( void );
  uint16 getSizeY( void );
  void setSizeX( uint16 x );
  void setSizeY( uint16 y );
  void fillDisplay( void );
  void fillDisplayTest( void );
  void resetAddress( void );
  void nextCursor(void);

protected: //member
  uint16 m_SizeX;
  uint16 m_SizeY;
  uint16 m_CursorX;
  uint16 m_CursorY;
  uint16 m_DrawStartX;
  uint16 m_DrawStartY;
  uint16 m_DrawSizeX;
  uint16 m_DrawSizeY;
};

#endif /* CCDISPLAY_H_ */
