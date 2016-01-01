#ifndef CCPOS_H
#define CCPOS_H

#include "CcBase.h"

class CcPos
{
public:
  CcPos();
  CcPos(uint16 x, uint16 y);
  virtual ~CcPos();

public:
  uint16 getX( void ){return m_X;}
  uint16 getY( void ){return m_Y;}
  void setPos(uint16 x, uint16 y);
  void setX(uint16 x);
  void setY(uint16 y);

private:
  uint16 m_X;
  uint16 m_Y;
};

#endif // CCPOS_H
