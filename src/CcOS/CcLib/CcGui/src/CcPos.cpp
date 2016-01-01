#include "CcPos.h"

CcPos::CcPos()
{
  setPos(0,0);
}

CcPos::CcPos(uint16 x, uint16 y)
{
  setPos(x, y);
}

CcPos::~CcPos()
{

}

void CcPos::setPos(uint16 x, uint16 y) {
  m_X = x;
  m_Y = y;
}

void CcPos::setX(uint16 x){
  m_X = x;
}

void CcPos::setY(uint16 y){
  m_Y = y;
}
