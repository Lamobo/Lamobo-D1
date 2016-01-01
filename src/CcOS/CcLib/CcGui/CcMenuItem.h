/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
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
