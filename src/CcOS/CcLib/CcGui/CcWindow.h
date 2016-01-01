/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcWindow.h
 * @brief    Class CcWindow
 */
#ifndef CCWINDOW_H_
#define CCWINDOW_H_

#include "CcBase.h"
#include "dev/CcDisplay.h"
#include "CcObject.h"
#include "CcWindowEvents.h"

#define CB_EVENT      0

class CcWindow {
public:
  CcWindow(CcDisplay *display = 0);
  CcWindow(uint16 sizeX, uint16 sizeY, CcDisplay *display = 0);
  virtual ~CcWindow();

  virtual void init(void);
  uint16 getSizeX( void ) { return m_sizeX;}
  uint16 getSizeY( void ) { return m_sizeY;}
  uint16 getPosX(void);
  uint16 getPosY(void);
  void setPos(uint16 x, uint16 y);
  void setSize(uint16 x, uint16 y);
  void setBackgroundColor(uchar R, uchar G, uchar B);
  void setBorder(uchar R, uchar G, uchar B, uint8 uiSize);
  CcWindowEvents* getEventHandler(void);
  virtual void setPixelWindow(uint16 xWindowPos, uint16 yWindowPos, uint16 xSize, uint16 ySize);
  virtual void draw(void);
  virtual void drawBorder(void);
  virtual void drawBackground(void);
  virtual void drawPixel(uchar R, uchar G, uchar B);
  void setDisplay(CcDisplay* Display);
  CcDisplay *getDisplay(void);

protected:
  virtual void onPositionChanged( void ){}
  virtual void onBackgroundChanged(void){}
  virtual void onColorChanged(void){}
  virtual void onSizeChanged( void ){}

private:
  void initWindow( void );
  void receiveEvent(void);

protected:
  CcDisplay* m_Display;
  char m_uiBorderSize;
  uint16 m_sizeX;
  uint16 m_sizeY;
  uint16 m_posX;
  uint16 m_posY;
  char m_cBackgroundR;
  char m_cBackgroundG;
  char m_cBackgroundB;
  char m_cBorderR;
  char m_cBorderG;
  char m_cBorderB;
  CcWindowEvents *m_Events;
};

#endif /* CCWINDOW_H_ */
