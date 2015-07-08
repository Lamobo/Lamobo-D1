// 
//AKFS.cpp: implementation of the CAKFS class.
//
//
//////////////////////////////////////////////////////////////////////


//此文件是文件系统的调用接口
//专门提供burn调用文件系统的接口使用
//注意：此烧录工具使用mount库是支持多个nand进行mount盘功能的。


#include "stdafx.h"
#include "AKFS.h"
#include "transc.h"
#include "ImageCreate.h"


//此处是包含C语言的代码
extern "C"
{
	#include "anyka_types.h"
	#include "mtdlib.h"
    #include "medium.h"
    #include "fs.h"
	#include "eng_dataconvert.h"
	#include "fsa.h"
	#include "fha.h"
}

//回调的声明
T_U8 Test_InStream(T_VOID);
T_VOID Test_OutStream(T_U16 ch);
T_U32 Test_GetSecond(T_VOID);//获取时间
T_VOID Test_SetSecond(T_U32 seconds);//设置时间
T_S32 Test_CrtSem(T_U32 initial_count, T_U8 suspend_type, T_S8 *FuncName , T_U32 Line);//创建信号量
T_S32 Test_RelSem(T_S32 semaphore, T_S8 *FuncName , T_U32 Line);//释放信号量
T_S32 Test_DelSem(T_S32 semaphore, T_S8 *FuncName , T_U32 Line);//删除信息号
T_S32 Test_ObtSem(T_S32 semaphore, T_U32 suspend, T_S8 *FuncName , T_U32 Line);//等待信号量
T_pVOID	Test_RamAlloc(T_U32 size, T_S8 *filename, T_U32 fileline); //分配内存
T_pVOID	Test_RamRealloc(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline);//重分配内存
T_pVOID Test_RamFree(T_pVOID var, T_S8 *filename, T_U32 fileline);//释放内存
T_U32 Test_GetChipType(T_VOID);//获取芯片类型
T_pVOID Test_Memcpy(T_pVOID dst, T_pCVOID src, T_U32 count);//拷贝
T_pVOID Test_MemSet(T_pVOID buf, T_S32 value, T_U32 count);//清空
T_S32   Test_MemCmp(T_pCVOID buf1, T_pCVOID buf2, T_U32 count);//比较
T_pVOID Test_MemMov(T_pVOID dst, T_pCVOID src, T_U32 count);//移走
T_pVOID Test_MemMov_FSA(T_pVOID dst,  T_pCVOID src, T_U32 count);
T_S32 Test_Printf(T_pCSTR s, ...);
void FPrintf(T_pCSTR s, ...);

//T_U32 GetImgCapacity(HANDLE hFile);
static T_VOID fs_sys_sleep(T_U32 ms);

//此处是包含C语言的代码
extern "C"
{
	T_S8  Utl_StrCmpNC(T_pCSTR str1, T_pCSTR str2, T_U16 length)
	{
		return strncmp((char*)str1, (char*)str2, length);
	}

	T_S16 Utl_UStrCmpN(T_pCWSTR str1, T_pCWSTR str2, T_U16 length)
	{
		T_U32 i;

		for(i = 0; i < length; i++)
		{
			if(str1[i] > str2[i])
			{
				return 1;
			}
			else if(str1[i] < str2[i])
			{
				return -1;
			}
		}
    
		return 0;
	}

	T_S16	Utl_UStrCmpNC(T_pCWSTR str1, T_pCWSTR str2, T_U16 length)
	{
		T_U32 i;

		for(i = 0; i < length; i++)
		{
			if(str1[i] > str2[i])
			{
				return 1;
			}
			else if(str1[i] < str2[i])
			{
				return -1;
			}
		}
    
		return 0;
	}
/*由于不用到mtd的函数所以在哪里用一些空函数替代*/
/*
//#if 0

    T_VOID Medium_Destroy(T_PMEDIUM medium)
    {
        if (medium != AK_NULL)
             free((T_pVOID)medium);
    }

    T_VOID MtdLib_SetCallBack(T_PMTDCALLBACK pMtdConfig)
    {

    }

    T_PMEDIUM Nand_CreateMedium(T_PNANDFLASH nand,  T_U32 StartBlk, T_U32 BlkCnt, T_U16 *RsvBlkNum)
    {
        return NULL;
    }

    T_PMEDIUM Medium_CreatePartition(T_PMEDIUM large, T_U32 StartPage,
                                 T_U32 PageCnt, T_U32 SecSize, T_U32 ProtectLevel)
    {
        return NULL;
    }
    
    T_U32 Medium_GetMTDRsvInfo(T_PNANDFLASH nand, T_U32 StartBlock, T_U32 TolBlock, 
                                     T_U16 RsvBlockPerMTD, T_U16 *RsvBlock, T_U16 *RsvNum)
    {
        return 0;
    }
    void Medium_ConnectFS(T_PMEDIUM medium, const T_U8 data[], T_U32 sector,T_U32 size)
    {
    }

    T_U32 NandMtd_Format(T_PNANDFLASH nand, T_U32 StartBlock, T_U32 BlkCnt)
    {
        return 0;
    }
//#endif
	*/
}

//如下函数是回调使用的
T_U8 Test_InStream(T_VOID)
{
     return 0;
}

T_VOID Test_OutStream(T_U16 ch)
{
}
//获取时间
T_U32 Test_GetSecond(T_VOID)
{
	struct tm tmBase;
	time_t tBase;
	time_t tNow;

	memset(&tmBase, 0, sizeof(tmBase));
	tmBase.tm_year = 79;
	tmBase.tm_mon = 12;
	tmBase.tm_mday = 1;

	tBase = mktime(&tmBase);
	time(&tNow);

	return tNow - tBase;
}
//设置时间
T_VOID Test_SetSecond(T_U32 seconds)
{
//	WinSecond = seconds;
}
//创建信号量
T_S32 Test_CrtSem(T_U32 initial_count, T_U8 suspend_type, T_S8 *FuncName , T_U32 Line)
{
	HANDLE handle = NULL;

	handle = CreateSemaphore(NULL, initial_count,initial_count, NULL);
	//if (handle == NULL)
		
	return (T_S32)handle;
}
//释放信号量
T_S32 Test_RelSem(T_S32 semaphore, T_S8 *FuncName , T_U32 Line)
{
	T_S8 result = 0;

	result = ReleaseSemaphore((HANDLE)semaphore,1,NULL);
	return 0;
}
//删除信号量
T_S32 Test_DelSem(T_S32 semaphore, T_S8 *FuncName , T_U32 Line)
{
	if (CloseHandle((HANDLE)semaphore))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
//等待信号量
T_S32 Test_ObtSem(T_S32 semaphore, T_U32 suspend, T_S8 *FuncName , T_U32 Line)
{
	WaitForSingleObject((HANDLE)semaphore,INFINITE);
	return 0;
}
//申请内存
T_pVOID	Test_RamAlloc(T_U32 size, T_S8 *filename, T_U32 fileline)
{
	T_pVOID ptr = NULL;

	ptr =  (T_pVOID)malloc(size);

	memset(ptr, 0, size);

	return ptr;
}
//重申请内存
T_pVOID	Test_RamRealloc(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline)
{
	T_pVOID ptr = NULL;

	ptr = realloc(var, size);

	return ptr;
}
//释放内存
T_pVOID Test_RamFree(T_pVOID var, T_S8 *filename, T_U32 fileline)
{
	free(var);

	return AK_NULL;
}
//获取芯片
T_U32 Test_GetChipType(T_VOID)
{
    return FS_AK32XX;
}
//拷贝
T_pVOID Test_Memcpy(T_pVOID dst, T_pCVOID src, T_U32 count)
{
	PBYTE pDst = (PBYTE)dst;
	PBYTE pSrc = (PBYTE)src;
	UINT i;

	if(NULL == dst || NULL == src)
	{
		return NULL;
	}

	for(i = 0; i < count; i++)
	{
		pDst[i] = pSrc[i];
	}

	return dst;
}

T_pVOID Test_MemSet(T_pVOID buf, T_S32 value, T_U32 count)
{
	if(NULL == buf)
	{
		return NULL;
	}

	UINT i;
	PBYTE pBuf = (PBYTE)buf;

	for(i = 0; i < count; i++)
	{
		pBuf[i] = (BYTE)value;
	}

	return buf;
}

T_S32 Test_MemCmp(T_pCVOID buf1, T_pCVOID buf2, T_U32 count)
{
	return memcmp(buf1, buf2, count);
}

T_pVOID Test_MemMov(T_pVOID dst, T_pCVOID src, T_U32 count)
{
	return memmove(dst, src, count);
}

T_pVOID Test_MemMov_FSA(T_pVOID dst, T_pCVOID src, T_U32 count)
{
	return memmove(dst, src, count);
}


T_S32 Test_Printf(T_pCSTR s, ...)
{
	return 0;
}

void FPrintf(T_pCSTR s, ...)
{
}

static T_VOID fs_sys_sleep(T_U32 ms)
{
    
}
//FSA分配内存
T_pVOID	Test_RamAlloc_FSA(T_U32 size)
{
	T_pVOID ptr = NULL;

	ptr =  (T_pVOID)malloc(size);

	memset(ptr, 0, size);

	return ptr;
}
//FSA释放内存
T_pVOID Test_RamFree_FSA(T_pVOID var)
{
	free(var);

	return AK_NULL;
}
//fsa获取MEDIUM
T_pVOID Test_FSA_GetImgMedium(T_U8 driverID)
{
    DRIVER_INFO DriverInfo = {0};
    
    if (!FS_GetDriver(&DriverInfo, driverID))
    {
        return 0;
    }
	
    return DriverInfo.medium;
}
//打开
T_hFILE Fwl_FileOpen(T_pCSTR path, T_U32 mode)
{
    T_U32 file = 0;
    T_U32 i, len;

	//获取路径的长度
    len = strlen((CHAR *)path);
    for (i=len; i !=0; i--)
    {
        if (path[i] == '/' || path[i] == '\\')
            break;
    }
	//通过路径创建文件夹
    if (i != 0 && path[i-1] != ':')
    {
        T_U8 buf[300]; 

        if (i > 300)
            return FS_INVALID_HANDLE;

        memcpy(buf, path, 300);
        buf[i] = 0;

        if (!File_MkdirsAsc(buf))
        {
            return FS_INVALID_HANDLE;
        }
    }
    //打开文件夹
    file = File_OpenAsc(0, (T_U8 *)path, mode);

    if (!File_IsFile(file))
    {
        File_Close(file);
        
        printf("FWL FILE ERROR: Fwl_FileOpen: \n");
        return FS_INVALID_HANDLE;
    }

    return (T_hFILE)file;
}
//关闭
T_BOOL  Fwl_FileClose(T_hFILE hFile)
{
	//判断句柄是否有效
    if (hFile != FS_INVALID_HANDLE)
    {
        File_Close((T_hFILE)hFile);
    }

	return AK_TRUE;
}
//偏移
T_U32   Fwl_FileSeek(T_hFILE hFile, T_S32 offset, T_U16 origin)
{
	//判断句柄是否有效
    if (hFile == FS_INVALID_HANDLE )
    {
        return FS_INVALID_SEEK;
    }

    return File_SetFilePtr((T_hFILE)hFile, offset, origin);    
}
//读
T_U32   Fwl_FileRead(T_hFILE hFile, T_pVOID buffer, T_U32 count)
{
    T_U32 ret = 0;
	//判断句柄是否有效
    if (hFile != FS_INVALID_HANDLE)
    {     
        ret = File_Read((T_hFILE)hFile, (T_U8*)buffer, count);
    }
    
    return ret;
}
//写数据
T_U32   Fwl_FileWrite(T_hFILE hFile, T_pCVOID buffer, T_U32 count)
{
    T_U32 ret = 0;
	
	//判断句柄是否有效
    if (hFile != FS_INVALID_HANDLE)
    {
        ret = File_Write((T_hFILE)hFile, (T_U8*)buffer, count);
    }

    return ret;
}

//创建文件夹
T_BOOL  Fwl_MkDir(T_pCSTR path)
{
    return File_MkdirsAsc((T_U8* )path);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAKFS::CAKFS()
{
	//initFlag = FALSE;
}

CAKFS::~CAKFS()
{
	UINT i = 0;

	i++;
}

BOOL CAKFS::Init(VOID)
{
    T_FSCallback mountinit;
	T_FSA_LIB_CALLBACK fsainit;

    mountinit.out = Test_OutStream;
    mountinit.in  = Test_InStream;
    mountinit.fGetSecond = Test_GetSecond;
    mountinit.fSetSecond = Test_SetSecond;    
    mountinit.fAscToUni  = AnsiStr2UniStr;
    mountinit.fUniToAsc  = UniStr2AnsiStr;
    mountinit.fRamAlloc  = Test_RamAlloc;
    mountinit.fRamRealloc = Test_RamRealloc;
    mountinit.fRamFree   = Test_RamFree;
    mountinit.fCrtSem    = Test_CrtSem;
    mountinit.fDelSem    = Test_DelSem;
    mountinit.fObtSem    = Test_ObtSem;
    mountinit.fRelSem    = Test_RelSem;
    mountinit.fMemCpy    = Test_Memcpy;
    mountinit.fMemSet    = Test_MemSet;
    mountinit.fMemMov    = Test_MemMov;
    mountinit.fMemCmp    = Test_MemCmp;

    mountinit.fPrintf    = Test_Printf;
    mountinit.fGetChipId = Test_GetChipType;
    mountinit.fSysRst    = NULL;  ///目前应该不用 
    mountinit.fRandSeed  = NULL;  //目前应该不用 
    mountinit.fGetRand   = NULL;  //目前应该不用 
    mountinit.fMountThead= NULL;  //目前应该不用 
    mountinit.fKillThead = NULL;  //目前应该不用 
    mountinit.fSystemSleep= fs_sys_sleep;

    //mount初始化
    if (FS_InitCallBack(&mountinit, 64))
	{		
		return FALSE;
	}
    
	fsainit.MemCmp       = Test_MemCmp;
	fsainit.MemCpy       = Test_Memcpy;
	fsainit.MemSet       = Test_MemSet;
	fsainit.MemMov       = Test_MemMov_FSA;
	fsainit.Printf       = Test_Printf;
	fsainit.RamAlloc     = Test_RamAlloc_FSA;
	fsainit.RamFree      = Test_RamFree_FSA;
	fsainit.GetImgMedium = Test_FSA_GetImgMedium;
	fsainit.fFs.FileClose = Fwl_FileClose;
	fsainit.fFs.FileOpen  = Fwl_FileOpen;
	fsainit.fFs.FileRead  = Fwl_FileRead;
	fsainit.fFs.FileSeek  = NULL;//Fwl_FileSeek;//目前应该不用 
	fsainit.fFs.FileWrite = Fwl_FileWrite;
	fsainit.fFs.FsMkDir   = Fwl_MkDir;
    
	//fsa初始化
	if (FSA_init(&fsainit) != FSA_SUCCESS)
	{
		return FALSE;
	}

	return TRUE;
}

//Destroy文件系统
VOID CAKFS::Destroy(VOID)
{
	//FS_Destroy();
}



//低格nand和sd的分区
BOOL CAKFS::LowFormat(PBYTE PartitionInfo, UINT nNumPartion, UINT resvSize, UINT StartBlock, 
					  UINT MediumType, PBYTE MediumInfo, UINT *StartID, UINT *IDCnt)
{
	FORMAT_INFO info = {0};
	BOOL RetVal;

	//nand
	if (MediumType == TRANSC_MEDIUM_NAND || MediumType == TRANSC_MEDIUM_SPI_NAND)
	{
		info.MediumType = FS_NAND;
	}
	else if (MediumType == TRANSC_MEDIUM_EMMC)
	{
		//介质是sd卡
		info.MediumType = FS_SD;
	}
	else
	{
		return FALSE;
	}
	info.obj  = (T_U32)MediumInfo;

	//低格nand和sd
	RetVal = FS_LowFormat((T_FS_PARTITION_INFO *)PartitionInfo, nNumPartion, resvSize, StartBlock, &info);
    //mount库是调试一个专烧录时用的库，
	//这二个值是返回id和idcnt
	*StartID = (UINT)info.MediumType;
	*IDCnt   = info.obj;

	return RetVal;
}

/***所有的信息都在obj那里***/
BOOL CAKFS::DownloadFile(UINT obj)
{
	T_DOWNLOAD_FILE *download_udisk = (T_DOWNLOAD_FILE *)obj;
	DWORD dwAttribute;
    CImageCreate cImage;
	
	if (NULL == download_udisk)
	{
		return FALSE;
	}
	//获取文件的属性
	dwAttribute = GetFileAttributes(download_udisk->pc_path);
	cImage.ExitFlag = FALSE;
	/**不处理比较文件了比较麻烦**/
	if((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
	{
		//加载文件夹
		if (!cImage.img_add_dir(download_udisk->pc_path, download_udisk->udisk_path))
		{
			return FALSE;
		}
	}
	else
	{
		//加载文件
		if (!cImage.img_add_file(download_udisk->pc_path, download_udisk->udisk_path))
		{
			return FALSE;
		}
	}

	return TRUE;
}
//umount 移动分区
VOID CAKFS::UnMountMemDev(UCHAR DriverID)
{
	FS_UnMountMemDev(DriverID);
	//FS_Destroy();
}
//umount nandflash
VOID CAKFS::UnMountNandFlash(UCHAR DriverID)
{
	FS_UnMountnNandflash(DriverID);
	//FS_Destroy();
}
//mount nandflash
UINT CAKFS::MountNandFlash(UINT NandBase, UINT StartBlock, UCHAR DriverList[], UCHAR *DriverCnt)
{
    return FS_MountNandFlash((T_PNANDFLASH)NandBase, StartBlock, DriverList, DriverCnt);
	//FS_UnMountnNandflash(DriverID);
	//FS_Destroy();
}
//获取分区信息
BOOL CAKFS::GetDriverInfo(UINT StartID, UINT DriverCnt,  UINT *DriverNum, UCHAR *Info, UINT MediumType)
{
	UINT i;
    T_DRIVER_INFO *pDriverInfo = (T_DRIVER_INFO *)Info;
    DRIVER_INFO Driver_Info;

    *DriverNum = 0;

	//分区的个数
    for (i=0; i<DriverCnt; i++)
	{
		//通过id获取相应的分区
		if (FS_GetDriver(&Driver_Info, StartID+i))
		{
            (*DriverNum)++;
            pDriverInfo->DiskName = Driver_Info.DriverID + 'A';
            if (MediumType == TRANSC_MEDIUM_NAND || MediumType == TRANSC_MEDIUM_SPI_NAND)
            {
				//nand
                pDriverInfo->PageCnt  = Driver_Info.medium->capacity >> Driver_Info.medium->SecPerPg;
            }
            else
            {
				//sd
                pDriverInfo->PageCnt  = Driver_Info.medium->capacity;//sd的只有扇区大小
            }
            
            pDriverInfo++;
		}
		else
		{
			return FALSE;
		}
    }

    if (0 == *DriverNum)
    {
        return FALSE;
    }

	return TRUE;
}

//针对snowbirdl的烧录结构
//进行下载镜像文件
BOOL CAKFS::DownloadImg(UINT nID, HANDLE hFile, T_IMG_INFO *img_info, UINT img_buf_len)
{
	T_U32 file_len;
	T_U32 read_len;
	T_U32 write_size;
	T_U8 *pBuf = AK_FALSE;
	T_BOOL ret = AK_FALSE;
	T_hFILE handle = FS_INVALID_HANDLE;
	
	//判断是否合法
	if(INVALID_HANDLE_VALUE == hFile || AK_NULL == img_info)
	{
		return AK_FALSE;
	}
	
	file_len = img_info->data_length;//镜像文件的长度
	//判断文件长度是否0
	if (0 == file_len)
	{
		return AK_FALSE;
	}
	
	pBuf = (T_U8 *)Test_RamAlloc_FSA(img_buf_len);
	memset(pBuf,0,img_buf_len);
	if(AK_NULL == pBuf)
	{

		return AK_FALSE;
	}
	
	//此接口返回一个结构体，
	//使用时要进行转换，库内部实现
	handle = FSA_write_image_begin_burn(img_info);
	if (FS_INVALID_HANDLE == handle)
	{
		FSA_write_image_end_burn(handle);
		Test_RamFree_FSA(pBuf);
		return AK_FALSE;
	}
	
	
	while (1)
	{   
		//读取数据
		ret = ReadFile(hFile, pBuf, img_buf_len, &read_len, NULL);
		if (!ret)
		{
			FSA_write_image_end_burn(handle);
			Test_RamFree_FSA(pBuf);
			return AK_FALSE;
		}
		else
		{
			write_size = read_len;
		}
		//每一次以write_size大小进行写下去
		if (FSA_FAIL == FSA_write_image_burn(handle, pBuf, write_size))
		{
			FSA_write_image_end_burn(handle);
			Test_RamFree_FSA(pBuf);
			return AK_FALSE;
		}

		BT_DownloadImg_length(nID,  write_size);

		file_len -= write_size;
		//如果读到结束就退出去
		if (0 == read_len || 0 == file_len)
		{
			break;
		}
	} 
	//释放内存
	FSA_write_image_end_burn(handle);
	Test_RamFree_FSA(pBuf);
	return TRUE;
}