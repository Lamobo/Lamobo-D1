/**
 * @copyright  Andreas Dirmeier (C) 2015
 *
 * This file is part of CcOS.
 *
 * CcOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CcOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CcOS.  If not, see <http://www.gnu.org/licenses/>.
 **/
/**
 * @author     Andreas Dirmeier
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
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
