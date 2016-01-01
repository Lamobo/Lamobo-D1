/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-08
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcWindowEvents.h
 * @brief    Class CcWindowEvents
 */
#ifndef CcWindowEvents_H_
#define CcWindowEvents_H_

#include "CcBase.h"
#include "CcObject.h"
#include "CcObjectHandler.h"
#include "CcVector.h"

#define CB_EVENT      0

/**
* @brief Register a Callback, if click recieved within this defined window.
*/
typedef struct {
  uint16 posXStart;
  uint16 posYStart;
  uint16 posXEnd;
  uint16 posYEnd;
  CcObjectHandler OH;
} sClickWindow;


class CcWindowEvents : public CcObject {
public:
  CcWindowEvents( void );
  virtual ~CcWindowEvents( void );
  void callback(uint8 nr, void *Param = 0);



  void registerOnClick(sClickWindow *window);
  void deleteOnClick(sClickWindow *window);

private:
  bool findOnClick(uint16 posX, uint16 posY);

  CcVector<sClickWindow*> clickList;
};

#endif /* CcWindowEvents_H_ */
