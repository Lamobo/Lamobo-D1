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
 * @file     CcHttpServerWorker.cpp
 * @brief    Implementation of Class CcHttpServerWorker
 *           Protocol: http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
 */
#include "CcKernel.h"
#include "CcThreadObject.h"
#include "CcHttpServerWorker.h"
#include "CcHttpRespHeader.h"

CcHttpServerWorker::CcHttpServerWorker(CcSocket *socket) :
  m_Socket(socket),
  m_Header(false)
{
}

CcHttpServerWorker::~CcHttpServerWorker(void){
  if (m_Socket != 0)
    delete m_Socket;
}

void CcHttpServerWorker::run(){
  char recBuf[1024];
  size_t recSize = 0;
  if (m_Socket != 0)
  {
    recSize = m_Socket->read(recBuf, 1024);
    while (recSize != SIZE_MAX && recSize != 0){
      m_InBuf.append(recBuf, recSize);
      if (chkReadBuf())
        break;
      recSize = m_Socket->read(recBuf, 1024);
    }
    //TODO: Implement WebpageProvider
    if (m_Header.m_HTTPMethod == "GET"){
      CcHttpRespHeader DefaultHeader(true);
      CcCharArray Message;
      Message.appendConst("<h1>Welcome to the CcOS HTTP-Webserver</h1>");
      Message.appendConst("This Page is default set on Webserver and indicates no installed WebpageProvider");
      DefaultHeader.m_ContentLength.appendNumber(Message.size());
      CcCharArray Response;
      CcString Resp = DefaultHeader.getHeader();
      Response.append(Resp.getCharString(), Resp.length());
      Response.append(Message);
      m_Socket->write(Response.getContent(), Response.size());
      m_Socket->close();
      m_InBuf.clear();
    }
    //TODO: Implement WebpageProvider
    else if (m_Header.m_HTTPMethod == "POST"){
      CcHttpRespHeader DefaultHeader(true);
      CcCharArray Message;
      Message.appendConst("<h1>Welcome to the CcOS HTTP-Webserver</h1>");
      Message.appendConst("This Page is default set on Webserver and indicates no installed WebpageProvider");
      DefaultHeader.m_ContentLength.appendNumber(Message.size());
      CcCharArray Response;
      CcString Resp = DefaultHeader.getHeader();
      Response.append(Resp.getCharString(), Resp.length());
      Response.append(Message);
      m_Socket->write(Response.getContent(), Response.size());
      m_Socket->close();
      m_InBuf.clear();
    }
    //TODO: Implement WebpageProvider
    else if (m_Header.m_HTTPMethod == "HEAD"){
      CcHttpRespHeader DefaultHeader(true);
      CcCharArray Message;
      DefaultHeader.m_ContentLength.appendNumber(Message.size());
      CcCharArray Response;
      CcString Resp = DefaultHeader.getHeader();
      Response.append(Resp.getCharString(), Resp.length());
      Response.append(Message);
      m_Socket->write(Response.getContent(), Response.size());
      m_Socket->close();
      m_InBuf.clear();
    }
  }
}

void CcHttpServerWorker::stop(void){
}

bool CcHttpServerWorker::chkReadBuf(void){
  bool bRet = false;
  size_t pos;
  CcCharArray toFind("\n\n");
  pos = m_InBuf.contains(toFind);
  if (pos == SIZE_MAX)
  {
    CcCharArray toFind("\r\n\r\n");
    pos = m_InBuf.contains(toFind);
  }
  if (pos != SIZE_MAX)
  {
    CcString sHeader;
    sHeader.append(m_InBuf,0, pos);
    m_Header.parse(sHeader);
    bRet = true;
  }
  return bRet;
}
