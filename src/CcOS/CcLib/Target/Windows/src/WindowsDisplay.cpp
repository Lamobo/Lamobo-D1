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
 */

#include "WindowsDisplay.h"
#include "WindowsSystem.h"
#include "WindowsGlobals.h"
#include "windowsx.h"
#include "CcKernel.h"

BITMAPINFO bmi;
CcVector<sWindowHandle> WindowsDisplay::m_WidgetList;

WindowsDisplay::WindowsDisplay(void) :
CcDisplay(),
m_WindowCnt(0)
{}

LRESULT CALLBACK WindowsDisplay::mainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
  LRESULT lRet = 0;
  WindowsWindow *wRet = getWindow(hWnd);
  if (message == WM_DESTROY){
    if (m_WidgetList.size() < 2)
      Kernel.stop();
  }
  if (wRet != 0)
  {
    lRet = wRet->executeMessage(hWnd, message, wParam, lParam);
  }
  else
    lRet = DefWindowProc(hWnd, message, wParam, lParam);
  return lRet;
}

void WindowsDisplay::registerWindow(HWND hWnd, WindowsWindow* Window){
  sWindowHandle sItem;
  sItem.hWindow = hWnd;
  sItem.Window = Window;
  m_WidgetList.append(sItem);
}

void WindowsDisplay::deleteWindow(HWND hWnd, WindowsWindow* Window){
  sWindowHandle sItem;
  sItem.hWindow = hWnd;
  sItem.Window = Window;
  for (uint16 i = 0; i < m_WidgetList.size(); i++)
  {
    if (m_WidgetList.at(i).hWindow == hWnd &&
      m_WidgetList.at(i).Window == Window)
      m_WidgetList.deleteAt(i);
  }
}

WindowsWindow* WindowsDisplay::getWindow(HWND hWnd){
  for (uint16 i = 0; i < m_WidgetList.size(); i++)
  {
    if (m_WidgetList.at(i).hWindow == hWnd)
      return m_WidgetList.at(i).Window;
  }
  return 0;
}
bool WindowsDisplay::open(uint16 flags){
  CC_UNUSED(flags);
  return false;
}

bool WindowsDisplay::close(void){
  return true;
}

void WindowsDisplay::setBackgroundLed(bool bState){
  CC_UNUSED(bState);
}
void WindowsDisplay::setCursor(uint16 x, uint16 y){
  CC_UNUSED(x);
  CC_UNUSED(y);
}
void WindowsDisplay::setAddress(uint16 x, uint16 y, uint16 xSize, uint16 ySize){
  CC_UNUSED(x);
  CC_UNUSED(y);
  CC_UNUSED(xSize);
  CC_UNUSED(ySize);
}
void WindowsDisplay::drawPixel(uchar R, uchar G, uchar B){
  CC_UNUSED(R);
  CC_UNUSED(G);
  CC_UNUSED(B);
}

CcWindow* WindowsDisplay::createWindow(void){
  CcWindow *wRet = new WindowsWindow(m_WindowCnt);
  m_WindowCnt++;
  return wRet;
}
