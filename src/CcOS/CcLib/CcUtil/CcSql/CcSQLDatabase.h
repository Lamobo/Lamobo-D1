/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
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
