/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcIOAsync.h
 * @brief    Class CcIOAsync
 */

#ifndef CCIOASYNC_H_
#define CCIOASYNC_H_

#include "CcBase.h"
#include "CcObject.h"
#include "CcIODevice.h"

#define CCIOASYNC_CB_READDONE   0
#define CCIOASYNC_CB_WRITEDONE  1


/**
 * @brief Abstract Class for inheriting to every IODevice
 */
class CcIOAsync : private CcObject {
public:
  /**
   * @brief Constructor
   */
  CcIOAsync(CcIODevice *device);

  /**
   * @brief Destructor
   */
  virtual ~CcIOAsync();
  virtual bool read(char* buffer, size_t size);
  virtual bool write(char* buffer, size_t size);

  virtual bool onReadDone(size_t size);
  virtual bool onWriteDone(size_t size);

  virtual void callback(uint8 nr, void *Param = 0);
private:
  CcIODevice *m_Device;
};

#endif /* CCIOASYNC_H_ */
