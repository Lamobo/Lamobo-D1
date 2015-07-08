#ifndef        _FSA_H_
#define        _FSA_H_

#include "anyka_types.h"
//#include "file.h"
#define   MAX_PATH   		    260

/** define the file handler*/
#ifndef T_hFILE
#define	T_hFILE					T_S32		/* FILE * */
#endif

#ifndef FS_SEEK_SET
#define FS_SEEK_SET			0
#endif

#ifndef FS_SEEK_CUR	
#define FS_SEEK_CUR			1	
#endif

#ifndef FS_SEEK_END
#define FS_SEEK_END			2
#endif

/** define the invalid file handle 
*/
#ifndef FS_INVALID_HANDLE
#define FS_INVALID_HANDLE		-1
#endif

#ifndef FS_INVALID_SEEK
#define FS_INVALID_SEEK -1
#endif

#define FHA_FILE_MODE_READ		0//FILE_MODE_READ
#define FHA_FILE_MODE_CREATE	1//FILE_MODE_CREATE
#define FHA_FILE_MODE_OVERLAY	2//FILE_MODE_OVERLAY
#define FHA_FILE_MODE_APPEND	3//FILE_MODE_APPEND


/*返回值：*/
#define FSA_SUCCESS 1
#define FSA_FAIL    0

#define FSA_Create_file 0
#define FSA_Create_Dir  1

/*初始化回调接口：*/

typedef T_hFILE (*FSA_FILE_Open)(T_pCSTR path, T_U32 mode);
typedef T_BOOL  (*FSA_FILE_Close)(T_hFILE hFile);
typedef T_U32   (*FSA_FILE_Read)(T_hFILE hFile, T_pVOID buffer, T_U32 count);
typedef T_U32   (*FSA_FILE_Write)(T_hFILE hFile, T_pCVOID buffer, T_U32 count);
typedef T_S32   (*FSA_FILE_Seek)(T_hFILE hFile, T_S32 offset, T_U16 origin);
typedef T_BOOL  (*FSA_FILE_MkDirs)(T_pCSTR path);
typedef struct tag_FsaFileFunc
{
    FSA_FILE_Open        FileOpen;
    FSA_FILE_Close       FileClose;
    FSA_FILE_Read		 FileRead;
    FSA_FILE_Write		 FileWrite;
    FSA_FILE_Seek		 FileSeek;
    FSA_FILE_MkDirs      FsMkDir;
 }T_FSA_FILE_FUNC, *T_PFSA_FILE_FUNC;

typedef T_pVOID (*FSA_RamAlloc)(T_U32 size);
typedef T_pVOID (*FSA_RamFree)(T_pVOID var);
typedef T_pVOID (*FSA_MemSet)(T_pVOID pBuf, T_S32 value, T_U32 count);
typedef T_pVOID (*FSA_MemCpy)(T_pVOID dst, T_pCVOID src, T_U32 count);
typedef T_S32   (*FSA_MemCmp)(T_pCVOID pbuf1, T_pCVOID pbuf2, T_U32 count);
typedef T_pVOID (*FSA_MemMov)(T_pVOID dst, T_pCVOID src, T_U32 count);
typedef T_S32  (*FSA_Printf)(T_pCSTR s, ...);
typedef  T_pVOID (*FSA_GetImgMedium)(T_U8 driverID);

typedef struct tag_FSA_LibCallback
{
    FSA_RamAlloc RamAlloc;
    FSA_RamFree RamFree;
    FSA_MemSet  MemSet;
    FSA_MemCpy  MemCpy;
    FSA_MemCmp  MemCmp;
    FSA_MemMov  MemMov;
    FSA_Printf  Printf;
    
    T_FSA_FILE_FUNC fFs;
    FSA_GetImgMedium GetImgMedium;
}T_FSA_LIB_CALLBACK, *T_PFSA_LIB_CALLBACK;

typedef struct
{
	T_U32 file_length;
	T_U32 file_mode;
	T_U32 file_time;
	T_BOOL bCheck;
	T_U8   resv[3];
	T_U8   apath[MAX_PATH+1];
}T_UDISK_FILE_INFO;

typedef struct tag_ImgInfo
{
    T_U32   data_length;
    T_U8    DriverName;
    T_U8    bCheck;
    T_U8    wFlag;
    T_U8    resv[1];
}T_IMG_INFO;

/************************************************************************
 * NAME:     FSA_init
 * FUNCTION  Initial FSA callback function
 * PARAM:    [in] pCB--callback function struct pointer
 * RETURN:   success return FSA_SUCCESS, fail retuen FSA_ FAIL
**************************************************************************/
T_U32  FSA_init(T_PFSA_LIB_CALLBACK pCB);

/************************************************************************
 * NAME:     FSA_write_file_begin
 * FUNCTION  set write disk file(pUdiskFile->apath) start init para
 * PARAM:    [in] pUdiskFile--disk file info struct pointer
 * RETURN:   success return FSA_SUCCESS, fail retuen FSA_ FAIL
**************************************************************************/
T_U32  FSA_write_file_begin(T_UDISK_FILE_INFO *pUdiskFile);

/************************************************************************
 * NAME:     FSA_write_file
 * FUNCTION  write file to disk
 * PARAM:    [in] pData-----file's data pointer addr
 *           [in] data_len--file's data length
 * RETURN:   success return FSA_SUCCESS, fail retuen FSA_ FAIL
**************************************************************************/
T_U32  FSA_write_file(const T_U8 *pData,  T_U32 data_len);

/************************************************************************
 * NAME:     FSA_write_image_begin
 * FUNCTION  set write disk image start init para
 * PARAM:    [in] img_info--disk image info struct pointer
 *                          if img_info->wFlag == AK_TRUE, will direct write disk
 *                          else will check disk fs and input image is not matching
 * RETURN:   success return FSA_SUCCESS, fail retuen FSA_ FAIL
**************************************************************************/
T_U32  FSA_write_image_begin(T_IMG_INFO *img_info);

/************************************************************************
 * NAME:     FSA_write_image
 * FUNCTION  write image to disk
 * PARAM:    [in] pData-----image's data pointer addr
 *           [in] data_len--image's data length
 * RETURN:   success return FSA_SUCCESS, fail retuen FSA_ FAIL
**************************************************************************/
T_U32  FSA_write_image(const T_U8 * pData,  T_U32 data_len);

/************************************************************************
 * NAME:     FSA_burn_close
 * FUNCTION  free all fsa malloc ram
 * PARAM:    [in] none
 * RETURN:   success return FSA_SUCCESS, fail retuen FSA_ FAIL
**************************************************************************/
T_U32  FSA_burn_close(T_VOID);

/************************************************************************
 * NAME:     FSA_write_image_begin_burn
 * FUNCTION  set write disk image start init para,and malloc g_ ram
 * PARAM:    [in] none
 * RETURN:   success return handle, fail retuen FS_INVALID_HANDLE
**************************************************************************/
T_hFILE  FSA_write_image_begin_burn(T_IMG_INFO *img_info);

/************************************************************************
 * NAME:     FSA_write_image_end_burn
 * FUNCTION  free FSA_write_image_begin_burn malloc ram
 * PARAM:    [in] handle     the handle from FSA_write_image_begin_burn 
 * RETURN:   void
**************************************************************************/
T_VOID FSA_write_image_end_burn(T_hFILE handle);

/************************************************************************
 * NAME:     FSA_write_image_burn
 * FUNCTION   write image to disk
 * PARAM: [in] handle     the handle from FSA_write_image_begin_burn 
 *             [in] pData-----image's data pointer addr
 *             [in] data_len--image's data length
 * RETURN:   success return FSA_SUCCESS, fail retuen FSA_ FAIL

**************************************************************************/

T_U32  FSA_write_image_burn(T_hFILE handle, const T_U8 * pData,  T_U32 data_len);



#endif       //_FSA_H_

