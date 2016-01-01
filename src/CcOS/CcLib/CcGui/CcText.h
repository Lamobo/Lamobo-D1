/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcText.h
 * @brief    Class CcText
 */

#ifndef CCTEXT_H_
#define CCTEXT_H_

#include "CcWidget.h"
#include "CcFont.h"
#include "CcString.h"

/**
 * @brief Class for creating an manipulating a Textbox on Display.
 */
class CcText: public CcWidget, CcFont {
public:
  /**
   * @brief Constructor with Font-size of displaying text
   * @param fontSize: FontSize of Displayin Text in pixles, default:6
   */
  CcText( CcWidget *parent, uint16 fontSize = 8 );
  /**
   * @brief Destructor
   */
  virtual ~CcText();

  /**
   * @brief Set Color of Font
   */
  void setFontColor(uchar R, uchar G, uchar B);
  /**
   * @brief Set Offset where Text has to Start on Display
   */
  void setTextOffset(uint16 x, uint16 y );

  /**
   * @brief Set String that has to be shown on Display
   */
  void setString(CcString sString);

  /**
   * @brief Flush Text out on Display
   */
  void drawString( void );

  /**
   * @brief Get the Size of Window Text will need to show
   */
  void getTextSize( uint16* x, uint16* y );

  /**
   * @brief get Handle to internal String-Member
   */
  CcString* getString( void );
protected:
  /**
   * @brief Start the Calulation for Size of Window for displaying Text
   */
  void calcTextSize( void );
private:
  /**
   * @brief Write a Char to Display, Position was previously set.
   * @param cValue: Char to Write
   */
  void writeChar(char cValue);

private: //members
  CcString m_sString;   ///< String for Display
  char m_cFontColorR;   ///< Red-Color of Font
  char m_cFontColorG;   ///< Green-Color of Font
  char m_cFontColorB;   ///< Blue-Color of Font
  uint16 m_uiOffsetX;   ///< Position-Offest Y-Value Text is shown
  uint16 m_uiOffsetY;   ///< Position-Offest X-Value Text is shown
  uint16 m_TextSizeX;   ///< Calculated width in Pixles of showing Text
  uint16 m_TextSizeY;   ///< Calculated height in Pixles of showing Text
};

#endif /* CCTEXT_H_ */
