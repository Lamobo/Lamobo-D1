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
