/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcButton.h
 * @brief    Class CcButton
 */
#ifndef CCBUTTON_H_
#define CCBUTTON_H_

#include "CcBase.h"
#include "dev/CcTouch.h"
#include "CcWidget.h"
#include "CcPos.h"

/**
 * @brief Button for GUI Applications
 */
class CcButton : public CcObject, public CcWidget{
public:
  /**
   * @brief Constructor
   */
  CcButton(CcWidget* parent);

  /**
   * @brief Destructor
   */
  virtual ~CcButton();

  void registerOnClick(CcObject *object, uint8 nr);
  void deleteOnClick(CcObject *object, uint8 nr);
  virtual void onClick(CcPos *pos);
  virtual void onRelease(CcPos *pos);

protected:
  void onPositionChanged( void );
  void onSizeChanged( void );

  CcTouch *m_Touch;
  sClickWindow m_ClickWindow;
private:
  void callback(uint8 nr, void *Param = 0);
  bool m_Clicked;
};

#endif /* CCBUTTON_H_ */
