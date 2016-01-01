/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
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
