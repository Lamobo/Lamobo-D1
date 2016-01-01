/*
* CcStdErr.cpp
*
*  Created on: 13.09.2015
*      Author: Andreas
*/

#include "CcStdErr.h"
#include "stdio.h"

CcStdErr::CcStdErr(void)
{
}

CcStdErr::~CcStdErr() {

}
size_t CcStdErr::size(void){
  return SIZE_MAX;
}

size_t CcStdErr::read(char* buffer, size_t size){
  CC_UNUSED(buffer);
  CC_UNUSED(size);
  return 0;
}

size_t CcStdErr::write(char* buffer, size_t size){
  fprintf(stderr, "%.*s\n", size, buffer);
  return size;
}

bool CcStdErr::open(uint16 flags){
  CC_UNUSED(flags);
  return false;
}

bool CcStdErr::close(){
  return false;
}

bool CcStdErr::setFilePointer(size_t pos){
  CC_UNUSED(pos);
  return false;
}

