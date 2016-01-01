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
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 * @file     CcThreadManager.h
 * @brief    Class CcThreadManager
 */
#ifndef CCTHREADMANAGER_H_
#define CCTHREADMANAGER_H_

#include "CcBase.h"
#include "CcThreadObject.h"
#include "CcString.h"
#include "CcSystem.h"
#include "CcVector.h"

/**
 * @brief Default Class to create a Application
 */
class CcThreadManager {
public:
  CcThreadManager(void);

  typedef struct {
    CcString     *sNname;
    CcThreadObject     *thread;
  }sThreadState;

  virtual ~CcThreadManager();

  void addThread(CcThreadObject *thread, CcString *Name);
  void closeAll(void);

public:
  CcVector<sThreadState> m_ThreadList;
};

#endif /* CCTHREADMANAGER_H_ */
