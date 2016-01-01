/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     LinuxFile.h
 * @brief    Class LinuxFile
 */
#ifndef LinuxFile_H_
#define LinuxFile_H_

#include "CcBase.h"
#include "CcFile.h"

/**
 * @brief Button for GUI Applications
 */
class LinuxFile : public CcFile {
public:
  /**
   * @brief Constructor
   */
  LinuxFile(CcString path = "");

  /**
   * @brief Destructor
   */
  virtual ~LinuxFile( void );

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
  FILE *m_hFile;
};

#endif /* LinuxFile_H_ */
