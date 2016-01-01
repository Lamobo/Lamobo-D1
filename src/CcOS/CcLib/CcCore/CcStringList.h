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
 * @brief    Class CcString
 **/
#ifndef CCSTRINGLIST_H_
#define CCSTRINGLIST_H_

#include "CcString.h"
#include "CcVector.h"

/**
 * @brief Handle list of Strings
 */
class CcStringList : public CcVector<CcString>
{
  public:
    CcStringList();
    virtual ~CcStringList();

    /**
     * @brief Parse an Commandline input into the Stringlist and return the argument
     * @param Line: Line to parse;
     * @return First argument in line
     */
    CcString parseArguments(CcString &Line);
    CcStringList& splitString(CcString &toSplit, char delimiter);
    CcString collapseList(CcString seperator);
};

#endif /* CCSTRINGLIST_H_ */
