/*
 * @(#)File.h
 * @date 2009/11/11
 * @version 1.0
 * @author Lu Qiliu.
 * Copyright 2009 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _FILE_H_
#define        _FILE_H_

#include "mtdlib.h"
//#include    "anyka_types.h"

typedef    enum
{
    FAT_FS_ERROR = 0,
    FAT_FS_16    = 1,
    FAT_FS_32    = 2,
    FAT_FS_EX    = 3
}E_FATFS;



/* define file operation mode */
#define    FILE_MODE_READ      0x00
#define    FILE_MODE_CREATE    0x01
#define    FILE_MODE_OVERLAY   0x02
#define    FILE_MODE_APPEND    0x03

/*2012-03-09, control whether separete flush fat table*/
#define    FILE_MODE_EXT_NO_SEPARATE_FAT   0x80000000

/*2012-07-09,  for mprove video  record, first page data not to write until file close in file operation mode == FILE_MODE_CREATE*/
#define    FILE_MODE_EXT_NO_WRITE_FIRST_PAGE_BUF   0x40000000

/* Mar.15,07 - added to extend the FILE MODE to speed up. */
#define FILE_MODE_EXT_MAST          0x00FFFFFF // Mask the extended highest 8 bits
#define FILE_MODE_ASYN              0x100     // it will be asynchronism write and read

#define CLOSE_FLAG_OK        0x123455AA
#define CLOSE_FLAG_ERROR     0xAA987655

#define FILE_ATTRIBUTE_READONLY   0x00000001  
#define FILE_ATTRIBUTE_HIDDEN     0x00000002  
#define FILE_ATTRIBUTE_SYSTEM     0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY  0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE    0x00000020  
#define FILE_ATTRIBUTE_ENCRYPTED  0x00000040  
#define FILE_ATTRIBUTE_NORMAL     0x00000080

/* Following macros are used to filter files or folders when copying a folder or
   listing the files/folders in a folder. */
#define FILTER_DEFAULT    0x0000

// we will find it deeply, search all file include the sub folder
#define FILTER_DEEP       0x0001

// it will display all folder without matching, if it is set,FILTER_DEEP will be ignored
#define FILTER_FOLDER     0x0002

/* Control whether File_FindNext will iterate or not. Default is to iterate. 
   When setting the macro, File_FindNext will not iterate. */
#define FILTER_NOTITERATE   0x0004

// it will only search the folder
#define FILTER_ONLYFOLDER   0x0008

// it will only search the file
#define FILTER_ONLYFILE    0x0010

/* Max length of any file's absolute path, including the root part(e.g.:"A:/")
   and the '\0'. */
#define MAX_ABSOL_PATH_LEN    (259)

/* Max length of file's name*/
#define MAX_FILE_NAME_LEN     (255)


#define FILE_SEEK_SET        0
#define FILE_SEEK_CUR        1
#define FILE_SEEK_END        2

typedef enum tag_SetFileAttr
{
    FILE_ATTR_READONLY = 0x01,    
    FILE_ATTR_HIDDEN   = 0x02,    

    FILE_ATTR_SYSTEM   = 0x04
}E_SETFILEATTR;

#ifdef WIN32
#pragma pack(4)
#endif

typedef 
#ifndef WIN32
__packed
#endif
struct tag_FileTime
{
    T_U16 year;
    T_U16 month;
    T_U16 day;
    T_U16 hour;
    T_U16 minute;
    T_U16 second;
}T_FILETIME, *T_PFILETIME;

typedef 
#ifndef WIN32
__packed
#endif
struct tag_FileInfo
{
    T_U32 CreatTime;
    T_U32 CreatDate;
    T_U32 ModTime;
    T_U32 ModDate;
    T_U32 ParentId;
    T_U32 FileId;
    T_U32 FileFdt;             // Record the corresponding short fdt of this file.
    T_U32 length;              //data length
    T_U32 excess;            //data length high
    T_U32 attr;                //READ_ONLY ....
    T_U32 NameLen;             // The length of the following LongName.
    T_U16 LongName[300];
    T_U16 HashValue;
	T_U16 SeriousFlag;
    T_U8 ShortName[12];        //83 format name.
}T_FILEINFO, *T_PFILEINFO;

/* Define the invalid fdt. */
#define INVALID_FDT 0xFFFFFFFF

/* Define the mark to indicate the first entrance to FindNext. */
#define FST_ENTER_FLAG      0x80000000
#define FST_ENTER_FLAG_MASK 0x7FFFFFFF

/* Record the search processing: searching folder or searching file, or nothing. */
#define SEARCH_TYPE_FOLDER  0
#define SEARCH_TYPE_FILE    1
#define SEARCH_TYPE_NOTHING 2
#define SEARCH_TYPE_ITERATE 4
#define SEARCH_TYPE_NOTITERATE 8




#define FILE_FIND_DEEP_CNT       7    //查找的最大层数不能超过6 = 7 - 1
#define FIND_FILENAME_PATHLEN    262  //最大路径


typedef 
#ifndef WIN32
__packed
#endif
struct tag_Every_FileInfo
{
    T_U32 FirstClusterID;
    T_U32 FDTInParent;
}T_EVERY_FILEINFO, *T_PEVERY_FILEINFO;


typedef 
#ifndef WIN32
__packed
#endif
struct tag_FileInfo_BeforePower
{
    T_EVERY_FILEINFO EveryFileInfo[FILE_FIND_DEEP_CNT];
    T_U32 deep_cnt;          //当前的层数
}T_FILEINFO_BEFOREPOWER, *T_PFILEINFO_BEFOREPOWER;


typedef 
#ifndef WIN32
__packed
#endif
struct tag_FindBufCtrl
{
    T_U32 NodeCnt;   // How many node in the blink.
    T_U16 patternLen;
    T_U16 type;      // Control the dirs' function: such as FILTER_DEEP.
    T_U16 *pattern;
}T_FINDBUFCTRL, *T_PFINDBUFCTRL;


typedef 
#ifndef WIN32
__packed
#endif
struct tag_FindBufInfo
{
    T_PFILEINFO_BEFOREPOWER find_fileinfo; //记录每一层文件的首簇号和fdt
    T_U16 *pattern;      // need find
    T_U16 *fileter_pattern;  //no need find, only folder
    T_U16  find_type;   // SEARCH_TYPE_ITERATE or SEARCH_TYPE_NOTITERATE
    T_BOOL deep_flag;   //deep find or not
    
}T_FINDBUFINFO, *T_PFINDBUFINFO;


typedef T_BOOL (*F_CopyCallback)(T_VOID *pData, T_U16 *FileName,T_U32 CurPos, T_U32 FileSize);
typedef T_BOOL (*F_DelCallback)(T_VOID *pData, T_U16 *FileName);


/************************************************************************
 * NAME:        File_FindFirst
 * FUNCTION  find some files in input path by bufCtrl condition
 * PARAM:      const T_U16 *path
                      T_PFINDBUFCTRL bufCtrl
 * RETURN:     it will return the find handle, return 0 if it fails, otherwise return the handle
**************************************************************************/

T_U32 File_FindFirst(const T_U16 *path, T_PFINDBUFCTRL bufCtrl);


/************************************************************************
 * NAME:        File_FindNext
 * FUNCTION  find next files with the finding hanle
 * PARAM:      T_U32 pFindCtrl
                      T_S32 Cnt     the need file number
 * RETURN:     it will return the found file number
**************************************************************************/

T_U32 File_FindNext(T_U32 pFindCtrl, T_S32 Cnt);

/************************************************************************
 * NAME:        File_FindClose
 * FUNCTION   it will release all resource of the finding handle.
 * PARAM:      T_U32 pFindCtrl
 * RETURN:     NONE
**************************************************************************/

T_VOID File_FindClose(T_U32 pFindCtrl);

/************************************************************************
 * NAME:        File_FindInfo
 * FUNCTION  get file info of the input pFindCtrl
 * PARAM:      T_U32 pFindCtrl
                      T_U32 position :the file's position of pFindCtrl's blink 
                      T_U32 *FileCnt :     file count
                      T_U32 *FolderCnt :     folder count
 * RETURN:     it will return file info structure pointer 
**************************************************************************/

T_PFILEINFO File_FindInfo(T_U32 pFindCtrl, T_U32 Position, T_U32 *FileCnt, T_U32 *FolderCnt);


/************************************************************************
 * NAME:        File_FindFirstFromHandle
 * FUNCTION  find some files in input handle by bufCtrl condition
 * PARAM:      T_U32 file
                      T_PFINDBUFCTRL bufCtrl
 * RETURN:     it will return the find handle, return 0 if it fails, otherwise return the handle
**************************************************************************/
T_U32 File_FindFirstFromHandle(T_U32 file, T_PFINDBUFCTRL bufCtrl);



/************************************************************************
 * NAME:        File_FindCloseWithHandle
 * FUNCTION   it will release all resource of the finding handle.
 * PARAM:      T_U32 pFindCtrl
 * RETURN:     NONE
**************************************************************************/

T_VOID File_FindCloseWithHandle(T_U32 obj);




/************************************************************************
 * NAME:        File_FindInfoIsFolder
 * FUNCTION  check if the file is the floder
 * PARAM:      T_PFILEINFO FileInf  ofile info structure pointer 
 * RETURN:     it will return AK_TRUE if the file is the folder 
**************************************************************************/

T_BOOL File_FindInfoIsFolder(T_PFILEINFO FileInfo);

/************************************************************************
 * NAME:        File_FindOpen
 * FUNCTION  it will open file with the find information and open mode
 * PARAM:      T_U32 parent   the parent directory handle
                       T_PFILEINFO FileInfo
 * RETURN:     return the file hanle with only read, we will use File_IsFile to decide if the file is exist. 
**************************************************************************/

T_U32 File_FindOpen(T_U32 parent, T_PFILEINFO FileInfo);

/************************************************************************
 * NAME:        File_Close
 * FUNCTION  reflush write data to medium and delete refrence from system.
                      and destroy a file object to free space.
 * PARAM:      T_U32 file
 * RETURN:     NONE 
**************************************************************************/

T_VOID File_Close(T_U32 file);


/************************************************************************
 * NAME:        File_OpenAsc
 * FUNCTION  it will open file with the asc file name and open mode
 * PARAM:      T_U32 parent   the parent directory handle
                       const T_U8* FileName     the file name
                       T_U32 mode                     the open mode
 * RETURN:     return the file hanle, we will use File_IsFile to decide if the file is exist. 
**************************************************************************/

T_U32 File_OpenAsc(T_U32 parent, const T_U8* FileName, T_U32 mode);


/************************************************************************
 * NAME:        File_OpenUnicode
 * FUNCTION  it will open file with the unicode file name and open mode
 * PARAM:      T_U32 parent   the parent directory handle
                       const T_U8* FileName     the file name
                       T_U32 mode                     the open mode
 * RETURN:     return the file hanle, we will use File_Exist to decide if the file is exist. 
**************************************************************************/

T_U32 File_OpenUnicode(T_U32 parent, const T_U16* FileName, T_U32 mode);

/************************************************************************
 * NAME:        File_IsFolder
 * FUNCTION  it will check the file handle if it is directory
 * PARAM:      T_U32 file
 * RETURN:     if file is exist and file is a directory return AK_TRUE else return AK_FALSE
**************************************************************************/

T_BOOL File_IsFolder(T_U32 file);

/************************************************************************
 * NAME:        File_IsFile
 * FUNCTION  it will check the file handle if it is file
 * PARAM:      T_U32 file
 * RETURN:     if file is exist and file is a data file return AK_TRUE else return AK_FALSE
**************************************************************************/

T_BOOL File_IsFile(T_U32 file);

/************************************************************************
 * NAME:        File_Exist
 * FUNCTION  it will check if the file handle is exist
 * PARAM:      T_U32 file
 * RETURN:     return AK_TRUE if the handle is exist ,otherwise return AK_FALSE
**************************************************************************/

T_BOOL File_Exist(T_U32 file);

/************************************************************************
 * NAME:        File_GetLength
 * FUNCTION  it will get the file handle iformation
 * PARAM:      T_U32 file
                       T_U32* excess        the high 32 bit of the file length, or the folder number
 * RETURN:     return file's length low 32 bit. and set high. If the file is a directory, then
                       high records its sub folder's num and the return value is its sub file's num
**************************************************************************/

T_U32 File_GetLength(T_U32 file, T_U32* excess);


/************************************************************************
 * NAME:        File_GetOccupiedSpace
 * FUNCTION  it will get the file handle iformation
 * PARAM:      T_U32 file
                       T_U32* high        the high 32 bit of the file length 
 * RETURN:     the file had used the low 32 bit of the file of the room
**************************************************************************/

T_U32 File_GetOccupiedSpace(T_U32 file, T_U32 *high);

/************************************************************************
 * NAME:        File_DelAsc
 * FUNCTION  delete a old file or folder.
 * PARAM:      const T_U8 *FileName the asc file name 
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL File_DelAsc(const T_U8 *FileName);

/************************************************************************
 * NAME:        File_DelUnicode
 * FUNCTION  delete a old file or folder.
 * PARAM:      const T_U8 *FileName the unicode file name 
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/
T_BOOL File_DelUnicode(const T_U16 *FileName);

/************************************************************************
 * NAME:        File_RenameTo
 * FUNCTION  rename or move file
 * PARAM:      T_U32 surfile
                      T_U32 surfile
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL File_RenameTo(T_U32 surfile, T_U32 destfile);


/************************************************************************
 * NAME:        File_Seek
 * FUNCTION  move data file read /write pointer.
 * PARAM:      T_U32 file
                      T_U32 LowPos      the file low 32 bit
                      T_U32 *HighPos    the file high 32 bit, it will return the high 32 bit of file point 
 * RETURN:     return the  low 32 bit of the file point
**************************************************************************/

T_U32 File_Seek(T_U32 file, T_U32 LowPos, T_U32 *HighPos);

/************************************************************************
 * NAME:        File_Read
 * FUNCTION  read data from a data file.
 * PARAM:      T_U32 file
                      T_pVOID buf
                      T_U32 byts
 * RETURN:     return the read size
**************************************************************************/

T_U32 File_Read(T_U32 file, T_pVOID buf, T_U32 byts);

/************************************************************************
 * NAME:        File_Write
 * FUNCTION  write data from a data file.
 * PARAM:      T_U32 file
                      T_pVOID buf
                      T_U32 byts
 * RETURN:     return the write size
**************************************************************************/

T_U32 File_Write(T_U32 file, T_pVOID buf, T_U32 size);

/************************************************************************
 * NAME:        File_DelDir
 * FUNCTION  delete all file and folder of a folder.
 * PARAM:      T_U32 folder
 * RETURN:     return the delete file and folder number 
**************************************************************************/

T_U32 File_DelDir(T_U32 folder);

/************************************************************************
 * NAME:        File_DelFile
 * FUNCTION  delete file
 * PARAM:      T_U32  file
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL File_DelFile(T_U32 file);

/************************************************************************
 * NAME:        File_MkdirsAsc
 * FUNCTION  create a folder by ascii path, include middle folder
 * PARAM:      const T_U8* path
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

//create a folder by ascii path, include middle folder.
T_BOOL File_MkdirsAsc(const T_U8* path);

/************************************************************************
 * NAME:        File_MkdirsUnicode
 * FUNCTION  create a folder by unicode path, include middle folder
 * PARAM:      const T_U16* path
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL File_MkdirsUnicode(const T_U16* path);

/************************************************************************
 * NAME:        File_Flush
 * FUNCTION  flush last data to file.
 * PARAM:      T_PFILE file
 * RETURN:     NONE
**************************************************************************/

T_VOID File_Flush(T_U32 file);

/************************************************************************
 * NAME:        File_FlushFat
 * FUNCTION  it will flush fat link to driver
 * PARAM:      T_U32 file
                      T_U8 *AsynFatBuf , the fat buffer
 * RETURN:     NONE
**************************************************************************/
T_VOID File_FlushFat(T_U32 file, T_U8 *AsynFatBuf);



/************************************************************************
 * NAME:        File_SetDefault
 * FUNCTION  if the file is folder, set it is defualt path
 * PARAM:      T_U32 file
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL File_SetDefault(T_U32 file);

/************************************************************************
 * NAME:        File_SetDefault
 * FUNCTION  get file's attribute by ascii path, such as folder(0x10)
 * PARAM:      const T_U8* path
 * RETURN:     return the file current attribute
**************************************************************************/

T_U32 File_GetAttrAsc(const T_U8* path);

/************************************************************************
 * NAME:        File_GetAttrUnicode
 * FUNCTION  get file's attribute by unicode path, such as folder(0x10)
 * PARAM:      const T_U16* path
 * RETURN:     return the file current attribute
**************************************************************************/

T_U32 File_GetAttrUnicode(const T_U16* path);

/************************************************************************
 * NAME:        File_GetAttrUnicode
 * FUNCTION  set file's attribute by ascii path.
 * PARAM:      const T_U16* path
                      E_SETFILEATTR attribute
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL File_SetAttrAsc(const T_U8* path, E_SETFILEATTR attribute);


/************************************************************************
 * NAME:        File_SetAttrUnicode
 * FUNCTION  set file's attribute by unicode path.
 * PARAM:      const T_U16* path
                      E_SETFILEATTR attribute
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL File_SetAttrUnicode(const T_U16* path, E_SETFILEATTR attribute);

/************************************************************************
 * NAME:        File_Truncate
 * @FUNCTION：Truncate an open file.
 * @PARAM:  T_U32 file -- the file you want to truncate.
                     T_U32 length -- the new length of the file,
                        it must be less than the file's length
 * @RETURN  : truncate successfully or not
 * @Attention: The file to be truncated couldn't be File_Destroy().
**************************************************************************/

T_BOOL File_Truncate(T_U32 file, T_U32 low, T_U32 high);

/************************************************************************
 * NAME:        File_SetBufferSize
 * FUNCTION  SectorSize must be 2^n! If not, the file->BufLenMask may not work!
 * PARAM:      T_U32 file
                      T_U32 BufSize --  the file's buffer size which is  kb.
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL File_SetBufferSize(T_U32 file, T_U32 SectorNum);

/************************************************************************
 * @FUNCTION：Get the Mod time of the input file.
 * @PARAM   : T_U32 file -- the file whose time you want to get.
              T_PFILETIME fileTime -- the structure used to put file time.
 * @RETURN  : Get ModTime successfully or not.
**************************************************************************/
T_BOOL File_GetModTime(T_U32 file, T_PFILETIME fileTime);

/************************************************************************
 * @FUNCTION：Get the Create time of the input file.
 * @PARAM   : T_U32 file -- the file whose time you want to get.
              T_PFILETIME fileCTime -- the structure used to put file time.
 * @RETURN  : Get CreateTime successfully or not.
**************************************************************************/
T_BOOL File_GetCreateTime(T_U32 file, T_PFILETIME fileCTime);

/************************************************************************
 * NAME:        File_GetFileinfo
 * FUNCTION  get file info of the input file
 * PARAM:      T_U32 file
                       T_PFILEINFO fileInfo
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL File_GetFileinfo(T_U32 file, T_PFILEINFO fileInfo);

/************************************************************************
 * NAME:        File_GetFileinfo
 * FUNCTION  copy  source file to dest file by ascii format.
 * PARAM:      const T_U8* srcPath
                      const T_U8* dstPath
                      T_BOOL replace    if it is ak_TRUE, we will replace the same file name
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL  File_CopyAsc(const T_U8* srcPath, const T_U8* dstPath, T_BOOL replace, F_CopyCallback pCallBack, T_VOID *pCallBackData);


/************************************************************************
 * NAME:        File_GetFileinfo
 * FUNCTION  copy  source file to dest file by unicode format.
 * PARAM:      const T_U16* srcPath
                      const T_U16* dstPath
                      T_BOOL replace    if it is ak_TRUE, we will replace the same file name
 * RETURN:     return ak_true if it is ok, otherwise return ak_false
**************************************************************************/

T_BOOL  File_CopyUnicode(const T_U16* srcPath, const T_U16* dstPath, T_BOOL replace, F_CopyCallback pCallBack, T_VOID *pCallBackData);

/************************************************************************
 * NAME:        File_GetFilePtr
 * FUNCTION  get the file point
 * PARAM:      T_U32 file
                      T_U32 *HighPos    the file high 32 bit, it will return the high 32 bit of file point 
 * RETURN:     return the  low 32 bit of the file point
**************************************************************************/

T_U32 File_GetFilePtr(T_U32 file, T_U32 *high);

/************************************************************************
 * NAME:        File_SetFilePtr
 * FUNCTION  set the file point
 * PARAM:      T_U32 file
                      T_U32 offset      the new offset
                      T_U16 origin      the mode
 * RETURN:     return the  low 32 bit of the file point
**************************************************************************/

T_U32 File_SetFilePtr(T_U32 file, T_S32 offset, T_U16 origin);

/************************************************************************
 * NAME:        File_GetPathObj
 * FUNCTION  get file's absolute path. it include device number, abstract path and last name.
 * PARAM:      T_U32 file
 * RETURN:     return the  path name information
**************************************************************************/

T_U32 File_GetPathObj(T_U32 file);

/************************************************************************
 * NAME:        File_GetAbsPath
 * FUNCTION  get file name from the handle of File_GetPathObj
 * PARAM:      T_U32 obj
 * RETURN:     return the  path name 
**************************************************************************/
T_U16* File_GetAbsPath(T_U32 obj);

/************************************************************************
 * NAME:        File_DestroyPathObj
 * FUNCTION  free the ram of File_GetPathObj
 * PARAM:      T_U32 obj
 * RETURN:     NONE
**************************************************************************/
T_VOID File_DestroyPathObj(T_U32 obj);

/************************************************************************
 * NAME:        File_SetFileSize
 * FUNCTION  it will malloc some room to the file, and make the file have serious room
 * PARAM:      T_U32 file
                      T_U32 fileSize    the file size
 * RETURN:     return the  path name information
**************************************************************************/
T_BOOL File_SetFileSize(T_U32 file, T_U32 fileSize);

/************************************************************************
 * NAME:        FSLib_GetVersion
 * FUNCTION  it will get the version of the fs library
 * PARAM:      T_VOID
 * RETURN:     return the  version string
**************************************************************************/

T_U8* FSLib_GetVersion(T_VOID);


/************************************************************************
* NAME:        File_DelUnicode_DelCallBack
* FUNCTION  it will delete the file with name. it can only delete the file or the empty folder,if delCallBack retruen
                    ak_false, File_DelUnicode_DelCallBack  return AK_FALSE
* PARAM:      const T_U16 *FileName      the file or folder name
* RETURN:     AK_TRUE for success, otherwise return AK_FALSE
**************************************************************************/
T_BOOL File_DelUnicode_DelCallBack(const T_U16 *FileName, F_DelCallback delCallBack, T_VOID *delCallBackData);


/************************************************************************
* NAME:        File_DelAsc_DelCallBack
* FUNCTION  it will delete the file with name. it can only delete the file or the empty folderif delCallBack retruen
                    ak_false, File_DelAsc_DelCallBack  return AK_FALSE
* PARAM:      const T_U8 *FileName      the file or folder name
* RETURN:     AK_TRUE for success, otherwise return AK_FALSE
**************************************************************************/

T_BOOL File_DelAsc_DelCallBack(const T_U8 *FileName, F_DelCallback delCallBack, T_VOID *delCallBackData);



/************************************************************************
 * NAME:        File_GetLastErrorType
 * FUNCTION  get the last error type
 * PARAM:      T_VOID
 * RETURN:     T_U32 last_errortype
**************************************************************************/
T_U32 File_GetLastErrorType(T_VOID);


/************************************************************************
 * NAME:     File_CreateVolumeAsc
 * FUNCTION  it will create driver's volume with the asc file name
 * PARAM:    FileName -- volume file name, e.g. "B:/xxxx", xxxx max 11 byte
 * RETURN:     AK_TRUE for success, otherwise return AK_FALSE
**************************************************************************/
T_BOOL File_CreateVolumeAsc(const T_U8* FileName);

/************************************************************************
 * NAME:     File_CreateVolumeUnicode
 * FUNCTION  it will create driver's volume with the unicode file name
 * PARAM:    FileName -- volume file name, e.g. "B:/xxxx", xxxx max 11 byte
 * RETURN:     AK_TRUE for success, otherwise return AK_FALSE
**************************************************************************/
T_BOOL File_CreateVolumeUnicode(const T_U16* FileName);




/************************************************************************
 * NAME:     File_GetfileclusterInfo_ToUpdate
 * FUNCTION  get the file cluster info to burn update
 * PARAM:    in  T_U32 file -- the handle of the file
                   out  T_U32 *ClusterNum  the num of cluster
                   out  T_U32 *SecPerClus  the secter per cluster
 * RETURN:     return  pFilePosInfo  for success, otherwise return ak_null,   free pFilePosInfo  by File_GetfileclusterInfo_ToFree()
**************************************************************************/
 T_U32 *File_GetfileclusterInfo_ToUpdate( T_U32 file, T_U32 *ClusterNum, T_U32 *SecPerClus);


/************************************************************************
 * NAME:     File_GetfileclusterInfo_ToFree
 * FUNCTION  free the ram of File_GetfileclusterInfo_ToUpdate()
 * PARAM:    in  T_U32 *pFilePosInfo -- the handle of the file
  * RETURN:     t_void
**************************************************************************/
T_VOID File_GetfileclusterInfo_ToFree(T_U32 *pFilePosInfo);

/* ************************************************************
	in order to shorten  time that find a file or folder.once found a  satisfied file or folder,function exit.
	from "PreFDT" position of "T_U16 *path" ,start find file and folder satisfying the setting "pattern" condition
**************************************************************************/
T_U32 File_FindFirstEX(const T_U16 *path, T_U16 *pattern, T_U16 *fileter_pattern, T_U32 PreFDT);


/************************************************************************
use together with  File_FindFirstEX, only find a folder and file 	
**************************************************************************/
T_U32 File_FindNextEX(T_U32 obj);


/************************************************************************
 * NAME:     File_GetFindFileData
 * FUNCTION read  data of the first cluster  
 * PARAM:    in  T_U32 *pFilePosInfo -- the handle of the file
  * RETURN:     t_void
**************************************************************************/
T_U32 File_GetFindFileData(T_U32 obj, T_PFILEINFO pInfo, T_U8 *data, T_U32 size);


/************************************************************************
 * NAME:     File_FindFirst_withIP
 * FUNCTION find  one file by IP AND FDT  
 * PARAM:    const T_U16 *path -- the path of the file
* PARAM:     T_U32 *ret_cnt        find the num of file
* PARAM:     T_U16 *path_buf                                               get the path of the file
* PARAM:     T_PFINDBUFINFO find_fileinfo        the info of  file  before  power down 


* RETURN:     t_void
**************************************************************************/
T_U32 File_FindFirst_WithID(const T_U16 *path, T_U32 *ret_cnt,  T_U16 *path_buf, T_PFINDBUFINFO find_fileinfo);


/************************************************************************
 * NAME:     File_FindNext_withID
 * FUNCTION find next file
 * PARAM:    const T_U32 obj, -- the hande of the file
 * PARAM:     T_S32 Cnt,            -1 or 1 
 * PARAM:     T_U32 *ret_cnt        find the num of file
  * RETURN:     t_void
**************************************************************************/
T_U32 File_FindNext_WithID(T_U32 obj, T_S32 Cnt, T_U32 *ret_cnt);


/************************************************************************
 * NAME:     File_get_findfile_info
 * FUNCTION read  data of the first cluster  
 * PARAM:    T_PFILEINFO_BEFOREPOWER find_fileinfo
* RETURN:     T_VOID
**************************************************************************/
T_VOID File_get_findfile_info_WithID(T_U32 obj, T_PFILEINFO_BEFOREPOWER fileinfo);



/************************************************************************
 * NAME:        File_FindInfo_WithID
 * FUNCTION  get file info of the input pFindCtrl
 * PARAM:      T_U32 find_handle
                      T_U32 position :the file's position of pFindCtrl's blink 
                      T_U32 *FileCnt :     file count
                      T_U32 *FolderCnt :     folder count
 * RETURN:     it will return file info structure pointer 
**************************************************************************/

T_PFILEINFO File_FindInfo_WithID(T_U32 obj, T_U32 Position, T_U32 *FileCnt, T_U32 *FolderCnt);


/************************************************************************
 * NAME:        File_FindCloseWithID
 * FUNCTION   it will release all resource of the finding handle.
 * PARAM:      T_U32 pFindCtrl
 * RETURN:     NONE
**************************************************************************/
    
T_VOID File_FindClose_WithID(T_U32 obj);



T_BOOL File_chkdsk_withfolder(const T_U8 *path);



/************************************************************************
 * NAME:     File_GetFindFileData_WithID
 * FUNCTION read  data of the first cluster  
 * PARAM:    in  T_U32 obj ------>the handle of File_FindFirst_withID or File_FindNext_withID return
                    T_PFILEINFO pInfo-----> File_FindInfo_WithID  return
                    T_U8 *data,  BUF
                    T_U32 size     4k DATA
  * RETURN:     t_void
**************************************************************************/
T_U32 File_GetFindFileData_WithID(T_U32 obj, T_PFILEINFO pInfo, T_U8 *data, T_U32 size);



#ifdef WIN32
#pragma pack()
#endif

#endif       //_FILE_H_

