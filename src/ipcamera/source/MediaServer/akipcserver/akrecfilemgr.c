#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

#include "akrecfilemgr.h"
#include "Thread.h"
#include "Tool.h"
#include "log.h"

static const T_U32 MAX_FILE_SIZE				= ((4UL<<30)-(100UL<<20));	// 4G - 100M
static const T_U32 MIN_DISK_SIZE_FOR_WRITE		= (64UL<<20);		// 64M 

typedef T_BOOL (*FILE_QUERY_CB)(T_pSTR pFileName);

typedef struct RecorderFileMgr{
	T_BOOL bOpen;
	T_BOOL bCycRec;
	T_pSTR szRecPath;
	T_U32  nMinSize;
	T_U32  nMaxSize;
	T_U32  nEstSize;
	T_U32  nMaxDuration;

}T_REC_FILE;

T_REC_FILE *recFile = NULL;


static T_S32 RecFile_delFile(T_BOOL bNeedLast, T_U16 needRecNum);


static T_S32 RecFile_init(T_pSTR pRecPath, T_U32 nRecordDuration);

static T_pSTR RecFile_makeName(T_pSTR path, T_BOOL cycRec)
{
	T_CHR fileName[30];
	T_pSTR pathName = NULL;
	T_pSTR strAdd = NULL;
	T_S32 iPathLen = 0, iIndex = 0;
	struct tm *tnow = GetCurTime();

	bzero(fileName, sizeof(fileName));

	if (tnow == NULL) {
		loge( "can't get current time, make file name will exit!\n" );
		return NULL;
	}

	// Create File Name
	if (cycRec) {
		sprintf(fileName,"%s%4d%02d%02d%02d%02d%02d%s%s", CYC_FILE_PREFIX,
			1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday,
			tnow->tm_hour, tnow->tm_min, tnow->tm_sec, FILE_SUFFIX, FILE_NAME_DIF);
	}else {
		sprintf(fileName,"%s%02d%02d%02d%s", REC_FILE_PREFIX, 
			tnow->tm_hour, tnow->tm_min, tnow->tm_sec, FILE_SUFFIX);
	}

	//加上结束位，和5个同名情况下要增加的字符."_1~1024"
	iPathLen = strlen(path) + strlen(fileName) + 6;
	pathName = (T_pSTR)malloc(iPathLen);
	if (NULL == pathName) {
		loge( "MakeFileName::out of memory!\n" );
		return (char*)NULL;
	}

	bzero(pathName, iPathLen);

	strcpy(pathName, path);
	strcat(pathName, fileName);

	strAdd = strrchr(pathName, '.');
	while (IsExists(pathName)) {
		bzero(strAdd, strlen(strAdd));
		sprintf(strAdd, "_%ld", ++iIndex);
		strcat(pathName, FILE_SUFFIX);
		
		if (iIndex > 1024) {
			logi( "the dir is file full!\n" );
			break;
		}
	}

	if (iIndex > 0)
		strcat(pathName, FILE_NAME_DIF);
	
	return pathName;
}


static T_pSTR RecFile_makeDir(T_pSTR pRecPath, T_BOOL bCycRec)
{
	T_pSTR path = NULL;
	T_S32 iPathLen = 0, iAddLen = 0;
	struct tm *tnow = NULL;
	T_CHR astrDate[10];
	bzero(astrDate, sizeof(astrDate));

	iPathLen = strlen(pRecPath);
	iAddLen = 1;

	//需求
	//普通录像的录制路径为设置的路径下的yyyymmdd文件夹。
	//循环录像的录制路径为设置的路径。
	if (!bCycRec) {
		tnow = GetCurTime();
		if (NULL == tnow) {
			loge( "get current time error!\n");
			return NULL;
		}

		sprintf(astrDate, "%4d%02d%02d/", 1900+tnow->tm_year, tnow->tm_mon+1, tnow->tm_mday);		
		iAddLen = 10;
	}

	if (pRecPath[iPathLen - 1] != DIR_SEPARATOR) {
		iPathLen += 1;
	}

	path = (T_pSTR)malloc(iPathLen + iAddLen);
	if (NULL == path) {
		loge( "MakeRecordPath::out of memory!\n" );
		return NULL;
	}

	memset(path, 0, iPathLen+iAddLen);
	strcpy(path, pRecPath);
	path[iPathLen - 1] = DIR_SEPARATOR;

	if (!bCycRec) {
		strcpy(path + iPathLen, astrDate);

		//创建普通录像的文件夹。
		if (!IsExists(path)
			&& mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO)) {
			loge( "MakeRecordPath::can't create dir %s, error = %s", path, strerror(errno) );
			return NULL;
		}
	}
	
	return path;
}


static T_S64 RecFile_calcFilesSize(T_pSTR pRecPath, FILE_QUERY_CB query)
{
	DIR 			*dirp = NULL;
    struct dirent 	*direntp = NULL; 
	struct stat 	statbuf;
	T_CHR			astrFile[MAX_PATH];
    T_S64 totalSize = 0;

	bzero( astrFile, sizeof( astrFile ) );
	
	if(!(dirp = opendir(pRecPath))) {  
		loge( "Open Directory %s Error: %s\n", pRecPath, strerror(errno) );
		return -1;
	}

	while ((direntp = readdir(dirp)) != NULL) {
		if (direntp->d_name == NULL) {
			continue;
		}
		
		if (!strcmp(direntp->d_name, "." )
			|| !strcmp(direntp->d_name, "..")
			|| !strcmp(direntp->d_name, SIGN_FILE_NAME)) {
			continue;
		}

		if (!query(direntp->d_name)) {
			continue;
		}

		bzero(astrFile, sizeof(astrFile));
		sprintf(astrFile, "%s%s", pRecPath, direntp->d_name);
		
		if (stat(astrFile, &statbuf) == -1) {
			loge( "Get stat on %s Error:%s\n ", direntp->d_name, strerror(errno) );  
			continue; 
		}

		if (!S_ISREG(statbuf.st_mode)) {
			continue;
		}

        totalSize += statbuf.st_size;
	}

	if (closedir(dirp) != 0) {
		loge( "close Directory %s Error:%s\n ", pRecPath, strerror(errno) );
	}

	return totalSize;
}


T_S32 RecFile_setMinSize(T_U32 nMinLimitSize )
{
	if (!recFile){
		printf("ERR: recFile NOT Init in %s", __func__);
		return -1;
	}
		
	if (nMinLimitSize < MIN_DISK_SIZE_FOR_WRITE) {
		printf( "the min limit record size set to recorder manager is less then 64M,\n \
			will use 64M instead!\n" );
		return 0;
	}

	recFile->nMinSize = nMinLimitSize;
	return 0;
}

T_S32 RecFile_open(T_pSTR pRecPath, T_U32 nFileBitRate, T_U32 nRecDuration_ms, T_BOOL bIsCyc )
{
	T_S32 ret = 0;

	if (NULL == pRecPath) {
		loge( "InitManager::invalid parameter!\n" );
		return -1;
	}
	
	if (!IsExists(pRecPath) 
		&& CompleteCreateDirectory(pRecPath) < 0 )
		return -1;
	
	//the record manager already open!
	if (recFile)
		return 0;

	recFile = (T_REC_FILE*)malloc(sizeof(T_REC_FILE));
	if (NULL == recFile)
		return -1;

	memset(recFile, 0, sizeof(T_REC_FILE));

	recFile->bCycRec = bIsCyc;
	//Estimate the media file size
	recFile->nEstSize = (nRecDuration_ms / 1000) * (nFileBitRate / 8);
	recFile->nMaxDuration = nRecDuration_ms;
	
	if ( recFile->nMinSize == 0 )
		recFile->nMinSize = MIN_DISK_SIZE_FOR_WRITE;
		

	ret = RecFile_init(pRecPath, nRecDuration_ms);
	if ( ret < 0 ) {
		loge( "can't init Manager, the recorder file path = %s \n", pRecPath );
		return -1;
	}

	return 1;
}


T_BOOL RecFile_limitSize(T_U32 nFileLen, T_U32 nFileDuration)
{	
	if (recFile->nMaxDuration != 0
		&& nFileDuration >= recFile->nMaxDuration) {
			return AK_TRUE;
	}

	return (nFileLen >= recFile->nMaxSize) ? AK_TRUE : AK_FALSE;
}

T_VOID RecFile_free()
{
	//record manager no open yet!
	if (!recFile) {
		return;
	}

	free(recFile->szRecPath);
	free(recFile);
	recFile = NULL;
}

T_BOOL RecFile_isCycRec(T_VOID)
{
	if (recFile)
		return recFile->bCycRec;

	return AK_FALSE;
}
static T_S32 RecFile_delFile(T_BOOL bNeedLast, T_U16 needRecNum )
{
	T_S64 iDiskFreeSize = 0, iNeedSize;
	T_S32 ret = 0;
	T_S32 avial, bsize;
	
	DiskFreeSize(recFile->szRecPath, &avial, &bsize );
	iDiskFreeSize = (T_S64)(T_U32)(avial) * (T_S64)(T_U32)(bsize);
	if ( iDiskFreeSize < 0 ) {
		printf( "ManageCycRecordFile::get disk %s free size error!\n", recFile->szRecPath );
		return -1;
	}

	iNeedSize = ((T_S64)recFile->nEstSize * (T_S64)needRecNum) + (T_S64)recFile->nEstSize;
	while (iDiskFreeSize < iNeedSize) {
		ret = RecFile_removeOldFile(recFile->szRecPath, bNeedLast);
		if (ret < 0){
			loge("ManageCycRecordFile::RemoveOldestFile error!\n" );
			printf("NOT DEL FILE!!!\n");
			return -1;
		}
		if ( 0 == ret )
			return 0;

		DiskFreeSize(recFile->szRecPath, &avial, &bsize);
		iDiskFreeSize = (T_S64)avial * (T_S64)bsize;
		if (iDiskFreeSize < 0){
			printf( "ManageCycRecordFile::get disk %s free size error!\n", recFile->szRecPath );
			return -1;
		}
	}
	
	return 0;
}

	
/*
	nRecordDuration : file time
*/
static T_S32 RecFile_init(T_pSTR pRecPath, T_U32 nRecordDuration)
{
	T_pSTR strSignFile = NULL;
	T_S32 fd = 0;
	T_S64 iDiskFreeSize = 0, iCompareTemp = 0;
    T_BOOL isFirstTimes;
	T_S32 avial, bsize;

	if (!(recFile->szRecPath = RecFile_makeDir(pRecPath, recFile->bCycRec))) {
		return -1;
	}
    
	if (recFile->bCycRec) {
        // if cyc, enable time limit and space limit
        recFile->nMaxDuration = nRecordDuration;
        
        // 如果估算大小小于最小写数据长度
        if (recFile->nEstSize < MIN_DISK_SIZE_FOR_WRITE) {
            recFile->nEstSize = MIN_DISK_SIZE_FOR_WRITE;
        }
        
        strSignFile = Unite2Str(recFile->szRecPath, SIGN_FILE_NAME);
		if ( strSignFile == NULL ) {
			return -1;
		}

		//有标志文件，证明当前文件夹和上次进行循环
		//录制的文件夹是同一个。
		if (IsExists(strSignFile)) {
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
		
		DiskFreeSize(recFile->szRecPath, &avial, &bsize);
		iDiskFreeSize = (T_S64)(avial) * (T_S64)(bsize);
		if ( iDiskFreeSize < 0 ) {
			printf( "InitManager::get disk %s free size error!\n", recFile->szRecPath );
			return -1;
		}

        // get max file size
        iCompareTemp = (iDiskFreeSize + RecFile_calcFilesSize(recFile->szRecPath, RecFile_isCycFile) - recFile->nMinSize )/2;
        if (iCompareTemp >= MAX_FILE_SIZE) {
            iCompareTemp = MAX_FILE_SIZE;
        }
		
        recFile->nMaxSize = iCompareTemp;

//		printf("g_maxfilesize = %d \n", g_MaxFileSize);
        //如果估算大小比空闲空间的一半还大
        //使用的是剩余空间/2作为循环录制文件的大小。
        if (recFile->nEstSize > recFile->nMaxSize) {
            recFile->nEstSize = recFile->nMaxSize;
        }
        
        RecFile_delFile(!isFirstTimes, 1);
        
	}
	else {
        // if normal, just enable space limit
		DiskFreeSize(recFile->szRecPath, &avial, &bsize );
		iDiskFreeSize = (T_S64)(T_U32)(avial) * (T_S64)(T_U32)(bsize);
		if ( iDiskFreeSize < (T_S64)recFile->nMinSize ) {
			loge( "InitManager::get disk %s free size error!\n", recFile->szRecPath);
			return -1;
		}
		iCompareTemp = (T_S64)MAX_FILE_SIZE + (T_S64)recFile->nMinSize;
		if ( iDiskFreeSize > iCompareTemp ) {
			recFile->nMaxSize = MAX_FILE_SIZE;
		}else {
			recFile->nMaxSize = iDiskFreeSize - recFile->nMinSize;
		}
	}
	
	DiskFreeSize(recFile->szRecPath, &avial, &bsize );
	iDiskFreeSize = (T_S64)(T_U32)(avial) * (T_S64)(T_U32)(bsize);

    if (iDiskFreeSize < recFile->nMinSize) {
        return -1;
    }

	return 0;
}


T_S32 RecFile_openFile(T_pSTR *filename)
{
	T_S32 fd = 0;

    if (RecFile_delFile(AK_TRUE, 1) < 0) {
		loge( "GetRecordFile::Free Disk Not Enough.\r\n", recFile->nMinSize );
        return -1;
    }

	*filename = RecFile_makeName(recFile->szRecPath, recFile->bCycRec);
	if (NULL == *filename) {
		printf("Make File Name ERR\n");
		return -1;
	}

	if ((fd = FileOpen(*filename)) < 0) {
		printf("FileOpen error! fd = %d\n", fd );
		free(*filename);
		*filename = NULL;
		return -1;
	}
    
	return fd;
}

//删除循环录制目录中，最老的媒体文件。
//返回: 0 没有删除任何文件
//		   1 成功删除了最老的文件
//		 -1 失败。
T_S32 RecFile_removeOldFile(T_pSTR path, T_BOOL bNeedLast)
{
	DIR 			*dirp = NULL;
    struct dirent 	*direntp = NULL; 
	struct stat 	statbuf;
	T_CHR			astrFile[MAX_PATH], astrTemp[MAX_PATH];
	time_t			stFirstTime = 0x7FFFFFFF;
	T_U32			nFileCnt = 0;

	bzero(astrFile, sizeof(astrFile));
	bzero(astrTemp, sizeof(astrTemp));
	
	if(!(dirp = opendir(path))) {  
		loge( "Open Directory %s Error: %s\n", path, strerror(errno) );
		printf("Opend DIR ERR\n");
		return -1;
	}

	while (NULL != (direntp = readdir(dirp))) {
		if (direntp->d_name == NULL)
			continue;
		
		if (!strcmp(direntp->d_name, ".")
			|| !strcmp(direntp->d_name, "..")
			|| !strcmp(direntp->d_name, SIGN_FILE_NAME)) {
			continue;
		}

		if (!RecFile_isCycFile(direntp->d_name))
			continue;

		bzero(astrFile, sizeof(astrFile));
		if (path[strlen(path)-1] == DIR_SEPARATOR)
			sprintf(astrFile, "%s%s", path, direntp->d_name);
		else
			sprintf(astrFile, "%s%c%s", path, DIR_SEPARATOR, direntp->d_name);
		
		if (stat(astrFile, &statbuf ) == -1) {
			loge( "Get stat on %s Error:%s\n ", direntp->d_name, strerror(errno) );  
			continue; 
		}

		if (!S_ISREG(statbuf.st_mode))
			continue;

		++nFileCnt;
		
		// Find Min Time Stamp File
		if (stFirstTime >= statbuf.st_mtime) {
			stFirstTime = statbuf.st_mtime;
			memcpy(astrTemp, astrFile, MAX_PATH);
		}
	}
	
	if (closedir(dirp) != 0)
		loge("close Directory %s Error:%s\n ", path, strerror(errno));

	//如果需要则保留循环录制目录中的最后一个文件。
	if (bNeedLast && nFileCnt < 2)
		return 0;

	if (strlen(astrTemp) == 0)
		return 0;

	if (remove(astrTemp) != 0)
		loge("remove %s Dir Error:%s\n ", astrTemp, strerror(errno));

	printf("DelOldFile: %s\r\n", astrTemp);
	
	return 1;
}

T_BOOL RecFile_isCycFile(T_pSTR pFileName)
{
	T_pSTR strPrefix = NULL;
	T_pSTR strSuffix = NULL;

	//名字固定长度为大于/等于25(不包括结束符)
	if (strlen(pFileName) < 25
		|| !(strPrefix = strstr(pFileName, CYC_FILE_PREFIX))		
		|| !(strSuffix = strstr(pFileName, FILE_SUFFIX))
		|| strPrefix != pFileName
		|| strSuffix-strPrefix < 21) {
		return AK_FALSE;
	}

	return AK_TRUE;
}

int RecFile_rename(T_pSTR fileName, T_pSTR newName)
{
	//char newName[1024];
	//memset(newName, 0x00, 1024);
	//strncpy(newName, fileName, strlen(fileName)-strlen(FILE_NAME_DIF));
	T_CHR dstName[64];
	memset(dstName, 0, 64);
	// Get Path
	memcpy(dstName, fileName, strrchr(fileName, DIR_SEPARATOR)-fileName+1);
	// Full Name
	strcat(dstName, newName);
	rename(fileName, dstName);
	return 0;
}


// End of File
