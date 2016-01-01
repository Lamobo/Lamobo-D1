/*
* CcStdOut.cpp
*
*  Created on: 13.09.2015
*      Author: Andreas
*/

#include "CcStdOut.h"
#include "stdio.h"

CcStdOut::CcStdOut(void)
{
}

CcStdOut::~CcStdOut() {

}
size_t CcStdOut::size(void){
  return SIZE_MAX;
}

size_t CcStdOut::read(char* buffer, size_t size){
  CC_UNUSED(buffer);
  CC_UNUSED(size);
  return 0;
}

size_t CcStdOut::write(char* buffer, size_t size){
  printf("%.*s", size, buffer);
  return size;
}

bool CcStdOut::open(uint16 flags){
  CC_UNUSED(flags);
  return false;
}

bool CcStdOut::close(){
  return false;
}

bool CcStdOut::setFilePointer(size_t pos){
  CC_UNUSED(pos);
  return false;
}

