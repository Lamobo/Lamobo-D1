#ifndef LINUX_MEDIA_RECORDER_TOOL
#define LINUX_MEDIA_RECORDER_TOOL

#ifdef __cplusplus
extern "C" {
#endif

#include "headers.h"

/**
* @brief   judge the file or dir exists?
* 
* @author hankejia
* @date 2012-07-05
* @param[in] pstrFilePath  file or dir is path.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 IsExists( T_pSTR pstrFilePath );

/**
* @brief   complete create the directory
* 
* @author hankejia
* @date 2012-07-05
* @param[in] pstrRecPath  dir is path.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 CompleteCreateDirectory(  T_pSTR pstrRecPath );

/**
* @brief   delay
*
* @author hankejia
* @date 2012-07-05
* @param[in] sec 		second
* @param[in] usec 	microsecond
* @return NONE
*/
void delay_loop( unsigned long sec, unsigned long usec );

/**
* @brief   get current is time, and convert to string
* 
* @author hankejia
* @date 2012-07-05
* @param NONE
* @return T_pSTR
* @retval if return NULL failed, otherwise the time is string 
*/
T_pSTR GetCurTimeStr();

/**
* @brief   convert time to time string.
* 
* @author wangxi
* @date 2012-07-05
* @param[in] tnow			time struct pointer
* @param[out] pstrTime	time string, format eg: 02/11/23 11:25:15
* @return T_U32
* @retval return the sting length
*/
T_U32 GetCurTimeStampStr(struct tm *tnow, char * pstrTime);


/**
* @brief   get current is time
*
* @author hankejia
* @date 2012-07-05
* @param NONE
* @return struct tm *
* @retval if return NULL failed, otherwise the current time 
*/
struct tm * GetCurTime();

/**
* @brief   connect two string
*
* @author hankejia
* @date 2012-07-05
* @param[in] str1 		first string
* @param[in] str2 		second string
* @return T_pSTR
* @retval if return NULL failed, otherwise the string make from str1 and str2
*/
T_pSTR Unite2Str( T_pSTR str1, T_pSTR str2 );

/**
* @brief   create and open a large file
*
* @author hankejia
* @date 2012-07-05
* @param[in] pstrPath 	file path
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 FileOpen( T_pSTR pstrPath );

/**
* @brief   complete write the data to file
*
* @author hankejia
* @date 2012-07-05
* @param[in] fd 			file fd
* @param[in] pData 		the data will write to file
* @param[in] nDataSize 	data size
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 WriteComplete( T_S32 fd, T_CHR * pData, T_U32 nDataSize );

/**
* @brief   file lock set
*
* @author hankejia
* @date 2012-07-05
* @param[in] fd 			file fd
* @param[in] type 			type, file r/w lock or r/w unlock
* @param[in] whence 		lock file is position, SEEK_SET, SEEK_CUR, SEEK_END
* @param[in] nlen 			lock len, lock data from whence to whence + nlen.
* @param[in] time_ms 		time out ms
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 flock_set( T_S32 fd, T_U32 type, T_U32 whence, T_U32 nlen, T_U32 time_ms );

/**
* @brief   convert rgb to yuv
*
* @author wangxi
* @date 2012-07-05
* @param[in] color888 	RGB888
* @return T_U32
* @retval yuv420
*/
T_U32 ColorConvert_RgbToYuv(T_U32 color888);

T_S64 GetDiskSize( T_pSTR pstrRecPath );

signed long long GetDiskFreeSize( T_pSTR pstrRecPath );


#ifdef __cplusplus
}
#endif

#endif
