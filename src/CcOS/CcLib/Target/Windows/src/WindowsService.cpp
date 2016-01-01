/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     WindowsService.cpp
 * @brief    Class WindowsService
 **/
#include "WindowsService.h"
#include "CcKernel.h"


WindowsService::WindowsService() {
}

WindowsService::~WindowsService() {
  // nothing to do
}

void WindowsService::init(CcString &Name){
  CcStringWin winName(Name);
  winName.getLPCSTR();
  SERVICE_TABLE_ENTRY ServiceTable[] =
  {
    { winName.getLPSTR(), (LPSERVICE_MAIN_FUNCTION)&WindowsService::serviceMain },
    { NULL, NULL }
  };

  if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
  {
    //return GetLastError();
  }
}

void WindowsService::serviceMain(DWORD argc, LPTSTR *argv)
{
  Kernel.setArg(argc, argv);
}