/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     WindowsFilesystem.h
 * @brief    Class WindowsFilesystem
 */
#ifndef WINDOWSFILESYSTEM_H_
#define WINDOWSFILESYSTEM_H_

#include "CcBase.h"
#include "CcFileSystem.h"
#include "CcFile.h"

/**
 * @brief Button for GUI Applications
 */
class WindowsFilesystem : public CcFileSystem{
public:
  /**
   * @brief Constructor
   */
  WindowsFilesystem( void );

  /**
   * @brief Destructor
   */
  virtual ~WindowsFilesystem( void );

  CcFile *getFile(CcString &path);
  bool mkdir(CcString Path);
  bool del(CcString Path);
  CcString &getWorkingDir(void);
};

#endif /* WINDOWSFILESYSTEM_H_ */
