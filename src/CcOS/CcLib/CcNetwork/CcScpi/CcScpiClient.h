/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcScpiClient.h
 * @brief    Class CcScpiClient
 */
#ifndef CcScpiClient_H_
#define CcScpiClient_H_

#include "CcBase.h"

/**
 * @brief Button for GUI Applications
 */
class CcScpiClient {
public:
  /**
   * @brief Constructor
   */
  CcScpiClient( void );

  /**
   * @brief Destructor
   */
  virtual ~CcScpiClient( void );
};

#endif /* CcScpiClient_H_ */
