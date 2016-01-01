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
 * @file     CcProcess.h
 * @brief    Class CcProcess
 */
#ifndef CcProcess_H_
#define CcProcess_H_

#include "CcBase.h"
#include "CcString.h"
#include "CcStringList.h"
#include "CcIODevice.h"

/**
 * @brief Button for GUI Applications
 */
class CcProcess {
public:
  /**
   * @brief Constructor
   */
  CcProcess( void );

  /**
   * @brief Destructor
   */
  virtual ~CcProcess( void );

  CcString m_Name;
  CcStringList m_Arguments;
  CcIODevice *m_Input;
  CcIODevice *m_Output;
};

#endif /* CcProcess_H_ */
