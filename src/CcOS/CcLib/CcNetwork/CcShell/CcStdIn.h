/*
 * CcStdIn.h
 *
 *  Created on: 13.09.2015
 *      Author: Andreas
 */

#ifndef CcStdIn_H_
#define CcStdIn_H_

#include "CcBase.h"
#include "CcFile.h"

class CcStdIn: public CcFile {
public:
  CcStdIn( void );
  virtual ~CcStdIn();


  virtual size_t size(void);
  virtual size_t read(char* buffer, size_t size);
  virtual size_t write(char* buffer, size_t size);
  virtual bool open(uint16 flags);
  virtual bool close();
  virtual bool setFilePointer(size_t pos);


  size_t write(CcCharArray &charArray, size_t offset = 0, size_t len = SIZE_MAX);
  size_t read(CcCharArray &charArray, size_t offset = 0, size_t len = SIZE_MAX);
};

#endif /* CcStdIn_H_ */
