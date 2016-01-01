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
 * @file     CcStringList.cpp
 * @brief    Implemtation of class CcStringList
 */

#include "CcStringList.h"

CcStringList::CcStringList()
{
  // TODO Auto-generated constructor stub

}

CcStringList::~CcStringList()
{
  // TODO Auto-generated destructor stub
}

CcString CcStringList::parseArguments(CcString &Line){
  CcString temp;
  for (size_t i = 0; i < Line.length(); i++){
    if (Line.at(i) == '\n'){

    }
    else if (Line.at(i) == '\r')
      ;
    else if (Line.at(i) == ' '){
      append(temp);
      temp.clear();
    }
    else{
      temp.append(Line.at(i));
    }
  }
  append(temp);
  temp = at(0);
  deleteAt(0);
  return temp;
}

CcStringList &CcStringList::splitString(CcString &toSplit, char delimiter){
  CcString temp;
  for (size_t i = 0; i < toSplit.length(); i++){
    if (toSplit.at(i) == delimiter){
      append(temp);
      temp.clear();
    }
    else{
      temp.append(toSplit.at(i));
    }
  }
  append(temp);
  return *this;
}

CcString CcStringList::collapseList(CcString seperator){
  CcString sRet;
  bool first = true;
  for (size_t i = 0; i < size(); i++){
    if (!first)
      first = false;
    else
      sRet.append(seperator);
    sRet.append(at(i));
  }
  return sRet;
}
