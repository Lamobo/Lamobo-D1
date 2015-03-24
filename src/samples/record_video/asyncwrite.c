#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdarg.h>
#include <time.h>
#include <assert.h>

#include <errno.h>

#include <akuio.h>


#include "asyncwrite.h"
#include "CycBuffer.h"
#include "Thread.h"
#include "log.h"

typedef struct AkAsyncFileWriter_handle_st
{
	CCycBuffer	*	mCycBuffer;

	T_BOOL			mbIsJustFd;
	T_S32			mfd;
	T_U32 			mWriteBlockSize;
	T_U32 			mWriteCacheSize;
	T_BOOL 			mRequestExit;
	T_U32			mWriteBytes;
		
	nthread_t		mTID;
	Condition		mCondition;
} AkAsyncFileWriter_handle;

static T_pVOID thread_begin( T_pVOID user );

static T_VOID Run();

static T_VOID flush( T_pVOID pthis );

static T_U32 getTotalWriteBytes( T_pVOID pthis );

/**
*@brief the thread call back fuction
*/
static T_pVOID thread_begin( T_pVOID user )
{
	AkAsyncFileWriter * self = (AkAsyncFileWriter*)(user);
	if ( self == NULL ) {
		loge( "thread_begin have a invail parm!" );
		return NULL;
	}

	Run( self );
	return NULL;
}

/**
*@brief the constructor of AkAsyncFileWriter
*/
DEFINE_CONSTRUCTOR_BEGIN( AkAsyncFileWriter )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = NULL;

	handle = (AkAsyncFileWriter_handle *)malloc( sizeof( AkAsyncFileWriter_handle ) );
	if ( NULL == handle ) {
		loge( "AkV4l2Camera constructor error! out of memory!\n" );
		return NULL;
	}

	this->handle = (T_pVOID)handle;
	
	handle->mCycBuffer = NULL;
	handle->mfd = -1;
	handle->mWriteBlockSize = 0;
	handle->mWriteCacheSize = 0;
	handle->mRequestExit = AK_TRUE;
	handle->mWriteBytes = 0;
	handle->mbIsJustFd = AK_FALSE;

	handle->mTID = thread_zeroid();
	Condition_Initialize( &handle->mCondition );

	return this;
}
DEFINE_CONSTRUCTOR_END


/**
*@brief the destructor of AkAsyncFileWriter
*/
DEFINE_DESTRUCTOR_BEGIN( AkAsyncFileWriter )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;
	
	if ( handle->mCycBuffer != NULL )
		DEL_SIMULATE_CLASS( CCycBuffer, handle->mCycBuffer );
	
	Condition_Destroy( &(handle->mCondition) );

	free( handle );
}
DEFINE_DESTRUCTOR_END


/**
* @brief   create the file writer
* @author hankejia
* @date 2012-07-05
* @param[in] pthis   			the pointer point to the AkAsyncFileWriter.
* @param[in] fd   	  			the file is descriptor
* @param[in] IsJustFd   	  	AK_TRUE do the async job, AK_FALSE just do the sync job.
* @param[in] writeCacheSize   	cache is size
* @param[in] writeBlockSize		each write to the file is size of data.
* @return T_S32
* @@retval	-1 for error,0 for success
*/
static T_S32 createFileWriter( T_pVOID pthis, T_S32 fd, T_BOOL IsJustFd,
								T_U32 writeCacheSize, T_U32 writeBlockSize )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;
	assert( fd > 0 );

	handle->mWriteBlockSize = writeBlockSize;
	handle->mWriteCacheSize = writeCacheSize;
	handle->mfd = fd;

	if ( IsJustFd ) {
		handle->mbIsJustFd = AK_TRUE;
		return 0;
	}
	
	if ( handle->mWriteBlockSize > handle->mWriteCacheSize ) {
		loge( "AkAsyncFileWriter::invalid parameter!\n" );
		handle->mCycBuffer = NULL;
		return -1;
	}else {
		handle->mCycBuffer = NEW_SIMULATE_CLASS( CCycBuffer );
		if ( NULL == handle->mCycBuffer ) {
			loge( "AkAsyncFileWriter::out of memory!\n" );
			return -1;
		}
		else {
			if ( handle->mCycBuffer->CreateCycBufferEx( handle->mCycBuffer, handle->mWriteCacheSize ) < 0 ) {
				loge( "AkAsyncFileWriter::can't create CycBuffer!\n" );
				DEL_SIMULATE_CLASS( CCycBuffer, handle->mCycBuffer );
				handle->mCycBuffer = NULL;
			}
		}
	}

	return 0;
}


/**
* @brief   reset the file fd
* @author hankejia
* @date 2012-07-05
* @param[in] pthis   			the pointer point to the AkAsyncFileWriter.
* @param[in] fd   	  			the file is descriptor
* @return T_S32
* @@retval	-1 for error,0 for success
*/
static T_S32 ResetFd( T_pVOID pthis, T_S32 fd )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;
	assert( fd > 0 );

	handle->mfd = fd;
	
	if ( handle->mbIsJustFd ) {
		return 0;
	}

	if ( NULL == handle->mCycBuffer ) {
		loge( "ResetFd::the cyc buffer is NULL!\n" );
		return -1;
	}
	
	handle->mCycBuffer->Clean( handle->mCycBuffer );
	handle->mWriteBytes = 0;
	return 0;
}


/**
* @brief   get the file fd
* @author hankejia
* @date 2012-07-05
* @param[in] pthis   			the pointer point to the AkAsyncFileWriter.
* @return T_S32
* @@retval 	the file is fd
*/
static T_S32 getFd( T_pVOID pthis )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;

	return handle->mfd;
}


/**
* @brief   the async write module just do the sync job?
* @author hankejia
* @date 2012-07-05
* @param[in] pthis   			the pointer point to the AkAsyncFileWriter.
* @return T_BOOL
* @@retval  return if do the async job return AK_TRUE, do the sync job return false
*/
static T_BOOL IsJustFd( T_pVOID pthis )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;

	return handle->mbIsJustFd;
}

/*
 * @brief		write buf to file writer
 * @author hankejia
 * @date 2012-07-05
 * @param[in] pthis 	the pointer point to the AkAsyncFileWriter.
 * @param[in] buf		the data will writed to the file
 * @param[in] size	 	data is size in bytes
 * @return	T_S32
 * @retval	<=0 for error,otherwise the length of write
 */
static T_S32 AsyncWrite( T_pVOID pthis, T_pVOID buf, T_S32 size )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;
	T_S32 retVal = 0;

	if ( handle->mbIsJustFd ) {
		logi( "current no offer Async write function!\n" );
		return 0;
	}
	
	//LOGV("write hfile 0x%lx,buf %p,size 0x%lx",hFile,buf,size);
	//incase out of memory 
 	if( handle->mCycBuffer == NULL ){
		loge( "out of memory in %s %s", __FILE__, __LINE__ );
		return -1;
	}

	retVal = handle->mCycBuffer->Push( handle->mCycBuffer, (T_CHR *)buf, size );
	if ( retVal < 0 ) {
		loge( "cycbuffer don't have enough space to push one frame!\n" );
		return -1;
	}
	
	Condition_Lock( handle->mCondition );
	handle->mWriteBytes += retVal;
	Condition_Unlock( handle->mCondition );
	return retVal;
}


/*
 * @brief		read data from file writer
 * @author hankejia
 * @date 2012-07-05
 * @param[in] pthis 	the pointer point to the AkAsyncFileWriter.
 * @param[in] buf		the data buff will save the data readed from file
 * @param[in] size	 	buf(read) is size
 * @return	T_S32
 * @retval	<=0 for error,otherwise the length of read
 */
static T_S32 AsyncRead( T_pVOID pthis, T_pVOID buf, T_S32 size )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;
	
	return read( handle->mfd, buf, size );
}

/*
 * @brief		seek the write postion according to offset & whence
 * @author hankejia
 * @date 2012-07-05
 * @param[in] pthis 	the pointer point to the AkAsyncFileWriter.
 * @param[in] offset	the offset from whence
 * @param[in] whence	whence
 					SEEK_SET
						The offset is set to offset bytes.
					SEEK_CUR
						The offset is set to its current location plus offset bytes.
 					SEEK_END
						The offset is set to the size of the file plus offset bytes.
 * @return	T_S32
 * @retval	<=0 for error,otherwise for succeed
 */
static T_U32 AsyncSeek( T_pVOID pthis, T_U32 offset, T_S32 whence )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;

	if ( handle->mbIsJustFd ) {
		logi( "current no offer Async seek function!\n" );
		return 0;
	}
	
	flush( this );
	//do actual seek job
	T_U32 ret = (T_U32)lseek64( handle->mfd, offset, whence );
	Condition_Lock( handle->mCondition );
	handle->mWriteBytes = ret;
	Condition_Unlock( handle->mCondition );
	
	return ret;
}

/*
 * @brief		check wether the file in file writer exsists
 * @param[in] pthis 	the pointer point to the AkAsyncFileWriter.
 * @return	T_S32
 * @retval	1 for exsists, 0 for not.
 */
static T_S32 fileExist( T_pVOID pthis )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;
	
	if( lseek64( handle->mfd, 0, SEEK_CUR ) < 0 ){
		loge( "file des %d not exsisted, in %s %s", handle->mfd, __FILE__, __LINE__);
		return 0;
	}

	return 1;
}

/*
 * @brief		return how many bytes the file writer has written, including cache.
 * @param[in] pthis 	the pointer point to the AkAsyncFileWriter.
 * @return	T_S32
 * @retval	<=0 for error,otherwise the length that has written.
 */
static T_U32 tell( T_pVOID pthis )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;

	if ( handle->mbIsJustFd ) {
		logi( "current no offer Async tell function!\n" );
		return 0;
	}
	
	//incase out of memory 
 	if( handle->mCycBuffer == NULL ){
		loge( "out of memory in %s %s", __FILE__, __LINE__ );
		return -1;
	}

//	LOGV("tell returns %ld",retVal);
	return getTotalWriteBytes( pthis );
}

/*
 * @brief		flush all buffer to file
 * @param[in] pthis 	the pointer point to the AkAsyncFileWriter.
 * @return	T_VOID
 * @retval	NONE
 */
static T_VOID flush( T_pVOID pthis )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;
	T_S32 ret = 0;

	if ( handle->mbIsJustFd ) {
		logi( "current no offer flush function!\n" );
		return;
	}
	
	LOGV( "flush buffer to file %d", handle->mfd );
	//incase out of memory 
 	if( handle->mCycBuffer == NULL ){
		loge( "out of memory in %s %s", __FILE__, __LINE__ );
		return;
	}

	ret = handle->mCycBuffer->flush( handle->mCycBuffer, handle->mfd );
	
	if ( ret < 0 )
		loge( "AkAsyncFileWriter::flush error!\n" );
}


/*
 * @brief		start write thread
 * @param[in] pthis 	the pointer point to the AkAsyncFileWriter.
 * @return	T_S32
 * @retval	0 for success, other for error 
 */
static T_S32 start( T_pVOID pthis )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;

	if ( handle->mbIsJustFd ) {
		logi( "no need to start!\n" );
		return 0;
	}
	
	//already start
	Condition_Lock( handle->mCondition );
	if ( !(handle->mRequestExit) ) {
		Condition_Unlock( handle->mCondition );
		return 0;
	}
	
	handle->mRequestExit = AK_FALSE;
	handle->mWriteBytes = 0;

	Condition_Unlock( handle->mCondition );

	//if mCycBuffer is NULL, try create it again
	if ( NULL == handle->mCycBuffer ) {
		handle->mCycBuffer = NEW_SIMULATE_CLASS( CCycBuffer );
		if ( NULL == handle->mCycBuffer ) {
			loge( "can't start the async file write, out of memory!\n" );
			return -1;
		}

		if ( handle->mCycBuffer->CreateCycBufferEx( handle->mCycBuffer, handle->mWriteCacheSize ) < 0 ) {
			loge( "can't start the async file write, create cycbuffer error!\n" );
			return -1;
		}
	}

	if ( pthread_create( &(handle->mTID), NULL, thread_begin, this ) != 0 ) {
		loge( "unable to create a thread for async write file!\n" );
		return -1;
	}

	return 0;
}

/*
 * @brief		stop write thread
 * @param[in] pthis  the pointer point to the AkAsyncFileWriter.
 * @return	int
 * @retval	0 for success, other for error 
 */
static T_S32 stop( T_pVOID pthis )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;

	if ( handle->mbIsJustFd ) {
		logi( "no need to stop!\n" );
		return 0;
	}
	
	if ( NULL == handle->mCycBuffer ) {
		return -1;
	}

	Condition_Lock( handle->mCondition );
	//already stop
	if ( handle->mRequestExit ){
		Condition_Unlock( handle->mCondition );
		return 0;
	}

	//if the Cyc buffer is empty, maybe our write thread was block in the WriteToFs.
	//so we mush call ForceQuit to make sure that situation wasn't happen before.
	if ( handle->mCycBuffer->IsEmpty( handle->mCycBuffer ) ) {
		handle->mCycBuffer->ForceQuit( handle->mCycBuffer );
	}
	
	handle->mRequestExit = AK_TRUE;
	Condition_Unlock( handle->mCondition );

	pthread_join( handle->mTID, NULL );
	logi( "Async file write thread is end!\n" );
	
	//flush the cycbuffer is data to file
	handle->mCycBuffer->ResumeForceQuitState( handle->mCycBuffer );
	flush( this );
	
	return 0;
}

/*
 * @brief		get total written bytes of file writer
 * @param[in] pthis  the pointer point to the AkAsyncFileWriter.
 * @return	T_U32
 * @retval	the length of bytes written
 */
static T_U32 getTotalWriteBytes( T_pVOID pthis )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;
	T_U32 ret = 0;

	if ( handle->mbIsJustFd ) {
		logi( "the AkAsyncFileWriter current no offer the async process!\n" );
		return 0;
	}
	
	Condition_Lock( handle->mCondition );
	ret = handle->mWriteBytes;
	Condition_Unlock( handle->mCondition );
	
	return ret;
}


/*
 * @brief		the thread run
 * @param[in] pthis  the pointer point to the AkAsyncFileWriter.
 * @return	NONE
 */
static T_VOID Run( T_pVOID pthis )
{
	AkAsyncFileWriter *this = (AkAsyncFileWriter *)pthis;
	AkAsyncFileWriter_handle * handle = (AkAsyncFileWriter_handle *)this->handle;
	T_S32 ret = 0;
		
	if ( NULL == handle->mCycBuffer ) {
		loge( "AkAsyncFileWriter have a process error! can't run the async write thread!\n" );
		return;
	}

	while( 1 ) {
		//request exit
		Condition_Lock( handle->mCondition );
		if ( handle->mRequestExit ) {
			Condition_Unlock( handle->mCondition );
			break;
		}
		Condition_Unlock( handle->mCondition );

		ret = handle->mCycBuffer->WriteToFs( handle->mCycBuffer, handle->mfd, handle->mWriteBlockSize );
		if ( ret < 0 ) {
			LOGV( "can't write data to file!\n" );
			continue;
		}
	}
}

#define LOG_BUF_SIZE		1024


/*
 * @brief		construct a file writer to do file writing 
 * @param	fd [in], IsJustFd[in], writebufsize[in], writeblocksize[in]
 * @return	AkAsyncFileWriter
 * @retval	NULL for error, otherwise a handle of AkAsyncFileWriter
 */
AkAsyncFileWriter* ak_rec_cb_load( int fd, T_BOOL IsJustFd, int writebufsize, int writeblocksize )
{
	T_S32 ret = 0;
	
	//construct a file write then  start it
	AkAsyncFileWriter* asyncwriter = NEW_SIMULATE_CLASS( AkAsyncFileWriter );
	if ( asyncwriter == NULL ){
		loge("unable to create aysnc file writer");
		return NULL;
	}

	ret = asyncwriter->createFileWriter( asyncwriter, fd, IsJustFd, writebufsize, writeblocksize );
	if ( ret < 0 ) {
		loge("unable to create aysnc file writer");
		return NULL;
	}

	if ( IsJustFd ) {
		return asyncwriter;
	}
	
	if ( asyncwriter->start( asyncwriter ) != 0 ) {
		loge("unable to start async file writer");
		DEL_SIMULATE_CLASS( AkAsyncFileWriter, asyncwriter );
		asyncwriter = NULL;
		return NULL;
	}

	return asyncwriter;
}

/*
 * @brief		release resource allocate by the file writer
 * @param	AkAsyncFileWriter[in]
 * @return	void
 * @retval	NONE
 */
void ak_rec_cb_unload( AkAsyncFileWriter* writer )
{
	//invlidate parameters
	if ( writer == NULL ){
		logi("ak record callback lib already unloaded");
		return;
	}

	if ( !(writer->IsJustFd( writer )) ) {
		writer->stop( writer );
	}

	//release here
	DEL_SIMULATE_CLASS( AkAsyncFileWriter, writer );
}

/*
 * @brief		used for media lib to print 
 * @param	format [in] ,... [in]
 * @return	T_VOID
 * @retval	NONE
 */
T_VOID ak_rec_cb_printf( T_pCSTR format, ... )
{
	//REC_TAG, used to identify print informations of media lib
#if 1
	va_list ap;
	char buf[LOG_BUF_SIZE];    

	va_start( ap, format );
	vsnprintf( buf, LOG_BUF_SIZE, format, ap );
	va_end( ap );

	fprintf( stderr, LOG_TAG );
	fprintf( stderr, "::" );
	fprintf( stderr, buf );
	fprintf( stderr, "\n" );
#endif
}

/*
 * @brief		used to alloc memory
 * @param	size [in] 
 * @return	T_pVOID
 * @retval	NULL for error,otherwise the handle of memory allocated.
 */
T_pVOID ak_rec_cb_malloc( T_U32 size )
{
	LOGV("malloc size 0x%lx",size);

	return malloc( size );
}

/*
 * @brief		free memory
 * @param	mem [in] 
 * @return	T_VOID
 * @retval	NONE
 */
T_VOID ak_rec_cb_free( T_pVOID mem )
{
	LOGV("free buffer %p",mem);
	return free( mem );
}

/*
 * @brief		used for memory copy
 * @param	dest [in, out],  src[in], size[in]
 * @return	T_pVOID
 * @retval	NULL for error,otherwise returns dest.
 */
T_pVOID ak_rec_cb_memcpy( T_pVOID dest, T_pCVOID src, T_U32 size )
{
	LOGV("memcpy dest %p,src %p,size 0x%lx",dest,src,size);
	
	return memcpy( dest, src, size );
}

/*
 * @brief		read data from file writer
 * @param	hFileWriter [in],  buf[in,out], size[in]
 * @return	T_S32
 * @retval	<=0 for error,otherwise the length of read
 */
T_S32 ak_rec_cb_fread( T_S32 hFileWriter, T_pVOID buf, T_S32 size )
{
	LOGV("fread file writer 0x%lx  buf %p, size 0x%lx, \n",hFileWriter, buf, size);

	AkAsyncFileWriter* writer = (AkAsyncFileWriter*)hFileWriter;
	if ( writer == NULL ) {
		LOGE("invalid parameter");
		return -1;
	}
	
	if ( writer->IsJustFd( writer ) ) {
		return read( writer->getFd( writer ), buf, size );
	}
	
	return writer->read( writer, buf, size );
}

/*
 * @brief		write data to file writer
 * @param	hFileWriter [in],  buf[in], size[in]
 * @return	T_S32
 * @retval	<=0 for error,otherwise the length of write
 */
T_S32 ak_rec_cb_fwrite( T_S32 hFileWriter, T_pVOID buf, T_S32 size )
{
	LOGV("fwrite file writer 0x%lx  buf %p, size 0x%lx, \n",hFileWriter, buf, size);
	
	AkAsyncFileWriter* writer = (AkAsyncFileWriter*)hFileWriter;
	if ( writer == NULL ) {
		LOGE("invalid parameter");
		return -1;
	}
	
	if ( writer->IsJustFd( writer ) ) {
		T_S32 ret = write( writer->getFd( writer ), buf, size );
		if ( ret < 0 ) {
			loge( "ak_rec_cb_fwrite error = %s\n", strerror(errno) );
		}
		return ret;
	}

	return writer->write( writer, buf, size );
}

/*
 * @brief		seek the write postion according to offset & whence
 * @param	hFileWriter [in],  offset[in], whence[in]
 * @return	T_S32
 * @retval	<=0 for error,otherwise for succeed
 */
T_U32 ak_rec_cb_fseek( T_S32 hFileWriter, T_U32 offset, T_U32 whence )
{
	AkAsyncFileWriter* writer = (AkAsyncFileWriter*)hFileWriter;
	if ( writer == NULL ) {
		LOGE("invalid parameter");
		return -1;
	}
	
	if ( writer->IsJustFd( writer ) ) {
		off_t ret = lseek64( writer->getFd( writer ), offset, whence );
		if ( ret < 0 ) {
			loge( "ak_rec_cb_fseek error = %s\n", strerror(errno) );
		}
		
		return (T_U32)ret;
	}

	return (T_U32)writer->seek( writer, offset, whence );
}

/*
 * @brief		return how many bytes the file writer has written, including cache.
 * @param	hFileWriter [in],
 * @return	T_S32
 * @retval	<=0 for error,otherwise the length that has written.
 */
T_U32 ak_rec_cb_ftell( T_S32 hFileWriter )
{
	LOGV("ftell file writer 0x%lx",hFileWriter);
	
	AkAsyncFileWriter* writer = (AkAsyncFileWriter*)hFileWriter;
	if ( writer == NULL ) {
		LOGE("invalid parameter");
		return -1;
	}

	if ( writer->IsJustFd( writer ) ) {
		off_t ret = lseek64( writer->getFd( writer ), 0, SEEK_CUR );
		if ( ret < 0 ) {
			loge( "ak_rec_cb_ftell error = %s\n", strerror(errno) );
		}
		return (T_U32)ret;
	}
	
	return writer->tell( writer );
}

/*
 * @brief		check wether file system is busy.
 * @param	void
 * @return	T_BOOL
 * @retval	AK_TRUE for busy,AK_FALSE for idle.
 */
T_BOOL ak_rec_cb_lnx_filesys_isbusy()
{
	LOGV("%s",__FUNCTION__);
  	return AK_FALSE;
}

/*
 * @brief		check wether the file in file writer exsists
 * @param	hFileWriter[in]
 * @return	T_S32
 * @retval	1 for exsists, 0 for not.
 */
T_S32 ak_rec_cb_lnx_fhandle_exist( T_S32 hFileWriter )
{
	LOGV("fhandle exist file writer %p",hFileWriter);
	
	AkAsyncFileWriter* writer = (AkAsyncFileWriter*)hFileWriter;
	if ( writer == NULL ) {
		LOGE("invalid parameter");
		return -1;
	}

	return writer->fileExist( writer );
}

/*
 * @brief		media lib will call this while idle.
 * @param	ticks[in]
 * @return	T_BOOL
 * @retval	AK_TRUE for success,other for error.
 */
T_BOOL ak_rec_cb_lnx_delay(T_U32 ticks)
{
	logi("delay 0x%lx ticks",ticks);
	
#ifdef ANDROID
	//usleep (ticks*1000);
	akuio_wait_irq();
	return true;
#else
	return (usleep(ticks*1000) == 0);
#endif
}


//--------------------------simulate class register------------------------------

REGISTER_FUN_BEGIN( AkAsyncFileWriter, CONSTRUCTOR_FUN( AkAsyncFileWriter ), DESTRUCTOR_FUN( AkAsyncFileWriter ) )
REGISTER_FUN( createFileWriter, createFileWriter )
REGISTER_FUN( getFd, getFd )
REGISTER_FUN( IsJustFd, IsJustFd )
REGISTER_FUN( ResetFd, ResetFd )
REGISTER_FUN( write, AsyncWrite )
REGISTER_FUN( read, AsyncRead )
REGISTER_FUN( seek, AsyncSeek )
REGISTER_FUN( fileExist, fileExist )
REGISTER_FUN( tell, tell )
REGISTER_FUN( flush, flush )
REGISTER_FUN( start, start )
REGISTER_FUN( stop, stop )
REGISTER_FUN( getTotalWriteBytes, getTotalWriteBytes )
REGISTER_FUN_END( AkAsyncFileWriter )

REGISTER_SIMULATE_CLASS_C( AkAsyncFileWriter )


