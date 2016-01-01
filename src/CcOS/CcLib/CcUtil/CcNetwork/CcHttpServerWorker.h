/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcHttpServerWorker.h
 * @brief    Class CcHttpServerWorker
 */
#ifndef CcHttpServerWorker_H_
#define CcHttpServerWorker_H_

#include "CcBase.h"
#include "com/CcSocket.h"
#include "CcCharArray.h"
#include "CcStringList.h"
#include "CcHttpRequHeader.h"
#include "CcWorker.h"

class CcHttpServerWorker : public CcWorker
{
public:
  CcHttpServerWorker(CcSocket *socket);
  virtual ~CcHttpServerWorker();

  void run(void);
  void stop(void);

  bool chkReadBuf(void);

  bool done;
  CcHttpRequHeader m_Header;
  CcSocket*     m_Socket;
  CcCharArray   m_InBuf;
};

#endif /* CcHttpServerWorker_H_ */
