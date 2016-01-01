/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcLog.h
 * @brief    Class CcLog
 */
#ifndef CcLog_H_
#define CcLog_H_

#include "CcBase.h"
#include "CcObject.h"

/**
 * @brief Default Class to create a Application
 */
class CcLog : public CcObject {
public:
  CcLog();
  virtual ~CcLog();
};

#endif /* CcLog_H_ */
