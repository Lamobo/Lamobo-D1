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
