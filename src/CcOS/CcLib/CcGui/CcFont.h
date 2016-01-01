/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
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
