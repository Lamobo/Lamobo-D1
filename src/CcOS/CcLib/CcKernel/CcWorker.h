/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcWorker.h
 * @brief    Class CcWorker
 */
#ifndef CcWorker_H_
#define CcWorker_H_

#include "CcBase.h"
#include "CcThreadObject.h"

/**
 * @brief Button for GUI Applications
 */
class CcWorker : public CcThreadObject {
public:
  /**
   * @brief Constructor
   */
  CcWorker( void );

  /**
   * @brief Destructor
   */
  virtual ~CcWorker( void );

  virtual void run()=0;

  void enterState(eThreadState State);

};

#endif /* CcWorker_H_ */
