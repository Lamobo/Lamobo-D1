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
 * @file     CcFont.h
 * @brief    Class CcFont
 */
#ifndef CCFONT_H_
#define CCFONT_H_

#include "CcBase.h"
/**
 * @brief Class for Handeling Fonts
 */
class CcFont {
public:
  /**
   * @brief Constructor with target Font-size
   */
  CcFont( uint16 uiFontSizeX );

  /**
   * @brief Destructor
   */
  virtual ~CcFont();

  /**
   * @brief Get Pixle-values from an char
   */
  char* getPixles(char cValue);

  /**
   * @brief Get width of actual Font
   */
  uint16 getFontSizeX( void ){ return m_FontSizeX;}

  /**
   * @brief get height of actual Font
   */
  uint16 getFontSizeY( void ){ return m_FontSizeY;}

protected:
  uint16 m_FontSize;    ///< Font-size set from constructor
  uint16 m_FontSizeX;   ///< Font-width of loaded Font
  uint16 m_FontSizeY;   ///< Font-height of loaded Font

  char* m_cFontBuffer;  ///< pointer to Buffer of loaded Font
};

#endif /* CCFONT_H_ */
