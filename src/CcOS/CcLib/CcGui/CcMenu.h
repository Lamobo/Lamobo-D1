/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcMenu.h
 * @brief    Class CcMenu
 */
#ifndef CCMENU_H_
#define CCMENU_H_

#include "CcBase.h"
#include "CcWidget.h"
#include "CcList.h"
#include "CcMenuItem.h"
#include "CcMenuReverse.h"
#include "CcTaskbarItem.h"

/**
 * @brief Menue-Tree start point with settings for display
 */
class CcMenu {
public:
  /**
   * @brief Constructor
   */
  CcMenu(CcWidget *parentWidget, CcTaskbar *parentTaskbar);

  /**
   * @brief Destructor
   */
  virtual ~CcMenu(void);

  /**
   * @brief Add a Top-Level entry to menu list
   * @param entry: top node of sub tree
   */
  void addItem(CcTaskbarItem *entry);
  bool delItem(CcTaskbarItem *toDel);
  CcMenuReverse *getReverseList(void);
  CcTaskbarItem *createItem(CcString name = "");
  CcTaskbarItem *at(uint16 pos);

  size_t size(void);

private:
  CcWidget     *m_parentWidget;     ///< Parent Window for followed items.
  CcVector<CcTaskbarItem*> m_MenuTree;///< Top-Level entry points to sub trees
  CcMenuReverse m_RevList;          ///< Reverse List to keep the active tree in min
  CcTaskbar    *m_Taskbar;          ///< Pointer to Taskbar where Menue is docked to.
};

#endif /* CCMENU_H_ */
