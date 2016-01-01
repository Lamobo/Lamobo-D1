/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcWidget.h
 * @brief    Class CcWidget
 */
#ifndef CcWidget_H_
#define CcWidget_H_

#include "CcBase.h"
#include "CcWindow.h"
#include "CcObject.h"
#include "CcInputEvent.h"
#include "CcWindowEvents.h"
#include "CcColor.h"

#define CB_EVENT      0

class CcWidget {
public:
  CcWidget( CcWidget  *parent );
  CcWidget( CcDisplay *Display = 0);
  virtual ~CcWidget();

  uint16 getSizeX( void ) { return m_sizeX;}
  uint16 getSizeY( void ) { return m_sizeY;}
  uint16 getPosX(void);
  uint16 getPosY(void);
  void setPos(uint16 x, uint16 y);
  void setSize(uint16 x, uint16 y);
  void setBackgroundColor(uchar R, uchar G, uchar B);
  CcColor* getBackgroundColor(void);
  void setWindow(CcWindow* Window);
  void setBorder(uchar R, uchar G, uchar B, uint8 uiSize);
  void setPixelWindow(uint16 xWindowPos, uint16 yWindowPos, uint16 xSize, uint16 ySize);
  CcWindowEvents* getEventHandler(void);
  virtual void draw(void);
  virtual void drawBorder(void);
  virtual void drawBackground(void);
  virtual void drawPixel(uchar R, uchar G, uchar B);
  CcWindow  *getWindow(void);
  CcWidget  *getParent( void );

protected:
  virtual void onPositionChanged( void ){}
  virtual void onBackgroundChanged ( void ){}
  virtual void onSizeChanged( void ){}

private:
  void initWindow( void );
  void receiveEvent(void);

protected:
  uint16 m_sizeX;
  uint16 m_sizeY;
  CcColor m_BorderColor;
private:
  CcWidget *m_Parent;
  CcWindow *m_Window;
  char m_uiBorderSize;
  CcColor m_BackgroundColor;
  uint16 m_posX;
  uint16 m_posY;
};

#endif /* CcWidget_H_ */
