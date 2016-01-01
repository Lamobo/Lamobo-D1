/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcFtpClient.h
 * @brief    Class CcFtpClient
 */
#ifndef CcFtpClient_H_
#define CcFtpClient_H_

#include "CcBase.h"
#include "CcThreadObject.h"
#include "com/CcSocket.h"
#include "CcCharArray.h"
#include "CcStringList.h"
#include "CcUrl.h"
/**
 * @brief Button for GUI Applications
 */
class CcFtpClient : public CcThreadObject {
public:
  /**
   * @brief Constructro
   */
  CcFtpClient( void );

  /**
   * @brief Destructor
   */
  virtual ~CcFtpClient( void );
private:
  CcSocket *m_Socket;
  bool m_Done;
};

#endif /* CcFtpClient_H_ */
