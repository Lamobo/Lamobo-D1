/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxFilesystem.h
 * @brief    Class LinuxFilesystem
 */
#ifndef LinuxFILESYSTEM_H_
#define LinuxFILESYSTEM_H_

#include "CcBase.h"
#include "CcFileSystem.h"
#include "CcFile.h"

/**
 * @brief Button for GUI Applications
 */
class LinuxFilesystem : public CcFileSystem{
public:
  /**
   * @brief Constructor
   */
  LinuxFilesystem( void );

  /**
   * @brief Destructor
   */
  virtual ~LinuxFilesystem( void );

  CcFile *getFile(CcString &path);
  bool mkdir(CcString Path);
  bool del(CcString Path);
  CcString &getWorkingDir(void);
};

#endif /* LinuxFILESYSTEM_H_ */
