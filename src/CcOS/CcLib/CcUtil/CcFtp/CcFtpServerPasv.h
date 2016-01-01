/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcFtpServerPasv.h
 * @brief    Class CcFtpServerPasv
 */
#ifndef CcFtpServerPasv_H_
#define CcFtpServerPasv_H_

#include "CcBase.h"
#include "com/CcSocket.h"
#include "CcCharArray.h"
#include "CcStringList.h"
#include "CcWorker.h"

class CcFtpServerPasv : public CcWorker
{
public:
  CcFtpServerPasv(CcSocket *socket);
  virtual ~CcFtpServerPasv();

  void run(void);

  void parseCommand(CcString &CommandBuf);

  bool done;
  CcString      m_WD;
  CcSocket*     m_Socket;
  CcCharArray   m_InBuf;
  bool m_TransferType;
  bool m_Active;
};

#endif /* CcFtpServerPasv_H_ */
