/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     WindowsService.h
 * @brief    Class WindowsService
 **/
#ifndef WINDOWSSERVICE_H_
#define WINDOWSSERVICE_H_

#include "CcBase.h"
#include "WindowsGlobals.h"
#include <winsvc.h>
#include "CcStringWin.h"

class WindowsService {
public: //methods
  WindowsService();
  virtual ~WindowsService();

  void init(CcString &Name);
  static void serviceMain(DWORD argc, LPTSTR *argv);

  SERVICE_STATUS m_State;
};

#endif /* WINDOWSSERVICE_H_ */
