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
