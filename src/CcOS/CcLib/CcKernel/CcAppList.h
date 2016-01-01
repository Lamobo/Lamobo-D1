/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
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
