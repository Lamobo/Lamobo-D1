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
 * @file     WindowsDisplay.cpp
 * @brief    Class WindowsDisplay
 **/
#ifndef WindowsDisplay_H_
#define WindowsDisplay_H_

#include "CcBase.h"
#include "CcVector.h"
#include "CcWindow.h"
#include "WindowsWindow.h"
#include "WindowsGlobals.h"


typedef struct{
  HWND hWindow;
  WindowsWindow* Window;
} sWindowHandle;

class WindowsDisplay : public CcDisplay
{
public:
  WindowsDisplay(void);
  virtual ~WindowsDisplay(){}
  static LRESULT CALLBACK mainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
  static void registerWindow(HWND hWnd, WindowsWindow* Window);
  static void deleteWindow(HWND hWnd, WindowsWindow* Window);
  static WindowsWindow* getWindow(HWND hWnd);
  bool open(uint16 flags = 0);
  bool close(void);
  void setBackgroundLed(bool bState);
  void setCursor(uint16 x, uint16 y);
  void setAddress(uint16 x, uint16 y, uint16 xSize, uint16 ySize);
  void drawPixel(uchar R, uchar G, uchar B);
  CcWindow* createWindow(void);

private: //member
  static CcVector<sWindowHandle> m_WidgetList;
  uint16 m_WindowCnt;
};

static WindowsDisplay g_WindowsDisplay;

#endif /* WindowsDisplay_H_ */
