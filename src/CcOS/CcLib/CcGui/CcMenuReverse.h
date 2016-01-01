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
