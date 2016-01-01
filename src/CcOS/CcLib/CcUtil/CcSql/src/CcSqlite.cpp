/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcSqlite.cpp
 * @brief    Implementation of Class CcSqlite
 */
#include "CcSqlite.h"
#include "stdio.h"

CcSqlite::CcSqlite( void )
{
}

CcSqlite::~CcSqlite( void )
{
}

bool CcSqlite::open(){
  int iState;
  iState = sqlite3_open(getDatabase()->getCharString(), &m_Sqlite);
  if( iState != SQLITE_OK ){
    //TODO: replace with CcLog
    printf("Can't open database: %s\n", sqlite3_errmsg(m_Sqlite));
    sqlite3_close(m_Sqlite);
    return(false);
  }
  return true;
}

bool CcSqlite::query(CcString &queryString){
  int iState;
  char *zErrMsg = 0;
  iState = sqlite3_exec(m_Sqlite, queryString.getCharString(), &CcSqlite::sqliteCallback, (void*)this, &zErrMsg);
  if( iState != SQLITE_OK ){
    //TODO: replace with CcLog
    printf("SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return false;
  }
  return true;
}

bool CcSqlite::close(){
  sqlite3_close(m_Sqlite);
  return false;
}

int CcSqlite::sqliteCallback(void *sqliteClass, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  ((CcSqlite*)sqliteClass)->call();
  return 0;
}

void CcSqlite::call(){
  printf("test\n\n");
}
