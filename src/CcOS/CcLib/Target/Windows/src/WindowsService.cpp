/**
 * @copyright  Andreas Dirmeier (C) 2015
 *
 * This file is part of CcOS.
 *
 * CcOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CcOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CcOS.  If not, see <http://www.gnu.org/licenses/>.
 **/
/**
 * @author     Andreas Dirmeier
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
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