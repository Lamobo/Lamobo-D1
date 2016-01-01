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
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 * @file     CcUrl.h
 * @brief    Implementation of class CcUrl
 */
#include "CcUrl.h"

CcUrl::CcUrl(CcString Url)
{
  if (Url != "")
    parseUrl(Url);
}

CcUrl::~CcUrl()
{
}

void CcUrl::parseUrl(CcString &url){
  m_IsUrl = false;
  CcString userPart;
  CcString hostPart;
  size_t pos = url.contains("://");
  if (pos < url.length()){
    m_IsUrl = true;
    m_Protocol = url.substr(0, pos);
    hostPart.append(url.getStringBetween("://", "/").getCharString());
    pos += 3 + hostPart.length();
    m_Path = url.substr(pos);
    if (m_Path.length() == 0){
      m_Path = "/";
    }
    pos = hostPart.contains("@");
    if (pos < hostPart.length())
    {
      userPart = hostPart.substr(0, pos);
      hostPart = hostPart.substr(pos + 1);
    }
    pos = hostPart.contains(":");
    if (pos < hostPart.length())
    {
      m_Hostname = hostPart.substr(0, pos);
      m_Port = hostPart.substr(pos + 1);
    }
    else
    {
      m_Hostname = hostPart;
      m_Port = "80";
    }
    pos = userPart.contains(":");
    if (pos < userPart.length())
    {
      m_Username = userPart.substr(0, pos);
      m_Password = userPart.substr(pos + 1);
    }
    else{
      m_Username = userPart;
    }
  }
  m_IsUrl = true;
}

bool CcUrl::isUrl(void){
  return m_IsUrl;
}

CcString &CcUrl::getHostname(void){
  return m_Hostname;
}

CcString &CcUrl::getPort(void){
  return m_Port;
}

CcString &CcUrl::getUsername(void){
  return m_Username;
}

CcString &CcUrl::getPassword(void){
  return m_Password;
}

CcString &CcUrl::getProtocol(void){
  return m_Protocol;
}

CcString &CcUrl::getPath(void){
  return m_Path;
}

