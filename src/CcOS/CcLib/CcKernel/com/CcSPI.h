#ifndef CCSPI_H
#define CCSPI_H

#include "CcBase.h"

#define CCSPI_MASTER  0x01
#define CCSPI_SLAVE   0x02

class CcSPI
{
public:
  CcSPI();
  virtual ~CcSPI();

  virtual bool init() = 0;
  virtual bool readWriteBuffer() = 0;

  bool writeBuffer();
  bool readBuffer();
private:
  uint8  m_SPIType;
  uint8  m_Frequency;
};

#endif // CCSPI_H
