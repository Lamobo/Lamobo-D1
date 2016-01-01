/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     WindowsPipeIn.h
* @brief    Class WindowsPipeIn
*/
#ifndef WindowsPipeIn_H_
#define WindowsPipeIn_H_

#include "CcBase.h"
#include "WindowsGlobals.h"
#include "CcIODevice.h"
#include "CcThreadObject.h"

/**
* @brief Button for GUI Applications
*/
class WindowsPipeIn : public CcThreadObject {
public:
  /**
  * @brief Constructor
  */
  WindowsPipeIn(CcIODevice *out);

  /**
  * @brief Start transfering to output device
  */
  void run(void);

  /**
  * @brief Destructor
  */
  virtual ~WindowsPipeIn(void);

  HANDLE m_Handle;
public:
  HANDLE m_hWrite;
  CcIODevice *m_IODev;
};

#endif /* WindowsPipeIn_H_ */
