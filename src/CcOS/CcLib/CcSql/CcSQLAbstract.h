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
 * @file     CcSQLAbstract.h
 * @brief    Class CcSQLAbstract
 */
#ifndef CCSQLABSTRACT_H_
#define CCSQLABSTRACT_H_

#include "CcBase.h"
#include "CcString.h"

/**
 * @brief Button for GUI Applications
 */
class CcSQLAbstract {
public:
  /**
   * @brief Constructor
   */
  CcSQLAbstract( void );

  /**
   * @brief Destructor
   */
  virtual ~CcSQLAbstract( void );

  virtual bool open()=0;
  virtual bool query(CcString &queryString)=0;
  virtual bool close()=0;
  void setConnection(CcString connection);
  void setUsername(CcString username);
  void setPassword(CcString password);
  void setDatabase(CcString database);
  CcString* getConnection(void);
  CcString* getUsername(void);
  CcString* getPassword(void);
  CcString* getDatabase(void);
private:
  CcString m_Connection;
  CcString m_Username;
  CcString m_Password;
  CcString m_Database;
};

#endif /* CCSQLABSTRACT_H_ */
