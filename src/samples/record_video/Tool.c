#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>
#include <fcntl.h>

#include "Tool.h"
#include "log.h"

/**
* @brief   judge the file or dir exists?
* 
* @author hankejia
* @date 2012-07-05
* @param[in] pstrFilePath  file or dir is path.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 IsExists( T_pSTR pstrFilePath )
{
	if ( NULL == pstrFilePath )
		return 0;

	if ( access( pstrFilePath, F_OK ) == 0 )
		return 1;

	return 0;
}


/**
* @brief   complete create the directory
* 
* @author hankejia
* @date 2012-07-05
* @param[in] pstrRecPath  dir is path.
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 CompleteCreateDirectory( T_pSTR pstrRecPath )
{
	T_S32 iRet = 0;
	T_CHR *pstrTemp = NULL, *pszBackSlash = NULL;
	
	assert( pstrRecPath != NULL );
	
	T_pSTR pstrPath = (T_pSTR)malloc( ( strlen( pstrRecPath ) + 1 ) );
	if ( NULL == pstrRecPath ) {
		loge( "CompleteCreateDirectory::Out of memory!\n" );
		return -1;
	}

	strcpy( pstrPath, pstrRecPath );

	pstrTemp = strchr( pstrPath, '/' );

	while( 1 ) {
		pszBackSlash = strchr( pstrTemp + 1, '/' );
		if ( NULL == pszBackSlash )
			break;
		
		*pszBackSlash= '\0';

		if ( IsExists( pstrPath ) ) {
			*pszBackSlash = '/';
			pstrTemp = pszBackSlash;
			continue;
		}

		if ( mkdir( pstrPath, S_IRWXU | S_IRWXG | S_IRWXO ) != 0 ) {
			loge( "CompleteCreateDirectory::can't create dir %s, error = %s", pstrPath, strerror(errno) );
			iRet = -1;
			goto Exit;
		}

		*pszBackSlash = '/';
        pstrTemp = pszBackSlash;
	}

	if ( ( mkdir( pstrPath, S_IRWXU | S_IRWXG | S_IRWXO ) != 0 ) && ( errno != EEXIST ) ) {
		loge( "CompleteCreateDirectory::can't complete create dir %s! error = %s!\n", pstrPath, strerror(errno) );
		iRet = -1;
	}
	
Exit:
	free( pstrPath );
	return iRet;
}

#define MAX_TIME_STRING		25


/**
* @brief   get current is time, and convert to string
* 
* @author hankejia
* @date 2012-07-05
* @param NONE
* @return T_pSTR
* @retval if return NULL failed, otherwise the time is string 
*/
T_pSTR GetCurTimeStr()
{
	char * pstrTime= NULL;

	struct tm *tnow = GetCurTime();
	if ( NULL == tnow ) {
		loge( "getCurTimeStr::GetCurTime error!\n" );
		return (T_pSTR)NULL;
	}

	pstrTime = (char *)malloc( MAX_TIME_STRING );
	if ( NULL == pstrTime ) {
		loge( "getCurTimeStr::Out of memory!\n" );
		return (T_pSTR)NULL;
	}

	memset( pstrTime, 0, MAX_TIME_STRING );
	sprintf( pstrTime, "%4d,%02d,%02d-%02d_%02d_%02d", 1900 + tnow->tm_year, tnow->tm_mon + 1,
			 tnow->tm_mday, tnow->tm_hour, tnow->tm_min, tnow->tm_sec );

	return pstrTime;
}


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
T_U32 GetCurTimeStampStr(struct tm *tnow, char * pstrTime)
{
	if ( NULL == tnow ) {
		loge( "getCurTimeStr::GetCurTime error!\n" );
		return 0;
	}

	if ( NULL == pstrTime ) {
		loge( "getCurTimeStr::Out of memory!\n" );
		return 0;
	}

	sprintf( pstrTime, "%02d/%02d/%02d %02d:%02d:%02d", (1900 + tnow->tm_year)%100, tnow->tm_mon + 1,
			 tnow->tm_mday, tnow->tm_hour, tnow->tm_min, tnow->tm_sec );

	return (6+2+1+6+2);
}

/**
* @brief   get current is time
*
* @author hankejia
* @date 2012-07-05
* @param NONE
* @return struct tm *
* @retval if return NULL failed, otherwise the current time 
*/
struct tm * GetCurTime()
{
	time_t now = { 0 };
	now = time(0);

	struct tm *tnow = localtime(&now);
	if ( NULL == tnow ) {
		loge( "GetCurTime::get local time error!\n" );
		return NULL;
	}

	return tnow;
}

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
T_pSTR Unite2Str( T_pSTR str1, T_pSTR str2 )
{
	T_pSTR pstrDes = NULL;
	T_S32 iDesLen = 0;

	iDesLen = strlen( str1 ) + strlen( str2 ) + 1;

	pstrDes = (T_pSTR)malloc( iDesLen );
	if ( NULL == pstrDes ) {
		loge( "Unite2Str::out of memory!\n" );
		return NULL;
	}

	bzero( pstrDes, iDesLen );

	strcpy( pstrDes, str1 );
	strcat( pstrDes, str2 );
	
	return pstrDes;
}


/**
* @brief   delay
*
* @author hankejia
* @date 2012-07-05
* @param[in] sec 		second
* @param[in] usec 	microsecond
* @return NONE
*/
void delay_loop( unsigned long sec, unsigned long usec )
{
	struct timespec requested, remaining;

	sec = sec + ( usec / 1000000 );
	usec %= 1000000;

	requested.tv_sec  = sec;
	requested.tv_nsec = usec * 1000L;

	while (nanosleep(&requested, &remaining) == -1){
		if (errno == EINTR)
			requested = remaining;
		else {
			loge( "nanosleep error! error = %s", strerror( errno ) );
			break;
		}
	}
}


/**
* @brief   create and open a large file
*
* @author hankejia
* @date 2012-07-05
* @param[in] pstrPath 	file path
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
T_S32 FileOpen( T_pSTR pstrPath )
{
	mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
	T_S32 fd = 0;
	
	assert( pstrPath );
	
	fd = open( pstrPath, O_LARGEFILE | O_RDWR | O_CREAT, mode );
	if ( fd < 0 ) {
		loge( "FileOpen::can't not open file %s! error = %s", pstrPath, strerror(errno) );
		return -1;
	}
	
	return fd;
}


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
T_S32 WriteComplete( T_S32 fd, T_CHR* pData, T_U32 nDataSize )
{
	T_S32 n = 0, sent = 0;

	if ( fd < 0 ) {
		loge( "WriteComplete::Invalid fd parameter\n" );
		return -1;
	}

	do {
		n = write( fd, pData + sent, nDataSize - sent );
		if ( n < 0 ) {
			if ( errno == EAGAIN ) {
				delay_loop( 0, 1000 ); // 1ms delay
				continue;
			}
			
			loge( "WriteComplete::write file error! error = %s\n", strerror(errno) );
			return -1;
		} else {
			sent += n;
		}
		
	}while( sent < nDataSize );
	
	return 0;
}


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
T_S32 flock_set( T_S32 fd, T_U32 type, T_U32 whence, T_U32 nlen, T_U32 time_ms )
{
	struct flock lock;
	T_U32 delay_ms_once = 10 * 1000UL, delay_ms = 0;
	
	lock.l_type = type;
	lock.l_whence = whence;
	lock.l_start = 0;
	lock.l_len = nlen;

	if ( time_ms > 0 ) //timeout wait
	{
		//Since there is no timeout API for fcntl(), So use while loop
		//to implement time out waiting.
		while( fcntl( fd, F_SETLK, &lock ) < 0 )
		{
			delay_loop( 0, delay_ms ); // delay 10 ms
			delay_ms += delay_ms_once;
			if ( delay_ms >= time_ms ) {
				return 1; // return 1, time out
			}
		}
	}
	else if ( time_ms == 0 )
	{
		if ( fcntl( fd, F_SETLK, &lock ) < 0 )
		{
			if ( fcntl( fd, F_GETLK, &lock ) < 0 ) {
				loge( "fcntl set %d unknown error! %s\n", type, strerror(errno) );
				return -1;
			}

			if ( lock.l_type == F_RDLCK ) {
				logi( "read lock already set by %d\n", lock.l_pid );
				return 2; //return 2 the operation already set by another program
			}

			if ( lock.l_type == F_WRLCK ) {
				logi( "write lock already set by %d\n", lock.l_pid );
				return 2;//return 2 the operation already set by another program
			}

			loge( "fcntl set %d unknown error! %s\n", type, strerror(errno) );
			return -1;
		}
	}
	else //block wait
	{
		if ( fcntl( fd, F_SETLKW, &lock ) < 0 ) {
			loge( "fcntl set %d block error! %s\n", type, strerror(errno) );
			return -1;
		}
	}

	return 0;
}


#define RGB2YUV(yy, uu, vv, rr, gg, bb)   {\
    (yy) = (77 * (rr) + 150* (gg) + 29 * (bb))>>8;\
    (uu) = ((-43 * (rr) - 85* (gg) + 128 *(bb))>>8) + 128;\
    (vv) = ((128 * (rr) - 107* (gg) - 21 *(bb))>>8) + 128;\
}

/**
* @brief   convert rgb to yuv
*
* @author wangxi
* @date 2012-07-05
* @param[in] color888 	RGB888
* @return T_U32
* @retval yuv420
*/
T_U32 ColorConvert_RgbToYuv(T_U32 color888)
{
	T_U8 R = 0;
	T_U8 G = 0;
	T_U8 B = 0;
    
	T_U8 Y = 0;
	T_U8 U = 0;
	T_U8 V = 0;
    
    B = (T_U8)color888;         // B
    G = (T_U8)(color888 >> 8);  // G
    R = (T_U8)(color888 >> 16); // R

    RGB2YUV(Y, U, V, R, G, B);

    return ((V<<16) | (U<<8) | Y);
}

T_S64 GetDiskSize( T_pSTR pstrRecPath )
{
	struct statfs disk_statfs;
	
	assert( pstrRecPath );

	bzero( &disk_statfs, sizeof( struct statfs ) );

	while ( statfs( pstrRecPath, &disk_statfs ) == -1 ) {
		if ( errno != EINTR ) {
			loge( "statfs: %s Last error == %s\n", pstrRecPath, strerror(errno) );
			return -1;
		}
	}

	return (T_S64)(disk_statfs.f_blocks) * (T_S64)(disk_statfs.f_bsize);
}


signed long long  GetDiskFreeSize( T_pSTR pstrRecPath )
{
	struct statfs disk_statfs;
	
	assert( pstrRecPath );

	bzero( &disk_statfs, sizeof( struct statfs ) );

	while ( statfs( pstrRecPath, &disk_statfs ) == -1 ) {
		if ( errno != EINTR ) {
			loge( "statfs: %s Last error == %s\n", pstrRecPath, strerror(errno) );
			return -1;
		}
	}
	//printf("f_bavail = %lld, f_bsize = %lld", disk_statfs.f_bavail, disk_statfs.f_bsize);
	return (signed long long)(disk_statfs.f_bavail) * (signed long long)(disk_statfs.f_bsize) >> 20;
}

