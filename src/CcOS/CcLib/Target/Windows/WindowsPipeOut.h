/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     WindowsPipeOut.h
 * @brief    Class WindowsPipeOut
 */
#ifndef WindowsPipeOut_H_
#define WindowsPipeOut_H_

#include "CcBase.h"
#include "WindowsGlobals.h"
#include "CcIODevice.h"
#include "CcThreadObject.h"

/**
 * @brief Button for GUI Applications
 */
class WindowsPipeOut : public CcThreadObject {
public:
  /**
   * @brief Constructor
   */
  WindowsPipeOut(CcIODevice *out);

  /**
   * @brief Start transfering to output device
   */
  void run(void);

  /**
   * @brief Destructor
   */
  virtual ~WindowsPipeOut( void );

  HANDLE m_Handle;
public:
  HANDLE m_hRead;
  CcIODevice *m_IODev;
};

#endif /* WindowsPipeOut_H_ */
