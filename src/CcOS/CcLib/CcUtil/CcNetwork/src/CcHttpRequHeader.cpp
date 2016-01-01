/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcHttpRequHeader.cpp
 * @brief    Implementation of Class CcHttpRequHeader
 */
#include "CcHttpRequHeader.h"
#include "CcKernel.h"

CcHttpRequHeader::CcHttpRequHeader(CcString Parse)
{
  parse(Parse);
}


CcHttpRequHeader::CcHttpRequHeader(bool init)
{
  m_Accept         = "";
  m_AcceptCharset  = "Accept-Charset: utf-8";
  m_AcceptEncoding = "Accept-Encoding: text,deflate";
  m_AcceptLanguage = "";
  m_Authorization  = "";
  m_CacheControl   = "Cache-Control: no-cache";
  m_Connection     = "Connection: close";
  m_Cookie         = "";
  m_ContentLength  = "";
  m_ContentType    = "Content-Type: text/html";
  m_Host           = "";
  m_Range          = "";
  m_Referer        = "";
  m_TransferEncoding = "";
  //m_UserAgent      = "CcOS Http-Client";
  m_UserAgent      = "Mozilla / 5.0 (Windows NT 10.0; WOW64) AppleWebKit / 537.36 (KHTML, like Gecko) Chrome / 47.0.2526.106 Safari / 537.36";
}

CcHttpRequHeader::~CcHttpRequHeader( void )
{
}

CcString CcHttpRequHeader::getHeader(void){
  CcString Header;
  if (m_Request.length())
    Header += m_Request + "\r\n";
  if(m_Accept.length())
    Header += CcString() + "Accept: " + m_Accept + "\r\n";
  if(m_AcceptCharset.length())
    Header += CcString() + "Accept-Charset: " + m_AcceptCharset + "\r\n";
  if (m_AcceptEncoding.length())
    Header += CcString() + "Accept-Encoding: " + m_AcceptEncoding + "\r\n";
  if (m_AcceptLanguage.length())
    Header += CcString() + "Accept-Language: " + m_AcceptLanguage + "\r\n";
  if (m_Authorization.length())
    Header += CcString() + "Authorization: " + m_Authorization + "\r\n";
  if (m_CacheControl.length())
    Header += CcString() + "Cach-Control: " + m_CacheControl + "\r\n";
  if (m_Connection.length())
    Header += CcString() + "Connection: " + m_Connection + "\r\n";
  if (m_Cookie.length())
    Header += CcString() + "Cookie: " + m_Cookie + "\r\n";
  if (m_ContentLength.length())
    Header += CcString() + "Content-Length: " + m_ContentLength + "\r\n";
  if (m_ContentType.length())
    Header += CcString() + "Content-Type: " + m_ContentType + "\r\n";
  if (m_Host.length())
    Header += CcString() + "Host: " + m_Host + "\r\n";
  if (m_Range.length())
    Header += CcString() + "Range: " + m_Range + "\r\n";
  if (m_Referer.length())
    Header += CcString() + "Referer: " + m_Referer + "\r\n";
  if (m_TransferEncoding.length())
    Header += CcString() + "TransferEncoding: " + m_TransferEncoding + "\r\n";
  if (m_UserAgent.length())
    Header += CcString() + "User-Agent: " + m_UserAgent + "\r\n";
  Header += "\r\n";
  return Header;
}

void CcHttpRequHeader::parse(CcString &Parse){
  CcString sLine = Parse.getLine();
  while (sLine != ""){
    parseLine(sLine);
    sLine = Parse.getLine();
  }
}

void CcHttpRequHeader::parseLine(CcString &Parse){
  size_t pos = Parse.contains(":");
  if (pos < Parse.length()){
    CcString sArgument = Parse.substr(0, pos);
    CcString sValue = Parse.substr(pos + 1).trim();
    if (sArgument == "Accept")
      m_Accept = sValue;
    else if (sArgument == "Accept-Charset")
      m_AcceptCharset = sValue;
    else if (sArgument == "Accept-Encoding")
      m_AcceptEncoding = sValue;
    else if (sArgument == "Accept-Language")
      m_AcceptLanguage = sValue;
    else if (sArgument == "Authorization")
      m_Authorization = sValue;
    else if (sArgument == "Cache-Control")
      m_CacheControl = sValue;
    else if (sArgument == "Connection")
      m_Connection = sValue;
    else if (sArgument == "Cookie")
      m_Cookie = sValue;
    else if (sArgument == "Content-Length")
      m_ContentLength = sValue;
    else if (sArgument == "Content-Type")
      m_ContentType = sValue;
    else if (sArgument == "Host")
      m_Host = sValue;
    else if (sArgument == "Range")
      m_Range = sValue;
    else if (sArgument == "Request")
      m_Request = sValue;
    else if (sArgument == "Referer")
      m_Referer = sValue;
    else if (sArgument == "Transfer-Encoding")
      m_TransferEncoding = sValue;
    else if (sArgument == "User-Agent")
      m_UserAgent = sValue;
  }
  else if (SIZE_MAX != Parse.contains("HTTP")){
    size_t posSpace = Parse.contains(" ");
    if (posSpace != SIZE_MAX){
      m_HTTPMethod = Parse.substr(0, posSpace);
      m_HTTPTarget = Parse.getStringBetween(" ", " ", posSpace + m_HTTPMethod.length());
    }
  }
}


void CcHttpRequHeader::setHost(CcString Host){
  m_Host = Host;
}
void CcHttpRequHeader::setUserAgent(CcString Agent){
  m_UserAgent = Agent;
}

void CcHttpRequHeader::setRquest(CcString Request){
  m_Request = Request;
}

void CcHttpRequHeader::setContentType(uint8 flags, CcString additional){
  switch (flags)
  {
    case HTTP_GET:
      m_ContentType = "text/plain";
      break;
    case HTTP_HEAD:
      m_ContentType = "";
      break;
    case HTTP_POST_MULTIP:
      m_ContentType = "multipart/form-data; ";
      m_ContentType.append(additional);
      break;
    case HTTP_POST_URLENC:
      m_ContentType = "application/x-www-form-urlencoded";
      break;
  }
}

void CcHttpRequHeader::setContentSize(size_t size){
  m_ContentLength.clear();
  m_ContentLength.appendNumber(size);
}
