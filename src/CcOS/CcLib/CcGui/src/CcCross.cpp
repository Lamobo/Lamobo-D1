#include "CcCross.h"

CcCross::CcCross(){

}

CcCross::CcCross(uint16 width, uint16 height, uint16 thickness){
  setValues( width, height, thickness);
}

CcCross::~CcCross(){

}

void CcCross::setValues(uint16 width, uint16 height, uint16 thickness)
{
  m_width = width;
  m_height = height;
  m_thick = thickness;
}
