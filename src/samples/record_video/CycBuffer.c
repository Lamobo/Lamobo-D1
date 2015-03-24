
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "CycBuffer.h"
#include "log.h"
#include "Tool.h"

typedef struct CycBuffer_handle_st {
	T_S32		mBufferSize;
	T_S32		mUseSize;

	T_CHR *		mRead;
	T_CHR *		mWrite;
	T_CHR *  	mCycBuffer;

	T_BOOL		mPushComplete;
	T_BOOL		mPopComplete;
	T_BOOL		mForceQuit;
	
	Condition 	mDataCon;
	Condition 	mWriteDataCon;
	//Condition	mWaitForPopCon;
	//Condition	mWaitForPushCon;
}CycBuffer_handle;

static T_S32 Clean( T_pVOID pthis );

static T_S32 FakePushFull( T_pVOID pthis );

static T_S32 DestroyCycBuffer( T_pVOID pthis );

static T_S32 ResumeForceQuitState( T_pVOID pthis );

static T_S32 ForceQuit( T_pVOID pthis );

static T_S32 ResumeForceQuitState( T_pVOID pthis );

static T_CHR * PopSingle( T_pVOID pthis, T_S32 iSize );

/**
* @brief   simulate class CCycbuffer constructor
* 
* initialize all the simulate class member variable
* @author hankejia
* @date 2012-07-05
* @param[in] pthis   the pointer point to the CCycBuffer.
* @return NONE
*/
DEFINE_CONSTRUCTOR_BEGIN( CCycBuffer )
{	
	CCycBuffer *this = (CCycBuffer *)pthis;
	CycBuffer_handle * handle = NULL;

	handle = (CycBuffer_handle *)malloc( sizeof( CycBuffer_handle ) );
	if ( NULL == handle ) {
		loge( "CycBuffer constructor error! out of memory!\n" );
		return NULL;
	}

	this->handle = (T_pVOID)handle;
	
	handle->mBufferSize = 0;
	handle->mUseSize = 0;
	handle->mRead = NULL;
	handle->mWrite = NULL;
	handle->mCycBuffer = NULL;
	handle->mPushComplete = AK_TRUE;
	handle->mPopComplete = AK_TRUE;
	handle->mForceQuit = AK_FALSE;
	Condition_Initialize( &handle->mDataCon );
	Condition_Initialize( &handle->mWriteDataCon );
	//Condition_Initialize( &handle->mWaitForPopCon );
	//Condition_Initialize( &handle->mWaitForPushCon );

	return this;
}
DEFINE_CONSTRUCTOR_END
	
/**
* @brief   simulate class CCycbuffer destructor
* 
* free all the simulate class member variable,
* @author hankejia
* @date 2012-07-05
* @param[in] pthis   the pointer point to the CCycBuffer.
* @return NONE
*/
DEFINE_DESTRUCTOR_BEGIN( CCycBuffer )
{	
	CCycBuffer *this = (CCycBuffer *)pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	
	//虚构时，如果当前还有线程在Push流程中等待
	//那么首先Pop出Buffer中的所有数据，唤醒在Push
	//中等待的线程，等待其完成push操作。
	Condition_Lock( handle->mDataCon );
	if ( !( handle->mPushComplete ) ) {
		Condition_Unlock( handle->mDataCon );
		Clean( this );

		//唤醒在Push中等待的线程
		Condition_Signal( &(handle->mDataCon) );

		Condition_Wait( &(handle->mDataCon) );
		Condition_Unlock( handle->mDataCon );
	} else
		Condition_Unlock( handle->mDataCon );

	//如果当前还有线程在pop流程中等待，那么首先
	//向Buffer中Push数据，唤醒在Pop中等待的线程，等
	//待其完成pop操作。
	Condition_Lock( handle->mDataCon );
	if ( !( handle->mPopComplete ) ) {
		Condition_Unlock( handle->mDataCon );
		FakePushFull( this );

		//唤醒在Pop中等待的线程
		Condition_Signal( &(handle->mDataCon) );
		
		Condition_Wait( &handle->mDataCon );
		Condition_Unlock( handle->mDataCon );
	} else
		Condition_Unlock( handle->mDataCon );
	
	DestroyCycBuffer( this );
	Condition_Destroy( &( handle->mDataCon ) );
	Condition_Destroy( &( handle->mWriteDataCon ) );
	//Condition_Destroy( &( handle->mWaitForPopCon ) );
	//Condition_Destroy( &( handle->mWaitForPushCon ) );

	free( handle );
}
DEFINE_DESTRUCTOR_END

/**
* @brief   set the Cyc Buffer is size in bytes
* @author hankejia
* @date 2012-07-05
* @param[in] pthis   the pointer point to the CCycBuffer.
* @param[in] iSize   the cyc buffer size.
* @return T_S32
* @retval if return 0, set buffer size success, otherwise failed 
*/
static T_S32 SetBufferSize( T_pVOID pthis, T_S32 iSize )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	
	assert( iSize > 0 );
	
	Condition_Lock( handle->mDataCon );
	handle->mBufferSize = iSize;
	Condition_Unlock( handle->mDataCon );
	
	return 0;	
}

/**
* @brief   create the Cyc Buffer, malloc memory
*
* call SetBufferSize fuction to set cyc buffer size before call this function 
* @author hankejia
* @date 2012-07-05
* @param[in] pthis   the pointer point to the CCycBuffer.
* @return T_S32
* @retval if return 0, create cyc buffer success, otherwise failed 
*/
static T_S32 CreateCycBuffer( T_pVOID pthis )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	T_S32 ret = 0;

	Condition_Lock( handle->mDataCon );
	
	if ( handle->mCycBuffer != NULL )
		DestroyCycBuffer( this );
	
	handle->mCycBuffer = (T_CHR *)malloc( handle->mBufferSize );
	if ( handle->mCycBuffer == NULL )
	{
		loge( "memory alloc error![%s::%s]\n", __FILE__, __LINE__ );
		ret = -1;
		goto End;
	}
	
	memset( handle->mCycBuffer, 0, handle->mBufferSize );

	handle->mRead = handle->mWrite = handle->mCycBuffer;

End:
	Condition_Unlock( handle->mDataCon );
	return ret;
}

/**
* @brief   create the Cyc Buffer, malloc memory
*
* not need to call setBufferSize function 
* @author hankejia
* @date 2012-07-05
* @param[in] pthis   the pointer point to the CCycBuffer.
* @param[in] iSize   the cyc buffer size.
* @return T_S32
* @retval if return 0, create cyc buffer success, otherwise failed 
*/
static T_S32 CreateCycBufferEx( T_pVOID pthis, T_S32 iSize )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	T_S32 ret = 0;

	ret = SetBufferSize( this, iSize );
	if ( ret < 0 )
		return ret;
	
	ret = CreateCycBuffer( this );
	return ret;
}


/**
* @brief   destory the Cyc Buffer, free memory
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis   the pointer point to the CCycBuffer.
* @param[in] iSize   the cyc buffer size.
* @return T_S32
* @retval if return 0 destroy cyc buffer success, otherwise failed 
*/
static T_S32 DestroyCycBuffer( T_pVOID pthis )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	
	Condition_Lock( handle->mDataCon );

	if ( handle->mCycBuffer != NULL )
		free( handle->mCycBuffer );

	handle->mCycBuffer = NULL;

	handle->mRead = NULL;
	handle->mWrite = NULL;
	
	handle->mBufferSize = 0;
	handle->mUseSize = 0;

	Condition_Unlock( handle->mDataCon );
	return 0;
}


/**
* @brief   pop data from cyc buffer
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis		the pointer point to the CCycBuffer.
* @param[out] pBuffer	the memory to saved the data
* @param[in] iSize		how many bytes you want to pop from cyc buffer
* @return T_S32
* @retval return  >=0 the size of bytes pop from cyc buffer, < 0  failed
*/
static T_S32 Pop( T_pVOID pthis, char * pBuffer, T_S32 iSize )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	T_S32 iGetSize= 0;
	T_S32 iLeavings = 0;
	T_S32 iReadSize = 0;
	T_S32 iLineSize = 0;
	
	if ( iSize > handle->mBufferSize ) {
		loge( "CCycBuffer::Pop() can't not Pop %d data from buffer, %d > %d(buffer size)", iSize, iSize, handle->mBufferSize );
		return -1;
	}

	//the cyc buffer is empty, wait for push
	Condition_Lock( handle->mDataCon );
	handle->mPopComplete = AK_FALSE;
	while ( ( handle->mUseSize < iSize ) && ( !(handle->mForceQuit) ) ) {
		//Condition_Unlock( handle->mDataCon );
		//logi( "buffer empty!\n" );
		Condition_Wait( &(handle->mDataCon) );
		//logi( "wake up from push signed!\n" );
		//Condition_Lock( handle->mDataCon );
	}

	if ( handle->mForceQuit ) {
		handle->mPopComplete = AK_TRUE;
		Condition_Unlock( handle->mDataCon );
		return 0;
	}
	
	assert( ( pBuffer != NULL ) && ( handle->mCycBuffer != NULL ) && ( iSize > 0 ) );
	assert( ( ( handle->mRead >= handle->mCycBuffer ) && ( handle->mRead < (handle->mCycBuffer + handle->mBufferSize) ) ) );
	assert( ( ( handle->mWrite >= handle->mCycBuffer ) && ( handle->mWrite < (handle->mCycBuffer + handle->mBufferSize) ) ) );

	iGetSize = ( (handle->mUseSize) > iSize )?iSize:(handle->mUseSize);
	iLeavings = iGetSize;

	while ( iLeavings > 0 )
	{
		iLineSize = ( handle->mCycBuffer + handle->mBufferSize ) - handle->mRead;
		iReadSize = ( iLineSize > iLeavings )?iLeavings:iLineSize;
		memcpy( pBuffer, handle->mRead, iReadSize );

		if ( iReadSize == iLineSize )
			handle->mRead = handle->mCycBuffer;
		else
			handle->mRead += iReadSize;

		pBuffer += iReadSize;
		iLeavings -= iReadSize;
	}
	
	handle->mUseSize -= iGetSize;

	handle->mPopComplete = AK_TRUE;
	Condition_Unlock( handle->mDataCon );

	//logi( "pop signal send!\n" );
	Condition_Signal( &(handle->mDataCon) );
	
	return iGetSize;
}


/**
* @brief   Push data to cyc buffer
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis		the pointer point to the CCycBuffer.
* @param[in] pBuffer	the data which you want to push to cyc buffer
* @param[in] iSize		how many bytes you want to push to cyc buffer
* @return T_S32
* @retval return  >=0 the size of bytes push to cyc buffer, < 0  failed
*/
static T_S32 Push( T_pVOID pthis, T_CHR * pBuffer, T_S32 iSize )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	T_S32 iLeavings = 0;
	T_S32 iWriteSize = 0;
	T_S32 iLineSize = 0;
	
	if ( iSize > handle->mBufferSize ) {
		loge( "CCycBuffer::Push() can't not push %d data to buffer, %d > %d(buffer size)", iSize, iSize, handle->mBufferSize );
		return -1;
	}
	
	//the cyc buffer is full. wait for pop
	Condition_Lock( handle->mDataCon );
	handle->mPushComplete = AK_FALSE;
	while ( ( ( handle->mBufferSize - handle->mUseSize ) < iSize ) && 
			( !(handle->mForceQuit) ) ) {
		//Condition_Unlock( handle->mDataCon );
		logi( "Push buffer full! %d\n", handle->mBufferSize );
		Condition_Wait( &(handle->mDataCon) );
		//logi( "wake up from pop signed!\n" );
		//Condition_Lock( handle->mDataCon );
	}

	if ( handle->mForceQuit ) {
		handle->mPushComplete = AK_TRUE;
		Condition_Unlock( handle->mDataCon );
		return 0;
	}
	
	assert( ( pBuffer != NULL ) && ( handle->mCycBuffer != NULL ) && ( iSize > 0 ) );
	assert( ( ( handle->mRead >= handle->mCycBuffer ) && ( handle->mRead < (handle->mCycBuffer + handle->mBufferSize) ) ) );
	assert( ( ( handle->mWrite >= handle->mCycBuffer ) && ( handle->mWrite < (handle->mCycBuffer + handle->mBufferSize) ) ) );
	
	iLeavings = iSize;

	while ( iLeavings > 0 )
	{
		iLineSize = ( handle->mCycBuffer + handle->mBufferSize ) - handle->mWrite;
		iWriteSize = ( iLineSize > iLeavings )?iLeavings:iLineSize;
		memcpy( handle->mWrite, pBuffer, iWriteSize );

		if ( iWriteSize == iLineSize )
			handle->mWrite = handle->mCycBuffer;
		else
			handle->mWrite += iWriteSize;

		pBuffer += iWriteSize;
		iLeavings -= iWriteSize;
	}
	
	handle->mUseSize = ( ( handle->mUseSize + iSize ) > handle->mBufferSize )?handle->mBufferSize:( handle->mUseSize + iSize );
	if ( handle->mUseSize == handle->mBufferSize )
		handle->mRead = handle->mWrite;

	handle->mPushComplete = AK_TRUE;
	Condition_Unlock( handle->mDataCon );

	//logi( "push signed send!\n" );
	Condition_Signal( &(handle->mDataCon) );
	return iSize;
}

static T_S32 PushSingle( T_pVOID pthis, T_CHR * pBuffer, T_S32 iSize )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	T_S32 iLeavings = 0;
	T_S32 iWriteSize = 0;
	T_S32 iLineSize = 0;
	T_CHR *	mTempWrite = NULL;
	
	if ( iSize > handle->mBufferSize ) {
		loge( "CCycBuffer::Push() can't not push %d data to buffer, %d > %d(buffer size)", iSize, iSize, handle->mBufferSize );
		return -1;
	}
	
	//the cyc buffer is full. wait for pop
	Condition_Lock( handle->mDataCon );
	handle->mPushComplete = AK_FALSE;
	while ( ( ( handle->mBufferSize - handle->mUseSize ) < iSize ) && 
			( !(handle->mForceQuit) ) ) {
		//Condition_Unlock( handle->mDataCon );
		logi( "PushSingle buffer full! %d\n", handle->mBufferSize );
		Condition_Wait( &(handle->mDataCon) );
		//logi( "wake up from pop signed!\n" );
		//Condition_Lock( handle->mDataCon );
	}

	if ( handle->mForceQuit ) {
		handle->mPushComplete = AK_TRUE;
		Condition_Unlock( handle->mDataCon );
		return 0;
	}
	
	assert( ( pBuffer != NULL ) && ( handle->mCycBuffer != NULL ) && ( iSize > 0 ) );
	assert( ( ( handle->mRead >= handle->mCycBuffer ) && ( handle->mRead < (handle->mCycBuffer + handle->mBufferSize) ) ) );
	assert( ( ( handle->mWrite >= handle->mCycBuffer ) && ( handle->mWrite < (handle->mCycBuffer + handle->mBufferSize) ) ) );

	mTempWrite = handle->mWrite;
	Condition_Unlock( handle->mDataCon );
	
	iLeavings = iSize;

	while ( iLeavings > 0 )
	{
		iLineSize = ( handle->mCycBuffer + handle->mBufferSize ) - mTempWrite;
		iWriteSize = ( iLineSize > iLeavings )?iLeavings:iLineSize;
		memcpy( mTempWrite, pBuffer, iWriteSize );

		if ( iWriteSize == iLineSize )
			mTempWrite = handle->mCycBuffer;
		else
			mTempWrite += iWriteSize;

		pBuffer += iWriteSize;
		iLeavings -= iWriteSize;
	}

	Condition_Lock( handle->mDataCon );
	handle->mWrite = mTempWrite;
	handle->mUseSize = ( ( handle->mUseSize + iSize ) > handle->mBufferSize )?handle->mBufferSize:( handle->mUseSize + iSize );
	if ( handle->mUseSize == handle->mBufferSize )
		handle->mRead = handle->mWrite;

	handle->mPushComplete = AK_TRUE;

	Condition_Unlock( handle->mDataCon );

	Condition_Signal( &(handle->mDataCon) );
	return iSize;
}


/**
* @brief   write cyc buffer is data to file
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis			the pointer point to the CCycBuffer.
* @param[in] fd			file is descriptor.the cyc buffer is data will write to this file
* @param[in] iSize			how many data in bytes from cyc buffer you want to write to the file
* @param[in] pWriteBuffer	the space alloc by caller,fuction will use this space to load the data,and write to file system,
*						if this param is NULL, function will alloc space by himself
* @return T_S32
* @retval return  >=0 the size of bytes write to file, < 0  failed
*/
static T_S32 WriteToFs( T_pVOID pthis, T_S32 fd, T_S32 iSize )
{	
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	T_CHR * pBuffer = NULL;
	T_S32 iLineSize = 0, iLeavings = 0;

	Condition_Lock( handle->mWriteDataCon );

	//need to flash all data
	if ( -1 == iSize ) {
		iSize = handle->mUseSize;
	}
	
	pBuffer = PopSingle( pthis, iSize );
	
	if ( pBuffer == NULL ) {
		ResumeForceQuitState( pthis );
		Condition_Unlock( handle->mWriteDataCon );
		delay_loop( 0, 10000 ); // 10ms
		return 0;
	}

	iLeavings = iSize;

	while( iLeavings > 0 ) {
		iLineSize = ( handle->mCycBuffer + handle->mBufferSize ) - pBuffer;
		if ( iLineSize == 0 ) {
			pBuffer = handle->mCycBuffer;
			continue;
		}else if ( iLineSize > iLeavings ) {
			iLineSize = iLeavings;
		}

		if ( WriteComplete( fd, pBuffer, iLineSize ) < 0 ) {
			Condition_Unlock( handle->mWriteDataCon );
			loge( "WriteToFs::WriteComplete error!\n" );
			return -1;
		}
		
		pBuffer += iLineSize;
		iLeavings -= iLineSize;
	}

	Condition_Lock( handle->mDataCon );
	handle->mRead = pBuffer;
	handle->mUseSize -= iSize;
	handle->mPopComplete = AK_TRUE;
	Condition_Unlock( handle->mDataCon );
	Condition_Unlock( handle->mWriteDataCon );

	//logi( "pop signal send!\n" );
	Condition_Signal( &(handle->mDataCon) );
	
	return iSize;
}


/**
* @brief   flush cyc buffer is data to file
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis			the pointer point to the CCycBuffer.
* @param[in] fd			file is descriptor.the cyc buffer is data will write to this file
* @return T_S32
* @retval return  >=0 the size of bytes write to file, < 0  failed
*/
static T_S32 flush( T_pVOID pthis, T_S32 fd )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;

	while ( !(handle->mPopComplete) ) {
		ForceQuit( pthis );
		delay_loop( 0, 1000 ); // 1ms
	}
	
	return WriteToFs( pthis, fd, -1 );
}

/**
* @brief   write cyc buffer is data to file
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis			the pointer point to the CCycBuffer.
* @param[in] fd			file is descriptor.the cyc buffer is data will write to this file
* @param[in] iSize			how many data in bytes from cyc buffer you want to write to the file
* @return T_S32
* @retval return  >=0 the size of bytes write to file, < 0  failed

static T_S32 WriteToFs( T_pVOID pthis, T_S32 fd, T_S32 iSize ) 
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	T_S32 iGetSize = 0, iLeavings = 0, iReadSize = 0, iLineSize = 0, iTempRead = 0, iRet = 0;
	
	if ( iSize > handle->mBufferSize ) {
		loge( "CCycBuffer::WriteToFs() can't not Pop %d data from buffer, %d > %d(buffer size)", iSize, iSize, handle->mBufferSize );
		return -1;
	}

	//the cyc buffer is empty, wait for push
	Condition_Lock( handle->mDataCon );
	handle->mPopComplete = AK_FALSE;
	while ( ( handle->mUseSize == 0 ) && ( !(handle->mForceQuit) ) ) {
		Condition_Unlock( handle->mDataCon );
		//logi( "buffer empty!\n" );
		Condition_Wait( &(handle->mWaitForPushCon) );
		//logi( "wake up from push signed!\n" );
		Condition_Lock( handle->mDataCon );
	}

	if ( handle->mForceQuit ) {
		handle->mPopComplete = AK_TRUE;
		Condition_Unlock( handle->mDataCon );
		return 0;
	}
	
	assert( ( fd > 0 ) && ( handle->mCycBuffer != NULL ) && ( iSize > 0 ) );
	assert( ( ( handle->mRead >= handle->mCycBuffer ) && ( handle->mRead < (handle->mCycBuffer + handle->mBufferSize) ) ) );
	assert( ( ( handle->mWrite >= handle->mCycBuffer ) && ( handle->mWrite < (handle->mCycBuffer + handle->mBufferSize) ) ) );

	iGetSize = ( handle->mUseSize > iSize )?iSize:(handle->mUseSize);
	iLeavings = iGetSize;
	iGetSize = 0;
	
	while ( iLeavings > 0 )
	{
		iLineSize = ( handle->mCycBuffer + handle->mBufferSize ) - handle->mRead;
		iReadSize = ( iLineSize > iLeavings )?iLeavings:iLineSize;

		iTempRead = iReadSize;
		do {
			iRet = write( fd, handle->mRead + iReadSize - iTempRead, iTempRead );
			if ( ( iRet < 0 ) && ( errno != EINTR ) ) {
				LOGV( "WriteToFs::can't not write date to file, error = %s.\n", strerror(errno) );
				iTempRead = 0;
				break;
			}
			
			iTempRead -= (iRet < 0) ? 0 : iRet;
			
		}while( iTempRead > 0 );

		iGetSize += ( iReadSize - iTempRead );
		
		if ( iReadSize == iLineSize )
			handle->mRead = handle->mCycBuffer;
		else
			handle->mRead += iReadSize;
		
		iLeavings -= iReadSize;
	}
	
	handle->mUseSize -= iGetSize;

	handle->mPopComplete = AK_TRUE;
	Condition_Unlock( handle->mDataCon );

	//logi( "pop signal send!\n" );
	Condition_Signal( &(handle->mWaitForPopCon) );

	if ( iRet < 0 )
		iGetSize = iRet;
	
	return iGetSize;
}
*/


/**
* @brief   get cyc buffer size
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis		the pointer point to the CCycBuffer.
* @return T_S32
* @retval return the cyc buffer size
*/
static T_S32 GetBufferSize( T_pVOID pthis )
{	
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	T_S32 ret = 0;
	
	Condition_Lock( handle->mDataCon );
	ret = handle->mBufferSize;
	Condition_Unlock( handle->mDataCon );
	
	return ret;
}

/**
* @brief   cyc buffer is empty?
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis		the pointer point to the CCycBuffer.
* @return T_S32
* @retval return AK_TRUE if cyc buffer is empty, otherwise return AK_FALSE
*/
static T_BOOL IsEmpty( T_pVOID pthis )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	T_BOOL bRet = AK_FALSE;
	
	Condition_Lock( handle->mDataCon );

	if ( !(handle->mPushComplete) ) {
		return AK_FALSE;
	}
	
	bRet = (handle->mUseSize == 0) ? AK_TRUE : AK_FALSE;
	Condition_Unlock( handle->mDataCon );
	
	return bRet;
}


/**
* @brief   get cyc buffer is idle size
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis		the pointer point to the CCycBuffer.
* @return T_S32
* @retval return cyc buffer is idle size
*/
static T_S32 GetIdleSize( T_pVOID pthis )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	T_S32 ret = 0;
	
	Condition_Lock( handle->mDataCon );
	ret = handle->mBufferSize - handle->mUseSize;
	Condition_Unlock( handle->mDataCon );
	
	return ret;
	
}


/**
* @brief   get cyc buffer is used size
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis		the pointer point to the CCycBuffer.
* @return T_S32
* @retval return cyc buffer is uesd size
*/
static T_S32 GetUsedSize( T_pVOID pthis )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	T_S32 ret = 0;
	
	Condition_Lock( handle->mDataCon );
	ret = handle->mUseSize;
	Condition_Unlock( handle->mDataCon );

	return ret;
}


/**
* @brief   clean the cyc buffer
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis		the pointer point to the CCycBuffer.
* @return T_S32
* @retval return 0 for success, otherwise for failed
*/
static T_S32 Clean( T_pVOID pthis )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	
	Condition_Lock( handle->mDataCon );
	if ( handle->mUseSize == 0 ) {
		Condition_Unlock( handle->mDataCon );
		return 0;
	}
	
	//复位read write指针
	handle->mRead = handle->mWrite = handle->mCycBuffer;
	handle->mUseSize = 0;
	Condition_Unlock( handle->mDataCon );

	ResumeForceQuitState( this );

	return 0;
}


/**
* @brief   let cyc buffer be full state, but no need to push any useful data
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis		the pointer point to the CCycBuffer.
* @return T_S32
* @retval return 0 for success, otherwise for failed
*/
static T_S32 FakePushFull( T_pVOID pthis )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	
	Condition_Lock( handle->mDataCon );
	if ( handle->mUseSize == handle->mBufferSize ) {
		Condition_Unlock( handle->mDataCon );
		return 0;
	}

	//复位read write指针
	handle->mRead = handle->mWrite = handle->mCycBuffer;
	handle->mUseSize = handle->mBufferSize;
	Condition_Unlock( handle->mDataCon );

	return 0;
}


/**
* @brief   if there are some threads block in push or pop process, call this function can
* force all these threads to exit.
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis		the pointer point to the CCycBuffer.
* @return T_S32
* @retval return 0 for success, otherwise for failed
*/
static T_S32 ForceQuit( T_pVOID pthis )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	
	Condition_Lock( handle->mDataCon );

	handle->mForceQuit = AK_TRUE;

	//Condition_Broadcast( &(handle->mWaitForPushCon) );
	//Condition_Broadcast( &(handle->mWaitForPopCon) );
	Condition_Broadcast( &(handle->mDataCon) );
	Condition_Unlock( handle->mDataCon );
	
	return 0;
}


/**
* @brief   call this function to resume the force quit state. If you call
* ForceQuit before, and then you want to push or pop again, please 
* call this function first.
*
* @author hankejia
* @date 2012-07-05
* @param[in] pthis		the pointer point to the CCycBuffer.
* @return T_S32
* @retval return 0 for success, otherwise for failed
*/
static T_S32 ResumeForceQuitState( T_pVOID pthis )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;
	
	Condition_Lock( handle->mDataCon );
	handle->mForceQuit = AK_FALSE;
	Condition_Unlock( handle->mDataCon );

	return 0;
}


/**
* @brief   pop the read point  from cyc buffer
*
* @author hankejia
* @date 2012-11-05
* @param[in] pthis			the pointer point to the CCycBuffer.
* @return T_S32
* @retval return the read point of cycbuffer,if return NULL failed
*/
static T_CHR * PopSingle( T_pVOID pthis, T_S32 iSize )
{
	CCycBuffer *this = ( CCycBuffer * )pthis;
	CycBuffer_handle * handle = (CycBuffer_handle *)this->handle;

	//the cyc buffer is empty, wait for push
	Condition_Lock( handle->mDataCon );
	handle->mPopComplete = AK_FALSE;
	while ( ( handle->mUseSize < iSize ) && ( !(handle->mForceQuit) ) ) {
		//Condition_Unlock( handle->mDataCon );
		Condition_Unlock( handle->mWriteDataCon );
		
		//logi( "PopSingle buffer empty!\n" );
		Condition_Wait( &(handle->mDataCon) );
		//logi( "wake up from push signed!\n" );

		Condition_Unlock( handle->mDataCon );
		Condition_Lock( handle->mWriteDataCon );
		Condition_Lock( handle->mDataCon );
	}

	if ( handle->mForceQuit ) {
		handle->mPopComplete = AK_TRUE;
		Condition_Unlock( handle->mDataCon );
		return NULL;
	}

	assert( ( ( handle->mRead >= handle->mCycBuffer ) && ( handle->mRead < (handle->mCycBuffer + handle->mBufferSize) ) ) );
	assert( ( ( handle->mWrite >= handle->mCycBuffer ) && ( handle->mWrite < (handle->mCycBuffer + handle->mBufferSize) ) ) );

	Condition_Unlock( handle->mDataCon );

	return handle->mRead;
}


//-------------------------------simulate class function register------------------------------

REGISTER_FUN_BEGIN( CCycBuffer, CONSTRUCTOR_FUN( CCycBuffer ), DESTRUCTOR_FUN( CCycBuffer ) )
REGISTER_FUN( SetBufferSize, SetBufferSize )
REGISTER_FUN( CreateCycBuffer, CreateCycBuffer )
REGISTER_FUN( CreateCycBufferEx, CreateCycBufferEx )
REGISTER_FUN( Pop, Pop )
REGISTER_FUN( Push, Push )
REGISTER_FUN( PushSingle, PushSingle )
REGISTER_FUN( WriteToFs, WriteToFs )
REGISTER_FUN( flush, flush )
REGISTER_FUN( GetBufferSize, GetBufferSize )
REGISTER_FUN( GetIdleSize, GetIdleSize )
REGISTER_FUN( GetUsedSize, GetUsedSize )
REGISTER_FUN( IsEmpty, IsEmpty )
REGISTER_FUN( Clean, Clean )
REGISTER_FUN( ForceQuit, ForceQuit )
REGISTER_FUN( ResumeForceQuitState, ResumeForceQuitState )
REGISTER_FUN( DestroyCycBuffer, DestroyCycBuffer )
REGISTER_FUN( FakePushFull, FakePushFull )
REGISTER_FUN_END( CCycBuffer )

REGISTER_SIMULATE_CLASS_C( CCycBuffer )


