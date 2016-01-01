/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcTaskbarItem.h
 * @brief    Class CcTaskbarItem
 */
#ifndef CCTASKBARITEM_H_
#define CCTASKBARITEM_H_

#include "CcBase.h"
#include "CcWidget.h"
#include "CcString.h"
#include "CcPushButton.h"
#include "CcMenuReverse.h"
#include "CcMenuItem.h"

class CcTaskbar;

/**
 * @brief Button for GUI Applications
 */
class CcTaskbarItem : public CcMenuItem {
public:
  /**
   * @brief Constructor
   */
  CcTaskbarItem(CcWidget *parent, CcString name = "");

  /**
   * @brief Destructor
   */
  virtual ~CcTaskbarItem(void);

  void setTaskbar(CcTaskbar *Taskbar);
  /**
  * @brief Draw Next Stage of Menu, and create Buttons for Input.
  */
  virtual void drawMenuTree(void);

  /**
  * @brief Hide all Displayed Elements of next Stages
  */
  virtual void hideMenuTree(void);
private:
  CcTaskbar *m_Taskbar;
};

#endif /* CCTASKBARITEM_H_ */
