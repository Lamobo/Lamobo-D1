/*
* CcShell.cpp
*
*  Created on: 13.09.2015
*      Author: Andreas
*/

#include "CcShell.h"
#include "CcKernel.h"
#include "CcThreadObject.h"
#include "CcCharArray.h"
#include "CcKernel.h"

CcShell::CcShell(CcIODevice *in, CcIODevice *out) :
m_Input(in),
m_Output(out)
{
}

CcShell::~CcShell() {

}

void CcShell::run(void) {
  char inBuf[256];
  size_t readSize;
  CcCharArray inData;
  if (m_WD == "")
    setWorkingDir(Kernel.getFileSystem()->getWorkingDir());
  /*m_Output->write("Login: ", 7);
  if (SIZE_MAX != (readSize = m_Input->read(inBuf, 256))){
    inData.append(inBuf, readSize);
    m_Output->write("Password: ", 10);
    if (SIZE_MAX != (readSize = m_Input->read(inBuf, 256))){
      inData.append(inBuf, readSize);
    }
  }*/
  m_Output->write(m_WD.getCharString(), m_WD.length());
  while (getThreadState() == CCTHREAD_RUNNING){
    while (SIZE_MAX != (readSize = m_Input->read(inBuf, 256))){
      inData.append(inBuf, readSize);
      m_Output->write("\r\n", 2);
      CcString line;
      line.append(inData, 0, readSize);
      inData.clear();
      parseLine(line.trim());
      m_Output->write(m_WD.getCharString(), m_WD.length());
    }
  }
}

void CcShell::parseLine(CcString &str){
  CcProcess process;
  process.m_Name = process.m_Arguments.parseArguments(str);
  process.m_Input = m_Input;
  process.m_Output = m_Output;
  Kernel.createProcess(process);
}

void CcShell::addApp(CcApp* pApp, CcString Name){

}

void CcShell::setWorkingDir(CcString path){
  m_WD = path + " $ ";
}
