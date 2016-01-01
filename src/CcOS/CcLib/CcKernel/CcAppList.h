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
* @file     CcAppList.h
* @brief    Class CcAppList
*/
#ifndef CcAppList_H_
#define CcAppList_H_

#include "CcBase.h"
#include "CcApp.h"
#include "CcString.h"

typedef struct{
  CcAppCreateFunction AppNew;
  CcString Name;
}sAppListItem;

/**
* @brief Handles all devices and Interfaces connected to Kernel
*/
class CcAppList : public CcVector<sAppListItem> {
public:
  CcAppList(){}
  virtual ~CcAppList(){}
};

#endif /* CcAppList_H_ */
