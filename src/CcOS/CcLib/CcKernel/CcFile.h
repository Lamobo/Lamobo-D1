/**
 * @author     Andreas Dirmeier
 * @copyright  Andreas Dirmeier (c) 2015
 * @version    0.01
 * @date       2015-10
 * @par        Language   C++ ANSI V3
 */
/**
 * @file     CcFile.h
 * @brief    Class CcFile
 */
#ifndef CcFile_H_
#define CcFile_H_

#include "CcBase.h"
#include "CcIODevice.h"
#include "CcString.h"
#include "CcCharArray.h"
#include "CcStringList.h"

#define SHOW_HIDDEN   0x01
#define SHOW_EXTENDED 0x02

/**
 * @brief Button for GUI Applications
 */
class CcFile : public CcIODevice {
public:
  /**
   * @brief Constructor
   */
  CcFile( CcString path = "");


  /**
   * @brief Destructor
   */
  virtual ~CcFile( void );

  virtual size_t size(void);
  virtual size_t read(char* buffer, size_t size);
  virtual size_t write(char* buffer, size_t size);
  virtual bool open(uint16 flags);
  virtual bool close();
  virtual bool setFilePointer(size_t pos);
  virtual bool isFile(void);
  virtual bool isDir(void);
  virtual bool move(CcString &Path);
  virtual tm getLastModified(void);

  virtual CcStringList getFileList(char showFlags = 0);

  size_t write(CcCharArray &charArray, size_t offset=0, size_t len = SIZE_MAX);
  size_t read(CcCharArray &charArray, size_t offset = 0, size_t len = SIZE_MAX);

protected: //Variables
  CcString m_Path;
  size_t m_filePointer;
private:
  CcFile *m_SystemFile;
};

#endif /* CcFile_H_ */
