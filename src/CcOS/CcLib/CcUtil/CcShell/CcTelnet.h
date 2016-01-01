/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcTelnet.h
 * @brief    Class CcTelnet
 */
#ifndef CcTelnet_H_
#define CcTelnet_H_

#include "CcBase.h"
#include "CcShell.h"
#include "com/CcSocket.h"

/**
 * @brief Button for GUI Applications
 */
class CcTelnet : CcApp {
public:
  /**
   * @brief Constructor
   */
  CcTelnet( uint16 Port = 23 );

  /**
   * @brief Destructor
   */
  virtual ~CcTelnet( void );

  void run(void);

private:
  uint16    m_Port;
  CcShell   *m_Shell;
  CcSocket  *m_Socket;
};

#endif /* CcTelnet_H_ */
