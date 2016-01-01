/**
* @author     Andreas Dirmeier
* @copyright  Andreas Dirmeier (c) 2015
* @version    0.01
* @date       2015-10
* @par        Language   C++ ANSI V3
*/
/**
* @file     CcDeviceList.h
* @brief    Class CcDeviceList
*/
#ifndef CcDeviceList_H_
#define CcDeviceList_H_

#include "CcBase.h"

typedef struct{
  eCcDeviceType Type;
  CcIODevice *Device;
} sDeviceListItem;

/**
* @brief Handles all devices and Interfaces connected to Kernel
*/
class CcDeviceList : public CcVector<sDeviceListItem> {
public:
  CcDeviceList(){}
  virtual ~CcDeviceList(){}
};

#endif /* CcDeviceList_H_ */
