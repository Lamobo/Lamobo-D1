/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcFtpServerPasv.cpp
 * @brief    Implementation of Class CcFtpServerPasv
 *           Protocol: http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
 */
#include "CcKernel.h"
#include "CcThreadObject.h"
#include "CcFtpServerPasv.h"
#include "CcFtpTypes.h"

CcFtpServerPasv::CcFtpServerPasv(CcSocket *socket) :
  m_Socket(socket)
{
  m_WD = Kernel.getFileSystem()->getWorkingDir();
}

CcFtpServerPasv::~CcFtpServerPasv(void){
  if (m_Socket != 0)
    delete m_Socket;
}

void CcFtpServerPasv::run(){
  char recBuf[1024];
  size_t recSize = 0;
  CcString recStr;
  CcSocket *temp;
  if (m_Socket != 0)
  {
    temp = m_Socket->accept();
    m_Socket->close();
    m_Socket = temp;
    recSize = m_Socket->read(recBuf, 1024);
    while (recSize != SIZE_MAX && recSize != 0){
      recStr.clear();
      recStr.append(recBuf, recSize);
      parseCommand(recStr);
      recSize = m_Socket->read(recBuf, 1024);
    }
  }
}

void CcFtpServerPasv::parseCommand(CcString &Command){
  CcStringList strList;
  Command = strList.parseArguments(Command);
  eFtpCommands eCmd = FTP_UNKNOWN;
  for (size_t i=0; i < sFtpCommandListSize && eCmd == FTP_UNKNOWN; i++){
    if (Command == sFtpCommandList[i].strCommand){
      eCmd = sFtpCommandList[i].eCommand;
    }
  }
  switch (eCmd)
  {
  case FTP_UNKNOWN:
    m_Socket->write("501\r\n", 5);
    break;
  case FTP_OPTS:
    m_Socket->write("200\r\n", 5);
    break;
  case FTP_USER:
    m_Socket->write("331 Password required\r\n", 23);
    break;
  case FTP_PASS:
    m_Socket->write("230 User logged in\r\n", 20);
    break;
  case FTP_AUTH:
    m_Socket->write("500 Syntax error\r\n", 18);
    break;
  case FTP_SYST:
    m_Socket->write("215 UNIX Type: L8\r\n", 19);
    break;
  case FTP_FEAT:
    m_Socket->write("211 \r\n", 6);
    break;
  case FTP_PWD:
  {
    CcString pwd("257 \"");
    pwd.append(m_WD);
    pwd.append("\" \r\n");
    m_Socket->write(pwd.getCharString(), pwd.length());
  }
  break;
  case FTP_TYPE:
    if (strList.at(0) == "I"){
      m_TransferType = false;
      m_Socket->write("200 Type set to I\r\n", 19);
    }
    else if (strList.at(0) == "A"){
      m_TransferType = true;
      m_Socket->write("200 Type set to A\r\n", 19);
    }
    break;
  case FTP_LIST:
  {
    m_Socket->write("150\r\n", 5);
    m_Socket->write("public_html\r\n", 13);
    m_Socket->write("public_htm1\r\n", 13);
    m_Socket->write("public_htm2\r\n", 13);
    m_Socket->write("public_htm3\r\n", 13);
    m_Socket->write("226 transfer complete\r\n", 23);
    break;
  }
  default:
    break;
  }
}
