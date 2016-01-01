/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcObject.h
 * @brief    Class CcObject
 */
#ifndef CCOBJECT_H_
#define CCOBJECT_H_

#include "CcBase.h"

class CcObject {
public:
  CcObject(){
  }
  virtual ~CcObject(){}

  virtual void callback(uint8, void *){
  }
};

#endif /* CCOBJECT_H_ */
