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
 */

#include "WindowsWindow.h"
#include "WindowsSystem.h"
#include "WindowsDisplay.h"
#include "CcInputEvent.h"
#include "CcKernel.h"
#include "WindowsGlobals.h"
#include "windowsx.h"
#include "Wingdi.h"

WindowsWindow::WindowsWindow(uint16 nr, uint16 width, uint16 height)
{
  m_WindowId.append("MainWClass-");
  m_WindowId.appendNumber(nr);
  setSize(width, height);
  InitializeCriticalSection(&m_CS);
}

WindowsWindow::~WindowsWindow() {
  DeleteCriticalSection(&m_CS);
  delete m_Bitmap.bitmap;
}

void WindowsWindow::init(void){
  WNDCLASSEX wcx;
  HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);
  // Fill in the window class structure with parameters 
  // that describe the main window. 
  wcx.cbSize = sizeof(wcx);          // size of structure 
  wcx.style = CS_HREDRAW | CS_VREDRAW; // redraw if size changes 
  wcx.lpfnWndProc = g_WindowsDisplay.mainWndProc;//MainWndProc;     // points to window procedure 
  wcx.cbClsExtra = 0;                // no extra class memory 
  wcx.cbWndExtra = 0;                // no extra window memory 
  wcx.hInstance = hinst;         // handle to instance 
  wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION); // predefined app. icon 
  wcx.hCursor = LoadCursor(NULL, IDC_ARROW); // predefined arrow 
  wcx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // white background brush 
  wcx.lpszMenuName = NULL;    // name of menu resource 
  wcx.lpszClassName = m_WindowId.getLPCSTR();  // name of window class 
  wcx.hIconSm = (HICON)LoadImage(hinst, // small class icon 
    MAKEINTRESOURCE(5),
    IMAGE_ICON,
    GetSystemMetrics(SM_CXSMICON),
    GetSystemMetrics(SM_CYSMICON),
    LR_DEFAULTCOLOR);
  RegisterClassEx(&wcx);

  // Register the window class. 

  // Create the main window. 
  m_hWnd = CreateWindow(
    m_WindowId.getLPCSTR(),     // name of window class 
    "Cc :)",          // title-bar string 
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,         // top-level window 
    getPosX(),                // default horizontal position 
    getPosY(),                // default vertical position 
    getSizeX(),       // default width 
    getSizeY(),       // default height 
    (HWND)NULL,       // no owner window 
    (HMENU)NULL,      // use class menu 
    hinst,          // handle to application instance 
    (LPVOID)NULL);    // no window-creation data
  DWORD dwTest;
  if (m_hWnd != 0){
    m_Bitmap.width  = m_sizeX;
    m_Bitmap.height = m_sizeY;
    m_Bitmap.pixCount = m_Bitmap.width * m_Bitmap.height;
    m_Bitmap.bitmap = new bitmapRGB[m_Bitmap.pixCount];
    memset(m_Bitmap.bitmap, 0x00, sizeof(bitmapRGB)*m_Bitmap.pixCount);
    WindowsDisplay::registerWindow(m_hWnd, this);
    memset(&m_bmi, 0, sizeof(m_bmi));
    m_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_bmi.bmiHeader.biWidth = m_Bitmap.width;
    m_bmi.bmiHeader.biHeight = m_Bitmap.height;
    m_bmi.bmiHeader.biPlanes = 1;
    m_bmi.bmiHeader.biBitCount = 24;
    m_bmi.bmiHeader.biCompression = BI_RGB;
    // Show the window and send a WM_PAINT message to the window 
    // procedure. 
    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);
  }
  else
    dwTest = GetLastError();
}

void WindowsWindow::drawPixel(uchar R, uchar G, uchar B){
  uint32 uiTemp = (m_Bitmap.height - (m_CursorY + m_DrawYStart)) * getSizeX();
  uiTemp += m_CursorX + m_DrawXStart;
  m_Bitmap.bitmap[uiTemp].R = R;
  m_Bitmap.bitmap[uiTemp].G = G;
  m_Bitmap.bitmap[uiTemp].B = B;

  if (m_CursorX < m_DrawXSize - 1){
    m_CursorX++;
  }
  else{
    m_CursorX = 0;
    if (m_CursorY < m_DrawYSize - 1){
      m_CursorY++;
    }
    else{
      m_CursorY = 0;
    }
  }
  RedrawWindow(m_hWnd, NULL, NULL, RDW_INVALIDATE);
}

void WindowsWindow::setPixelWindow(uint16 x, uint16 y, uint16 xSize, uint16 ySize){
  if(x +xSize > getSizeX() ||
     y +ySize > getSizeY()){
    m_DrawXSize = 0;
  }
  m_DrawXStart = x;
  m_DrawYStart = y;
  m_DrawXSize = xSize;
  m_DrawYSize = ySize;
  m_CursorX = 0;
  m_CursorY = 0;
}

void WindowsWindow::drawBitmap(HWND hWnd, bitmapAll *pBitmap){
  PAINTSTRUCT 	ps;
  uint8        *data;

  HDC hdc = BeginPaint(hWnd, &ps);
  HDC hMemDC = CreateCompatibleDC(hdc);
  HBITMAP hBitmap = CreateDIBSection(hdc, &m_bmi, DIB_RGB_COLORS, (void **)&data, 0, 0);
  HGDIOBJ hOldBmp = SelectObject(hMemDC, hBitmap);

  memcpy(data, pBitmap->bitmap, sizeof(bitmapRGB)*(pBitmap->pixCount));

  SelectObject(hMemDC, hBitmap);
  BitBlt(hdc, 0, 0, pBitmap->width, pBitmap->height, hMemDC, 0, 0, SRCCOPY);

  SelectObject(hMemDC, hOldBmp);
  DeleteObject(hOldBmp);
  DeleteObject(hBitmap);
  DeleteDC(hMemDC);

  EndPaint(hWnd, &ps);
}

LRESULT WindowsWindow::executeMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
  EnterCriticalSection(&m_CS);
  LRESULT lRet(0);

  switch (message)
  {
  case WM_LBUTTONDOWN:
  {
    CcInputEvent Event;
    Event.setMouseEvent(EVT_MOUSE, EVC_MOUSE_L_DOWN, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    m_Events->callback(0, &Event);
    break;
  }
  case WM_PRINTCLIENT:
  case WM_DISPLAYCHANGE:
  case WM_SETREDRAW:
  case WM_PAINT:
    drawBitmap(m_hWnd, &m_Bitmap);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    DefWindowProc(hWnd, message, wParam, lParam);
    break;
  /*case WM_NCACTIVATE:
  case WM_NCPAINT:
    {
      HDC hdc;
      RECT rect;
      HBRUSH b;
      HPEN pe;

      hdc = GetDCEx(m_hWnd, (HRGN)wParam, DCX_WINDOW | DCX_CACHE | DCX_INTERSECTRGN | DCX_LOCKWINDOWUPDATE);
      GetWindowRect(m_hWnd, &rect);
      b = CreateSolidBrush(RGB(0, 180, 180));
      SelectObject(hdc, b);
      pe = CreatePen(PS_SOLID, 1, RGB(90, 90, 90));
      SelectObject(hdc, pe);
      Rectangle(hdc, 0, 0, (rect.right - rect.left), (rect.bottom - rect.top));
      DeleteObject(pe);
      DeleteObject(b);

      ReleaseDC(m_hWnd, hdc);
      RedrawWindow(m_hWnd, &rect, (HRGN)wParam, RDW_UPDATENOW);
      return 0;
    }*/
  default:
    lRet = DefWindowProc(hWnd, message, wParam, lParam);
  }
  LeaveCriticalSection(&m_CS);
  return lRet;
}

void WindowsWindow::onPositionChanged( void ){
  SetWindowPos(m_hWnd, 0, getPosX(), getPosY(), getSizeX(), getSizeY(), 0);
}

void WindowsWindow::onSizeChanged(void){
  SetWindowPos(m_hWnd, 0, getPosX(), getPosY(), getSizeX(), getSizeY(), 0);
}

void WindowsWindow::onColorChanged(void){

}
