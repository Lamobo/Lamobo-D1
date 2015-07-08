// SpiImageCreate.cpp: implementation of the SpiImageCreate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "burntool.h"
#include "SpiImageCreate.h"
#include "Config.h"
#include "stdio.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define  SPIBOOTAREA_PAGE_NUM   257//160   //spi bin 文件的开始
#define  LINUX_SPIBOOTAREA_PAGE_NUM   64//160   //spi bin 文件的开始
#define  LEN_4G  (4*1024*1024*1024-1)   //4G
#define RAM_REG_MAX_NUM         100      //
#define BURN_OBJECT_MAX_NUM     32       //

typedef struct 
{
    T_RAM_REG ramReg[RAM_REG_MAX_NUM];
    UINT      numReg;
}T_RAM_REG_PARA;

typedef struct
{
    HANDLE hBurnThread;//hBurnThread
    HANDLE hEvent;//hEvent
    HANDLE hUSBDevice;//hUSBDevice
	HANDLE t_usb_init_event;//t_usb_init_event
    UINT   nID;
    UINT   status;
	UINT   download_length;//download_length
    TCHAR  strUSBName[MAX_PATH+1];
    UINT   NandChipCnt;
    UINT   AttachUSBCnt;
	TCHAR  strHubport[MAX_PATH+1];//用于保存初始化的通道HUB port ID
	union
	{
		T_NAND_PHY_INFO NandPhyInfo;//NandPhyInfo
		T_SFLASH_PHY_INFO SpiphyInfo;//SpiphyInfo
	};
    
}T_BURN_OBJECT;

T_SFLASH_PHY_INFO SpiInfo;  //spi参数
HANDLE Spi_hHandle = NULL;//Spi_hHandle

extern CConfig theConfig;//config
extern CBurnToolApp theApp;//theApp

static T_BURN_OBJECT g_pBurnObject[BURN_OBJECT_MAX_NUM] = {0};

T_pVOID	Spi_Malloc(T_U32 size);
T_pVOID Spi_Free(T_pVOID var);
T_pVOID Spi_Memcpy(T_pVOID dst, T_pCVOID src, T_U32 count);
T_pVOID Spi_MemSet(T_pVOID buf, T_S32 value, T_U32 count);
T_S32 Spi_MemCmp(T_pCVOID buf1, T_pCVOID buf2, T_U32 count);
T_pVOID Spi_MemMov(T_pVOID dst, const T_pCVOID src, T_U32 count);
T_S32 Spi_Printf(T_pCSTR s, ...);
void config_spiboot_aspen(BYTE *sflashboot, DWORD dwSize);
void config_spiboot_snowbird(BYTE *sflashboot, UINT chip_type, DWORD dwSize, UINT clock);
void ConfigSpibootParam(UINT nID, BYTE *buf);
T_pVOID fhalib_init(T_U32 eMedium, T_U32 eMode);
T_BOOL spi_flash_erase(T_U32 sector);//
T_BOOL spi_flash_write(T_U32 page, const T_U8 *buf);
T_BOOL spi_flash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt);
T_U32 FHA_Spi_Erase(T_U32 nChip,  T_U32 nPage);//
T_U32 FHA_Spi_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType);
T_U32 FHA_Spi_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType);
BOOL  FHA_download_bin(T_VOID);//
BOOL  FHA_download_boot(T_VOID);//

SpiImageCreate::SpiImageCreate()
{

}

SpiImageCreate::~SpiImageCreate()
{

}
//根据芯片类型获取相应的芯片
E_FHA_CHIP_TYPE get_fha_chip(T_VOID)
{
	E_FHA_CHIP_TYPE fha_chip = FHA_CHIP_10XX;
	
	switch(theConfig.chip_type)
	{
		case CHIP_880X://CHIP_880X
			fha_chip = FHA_CHIP_880X;
			break;
		case CHIP_10X6://CHIP_10X6
		case CHIP_1080L://CHIP_1008L
		case CHIP_10XXC://CHIP_10XXC
		case CHIP_1080A://CHIP_1080A
			fha_chip = FHA_CHIP_10XX;
			break;
		case CHIP_980X://CHIP_980X
			fha_chip = FHA_CHIP_980X;
			break;
		case CHIP_37XX://CHIP_37XX
			fha_chip = FHA_CHIP_37XX;
			break;
		case CHIP_39XX://CHIP_39XX
			fha_chip = FHA_CHIP_39XX;
			break;
		case CHIP_11XX://CHIP_11XX
			fha_chip = FHA_CHIP_11XX;
			break;
		default:
			break;
	}

	return fha_chip;
}
//根据平台和芯片进行获取平台下哪一个芯片平台
E_FHA_PLATFORM_TYPE get_fha_platform(T_VOID)
{
	E_FHA_PLATFORM_TYPE fha_platform = PLAT_SPOT;//PLAT_SPOT
	
	switch(theConfig.planform_tpye)
	{
		case E_ROST_PLANFORM:
			if ((CHIP_11XX == theConfig.chip_type) || (CHIP_10X6 == theConfig.chip_type))
			{
				fha_platform = PLAT_SPOT; // CHIP_11XX and CHIP_10X6
			}
			else if ((CHIP_37XX == theConfig.chip_type) || (CHIP_980X == theConfig.chip_type))
			{
				fha_platform = PLAT_SWORD;// CHIP_37XX 英CHIP_980X
			}
			break;
			
		case E_LINUX_PLANFORM:
			fha_platform = PLAT_LINUX;//PLAT_LINUX
			break;
			
		default:
			break;
	}

	return fha_platform;
}

//媒介初始化
T_pVOID fhalib_init(T_U32 eMedium, T_U32 eMode)
{
	T_FHA_LIB_CALLBACK  pCB;
	T_FHA_INIT_INFO     pInit;
	T_SPI_INIT_INFO     spi_fha; 
	T_pVOID             pInfo = AK_NULL;

	pInit.nChipCnt   = 1;//nChipCnt
    pInit.nBlockStep = 1; //nBlockStep
    pInit.eAKChip    = get_fha_chip();//eAKChip
    pInit.ePlatform  = get_fha_platform();//ePlatform
    pInit.eMedium    = (E_FHA_MEDIUM_TYPE)eMedium;
    pInit.eMode      = (E_BURN_MODE)eMode;

	if (MEDIUM_SPIFLASH == eMedium)
    {
        //for spi init
        pCB.Erase  = FHA_Spi_Erase;//Erase
        pCB.Write  = FHA_Spi_Write;//Write
        pCB.Read   = FHA_Spi_Read;//Read
        pCB.ReadNandBytes = (FHA_ReadNandBytes)AK_NULL;

        spi_fha.PageSize = SpiInfo.page_size;//page_size
        spi_fha.PagesPerBlock = SpiInfo.erase_size / SpiInfo.page_size;//PagesPerBlock
		if(pInit.ePlatform == PLAT_LINUX)
		{
			spi_fha.BinPageStart = LINUX_SPIBOOTAREA_PAGE_NUM;//BinPageStart
		}
		else
		{
			spi_fha.BinPageStart = SPIBOOTAREA_PAGE_NUM - 1;//BinPageStart
		}

        pInfo    = (T_pVOID)(&spi_fha);
		
	}

    pCB.RamAlloc = Spi_Malloc; //RamAlloc
    pCB.RamFree  = Spi_Free;//RamFree
    pCB.MemSet   = Spi_MemSet;//MemSet
    pCB.MemCpy   = Spi_Memcpy;//MemCpy
    pCB.MemCmp   = Spi_MemCmp;//MemCmp
    pCB.MemMov   = Spi_MemMov;//MemMov
    pCB.Printf   = Spi_Printf;//Printf

    if (FHA_SUCCESS == FHA_burn_init(&pInit, &pCB, pInfo))//FHA_burn_init
    {
        return pInfo;
    }
    else
    {
    	printf("FHA_burn_init inits fail\n");
        return AK_NULL;
    }    
}
//调用spi的低层擦
T_U32 FHA_Spi_Erase(T_U32 nChip,  T_U32 nPage)
{
	T_U32 nBlock = nPage / (SpiInfo.erase_size/SpiInfo.page_size);//nBlock
	if (!spi_flash_erase(nBlock))//spi_flash_erase
	{
		printf("fw:erase fail\n");
		return FHA_FAIL;
	}

	printf("+fw:erase :%d \r\n", nBlock);

	return FHA_SUCCESS;
}
//调用spi的低层读
T_U32 FHA_Spi_Read(T_U32 nChip,  T_U32 nPage, T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen , T_U32 eDataType)
{ 
	T_U32 i = 0;

	for (i = nPage; i < nDataLen+nPage; i++)
	{
		if (!spi_flash_read(i, &pData[(i-nPage) * SpiInfo.page_size], 1))//
		{
			return FHA_FAIL;
		}
	}
    
	return FHA_SUCCESS;
}
//调用spi的低层写
T_U32 FHA_Spi_Write(T_U32 nChip, T_U32 nPage, const T_U8 *pData, T_U32 nDataLen,  T_U8 *pOob, T_U32 nOobLen, T_U32 eDataType)
{	
	T_U32 i = 0;

	for (i = nPage; i < nDataLen + nPage; i++)
	{
		if (!spi_flash_write(i, &pData[(i-nPage) * SpiInfo.page_size]))//
		{
			return FHA_FAIL;
		} 
	}
    
	return FHA_SUCCESS;
}

//下载bin 文件
BOOL FHA_download_bin(T_VOID)
{
	T_FHA_BIN_PARAM  BinParam = {0};
	TCHAR file_path[MAX_PATH+1] = {0};
	DWORD count = 0;
	BYTE *buf = NULL;
	
	buf = (BYTE *)Spi_Malloc(4096 * sizeof(BYTE));//
	Spi_MemSet(buf, 0, 4096 * sizeof(BYTE));//

	if(NULL == buf)
	{
		return FALSE;
	}
	
	USES_CONVERSION;
	for(UINT i = 0; i < theConfig.download_nand_count; i++)
	{
		BinParam.ld_addr = theConfig.download_nand_data[i].ld_addr;//ld_addr
		_tcscpy(file_path, theApp.ConvertAbsolutePath(theConfig.download_nand_data[i].pc_path));//pc_path
		memcpy(BinParam.file_name, T2A(theConfig.download_nand_data[i].file_name), 16);//file_name
		BinParam.bBackup = theConfig.download_nand_data[i].bBackup;//bBackup
		BinParam.bCheck = theConfig.download_nand_data[i].bCompare;//bCompare
		BinParam.bUpdateSelf = theConfig.bUpdateself;//bUpdateself

		//spi不支持备份
		if (1 == BinParam.bBackup)
		{
			AfxMessageBox(_T("SPI不支持备份，系统自动转换为不备份"));
			BinParam.bBackup = 0;
			theConfig.download_nand_data[i].bBackup = 0;
		}

		//设置每一个bin文件后的扩展大小
		if (theConfig.download_nand_data[i].bin_revs_size != 0)
		{
			if (FHA_FAIL == FHA_set_bin_resv_size((UINT)theConfig.download_nand_data[i].bin_revs_size))
			{
				Spi_Free(buf);//
				return FALSE;
			}
		}
		
		HANDLE hFile = CreateFile(file_path, 
        GENERIC_READ, 
        FILE_SHARE_READ, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL);//创建
		
		if(INVALID_HANDLE_VALUE == hFile)//INVALID_HANDLE_VALUE
		{
			Spi_Free(buf);//
			return FALSE;
		}

		//获取bin文件大小
		UINT dwSize = GetFileSize(hFile, NULL);//GetFileSize

		if(0xFFFFFFFF == dwSize)
		{
			CloseHandle(hFile);//关闭
			Spi_Free(buf);
			return FALSE;
		}
		
		BinParam.data_length = dwSize;//dwSize

		if (FHA_FAIL == FHA_write_bin_begin(&BinParam))//FHA_write_bin_begin
		{
			printf("FHA_write_bin_begin fail\n");
			CloseHandle(hFile);
			Spi_Free(buf);
			return FALSE;
		}

		//每次读4096 byte 进buf 并写bin
		while(ReadFile(hFile, buf, 4096, &count, NULL))//ReadFile
		{
			if(0 == count)
			{
				break;
			}
			
			if (FHA_FAIL == FHA_write_bin(buf, count))//FHA_write_bin
			{
				printf("FHA_write_bin fail\n");
				CloseHandle(hFile);
			    Spi_Free(buf);
				return FALSE;

			}
		}
	
		CloseHandle(hFile);//CloseHandle
	}
	Spi_Free(buf);//Spi_Free

    return TRUE;
}


BOOL FHA_Set_FS(T_VOID)
{
	T_SPIFLASH_PARTION_INFO *spi_partInfo = NULL;
	BYTE *buf = NULL;
	UINT i = 0;
	
	buf = (BYTE *)Spi_Malloc( 4 + sizeof(T_SPIFLASH_PARTION_INFO)*theConfig.format_count + theConfig.format_count*4);
	if (buf == NULL)
	{
		return FALSE;
	}
	spi_partInfo = new T_SPIFLASH_PARTION_INFO[theConfig.format_count];
	if (spi_partInfo == NULL)
	{
		Spi_Free(buf);//Spi_Free
		return FALSE;
	}
	
	memcpy(spi_partInfo, theConfig.format_data, sizeof(T_PARTION_INFO)*theConfig.format_count);
	
	//获取spi分区的信息
	for (i = 0; i < theConfig.format_count; i++)
	{			
		spi_partInfo[i].Size = theConfig.spi_format_data[i].Size;   //此变量不相同
	}

	//记录盘符的个数
	memcpy(buf, &theConfig.format_count, 4);
	memcpy(buf+4, spi_partInfo, sizeof(T_SPIFLASH_PARTION_INFO)*theConfig.format_count);

	FHA_set_fs_part(buf, sizeof(T_SPIFLASH_PARTION_INFO)*theConfig.format_count + 4);

	delete[] spi_partInfo; 
	Spi_Free(buf);//Spi_Free

	return TRUE;
}


//下载mtd镜像文件
BOOL FHA_download_MTD(T_VOID)
{
	T_FHA_BIN_PARAM  BinParam = {0};
	TCHAR file_path[MAX_PATH+1] = {0};
	DWORD count = 0;
	BYTE *buf = NULL;
	
	buf = (BYTE *)Spi_Malloc(4096 * sizeof(BYTE));//
	Spi_MemSet(buf, 0, 4096 * sizeof(BYTE));//
	
	if(NULL == buf)
	{
		return FALSE;
	}
	
	USES_CONVERSION;
	for(UINT i = 0; i < theConfig.download_mtd_count; i++)
	{
		_tcscpy(file_path, theApp.ConvertAbsolutePath(theConfig.download_mtd_data[i].pc_path));//pc_path
		BinParam.bCheck = theConfig.download_mtd_data[i].bCompare;//bCompare

		HANDLE hFile = CreateFile(file_path, 
			GENERIC_READ, 
			FILE_SHARE_READ, 
			NULL, 
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL, 
			NULL);//创建
		
		if(INVALID_HANDLE_VALUE == hFile)//INVALID_HANDLE_VALUE
		{
			Spi_Free(buf);//
			return FALSE;
		}
		
		//获取bin文件大小
		UINT dwSize = GetFileSize(hFile, NULL);//GetFileSize
		
		if(0xFFFFFFFF == dwSize)
		{
			CloseHandle(hFile);//关闭
			Spi_Free(buf);
			return FALSE;
		}
		
		BinParam.data_length = dwSize;//dwSize
		
		if (FHA_FAIL == FHA_write_MTD_begin(&BinParam))//FHA_write_bin_begin
		{
			printf("FHA_write_bin_begin fail\n");
			CloseHandle(hFile);
			Spi_Free(buf);
			return FALSE;
		}
		
		//每次读4096 byte 进buf 并写bin
		while(ReadFile(hFile, buf, 4096, &count, NULL))//ReadFile
		{
			if(0 == count)
			{
				break;
			}
			
			if (FHA_FAIL == FHA_write_MTD(buf, count))//FHA_write_bin
			{
				printf("FHA_write_bin fail\n");
				CloseHandle(hFile);
				Spi_Free(buf);
				return FALSE;
			}
		}
		
		CloseHandle(hFile);//CloseHandle
	}
	Spi_Free(buf);//Spi_Free
	
    return TRUE;
}

//下载mac地址和序列号
BOOL FHA_download_MAC_SERIAL(UINT spi_nid)
{
	USES_CONVERSION;
	/***********************************************/
	//判断是否烧录MAC地址
	if(theConfig.macaddr_flag) 
	{
		TCHAR macbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
		BYTE mac_tempbuf[MAX_MAC_SEQU_ADDR_COUNT*2+1] = {0};
		UINT *ptr = (UINT *)mac_tempbuf; 
		
		_tcscpy(macbuf, theConfig.mac_start_high);
		_tcscat(macbuf, A2T(":"));
		_tcscat(macbuf, theConfig.mac_start_low);

		ptr[0] = wcslen(macbuf);
		memcpy(&mac_tempbuf[4], T2A(macbuf), ptr[0]);

		if((theConfig.spiflash_parameter[spi_nid].erase_size != 4096) 
			&& (theConfig.spiflash_parameter[spi_nid].program_size != 16))
		{
			MessageBox(NULL, _T("spi erase_size != 4096 or program_size != 16"), NULL,MB_OK);
			return FALSE;
		}
		
		//写mac地址到安全区内
		if (FHA_asa_write_file((T_U8 *)"MACADDR", mac_tempbuf, ptr[0]+4, ASA_MODE_CREATE) == FHA_FAIL)
		{
			return FALSE;
		}
	}

	//判断是否烧录序列号
	if(theConfig.sequenceaddr_flag) 
	{
		TCHAR serailbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
		BYTE serail_tempbuf[MAX_MAC_SEQU_ADDR_COUNT*2+1] = {0};
		UINT *ptr = (UINT *)serail_tempbuf; 

		_tcscpy(serailbuf, theConfig.sequence_start_high);
		_tcscat(serailbuf, theConfig.sequence_start_low);
		
		ptr[0] = wcslen(serailbuf);
		memcpy(&serail_tempbuf[4], T2A(serailbuf), ptr[0]);

		if((theConfig.spiflash_parameter[spi_nid].erase_size != 4096) 
			&& (theConfig.spiflash_parameter[spi_nid].program_size != 16))
		{
			MessageBox(NULL, _T("spi erase_size != 4096 or program_size != 16"), NULL,MB_OK);
			return FALSE;
		}
				
		if (FHA_asa_write_file((T_U8 *)"SERADDR", serail_tempbuf, ptr[0]+4, ASA_MODE_CREATE) == FHA_FAIL)
		{
			return FALSE;
		}
	}
	
    return TRUE;
}

//下载boot
BOOL FHA_download_boot(UINT clock)
{
	DWORD count;
	UINT ChipType = theConfig.chip_type;
	BYTE *buf = NULL;
		
	HANDLE boot_hFile = CreateFile(theApp.ConvertAbsolutePath(theConfig.path_nandboot), 
        GENERIC_READ, 
        FILE_SHARE_READ, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL);//CreateFile

    if(INVALID_HANDLE_VALUE == boot_hFile)//判断
    {
        return FALSE;
    }

    UINT boot_dwSize = GetFileSize(boot_hFile, NULL);//boot_dwSize

	if(0xFFFFFFFF == boot_dwSize)
	{
		CloseHandle(boot_hFile);
		return FALSE;
	}

	buf = (BYTE *)Spi_Malloc(boot_dwSize * sizeof(BYTE));
	Spi_MemSet(buf, 0, boot_dwSize * sizeof(BYTE));

	if(NULL == buf)//分配失败
	{
		return FALSE;
	}
	
	if (FHA_FAIL == FHA_write_boot_begin(boot_dwSize))//FHA_write_boot_begin
	{
		printf("FHA_write_boot_begin fail\n");
		return FALSE;
	}

	//读boot 的前512 byte
	if (!ReadFile(boot_hFile, buf, 512,  &count, NULL))//ReadFile
	{
		CloseHandle(boot_hFile);
		return FALSE;
	}
	
	if (512 != count)
	{
		printf("read fail\n");
	}

	//根据不同的芯片改变某些值
	switch(ChipType)
	{
		case CHIP_780X:
		case CHIP_880X:
		case CHIP_980X:
		case CHIP_37XX:
		case CHIP_39XX:
			config_spiboot_aspen(buf, boot_dwSize - 512);//config_spiboot_aspen
			break;
		case CHIP_10X6:
		case CHIP_1080A:
		case CHIP_1080L:
		case CHIP_11XX:
		case CHIP_10XXC:
			config_spiboot_snowbird(buf, ChipType, boot_dwSize, clock);//config_spiboot_snowbird
			break;		
		default:
			break;
	}

	//读boot 512 后的byte
	if (!ReadFile(boot_hFile, &buf[512], boot_dwSize - 512,  &count, NULL))
	{
		CloseHandle(boot_hFile);
		return FALSE;
	}

	if ((boot_dwSize - 512) != count)//读长度不一样
	{
		printf("read fail\n");
	}

	ConfigSpibootParam(0, buf);//ConfigSpibootParam
	
	//写boot
	if (FHA_FAIL == FHA_write_boot(buf, boot_dwSize))//FHA_write_boot
	{
		printf("FHA_write_boot fail\n");
		return FALSE;
	}
		
	Spi_Free(buf);
	CloseHandle(boot_hFile);//CloseHandle

    return TRUE;
}
    
void config_spiboot_aspen(BYTE *sflashboot, DWORD dwSize)
{
	T_RAM_REG_PARA g_RamReg = {NULL, 0};
	UINT i;


	if(NULL == sflashboot)
	{
		return;
	}

	*(DWORD *)(sflashboot + 0xC) = dwSize;  //data size


	g_RamReg.numReg = config_ram_param(g_RamReg.ramReg);

	//setup ram info
	if (theConfig.chip_type == CHIP_39XX)
	{
		for(i = 0; i < g_RamReg.numReg; i++)
		{
			*(DWORD *)(sflashboot + 0x28 + 8*i) = g_RamReg.ramReg[i].reg_addr;      //地址
			*(DWORD *)(sflashboot + 0x28 + 8*i + 4) = g_RamReg.ramReg[i].reg_value;  //值
		}
	} 
	else
	{
		for(i = 0; i < g_RamReg.numReg; i++)
		{
			*(DWORD *)(sflashboot + 0x18 + 8*i) = g_RamReg.ramReg[i].reg_addr;      //地址
			*(DWORD *)(sflashboot + 0x18 + 8*i + 4) = g_RamReg.ramReg[i].reg_value;  //值
		}
	}
}


//修改boot分区前512字节的内容中的密码
void config_spiboot_snowbird(BYTE *sflashboot, UINT chip_type, DWORD dwSize, UINT clock)
{
	UINT Asciclock = 60000000; //初始频率60M
	BYTE clock_temp = 0; //临时变量

	if(NULL == sflashboot)
	{
		return;
	}

	*(BYTE *)(sflashboot + 0x25) = 'N';//
	
	if ((CHIP_11XX == chip_type) || (CHIP_1080L == chip_type)  || (CHIP_10XXC == chip_type))//nn2
	{
		*(BYTE *)(sflashboot + 0x26) = 'N';//
	}
	else
	{
		*(BYTE *)(sflashboot + 0x26) = 'B';//
	}
	
	if (CHIP_10X6 == chip_type)//nb1
	{
		*(BYTE *)(sflashboot + 0x27) = '1';//
	}
	else if (CHIP_1080L == chip_type)//nnL
	{
		*(BYTE *)(sflashboot + 0x27) = 'L';
	}
	else if (CHIP_10XXC == chip_type)//nnc
	{
		*(BYTE *)(sflashboot + 0x27) = 'C';
	}
	else
	{
		*(BYTE *)(sflashboot + 0x27) = '2';//
	}

	*(DWORD *)(sflashboot + 0x28) = dwSize;  //data size

	if (Asciclock/clock >= 2)
	{
		clock_temp = (Asciclock/clock)/2-1;  //根据烧录工具的传进来的参数进行决定
	}
	*(WORD *)(sflashboot + 0x2C)  = clock_temp; //spi参数的clock
	
	if(CHIP_1080L == chip_type || CHIP_10XXC == chip_type)
	{
		*(DWORD *)(sflashboot + 0x30) = 0x800000; //10L
	}
}
//修spi 的参数
void ConfigSpibootParam(UINT nID, BYTE *buf)
{
	UINT offset = 0;

	for (UINT i = 0; i < 600; i++)
	{
		if (('S' == buf[i]) && ('P' == buf[i+1]) && ('I' == buf[i+2]) && ('P' == buf[i+3]))
		{
			offset = i;//
			break;//
		}
	}

	if (i < 600)
	{
		memcpy(buf + offset + 4, &g_pBurnObject[nID-1].SpiphyInfo, sizeof(T_SFLASH_PHY_INFO));
	}
}
//实现spi的低层擦
T_BOOL spi_flash_erase(T_U32 sector)
{
	T_U32  pos = 0;
	T_U32 dwWriteSize = 0;
	T_U32 i = 0;
	T_S8  *ptmp = (signed char *)AK_NULL;
	HANDLE filehandle = Spi_hHandle;
	T_U32 high_pos = 0;
	T_U32 *pSpare = (unsigned long *)AK_NULL;
    T_U32  NotSpareSize = SpiInfo.page_size;//NotSpareSize
    T_U32  dwptr;
    T_U32 tmptestbyte = SpiInfo.erase_size;//tmptestbyte
    //high_pos = (T_U32)(tmptestbyte/LEN_4G);

	pos = sector * SpiInfo.erase_size;

    dwptr = SetFilePointer( filehandle, pos , (long *)&high_pos, FILE_BEGIN);//SetFilePointer
    if ( T_U32_MAX == dwptr && (GetLastError() != NO_ERROR) )
    {
        printf("spi_flash_erase: SetFilePointer faild  !\r\n");
		return AK_FALSE;
    }
  
     
	ptmp = (T_S8 *)Spi_Malloc(SpiInfo.page_size);
	if(ptmp == AK_NULL)
	{
		printf("spi_flash_erase: ptmp malloc  size faild !\r\n");
		return AK_FALSE;
  	}
 
	memset(ptmp, 0xff, SpiInfo.page_size);
 
	for(i = 0; i < SpiInfo.erase_size/SpiInfo.page_size; i++)//循环写
	{
	
		if(WriteFile(filehandle, ptmp, NotSpareSize, &dwWriteSize, NULL) == 0)//WriteFile
        {	
            DWORD result;
		    result = GetLastError();
            printf("erase err:%d\r\n",result);
			return AK_FALSE;
		}
		else if(dwWriteSize!=NotSpareSize )//写长度不对
		{
			printf("spi_flash_erase: WriteFile faild  !\r\n");
			return AK_FALSE;
		}
	
	}
	
	Spi_Free(ptmp);

	return AK_TRUE;
}
//实现spi的低层写
T_BOOL spi_flash_write(T_U32 page, const T_U8 *buf)
{
	T_U32 blockId = page / (SpiInfo.erase_size/SpiInfo.page_size);//blockId
	T_U32 dwWriteSize = 0;
	HANDLE filehandle = Spi_hHandle;//filehandle
	T_U32  low_pos = page*SpiInfo.page_size;//low_pos
	T_U32  NotSpareSize = SpiInfo.page_size;//NotSpareSize
	T_U32  high_pos = 0;
    T_U32  dwptr;
    T_U32 tmptestbyte = page*SpiInfo.page_size;//tmptestbyte
	//   high_pos = (T_U32)(tmptestbyte/LEN_4G);

	if(filehandle==INVALID_HANDLE_VALUE)
	{
		printf("spi_flash_write : filehandle==INVALID_HANDLE_VALUE\r\n");
		return AK_FALSE;
	}
	
    dwptr = SetFilePointer( filehandle, low_pos , (long *)&high_pos, FILE_BEGIN);//SetFilePointer
    if ( T_U32_MAX == dwptr && (GetLastError() != NO_ERROR) )
    {
        printf("spi_flash_write  : SetFilePointer faild\r\n");
		return AK_FALSE;
    }
	
	if(WriteFile(filehandle, buf, NotSpareSize, &dwWriteSize, NULL)==0)//WriteFile
	{
		DWORD result;
		result = GetLastError();
		printf("spi_flash_write : WriteFile NFC_FIFO_SIZE faild,err:%d\r\n", result);
		return AK_FALSE;
	}
	
	if(dwWriteSize != NotSpareSize)
	{
		printf("spi_flash_write : dwReadSize!=NFC_FIFO_SIZE\r\n");
		return AK_FALSE;
	}
	
	return AK_TRUE;

}
//实现spi的低层读
T_BOOL spi_flash_read(T_U32 page, T_U8 *buf, T_U32 page_cnt)
{
	T_U32 blockId = page /(SpiInfo.erase_size/SpiInfo.page_size);//blockId
	HANDLE filehandle = Spi_hHandle;
	T_U32  low_pos = page*SpiInfo.page_size;//low_pos
	T_U32  dwReadSize = 0;
	T_U32  NotspareSize = page_cnt*SpiInfo.page_size;//NotspareSize
	T_U32  high_pos=0, dwptr;
    T_U32 tmptestbyte = page*SpiInfo.page_size;//tmptestbyte
    //high_pos = (T_U32)(tmptestbyte/LEN_4G);
		
	if(filehandle==INVALID_HANDLE_VALUE)
	{
		printf("nand_readsector_large : filehandle==INVALID_HANDLE_VALUE\r\n");
		return AK_FALSE;
	}
	
    dwptr = SetFilePointer( filehandle, low_pos , (long *)&high_pos, FILE_BEGIN);//SetFilePointer
    if ( T_U32_MAX == dwptr && (GetLastError() != NO_ERROR) )
    {
        printf("nand_readsector_large: SetFilePointer faild  !\r\n");
		return AK_FALSE;
    }
	
	
	if(ReadFile(filehandle,buf, NotspareSize, &dwReadSize, NULL)==0)//读
	{
        DWORD result;
		result = GetLastError();
		printf("nand_readsector_large : WriteFile NFC_FIFO_SIZE faild,err:%d\r\n",result);
		return AK_FALSE;
	}
	
	if(dwReadSize!= NotspareSize)
	{
		printf("nand_readsector_large : dwReadSize!=NFC_FIFO_SIZE\r\n");
		return AK_FALSE;
	}
		
	return AK_TRUE;
	
}

T_pVOID	Spi_Malloc(T_U32 size)
{
	T_pVOID ptr = NULL;

	ptr =  (T_pVOID)malloc(size);

	memset(ptr, 0, size);//

	return ptr;
}

T_pVOID Spi_Free(T_pVOID var)
{
	free(var);//

	return AK_NULL;
}

T_pVOID Spi_Memcpy(T_pVOID dst, T_pCVOID src, T_U32 count)
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
		pDst[i] = pSrc[i];//
	}

	return dst;
}

T_pVOID Spi_MemSet(T_pVOID buf, T_S32 value, T_U32 count)
{
	if(NULL == buf)
	{
		return NULL;
	}

	UINT i;
	PBYTE pBuf = (PBYTE)buf;

	for(i = 0; i < count; i++)
	{
		pBuf[i] = (BYTE)value;//
	}

	return buf;
}

T_S32 Spi_MemCmp(T_pCVOID buf1, T_pCVOID buf2, T_U32 count)
{
	return memcmp(buf1, buf2, count);
}

T_pVOID Spi_MemMov(T_pVOID dst, const T_pCVOID src, T_U32 count)
{
	return memmove(dst, src, count);
}

T_S32 Spi_Printf(T_pCSTR s, ...)
{
	return 0;
}

//入口
void SpiImageCreate::Spi_Enter(CString spiName, CString strPath)
{
	UINT i = 0;

	//根据spi型号获取参数
	USES_CONVERSION;

	for(i = 0; i < theConfig.spiflash_parameter_count; i++)//循环查找
	{
		if (_tcscmp(spiName, A2T(theConfig.spiflash_parameter[i].des_str)) == 0)
		{
			memcpy(&SpiInfo, &theConfig.spiflash_parameter[i], sizeof(T_SFLASH_PHY_INFO_TRANSC));
			break;
		}
	}

	if (i == theConfig.spiflash_parameter_count)//没有找到
	{
		MessageBox(NULL, _T("not find the spiflash!"), NULL,MB_OK);
		return;
	}
	
	
	//打开handle
	Spi_hHandle = CreateFile(strPath, GENERIC_WRITE | GENERIC_READ,
					     FILE_SHARE_WRITE | FILE_SHARE_READ,
					     NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == Spi_hHandle)
	{
        MessageBox(NULL, _T("creat spiimage CreateFile fail!"), NULL,MB_OK);
		return;
	}

	//初始spi	
	if (AK_NULL == fhalib_init(MEDIUM_SPIFLASH, MODE_NEWBURN))//fhalib_init(MEDIUM_SPIFLASH, MODE_NEWBURN, &SpiInfo))
    {
		printf("fhalib_init fail\n");
		MessageBox(NULL, _T("creat spiimage fhalib_init fail!"), NULL,MB_OK);
		return;
    }
	
	//下载bin	
	if (!FHA_download_bin()) //
	{
		printf("FHA_download_bin fail\n");
		MessageBox(NULL, _T("creat spiimage download bin fail!"), NULL,MB_OK);
		return;
	}

	//下载mtd
	if (theConfig.chip_type == CHIP_39XX)
	{
		if (!FHA_download_MAC_SERIAL(i)) //
		{
			printf("FHA_download_MAC_SERIAL fail\n");
			MessageBox(NULL, _T("creat spiimage write mac or serial fail!"), NULL,MB_OK);
			return;
		}

		if (!FHA_download_MTD()) //
		{
			printf("FHA_download_MTD fail\n");
			MessageBox(NULL, _T("creat spiimage download MTD fail!"), NULL,MB_OK);
			return;
		}

		if (!FHA_Set_FS()) //
		{
			printf("FHA_set_fs fail\n");
			MessageBox(NULL, _T("fha_set_fs fail!"), NULL,MB_OK);
			return;
		}
	}
	

	//下载boot
	if (!FHA_download_boot(SpiInfo.clock))//
	{
		printf("FHA_download_boot fail\n");
		MessageBox(NULL, _T("creat spiimage download boot fail!"), NULL,MB_OK);
		return;
	}


	FHA_close();  //写bin文件信息
	
	CloseHandle(Spi_hHandle);//关闭handle
	AfxMessageBox(_T("完成SPI镜像制作"));//提示完成
	return;
}
