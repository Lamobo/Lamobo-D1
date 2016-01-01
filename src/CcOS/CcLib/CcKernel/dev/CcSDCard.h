/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcSDCard.h
 * @brief    Class CcSDCard
 */

#ifndef CCSDCARD_H_
#define CCSDCARD_H_

#include "CcBase.h"
#include "CcIODevice.h"

class CcSDCard {
public:
  CcSDCard();
  virtual ~CcSDCard();

  bool getAddr(uint32 Address, char* cReadBuf, uint32 length);

private:
  uint32 m_uiBlockSize;
  uint32 m_uiSDSize;

  CcIODevice *m_DeviceCom;
};

#endif /* CCSDCARD_H_ */
