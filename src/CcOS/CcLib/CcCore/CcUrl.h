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
 * @file     CcUrl.h
 * @brief    Class CcUrl
 */
#ifndef CcUrl_H_
#define CcUrl_H_

#include "CcBase.h"
#include "CcTypes.h"
#include "CcString.h"

/**
 * @brief String-class for basic string handling
 *
 *  The class is based on std::string and adds some usefull functions
 */
class CcUrl {
private:

public: //methods
  /**
   * @brief Create a empty string-class.
   */
  CcUrl(CcString url = "");

  /**
   * @brief Create a String-class with initialized const char array
   * @param cString: pointer to char array containing a null terminated string
   */
  ~CcUrl();

  void parseUrl(CcString &url);
  bool isUrl(void);

  CcString &getHostname(void);
  CcString &getPort(void);
  CcString &getUsername(void);
  CcString &getPassword(void);
  CcString &getProtocol(void);
  CcString &getPath(void);

private:
  CcString m_Hostname;
  CcString m_Port;
  CcString m_Username;
  CcString m_Password;
  CcString m_Protocol;
  CcString m_Path;
  bool m_IsUrl;
};

#endif /* CcUrl_H_ */
