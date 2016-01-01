/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
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
