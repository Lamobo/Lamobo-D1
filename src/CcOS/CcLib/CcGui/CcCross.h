#ifndef CCCROSS_H
#define CCCROSS_H

#include "CcBase.h"

class CcCross
{
public:
  CcCross();
  CcCross(uint16 width, uint16 height, uint16 thickness);
  virtual ~CcCross();
  void setValues(uint16 width, uint16 height, uint16 thickness);
  uint16 m_width;
  uint16 m_height;
  uint16 m_thick;
};

#endif // CCCROSS_H
