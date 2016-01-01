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
 * @file     CcApp.h
 * @brief    Class CcApp
 */
#ifndef CCAPPLICATION_H_
#define CCAPPLICATION_H_

#include "CcBase.h"
#include "CcThreadObject.h"
#include "CcStringList.h"

class CcApp;

typedef CcApp*(*CcAppCreateFunction)(CcStringList *Arg);

/**
 * @brief Default Class to create a Application
 */
class CcApp : public CcThreadObject {
public:
  CcApp();
  virtual ~CcApp();

  /**
   * @brief Virtual function for Running-Code
   *        Must be implemented by target application.
   */
  virtual void run(void) = 0;
};

#endif /* CCAPPLICATION_H_ */
