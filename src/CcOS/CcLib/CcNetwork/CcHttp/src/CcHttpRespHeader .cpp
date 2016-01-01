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
 * @file     CcHttpRespHeader.cpp
 * @brief    Implementation of Class CcHttpRespHeader
 */
#include "CcHttpRespHeader.h"
#include "CcKernel.h"

CcHttpRespHeader::CcHttpRespHeader( CcString Parse )
{
  if (Parse.length())
    parse(Parse);
}

CcHttpRespHeader::CcHttpRespHeader(bool init)
{
  if (init){
    m_AcceptRanges      = "bytes";
    m_Allow             = "GET, HEAD, POST";
    m_CacheControl      = "";
    m_Connection        = "close";
    m_ContentEncoding   = "";
    m_ContentLength     = "";
    m_ContentLocation   = "";
    m_ContentMd5        = "";
    m_ContentRange      = "";
    m_ContentType       = "";
    m_Date              = "";
    m_HTTP              = "HTTP/1.1 200 All Ok";
    m_LastModified      = "";
    m_Location          = "";
    m_Refresh           = "";
    m_Server            = "CcOS HttpServer";
    m_SetCookie         = "";
    m_TransferEncoding  = "";
  }
}

CcHttpRespHeader::~CcHttpRespHeader( void )
{
}

CcString CcHttpRespHeader::getHeader(void){
  CcString Header;
  if (m_HTTP.length())
    Header += m_HTTP + "\r\n";
  if (m_AcceptRanges.length())
    Header += CcString() + "Accept-Ranges: " + m_AcceptRanges + "\r\n";
  if (m_Allow.length())
    Header += CcString() + "Allow: " + m_Allow + "\r\n";
  if (m_CacheControl.length())
    Header += CcString() + "Cache-Control: " + m_CacheControl + "\r\n";
  if (m_Connection.length())
    Header += CcString() + "Connection: " + m_Connection + "\r\n";
  if (m_ContentEncoding.length())
    Header += CcString() + "Content-Encoding: " + m_ContentEncoding + "\r\n";
  if (m_ContentLength.length())
    Header += CcString() + "Content-Length: " + m_ContentLength + "\r\n";
  if (m_ContentLocation.length())
    Header += CcString() + "Content-Location: " + m_ContentLocation + "\r\n";
  if (m_ContentMd5.length())
    Header += CcString() + "Content-MD5: " + m_ContentMd5 + "\r\n";
  if (m_ContentRange.length())
    Header += CcString() + "Content-Range: " + m_ContentRange + "\r\n";
  if (m_ContentType.length())
    Header += CcString() + "Content-Type: " + m_ContentType + "\r\n";
  if (m_Date.length())
    Header += CcString() + "Date: " + m_Date + "\r\n";
  if (m_LastModified.length())
    Header += CcString() + "Last-Modified: " + m_LastModified + "\r\n";
  if (m_Location.length())
    Header += CcString() + "Location: " + m_Location + "\r\n";
  if (m_Refresh.length())
    Header += CcString() + "Refresh: " + m_Refresh + "\r\n";
  if (m_Server.length())
    Header += CcString() + "Server: " + m_Server + "\r\n";
  if (m_SetCookie.length())
    Header += CcString() + "Set-Cookie: " + m_SetCookie + "\r\n";
  if (m_TransferEncoding.length())
    Header += CcString() + "Transfer-Encoding: " + m_TransferEncoding + "\r\n";
  Header += "\r\n";
  return Header;
}

void CcHttpRespHeader::parse(CcString &Parse){
  CcString sLine = Parse.getLine();
  while (sLine != ""){
    parseLine(sLine);
    sLine = Parse.getLine();
  }
}

void CcHttpRespHeader::parseLine(CcString &Parse){
  size_t pos = Parse.contains(":");
  if (pos < Parse.length()){
    CcString sArgument = Parse.substr(0, pos);
    CcString sValue = Parse.substr(pos + 1).trim();
    if      (sArgument == "Accept-Ranges")
      m_AcceptRanges = sValue;
    else if (sArgument == "Allow")
      m_Allow = sValue;
    else if (sArgument == "Cache-Control")
      m_CacheControl = sValue;
    else if (sArgument == "Connection")
      m_Connection = sValue;
    else if (sArgument == "Content-Encoding")
      m_ContentEncoding = sValue;
    else if (sArgument == "Content-Length")
      m_ContentLength = sValue;
    else if (sArgument == "Content-Location")
      m_ContentLocation = sValue;
    else if (sArgument == "Content-MD5")
      m_ContentMd5 = sValue;
    else if (sArgument == "Content-Range")
      m_ContentRange = sValue;
    else if (sArgument == "Content-Type")
      m_ContentType = sValue;
    else if (sArgument == "Date")
      m_Date = sValue;
    else if (sArgument == "Last-Modified")
      m_LastModified = sValue;
    else if (sArgument == "Location")
      m_Location = sValue;
    else if (sArgument == "Refresh")
      m_Refresh = sValue;
    else if (sArgument == "Server")
      m_Server = sValue;
    else if (sArgument == "Cookie")
      m_SetCookie = sValue;
    else if (sArgument == "Transfer-Encoding")
      m_TransferEncoding = sValue;
    else
      Kernel.delayMs(1);
  }
  else if (SIZE_MAX != Parse.contains("HTTP")){
    m_HTTP = Parse.getStringBetween(" ", " ");
  }
}
