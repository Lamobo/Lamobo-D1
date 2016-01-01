/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
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
