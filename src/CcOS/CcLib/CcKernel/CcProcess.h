/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcProcess.h
 * @brief    Class CcProcess
 */
#ifndef CcProcess_H_
#define CcProcess_H_

#include "CcBase.h"
#include "CcString.h"
#include "CcStringList.h"
#include "CcIODevice.h"

/**
 * @brief Button for GUI Applications
 */
class CcProcess {
public:
  /**
   * @brief Constructor
   */
  CcProcess( void );

  /**
   * @brief Destructor
   */
  virtual ~CcProcess( void );

  CcString m_Name;
  CcStringList m_Arguments;
  CcIODevice *m_Input;
  CcIODevice *m_Output;
};

#endif /* CcProcess_H_ */
