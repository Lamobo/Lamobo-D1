// Windows/FileIO.h

#ifndef __WINDOWS_FILEIO_H
#define __WINDOWS_FILEIO_H

#include "../Common/Types.h"

#include "Defs.h"

namespace NWindows {
namespace NFile {
namespace NIO {

struct CByHandleFileInfo
{
  DWORD Attrib;
  FILETIME CTime;
  FILETIME ATime;
  FILETIME MTime;
  DWORD VolumeSerialNumber;
  UInt64 Size;
  DWORD NumberOfLinks;
  UInt64 FileIndex;
};

class CFileBase
{
protected:
  HANDLE _handle;
  
  bool Create(LPCTSTR fileName, DWORD desiredAccess,
      DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);
  #ifndef _UNICODE
  bool Create(LPCWSTR fileName, DWORD desiredAccess,
      DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);
  #endif

public:
  #ifdef SUPPORT_DEVICE_FILE
  bool IsDeviceFile;
  bool LengthDefined;
  UInt64 Length;
  #endif

  CFileBase(): _handle(INVALID_HANDLE_VALUE) {};
  ~CFileBase();

  bool Close();

  bool GetPosition(UInt64 &position) const;
  bool GetLength(UInt64 &length) const;

  bool Seek(Int64 distanceToMove, DWORD moveMethod, UInt64 &newPosition) const;
  bool Seek(UInt64 position, UInt64 &newPosition);
  bool SeekToBegin();
  bool SeekToEnd(UInt64 &newPosition);
  
  bool GetFileInformation(CByHandleFileInfo &fileInfo) const;
};

#define IOCTL_CDROM_BASE  FILE_DEVICE_CD_ROM
#define IOCTL_CDROM_GET_DRIVE_GEOMETRY  CTL_CODE(IOCTL_CDROM_BASE, 0x0013, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CDROM_MEDIA_REMOVAL  CTL_CODE(IOCTL_CDROM_BASE, 0x0201, METHOD_BUFFERED, FILE_READ_ACCESS)

class CInFile: public CFileBase
{
  #ifdef SUPPORT_DEVICE_FILE
  bool DeviceIoControl(DWORD controlCode, LPVOID inBuffer, DWORD inSize,
      LPVOID outBuffer, DWORD outSize, LPDWORD bytesReturned, LPOVERLAPPED overlapped) const
  {
    return BOOLToBool(::DeviceIoControl(_handle, controlCode, inBuffer, inSize,
        outBuffer, outSize, bytesReturned, overlapped));
  }

  bool DeviceIoControl(DWORD controlCode, LPVOID inBuffer,
      DWORD inSize, LPVOID outBuffer, DWORD outSize) const
  {
    DWORD ret;
    return DeviceIoControl(controlCode, inBuffer, inSize, outBuffer, outSize, &ret, 0);
  }

  bool DeviceIoControlOut(DWORD controlCode, LPVOID outBuffer, DWORD outSize) const
    { return DeviceIoControl(controlCode, NULL, 0, outBuffer, outSize); }

  #ifndef UNDER_CE
  bool GetGeometry(DISK_GEOMETRY *res) const
    { return DeviceIoControlOut(IOCTL_DISK_GET_DRIVE_GEOMETRY, res, sizeof(*res)); }

  bool GetCdRomGeometry(DISK_GEOMETRY *res) const
    { return DeviceIoControlOut(IOCTL_CDROM_GET_DRIVE_GEOMETRY, res, sizeof(*res)); }
  
  bool GetPartitionInfo(PARTITION_INFORMATION *res)
    { return DeviceIoControlOut(IOCTL_DISK_GET_PARTITION_INFO, LPVOID(res), sizeof(*res)); }
  #endif

  void GetDeviceLength();
  #endif

public:
  bool Open(LPCTSTR fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);
  bool OpenShared(LPCTSTR fileName, bool shareForWrite);
  bool Open(LPCTSTR fileName);
  #ifndef _UNICODE
  bool Open(LPCWSTR fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);
  bool OpenShared(LPCWSTR fileName, bool shareForWrite);
  bool Open(LPCWSTR fileName);
  #endif
  bool Read1(void *data, UInt32 size, UInt32 &processedSize);
  bool ReadPart(void *data, UInt32 size, UInt32 &processedSize);
  bool Read(void *data, UInt32 size, UInt32 &processedSize);
};

class COutFile: public CFileBase
{
public:
  bool Open(LPCTSTR fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);
  bool Open(LPCTSTR fileName, DWORD creationDisposition);
  bool Create(LPCTSTR fileName, bool createAlways);

  #ifndef _UNICODE
  bool Open(LPCWSTR fileName, DWORD shareMode, DWORD creationDisposition, DWORD flagsAndAttributes);
  bool Open(LPCWSTR fileName, DWORD creationDisposition);
  bool Create(LPCWSTR fileName, bool createAlways);
  #endif

  bool SetTime(const FILETIME *cTime, const FILETIME *aTime, const FILETIME *mTime);
  bool SetMTime(const FILETIME *mTime);
  bool WritePart(const void *data, UInt32 size, UInt32 &processedSize);
  bool Write(const void *data, UInt32 size, UInt32 &processedSize);
  bool SetEndOfFile();
  bool SetLength(UInt64 length);
};

}}}

#endif
