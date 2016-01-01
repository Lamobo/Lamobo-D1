/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcFtpServer.h
 * @brief    Class CcFtpServer
 */
#ifndef CcFtpServer_H_
#define CcFtpServer_H_

#include "CcBase.h"
#include "com/CcSocket.h"
#include "CcCharArray.h"
#include "CcStringList.h"
#include "CcUrl.h"
#include "CcApp.h"
#include "CcFtpServerWorker.h"

/**
 * @brief Button for GUI Applications
 */
class CcFtpServer : public CcApp {
public:
  /**
   * @brief Constructor
   */
  CcFtpServer( uint16 Port = 27521 );
  CcFtpServer(CcStringList *Arg);

  /**
   * @brief Destructor
   */
  virtual ~CcFtpServer( void );

  static CcApp* main(CcStringList *Arg);
  void run(void);

private:
  CcSocket *m_Socket;
  CcVector<CcFtpServerWorker*> m_WorkerList;
  uint16    m_Port;
};

#endif /* CcFtpServer_H_ */
