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
 * @file     CcSQLDatabase.h
 * @brief    Class CcSQLDatabase
 */
#ifndef CCSQLDATABASE_H_
#define CCSQLDATABASE_H_

#include "CcBase.h"
#include "CcSQLAbstract.h"

typedef enum{
  eSqlite = 0,
}eSQLDatabaseType;

/**
 * @brief Button for GUI Applications
 */
class CcSQLDatabase {
public:
  /**
   * @brief Constructor
   */
  CcSQLDatabase( eSQLDatabaseType type = eSqlite );

  /**
   * @brief Destructor
   */
  virtual ~CcSQLDatabase( void );

  bool open();
  bool query(CcString queryString);
  bool close();
  void setConnection(CcString connection);
  void setUsername(CcString username);
  void setPassword(CcString password);
  void setDatabase(CcString database);
  CcString* getConnection(void);
  CcString* getUsername(void);
  CcString* getPassword(void);
  CcString* getDatabase(void);

  CcSQLAbstract* m_Database;
};

#endif /* CCSQLDATABASE_H_ */
