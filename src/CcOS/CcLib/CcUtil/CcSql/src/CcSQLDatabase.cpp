/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
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
