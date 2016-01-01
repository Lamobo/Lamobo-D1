/*
* CcStdFile.cpp
*
*  Created on: 13.09.2015
*      Author: Andreas
*/

#include "CcStdFile.h"
#include "CcKernel.h"
#include "CcThreadObject.h"
#include "CcCharArray.h"
#include "stdio.h"

CcStdFile::CcStdFile(FILE * stdFile) :
m_File(stdFile)
{
}

CcStdFile::~CcStdFile() {

}
size_t CcStdFile::size(void){
  return SIZE_MAX;
}

size_t CcStdFile::read(char* buffer, size_t size){
  return fread(buffer, size, sizeof(char), m_File);
}

size_t CcStdFile::write(char* buffer, size_t size){
  return fwrite(buffer, size, sizeof(char), m_File);
}

bool CcStdFile::open(uint16 flags){
  CC_UNUSED(flags);
  return false;
}

bool CcStdFile::close(){
  if (fclose(m_File))
    return false;
  return true;
}

bool CcStdFile::setFilePointer(size_t pos){
  /*if (lseek(m_File, &pos))
    return false;*/
  return true;
}

