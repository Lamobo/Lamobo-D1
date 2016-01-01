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
  virtual bool open( uint16 flags ) = 0;
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
