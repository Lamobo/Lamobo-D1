/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcHttpServer.h
 * @brief    Class CcHttpServer
 */
#ifndef CcHttpServer_H_
#define CcHttpServer_H_

#include "CcBase.h"
#include "com/CcSocket.h"
#include "CcCharArray.h"
#include "CcStringList.h"
#include "CcUrl.h"
#include "CcHttpRequHeader.h"
#include "CcHttpRespHeader.h"
#include "CcApp.h"
#include "CcHttpServerWorker.h"

/**
 * @brief Button for GUI Applications
 */
class CcHttpServer : public CcApp {
public:
  /**
   * @brief Constructor
   */
  CcHttpServer( uint16 Port = 80 );
  CcHttpServer(CcStringList *Arg);

  /**
   * @brief Destructor
   */
  virtual ~CcHttpServer( void );

  static CcApp* main(CcStringList *Arg);

  void run(void);

private:
  CcSocket *m_Socket;
  CcVector<CcHttpServerWorker*> m_WorkerList;
  uint16    m_Port;
};

#endif /* CcHttpServer_H_ */
