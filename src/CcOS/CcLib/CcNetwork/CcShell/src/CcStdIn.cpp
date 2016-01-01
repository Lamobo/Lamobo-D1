/*
* CcStdIn.cpp
*
*  Created on: 13.09.2015
*      Author: Andreas
*/

#include "CcStdIn.h"
#include "stdio.h"

CcStdIn::CcStdIn(void)
{
}

CcStdIn::~CcStdIn() {

}
size_t CcStdIn::size(void){
  return SIZE_MAX;
}

size_t CcStdIn::read(char* buffer, size_t size){
  size_t iRet = 0;
  if (fgets(buffer, size, stdin) != NULL){
    iRet = strlen(buffer);
  }
  return iRet;
}

size_t CcStdIn::write(char* buffer, size_t size){
  CC_UNUSED(buffer);
  CC_UNUSED(size);
  return 0;
}

bool CcStdIn::open(uint16 flags){
  CC_UNUSED(flags);
  return false;
}

bool CcStdIn::close(){
  return false;
}

bool CcStdIn::setFilePointer(size_t pos){
  CC_UNUSED(pos);
  return false;
}

