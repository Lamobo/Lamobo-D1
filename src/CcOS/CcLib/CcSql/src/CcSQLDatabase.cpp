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
 * @file     CcSQLDatabase.cpp
 * @brief    Implementation of Class CcSQL
 */
#include "CcSQLDatabase.h"
#include "CcSqlite.h"

CcSQLDatabase::CcSQLDatabase(eSQLDatabaseType type )
{
  switch(type){
    case eSqlite:
      m_Database = new CcSqlite;
      break;
  }
}

CcSQLDatabase::~CcSQLDatabase( void )
{
}

bool CcSQLDatabase::open()
{
  return m_Database->open();
}
bool CcSQLDatabase::query(CcString queryString)
{
  return m_Database->query(queryString);
}
bool CcSQLDatabase::close()
{
  return m_Database->close();
}

void CcSQLDatabase::setConnection(CcString connection)
{
  m_Database->setConnection(connection);
}

void CcSQLDatabase::setUsername(CcString username)
{
  m_Database->setUsername(username);
}
void CcSQLDatabase::setPassword(CcString password)
{
  m_Database->setPassword(password);
}
void CcSQLDatabase::setDatabase(CcString database)
{
  m_Database->setDatabase(database);
}
CcString* CcSQLDatabase::getConnection(void)
{
  return m_Database->getConnection();
}
CcString* CcSQLDatabase::getUsername(void)
{
  return m_Database->getUsername();
}
CcString* CcSQLDatabase::getPassword(void)
{
  return m_Database->getPassword();
}
CcString* CcSQLDatabase::getDatabase(void)
{
  return m_Database->getDatabase();
}
