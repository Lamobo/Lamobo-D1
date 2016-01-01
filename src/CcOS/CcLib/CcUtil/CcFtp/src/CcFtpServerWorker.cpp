/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcFtpServerWorker.cpp
 * @brief    Implementation of Class CcFtpServerWorker
 *           Protocol: http://www.w3.org/Protocols/rfc2616/rfc2616-sec14.html
 */
#include "CcKernel.h"
#include "CcThreadObject.h"
#include "CcFtpServerWorker.h"
#include "CcFtpTypes.h"
#include "stdio.h"

CcFtpServerWorker::CcFtpServerWorker(CcSocket *socket) :
  m_Socket(socket),
  m_DataPortInc(12378)
{
  m_WD = Kernel.getFileSystem()->getWorkingDir();
}

CcFtpServerWorker::~CcFtpServerWorker(void){
  if (m_Socket != 0)
    delete m_Socket;
}

void CcFtpServerWorker::run(){
  char recBuf[1024];
  size_t recSize = 0;
  CcString recStr;
  if (m_Socket != 0)
  {
    recSize = m_Socket->write((char*)"220 FTP-Server ready\r\n", 22);
    if (recSize != 0 && recSize != SIZE_MAX){
      recSize = m_Socket->read(recBuf, 1024);
      while (recSize != SIZE_MAX && recSize != 0){
        recStr.clear();
        recStr.append(recBuf, recSize);
        parseCommand(recStr);
        recSize = m_Socket->read(recBuf, 1024);
      }
      Kernel.delayMs(1);
    }
  }
}

void CcFtpServerWorker::parseCommand(CcString &Command){
  CcString param;
  Command.trim();
  size_t posParam = Command.contains(" ");
  if (posParam != SIZE_MAX){
    param = Command.substr(posParam+1);
    Command = Command.substr(0, posParam);
  }
  eFtpCommands eCmd = FTP_UNKNOWN;
  for (size_t i=0; i < sFtpCommandListSize && eCmd == FTP_UNKNOWN; i++){
    if (Command == sFtpCommandList[i].strCommand){
      eCmd = sFtpCommandList[i].eCommand;
    }
  }
  switch (eCmd)
  {
  case FTP_UNKNOWN:
    m_Socket->write((char*)FTP_501, sizeof(FTP_501));
    break;
  case FTP_OPTS:
    m_Socket->write((char*)"200\r\n", 5);
    break;
  case FTP_USER:
    m_Socket->write((char*)"331 Password required\r\n", 23);
    break;
  case FTP_PASS:
    m_Socket->write((char*)"230 User logged in\r\n", 20);
    break;
  case FTP_AUTH:
    m_Socket->write((char*)"500 Syntax error\r\n", 18);
    break;
  case FTP_SYST:
    m_Socket->write((char*)"215 UNIX Type: L8\r\n", 19);
    break;
  case FTP_FEAT:
    m_Socket->write((char*)"211 \r\n", 6);
    m_Socket->write((char*)"SIZE\r\n", 6);
    m_Socket->write((char*)"211 END\r\n", 6);
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
    if (param == "I"){
      m_TransferType = false;
      m_Socket->write((char*)"200 Type set to I\r\n", 19);
    }
    else if (param == "A"){
      m_TransferType = true;
      m_Socket->write((char*)"200 Type set to A\r\n", 19);
    }
    break;
  case FTP_PASV:{
    m_Active = false;
    m_Socket->write((char*)FTP_501, sizeof(FTP_501));
    /**
    if ((m_DataSocket = Kernel.getSocket(eTCP)) != 0)
    {
      while(true != m_DataSocket->bind({ 127, 0, 0, 1 }, m_DataPortInc)){
          if (m_DataPortInc > 20000) m_DataPortInc = 12378;
          else m_DataPortInc++;
      }
      m_DataSocket->listen();
      CcString sRet((char*)"227 Entering Passive Mode (127,0,0,1,");
      uint8 tempPort = ((m_DataPortInc) & 0xff00) >> 8;
      sRet += CcString().appendNumber(tempPort);
      tempPort = (m_DataPortInc) & 0xff;
      sRet += CcString(",") + CcString().appendNumber(tempPort);
      sRet.append(").\r\n");
      m_Socket->write(sRet.getCharString(), sRet.length());
      if (m_DataPortInc > 20000) m_DataPortInc = 12378;
      else m_DataPortInc++;
    }*/
    break;
  }
  case FTP_LIST:
  {
    m_Socket->write((char*)"150 Opening ASCII mode data connection for /bin/ls \r\n", 53);
    acceptDataConnection();
    CcFile dir(m_WD);
    CcStringList slFiles;
    if (dir.isDir()){
      if (param == "-a")
        slFiles = dir.getFileList(SHOW_EXTENDED | SHOW_HIDDEN);
      else if (param == "-l")
        slFiles = dir.getFileList(SHOW_EXTENDED);
      else
        slFiles = dir.getFileList(SHOW_EXTENDED);
      for (size_t i = 0; i < slFiles.size(); i++){
        m_DataSocket->write(slFiles.at(i).getCharString(), slFiles.at(i).length());
      }
      m_DataSocket->close();
      delete m_DataSocket;
      m_Socket->write((char*)FTP_226, sizeof(FTP_226));
    }
    else{
      m_Socket->write((char*)FTP_500, sizeof(FTP_500));
    }
    break;
  }
  case FTP_NLST:
  {
    m_Socket->write((char*)"150 Opening ASCII mode data connection for /bin/ls \r\n", 53);
    acceptDataConnection();
    CcFile dir(m_WD);
    if (dir.isDir()){
      CcStringList slFiles = dir.getFileList(false);
      for (size_t i = 0; i < slFiles.size(); i++){
        m_DataSocket->write(slFiles.at(i).getCharString(), slFiles.at(i).length());
      }
      m_DataSocket->close();
      delete m_DataSocket;
      m_Socket->write((char*)"226 Transfer complete. \r\n", 24);
    }
    else{
      m_Socket->write((char*)FTP_500, sizeof(FTP_500));
    }
    break;
  }
  case FTP_CWD:
  {
    CcString sTemp = produceAbsolutPath(param);
    CcFile dir(sTemp);
    if (dir.isDir()){
      m_WD = sTemp;
      m_Socket->write((char*)"250 CWD command successful.\r\n", 29);
    }
    else{
      CcString resp( CcString() + "550 " + sTemp + " No such file or directory.\r\n");
      m_Socket->write(resp.getCharString(), resp.length());
    }
    break;
  }
  case FTP_CDUP:
  {
    CcString sTemp;
    size_t pos = m_WD.findLast("/");
    sTemp = m_WD.substr(0, pos);
    CcFile *dir = Kernel.getFileSystem()->getFile(sTemp);
    if (dir->isDir()){
      m_WD = sTemp;
      m_Socket->write((char*)"250 CDUP command successful.\r\n", 29);
    }
    else{
      CcString resp(CcString() + "550 " + sTemp + " No such file or directory.\r\n");
      m_Socket->write(resp.getCharString(), resp.length());
    }
    break;
  }
  case FTP_RETR:
  {
    bool bDone = false;
    CcString sRet;
    CcString sTemp = produceAbsolutPath(param);
    CcFile file(sTemp);
    if (file.isFile() && file.open(Open_Read)){
      m_Socket->write((char*)"150 Opening ASCII mode data connection for /bin/ls \r\n", 53);
      if(acceptDataConnection()){
        size_t read, readLeft;
        char buf[1024];
        while (!bDone){
          read = file.read(buf, 1024);
          if (read != SIZE_MAX && read != 0)
          {
            readLeft = m_DataSocket->write(buf, read);
            while ((read != SIZE_MAX) && (readLeft != read))
              readLeft += m_DataSocket->write(buf, read - readLeft);
            if (readLeft == SIZE_MAX){
              bDone = true;
              sRet = "426 Error occured on connection file \r\n";
            }
          }
          else if (read == SIZE_MAX){
            bDone = true;
            sRet = (char*)"551 Error reading file from disk \r\n";
          }
          else{
            bDone = true;
            sRet = "226 Transfer complete. \r\n";
          }
        }
        file.close();
        m_DataSocket->close();
        delete m_DataSocket;
      }
    }
    else{
      sRet = "551 File not exists \r\n";
    }
    m_Socket->write(sRet.getCharString(), sRet.length());
    break;
  }
  case FTP_RNFR:
  {
    CcString sTemp = produceAbsolutPath(param);
    m_Socket->write((char*)FTP_200, sizeof(FTP_200));
    break;
  }
  case FTP_RNTO:
  {
    CcString sTemp;
    param.strReplace("\\", "/");
    if (param.begins("/"))
      sTemp = param.strReplace("\\", "/");
    else if (param.begins("./"))
      sTemp = m_WD + "/" + sTemp.substr(2);
    else if (param.at(1) == ':') //WindowsDrive
      sTemp = param.strReplace("\\", "/");
    else
      sTemp = m_WD + "/" + param;
    CcFile file(m_Temp);
    if (file.isDir()){
      if (file.move(sTemp))
        m_Socket->write((char*)FTP_200, sizeof(FTP_200));
      else
        m_Socket->write((char*)FTP_500, sizeof(FTP_500));
    }
    else if (file.isFile()){
      if (file.move(sTemp))
        m_Socket->write((char*)FTP_200, sizeof(FTP_200));
      else
        m_Socket->write((char*)FTP_500, sizeof(FTP_500));
    }
    break;
  }
  case FTP_STOR:
  {
    bool bDone = false;
    CcString sRet;
    param.trim();
    CcString sTemp = m_WD.calcPathAppend(param);
    CcFile file(sTemp);
    if (file.open(Open_Write | Open_Overwrite)){
      m_Socket->write((char*)"150 Opening ASCII mode data connection for /bin/ls \r\n", 53);
      if(acceptDataConnection())
      {
        size_t read, readLeft;
        char buf[1024];
        while (!bDone){
          read = m_DataSocket->read(buf, 1024);
          if (read != SIZE_MAX && read != 0)
          {
            readLeft = file.write(buf, read);
            while ((read != SIZE_MAX) && (readLeft != read))
              readLeft += file.write(buf, read - readLeft);
            if (readLeft == SIZE_MAX){
              bDone = true;
              sRet = "426 Error occured on connection file \r\n";
            }
          }
          else if (read == SIZE_MAX){
            bDone = true;
            sRet = "551 Error reading fiel from socket \r\n";
          }
          else{
            bDone = true;
            sRet = "226 Transfer complete. \r\n";
          }
        }
        file.close();
        m_DataSocket->close();
        delete m_DataSocket;
      }
    }
    else{
      sRet = FTP_550;
    }
    m_Socket->write(sRet.getCharString(), sRet.length());
    break;
  }
  case FTP_MKD:
  {
    CcString sTemp = produceAbsolutPath(param);
    if (Kernel.getFileSystem()->mkdir(sTemp)){
      sTemp = CcString() + "257 \"" + sTemp + "\" created\r\n";
      m_Socket->write(sTemp.getCharString(), sTemp.length());
    }
    else
      m_Socket->write((char*)FTP_550, sizeof(FTP_550));
    break;
  }
  case FTP_DELE:
  {
    CcString sTemp = produceAbsolutPath(param);
    if (Kernel.getFileSystem()->del(sTemp)){
      sTemp = CcString() + "257 \"" + sTemp + "\" created\r\n";
      m_Socket->write((char*)FTP_250, sizeof(FTP_250));
    }
    else
      m_Socket->write((char*)FTP_550, sizeof(FTP_550));
    break;
  }
  case FTP_RMD:
  {
    CcString sTemp = produceAbsolutPath(param);
    if (Kernel.getFileSystem()->del(sTemp)){
      sTemp = CcString() + "257 \"" + sTemp + "\" created\r\n";
      m_Socket->write((char*)FTP_250, sizeof(FTP_250));
    }
    else
      m_Socket->write((char*)FTP_550, sizeof(FTP_550));
    break;
  }
  case FTP_PORT:{
    m_Active = true;
    CcStringList ipList;
    param.trim();
    ipList.splitString(param, ',');
    if(ipList.size() == 6){
      m_PasvIP.ip4 = ipList.at(0).toUint16();
      m_PasvIP.ip3 = ipList.at(1).toUint16();
      m_PasvIP.ip2 = ipList.at(2).toUint16();
      m_PasvIP.ip1 = ipList.at(3).toUint16();
      m_PasvPort  = ipList.at(4).toUint16()*0x0100;
      m_PasvPort += ipList.at(5).toUint16();
      m_Socket->write((char*)"200 PORT command successfull\r\n", 30);
    }
    else{
      m_Socket->write((char*)FTP_501, sizeof(FTP_501));
    }
    break;
  }
  case FTP_SIZE:{
    CcString sTemp = produceAbsolutPath(param);
    CcFile file(sTemp);
    if (file.isFile()){
      size_t size = file.size();
      sTemp = CcString() + "213 " + CcString().appendNumber(size) + "\r\n";
    }
    else
      sTemp = FTP_550;
    m_Socket->write(sTemp.getCharString(), sTemp.length());
    break;
  }
  case FTP_MDTM:{
    CcString sTemp = produceAbsolutPath(param);
    CcFile file(sTemp);
    if (file.isFile()){
      tm tmTime = file.getLastModified();
      char buf[15];
      sprintf(buf, "%04d%02d%02d%02d%02d%02d", tmTime.tm_year, tmTime.tm_mon, tmTime.tm_mday, tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);
      buf[14] = '0';
      sTemp = CcString() + "213 " + buf + "\r\n";
    }
    else
      sTemp = FTP_550;
    m_Socket->write(sTemp.getCharString(), sTemp.length());
    break;
  }
  default:
    break;
  }
}

bool CcFtpServerWorker::acceptDataConnection(void){
  CcSocket *temp;
  if (m_Active != true){
    temp = m_DataSocket->accept();
    m_DataSocket->close();
    delete m_DataSocket;
    m_DataSocket = temp;
  }
  else{
    m_DataSocket = Kernel.getSocket(eTCP);
    m_DataSocket->connect(m_PasvIP, m_PasvPort);
  }
  if (m_DataSocket != 0){
    return true;
  }
  return false;
}

CcString CcFtpServerWorker::produceAbsolutPath(CcString &input){
  CcString m_Temp;
  input.strReplace("\\", "/");
  if (input.begins("/"))
    m_Temp = input.strReplace("\\", "/");
  else if (input.begins("./"))
    m_Temp = m_WD + "/" + input.substr(2);
  else if (input.at(1) == ':') //WindowsDrive
    m_Temp = input.strReplace("\\", "/");
  else
    m_Temp = m_WD + "/" + input;
  return m_Temp;
}
