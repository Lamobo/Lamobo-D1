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
