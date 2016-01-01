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
 * @file     CcColor.h
 * @brief    Class CcColor
 */
#ifndef CCCOLOR_H_
#define CCCOLOR_H_

#include "CcBase.h"

class CcColor {
public:
  /**
   * @brief Enum for arangement of Colors in a single value.
   */
  typedef enum{
    eColorRGB,
    eColorARGB,
    eColorBGR,
  }eColorType;

  /**
   * @brief Constructor
   */
  CcColor();

  /**
   * @brief Create Object with initiating Color
   * @param color: RGB value stored in a 16bit value
   * @prarm type: Type of color arrangement in color-value, default eColorRGB
   */
  CcColor(uint16 color, eColorType type = eColorRGB);

  /**
   * @brief Create Object with initiating Color
   * @param color: RGB value stored in a 32bit value
   * @prarm type: Type of color arrangement in color-value, default eColorRGB
   */
  CcColor(uint32 color, eColorType type = eColorRGB);

  /**
   * @brief Create Object with initiating Color
   * @param R: Red value
   * @param G: Green value
   * @param B: Blue value
   * @param A: Alpha value for transparency;
   */
  CcColor(uint8 R, uint8 G, uint8 B, uint8 A=0);

  virtual ~CcColor();

  void setColor(uint16 color, eColorType type = eColorRGB);
  void setColor(uint32 color, eColorType type = eColorRGB);
  void setColor(uint8 R, uint8 G, uint8 B, uint8 A=0);
  uint8 getR( void ){ return m_R;}
  uint8 getG( void ){ return m_G;}
  uint8 getB( void ){ return m_B;}
  uint8 getA( void ){ return m_A;}
  void setColorR(uint8 R){m_R=R;}
  void setColorG(uint8 G){m_G=G;}
  void setColorB(uint8 B){m_B=B;}
  void setColorA(uint8 A){m_A=A;}

  uint16 getColor16Bit( eColorType type = eColorRGB);
  uint32 getColor32Bit( eColorType type = eColorRGB);

  uint8 m_R;    ///< Red value
  uint8 m_G;    ///< Green value
  uint8 m_B;    ///< Blue value
  uint8 m_A;    ///< Alpha value
};

#endif /* CCCOLOR_H_ */
