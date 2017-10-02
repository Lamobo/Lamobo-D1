#ifndef __ASYNCWRITE_H__
#define __ASYNCWRITE_H__
#include "simulate_class.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct AkAsyncFileWriter
{
	//declare the constructor function is point
	DECLARE_CONSTRUCTOR_POINT( AkAsyncFileWriter );
	//declare the destructor fucntion is point
	DECLARE_DESTRUCTOR_POINT( AkAsyncFileWriter );

	/*
	 * @brief		create the file writer
	 * @param	[in] pthis   			the pointer point to the AkAsyncFileWriter.
	 * @param	[in] fd,				the file is descriptor
	 * @param	[in] IsJustFd			if is ture the AkAsyncFileWriter just use as a fd,
	 *								if is false the AkAsyncFileWriter do the Async process.
	 * @param	[in] writeCacheSize	cache is size
	 * @param	[in] writeBlockSize		each write to the file is size of data.
	 * @return	T_S32
	 * @retval	-1 0 for error,0 for success
	 */
	T_S32 (*createFileWriter)( T_pVOID pthis, T_S32 fd, T_BOOL IsJustFd,
							   T_U32 writeCacheSize, T_U32 writeBlockSize );

	/**
	* @brief   get the file fd
	* @author hankejia
	* @date 2012-07-05
	* @param[in] pthis   			the pointer point to the AkAsyncFileWriter.
	* @return T_S32
	* @@retval 	the file is fd
	*/
	T_S32 (*getFd)( T_pVOID pthis );

	/**
	* @brief   reset the file fd
	* @author hankejia
	* @date 2012-07-05
	* @param[in] pthis   			the pointer point to the AkAsyncFileWriter.
	* @param[in] fd   	  			the file is descriptor
	* @return T_S32
	* @@retval	-1 for error,0 for success
	*/
	T_S32 (*ResetFd)( T_pVOID pthis, T_S32 fd );


	/**
	* @brief   the async write module just do the sync job?
	* @author hankejia
	* @date 2012-07-05
	* @param[in] pthis   			the pointer point to the AkAsyncFileWriter.
	* @return T_BOOL
	* @@retval  return if do the async job return AK_TRUE, do the sync job return false
	*/
	T_BOOL (*IsJustFd)( T_pVOID pthis );
	
	/*
	 * @brief		write buf to file writer
	 * @param	[in]pthis, buf , size
	 * @return	T_S32
	 * @retval	<=0 for error,otherwise the length of write
	 */
	T_S32 (*write)( T_pVOID pthis, T_pVOID buf, T_S32 size );

	/*
	 * @brief		read data from file writer
	 * @param	hFileWriter [in],  buf[in,out], size[in], pthis[in]
	 * @return	T_S32
	 * @retval	<=0 for error,otherwise the length of read
	 */
	T_S32 (*read)( T_pVOID pthis, T_pVOID buf, T_S32 size );
	
	/*
	 * @brief		seek the write postion according to offset & whence
	 * @param	hFileWriter [in],  offset[in], whence[in], pthis[in]
	 * @return	T_S32
	 * @retval	<=0 for error,otherwise for succeed
	 */
	T_U32 (*seek)( T_pVOID pthis, T_U32 offset, T_S32 whence );

	/*
	 * @brief		check wether the file in file writer exsists
	 * @param	pthis[in]
	 * @return	T_S32
	 * @retval	1 for exsists, 0 for not.
	 */
	T_S32 (*fileExist)( T_pVOID pthis );

	/*
	 * @brief		return how many bytes the file writer has written, including cache.
	 * @param	pthis[in]
	 * @return	T_S32
	 * @retval	<=0 for error,otherwise the length that has written.
	 */
	T_U32 (*tell)( T_pVOID pthis );

	/*
	 * @brief		flush all buffer to file
	 * @param	pthis[in]
	 * @return	T_VOID
	 * @retval	NONE
	 */
	T_VOID (*flush)( T_pVOID pthis );

	/*
	 * @brief		start write thread
	 * @param	pthis[in]
	 * @return	T_S32
	 * @retval	0 for success, other for error 
	 */
	T_S32 (*start)( T_pVOID pthis );

	/*
	 * @brief		stop write thread
	 * @param	pthis[in]
	 * @return	T_S32
	 * @retval	0 for success, other for error 
	 */
 	T_S32 (*stop)( T_pVOID pthis );

	/*
	 * @brief		get total written bytes of file writer
	 * @param	NONE
	 * @return	uint32_t
	 * @retval	the length of bytes written
	 */
	T_U32 (*getTotalWriteBytes)( T_pVOID pthis );

	//member variable storage struct point, user should never change it
	T_pVOID handle;
}AkAsyncFileWriter;

REGISTER_SIMULATE_CLASS_H( AkAsyncFileWriter );

/*
 * @brief		construct a file writer to do file writing 
 * @param	fd [in], writebufsize[in], writeblocksize[in]
 * @return	AkAsyncFileWriter
 * @retval	NULL for error, otherwise a handle of AkAsyncFileWriter
 */
 AkAsyncFileWriter* ak_rec_cb_load(int fd, T_BOOL IsJustFd, int writebufsize, int writeblocksize);

/*
 * @brief		release resource allocate by the file writer
 * @param	AkAsyncFileWriter[in]
 * @return	void
 * @retval	NONE
 */
void ak_rec_cb_unload(AkAsyncFileWriter* writer);

/*
 * @brief		used for media lib to print 
 * @param	format [in] ,... [in]
 * @return	T_VOID
 * @retval	NONE
 */
T_VOID ak_rec_cb_printf(T_pCSTR format, ...);

/*
 * @brief		used to alloc memory
 * @param	size [in] 
 * @return	T_pVOID
 * @retval	NULL for error,otherwise the handle of memory allocated.
 */
T_pVOID ak_rec_cb_malloc(T_U32 size);

/*
 * @brief		free memory
 * @param	mem [in] 
 * @return	T_VOID
 * @retval	NONE
 */
T_VOID ak_rec_cb_free(T_pVOID mem);

/*
 * @brief		used for memory copy
 * @param	dest [in, out],  src[in], size[in]
 * @return	T_pVOID
 * @retval	NULL for error,otherwise returns dest.
 */
T_pVOID ak_rec_cb_memcpy(T_pVOID dest, T_pCVOID src, T_U32 size);

/*
 * @brief		read data from file writer
 * @param	hFileWriter [in],  buf[in,out], size[in]
 * @return	T_S32
 * @retval	<=0 for error,otherwise the length of read
 */
T_S32 ak_rec_cb_fread(T_S32 hFileWriter, T_pVOID buf, T_S32 size);

/*
 * @brief		write data to file writer
 * @param	hFileWriter [in],  buf[in], size[in]
 * @return	T_S32
 * @retval	<=0 for error,otherwise the length of write
 */
T_S32 ak_rec_cb_fwrite(T_S32 hFileWriter, T_pVOID buf, T_S32 size);

/*
 * @brief		seek the write postion according to offset & whence
 * @param	hFileWriter [in],  offset[in], whence[in]
 * @return	T_S32
 * @retval	<=0 for error,otherwise for succeed
 */
T_U32 ak_rec_cb_fseek(T_S32 hFileWriter, T_U32 offset, T_U32 whence );

/*
 * @brief		return how many bytes the file writer has written, including cache.
 * @param	hFileWriter [in],
 * @return	T_S32
 * @retval	<=0 for error,otherwise the length that has written.
 */
T_U32 ak_rec_cb_ftell(T_S32 hFileWriter);

/*
 * @brief		check wether file system is busy.
 * @param	void
 * @return	T_BOOL
 * @retval	AK_TRUE for busy,AK_FALSE for idle.
 */
T_BOOL ak_rec_cb_lnx_filesys_isbusy(void);

/*
 * @brief		check wether the file in file writer exsists
 * @param	hFileWriter[in]
 * @return	T_S32
 * @retval	1 for exsists, 0 for not.
 */
T_S32 ak_rec_cb_lnx_fhandle_exist(T_S32 hFileWriter);

/*
 * @brief		media lib will call this while idle.
 * @param	ticks[in]
 * @return	T_BOOL
 * @retval	AK_TRUE for success,other for error.
 */
T_BOOL ak_rec_cb_lnx_delay(T_U32 ticks);


#ifdef __cplusplus
}
#endif

#endif
