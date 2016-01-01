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
 * @file     CcMenuItem.h
 * @brief    Class CcMenuItem
 */
#ifndef CCMENUITEM_H_
#define CCMENUITEM_H_

#include "CcBase.h"
#include "CcWidget.h"
#include "CcString.h"
#include "CcTree.h"
#include "CcPushButton.h"
#include "CcMenuReverse.h"

/**
 * @brief Button for GUI Applications
 */
class CcMenuItem : public CcObject,  public CcTree {
public:
  /**
   * @brief Constructor
   */
  CcMenuItem(CcWidget *Parent, CcString name = "");

  /**
   * @brief Destructor
   */
  virtual ~CcMenuItem(void);

  /**
   * @brief Create a new menue item and add it to subtree.
   * @param name: Value of Item
   */
  CcMenuItem* createItem(CcString name = "");

  /**
   * @brief Add a Item to Menu-Tree
   * @param toAdd: Item to Add
   */
  void addItem(CcMenuItem *toAdd);

  /**
  * @brief Remove a item from Menu-Tree
  * @param toDel: Item to Delete
  */
  void delItem(CcMenuItem *toDel);

  /**
   * @brief Return Item on Position
   * @param pos: Requested Position
   * @return stored Item
   */
  CcMenuItem* at(uint16 pos);

  void setValue(CcString &toSet);
  CcString* getValue( void );

  void setReverseList(CcMenuReverse *list);
  CcMenuReverse* getReverseList(void);

  CcPushButton* createButton(uint16 startX, uint16 startY);
  CcPushButton* getButton(void);

  /**
   * @brief Draw Next Stage of Menu, and create Buttons for Input.
   */
  virtual void drawMenuTree( void );

  /**
  * @brief Hide all Displayed Elements of next Stages
  */
  virtual void hideMenuTree(void);

  void onClick(CcPos *pos);

  /**
   * @brief reimplementation of Objects Callback
   */
  void callback(uint8 nr, void *Param = 0);
private:
  CcWidget      *m_parentWidget;
  CcPushButton  *m_Button;
  CcString       m_Name;
  CcMenuReverse *m_ReverseList;
};

#endif /* CCMENUITEM_H_ */
