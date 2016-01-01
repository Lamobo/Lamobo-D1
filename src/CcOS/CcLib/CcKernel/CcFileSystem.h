/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcFileSystem.h
 * @brief    Class CcFileSystem
 */
#ifndef CCFILESYSTEM_H_
#define CCFILESYSTEM_H_

#include "CcBase.h"
#include "CcFile.h"
#include "CcVector.h"

class CcFileSystem;

typedef struct {
  CcString      Path;
  CcFileSystem  *FS;
} sCcFileSystemListItem;

/**
 * @brief Button for GUI Applications
 */
class CcFileSystem {
public:
  /**
   * @brief Constructor
   */
  CcFileSystem(void);

  /**
   * @brief Destructor
   */
  virtual ~CcFileSystem(void);

  virtual CcFile *getFile(CcString &path);

  virtual void addMountPoint(CcString &Path, CcFileSystem* Filesystem);
  virtual CcString &getWorkingDir(void);
  virtual bool mkdir(CcString Path);
  virtual bool del(CcString Path);
  void setWorkingDir(CcString &Path);

protected:
  CcString m_WorkingDir;
private:
  CcVector<sCcFileSystemListItem> m_FSList;
};

#endif /* CcFileSystem_H_ */
