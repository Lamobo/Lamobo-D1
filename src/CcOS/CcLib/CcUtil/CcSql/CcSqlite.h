/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcSqlite.h
 * @brief    Class CcSqlite
 */
#ifndef CcSqlite_H_
#define CcSqlite_H_

#include "CcBase.h"
#include "CcSQLAbstract.h"
#include "sqlite/sqlite3.h"

/**
 * @brief Button for GUI Applications
 */
class CcSqlite : public CcSQLAbstract {
public:
  /**
   * @brief Constructor
   */
  CcSqlite( void );

  /**
   * @brief Destructor
   */
  virtual ~CcSqlite( void );

  bool open();
  bool query(CcString &queryString);
  bool close();
  void call();
  static int sqliteCallback(void *sqliteClass, int argc, char **argv, char **azColName);

private:
  sqlite3 *m_Sqlite;
};

#endif /* CcSqlite_H_ */
