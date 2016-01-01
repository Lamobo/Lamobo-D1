#include "CcObjectHandler.h"

CcObjectHandler::CcObjectHandler()
{

}

CcObjectHandler::~CcObjectHandler()
{

}

void CcObjectHandler::call(void *Param){
  for (uint32 i = 0; i< size(); i++)
  {
    at(i).targetObject->callback(at(i).targetId, Param);
  }
}

void CcObjectHandler::add( CcObject &Object, uint8 nr){
  add(&Object, nr);
}

void CcObjectHandler::add( CcObject *Object, uint8 nr){
  CallbackStruct newCBS;
  newCBS.targetObject = Object;
  newCBS.targetId     = nr;
  append(newCBS);
}

void CcObjectHandler::remove(CcObject &Object){
  remove(&Object);
}

void CcObjectHandler::remove(CcObject *Object){
  for (uint32 i = 0; i<size(); i++)
  {
    if (at(i).targetObject == Object)
      deleteAt(i);
  }
}

void CcObjectHandler::remove(CcObject &Object, uint8 nr){
  remove(&Object, nr);
}

void CcObjectHandler::remove(CcObject *Object, uint8 nr){
  for(uint32 i=0; i<size(); i++)
  {
    if( at(i).targetObject == Object &&
        at(i).targetId == nr)
      deleteAt(i);
  }
}
