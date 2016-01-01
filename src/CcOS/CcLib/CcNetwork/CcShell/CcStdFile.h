/*
 * CcStdFile.h
 *
 *  Created on: 13.09.2015
 *      Author: Andreas
 */

#ifndef CcStdFile_H_
#define CcStdFile_H_

#include "CcBase.h"
#include "CcString.h"
#include "CcApp.h"
#include "CcFile.h"

class CcStdFile: public CcApp {
public:
  CcStdFile( FILE* stdFile );
  virtual ~CcStdFile();


  virtual size_t size(void);
  virtual size_t read(char* buffer, size_t size);
  virtual size_t write(char* buffer, size_t size);
  virtual bool open(uint16 flags);
  virtual bool close();
  virtual bool setFilePointer(size_t pos);


  size_t write(CcCharArray &charArray, size_t offset = 0, size_t len = SIZE_MAX);
  size_t read(CcCharArray &charArray, size_t offset = 0, size_t len = SIZE_MAX);

private:
  FILE *m_File;
};

#endif /* CcStdFile_H_ */
