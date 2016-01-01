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
 * @file     CcLed.h
 * @brief    Class CcLed
 */

#ifndef CcLed_H_
#define CcLed_H_

#include "CcBase.h"

/**
 * @brief Control the Input and Outputports on device
 */
class CcLed {
public:
  /**
   * @brief Structure holding Pin Settings
   */
  typedef struct{
    uint16 PinFunction;
    uint16 PinFunctionAvailable;
  } sPin;

  /**
   * @brief Constructor
   */
  CcLed();

  /**
   * @brief Destructor
   */
  virtual ~CcLed();

  /**
   * @brief Initialize basic settings for General Purpose Input Output
   */
  virtual void init( void ) = 0;

  sPin *m_PinList; ///< List of all existing Pins available in this Class.
};

#endif /* CcLed_H_ */
