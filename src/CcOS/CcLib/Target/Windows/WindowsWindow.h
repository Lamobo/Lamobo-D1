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
 * @file     WindowsWindow.cpp
 * @brief    Class WindowsWindow
 **/
#ifndef WINDOWSWINDOW_H_
#define WINDOWSWINDOW_H_

#include "CcBase.h"
#include "CcPainter.h"
#include "dev/CcTimer.h"
#include "WindowsTouch.h"
#include "dev/CcDisplay.h"
#include "CcStringWin.h"
#include "CcColor.h"
#include "WindowsGlobals.h"

#define CC_WINDOWSWINDOW_DEFAULT_X 500
#define CC_WINDOWSWINDOW_DEFAULT_Y 500

typedef struct{
  uint8 B;
  uint8 G;
  uint8 R;
} bitmapRGB;

typedef  struct{
  uint16 width;
  uint16 height;
  uint32 pixCount;
  bitmapRGB *bitmap;
} bitmapAll;

class WindowsWindow : public CcWindow
{
public:
  WindowsWindow(uint16 nr, uint16 width = CC_WINDOWSWINDOW_DEFAULT_X, uint16 height = CC_WINDOWSWINDOW_DEFAULT_Y);
  virtual ~WindowsWindow();

  void init( void );
  void drawPixel(uchar R, uchar G, uchar B);
  void setPixelWindow(uint16 x, uint16 y, uint16 xSize, uint16 ySize);

  void onColorChanged( void );
  void onPositionChanged(void);
  void onSizeChanged( void );

  LRESULT executeMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private: //methods
  void drawBitmap(HWND hWnd, bitmapAll *pBitmap);

private: //member
  HWND      m_hWnd;
  bitmapAll m_Bitmap;
  BITMAPINFO m_bmi;

  uint16 m_DrawXStart;
  uint16 m_DrawYStart;
  uint16 m_DrawXSize;
  uint16 m_DrawYSize;
  uint16 m_CursorX;
  uint16 m_CursorY;
  CcStringWin m_WindowTitle;
  CcStringWin m_WindowId;
  CRITICAL_SECTION m_CS;
};

#endif /* WINDOWSWINDOW_H_ */
