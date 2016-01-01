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
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 * @file     CcTaskbar.h
 * @brief    Class CcTaskbar
 */
#ifndef CCTASKBAR_H_
#define CCTASKBAR_H_

#include "CcBase.h"
#include "CcWidget.h"
#include "CcMenu.h"

/**
 * @brief A Taskbar which can hold A Menu and Tray Icons
 */
class CcTaskbar : public CcObject ,public CcWidget {
public:
  /**
   * @brief Constructor
   */
  CcTaskbar(CcWidget*);

  /**
   * @brief Destructor
   */
  virtual ~CcTaskbar();

  /**
   * @brief Set Menue showed frist in Taskbar
   * @param menu: Menu with filled menu-tree
   */
  CcMenu * createMenu( void );

  /**
   * @brief Draw menue beginning left
   */
  void drawMenu(void);

  /**
  * @brief Remove Menu from display
  */
  void hideMenu(void);

  /**
   * @brief Draw a dummy window to center, to receive callback for closing menue
   */
  void drawDummy(void);

  /**
  * @brief Remove Dummy from display
  */
  void hideDummy(void);

  /**
  * @brief Draw tray icons on right side
  */
  void drawTray(void);

  /**
   * @brief set target window for applications
   * @param center: Pointer to window of target
   */
  void setCenterWindow(CcWidget* center);

  /**
   * @brief Implementation of callback routine to recieve events
   */
  void callback(uint8 nr, void *Param = 0);

  /**
   * @brief Delete all Draws outside of Taskbar.
   *      Function gets called from callback.
   */
  void cb_DeleteDraws(void);
private:
  CcButton      *m_DummyCenter;///< Dummycenter to overlap Target Window.
  CcMenu        *m_Menu;       ///< Menu tree to show at first.
  CcWidget      *m_Center;     ///< Target window for all elements.
};

#endif /* CCTASKBAR_H_ */
