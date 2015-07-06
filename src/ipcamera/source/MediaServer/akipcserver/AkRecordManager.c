#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

#include "AkRecordManager.h"
#include "Thread.h"
#include "Tool.h"
#include "log.h"

#define SIGN_FILE_NAME							".LMR_RecordManger_Sign"
#define FILE_SUFFIZ								".avi"
#define FILE_NAME_DIF							"(New)"
#define DIR_SEPARATOR							'/'
#define OUR_FILE_PREFIX							"REC_DV_"
#define INI_RECORDER                            "/etc/jffs2/camera.ini"

static const T_U32 MAX_FILE_PATH_LEN			= 1024UL;
static const T_U32 DISK_SET_ASIDE_SIZE			= 10485760UL;		// 10M
static const T_U32 MAX_FILE_SIZE				= 4190109696UL;	// 4G - 100M
static const T_U32 MAX_DURATION					= 14400000UL;		// 4hour
static const T_U32 MIN_DISK_SIZE_FOR_WRITE		= 67108864UL;		// 64M 

static T_pSTR			g_RecPath = NULL;
static T_BOOL 			g_bIsCyc = AK_TRUE;
static T_BOOL			g_bIsStop = AK_FALSE;
static T_BOOL			g_bIsOpen = AK_FALSE;

static nthread_t		g_ManagerCycTID;

static T_U32			g_nMinLimitSize = 0;
static T_U32			g_EstimateFSize = 0;
static T_U32			g_MaxDuration = 0;
static T_U32			g_MaxFileSize = 0;

static Condition		g_condition;
static Condition		g_conMangerRun;

static T_pSTR MakeFileName();

static T_S32 recmgr_del_file( T_BOOL bNeedLast, T_U16 needRecNum );

static T_S32 RemoveOldestFile( T_BOOL bNeedLast );

static T_S64 GetDiskFreeSize( T_pSTR pstrRecPath, T_S32 *bavail, T_S32 *bsize );

static T_S64 GetOldFilesSize( T_pSTR pstrRecPath );

static T_S32 InitManager( T_pSTR pstrRecPath, T_U32 nRecordDuration );

static T_S32 MakeRecordPath( T_pSTR pstrRecPath, T_BOOL bIsCyc );

int delete_oldest_file(){
    int t = 0;
    if(!g_RecPath){
        g_RecPath = "/mnt";
        t = 1;
    }
    RemoveOldestFile(AK_TRUE);
    if(t)
        g_RecPath= NULL;
    return 0;
}
static T_pVOID recmgr_thread_entry( T_pVOID user )
{
	while (AK_TRUE) {
		Condition_Wait(&g_conMangerRun);
		Condition_Unlock(g_conMangerRun);		
		Condition_Lock(g_condition);
		
		if (g_bIsStop) {
			Condition_Unlock(g_condition);
			break;
		}
		
		Condition_Unlock(g_condition);
		recmgr_del_file(AK_TRUE, 1);
	}
	
	return NULL;
}

T_S32 SetMinRecordLimit( T_U32 nMinLimitSize )
{
	if ( nMinLimitSize < MIN_DISK_SIZE_FOR_WRITE ) {
		printf( "the min limit record size set to recorder manager is less then 64M,\n \
			will use 64M instead!\n" );
		return 0;
	}

	g_nMinLimitSize = nMinLimitSize;
	return 0;
}

T_S32 recmgr_open( T_pSTR pstrRecPath, T_U32 nFileBitRate, 
								T_U32 nRecordDuration, T_BOOL bIsCyc )
{
	T_S32 ret = 0;

	//the record manager already open!
	if (g_bIsOpen)
		return 1;

	g_bIsStop = AK_FALSE;
	g_MaxDuration = 0;
	g_MaxFileSize = 0;
	g_ManagerCycTID = thread_zeroid();

	Condition_Initialize( &g_condition );
	Condition_Initialize( &g_conMangerRun );

	if ( g_nMinLimitSize == 0 )
		g_nMinLimitSize = MIN_DISK_SIZE_FOR_WRITE;
		
	if (!IsExists(pstrRecPath) 
		&& CompleteCreateDirectory(pstrRecPath) < 0 )
		return -1;
	
	g_bIsCyc = bIsCyc;
	
	//Estimate the media file size
	if (bIsCyc)
		g_EstimateFSize = ( nRecordDuration / 1000 ) * ( nFileBitRate / 8 );

//		printf(" the media file size = %d \n", g_EstimateFSize);
	
	ret = InitManager(pstrRecPath, nRecordDuration);
	if ( ret < 0 ) {
		loge( "can't init Manager, the recorder file path = %s \n", pstrRecPath );
		return -1;
	}

	if (g_bIsCyc)
	{
		pthread_attr_t SchedAttr;
		struct sched_param	SchedParam;
		
		memset(&SchedAttr, 0, sizeof(pthread_attr_t));
		memset(&SchedParam, 0, sizeof(SchedParam));
				
		pthread_attr_init( &SchedAttr );				
		SchedParam.sched_priority = 10;	
		pthread_attr_setschedparam( &SchedAttr, &SchedParam );
						
		pthread_attr_setschedpolicy( &SchedAttr, SCHED_RR );
		
		if ( pthread_create( &g_ManagerCycTID, &SchedAttr, recmgr_thread_entry, NULL ) != 0 ) {
			loge( "unable to create a thread for manager cyc record file!\n" );
			pthread_attr_destroy(&SchedAttr);
			return -1;
		}
		
		pthread_attr_destroy(&SchedAttr);
	}

	g_bIsOpen = AK_TRUE;
	
	return 1;
}

REC_FUNC_TYPE GetRecordFunction()
{
	return g_bIsCyc ? REC_FUNC_TYPE_CYC : REC_FUNC_TYPE_NORMAL;
}

T_S32 GetRecordFile(T_pSTR filename)
{
	T_S32 fd = 0;
	T_pSTR pstrFileName = NULL;

#if 0
    if (recmgr_del_file( AK_TRUE, 0 ) < 0) {
		loge( "GetRecordFile::Free Disk Not Enough.\r\n", g_nMinLimitSize );
        return -1;
    }
#endif 
	pstrFileName = MakeFileName();
	if ( NULL == pstrFileName ) {
		return -1;
	}
    
    
	if ( ( fd = FileOpen( pstrFileName ) ) < 0 ) {
		loge( "GetRecordFile::FileOpen error! fd = %d\n", fd );
		free( pstrFileName );
		return -1;
	}
	//循环录制?是则唤醒管理线程。
	if ( g_bIsCyc ) {
		Condition_Signal( &g_conMangerRun );
	}

//	printf("AddNewFile:%s\r\n", pstrFileName);
    
	if ( filename != NULL )
		strcpy( filename, pstrFileName );
	
	free( pstrFileName );

	return fd;
}

T_BOOL ReachLimit( T_U32 nFileLen, T_U32 nFileDuration )
{
	if ( g_MaxDuration != 0 ) {
		if ( nFileDuration >= g_MaxDuration ) {
			return AK_TRUE;
		}
	}
		
	return (nFileLen >= g_MaxFileSize) ? AK_TRUE : AK_FALSE;
}

T_VOID CloseRecordManager()
{
	//record manager no open yet!
	if ( !g_bIsOpen ) {
		return;
	}

	if ( g_bIsCyc ) {
		Condition_Lock( g_condition );
		g_bIsStop = AK_TRUE;
		Condition_Unlock( g_condition );

		Condition_Signal( &g_conMangerRun );

		pthread_join( g_ManagerCycTID, NULL );
		g_bIsStop = AK_FALSE;

		logi( "Cyc record manager thread stop!\n" );
	}
	
	if ( g_RecPath != NULL ) {
		free( g_RecPath );
		g_RecPath = NULL;
	}

	Condition_Destroy( &g_condition );
	Condition_Destroy( &g_conMangerRun );

	g_bIsOpen = AK_FALSE;
}

static T_S64 GetDiskFreeSize( T_pSTR pstrRecPath, T_S32 *bavail, T_S32 *bsize )
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
	*bavail = disk_statfs.f_bavail;
	*bsize = disk_statfs.f_bsize;
	return (T_S64)(disk_statfs.f_bavail) * (T_S64)(disk_statfs.f_bsize)/1000000;
}

T_BOOL IsOurCycFile( T_pSTR pstrFileName )
{
	T_pSTR strPrefix = NULL;
	T_pSTR strSuffiz = NULL;
	T_U32 len = 0;

	len = strlen( pstrFileName );
	//名字固定长度为25不包括结束符.
	if ( len != 17 )
		return AK_FALSE;
	
	if ( ( strPrefix = strstr( pstrFileName, OUR_FILE_PREFIX ) ) == NULL ) {
		return AK_FALSE;
	}

	if ( strPrefix != pstrFileName ) {
		return AK_FALSE;
	}

	if ( ( strSuffiz = strstr( pstrFileName, FILE_SUFFIZ ) ) == NULL ) {
		return AK_FALSE;
	}

	if ( ( strSuffiz - strPrefix ) != 13 ) {
		return AK_FALSE;
	}

	return AK_TRUE;
}

static T_S64 GetOldFilesSize( T_pSTR pstrRecPath )
{
	DIR 			*dirp = NULL;
    struct dirent 	*direntp = NULL; 
	struct stat 	statbuf;
	T_CHR			astrFile[MAX_PATH];
    T_S64 totalSize = 0;

	bzero( astrFile, sizeof( astrFile ) );
	
	if( ( dirp = opendir( pstrRecPath ) ) == NULL ) {  
		loge( "Open Directory %s Error: %s\n", pstrRecPath, strerror(errno) );
		return -1;
	}

	while ( ( direntp = readdir( dirp ) ) != NULL ) {
		if ( direntp->d_name == NULL ) {
			continue;
		}
		
		if ( !( strcmp( direntp->d_name, "." ) ) || 
			 !( strcmp( direntp->d_name, ".." ) ) ||
			 !( strcmp( direntp->d_name, SIGN_FILE_NAME ) ) ) {
			continue;
		}

		if ( !(IsOurCycFile( direntp->d_name )) ) {
			continue;
		}

		bzero( astrFile, sizeof( astrFile ) );
		sprintf( astrFile, "%s%s", pstrRecPath, direntp->d_name );
		
		if ( stat( astrFile, &statbuf ) == -1 ) {
			loge( "Get stat on %s Error:%s\n ", direntp->d_name, strerror(errno) );  
			continue; 
		}

		if ( !S_ISREG( statbuf.st_mode ) ) {
			continue;
		}

        totalSize += statbuf.st_size;
	}

	if ( closedir( dirp ) != 0 ) {
		loge( "close Directory %s Error:%s\n ", pstrRecPath, strerror(errno) );
	}

	return totalSize;
}

//删除循环录制目录中，最老的媒体文件。
//返回: 0 没有删除任何文件
//		   1 成功删除了最老的文件
//		 -1 失败。
static T_S32 RemoveOldestFile( T_BOOL bNeedLast )
{
	DIR 			*dirp = NULL;
    struct dirent 	*direntp = NULL; 
	struct stat 	statbuf;
	T_CHR			astrFile[MAX_PATH], astrTemp[MAX_PATH];
	time_t			stFirstTime = 0x7FFFFFFF;
	T_U32			nFileCnt = 0;

	bzero( astrFile, sizeof( astrFile ) );
	bzero( astrTemp, sizeof( astrTemp ) );
	
	if( ( dirp = opendir( g_RecPath ) ) == NULL ) {  
		loge( "Open Directory %s Error: %s\n", g_RecPath, strerror(errno) );
		return -1;
	}

	while ( ( direntp = readdir( dirp ) ) != NULL ) {
		if ( direntp->d_name == NULL ) {
			continue;
		}
		
		if ( !( strcmp( direntp->d_name, "." ) ) || 
			 !( strcmp( direntp->d_name, ".." ) ) ||
			 !( strcmp( direntp->d_name, SIGN_FILE_NAME ) ) ) {
			continue;
		}

		if ( !(IsOurCycFile( direntp->d_name )) ) {
			continue;
		}

		bzero( astrFile, sizeof( astrFile ) );
		sprintf( astrFile, "%s/%s", g_RecPath, direntp->d_name );
		
		if ( stat( astrFile, &statbuf ) == -1 ) {
			loge( "Get stat on %s Error:%s\n ", direntp->d_name, strerror(errno) );  
			continue; 
		}

		if ( !S_ISREG( statbuf.st_mode ) ) {
			continue;
		}

		++nFileCnt;
		
		if ( stFirstTime >= statbuf.st_mtime ) {
			stFirstTime = statbuf.st_mtime;
			memcpy( astrTemp, astrFile, MAX_PATH );
		}
	}

	if ( closedir( dirp ) != 0 ) {
		loge( "close Directory %s Error:%s\n ", g_RecPath, strerror(errno) );
	}

	//如果需要则保留循环录制目录中的最后一个文件。
	if ( ( bNeedLast ) && ( nFileCnt < 2 ) ) {
		return 0;
	}

	if ( strlen( astrTemp ) == 0 ) {
		return 0;
	}

	if ( remove( astrTemp ) != 0 ) {
		loge( "remove %s Dir Error:%s\n ", astrTemp, strerror(errno) );
	}
	printf("DelOldFile:%s\r\n", astrTemp);
	return 1;
}

static T_S32 recmgr_del_file( T_BOOL bNeedLast, T_U16 needRecNum )
{
	T_S64 iDiskFreeSize = 0, iNeedSize;
	T_S32 ret = 0;
	T_S32 avial, bsize;
	
	GetDiskFreeSize( g_RecPath, &avial, &bsize );
	iDiskFreeSize = (T_S64)(T_U32)(avial) * (T_S64)(T_U32)(bsize);
	if ( iDiskFreeSize < 0 ) {
		printf( "ManageCycRecordFile::get disk %s free size error!\n", g_RecPath );
		return -1;
	}

    //iNeedSize = ((T_S64)g_EstimateFSize * (T_S64)needRecNum) + (T_S64)g_nMinLimitSize;
	iNeedSize = ((T_S64)g_EstimateFSize * (T_S64)needRecNum) + (T_S64)g_EstimateFSize;
	while ( iDiskFreeSize < iNeedSize ) {
		ret = RemoveOldestFile( bNeedLast );
		if ( ret < 0 )
			loge( "ManageCycRecordFile::RemoveOldestFile error!\n" );
			return -1;
		
		if ( 0 == ret )
			return 0;

		GetDiskFreeSize( g_RecPath, &avial, &bsize );
		iDiskFreeSize = (T_S64)(T_U32)(avial) * (T_S64)(T_U32)(bsize);
		if ( iDiskFreeSize < 0 )
			printf( "ManageCycRecordFile::get disk %s free size error!\n", g_RecPath );
			return -1;

//        printf("FreeSpace:%lld/%lld,(Need:%d X %X,%X)\r\n", iDiskFreeSize, iNeedSize, needRecNum, g_EstimateFSize, g_nMinLimitSize);
	}
	
	return 0;
}

static char g_filename[1024];
int ChangFileName( void )
{
	char filename[1024];
	memset(filename, 0x00, 1024);
	strncpy(filename, g_filename, strlen(g_filename)-5);
	printf(" filename %s \n", filename);
	rename(g_filename, filename);
	return 0;
}

int rec_count = 0;

static T_pSTR MakeFileName()
{
	T_CHR astrFileName[30];
	T_pSTR strCompletePath = NULL;
	T_pSTR strAdd = NULL;
	T_S32 iPathLen = 0, iIndex = 0;
	struct tm *tnow = GetCurTime();

	bzero( astrFileName, sizeof( astrFileName ) );

	if ( tnow == NULL ) {
		loge( "can't get current time, make file name will exit!\n" );
		return NULL;
	}

    dictionary * ini = iniparser_load(INI_RECORDER);

    if(ini){
        rec_count = iniparser_getint(ini, "recoder:count", 0);
    }

	if ( g_bIsCyc ) {
		sprintf( astrFileName,"%s%06d%s%s", OUR_FILE_PREFIX, rec_count,
			 FILE_SUFFIZ, FILE_NAME_DIF );
	}else {
		sprintf( astrFileName,"%s%06d%s", "DV_", rec_count, FILE_SUFFIZ );
	}

    rec_count++;
    if(rec_count > 999999)
        rec_count = 1;
    if(ini){
        char cct[16];
        sprintf(cct,"%d", rec_count);
        iniparser_set(ini,"recoder:count",cct);
        FILE *fp = fopen(INI_RECORDER, "w");
        if(fp){
            iniparser_dump_ini(ini, fp);
            fclose(fp);
        }
        iniparser_freedict(ini);
    }

    /*      
	if ( g_bIsCyc ) {
		sprintf( astrFileName,"%s%4d%02d%02d%02d%02d%02d%s%s", OUR_FILE_PREFIX,
			1900 + tnow->tm_year, tnow->tm_mon + 1, tnow->tm_mday, tnow->tm_hour, 
			tnow->tm_min, tnow->tm_sec, FILE_SUFFIZ, FILE_NAME_DIF );
	}else {
		sprintf( astrFileName,"%s%02d%02d%02d%s", "DV_", tnow->tm_hour, tnow->tm_min,
			tnow->tm_sec, FILE_SUFFIZ );
	}
    */
	//加上结束位，和5个同名情况下要增加的字符."_1~1024"
	iPathLen = strlen( g_RecPath ) + strlen( astrFileName ) + 6;
	strCompletePath = (T_pSTR)malloc( iPathLen );
	if ( NULL == strCompletePath ) {
		loge( "MakeFileName::out of memory!\n" );
		return (char*)NULL;
	}

	bzero( strCompletePath, iPathLen );

	strcpy( strCompletePath, g_RecPath );
	strcat( strCompletePath, astrFileName );

	strAdd = strrchr( strCompletePath, '.' );
	while (IsExists(strCompletePath)) {
		++iIndex;
		bzero( strAdd, strlen( strAdd ) );
		sprintf( strAdd, "_%d", (int)iIndex );
		strcat( strCompletePath, FILE_SUFFIZ );
		
		if ( iIndex > 1024 ) {
			logi( "the dir is file full!\n" );
			break;
		}
	}
	
	memset(g_filename, 0x00, 1024);
	memcpy(g_filename, strCompletePath, strlen(strCompletePath));
	return strCompletePath;
}

/*
static T_S32 CanRecord( T_pSTR pstrRecPath )
{
	T_S64 iDiskFreeSize = 0;
	
	assert( pstrRecPath );

	iDiskFreeSize = GetDiskFreeSize( pstrRecPath );
	if ( iDiskFreeSize < 0 ) {
		loge( "CanRecord::get disk free size error!\n" );
		return -1;
	}
	
	if ( iDiskFreeSize <= g_nMinLimitSize ) {
		return 0;
	}
	
	return 1;
}
*/

/*
	nRecordDuration : file time

*/
static T_S32 InitManager( T_pSTR pstrRecPath, T_U32 nRecordDuration )
{
	T_pSTR strSignFile = NULL;
	T_S32 fd = 0;
	T_S64 iDiskFreeSize = 0, iCompareTemp = 0;
    T_BOOL isFirstTimes;
	T_S32 avial, bsize;
	if ( NULL == pstrRecPath ) {
		loge( "InitManager::invalid parameter!\n" );
		return -1;
	}

	if ( MakeRecordPath( pstrRecPath, g_bIsCyc ) < 0 ) {
		return -1;
	}
    
	if ( g_bIsCyc ) {
        // if cyc, enable time limit and space limit
        g_MaxDuration = nRecordDuration;
        
        // 如果估算大小小于最小写数据长度
        if (g_EstimateFSize < MIN_DISK_SIZE_FOR_WRITE) {
            g_EstimateFSize = MIN_DISK_SIZE_FOR_WRITE;
        }
        
        strSignFile = Unite2Str( g_RecPath, SIGN_FILE_NAME );
		if ( strSignFile == NULL ) {
			return -1;
		}

		//有标志文件，证明当前文件夹和上次进行循环
		//录制的文件夹是同一个。
		if ( IsExists( strSignFile ) ) {
            isFirstTimes = AK_FALSE;
            // this directory record file is not  first times
		} else {
            isFirstTimes = AK_TRUE;
            // record first times 
			if ( ( fd = FileOpen( strSignFile ) ) < 0 ) {
				loge( "InitManager::can't create the sign file!\n" );
				free( strSignFile );
				return -1;
			}

			WriteComplete( fd, (T_U8*)SIGN_FILE_NAME, strlen( SIGN_FILE_NAME ) + 1 );
			fsync( fd );
			close( fd );
		}

		free( strSignFile );
		
		 GetDiskFreeSize( g_RecPath, &avial, &bsize );
		 iDiskFreeSize = (T_S64)(T_U32)(avial) * (T_S64)(T_U32)(bsize);
		if ( iDiskFreeSize < 0 ) {
			printf( "InitManager::get disk %s free size error!\n", g_RecPath );
			return -1;
		}

        // get max file size
        iCompareTemp = (iDiskFreeSize + GetOldFilesSize(g_RecPath) - g_nMinLimitSize )/2;
        if (iCompareTemp >= MAX_FILE_SIZE) {
            iCompareTemp = MAX_FILE_SIZE;
        }
        g_MaxFileSize = iCompareTemp;

//		printf("g_maxfilesize = %d \n", g_MaxFileSize);
        //如果估算大小比空闲空间的一半还大
        //使用的是剩余空间/2作为循环录制文件的大小。
        if (g_EstimateFSize > g_MaxFileSize) {
            g_EstimateFSize = g_MaxFileSize;
        }
        
        recmgr_del_file( !isFirstTimes, 1 );
        
	} else {
        // if normal, just enable space limit
		GetDiskFreeSize( g_RecPath, &avial, &bsize );
		iDiskFreeSize = (T_S64)(T_U32)(avial) * (T_S64)(T_U32)(bsize);
		if ( iDiskFreeSize < (T_S64)g_nMinLimitSize ) {
			loge( "InitManager::get disk %s free size error!\n", g_RecPath );
			return -1;
		}
		iCompareTemp = (T_S64)MAX_FILE_SIZE + (T_S64)g_nMinLimitSize;
		if ( iDiskFreeSize > iCompareTemp ) {
			g_MaxFileSize = MAX_FILE_SIZE;
		}else {
			g_MaxFileSize = iDiskFreeSize - g_nMinLimitSize;
		}
	}
	GetDiskFreeSize( g_RecPath, &avial, &bsize );
	iDiskFreeSize = (T_S64)(T_U32)(avial) * (T_S64)(T_U32)(bsize);

    if (iDiskFreeSize < g_nMinLimitSize) {
//        printf( "InitManager::disk Free(%lld) less than minilimit(%d)\n", iDiskFreeSize, g_nMinLimitSize );
        return -1;
    }

//    printf( "InitManager:CtrlParam: %s Max=0x%X,Estimate=0x%X,MinLimit=0x%X\r\n", 
//        g_RecPath, MAX_FILE_SIZE,  g_EstimateFSize, g_nMinLimitSize);
	return 0;
}

static T_S32 MakeRecordPath( T_pSTR pstrRecPath, T_BOOL bIsCyc )
{
	T_S32 iPathLen = 0, iAddLen = 0;
	struct tm * tnow = NULL;
	T_CHR astrDate[10];
	bzero( astrDate, sizeof( astrDate ) );
	
	if ( NULL != g_RecPath ) {
		free( g_RecPath );
		g_RecPath = NULL;
	}

	iPathLen = strlen( pstrRecPath );
	iAddLen = 1;

	//需求
	//普通录像的录制路径为设置的路径下的yyyymmdd文件夹。
	//循环录像的录制路径为设置的路径。
	if ( !bIsCyc ) {
		tnow = GetCurTime();
		if ( NULL == tnow ) {
			loge( "get current time error!\n");
			return -1;
		}

		sprintf( astrDate, "%4d%02d%02d/", 1900 + tnow->tm_year, 
				 tnow->tm_mon + 1, tnow->tm_mday );
		
		iAddLen = 10;
	}

	if ( pstrRecPath[iPathLen - 1] != DIR_SEPARATOR ) {
		iPathLen += 1;
	}

	g_RecPath = (char *)malloc( iPathLen + iAddLen );
	if ( NULL == g_RecPath ) {
		loge( "MakeRecordPath::out of memory!\n" );
		return -1;
	}

	memset( g_RecPath, 0, iPathLen + iAddLen );
	strcpy( g_RecPath, pstrRecPath );
	g_RecPath[iPathLen - 1] = DIR_SEPARATOR;

	if ( !bIsCyc ) {
		strcpy( g_RecPath + iPathLen, astrDate );

		//创建普通录像的文件夹。
		if ( !IsExists( g_RecPath ) ) {
			if ( mkdir( g_RecPath, S_IRWXU | S_IRWXG | S_IRWXO ) != 0 ) {
				loge( "MakeRecordPath::can't create dir %s, error = %s", g_RecPath, strerror(errno) );
				return -1;
			}
		}
	}
	
	return 0;
}

// End of File
