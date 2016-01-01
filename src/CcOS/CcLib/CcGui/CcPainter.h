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
 * @date       2015-09
 * @par        Language   C++ ANSI V3
 * @file     CcPainter.cpp
 * @brief    Class CcPainter
 *
 *  Low-Level Painting on a Window
 */
#ifndef CCPAINTER_H_
#define CCPAINTER_H_

#include "CcWindow.h"
#include "CcPos.h"
#include "CcCross.h"
#include "CcColor.h"

/**
 * @brief Execute drawing algorithms for CcWindows
 */
class CcPainter {
public:
  /**
  * @brief Constructor
  * @param Window: Target Window for Painting
  **/
  CcPainter(CcWindow *Window);

  /**
   * @brief Destructor
   */
  virtual ~CcPainter();

  /**
   * @brief Set Color of Figure, that has to get drawn
   * @param R: Red-Value
   * @param G: Green-Value
   * @param B: Blue-Value
   */
  void setColor(uint8 R, uint8 G, uint8 B);

  /**
   * @brief Set Color of Figure, that has to get drawn
   * @param Color: Color values
   */
  void setColor(CcColor Color);

  /**
   * @brief Draw a Line to Window
   * @param startX: X Coordinate of Startpoint
   * @param startY: Y Coordinate of Startpoint
   * @param stopX: X Coordinate of Stoppoint
   * @param stopY: Y Coordinate of Stoppoint
   */
  void drawLine(uint16 startX, uint16 startY, uint16 stopX, uint16 stopY);

  /**
   * @brief draw a symetric + Symbol to Window
   * @param position: Position of upper-left corner of cr
   * @param cross
   */
  void drawCross(CcPos &position, CcCross &cross);

private:
  CcWindow* m_Window;
  CcColor   m_Color;
};

#endif /* CCPAINTER_H_ */
