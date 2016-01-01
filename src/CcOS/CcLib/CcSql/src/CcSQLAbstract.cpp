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
