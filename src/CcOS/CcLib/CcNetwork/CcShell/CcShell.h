/*
 * CcShell.h
 *
 *  Created on: 13.09.2015
 *      Author: Andreas
 */

#ifndef CCSHELL_H_
#define CCSHELL_H_

#include "CcBase.h"
#include "CcString.h"
#include "CcApp.h"
#include "CcFile.h"

class CcShell: public CcApp {
public:
  CcShell(CcIODevice*in, CcIODevice *out);
  virtual ~CcShell();

  void run(void);

  void parseLine(CcString &line);

  void addApp(CcApp* pApp, CcString Name);

  void setWorkingDir(CcString path);

private:
  CcIODevice *m_Input;
  CcIODevice *m_Output;
  CcString m_WD;
};

#endif /* CCSHELL_H_ */
