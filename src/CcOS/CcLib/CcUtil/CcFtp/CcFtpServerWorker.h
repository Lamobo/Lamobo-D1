/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcFtpServerWorker.h
 * @brief    Class CcFtpServerWorker
 */
#ifndef CcFtpServerWorker_H_
#define CcFtpServerWorker_H_

#include "CcBase.h"
#include "com/CcSocket.h"
#include "CcCharArray.h"
#include "CcStringList.h"
#include "CcWorker.h"
#include "CcFtpServerPasv.h"

class CcFtpServerWorker : public CcWorker
{
public:
  CcFtpServerWorker(CcSocket *socket);
  virtual ~CcFtpServerWorker();

  void run(void);

  void parseCommand(CcString &CommandBuf);
  bool acceptDataConnection(void);
  CcString produceAbsolutPath(CcString &input);

  bool done;
private:
  CcString      m_WD;
  CcSocket     *m_Socket;
  CcSocket     *m_DataSocket;
  CcCharArray   m_InBuf;
  CcString      m_Temp;
  uint16        m_DataPortInc;
  uint16        m_PasvPort;
  ipv4_t        m_PasvIP;
  bool          m_TransferType;
  bool          m_Active;
};

#endif /* CcFtpServerWorker_H_ */
