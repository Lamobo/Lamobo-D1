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
 * @file     CcTelnet.h
 * @brief    Class CcTelnet
 */
#ifndef CcTelnet_H_
#define CcTelnet_H_

#include "CcBase.h"
#include "CcShell.h"
#include "com/CcSocket.h"

/**
 * @brief Button for GUI Applications
 */
class CcTelnet : CcApp {
public:
  /**
   * @brief Constructor
   */
  CcTelnet( uint16 Port = 23 );

  /**
   * @brief Destructor
   */
  virtual ~CcTelnet( void );

  void run(void);

private:
  uint16    m_Port;
  CcShell   *m_Shell;
  CcSocket  *m_Socket;
};

#endif /* CcTelnet_H_ */
