/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcMenuReverse.h
 * @brief    Class CcMenuReverse
 */
#ifndef CCMENUREVERSE_H_
#define CCMENUREVERSE_H_

#include "CcBase.h"
#include "CcVector.h"
#include "CcObjectHandler.h"

class CcMenuItem;

/**
 * @brief Button for GUI Applications
 */
class CcMenuReverse : public CcVector<CcMenuItem*> {
public:
  /**
   * @brief Constructor
   */
  CcMenuReverse(void);

  /**
   * @brief Destructor
   */
  virtual ~CcMenuReverse(void);

  /**
   * @brief set position to next Item
   */
  void nextPos(void);

  /**
   * @brief get item from actual position
   * @return actual item
   */
  CcMenuItem* getPos(void);

  /**
  * @brief reset position to first element in list
  */
  void resetPos(void);

private:
  uint16 m_Pos;
};

#endif /* CCMENUREVERSE_H_ */
