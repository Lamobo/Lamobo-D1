/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     WindowsFile.h
 * @brief    Class WindowsFile
 */
#ifndef WindowsFile_H_
#define WindowsFile_H_

#include "CcBase.h"
#include "CcStringWin.h"
#include "WindowsGlobals.h"
#include "CcFile.h"

/**
 * @brief Button for GUI Applications
 */
class WindowsFile : public CcFile {
public:
  /**
   * @brief Constructor
   */
  WindowsFile(CcStringWin path = "");

  /**
   * @brief Destructor
   */
  virtual ~WindowsFile( void );

  size_t size(void);
  size_t read(char* buffer, size_t size);
  size_t write(char* buffer, size_t size);
  bool open(uint16 flags);
  bool close(void);
  bool isFile(void);
  bool isDir(void);
  bool move(CcString &Path);
  tm getLastModified(void);

  virtual bool setFilePointer(size_t pos);

  bool createFile();

  CcStringList getFileList(char showFlags = 0);

private:
  HANDLE m_hFile;
};

#endif /* WindowsFile_H_ */
