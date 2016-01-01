#ifndef CCOBJECTHANDLER_H
#define CCOBJECTHANDLER_H

#include "CcBase.h"
#include "CcObject.h"
#include "CcVector.h"

typedef struct{
  CcObject *targetObject;
  uint8 targetId;
} CallbackStruct;

class CcObjectHandler : private CcVector<CallbackStruct>
{
public:
  CcObjectHandler();
  virtual ~CcObjectHandler();

  void call( void *Param = 0 );
  void add(CcObject &Object, uint8 nr);
  void add(CcObject *Object, uint8 nr);
  void remove(CcObject &Object);
  void remove(CcObject *Object);
  void remove(CcObject &Object, uint8 nr);
  void remove(CcObject *Object, uint8 nr);
  
};

#endif // CCOBJECTHANDLER_H
