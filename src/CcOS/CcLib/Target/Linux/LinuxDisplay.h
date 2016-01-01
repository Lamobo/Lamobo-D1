/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxDisplay.cpp
 * @brief    Class LinuxDisplay
 **/
#ifndef LinuxDisplay_H_
#define LinuxDisplay_H_

#include "CcBase.h"
#include "TargetConfig.h"

#if (CC_USE_GUI > 0)

#include "dev/CcTimer.h"
#include "dev/CcDisplay.h"
#include "CcColor.h"
#include <X11/Xlib.h>
#include "CcThread.h"

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

extern bitmapAll g_Bitmap;

class LinuxDisplay : public CcDisplay
{
public:
  LinuxDisplay(uint16 width, uint16 height);
  virtual ~LinuxDisplay();

  bool open( uint16 flags =0);
  bool close(void);

  void setCursor( uint16 x, uint16 y );
  void setAddress( uint16 x, uint16 y, uint16 xSize, uint16 ySize);
  void setBackgroundLed( bool bState );
  void drawPixel(uchar R, uchar G, uchar B);
private: //member
  bool    m_BackgroundLED;
  Display *m_Display;
  Window  m_Window;
  int     m_Screen;

  CcThread *m_EventThread;
};

#endif /* LinuxDisplay_H_ */

#endif // use gui
