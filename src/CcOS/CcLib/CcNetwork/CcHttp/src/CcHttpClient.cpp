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
 * @file     CcHttpClient.cpp
 * @brief    Implementation of Class CcHttpClient
 */
#include "CcHttpClient.h"
#include "CcKernel.h"
#include "CcHttpRespHeader.h"

CcHttpClient::CcHttpClient(void) :
m_Socket(0),
m_Done(false),
m_Output(0)
{
  m_WD = Kernel.getFileSystem()->getWorkingDir();
}

CcHttpClient::~CcHttpClient( void ) {
}

bool CcHttpClient::execGet(CcUrl url){
  m_Url = url;
  bool bRet = false;
  if (url.isUrl())
  {
    CcString httpRequest = CcString() + "GET " + url.getPath() + " HTTP/1.1\n";
    m_Header.setHost(url.getHostname());
    m_Header.setRquest(httpRequest);
    httpRequest = m_Header.getHeader();
    connectSocket();
    m_Socket->write(httpRequest.getCharString(), httpRequest.length());
    Kernel.createThread(this);
    while (!isDone());
    if (m_RespHeader.m_HTTP.toUint16() == 200){
      bRet = true;
    }
  }
  return bRet;
}

bool CcHttpClient::execHead(CcUrl url){
  m_Url = url;
  bool bRet = false;
  if (url.isUrl())
  {
    CcString httpRequest = CcString() + "HEAD " + url.getPath() + " HTTP/1.1\n";
    m_Header.setHost(url.getHostname());
    m_Header.setRquest(httpRequest);
    httpRequest = m_Header.getHeader();
    connectSocket();
    m_Socket->write(httpRequest.getCharString(), httpRequest.length());
    size_t rec;
    char receive[1024];
    rec = m_Socket->read(receive, 1024);
    while (rec){
      m_Buffer.append(receive, rec);
      rec = m_Socket->read(receive, 1024);
    }
    CcCharArray toFind("\n\n");
    rec = m_Buffer.contains(toFind);
    if (rec == SIZE_MAX)
    {
      CcCharArray toFind("\r\n\r\n");
      rec = m_Buffer.contains(toFind);
    }
    if (rec != SIZE_MAX)
    {
      CcString HeaderString;
      HeaderString.append(m_Buffer, 0, rec);
      m_Buffer.deleteAt(0, rec + 2);
      if (m_Buffer.at(0) == '\r')
        m_Buffer.deleteAt(0, 2);
      m_RespHeader.parse(HeaderString);
      size_t contentLength = m_RespHeader.m_ContentLength.toUint32();
      if (contentLength >m_Buffer.size() && contentLength > 0)
      {
        m_Buffer.deleteAt(contentLength, m_Buffer.size() - contentLength);
      }
    }
    if (m_RespHeader.m_HTTP.toUint16() == 200){
      bRet = true;
    }
  }
  return bRet;
}

bool CcHttpClient::execPost(CcUrl url, CcVector<CcStringPair> *PostData){
  CcString httpRequest = CcString() + "POST " + url.getPath() + " HTTP/1.1";
  m_Header.setHost(url.getHostname());
  m_Header.setRquest(httpRequest);
  CcString BoundaryName = "----------ABCDEFG";
  CcString BoundaryNameBegin = CcString() + "--" + BoundaryName + "\r\n";
  CcString BoundaryNameEnd = CcString() + "--" + BoundaryName + "--\r\n";
  m_Header.setContentType(HTTP_POST_MULTIP, CcString() + "boundary=" + BoundaryName);
  return false;
}

bool CcHttpClient::execPostMultip(CcUrl url, CcVector<CcStringPair> *PostData, CcVector<CcStringPair> *files){
  CcString httpRequest = CcString() + "POST " + url.getPath() + " HTTP/1.1";
  m_Header.setHost(url.getHostname());
  m_Header.setRquest(httpRequest);
  CcString BoundaryName = "----------ABCDEFG";
  CcString BoundaryNameBegin = CcString() + "--" + BoundaryName + "\r\n";
  CcString BoundaryNameEnd = CcString() + "--" + BoundaryName + "--\r\n";
  m_Header.setContentType(HTTP_POST_MULTIP, CcString() + "boundary=" + BoundaryName);
  size_t contentSize = 0;
  CcStringList fileContent;
  CcStringList postParam;
  for (size_t i = 0; i < files->size(); i++){
    CcFile file(m_WD.calcPathAppend(files->at(i).value));
    contentSize += file.size();
    CcString sFileContent = BoundaryNameBegin;
    sFileContent.append("Content-Disposition: form-data; filename=\"");
    CcString temp = files->at(i).value.extractFilename();
    sFileContent.append(temp);
    sFileContent.append("\"; ");
    sFileContent.append("name=\"");
    sFileContent.append(files->at(i).name);
    sFileContent.append("\"\r\n");
    sFileContent.append("Content-Type: application/octet-stream\r\n\r\n");
    fileContent.append(sFileContent);
    contentSize += sFileContent.length()+2;
  }
  for (size_t i = 0; PostData!= 0 && i < PostData->size(); i++){
    CcString sParam = BoundaryNameBegin;
    sParam.append("Content-Disposition: form-data; name=\"");
    sParam.append(PostData->at(i).name);
    sParam.append("\"\r\n\r\n");
    sParam.append(PostData->at(i).value);
    postParam.append(sParam);
    contentSize += sParam.length()+2;
  }
  postParam.append(BoundaryNameEnd);
  contentSize += BoundaryNameEnd.length();
  m_Header.setContentSize(contentSize);
  CcString Header = m_Header.getHeader();
  if(connectSocket()){
    if (m_Socket->connect(url.getHostname(), url.getPort())){
      m_Socket->write(Header.getCharString(), Header.length());
    }
    for (size_t i = 0; i < files->size(); i++){
      m_Socket->write(fileContent.at(i).getCharString(), fileContent.at(i).length());
      bool bDone = false;
      CcFile file(m_WD.calcPathAppend(files->at(i).value));
      if (file.isFile() && file.open(Open_Read)){
        size_t read, readLeft;
        char buf[1024];
        while (!bDone){
          read = file.read(buf, 1024);
          if (read != SIZE_MAX && read != 0)
          {
            readLeft = m_Socket->write(buf, read);
            while ((read != SIZE_MAX) && (readLeft != read))
              readLeft += m_Socket->write(buf, read - readLeft);
            if (readLeft == SIZE_MAX){
              bDone = true;
            }
          }
          else if (read == SIZE_MAX){
            bDone = true;
          }
          else{
            bDone = true;
          }
        }
        file.close();
      }
      m_Socket->write("\r\n", 2);
    }
    size_t read;
    for (size_t i = 0; i < postParam.size(); i++){
      m_Socket->write(postParam.at(i).getCharString(), postParam.at(i).length());
      m_Socket->write("\r\n", 2);
    }
    bool bDone = false;
    char buf[1024];
    while (!bDone){
      read = m_Socket->read(buf, 1024);
      if (m_Output != 0){
        m_Output->write(buf, read);
      }
      if (read == SIZE_MAX || read == 0)
      {
        bDone = true;
      }
    }
    m_Socket->close();
  }
  return false;
}

bool CcHttpClient::isDone(void){
  return m_Done;
}

void CcHttpClient::setOutputDevice(CcIODevice*output){
  m_Output = output;
}

CcCharArray* CcHttpClient::getCharArray(void){
  return &m_Buffer;
}

void CcHttpClient::run(void){
}

bool CcHttpClient::connectSocket(void){
  m_Socket = Kernel.getSocket(eTCP);
  if(m_Socket)
    return m_Socket->connect(m_Url.getHostname(), m_Url.getPort());
  return 0;
}