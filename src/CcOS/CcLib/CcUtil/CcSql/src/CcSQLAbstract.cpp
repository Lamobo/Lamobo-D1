/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcSQLAbstract.cpp
 * @brief    Implementation of Class CcSQLAbstract
 */
#include "CcSQLAbstract.h"

CcSQLAbstract::CcSQLAbstract( void )
{
}

CcSQLAbstract::~CcSQLAbstract( void )
{
}

void CcSQLAbstract::setConnection(CcString connection){
  m_Connection = connection;
}

void CcSQLAbstract::setUsername(CcString username){
  m_Username = username;
}

void CcSQLAbstract::setPassword(CcString password){
  m_Password = password;
}

void CcSQLAbstract::setDatabase(CcString database){
  m_Database = database;
}

CcString* CcSQLAbstract::getConnection(void){
  return &m_Connection;
}

CcString* CcSQLAbstract::getUsername(void){
  return &m_Username;
}

CcString* CcSQLAbstract::getPassword(void){
  return &m_Password;
}

CcString* CcSQLAbstract::getDatabase(void){
  return &m_Database;
}
