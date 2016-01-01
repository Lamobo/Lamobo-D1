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
* @file     WindowsPipeIn.h
* @brief    Class WindowsPipeIn
*/
#ifndef WindowsPipeIn_H_
#define WindowsPipeIn_H_

#include "CcBase.h"
#include "WindowsGlobals.h"
#include "CcIODevice.h"
#include "CcThreadObject.h"

/**
* @brief Button for GUI Applications
*/
class WindowsPipeIn : public CcThreadObject {
public:
  /**
  * @brief Constructor
  */
  WindowsPipeIn(CcIODevice *out);

  /**
  * @brief Start transfering to output device
  */
  void run(void);

  /**
  * @brief Destructor
  */
  virtual ~WindowsPipeIn(void);

  HANDLE m_Handle;
public:
  HANDLE m_hWrite;
  CcIODevice *m_IODev;
};

#endif /* WindowsPipeIn_H_ */
