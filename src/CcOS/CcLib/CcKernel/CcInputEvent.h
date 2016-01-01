/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcInputEvent.h
 * @brief    Class CcInputEvent
 */
#ifndef CcInputEvent_H_
#define CcInputEvent_H_

#include "CcBase.h"
#include "CcString.h"
#include "CcInputEvent.defs.h"

typedef struct {
  uint16 type;
  uint16 code;
  uint16 x;
  uint16 y;
}sCcMouseEvent;

/**
 * @brief Default Class for Input Events like Keyboard, Mouse
 */
class CcInputEvent {
public:
  CcInputEvent();
  virtual ~CcInputEvent();

  void setMouseEvent(uint16 type, uint16 code, uint16 x, uint16 y);
  sCcMouseEvent getMouseEvent(void);

  typedef union{
    struct {
      uint16 x;
      uint16 y;
    } coord;
    uint32 value;
  } mouse_convert;

  uint16 m_type;
  uint16 m_code;
  uint32 m_value;
};

#endif /* CcInputEvent_H_ */
