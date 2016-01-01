/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcThreadManager.h
 * @brief    Class CcThreadManager
 */
#ifndef CCTHREADMANAGER_H_
#define CCTHREADMANAGER_H_

#include "CcBase.h"
#include "CcThreadObject.h"
#include "CcString.h"
#include "CcSystem.h"
#include "CcVector.h"

/**
 * @brief Default Class to create a Application
 */
class CcThreadManager {
public:
  CcThreadManager(void);

  typedef struct {
    CcString     *sNname;
    CcThreadObject     *thread;
  }sThreadState;

  virtual ~CcThreadManager();

  void addThread(CcThreadObject *thread, CcString *Name);
  void closeAll(void);

public:
  CcVector<sThreadState> m_ThreadList;
};

#endif /* CCTHREADMANAGER_H_ */
