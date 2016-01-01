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
* @file     CcSocket.h
* @brief    Class CcSocket
*/
#ifndef CcSocket_H_
#define CcSocket_H_

#include "CcBase.h"
#include "CcIODevice.h"
#include "CcTypes.h"
#include "CcString.h"

typedef enum {
  eTCP = 0,
  eUDP,
} eSocketType;

/**
* @brief Button for GUI Applications
*/
class CcSocket : public CcIODevice{
public:
  /**
  * @brief Constructor
  */
  CcSocket( eSocketType type = eTCP );

  /**
   * @brief Destructor
   */
  virtual ~CcSocket( void );


  virtual bool open(uint16) = 0;
  virtual bool close() = 0;
  virtual size_t write(char *buf, size_t bufSize) = 0;
  virtual size_t read(char *buf, size_t bufSize) = 0;
  /**
   * @brief connect to Host with known IP-Address and Port
   * @param ipAdress: IpAddress of Host
   * @param Port:     Port where host ist waiting for connection
   * @return true if connection was successfully established
   */
  virtual bool bind(ipv4_t ipAddress, uint16 Port) = 0;

  /**
   * @brief connect to Host with known Name in Network and Port
   * @param hostName: Name of Host to connect to
   * @param Port:     Port where host ist waiting for connection
   * @return true if connection was successfully established
   */
  virtual bool connect(ipv4_t ipAddress, uint16 Port) = 0;

  /**
   * @brief Socket becomes a Host and listen on Port
   * @param Port: Value of Port-Address
   * @return true if port is successfully initiated.
   */
  virtual bool listen(void) = 0;

  /**
   * @brief Waiting for an incoming connection.
   * @return Valid socket if connection established, otherwise 0.
   */
  virtual CcSocket* accept(void) = 0;

  virtual bool getHostByName(CcString &hostname, ipv4_t *addr) = 0;

  /**
  * @brief connect to Host with known Name in Network and Port
  *        can be overloaded if System can connect by name.
  * @param hostName: Name of Host to connect to
  * @param hostPort: Port where host ist waiting for connection
  * @return true if connection was successfully established
  */
  virtual bool connect(CcString& hostName, CcString &hostPort);

public:
  eSocketType m_SockType;
  ipv4_t m_ConnectionIP;      ///< Connection IP used for Client and Server
  uint16 m_ConnectionPort;    ///< Connection Port used for Client and Server
  ipv4_t m_FromIP;
  uint16 m_FromPort;
};

#endif /* CcSocket_H_ */
