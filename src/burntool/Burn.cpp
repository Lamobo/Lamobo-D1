/***
*Burn.cpp - This is the cpp file for Burn.
*
*       Copyright (c) 2000-2007, Anyka (GuangZhou) Software Technology Co., Ltd. All rights reserved.
*
*Purpose:
*       define the CBurnBase class and Subclass.
*
* @author	Anyka
* @date		2007-10
* @version	2.0
*
*******************************************************************************/

#include "StdAfx.h"
#include "BurnTool.h"
#include "Burn.h"
#include "Config.h"
#include "MainFrm.h"
#include "logFile.h"
#include "AKFS.h"
#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>


extern "C"
{
#include "fsa.h"
//#include "fha_test.h"
#include "fha.h"
}

extern UINT m_worknum;   //u盘烧录时动行线程的个数
extern UINT g_workTotalnum; //按键烧录时获取动行线程的个数
extern UINT g_workThreadnum;//
extern CConfig theConfig;   //config
extern CBurnToolApp theApp;  //mainfrm

extern UINT g_download_nand_count; // 下载nand闪存的文件个数
extern T_DOWNLOAD_NAND g_download_nand[];//nand闪存的结构体

extern UINT g_download_udisk_count;// 下载u盘的文件个数
extern T_DOWNLOAD_UDISK g_download_udisk[];

extern UINT g_download_mtd_count;// 下载镜像的文件个数
extern T_DOWNLOAD_MTD g_download_mtd[];
extern UINT g_timer_counter;    //延时
extern BOOL g_bEraseMode;   //擦模式
//UINT g_udisk_burn_index = 0;
//BOOL g_udisk_flag = FALSE;

//extern HANDLE image_event[MAX_DEVICE_NUM];
extern HANDLE image_event;    //创建镜像事件
extern HANDLE udiskburn_event;   //u盘烧录的事件
extern HANDLE capacity_event;   //检测容量大小的事件
extern HANDLE ResetDevice_event;  //重启事件
//extern UINT g_img_stat[MAX_DEVICE_NUM];
extern UINT g_img_stat;    //镜像的状态
extern UINT g_disk_count;    //分区的个数
extern BOOL m_budisk_burn;   //控制动烧录和U盘烧录
extern BOOL  USB_attachflag; //当开始连接usb时，那么此时就不再进行查找usb设备，直到烧录完成
//extern T_DISK_INFO g_disk_info[16];
extern T_nID_DISK_INFO g_nID_disk_info[MAX_DEVICE_NUM];  //磁盘信息
extern char g_download_mtd_flag[MAX_DEVICE_NUM];
extern CHAR burn_detel_usb_flag[MAX_DEVICE_NUM]; //
UCHAR *m_pBuf_BadBlk[MAX_DEVICE_NUM] = {0};  //坏块
UCHAR *m_pBuf_freeBlk[MAX_DEVICE_NUM] = {0}; //空闲块
//UINT badbufnum = 0;
extern volatile UINT g_udisk_burnnum;  //插入usb设备的个数
extern BOOL  m_budisk_getUSBnum; //只有当是U盘烧录方式下才是ture
extern BOOL g_bUploadbinMode;    //bin回读标志
extern g_bUpload_spialldata;
BOOL g_capacity_flag = AK_TRUE;    //nand 和 sd卡的容量是否相等
UINT g_capacity_size[MAX_DEVICE_NUM] = {0};  //各nand 和 sd卡的容量
UINT g_capacity_burnnum = 0;
extern HANDLE g_handle;   //信号量

#define ERASE_NAND_MODE 1

#define OOB_READTURE 1    //oob 读成功
#define OOB_READFAIL 0    //oob 读失败   

#define FHA_SUCCESS 1
#define FHA_FAIL    0

#define  passY               _T("Y")    //mac和序列号成功的标致
#define  passN               _T("N")    //mac和序列号成功的标致
#define  COMMODE_LEN         20

#define BLOCK_PAGE0_FLAG_INVALID 0x00   //不可用
#define BLOCK_PAGE0_FLAG_VALID   0xFF   //空闲
#define MTD_OOB_LEN              8      //oob的长度
#define USB_ONE_BUF_SIZE    8192        //usb的buf长度
//#define DELAY_UDSIK_BURN_TIMER    150

#define IMG_BUF_SIZE_NAND    (128 * 1024)   //下载镜像时nand的大小
#define IMG_BUF_SIZE_SD    (64 * 1024)      //下载镜像的sd大小

//oob的数据，用于在oob无效时使用
static m_mtd_oob_invalid[MTD_OOB_LEN]  = {BLOCK_PAGE0_FLAG_INVALID};

//镜像文件查找时使用
static PTCHAR m_ImgTable[] =
{
    _T("512"),
    _T("1k"),
    _T("2k"),
    _T("4k"),
    _T("8k"),
};

typedef struct
{
	T_PMEDIUM medium;
	T_U32 ID;
}T_PC_MEDIUM;

//sd卡的结构体
typedef struct
{
    UINT total_block;
    UINT block_size;
}T_EMMC_INFO;

//如下是函数的声明

//获取mac地址到buf
BOOL Get_Mac_Addr(TCHAR *buf, UINT channelID);
//获取序列号到buf
BOOL Get_serial_Addr(TCHAR *buf, UINT channelID);
//写mac和序列号到文件上
BOOL write_config(LPCTSTR file_path, TCHAR *pBufHigh, TCHAR *pBufLow);
//烧录失败后，记录mac和序列号
void Burn_Fail(TCHAR *serialtempbuf, TCHAR *mactempbuf, UINT nID, BOOL readSerialflag, BOOL readmacflag);
//判断mac是否可用
BOOL Macaddr_isunuse(char *buf, int len);
//判断序列号是否可用
BOOL Serialaddr_isunuse(char *buf, int len);
//设置分区信息
void SetPartInfo(UINT nID, T_DRIVER_INFO *pPartInfo, UINT num, UINT SectorPerPage, UINT SectorSize, UINT nDISKB_Enlarge);
//记录u盘文件所在的符盘
void Save_udiskfile_drivernum(void);
//重置nand的回调
VOID Burn_ResetNandBase(UINT nID, T_PNANDFLASH nand);
//获取坏块到BUF
T_BOOL fNand_Get_BadBlockBuf(UINT nID, T_U32 BlockNum, T_U32 nChipCnt, T_MODE_CONTROL *ModeCtrl);
//释放所有空闲块
void fNand_Free_BadBlockBuf(UINT nID);
//获取所有空闲块
T_BOOL fNand_Get_FreeBlockBuf(UINT nID, T_U32 chip, T_U32 BlockNum, T_U32 nChipCnt, UINT StartBlock);
//保存出来bin的位置
T_BOOL browser_for_binUpload(TCHAR *folderPath);
//重置sd卡
VOID Burn_ResetMedium(UINT nID, T_PMEDIUM medium);
//释放sd介质的内存
T_VOID Burn_Free_Medium(T_PMEDIUM medium);
//分配sd介质的内存
T_PMEDIUM Burn_Malloc_Medium(UINT ID, UINT secsize, UINT capacity, UINT SecPerPg);
//获取镜像的路径
BOOL GetDownloadImgFromPCPath(PTCHAR pDst, PTCHAR pSrc, USHORT  page_size);
//创建分区
BOOL Burn_CreatePartion(UINT nID, HWND hWnd, PTCHAR file_name, T_PARTION_INFO *partInfo);
//创建spi分区
BOOL Burn_CreatePartion_SpiFlash(UINT nID, HWND hWnd, PTCHAR file_name);
//下载文件到u盘上
BOOL Burn_DownloadFile(UINT nID, HWND hWnd, PTCHAR file_name);
BOOL Burn_Get_spiAlldata(UINT nID, HWND hWnd, PTCHAR file_name, UINT spiflash_len);

//获扩容功能暂时没有使用
UINT GetDISKBSizeEnlarge()
{
	UINT diskB_enlarge = 0;

	/*扩展扩容的函数， 留给后来使用*/

	return diskB_enlarge;
}

//判断是否全0值
static BOOL is_zero_ether_addr(TCHAR *addr)
{
	BOOL flag = FALSE;
	
	//0表示48
	if ((addr[0] == 48 && addr[1] == 48 && addr[3] == 48 
		&& addr[4] == 48 && addr[6] == 48 && addr[7] == 48))
	{
		flag = TRUE;
	}
	
	return flag;
	//return !(addr[0] | addr[1] | addr[3] | addr[4] | addr[6] | addr[7]);
}

//从安全区读取MAC地址
BOOL Burn_GetMACInfo(UINT nID, HWND hWnd, PTCHAR file_name, 
					 PTCHAR maclowbuf, PTCHAR mactempbuf, BOOL *readmacflag, UINT  *macmode)
{
	BOOL macaddrflag = TRUE;
	UINT maclen = MAX_MAC_SEQU_ADDR_COUNT;
	TCHAR tempbuf_mac[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
    TCHAR MacBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	USES_CONVERSION;

	//针对39芯片上的spi烧录，如果升级烧录时，也可以进行升级mac地址和序列号
	//因为spi的mac和序列号是以bin文件方式存放的，这里只更更内容。
	if(theConfig.burn_mode != E_CONFIG_SFLASH)//非spi烧录的升级都不支持mac和序列号
	{
		if(theConfig.bUpdate)
		{
			return TRUE;
		}
	}

	//判断是否烧录MAC地址
	if(theConfig.macaddr_flag) 
	{
		BYTE buf[MAX_MAC_SEQU_ADDR_COUNT*2+1] = {0};
		CLogFile  burnFile(file_name);
		
		//判断是否强制烧录MAC地址
		if (theConfig.fore_write_mac_addr)
		{
			*macmode = ASA_MODE_CREATE;

			//获取mac地址
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_COMPARE));
			macaddrflag = Get_Mac_Addr(maclowbuf,  nID);
		}
		else
		{
			*macmode = ASA_MODE_OPEN;
			//读asa区的mac地址
			//if不是强制写入
			burnFile.WriteLogFile(LOG_LINE_TIME,  "++read asa file++\r\n" );
			if (BT_ReadASAFile(nID, "MACADDR", buf, &maclen)  == BT_SUCCESS)
			{
				*readmacflag = TRUE;
				
				//判断读出来的mac地址是否有效
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_READ));
				if (!Macaddr_isunuse((char *)buf, maclen))
				{
					burnFile.WriteLogFile(0, "read MAC ADDR is error from asafile!\r\n");
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_READ_ERROR));
					return FALSE;
				}

				memset(tempbuf_mac, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
				theConfig.lower_to_upper(A2T((char *)buf), tempbuf_mac);
				_tcscpy(MacBuf, tempbuf_mac);
				
			}
			else
			{
				*readmacflag = FALSE;
				//重新获取mac地址
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_COMPARE));
				macaddrflag = Get_Mac_Addr(maclowbuf,  nID);
			}
		}

		if (!macaddrflag)
		{
			//判断mac地址是否大于最大值
			burnFile.WriteLogFile(0, "MAC ADDR beyond the most addr!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_FAIL));
			return FALSE;
		}

		//把获取到的地址放到一个临时的buf里
		if (*readmacflag)
		{
			_tcscpy(mactempbuf, MacBuf);
		} 
		else
		{
			_tcscpy(mactempbuf, theConfig.mac_current_high);
			_tcscat(mactempbuf, _T(":"));
			_tcscat(mactempbuf, maclowbuf);
			//memcpy(mactempbuf, buf, MAX_MAC_SEQU_ADDR_COUNT);
		}
		_tcscpy(maclowbuf, &mactempbuf[9]); //提示低8位的值

		//显示当前通道的mac地址
		_tcscpy(theConfig.g_mac_show_current_low[nID-1], maclowbuf);
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_SHOW));
		
	}

	return TRUE;
}
//从安全区读取序列号
BOOL Burn_GetSERIALInfo(UINT nID, HWND hWnd, PTCHAR file_name, 
					 PTCHAR seriallowbuf, PTCHAR serialtempbuf, BOOL *readSerialflag, UINT  *Serialmode)
{
	BOOL serialaddrflag = TRUE;
	UINT Seriallen = MAX_MAC_SEQU_ADDR_COUNT;
	TCHAR tempbuf_serial[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
    TCHAR SerialBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	USES_CONVERSION;

	//针对39芯片上的spi烧录，如果升级烧录时，也可以进行升级mac地址和序列号
	//因为spi的mac和序列号是以bin文件方式存放的，这里只更更内容。
	if(theConfig.burn_mode != E_CONFIG_SFLASH)//非spi烧录的升级都不支持mac和序列号
	{
		if(theConfig.bUpdate)
		{
			return TRUE;
		}
	}

	if(theConfig.sequenceaddr_flag) 
	{
		BYTE buf[MAX_MAC_SEQU_ADDR_COUNT*2+1] = {0};
		CLogFile  burnFile(file_name);

		//判断是否强制烧录序列号
		if (theConfig.fore_write_serial_addr)
		{
			*Serialmode = TRUE;
			//获取序列号地址
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_COMPARE));
			serialaddrflag = Get_serial_Addr(seriallowbuf,  nID);
		}
		else
		{
			*Serialmode = FALSE;
			//if不是强制写入
			burnFile.WriteLogFile(LOG_LINE_TIME,  "++read asa file++\r\n" );
			if (BT_ReadASAFile(nID, "SERADDR", buf, &Seriallen)  == BT_SUCCESS)
			{
				*readSerialflag = TRUE;

				//判断读出来的序列号地址是否有效
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_READ));
				if (!Serialaddr_isunuse((char *)buf, Seriallen))
				{
					burnFile.WriteLogFile(0, "read serial ADDR is error from asafile!\r\n");
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_READ_ERROR));
					return FALSE;
				}

				memset(tempbuf_serial, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
				theConfig.lower_to_upper(A2T((char *)buf), tempbuf_serial);
				_tcscpy(SerialBuf, tempbuf_serial);
				
				
			}
			else
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_COMPARE));
				*readSerialflag = FALSE;	
				//获取序列号地址
				serialaddrflag = Get_serial_Addr(seriallowbuf,  nID);
			}
		}
		
		
		if (!serialaddrflag)
		{	
			//判断序列号的值是否大于最大值
			burnFile.WriteLogFile(0, "serial ADDR beyond the most addr!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_FAIL));
			return FALSE;
		}
		
		//如果读成功了，那么就用记录读出来的序列号
		if (*readSerialflag)
		{
			_tcscpy(serialtempbuf, SerialBuf);
		}
		else
		{
			//否则记录当前的序列号
			_tcscpy(serialtempbuf, theConfig.sequence_current_high);
			_tcscat(serialtempbuf, seriallowbuf);
		}
		_tcscpy(seriallowbuf, &serialtempbuf[10]); //提示低6位的值

		//显示当前通道的序列号地址
		_tcscpy(theConfig.g_sequence_show_current_low[nID-1], seriallowbuf);
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_SHOW));
	}
	
	return TRUE;
}

//写mac地址到安全区
BOOL Burn_WriteMACInfo(UINT nID, HWND hWnd, PTCHAR file_name, PTCHAR maclowbuf, PTCHAR mactempbuf, 
					   BOOL readmacflag, UINT  macmode)
{
	USES_CONVERSION;
	/***********************************************/

	//针对39芯片上的spi烧录，如果升级烧录时，也可以进行升级mac地址和序列号
	//因为spi的mac和序列号是以bin文件方式存放的，这里只更更内容。
	if(theConfig.burn_mode != E_CONFIG_SFLASH)//非spi烧录的升级都不支持mac和序列号
	{
		if(theConfig.bUpdate)
		{
			return TRUE;
		}
	}

	//判断是否烧录MAC地址
	if(theConfig.macaddr_flag) 
	{
		TCHAR buf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
		BYTE tbuf[MAX_MAC_SEQU_ADDR_COUNT*2+1];
		UINT *ptr = (UINT *)tbuf; 
		CLogFile  burnFile(file_name);
		
		_tcscpy(buf, mactempbuf);
		ptr[0] = wcslen(buf);
		memcpy(&tbuf[4], T2A(buf), 56);
		
		
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_WRITE_MAC_ADDR_ASA_START));
		burnFile.WriteLogFile(LOG_LINE_TIME, "++write mac add into asa file++\r\n");
		
		//写mac地址到安全区内
		if (BT_WriteASAFile(nID, "MACADDR", tbuf, 30, macmode) != BT_SUCCESS)
		{
			//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_WRITE_MAC_ADDR_ASA_FAIL));
			return FALSE;
		}
		//如果是读失败的话，就把当前的MAC地址写到文档，否则就不写
		if (!readmacflag)
		{
			theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, passY, maclowbuf, nID-1);
		}
	}
	
	return TRUE;
}
//写序列号到安全区
BOOL Burn_WriteSERIALInfo(UINT nID, HWND hWnd, PTCHAR file_name, PTCHAR seriallowbuf, PTCHAR serialtempbuf, 
						  BOOL readSerialflag, UINT  Serialmode)
{
	USES_CONVERSION;

	//针对39芯片上的spi烧录，如果升级烧录时，也可以进行升级mac地址和序列号
	//因为spi的mac和序列号是以bin文件方式存放的，这里只更更内容。
	if(theConfig.burn_mode != E_CONFIG_SFLASH)//非spi烧录的升级都不支持mac和序列号
	{
		if(theConfig.bUpdate)
		{
			return TRUE;
		}
	}

	if(theConfig.sequenceaddr_flag) 
	{
		TCHAR buf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
		BYTE tbuf[MAX_MAC_SEQU_ADDR_COUNT*2+1];
		UINT *ptr = (UINT *)tbuf; 
		CLogFile  burnFile(file_name);
		
		_tcscpy(buf, serialtempbuf);
		ptr[0] = wcslen(buf);
		memcpy(&tbuf[4], T2A(buf), MAX_MAC_SEQU_ADDR_COUNT*2-4);
		
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_WRITE_SERIAL_ADDR_ASA_START));
		burnFile.WriteLogFile(LOG_LINE_TIME, "++write serial add into asa file++\r\n");
		
		if (BT_WriteASAFile(nID, "SERADDR", tbuf, 30, Serialmode) != BT_SUCCESS)
		{
			//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_WRITE_SERIAL_ADDR_ASA_FAIL));
			return FALSE;
		}
		
		//如果是读失败的话，就把当前的序列号写到文档，否则就不写
		if (!readSerialflag)
		{
			theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, passY, seriallowbuf, nID-1);
		}
		//MessageBox(NULL, _T("2!"), NULL,MB_OK);
	}
	return TRUE;
}

BOOL Burn_upload_spiflash_Data(UINT nID, HWND hWnd, PTCHAR file_name, T_SFLASH_PHY_INFO *sFlashPhyInfo)
{
	BOOL ret = TRUE;

	//回读整一个spi的数据
	if (g_bUpload_spialldata)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_UPLOAD_SPIFLASH_START));
		if (BT_SetSPIParam(nID, sFlashPhyInfo) == BT_FAIL)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_CHIP_PARA_FAIL));
			return FALSE;
		}

		if (!Burn_Get_spiAlldata(nID, hWnd, file_name,  sFlashPhyInfo->total_size))
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_UPLAOD_SPIFLASH_FAIL));
			ret = FALSE;
		}
		else
		{
			ret = TRUE;
		}
		//关闭
		if (BT_Close(nID) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_FAIL));
			return FALSE;
		}
		
	}
	
	return ret;

}

//spi的烧录
BOOL burn_spiflash(UINT nID, HWND hWnd, T_MODE_CONTROL ModeCtrl,  PTCHAR file_name)
{
	T_DOWNLOAD_BIN download_bin;
	UINT i;
	BOOL readmacflag_spi = FALSE;//写标志
	TCHAR mactempbuf_spi[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};//记录临时
	TCHAR maclowbuf_spi[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};//记录低位
	UINT  macmode_spi = 0;//是创建还是打开
	BOOL readSerialflag_spi = FALSE;//写标志
	TCHAR serialtempbuf_spi[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};//记录临时
	TCHAR seriallowbuf_spi[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};//记录低位
	UINT  Serialmode_spi = 0;//是创建还是打开
	BOOL ret = TRUE;

	USES_CONVERSION;

	UINT sflashID, sflashChipCnt;
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_CHIP_PARA));
	//通过produce获取相应的spiflash
	if (BT_GetFlashID(nID, 254, 254, &sflashID, &sflashChipCnt) != BT_SUCCESS)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_CHIP_PARA_FAIL));
		return FALSE;
	}

	//通过上面获到的spiflash进行在烧录工具那里获取相应的spiflash
	T_SFLASH_PHY_INFO_TRANSC sFlashPhyInfo;
	for(i = 0; i < theConfig.spiflash_parameter_count; i++)
	{
		//spi vailed in (bit24-0)
		if((sflashID&0xffffff) == (theConfig.spiflash_parameter[i].chip_id&0xffffff))
		{
			memcpy(&sFlashPhyInfo, &theConfig.spiflash_parameter[i], sizeof(T_SFLASH_PHY_INFO_TRANSC));
			break;
		}
	}
	 //获取spiflash
	//并设置一下
	if (i == theConfig.spiflash_parameter_count || BT_SetSPIParam(nID, &sFlashPhyInfo) == BT_FAIL)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_CHIP_PARA_FAIL));
		return FALSE;
	}

	//当此时是bin回读时，那么就直接返回
	if (g_bUploadbinMode)
	{
		return TRUE;
	}


	//判断是否烧录MAC地址
    if (!Burn_GetMACInfo(nID, hWnd, file_name, maclowbuf_spi, mactempbuf_spi, &readmacflag_spi, &macmode_spi))
	{
		return FALSE;
	}
	
	//写序列号
	if (!Burn_GetSERIALInfo(nID, hWnd, file_name, seriallowbuf_spi, serialtempbuf_spi, &readSerialflag_spi, &Serialmode_spi))
	{
		Burn_Fail(seriallowbuf_spi, maclowbuf_spi, nID, readSerialflag_spi, readmacflag_spi);
		return FALSE;
	}

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_START));
	for(i = 0; i < g_download_nand_count; i++)
	{
		download_bin.bCompare = g_download_nand[i].bCompare;
		_tcscpy(download_bin.pc_path , theApp.ConvertAbsolutePath(g_download_nand[i].pc_path));
		download_bin.ld_addr = g_download_nand[i].ld_addr;
		memcpy(download_bin.file_name, T2A(g_download_nand[i].file_name), MAX_PATH);
		
		download_bin.bBackup = g_download_nand[i].bBackup;
		download_bin.bUpdateSelf = theConfig.bUpdateself;

		//设置每一个bin文件后的扩展大小
		if (g_download_nand[i].bin_revs_size != 0)
		{
			UINT bin_len = 0;

			if (theConfig.planform_tpye == E_LINUX_PLANFORM)
			{
				bin_len = (UINT)(g_download_nand[i].bin_revs_size*1024*1024 + 1023)/1024;//以K为单位对齐
			}
			else
			{
				bin_len = (UINT)g_download_nand[i].bin_revs_size;
			}

			if (BT_Set_BinResvSize(nID, bin_len) != BT_SUCCESS)
			{
				Burn_Fail(seriallowbuf_spi, maclowbuf_spi, nID, readSerialflag_spi, readmacflag_spi);
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_RESV_AREA_FAIL));
				return FALSE;
			}
		}

		
		if (BT_DownloadBin(nID, &download_bin) != BT_SUCCESS)
		{
			Burn_Fail(seriallowbuf_spi, maclowbuf_spi, nID, readSerialflag_spi, readmacflag_spi);
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_FAIL));
			return FALSE;
		}
	}

	//记录已写的MAC地址
	if (!Burn_WriteMACInfo(nID, hWnd, file_name, maclowbuf_spi, mactempbuf_spi, readmacflag_spi, macmode_spi))
	{
		Burn_Fail(seriallowbuf_spi, maclowbuf_spi, nID, readSerialflag_spi, readmacflag_spi);
		return FALSE;
	}
	
	//记录序列号
	if (!Burn_WriteSERIALInfo(nID, hWnd, file_name, seriallowbuf_spi, serialtempbuf_spi, readSerialflag_spi, Serialmode_spi))
	{
		Burn_Fail(seriallowbuf_spi, maclowbuf_spi, nID, readSerialflag_spi, readmacflag_spi);
		return FALSE;
	}


	//分区和下载镜像只支持linux平台
	if ((theConfig.planform_tpye == E_LINUX_PLANFORM) && (theConfig.format_count != 0))
	{
		UINT pagesize = sFlashPhyInfo.page_size;
		T_DOWNLOAD_IMG download_img;

		//对spi进行分区
		if (!Burn_CreatePartion_SpiFlash(nID, hWnd, file_name))
		{
			return FALSE;
		}

		//下载镜像文件		
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_START));
		for(i = 0; i < theConfig.download_mtd_count; i++)
		{
			download_img.bCompare = theConfig.download_mtd_data[i].bCompare;
			_tcscpy(download_img.pc_path, theApp.ConvertAbsolutePath(theConfig.download_mtd_data[i].pc_path));
	
			download_img.driver_name = (char)theConfig.download_mtd_data[i].disk_name[0];
			//linux平台不需要区分什么时下载镜像，镜像文件由linux那边控制
			//if (ModeCtrl.burn_mode != MODE_UPDATE)// || (partInfo[driverNo].bOpenZone == FALSE))
			{
				if (BT_DownloadImg(nID, &download_img) != BT_SUCCESS)
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
			}
		}

		if (!Burn_DownloadFile(nID, hWnd, file_name))
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
			return FALSE;
		}
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_START));
		//当是升级模式，并下载bin区文件的个数是0,就不下载boot
		if (ModeCtrl.burn_mode != MODE_UPDATE && theConfig.download_nand_count != 0)
		{
			if (CHIP_37XX_L == theConfig.chip_type)
			{
				if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot_new), theConfig.chip_type) != BT_SUCCESS)
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
					return FALSE;
				}
			}
			else
			{
				if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot), theConfig.chip_type) != BT_SUCCESS)
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
					return FALSE;
				}
			}
		}
		
		//关闭
		if (BT_Close(nID) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_FAIL));
			return FALSE;
		}

		//spi镜像制作
		if (!Burn_upload_spiflash_Data(nID,  hWnd,  file_name, &sFlashPhyInfo))
		{
			return FALSE;
		}

		//烧录完成
		return TRUE;
	}

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_START));

	if (CHIP_37XX_L == theConfig.chip_type)
	{
		if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot_new), theConfig.chip_type) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
			return FALSE;
		}
	}
	else
	{
		if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot), theConfig.chip_type) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
			return FALSE;
		}
	}

	if (BT_Close(nID) != BT_SUCCESS)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_FAIL));
		return FALSE;
	}

	//spi镜像制作
	if (!Burn_upload_spiflash_Data(nID,  hWnd,  file_name, &sFlashPhyInfo))
	{
		return FALSE;
	}

	return TRUE;

}

//把大写转换成小写
void MakeLower(LPTSTR str)
{
	TCHAR *pStr;
	
	if(!str || !str[0])
	{
		return;
	}

	pStr = str;
	while(*pStr++)
	{
		if(*pStr >= 'A' && *pStr <= 'Z')
		{
			*pStr += 'a' - 'A';
		}
	}
}

/*
static BOOL CheckImgFileName(PTCHAR str)
{
    int i, j;

    j = sizeof(m_ImgTable) / sizeof(PTCHAR);
    
    for (i=0; i<j; i++)
    {
        if (_tcsstr(str, m_ImgTable[i]) != NULL)
        {
            return FALSE;
        }
    }
    
    return TRUE;
}
*/

//在文件夹中查找镜像文件
BOOL FindImgFileInDir(LPTSTR pathImg, LPTSTR pathPC, int ImgID)
{
	WIN32_FIND_DATA fd;
	HANDLE hSearch;
	TCHAR searchPath[MAX_PATH+1];
    TCHAR tmpPCPath[MAX_PATH+1];
    int tabID = sizeof(m_ImgTable) / sizeof(PTCHAR);

    if (pathImg == NULL || pathPC == NULL || ImgID >= tabID)
    {
        return FALSE;
    }
    
    _tcsncpy(searchPath, pathPC, MAX_PATH);
	_tcscat(searchPath, _T("\\*"));

	hSearch = FindFirstFile(searchPath, &fd);
	if(INVALID_HANDLE_VALUE == hSearch)
	{
		return FALSE;
	}
	
	USES_CONVERSION;
	
	do
	{
		if((0 != _tcscmp(fd.cFileName, _T("."))) && (0 != _tcscmp(fd.cFileName, _T(".."))))
		{
			if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
                _tcscpy(tmpPCPath, fd.cFileName);
                MakeLower(tmpPCPath);//大写转小写
                if (_tcsstr(tmpPCPath, m_ImgTable[ImgID]) != NULL)
                {
			        _tcscpy(pathImg, pathPC);
                    _tcscat(pathImg, _T("\\"));
			        _tcscat(pathImg, fd.cFileName);
                    FindClose(hSearch);

                    return TRUE;
                }
			}
		}
	}
	while(FindNextFile(hSearch, &fd));

	FindClose(hSearch);
    
	return FALSE;	
}

//通过pc路径下载镜像文件
BOOL GetDownloadImgFromPCPath(PTCHAR pDst, PTCHAR pSrc, USHORT  page_size)
{
    DWORD dwAttr;
    USHORT SecSize = page_size;
    int SecBit = 0;

    if (page_size % 512)
        return FALSE;

    SecSize >>= 9;

	//算扇区的位数
    while (SecSize > 1)
    {
        SecBit++;
        SecSize >>= 1;
    }
    
	//获取源文件的属性
	dwAttr = GetFileAttributes(pSrc);
	if(0xFFFFFFFF == dwAttr)
	{
		return FALSE;
	}    

	if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    {
        //需要搜索文件夹内部的相应img
        if (!FindImgFileInDir(pDst , pSrc, SecBit))
            return FALSE;
    }
    else
    {
		//非文件夹
		_tcscpy(pDst , pSrc);
    }

    return TRUE;
}

//下载comAddr到内存
BOOL Download_comAddr_ToRam(UINT nID, HWND hWnd, PTCHAR file_name, UINT com)
{
	//UINT com = 0;
	UINT cmddata = 0x30700000;
	
	CLogFile  burnFile(file_name);

	if (BT_DownloadDataToRam(nID, cmddata,(PBYTE)&com, 4) != BT_SUCCESS)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "->BT_DownloadDataToRam fail.\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_REGISTER_FAIL));
		return FALSE;
	}
	return TRUE;
}

//对37，98需在下载一些值到内存下
BOOL Download_Channel_Addranddata_ToRam(UINT nID, HWND hWnd, TCHAR *file_name)
{
	UINT channel_addr = 0x80100000; // 初始化
	UINT cmddata      = 0x81a00000; // 初始化

	CLogFile  burnFile(file_name);
	
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_SET_CHANNELID));
	
	//ak98芯片
	if(theConfig.chip_type == CHIP_980X)
	{
		burnFile.WriteLogFile(0,  "channel ID=0x80100000\n");
		channel_addr = 0x80100000;
		
	}
	else if(theConfig.chip_type == CHIP_37XX || theConfig.chip_type == CHIP_37XX_L)//ak37芯片
	{
		burnFile.WriteLogFile(0,  "channel ID=0x30100000\\n");
		channel_addr = 0x30100000;
	}
	else if(theConfig.chip_type == CHIP_39XX)//ak39芯片
	{
		burnFile.WriteLogFile(0,  "channel ID=0x80100000\\n");
		channel_addr = 0x80100000;
	}
	else
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "->chip_type  error.\r\n");
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_CHANNELID_FAIL));
		return FALSE;
	}
	
	//下载地址
	if (BT_DownloadDataToRam(nID, channel_addr, NULL, 0) != BT_SUCCESS)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME,  "->SetRamParam fail.\r\n");
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_REGISTER_FAIL));
		return FALSE;
	}
	
	if (theConfig.cmdLine.bCmdLine)
	{
		burnFile.WriteLogFile(0, "data{%s}\n", theConfig.cmdLine.CmdData );
		if(theConfig.chip_type == CHIP_980X)//ak98芯片
		{
			burnFile.WriteLogFile(0,  "cmddata =0x81a00100\n");
			cmddata = 0x81a00100;
			
		}
		else if(theConfig.chip_type == CHIP_37XX || theConfig.chip_type == CHIP_37XX_L)//ak37芯片
		{
			burnFile.WriteLogFile(0,  "cmddata =0x30400100\\n");
			cmddata = 0x30400100;
		}
		else if(theConfig.chip_type == CHIP_39XX)//ak39芯片
		{
			burnFile.WriteLogFile(0,  "cmddata =0x80100100\\n");
			cmddata = 0x80100100;
		}
		else
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->chip_type error1.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_CHANNELID_FAIL));
			return FALSE;
		}
		//下载值
		if (BT_DownloadDataToRam(nID, cmddata,(PBYTE)theConfig.cmdLine.CmdData, 128) != BT_SUCCESS)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->SetRamParam fail.\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_REGISTER_FAIL));
			return FALSE;
		}
	}
	return TRUE;
}

//读取chip文件中的串口类型
UINT Read_commode_fromfile(void)
{
	TCHAR commode[COMMODE_LEN] = {0};
	TCHAR tempfilename[COMMODE_LEN] = {0};
	TCHAR filename[COMMODE_LEN] = {0};
	UINT i = 0;
	UINT com_mode = 0;
	//T_DOWNLOAD_BIN download_bin;
	TCHAR pc_path[MAX_PATH+1] = {0};

	USES_CONVERSION;

	for(i = 0; i < g_download_nand_count; i++)
	{

		//如果是小写的,那么转换为大写
        _tcsncpy(tempfilename, g_download_nand[i].file_name, 5);
		theConfig.lower_to_upper(tempfilename, filename);


		if (_tcscmp(A2T("CHIP"), filename) == 0)
		{
			_tcscpy(pc_path , theApp.ConvertAbsolutePath(g_download_nand[i].pc_path));
		}
	}
	
	GetPrivateProfileString(_T("chip"), _T("uart_ID"), NULL, commode, COMMODE_LEN, pc_path);	

	com_mode = atoi(T2A(commode));//把字符型转换成整型

	return com_mode;
}
//下载变频小程序
BOOL Burn_DownloadChangClock(UINT nID, HWND hWnd, PTCHAR file_name)
{
	if (CHIP_980X == theConfig.chip_type) 
	{
		CLogFile  burnFile(file_name);

		burnFile.WriteLogFile(0, "SUPPORT_LINUX\r\n");
		if (!theConfig.bUDiskUpdate)
		{
			burnFile.WriteLogFile(0,  "Not Udiskburn!\n" );
			burnFile.WriteLogFile(LOG_LINE_TIME, "++Begin donwload change_clk++\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_CHANGE_CLK_START));
			if (BT_DownloadProducer(nID, theApp.ConvertAbsolutePath(_T("CHANG_CLK.bin")), 
				0x48000000) != BT_SUCCESS)
			{
				burnFile.WriteLogFile(0, "->donwload change_clk fail!\r\n");
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_CHANGE_CLK_FAIL));
				return FALSE;
			} 
			/*第一次下载变频小程序后需要重新断开连接usb*/
	//		if (BT_WaitUSBAttach(nID) == BT_FAIL)
	//		{
	//			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_CHANGE_CLK_USB_FAIL));
	//            return FALSE;
	//		}
		}
		else
		{
			burnFile.WriteLogFile(0, "Is Udiskburn!\r\n" );
			burnFile.WriteLogFile(LOG_LINE_TIME, "++USB Attach++\r\n" );
		}
    
		g_workThreadnum++;
		burnFile.WriteLogFile(0, "Work Thread num = %d\n", g_workThreadnum);

		/*等待所有的线程*/
		burnFile.WriteLogFile(LOG_LINE_TIME, "...waiting...\r\n" );
		g_timer_counter = 0;
		while ((g_workThreadnum < g_workTotalnum) && (g_timer_counter <= 60))
		{
			Sleep(10);
		}

		burnFile.WriteLogFile(LOG_LINE_TIME, "counter:%d\n",g_timer_counter);

		//如果时间超过1分钟，那么就出错
		if (g_timer_counter > 60)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "time out!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_CHANGECLK_TIMEOUT_FAIL));
			return FALSE;
		}
		//此处sleep的作用是因为下变频后，需要一个等待usb连接的时间
		Sleep(500);
	}
	else
	{
		g_workThreadnum++;
	}
	
	return TRUE;
}
//设置内存参数
BOOL Burn_SetRamParam(UINT nID, HWND hWnd, PTCHAR file_name)
{
    T_RAM_REG ram_reg[64];
    UINT  ram_cnt;
	UINT i;
    
	//对于10/11芯片的，都不需要设置内存参数
	if ((CHIP_11XX != theConfig.chip_type) && (CHIP_10X6 != theConfig.chip_type) 
		&& (CHIP_1080A != theConfig.chip_type) && (CHIP_1080L != theConfig.chip_type)
		 && (CHIP_10XXC != theConfig.chip_type))
	{
		CLogFile  burnFile(file_name);
		burnFile.WriteLogFile(LOG_LINE_TIME, "++SetRamParam++\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_SET_RAMPARAM));

		//通过内存参数进行读取内存地址和值
		ram_cnt = config_ram_param(ram_reg);
		burnFile.WriteLogFile(0, "ram_count = %d\n", ram_cnt);
		for (i=0; i<ram_cnt; i++)
		{
			burnFile.WriteLogFile(0, "ram[%02d]: addr=%08x, value=%08x\n", i, ram_reg[i].reg_addr, ram_reg[i].reg_value);

		}
        //把读到的内存值和地址传下去，以写到boot区内		
		if (BT_SetRamParam(nID, ram_reg, ram_cnt) != BT_SUCCESS)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->SetRamParam fail.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_RAMPARAM_FAIL));
			return FALSE;
		}	
	}
	return TRUE;
}
//在rtos平台下，需要写入串口类型
BOOL Burn_RTOSDownComInfo(UINT nID, HWND hWnd, PTCHAR file_name)
{
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_37XX == theConfig.chip_type)  || (CHIP_37XX_L == theConfig.chip_type)))
	{
		CLogFile  burnFile(file_name);

		//从文件中读取出theConfig.com_mode
		burnFile.WriteLogFile(LOG_LINE_TIME,  "++read com mode++\r\n" );

		//读取com
		theConfig.com_mode = Read_commode_fromfile();
		if (theConfig.com_mode != 0 && theConfig.com_mode != 1 )
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->Download_comAddr_ToRam fail.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_COMMODE_FAIL));
			return FALSE;
		}
		
		//把COM类型写到ram中
		if (FALSE == Download_comAddr_ToRam( nID,  hWnd,  file_name,  theConfig.com_mode))
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->Download_comAddr_ToRam fail.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_COMADDR_FAIL));
			return FALSE;
		}
	}	

	return TRUE;
}
//在linux平台下需要等待produce的下载完
BOOL Burn_WaitProducerInitFinish(UINT nID, HWND hWnd, PTCHAR file_name)
{
    //#ifdef SUPPORT_LINUX
	if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	{
		CLogFile  burnFile(file_name);
		/*下载producer后需要重新断开连接usb*/
		burnFile.WriteLogFile(LOG_LINE_TIME, "++Attach USB++\r\n" );
		if (BT_WaitUSBAttach(nID) == BT_FAIL)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME,  "->Attach USB fail.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_PRODUCER_USB_FAIL));
			return FALSE;
		}
		
		g_workThreadnum++;
		/*等待所有的线程*/
		g_timer_counter = 0;
		while ((g_workThreadnum < (g_workTotalnum *2)) && (g_timer_counter <= 60)) 
		{
			Sleep(10);
		}
		//超60就出错划
		if (g_timer_counter > 60)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME,  "time out.\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_PRODUCER_TIMEOUT_FAIL));
			return FALSE;
		}
	}

	Sleep(1500);	

	return TRUE;
}

//由usb1.1转换为usb2.0
BOOL Burn_SwitchUSB11To20(UINT nID, HWND hWnd, PTCHAR file_name)
{
	//由于1080L芯片本身就是2.0，所以不需要进行转换到2.0上
	//但是如果想变频操作话,10L和10C也需要进行usb切换和变频
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && (!g_bEraseMode))
	{
		if (((theConfig.bUsb2) && (CHIP_1080L != theConfig.chip_type) && (CHIP_10XXC != theConfig.chip_type) && (CHIP_37XX_L != theConfig.chip_type))
			|| ((theConfig.bPLL_Frep_change) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type) || (CHIP_37XX_L == theConfig.chip_type))))
		{
			CLogFile  burnFile(file_name);

			burnFile.WriteLogFile(LOG_LINE_TIME,  "++Switch USB to high speed, bUsb2: %d, g_bEraseMode: %d++\r\n", theConfig.bUsb2, g_bEraseMode);
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_INIT_USB));
			//此接口不需要再进行等待一个一个完成，可能同时执行
			if (BT_SwitchUSBHighSpeedAsyn(nID) != BT_SUCCESS)
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "->Switch USB to high speed fail.\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_INIT_USB_FAIL));
				return FALSE;
			}
		}
	}

	return TRUE;
}

//下载producerg ,进行对usb的重新连接的测试
BOOL Burn_TestPCAndProducerConnectOK(UINT nID, HWND hWnd, PTCHAR file_name)
{
	CLogFile  burnFile(file_name);
	burnFile.WriteLogFile(LOG_LINE_TIME, "++test connetion++\r\n" );
    PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_TEST_TRANSC));
    //发送B和T，看produce是否可以收到
	if (BT_TestConnection(nID) != BT_SUCCESS)
    {
		burnFile.WriteLogFile(LOG_LINE_TIME, "->connetion fail!\r\n" );
        PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_TEST_TRANSC_FAIL));
        return FALSE;
    }

	return TRUE;
}
//设置gpio
BOOL Burn_SetGPIO(UINT nID, HWND hWnd, PTCHAR file_name)
{
    if (theConfig.gpio_pin_select)
    {
        T_GPIO_PARAM GpioParam = {0};
		CLogFile  burnFile(file_name);

        GpioParam.num = theConfig.gpio_pin;
		

        burnFile.WriteLogFile(0,  "GpioParam.num = %d\n", GpioParam.num);
    	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BASEBAND_GPIO_SETTING));
        burnFile.WriteLogFile(LOG_LINE_TIME,  "++SetGPIO++\r\n");
        //设置相应的gpio
		if (BT_SetGPIO(nID, &GpioParam) == BT_FAIL)
        {
			burnFile.WriteLogFile(LOG_LINE_TIME, "->setGPIO fail!\r\n" );
	    	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BASEBAND_GPIO_SETTING_FAIL));
            return FALSE;
        }
    }

	return TRUE;
}

/************************************************************************/
/* 设置部分烧录中的需要控制的动作到producer                             */
/************************************************************************/
BOOL Burn_SetBurnPara(UINT nID, HWND hWnd, PTCHAR file_name)
{
	if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	{
		CLogFile  burnFile(file_name);

		burnFile.WriteLogFile(LOG_LINE_TIME,  "++Set Burned Param++\r\n");
		burnFile.WriteLogFile(0,  "mode: %d\n",theConfig.burned_param.burned_mode);
		if (BT_SetBurnedParam(nID, &theConfig.burned_param) == BT_FAIL)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->Set Burned Param fail\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_PARAM_TO_PRODUCER_FAIL));
			return FALSE;
		}
	}

	return TRUE;
}

//设置擦除模式
BOOL Burn_SetEraseMode(UINT nID, HWND hWnd, PTCHAR file_name, T_MODE_CONTROL ModeCtrl)
{
	UINT erase_mode = ERASE_NAND_MODE;  //设为擦block模式

	CLogFile  burnFile(file_name);
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_ERASEMODE_START));
	burnFile.WriteLogFile(LOG_LINE_TIME, "++set erase nand++\r\n" );
	burnFile.WriteLogFile(0, "erase_mode = %d, eMedium = %d\n", erase_mode, ModeCtrl.eMedium );

	if (BT_SetEraseMode(nID, erase_mode) == BT_FAIL)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "->set erase nand mode fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_ERASEMODE_FAIL));
		return FALSE;
	}
	return TRUE;
}
//设置烧录模式
BOOL Burn_SetBurnMode(UINT nID, HWND hWnd, PTCHAR file_name, T_MODE_CONTROL ModeCtrl)
{
	CLogFile  burnFile(file_name);
    PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_MODE_START));
    burnFile.WriteLogFile(LOG_LINE_TIME, "++set mode++\r\n" );
    burnFile.WriteLogFile(0, "burn_mode = %d, eMedium = %d\n", ModeCtrl.burn_mode, ModeCtrl.eMedium );
    if (BT_SetMode(nID, ModeCtrl) == BT_FAIL)
    {
		burnFile.WriteLogFile(LOG_LINE_TIME, "->set mode fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_MODE_FAIL));
        return FALSE;
    }

	return TRUE;
}
//只支持linux平台
BOOL Burn_SetGPIOToNandChip(UINT nID, HWND hWnd, PTCHAR file_name, T_MODE_CONTROL ModeCtrl)
{
	if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	{
		if (TRANSC_MEDIUM_NAND == ModeCtrl.eMedium || TRANSC_MEDIUM_SPI_NAND == ModeCtrl.eMedium)
		{
			CLogFile  burnFile(file_name);

			burnFile.WriteLogFile(LOG_LINE_TIME, "nandflash chip num=%d, ce2:%d, ce3=%d\n", 
				theConfig.chip_select_ctrl.chip_sel_num,
				theConfig.chip_select_ctrl.gpio_chip_2,
				theConfig.chip_select_ctrl.gpio_chip_3);
			if (BT_FAIL == BT_SetNandGpioCe(nID, &theConfig.chip_select_ctrl))
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_NAND_GPIOCE_FAIL));
				return FALSE;
			}
		}
	}

	return TRUE;
}
//通过nand的ID进行设置NAND的参数
BOOL Burn_GetFlashIDAndSetNandParam(UINT nID, HWND hWnd, PTCHAR file_name, 
					 UINT *sector_per_page, UINT *SectorSize, T_NAND_PHY_INFO *NandPhyInfo, UINT *flash_ChipCnt)
{
	UINT flashID, flashChipCnt;
	UINT HighID;
	UINT i;
	UINT parameter_count = 0;
	UINT chip_id = 0;
	UINT flag = 0;
	UINT cmd_len = 0;

	CLogFile  burnFile(file_name);
	if (E_CONFIG_NAND == theConfig.burn_mode || E_CONFIG_SPI_NAND == theConfig.burn_mode)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_CHIP_PARA));
		burnFile.WriteLogFile(LOG_LINE_TIME, "++BT_GetFlashID++\r\n" );
		if (BT_GetFlashID(nID, theConfig.chip_select_ctrl.gpio_chip_2, theConfig.chip_select_ctrl.gpio_chip_3, &flashID, &flashChipCnt) != BT_SUCCESS)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME,  "->GetFlashID fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_CHIP_PARA_FAIL));
			return FALSE;
		}

		burnFile.WriteLogFile(0,  "FlashID = 0x%x, chip count = %d\n", flashID,flashChipCnt );
		*flash_ChipCnt = flashChipCnt; //(低位是chip_cnt, 高位highID)

		//T_NAND_PHY_INFO NandPhyInfo;
		if (theConfig.burn_mode == E_CONFIG_SPI_NAND)
		{
			parameter_count = theConfig.spi_nandflash_parameter_count;
		}
		else
		{
			parameter_count = theConfig.nandflash_parameter_count;
		}

		for(i = 0; i < parameter_count; i++)
		{
			if (theConfig.burn_mode == E_CONFIG_SPI_NAND)
			{
				chip_id = theConfig.spi_nandflash_parameter[i].chip_id;
			}
			else
			{
				chip_id = theConfig.nandflash_parameter[i].chip_id;
				flag = theConfig.nandflash_parameter[i].flag;
				cmd_len = theConfig.nandflash_parameter[i].cmd_len;
			}

			if(flashID == chip_id)
			{
				if ((theConfig.burn_mode != E_CONFIG_SPI_NAND && flag & (1 << 25)) != 0)
				{
					if (BT_GetFlash_HighID(nID, theConfig.chip_select_ctrl.gpio_chip_2, theConfig.chip_select_ctrl.gpio_chip_3, &HighID) != BT_SUCCESS)
					{
						burnFile.WriteLogFile(LOG_LINE_TIME,  "->BT_GetFlash_HighID fail!\r\n");
						PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_HIGHID_FAIL));
						return FALSE;
					}
					

					//比较highID是否相同
					if((HighID & 0xFFFF) != (cmd_len & 0xFFFF))
					{
						continue;
					}
				}

				if (theConfig.burn_mode == E_CONFIG_SPI_NAND)
				{
					memcpy(NandPhyInfo, &theConfig.spi_nandflash_parameter[i], sizeof(T_NAND_PHY_INFO));
				} 
				else
				{
					memcpy(NandPhyInfo, &theConfig.nandflash_parameter[i], sizeof(T_NAND_PHY_INFO));
				}
				
				if ((theConfig.planform_tpye == E_ROST_PLANFORM) && (nID == 1) && (theConfig.bonline_make_image == TRUE))
				{
					//如果大于4K的页，那么页大小按4K来算
					if(NandPhyInfo->page_size > 4096)
					{
						*SectorSize = 4096;
					}
					else
					{
						//否则按页大小来算
						*SectorSize = NandPhyInfo->page_size;
					}
				}
				else if ((theConfig.planform_tpye == E_ROST_PLANFORM) && (theConfig.chip_type == CHIP_1080L || theConfig.chip_type == CHIP_10XXC))
				{
					//10平台都是按512字节来自
					*SectorSize = 512;
				}
				//每一个页有多少个扇区
				*sector_per_page = NandPhyInfo->page_size / *SectorSize;
				break;
			}

		}
		
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_NAND_PARA_START));

		burnFile.WriteLogFile(LOG_LINE_TIME,   "++set nand para++\r\n");
		burnFile.WriteLogFile(0, "page_size = %d, spare size = %d, data_len = 0x%x, cmd_len = %d\n", 
			NandPhyInfo->page_size, NandPhyInfo->spare_size,
			NandPhyInfo->data_len, NandPhyInfo->cmd_len);

		//获取到的nandID传到produce上
		if (i == parameter_count || BT_SetNandParam(nID, NandPhyInfo) == BT_FAIL)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->set nand para fail\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_NAND_PARA_FAIL));
			return FALSE;
		}
	}


	return TRUE;
}
//设置非文件系统保留区大小
BOOL Burn_SetResvAreaSize(UINT nID, HWND hWnd, PTCHAR file_name)
{
	CLogFile  burnFile(file_name);
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_RESV_AREA_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++set resv area++\r\n" );
    burnFile.WriteLogFile(0,  "nonfs_res_size = %d\n", theConfig.nonfs_res_size);
    //单位以M
    if (BT_SetResvAreaSize(nID, theConfig.nonfs_res_size, TRUE) != BT_SUCCESS)
	{
        //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_RESV_AREA_FAIL));
		return FALSE;
	}

	return TRUE;
}

//下载BIN文件到闪存下
BOOL Burn_DownloadBin(UINT nID, HWND hWnd, PTCHAR file_name)
{
	UINT i;
	T_DOWNLOAD_BIN download_bin;
	CLogFile  burnFile(file_name);
	UINT bin_len = 0;
	
	USES_CONVERSION;
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_START));
    burnFile.WriteLogFile(LOG_LINE_TIME, "++download bin++\r\n" );
    burnFile.WriteLogFile(0,  "download_nand_count = %d\n", g_download_nand_count);
     
    for(i = 0; i < g_download_nand_count; i++)
	{
		download_bin.bCompare = g_download_nand[i].bCompare;  //是否比较
		_tcscpy(download_bin.pc_path , theApp.ConvertAbsolutePath(g_download_nand[i].pc_path));  //pc上的路径
		download_bin.ld_addr = g_download_nand[i].ld_addr; //连接地址
		memcpy(download_bin.file_name, T2A(g_download_nand[i].file_name), MAX_PATH);  //bin文件名
		
		download_bin.bBackup = g_download_nand[i].bBackup; //备份
		download_bin.bUpdateSelf = theConfig.bUpdateself;  //自升级

		if (theConfig.planform_tpye == E_LINUX_PLANFORM)
		{
			bin_len = (UINT)(g_download_nand[i].bin_revs_size*1024*1024 + 1023)/1024;//以K为单位对齐
		}
		else
		{
			bin_len = (UINT)g_download_nand[i].bin_revs_size;
		}

		//设置每一个bin文件后的扩展大小
		if (BT_Set_BinResvSize(nID, bin_len) != BT_SUCCESS)
		{
			//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_RESV_AREA_FAIL));
			return FALSE;
		}
		
		if (BT_DownloadBin(nID, &download_bin) != BT_SUCCESS)
		{
           //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			burnFile.WriteLogFile(LOG_LINE_TIME,  "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_FAIL));
			return FALSE;
		}
	}

	return TRUE;
}
//低格SD的分区
BOOL Burn_LowFormat_SD(UINT nID, HWND hWnd, PTCHAR file_name, T_PARTION_INFO *partInfo, 
					T_PMEDIUM medium, UINT StartBlock, T_MODE_CONTROL ModeCtrl, UINT *StartID, UINT *IDCnt)
{
	CAKFS cAK;
	CLogFile  burnFile(file_name);

	if (nID != 1) 
	{
		/**必须等待第一通道的在线制作ok后才能继续**/
		while (theConfig.bonline_make_image)
		{
			Sleep(10);
		}
	}

	memcpy(partInfo, theConfig.format_data, sizeof(T_PARTION_INFO)*theConfig.format_count);
	
    /*
    //扩容只能扩最后一个分区的容量
    if(MODE_NEWBURN == ModeCtrl.burn_mode)
    {
        partInfo[theConfig.format_count - 1].EnlargeSize = 8 * 1024;
    }
    */
  
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_LOW_FORMAT_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++sd low format start++\r\n");
    burnFile.WriteLogFile(0, "format_count = %d, res_blocks = %d\n", theConfig.format_count, theConfig.fs_res_blocks );

	if (!cAK.LowFormat((PBYTE)partInfo, theConfig.format_count, theConfig.fs_res_blocks, 
		StartBlock, ModeCtrl.eMedium, (PBYTE)medium, StartID, IDCnt))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_LOW_FORMAT_FAIL));
		return FALSE;
	}
	

	return TRUE;
}

//低格nand
BOOL Burn_LowFormat(UINT nID, HWND hWnd, PTCHAR file_name, T_PARTION_INFO *partInfo, 
					T_PNANDFLASH nandBase, UINT StartBlock, T_MODE_CONTROL ModeCtrl, UINT *StartID, UINT *IDCnt)
{
	CAKFS cAK;
	CLogFile  burnFile(file_name);

	if (nID != 1) 
	{
		/**必须等待第一通道的在线制作ok后才能继续**/
		while (theConfig.bonline_make_image)
		{
			Sleep(10);
		}
	}

	memcpy(partInfo, theConfig.format_data, sizeof(T_PARTION_INFO)*theConfig.format_count);
	
	//格式化碰盘的标志
	partInfo->resv[0] = 1;  //1表示有格式化， 0表示不格式化

    /*
    //扩容只能扩最后一个分区的容量
    if(MODE_NEWBURN == ModeCtrl.burn_mode)
    {
        partInfo[theConfig.format_count - 1].EnlargeSize = 8 * 1024;
    }
    */
  
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_LOW_FORMAT_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++low format start++\r\n");
    burnFile.WriteLogFile(0, "format_count = %d, res_blocks = %d\n", theConfig.format_count, theConfig.fs_res_blocks );

	if (!cAK.LowFormat((PBYTE)partInfo, theConfig.format_count, theConfig.fs_res_blocks, 
		StartBlock, ModeCtrl.eMedium, (PBYTE)nandBase, StartID, IDCnt))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_LOW_FORMAT_FAIL));
		return FALSE;
	}

	return TRUE;
}

//linux平台下， spi也可以创建分区划
BOOL Burn_CreatePartion_SpiFlash(UINT nID, HWND hWnd, PTCHAR file_name)
{
	CLogFile  burnFile(file_name);
	T_SPIFLASH_PARTION_INFO *spi_partInfo = NULL;
	UINT i = 0;
	
	spi_partInfo = new T_SPIFLASH_PARTION_INFO[theConfig.format_count];
	if (spi_partInfo == NULL)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CREATE_PARTITION_FAIL));
		return FALSE;
	}

	memcpy(spi_partInfo, theConfig.format_data, sizeof(T_PARTION_INFO)*theConfig.format_count);

	//获取spi分区的信息
	for (i = 0; i < theConfig.format_count; i++)
	{			
		spi_partInfo[i].Size = theConfig.spi_format_data[i].Size;   //此变量不相同
	}

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CREATE_PARTITION_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++create partition++\r\n");
    burnFile.WriteLogFile(0, "format_count = %d, res_blocks = %d\n", theConfig.format_count, theConfig.fs_res_blocks );

	//创建分区的功能由produce执行
    if (BT_CreatePartion_SpiFlash(nID, spi_partInfo, theConfig.format_count, theConfig.fs_res_blocks) != BT_SUCCESS)
	{
        //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
		delete[] spi_partInfo; 
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CREATE_PARTITION_FAIL));
		return FALSE;
	}
	delete[] spi_partInfo; 

	return TRUE;
}

BOOL Burn_CreatePartion(UINT nID, HWND hWnd, PTCHAR file_name, T_PARTION_INFO *partInfo)
{
	CLogFile  burnFile(file_name);
	
	memcpy(partInfo, theConfig.format_data, sizeof(T_PARTION_INFO)*theConfig.format_count);
	
    /*
    //扩容只能扩最后一个分区的容量
    if(MODE_NEWBURN == ModeCtrl.burn_mode)
    {
        partInfo[theConfig.format_count - 1].EnlargeSize = 8 * 1024;
    }
    */
  
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CREATE_PARTITION_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++create partition++\r\n");
    burnFile.WriteLogFile(0, "format_count = %d, res_blocks = %d\n", theConfig.format_count, theConfig.fs_res_blocks );

    if (BT_CreatePartion(nID, partInfo, theConfig.format_count, theConfig.fs_res_blocks) != BT_SUCCESS)
	{
        //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CREATE_PARTITION_FAIL));
		return FALSE;
	}

	return TRUE;
}

BOOL Burn_CreateDiskVolume(UINT nID, HWND hWnd, PTCHAR file_name, T_MODE_CONTROL ModeCtrl)
{
	UINT i;
	CLogFile  burnFile(file_name);

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_FORMAT_TRANSC));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++format++\r\n" );

	if (theConfig.pVolumeLable != NULL)
	{
		T_FORMAT_DRIVER formatDriver;
		
		if(MODE_NEWBURN == ModeCtrl.burn_mode)
		{
			for (i=0; i<theConfig.format_count; i++)
			{
				//formatDriver.driver_Name     = theConfig.pVolumeLable[i].Disk_Name;
				formatDriver.driver_Name     = theConfig.format_data[i].Disk_Name;
				strcpy(formatDriver.pstrVolumeLable, theConfig.pVolumeLable[i].volume_lable);
                burnFile.WriteLogFile(0, "%ddriver name=%c, label=%s\n", i, formatDriver.driver_Name, formatDriver.pstrVolumeLable );
				if (BT_FormatDriver(nID, &formatDriver) != BT_SUCCESS)
				{
                    //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
					burnFile.WriteLogFile(LOG_LINE_TIME,  "->fail\r\n");
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_FORMAT_TRANSC_FAIAL));
					//delete[] partInfo;
					return FALSE;
				}
			}
		}
		else
		{
			char driverName[26];
			for (i=0; i<theConfig.format_count; i++)
			{
				driverName[i] = theConfig.format_data[i].Disk_Name;
			}
			
			if(BT_SUCCESS != BT_MountDriver(nID, driverName, theConfig.format_count))
			{
                //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				burnFile.WriteLogFile(LOG_LINE_TIME,  "->mount fail!\r\n");
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_FORMAT_TRANSC_FAIAL));
				//delete[] partInfo;
				return FALSE;
			}
		}
		
	}
	else
	{
        //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		burnFile.WriteLogFile(LOG_LINE_TIME,  "->pVolumeLable null!\r\n");
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_FORMAT_TRANSC_FAIAL));
		//delete[] partInfo;
		return FALSE;
	}
	return TRUE;
}

//针对snowrdbirdL进行获取盘符信息
BOOL Burn_GetDiskInfo (UINT StartID, UINT DriverCnt,  UINT *DriverNum, T_DRIVER_INFO* pDriverInfo, UCHAR MediumType)
{
	CAKFS cAK;

	return cAK.GetDriverInfo(StartID, DriverCnt, DriverNum, (UCHAR *)pDriverInfo, MediumType);
}
//在线制作镜像
BOOL Burn_OnlineMakingImage(UINT nID, HWND hWnd, PTCHAR file_name, UINT sector_per_page, UINT SectorSize, UINT StartID, UINT DriverCnt, UCHAR MediumType)
{
	UINT nEnlargeDiskB_Size = 0;
	UINT i;
	UINT driverNum;
	BYTE pDriverInfoBuf[512];
	
	//在线制作镜像
	if (1)//eConfig.planform_tpye == E_ROST_PLANFORM))
	{
		if ((nID == 1 && theConfig.bonline_make_image) 
			|| (CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type))
		{
			CLogFile  burnFile(file_name);
			
			if (theConfig.format_count > 0)
			{
			   // enlargeSize setting;
				if (theConfig.burn_mode == E_CONFIG_NAND || theConfig.burn_mode == E_CONFIG_SPI_NAND)   // 只对nand版本扩容
				{
					nEnlargeDiskB_Size = GetDISKBSizeEnlarge();    // 扩容值
					burnFile.WriteLogFile(0, "enlarge size:%d\n", nEnlargeDiskB_Size);
					for (i=0; i<theConfig.format_count; i++)
					{
						if ((theConfig.format_data[i].bOpenZone) && (nEnlargeDiskB_Size != 0))  // 用户盘
						{
							theConfig.format_data[i].EnlargeSize = nEnlargeDiskB_Size; // 扩容值
						}
					}
				}
			}
			
			burnFile.WriteLogFile(LOG_LINE_TIME, "++get disk info++\n");
			if (CHIP_1080L == theConfig.chip_type || CHIP_10XXC == theConfig.chip_type)
			{
				//只有snowbirdL平台使用时，保存下载到u盘的文件所在的盘符
				Save_udiskfile_drivernum();

				if (!Burn_GetDiskInfo(StartID, DriverCnt, &driverNum, (T_DRIVER_INFO*)pDriverInfoBuf, MediumType))
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
					return FALSE;
				}
			}
			else if (BT_FAIL == BT_GetDiskInfo(nID, &driverNum, (T_DRIVER_INFO*)pDriverInfoBuf))
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
				return FALSE;
			}

			if (0 == driverNum || driverNum > 26)
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
				return FALSE;
			}
			burnFile.WriteLogFile(0, "drvNum:%d\n", driverNum);
			SetPartInfo(nID, (T_DRIVER_INFO*)pDriverInfoBuf, driverNum, sector_per_page, SectorSize, nEnlargeDiskB_Size);

			//finish image make 
			//theConfig.bonline_make_image = FALSE;

			//create image
			burnFile.WriteLogFile(LOG_LINE_TIME,"create image\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_IMAGE_CREATE));
			
			//wait creat image
			if(WaitForSingleObject(image_event, 3600000) != WAIT_OBJECT_0)
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "create time out\n");
				//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
				//delete[] partInfo;
				return FALSE;
			}
			
			if(E_IMG_FAIL == g_img_stat)
			{
				//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
				//delete[] partInfo;
				return FALSE;
			}
			else
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "++镜像制作已完成++\n");
				//由于限定只作一次制作镜像,那么在完成一次后,就复位,如果再想进行制作,那么再次进行勾选
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_RESET));
			}
			//finish image make 
			theConfig.bonline_make_image = FALSE;
			
		}
	}

	return TRUE;
}
//下载镜像文件
BOOL Burn_DownloadImg(UINT nID, HWND hWnd, PTCHAR file_name, T_MODE_CONTROL ModeCtrl, UINT SectorSize, 
					  T_NAND_PHY_INFO NandPhyInfo, T_PARTION_INFO *partInfo)
{
	UINT i, n = 0;
    UINT pagesize = 0;
    UINT  driverNo;
	T_DOWNLOAD_IMG download_img;
	CLogFile  burnFile(file_name);
	CAKFS cAK;
	UINT img_buf_len = IMG_BUF_SIZE_SD;
	UINT download_MTD_num = 0;
	BOOL mtd_flag = AK_FALSE;
	CString str;
	

	USES_CONVERSION;
	
    PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++download img++\r\n");
    
	//由于snowbirdl的烧录结构，在mount盘时没有格式化，只有通过下载镜像时才格式化
	//所以在下载镜像时，snowbirdl芯片烧录要必需每一个盘下载镜文件，那么与37，98区分开
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((theConfig.chip_type == CHIP_1080L) || (theConfig.chip_type == CHIP_10XXC)))
	{
		download_MTD_num = theConfig.format_count;
	}
	else
	{
		//37/98
		download_MTD_num = g_download_mtd_count;
	}
	burnFile.WriteLogFile(0, "download_MTD_num:%d\r\n", download_MTD_num);

	if (theConfig.burn_mode == E_CONFIG_NAND || theConfig.burn_mode == E_CONFIG_SPI_NAND)
	{
		pagesize = NandPhyInfo.page_size;
		img_buf_len = IMG_BUF_SIZE_NAND; //NAND卡的usb的传输大小
	} 
	if (theConfig.burn_mode == E_CONFIG_SD)
	{
		pagesize = SectorSize;
		img_buf_len = IMG_BUF_SIZE_SD; //SD卡的usb的传输大小
	}

	for(i = 0; i < download_MTD_num; i++)
	{
		download_img.bCompare = g_download_mtd[i].bCompare;

		//由于11平台在线生成镜像，每一个通道不一样，所以区分开
		if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((theConfig.chip_type == CHIP_1080L) || (theConfig.chip_type == CHIP_10XXC)))
		{
			mtd_flag = AK_FALSE;
			g_download_mtd_flag[nID-1] = 1;
			//根据下载镜像判断是否要进行制作镜像
			for (n = 0; n < theConfig.download_mtd_count; n++)
			{
				if (theConfig.download_mtd_data[n].disk_name[0] == theConfig.format_data[i].Disk_Name)
				{
					mtd_flag = AK_TRUE;  
					break;
				}
			}

			//如果mtd_flag是AK_TRUE
			//要根据下载mtd是否下载新制作出来的镜像，否则下载原有的
			if (AK_TRUE == mtd_flag)
			{
				if (!GetDownloadImgFromPCPath(download_img.pc_path, theApp.ConvertAbsolutePath(g_download_mtd[i].pc_path), pagesize))
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
			} 
			else
			{
				//下载已有的镜像文件
				str.Format(_T("Image\\burn%c.img"), theConfig.format_data[i].Disk_Name);
				if (!GetDownloadImgFromPCPath(download_img.pc_path, theApp.ConvertAbsolutePath(str), pagesize))
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
			}
			download_img.driver_name = (char)theConfig.format_data[i].Disk_Name;
		} 
		else
		{
			download_img.driver_name = (char)g_download_mtd[i].disk_name[0];
			if (!GetDownloadImgFromPCPath(download_img.pc_path, theApp.ConvertAbsolutePath(g_download_mtd[i].pc_path), pagesize))
			{
				//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
				//delete[] partInfo;
				return FALSE;
			}
		}

	    //_tcscpy(download_img.pc_path, theApp.ConvertAbsolutePath(g_download_mtd[i].pc_path));
        for (driverNo=0; driverNo<theConfig.format_count; driverNo++)
        {
			//先进行变大写，再比较
            if (toupper(download_img.driver_name) == toupper(partInfo[driverNo].Disk_Name))
            {
                break;
            }
        }
        if (driverNo == theConfig.format_count)
        {
            burnFile.WriteLogFile(0, "->Driver Name no matching!\r\n" );
            continue;
        }
		

        burnFile.WriteLogFile(0,"driver name:%c, driverNo:%d, mode:%d, UserZone:%d\r\n",
            download_img.driver_name, driverNo, ModeCtrl.burn_mode, partInfo[driverNo].bOpenZone);

        if ((ModeCtrl.burn_mode != MODE_UPDATE) || (partInfo[driverNo].bOpenZone == FALSE))
        {
            burnFile.WriteLogFile(LOG_LINE_TIME, "DOWNLOAD IMG...\r\n" );
			if((theConfig.chip_type == CHIP_1080L) || (theConfig.chip_type == CHIP_10XXC))
			{
				HANDLE file = INVALID_HANDLE_VALUE;
				T_IMG_INFO img_info;
				DWORD  high = 0;
				
				//打开已有的镜像文件 
				file = CreateFile(download_img.pc_path, GENERIC_READ, FILE_SHARE_READ, 
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if(INVALID_HANDLE_VALUE == file)
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}

				img_info.DriverName = g_nID_disk_info[nID-1].g_disk_info[i].diskName;
				//img_info.DriverName = download_img.driver_name;
				img_info.data_length = GetFileSize(file, &high);
				img_info.bCheck = download_img.bCompare;
				img_info.wFlag = AK_TRUE;
				//初始化文件系统库和fsa
				if(cAK.Init() != AK_TRUE)
				{
					burnFile.WriteLogFile(LOG_LINE_TIME,  "->fsa Init fail!\r\n" );
					CloseHandle(file);
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
				//下载镜像
				if(cAK.DownloadImg(nID, file, &img_info, img_buf_len) != AK_TRUE)
				{
					burnFile.WriteLogFile(LOG_LINE_TIME,  "->fail!\r\n" );
					CloseHandle(file);
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
				CloseHandle(file);

			}
			else
			{
				//10L平台外的平台的镜像下载
				if (BT_DownloadImg(nID, &download_img) != BT_SUCCESS)
				{
					burnFile.WriteLogFile(LOG_LINE_TIME,  "->fail!\r\n" );
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
			}
        }
	}

	if (theConfig.planform_tpye == E_LINUX_PLANFORM && g_disk_count != 0)
	{
		for(i = 0; i < g_disk_count; i++)
		{
			download_img.bCompare = 1;  //
		
			//下载已有的镜像文件
			str.Format(_T("Image\\burn%c.img"), g_nID_disk_info[nID-1].g_disk_info[i].diskName);
			if (!GetDownloadImgFromPCPath(download_img.pc_path, theApp.ConvertAbsolutePath(str), pagesize))
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
				return FALSE;
			}
			download_img.driver_name = (char)g_nID_disk_info[nID-1].g_disk_info[i].diskName;

			for (driverNo=0; driverNo<theConfig.format_count; driverNo++)
			{
				//先进行变大写，再比较
				if (toupper(download_img.driver_name) == toupper(partInfo[driverNo].Disk_Name))
				{
					break;
				}
			}
			if (driverNo == theConfig.format_count)
			{
				burnFile.WriteLogFile(0, "->Driver Name no matching!\r\n" );
				continue;
			}

			burnFile.WriteLogFile(0,"driver name:%c, driverNo:%d, mode:%d, UserZone:%d\r\n",
				download_img.driver_name, driverNo, ModeCtrl.burn_mode, partInfo[driverNo].bOpenZone);

			if (ModeCtrl.burn_mode != MODE_UPDATE)
			{
				//10L平台外的平台的镜像下载
				if (BT_DownloadImg(nID, &download_img) != BT_SUCCESS)
				{
					burnFile.WriteLogFile(LOG_LINE_TIME,  "->fail!\r\n" );
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
			}
		}
	}
	
	return TRUE;
}
//下载到u盘的文件
BOOL Burn_DownloadFile(UINT nID, HWND hWnd, PTCHAR file_name)
{
	UINT i, j;
	CLogFile  burnFile(file_name);
	BOOL udisk_file_flag = FALSE;
	
	USES_CONVERSION;
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_DOWNLOAD_FILE));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++download disk file++\r\n" );
    T_DOWNLOAD_FILE download_file;

    burnFile.WriteLogFile(0, "download_udisk_count:%d\r\n", g_download_udisk_count);
	for(i = 0; i < g_download_udisk_count; i++)
	{
		download_file.bCompare = g_download_udisk[i].bCompare;
		_tcscpy(download_file.pc_path, theApp.ConvertAbsolutePath(g_download_udisk[i].pc_path));
        memset(download_file.udisk_path,0,MAX_PATH+1);
		// strcpy(download_file.udisk_path, T2A(g_download_udisk[i].udisk_path));
        burnFile.WriteLogFile(0, "download_udisk_count_%d: %s\r\n", i,T2A(g_download_udisk[i].udisk_path));
		
		//linux平台的需要转换字符
		if (theConfig.planform_tpye == E_LINUX_PLANFORM)
        { 
			//针对linux
			for(j = 0; j < g_disk_count; j++)
			{
				if (g_download_udisk[i].udisk_path[0] == g_nID_disk_info[nID-1].g_disk_info[j].diskName)
				{
					udisk_file_flag = TRUE;
					break;
				}
			}
			if (TRUE == udisk_file_flag)
			{
				udisk_file_flag = FALSE;
				continue;
			}

			// change to UTF-8 code 
            UINT nLen = _tcslen(g_download_udisk[i].udisk_path);
            UINT utf8Len = WideCharToMultiByte(CP_UTF8, 0, g_download_udisk[i].udisk_path, nLen,NULL, 0, NULL, NULL);
           
            WideCharToMultiByte(CP_UTF8,0,g_download_udisk[i].udisk_path,utf8Len,download_file.udisk_path,utf8Len,NULL,NULL);
        }
		else
		{
			strcpy(download_file.udisk_path, T2A(g_download_udisk[i].udisk_path));
		}
		//下载文件
		if (BT_DownloadFile(nID, &download_file) != BT_SUCCESS)
		{
			//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
            burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
			return FALSE;
		}
	}
	
	return TRUE;
}
//下载到u盘的文件，针对swordbirdL
/*
BOOL Burn_DownloadFileNew(UINT nID, HWND hWnd, PTCHAR file_name, UINT StartID)
{
	UINT i;
	CAKFS cAK;
	CLogFile  burnFile(file_name);
	
	USES_CONVERSION;
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_DOWNLOAD_FILE));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++download disk file++\r\n" );
    T_DOWNLOAD_FILE download_file;

    burnFile.WriteLogFile(0, "download_udisk_count:%d\r\n", g_download_udisk_count);
	for(i = 0; i < g_download_udisk_count; i++)
	{
		download_file.bCompare = g_download_udisk[i].bCompare;
		_tcscpy(download_file.pc_path, theApp.ConvertAbsolutePath(g_download_udisk[i].pc_path));
        memset(download_file.udisk_path,0,MAX_PATH+1);
		// strcpy(download_file.udisk_path, T2A(g_download_udisk[i].udisk_path));
        burnFile.WriteLogFile(0, "download_udisk_count_%d: %s\r\n", i,T2A(g_download_udisk[i].udisk_path));
		
		strcpy(download_file.udisk_path, T2A(g_download_udisk[i].udisk_path));
	
		download_file.udisk_path[0] += StartID;
		//正常最大ID是0-25，因为英文字母最大的是26
		if (download_file.udisk_path[0] > ('A' + 25))
		{
            burnFile.WriteLogFile(LOG_LINE_TIME, "->fail! ID>25\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
			return FALSE;
		}

		if (!cAK.DownloadFile((UINT)&download_file))
		{
            burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
			return FALSE;
		}
	}
	
	return TRUE;
}
*/

//下载boot文件
BOOL Burn_DownloadBoot(UINT nID, HWND hWnd, PTCHAR file_name)
{
	CLogFile  burnFile(file_name);

	USES_CONVERSION;
    PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_START));
    //burnFile.WriteLogFile(LOG_LINE_TIME, "++download boot file++\r\n");
    burnFile.WriteLogFile(LOG_LINE_TIME, "boot file path:%s\r\n", W2A(theConfig.path_nandboot));
	if (CHIP_37XX_L == theConfig.chip_type)
	{
		if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot_new), theConfig.chip_type) != BT_SUCCESS)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
			return FALSE;
		}
	}
	else
	{
		if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot), theConfig.chip_type) != BT_SUCCESS)
		{
			//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
			return FALSE;
		}
	}

	return TRUE;
}

//重启
BOOL Burn_ReSetDev(UINT nID, HWND hWnd, PTCHAR file_name)
{
	//只有10和11芯片才支持重启
	if(theConfig.planform_tpye == E_ROST_PLANFORM 
		&& (CHIP_1080L == theConfig.chip_type || CHIP_11XX == theConfig.chip_type
		|| CHIP_1080A == theConfig.chip_type || CHIP_10X6 == theConfig.chip_type
		|| CHIP_10XXC == theConfig.chip_type))
	{
		if (m_worknum != 0)
		{
			//等待m_worknum == 0时才所有重启
			if(WaitForSingleObject(ResetDevice_event, 3600000) != WAIT_OBJECT_0)
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
				return FALSE;
			}
		}
		BT_ResetDevice(nID);
	}

	return TRUE;
}

//umount nandflash
void Burn_UnMountMedium(UINT StartID, T_PMEDIUM Pmedium)
{
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)))
	{
		CAKFS cAK; 
		if (theConfig.burn_mode == E_CONFIG_NAND || theConfig.burn_mode == E_CONFIG_SPI_NAND)
		{
			cAK.UnMountNandFlash(StartID);
		}
		else
		{
			cAK.UnMountMemDev(StartID);
			Burn_Free_Medium(Pmedium);
		}
		
	}
}
//mount nandflash
/*
BOOL Burn_MountMedium(UINT medium, UINT StartBlock, UINT *StartID, UINT *DriverCnt)
{
	*StartID = 0xff;

	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && (CHIP_1080L == theConfig.chip_type))
	{
		CAKFS cAK; 
		
		*StartID = cAK.MountNandFlash(medium,StartBlock, NULL, (UCHAR *)DriverCnt);

	}

	if (*StartID == 0xff)
		return FALSE;
	else
		return TRUE;

}
*/
BOOL Burn_Read_spiAlldata(UINT nID, HWND hWnd, CString path, UINT spiflash_len)
{
	TCHAR pc_path[MAX_PATH+1] = {0};
	
	USES_CONVERSION;
	
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_DOWNLOAD_FILE));
	
    CreateDirectory(theApp.ConvertAbsolutePath(path + _T("//UploadBin")), NULL);

	CString strTmp;
	
    strTmp = path + _T("//UploadBin//");
	strTmp += _T("spiflash");
	strTmp += _T(".bin");
	
	_tcscpy(pc_path , theApp.ConvertAbsolutePath(strTmp));
	//回读bin文件
	if (BT_Upload_SpiAlldata(nID, pc_path, spiflash_len) != BT_SUCCESS)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
		return FALSE;
	}

	
	return TRUE;

}

BOOL BurnDebugMode(UINT nID, HWND hWnd, CString path)
{
	T_UPLOAD_BIN uploadbin;
	T_U32 i;
	
	USES_CONVERSION;
	
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_DOWNLOAD_FILE));

    CreateDirectory(theApp.ConvertAbsolutePath(path + _T("//UploadBin")), NULL);
#if 1
	for(i = 0; i < g_download_nand_count; i++)
	{
		CString strTmp;
		memset(&uploadbin, 0, sizeof(T_UPLOAD_BIN));
		memcpy(uploadbin.file_name, T2A(g_download_nand[i].file_name), 16);
		
        strTmp = path + _T("//UploadBin//");
		strTmp += g_download_nand[i].file_name;
		strTmp += _T(".bin");

		_tcscpy(uploadbin.pc_path , theApp.ConvertAbsolutePath(strTmp));
		//回读bin文件
		if (BT_UploadBin (nID, &uploadbin) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
			return FALSE;
		}
	}
#endif
	//uplaod Bios
	CString strTmp;
	strTmp = path + _T("//UploadBin//BOOT.bin");
	//回读boot文件
	if (BT_UploadBoot(nID, theApp.ConvertAbsolutePath(strTmp), 32*1024) != BT_SUCCESS)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
		return FALSE;
	}

	return TRUE;
}

//bin回读功能
BOOL Burn_DebugMode(UINT nID, HWND hWnd, PTCHAR file_name)
{

	TCHAR   BinUpload_path[MAX_PATH+1];
	CLogFile  burnFile(file_name);
	
	burnFile.WriteLogFile(LOG_LINE_TIME, "upload bin\n");
	browser_for_binUpload(BinUpload_path);
	
	if (!BurnDebugMode(nID, hWnd, BinUpload_path)) 
	{
		return FALSE;
	}
		
	return TRUE;	
}

BOOL Burn_Get_spiAlldata(UINT nID, HWND hWnd, PTCHAR file_name, UINT spiflash_len)
{
	
	TCHAR   Uploadspi_path[MAX_PATH+1];
	CLogFile  burnFile(file_name);
	
	burnFile.WriteLogFile(LOG_LINE_TIME, "upload bin\n");
	browser_for_binUpload(Uploadspi_path);
	
	if (!Burn_Read_spiAlldata(nID, hWnd, Uploadspi_path, spiflash_len)) 
	{
		return FALSE;
	}
	
	return TRUE;	
}


//当是ak10xx时，那么获取boot下的值是否是CHIP_1080A还是CHIP_1080L
BOOL Burn_GetRegValue(UINT nID, HWND hWnd, PTCHAR file_name)
{
//	CLogFile  burnFile(file_name);

//	burnFile->WriteLogFile(LOG_LINE_TIME, "Burn_GetRegValue fail\n");

	if (theConfig.chip_type == CHIP_10X6)
	{
		UINT metafixAddr = 0x6228, metafixValue;
		if (BT_GetRegValue(nID, metafixAddr, &metafixValue) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
			return FALSE;
		}
		if (0x32424e41 == metafixValue)
		{
			//AK3671芯片V6.0版本的芯片
			theConfig.chip_type = CHIP_1080A;
		}
		else
		{
			//读取芯片的chip id
			metafixAddr = 0x00400000, metafixValue;
			if (BT_GetRegValue(nID, metafixAddr, &metafixValue) != BT_SUCCESS)
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
				return FALSE;
			}
			if (0x20120300 == metafixValue)
			{
				theConfig.chip_type = CHIP_1080L;//1080L,1060L,1050L都是同一种
			}
			else if(0x20120A00 == metafixValue)
			{
				theConfig.chip_type = CHIP_10XXC;
			}
		}
	}
	//区分是37还是37L
	if (theConfig.chip_type == CHIP_37XX || theConfig.chip_type == CHIP_37XX_L)
	{
		UINT metafixAddr = 0x08000000, metafixValue;
		if (BT_GetRegValue(nID, metafixAddr, &metafixValue) != BT_SUCCESS)
		{
			return FALSE;
		}
		if (0x20130100 == metafixValue)
		{
			//AK37L芯片V6.0版本的芯片
			theConfig.chip_type = CHIP_37XX_L;
		}
		else
		{
			theConfig.chip_type = CHIP_37XX;
		}
	}
	return TRUE;
}

//u盘方式下的烧录等待1S时间让所以设备都进行massboot
BOOL Burn_Udisk_Wait_Massboot(CLogFile  burnFile)
{
	//UINT i   = 0;
	//BOOL udisk_flagtemp = FALSE;
	//u盘烧录方式下，当每开始一个设备，m_budisk_getUSBnum自减1，
	//一直等于0时才激活事件，否则都等待状态
	if (theConfig.bUDiskUpdate && m_budisk_getUSBnum)
	{
		
		WaitForSingleObject(g_handle,INFINITE);
		burnFile.WriteLogFile(0, "01 g_udisk_burnnum: %d\n", g_udisk_burnnum);
		g_udisk_burnnum--;
		burnFile.WriteLogFile(0, "02 g_udisk_burnnum: %d\n", g_udisk_burnnum);
		ReleaseSemaphore(g_handle,1,NULL);

		
		if (g_udisk_burnnum == 0)
		{
			SetEvent(udiskburn_event);
		}
		burnFile.WriteLogFile(0, "03 g_udisk_burnnum: %d\n", g_udisk_burnnum);
		
		//wait creat image
		if(WaitForSingleObject(udiskburn_event, 3600000) != WAIT_OBJECT_0)
		{
			return FALSE;
		}
	}
	return TRUE;

}

BOOL Burn_capacity_check_Wait(UINT nID)
{
	CString str;
	UINT idex   = 0;
	
	
	WaitForSingleObject(g_handle,INFINITE);
	g_capacity_burnnum++;
	ReleaseSemaphore(g_handle,1,NULL);

	//compare the nand size
	for (idex = 0; idex < theConfig.device_num; idex++)
	{
		if (g_capacity_size[idex] != 0 && g_capacity_flag == AK_TRUE)
		{
			if (g_capacity_size[idex] != g_capacity_size[nID-1])
			{
				//str.Format(IDS_MEDIUM_CAPACITY_CHECK_FAIL);
				//AfxMessageBox(str, MB_OK);
				g_capacity_flag = AK_FALSE;
				SetEvent(capacity_event);
			}
		}
	}
	
	//
	if (g_capacity_flag == AK_TRUE && g_capacity_burnnum == m_worknum)
	{
		SetEvent(capacity_event);
	}
	
	//wait creat image
	if(WaitForSingleObject(capacity_event, 3600000) != WAIT_OBJECT_0)
	{
		g_capacity_size[nID-1] = 0;
		return FALSE;
	}

	g_capacity_size[nID-1] = 0;
	//有一台容量不相同，其他的都失败
	if (AK_FALSE == g_capacity_flag)
	{
		return FALSE;
	}
	
	return TRUE;
	
}


BOOL BurnThread(UINT nID)
{
    HWND hWnd = AfxGetMainWnd()->GetSafeHwnd();
    CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	T_PARTION_INFO *partInfo = NULL;
	T_MODE_CONTROL ModeCtrl;
    T_TRANSC_PARA  transc_para = {0};

	//读asa区的mac地址
	BOOL readmacflag = FALSE;
	TCHAR mactempbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR maclowbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	UINT  macmode = 0;

	//读asa区的序列号地址
	BOOL readSerialflag = FALSE;
	TCHAR serialtempbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR seriallowbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	UINT  Serialmode = 0;
	UINT com_mode = 0;
	CString str;
	UINT sector_per_page = 1;
    UINT SectorSize = 512;
	UINT pagesize = 0;
	UINT StartID = 0;
	UINT IDCnt   = 0;
	UINT flash_ChipCnt = 0;
	T_NAND_PHY_INFO NandPhyInfo = {0};
	T_PMEDIUM   Pmedium = NULL;
	time_t ltime;
//	struct _timeb timebuffer;
//	char *timeline;


    TCHAR     file_name[MAX_PATH] = {0};
    SYSTEMTIME burnTime;
    GetLocalTime(&burnTime);
    swprintf(file_name, _T("log\\ch[%d]_log%02d_%02d_%02d.txt"), nID,
        burnTime.wYear,
        burnTime.wMonth,
        burnTime.wDay);
    CLogFile  burnFile(file_name);
    
    burnFile.WriteLogFile(0, "=============Burn Information For Channel[%d]=============\n", nID);
    burnFile.WriteLogFile(0, "Print Time: %d-%d-%d %02d:%02d:%02d\n\n",
        burnTime.wYear,
        burnTime.wMonth,
        burnTime.wDay,
        burnTime.wHour,
        burnTime.wMinute,
        burnTime.wSecond);
	burnFile.WriteLogFile(0, "ss g_udisk_burnnum: %d\n", g_udisk_burnnum);


	//u盘方式下的烧录等待1S时间让所以设备都进行massboot
	/*
	if (!Burn_Udisk_Wait_Massboot(burnFile))
	{
		burnFile.WriteLogFile(0, "Burn_Udisk_Wait_Massboot fail\n");
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
		return FALSE;
	}
	*/
	if (theConfig.bUDiskUpdate && m_budisk_getUSBnum)
	{
		WaitForSingleObject(g_handle,INFINITE);

		//_ftime( &timebuffer );
		//timeline = ctime( & ( timebuffer.time ) );
		//burnFile.WriteLogFile(0, "The time is %.19s.%hu", timeline, timebuffer.millitm);

		time(&ltime);
		burnFile.WriteLogFile(0, "01 g_udisk_burnnum: %d, %d\n", g_udisk_burnnum, ltime);
		g_udisk_burnnum--;
		burnFile.WriteLogFile(0, "02 g_udisk_burnnum: %d\n", g_udisk_burnnum);
		ReleaseSemaphore(g_handle,1,NULL);
	
		if (g_udisk_burnnum == 0)
		{
			SetEvent(udiskburn_event);
		}
		burnFile.WriteLogFile(0, "03 g_udisk_burnnum: %d\n", g_udisk_burnnum);
		
		//wait creat image
		if(WaitForSingleObject(udiskburn_event, 3600000) != WAIT_OBJECT_0)
		{
			burnFile.WriteLogFile(0, "Burn_Udisk_Wait_Massboot fail\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
			return FALSE;
		}
	}
	

	//每个通过开始时进行状态清0
	pMain->StartCountTime(nID);
    if (m_worknum == 0)
    {
        pMain->disable_control();
    }

    m_worknum++;
	g_nID_disk_info[nID-1].nID = nID;
	burn_detel_usb_flag[nID-1] = 1;
	g_img_stat = E_IMG_INIT;

    burnFile.WriteLogFile(LOG_LINE_TIME, "work number = %d\n", m_worknum);

	USES_CONVERSION;

	//烧录模式的改变
	theConfig.ChangeBurnMode(&ModeCtrl);

    burnFile.WriteLogFile(LOG_LINE_TIME, "burn_mode = %d, eMedium = %d\n", ModeCtrl.burn_mode, ModeCtrl.eMedium);
	
	//如果是ak10XX,那么要区分出是10x6还是chip10XX_160K
	if (!Burn_GetRegValue(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_GetRegValue fail\n");
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
		return FALSE;
	}

	transc_para.planform = theConfig.planform_tpye; // 平台类型
	transc_para.time = theConfig.event_wait_time;   //等待时间
	transc_para.ChipType = theConfig.chip_type;     //芯片类型

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_BURN));

	//传输平台和芯片的信息
	BT_SetTranscParam(&transc_para);

	//下载变频小程序
    if (!Burn_DownloadChangClock(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DownloadChangClock fail\n");
		return FALSE;
	}
	//设置内存参数
    if (!Burn_SetRamParam(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetRamParam fail\n");
		return FALSE;
	}

    //#ifdef  SUPPORT_LINUX
	if (theConfig.planform_tpye== E_LINUX_PLANFORM)
	{
		if (FALSE == Download_Channel_Addranddata_ToRam( nID,  hWnd, file_name))
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->Download_Channel_Addranddata_ToRam fail.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_CHANNELID_FAIL));
			return FALSE;
		}	
	}//#endif //SUPPORT_LINUX 

	//在rtos平台下，需要写入串口类型
    if (!Burn_RTOSDownComInfo(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_RTOSDownComInfo fail\n");
		return FALSE;
	}


	if (TRUE == theConfig.bTest_RAM)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_AND_TESTRAM_START));
		if (BT_Download_and_TestRam(nID, theApp.ConvertAbsolutePath(theConfig.path_producer), 
			theConfig.producer_run_addr, theConfig.ram_param.size*1024*1024) != BT_SUCCESS)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "download and test ram fail.\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_AND_TESTRAM_FAIL));
			return FALSE;
		}
		else
		{
			burnFile.WriteLogFile(LOG_LINE_TIME,  "->test ram success\r\n");
			goto BURN_SUCCESS;
		}
	}

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_PRODUCER_START));
    if (E_CONFIG_JTAG == theConfig.burn_mode)
	{
        burnFile.WriteLogFile(LOG_LINE_TIME,  "->Mode is JTAG. Quit!\r\n");
		goto BURN_SUCCESS;
	}
	
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++Start download producer++\r\n");
    burnFile.WriteLogFile(0,  "producer path: %s, addr=%08x\n", W2A(theConfig.path_producer), theConfig.producer_run_addr);
	
	//下载produce
    if (BT_DownloadProducer(nID, theApp.ConvertAbsolutePath(theConfig.path_producer), 
        theConfig.producer_run_addr) != BT_SUCCESS)
    {
		burnFile.WriteLogFile(LOG_LINE_TIME, "download producer fail.\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_PRODUCER_FAIL));
        return FALSE;
    }
	
	//调试模式
	if (E_CONFIG_DEBUG == theConfig.burn_mode)
	{
        burnFile.WriteLogFile(LOG_LINE_TIME,  "->Debug mode. Quit!\r\n");
		goto BURN_SUCCESS;
	}
	
    //wait for producer initial finish
    if (!Burn_WaitProducerInitFinish(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_WaitProducerInitFinish fail.\r\n" );
		return FALSE;
	}   
	//在10L芯片上是不需要进行转换USB
    if (!Burn_SwitchUSB11To20(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SwitchUSB11To20 fail.\r\n" );
		return FALSE;
	} 
	//测试USB
    if (!Burn_TestPCAndProducerConnectOK(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_TestPCAndProducerConnectOK fail.\r\n" );
		return FALSE;
	}

    if (!Burn_SetGPIO(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetGPIO fail.\r\n" );
		return FALSE;
	}	

    if (!Burn_SetBurnPara(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetBurnPara fail.\r\n" );
		return FALSE;
	}

	//由于10bLUE平台使用的produce是根据MODE_DEBUG值进行读取spi文件的信息,
	//所以在bin回读时进行ModeCtrl.burn_mode = MODE_DEBUG;
	if(g_bUploadbinMode && (theConfig.planform_tpye== E_ROST_PLANFORM) && (theConfig.chip_type == CHIP_1080A) && (E_CONFIG_SFLASH == theConfig.burn_mode))
	{
		ModeCtrl.burn_mode = MODE_DEBUG;
	}

    if (!Burn_SetBurnMode(nID, hWnd, file_name, ModeCtrl))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetBurnMode fail.\r\n" );
		return FALSE;
	}
	
    if (!Burn_SetGPIOToNandChip(nID, hWnd, file_name, ModeCtrl))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetGPIOToNandChip fail.\r\n" );
		return FALSE;
	}

	if (TRANSC_MEDIUM_EMMC == ModeCtrl.eMedium)
	{
		if (g_bUploadbinMode)
		{
			goto UPDATA_BINMODE;//bin回读
		}
		else
		{
			goto SET_RESV_AREA;//sd卡烧录
		}
	}

	if (TRANSC_MEDIUM_SPIFLASH == ModeCtrl.eMedium)
	{
		//spi 烧录
		if (burn_spiflash(nID, hWnd, ModeCtrl, file_name))
		{
			if (g_bUploadbinMode)
			{
				goto UPDATA_BINMODE;//bin回读
			}
			else
			{
				goto BURN_SUCCESS;
			}
		}
		else
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "burn_spiflash fail.\r\n" );
			return FALSE;
		}
	}

	//
    if (!Burn_GetFlashIDAndSetNandParam(nID, hWnd, file_name, &sector_per_page, &SectorSize, &NandPhyInfo, &flash_ChipCnt))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_GetFlashIDAndSetNandParam fail.\r\n" );
		return FALSE;
	}

	//只有全新烧录和nand烧录时才会有擦除操作
	if (ModeCtrl.burn_mode == MODE_NEWBURN && g_bEraseMode && (theConfig.burn_mode == E_CONFIG_NAND || theConfig.burn_mode == E_CONFIG_SPI_NAND))
	{
		if (!Burn_SetEraseMode(nID, hWnd, file_name, ModeCtrl))
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetEraseMode fail.\r\n" );
			return FALSE;
		}
	}
	
	//低格完成
	if(g_bEraseMode)
	{
		//Finish
		//g_bEraseMode = FALSE;
		goto BURN_SUCCESS;
	}

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_SEC_AREA_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++init security area++\r\n");
    //建立安全区
	if (BT_InitSecArea(nID, 0) != BT_SUCCESS)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "BT_InitSecArea fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_SEC_AREA_FAIL));
		return FALSE;
	}

UPDATA_BINMODE:
	// upload bin call burndebugmode  and then finish burning
	if (g_bUploadbinMode)	
	{
		//实现bin回读，读出bin数据和boot数据理力争
		if(!Burn_DebugMode(nID,  hWnd,  file_name)) 
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DebugMode fail!\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BIN_UPLOAD_FAIL));
			return FALSE;
		}
		else
		{	
			goto BURN_SUCCESS;
		}
	}

	//只有是10L芯片并是nand版本才会进行获取坏块信息
	if (fNand_Get_BadBlockBuf(nID, NandPhyInfo.blk_num, flash_ChipCnt, &ModeCtrl)  != BT_SUCCESS)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "fNand_Get_BadBlockBuf fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_BAD_BLOCK_FAIL));
		return FALSE;
	}

SET_RESV_AREA:
	/*****************************************************/
	//判断是否烧录MAC地址
    if (!Burn_GetMACInfo(nID, hWnd, file_name, maclowbuf, mactempbuf, &readmacflag, &macmode))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_GetMACInfo fail!\r\n" );
		fNand_Free_BadBlockBuf(nID);
		return FALSE;
	}

	//写序列号
	if (!Burn_GetSERIALInfo(nID, hWnd, file_name, seriallowbuf, serialtempbuf, &readSerialflag, &Serialmode))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_GetSERIALInfo fail!\r\n" );
		fNand_Free_BadBlockBuf(nID);
		return FALSE;
	}
	
	/*****************************************************/
	//设置非文件系系统保留区大小，
	//并对此区进行擦
	if (!Burn_SetResvAreaSize(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetResvAreaSize fail!\r\n" );
        Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		return FALSE;
	}
	
	//下载bin文件
	if (!Burn_DownloadBin(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DownloadBin fail!\r\n" );
        Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		return FALSE;
	}
	
	//snowbirdL升级模式只下载bin和boot
	if ((ModeCtrl.burn_mode == MODE_UPDATE) && (theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)))
	{
		goto BURN_BOOT;
	}

	partInfo = new T_PARTION_INFO[theConfig.format_count];
	if (partInfo == NULL)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "partInfo null!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_PARTTION_INFORMATION_IS_NULL));
		return FALSE;
	}

	T_NANDFLASH nandBase; /*文件系统的时候需要用到nandbase*/
	UINT        StartBlock; //文件系统分区的开始位置
	UINT        capacity_size;

	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)))
	{
		if (TRANSC_MEDIUM_NAND == ModeCtrl.eMedium || TRANSC_MEDIUM_SPI_NAND == ModeCtrl.eMedium)
		{
			//通过produce获取nand的信息
			if (BT_GetMediumStruct(nID, (unsigned char *)&nandBase, sizeof(T_NANDFLASH), &StartBlock) != BT_SUCCESS)
			{
				//只是拿来测试而已
				burnFile.WriteLogFile(LOG_LINE_TIME, "BT_GetMediumStruct fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_MEDIUM_DATAINFO_FAIL));
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				delete[] partInfo;
				return FALSE;
			}
			capacity_size = nandBase.BlockPerPlane*nandBase.PlanePerChip*(nandBase.PagePerBlock*nandBase.SectorPerPage*nandBase.BytesPerSector/1024); //以k为单位存放
			g_capacity_size[nID-1] = capacity_size;
			

			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MMESSAGE_MEDIUM_CAPACITY_CHECK));
			//check the nand capacity is or not differnt
			if (!Burn_capacity_check_Wait(nID))
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_capacity_check_Wait nand fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_MEDIUM_CAPACITY_FAIL));
				return FALSE;
			}
			
			/**重新附值nandbase的部分参数**/
			Burn_ResetNandBase(nID, &nandBase);
			
			//获取所有的空闲块
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_ALL_FREE_BLOCK));
			if (fNand_Get_FreeBlockBuf(nID, NandPhyInfo.chip_id, nandBase.BlockPerPlane*nandBase.PlanePerChip, flash_ChipCnt, StartBlock) != BT_SUCCESS)
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "fNand_Get_FreeBlockBuf fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_FREE_BLOCK_FAIL));
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				delete[] partInfo;
				return FALSE;
			}

			//if (!Burn_MountMedium((UINT)&nandBase, StartBlock, &StartID, &IDCnt))
			if (!Burn_LowFormat(nID, hWnd, file_name, partInfo, &nandBase, StartBlock, ModeCtrl, &StartID, &IDCnt))
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_LowFormat fail!\r\n" );
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				delete[] partInfo;
				Burn_UnMountMedium(StartID, Pmedium);
				return FALSE;
			}

			//由于小页的nand，需要取合并的
			if (SectorSize == 512)
			{
				sector_per_page = nandBase.BytesPerSector / SectorSize;//计算一下nand返回来的一个页有多少个扇区
			}
			

			//StartID = 0;//因为底层没有实现，所以先附值
		}
		else if(TRANSC_MEDIUM_EMMC == ModeCtrl.eMedium)
		{
			T_EMMC_INFO medium;
			UINT     SD_StartBlock = 0;
			UINT     SecPerPg = 1;
			
			//获取sd卡的信息
			if (BT_GetMediumStruct(nID, (unsigned char *)&medium, sizeof(T_EMMC_INFO), &SD_StartBlock) != BT_SUCCESS)
			{
				//只是拿来测试而已
				burnFile.WriteLogFile(LOG_LINE_TIME, "BT_GetMediumStruct fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_MEDIUM_DATAINFO_FAIL));
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				delete[] partInfo;
				return FALSE;
			}

			g_capacity_size[nID-1] = (medium.total_block*medium.block_size) >> 10; //以k为单位存放

			//check the sd capacity is or not differnt
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MMESSAGE_MEDIUM_CAPACITY_CHECK));
			if (!Burn_capacity_check_Wait(nID))
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_capacity_check_Wait sd fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_MEDIUM_CAPACITY_FAIL));
				return FALSE;
			}

			//构造一个medium出来
			Pmedium = Burn_Malloc_Medium(nID, medium.block_size, medium.total_block, SecPerPg);
			if (NULL == Pmedium)
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_Malloc_Medium fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_MALLOC_MEDIUM_FAIL));
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				Burn_Free_Medium(Pmedium);
				delete[] partInfo;
				return FALSE;
			}

			//Burn_ResetMedium(nID, &medium);
			if (!Burn_LowFormat_SD(nID, hWnd, file_name, partInfo, Pmedium, SD_StartBlock, ModeCtrl, &StartID, &IDCnt))
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_LowFormat_SD fail!\r\n" );
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				Burn_UnMountMedium(StartID, Pmedium);
				delete[] partInfo;
				return FALSE;
			}
		}
	}
	else
	{
		//10L平台外的创建分区的操作
		if (!Burn_CreatePartion(nID, hWnd, file_name, partInfo))
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_CreatePartion fail!\r\n" );
			Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			//Burn_UnMountMedium(StartID);
			delete[] partInfo;
			return FALSE;
		}
	}

	//创建卷标
	if (!Burn_CreateDiskVolume(nID, hWnd, file_name, ModeCtrl))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_CreateDiskVolume fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		Burn_UnMountMedium(StartID, Pmedium);
		delete[] partInfo;
		return FALSE;
	}

	/**由于在线制作镜像的时候会占有A开始的磁盘，所有必须先卸载driver**/
	if (!Burn_OnlineMakingImage(nID, hWnd, file_name, sector_per_page, SectorSize, StartID, IDCnt, (UCHAR)ModeCtrl.eMedium))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_OnlineMakingImage fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		Burn_UnMountMedium(StartID, Pmedium);
		delete[] partInfo;
		return FALSE;
	}

	//下开镜像文件
	if (!Burn_DownloadImg(nID, hWnd, file_name, ModeCtrl, SectorSize, NandPhyInfo, partInfo))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DownloadImg fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		Burn_UnMountMedium(StartID, Pmedium);
		delete[] partInfo;
		return FALSE;
	}

    delete[] partInfo;

	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)))
	{
/*
		速度上让人没法承受，所以不支持文件的直接写入, 直接是镜像的写入
		if (!Burn_DownloadFileNew(nID, hWnd, file_name, StartID))
		{
			Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			Burn_UnMountMedium(StartID, Pmedium);
			return FALSE;
		}	
*/
		//umount分区
		Burn_UnMountMedium(StartID, Pmedium);
		
	}
	else
	{
		//下载文件到u盘上
		if (!Burn_DownloadFile(nID, hWnd, file_name))
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DownloadFile fail!\r\n" );
			Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			return FALSE;
		}	
	}

BURN_BOOT:
	if (!Burn_DownloadBoot(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DownloadBoot fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		return FALSE;
	}
	//记录已写的MAC地址
	if (!Burn_WriteMACInfo(nID, hWnd, file_name, maclowbuf, mactempbuf, readmacflag, macmode))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_WriteMACInfo fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		return FALSE;
	}

	//记录序列号
	if (!Burn_WriteSERIALInfo(nID, hWnd, file_name, seriallowbuf, serialtempbuf, readSerialflag, Serialmode))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_WriteSERIALInfo fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		return FALSE;
	}


	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_START));
    burnFile.WriteLogFile(LOG_LINE_TIME, "++write config++\r\n");
    if (BT_Close(nID) != BT_SUCCESS)
    {
        burnFile.WriteLogFile(LOG_LINE_TIME,  "BT_Close fail!\r\n");
		fNand_Free_BadBlockBuf(nID);
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_FAIL));
        return FALSE;
    }

	//test save info before sd fs 
	//PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_START));
	/*
	//这里主要是回读所有的数据放到一个文件内，并只有对sd卡，
    if (0)//(MEDIUM_EMMC == ModeCtrl.eMedium)
	{
        T_UPLOAD_BIN pUploadBin;

		_tcscpy(pUploadBin.pc_path , theApp.ConvertAbsolutePath(_T("ALLDATA.upd")));
		memcpy(pUploadBin.file_name, "ALLDATA", DOWNLOAD_BIN_FILENAME_SIZE);

		if (BT_UploadBin(nID, &pUploadBin) != BT_SUCCESS)
        {
			fNand_Free_BadBlockBuf(nID);
			//PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_FAIL));
			return FALSE;
            
        }
	}
	*/
BURN_SUCCESS:

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_TASK_COMPLETE));
   
	if (m_worknum != 0)
    {
        m_worknum--;
        if (0 == m_worknum)
        {
			//当所以通首的都完成后，再进行设置事件
			SetEvent(ResetDevice_event);
			g_bEraseMode = FALSE; //是否低格所有块
			m_budisk_burn = FALSE; //
			m_budisk_getUSBnum = FALSE; //u盘烧录模式下的标志
			USB_attachflag = FALSE;
			g_capacity_flag = AK_TRUE;
            pMain->enable_control();
        }
    }

	//在烧录完所有通道后，进行统一重启
	if (!Burn_ReSetDev(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME,  "Burn_ReSetDev fail!\r\n");
		fNand_Free_BadBlockBuf(nID);
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_FAIL));
        return FALSE;
	}
	
	g_download_mtd_flag[nID-1] = 0;
	g_bUpload_spialldata = FALSE;
	g_bUploadbinMode = FALSE;
	
    burnFile.WriteLogFile(LOG_LINE_TIME, "++Burn success++\r\n");
	pMain->StopCountTime(nID);
	fNand_Free_BadBlockBuf(nID);
	pMain->set_window_title();

    return TRUE;
}

void BurnProgress(UINT nID, UINT nDatLen)
{
	HWND hWnd = AfxGetMainWnd()->GetSafeHwnd();
	PostMessage(hWnd, ON_BURNFLASH_PROCESS, WPARAM(nID+100), nDatLen);
}

//回读bin
T_BOOL browser_for_binUpload(TCHAR *folderPath)
{
	BROWSEINFOW opp;
	
	opp.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	opp.pidlRoot = NULL;
	opp.pszDisplayName = folderPath;
	opp.lpszTitle = _T("选择一个文件夹，然后点击'确定'");
	opp.ulFlags = BIF_RETURNONLYFSDIRS;
	opp.lpfn = NULL;
	opp.iImage = 0;

	LPITEMIDLIST pIDList = SHBrowseForFolder(&opp); //调用显示选择对话框

	if (pIDList)
	{
	    SHGetPathFromIDList(pIDList, folderPath);
	    return TRUE;
	}
	else 
	{
		return FALSE;
	}
}
//mac地址加一
BOOL Mac_Addr_add_1(TCHAR *buf, TCHAR *buf_temp)
{
	CHAR tempbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR tempAddrBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR dstAddrBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	UINT tempmac;
	
	//比较mac地址是否大于结束的mac地址
	if(_tcscmp(buf_temp, theConfig.mac_end_low) >= 0)
	{
		return FALSE;
	}
	
	_tcscpy(buf, buf_temp);
	//对低位进行十六进制加一
	sprintf(tempbuf, "%c%c%c%c%c%c", buf_temp[0], buf_temp[1], buf_temp[3], buf_temp[4], buf_temp[6], buf_temp[7]);
	//地址递增一
	sscanf(tempbuf, "%x", &tempmac);
	tempmac ++;
	sprintf(tempbuf, "%06x", tempmac);
	swprintf(tempAddrBuf, _T("%c%c:%c%c:%c%c"), tempbuf[0],tempbuf[1],tempbuf[2],tempbuf[3],tempbuf[4],tempbuf[5]);
	theConfig.lower_to_upper(tempAddrBuf, dstAddrBuf);
	_tcscpy(theConfig.mac_current_low, dstAddrBuf);  //记录当前的mac地址

	return TRUE;
}

//序列号地址加1
BOOL serial_Addr_add_1(TCHAR *buf, TCHAR *buf_temp)
{
	UINT tempsequence;
	CString str;

	USES_CONVERSION;

	//比较序列号是否大于结束的序列号
	if(_tcscmp(buf_temp, theConfig.sequence_end_low) >= 0)
	{
		return FALSE;
	}
	_tcscpy(buf, buf_temp);
	//地址递增一
	tempsequence = atoi(T2A(buf_temp));
	tempsequence++;
	str.Format(_T("%06d"), tempsequence);
	_tcscpy(theConfig.sequence_current_low, str); //记录当前的序列号
	
	return TRUE;
}

BOOL Get_Mac_Addr(TCHAR *buf, UINT channelID)
{
	TCHAR tempAddrBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR tempcurrnetbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	UINT i;

	if(WaitForSingleObject(theConfig.m_hMutGetAddr, 5000) != WAIT_OBJECT_0)
	{
		ReleaseMutex(theConfig.m_hMutGetAddr);
		return FALSE;
	}
    //如果之前有些通道出错烧录失败的，获取此通道的mac地址
	if (theConfig.g_mac_current_low[channelID-1][0] != 0)
	{
		memcpy(buf, theConfig.g_mac_current_low[channelID-1], MAX_MAC_SEQU_ADDR_COUNT);
		memset(theConfig.g_mac_current_low[channelID-1], 0, MAX_MAC_SEQU_ADDR_COUNT+1);

		//当烧录失败时，再进入烧录，如果全局变量值大于起始值的话，那么就用起始值
		if (_tcscmp(theConfig.mac_start_low, buf) > 0)
		{
			while(1)
			{
				if (!Mac_Addr_add_1(buf, theConfig.mac_current_low))
				{
					ReleaseMutex(theConfig.m_hMutGetAddr);
					return FALSE;
				}

				//增加判断buf是否全0，如果是就再自增
				if(!is_zero_ether_addr(buf))
				{
					break;
				}

			}
		}

		//比较mac地址是否大于结束的值
		if(_tcscmp(buf, theConfig.mac_end_low) >= 0)
		{
			ReleaseMutex(theConfig.m_hMutGetAddr);
			return FALSE;
		}
	}
	else
	{
		for (i=0; i<32; i++)
		{
			//判断是否有些通过在上一次有烧录失败过
			if (theConfig.g_mac_current_low[i][0] != 0)
			{
				memcpy(buf, theConfig.g_mac_current_low[i], MAX_MAC_SEQU_ADDR_COUNT);
				memset(theConfig.g_mac_current_low[i], 0, MAX_MAC_SEQU_ADDR_COUNT+1);
				theConfig.read_currentdevicenum_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, NULL, tempcurrnetbuf, channelID-1);
				theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, passY, tempcurrnetbuf, i);
				theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, passN, buf, channelID-1);

				ReleaseMutex(theConfig.m_hMutGetAddr);
				return TRUE;
			}
		}

		while(1)
		{
			if (!Mac_Addr_add_1(buf, theConfig.mac_current_low))
			{
				ReleaseMutex(theConfig.m_hMutGetAddr);
				return FALSE;
			}

			//增加判断buf是否合法，如果不合法就再自增
			if(!is_zero_ether_addr(buf))
			{
				break;
			}

		}
	} 
	ReleaseMutex(theConfig.m_hMutGetAddr);

	return TRUE;
}

BOOL Get_serial_Addr(TCHAR *buf, UINT channelID)
{
	TCHAR tempcurrnetbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	UINT  i;
	CString str;
	
	if(WaitForSingleObject(theConfig.m_hMutGetAddr, 5000) != WAIT_OBJECT_0)
	{
		ReleaseMutex(theConfig.m_hMutGetAddr);
		return FALSE;
	}

    //如果之前有些通道出错烧录失败的，获取此通道的序列号  
	if (theConfig.g_sequence_current_low[channelID-1][0] != 0)
	{
		memcpy(buf, theConfig.g_sequence_current_low[channelID-1], MAX_MAC_SEQU_ADDR_COUNT);
		memset(theConfig.g_sequence_current_low[channelID-1], 0, MAX_MAC_SEQU_ADDR_COUNT+1);

		//当烧录失败时，再进入烧录，如果全局变量值大于起始值的话，那么就用起始值
		if (_tcscmp(theConfig.sequence_start_low, buf) > 0)
		{
			if (!serial_Addr_add_1(buf, theConfig.sequence_current_low))
			{
				ReleaseMutex(theConfig.m_hMutGetAddr);
				return FALSE;
			}
		}
		//比较序列是否大于结束的值
		if(_tcscmp(buf, theConfig.sequence_end_low) >= 0)
		{
			ReleaseMutex(theConfig.m_hMutGetAddr);
			return FALSE;
		}
	}
	else
	{
		for (i=0; i<32; i++)
		{
			//判断是否有些通过在上一次有烧录失败过
			if (theConfig.g_sequence_current_low[i][0] != 0)
			{
				memcpy(buf, theConfig.g_sequence_current_low[i], MAX_MAC_SEQU_ADDR_COUNT);
				memset(theConfig.g_sequence_current_low[i], 0, MAX_MAC_SEQU_ADDR_COUNT+1);
				theConfig.read_currentdevicenum_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, NULL, tempcurrnetbuf, channelID-1);
				theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, passY, tempcurrnetbuf, i);
				theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, passN, buf, channelID-1);

				ReleaseMutex(theConfig.m_hMutGetAddr);
				return TRUE;
			}
		}

		if (!serial_Addr_add_1(buf, theConfig.sequence_current_low))
		{
			ReleaseMutex(theConfig.m_hMutGetAddr);
			return FALSE;
		}
	} 
	ReleaseMutex(theConfig.m_hMutGetAddr);
	return TRUE;
}

//烧录失败后记录当前的值
void Burn_Fail_setcurrent(TCHAR *serialtempbuf, TCHAR *mactempbuf, UINT channelID)
{
	if(WaitForSingleObject(theConfig.m_hMutGetAddr, 5000) != WAIT_OBJECT_0)
	{
		ReleaseMutex(theConfig.m_hMutGetAddr);
		return;
	}

	if (mactempbuf != NULL && theConfig.g_mac_current_low[channelID][0] == NULL)
	{
		//烧录失败时记录此通过的值
		_tcscpy(theConfig.g_mac_current_low[channelID], mactempbuf);
	}
	if (serialtempbuf != NULL && theConfig.g_sequence_current_low[channelID][0] == NULL)
	{
		//烧录失败时记录此通过的值
		_tcscpy(theConfig.g_sequence_current_low[channelID], serialtempbuf);
	}


	ReleaseMutex(theConfig.m_hMutGetAddr);

}

//烧录失败
void Burn_Fail(TCHAR *serialtempbuf, TCHAR *mactempbuf, UINT nID, BOOL readSerialflag, BOOL readmacflag)
{
	if(!theConfig.bUpdate && (theConfig.sequenceaddr_flag || theConfig.macaddr_flag) && !readmacflag && !readSerialflag)
	{
		Burn_Fail_setcurrent(serialtempbuf, mactempbuf, nID-1);
	}
	
	if(!theConfig.bUpdate && theConfig.sequenceaddr_flag && !readSerialflag) 
	{
		//烧录失败时记录此通过的值到文档上
		theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, passN, serialtempbuf, nID-1);
	}
	if(!theConfig.bUpdate && theConfig.macaddr_flag && !readmacflag) 
	{
		//烧录失败时记录此通过的值到文档上
		theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, passN, mactempbuf, nID-1);
	}
	fNand_Free_BadBlockBuf(nID);
	USB_attachflag = FALSE;
	g_download_mtd_flag[nID-1] = 0;
}

//判断MACADDR是否有效
BOOL Macaddr_isunuse(char *buf, int len)
{
	int str_len = 0;
	int i = 0;
	int flag = 0;
	CHAR temphuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	USES_CONVERSION;

	memcpy(temphuf, buf, 8);

	//比较高位是是否相等
	if(memcmp(temphuf, T2A(theConfig.mac_start_high), 8) != 0)
	{
		return FALSE;
	}

	str_len = strlen((char *)buf);
	if (str_len != len)
	{
		return FALSE;
	}
	else
	{		
		for (i = 0; i < len; i++)
		{
			//mac地址中这几位必需是":"
			if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14)
			{
				if (buf[i] != ':')
				{
					return FALSE;
				}
			}
			else
			{
				//mac地址必需是数字，否则出错
				if (!isalnum(buf[i]))
				{
					return FALSE;
				}
			}
			
		}
	}

	return TRUE;
}

//判断序列号是否有效
BOOL Serialaddr_isunuse(char *buf, int len)
{

	int str_len = 0;
	int i = 0;
	CHAR temphuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	
	USES_CONVERSION;

	memcpy(temphuf, buf, 10);
	
	//比较
	if(memcmp(temphuf, T2A(theConfig.sequence_start_high), 10) != 0)
	{
		return FALSE;
	}

	str_len = strlen((char *)buf);
	if (str_len != len)
	{
		return FALSE;
	}
	else
	{
		for (i = 0; i < len; i++)
		{
			//序列号必需是数字，否则出错
			if (!isalnum(buf[i]))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

//如下是回读的函数
E_NANDERRORCODE Reset_Nand_ExReadFlag(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len)
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val = NF_FAIL;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;
	UCHAR *ptr;
	
	//如果已知是空闲块就直接拷贝，不需要再读
	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		tmpBlock = chip * nand->BlockPerPlane * nand->PlanePerChip + block;
		ptr = &m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN];
		if (memcmp(ptr, m_mtd_oob_invalid, MTD_OOB_LEN) != 0)
		{
			memcpy(oob, &m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], oob_len);
			return NF_SUCCESS;
		}
	}

	info.chip  = chip; //chip id
	info.block = block; //块
	info.page  = page;  //页
    info.oob_len = oob_len; // oob长度

	if (BT_MediumReadFlag(ID, oob, &info, oob_len, &ret_val, 1) != BT_SUCCESS)
	{
		return NF_FAIL;
	}
	//读出来的OOB
	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;
		memcpy(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], oob, oob_len);
	}

	return (E_NANDERRORCODE)ret_val;
}
                          
E_NANDERRORCODE Reset_Nand_ExReadSector(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 plane_num, T_U32 block, T_U32 page,T_U8 data[], T_U8* spare_tbl,T_U32 oob_len, T_U32 page_num)//SpareTbl将是包含MutiPlaneNum个T_MTDOOB缓冲的指针
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;     //chip id
	info.plane_num = plane_num;  //plane数
    info.block     = block;      // 块
	info.page      = page;       //页
	info.data      = data;        //BUF
	info.spare_tbl = spare_tbl;
	info.oob_len   = oob_len;
    info.page_num  = page_num;    //页数
	info.lib_type  = 1;           //1表示exnftl ， 0表示nftl

	tmpBlock = chip * nand->BlockPerPlane * nand->PlanePerChip + block;

	if (BT_MediumReadSector(ID, &info, nand->BytesPerSector * page_num, &ret_val) != BT_SUCCESS)
	{
		//如果读失败并不是空闲块就置0
		if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
		{
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, oob_len);
		}
		return NF_FAIL;
	}

	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		/***原则上置为无效，原因是因为可能会存在写入后会读取马上失败***/
		memcpy(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], spare_tbl, oob_len);
	}

	return (E_NANDERRORCODE)ret_val;
}
                              
E_NANDERRORCODE Reset_Nand_ExWriteSector(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 plane_num, T_U32 block, T_U32 page,const T_U8 data[], T_U8* spare_tbl,T_U32 oob_len, T_U32 page_num)    //SpareTbl将是包含MutiPlaneNum个T_MTDOOB缓冲的指针
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;         //chip id
	info.plane_num = plane_num;    //plane num
    info.block     = block;        //块
	info.page      = page;         //页
	info.data      = (BYTE *)data; //buf
	info.spare_tbl = spare_tbl;    //oob buf
	info.oob_len   = oob_len;      //oob len
    info.page_num  = page_num;     //页数
	info.lib_type  = 1;            //1表示exnftl ， 0表示nftl

	//
	tmpBlock = chip * nand->BlockPerPlane * nand->PlanePerChip + block;

	//如果有写入时，那么把缓冲区清0
	//memset(&m_pBuf_freeBlk[ID]+block*oob_len, 0, oob_len);

	if (BT_MediumWriteSector(ID, &info, nand->BytesPerSector * page_num, &ret_val) != BT_SUCCESS)
	{
		//如果写失败并不是空闲块就置0
		if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
		{
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, oob_len);
		}
		return NF_FAIL;
	}

	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		/***原则上置为无效，原因是因为可能会存在写入后会读取马上失败***/
		memcpy(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], spare_tbl, oob_len);
	}

	return (E_NANDERRORCODE)ret_val;
}
   
E_NANDERRORCODE Reset_Nand_ExEraseBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block)
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;  //chip id
	info.plane_num = plane_num;  //plane num 
    info.block     = block;      //块
	info.lib_type  = 1;        //1表示exnftl 0表示nftl


	if (BT_MediumEraseBlock(ID, &info, &ret_val, 1) != BT_SUCCESS)
	{
		//如果擦失败并不是空闲块就置0
		if (m_pBuf_freeBlk[ID-1] != NULL)
		{
			tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, MTD_OOB_LEN);
		}
		return NF_FAIL;
	}

	//判断是否空闲块
	if (m_pBuf_freeBlk[ID-1] != NULL)
	{
		tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;
		memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_VALID, MTD_OOB_LEN);
	}
	

	return (E_NANDERRORCODE)ret_val;
}

//建立缓冲区，进行把所有的block的oob个数读取出来
T_BOOL fNand_Get_FreeBlockBuf(UINT nID, T_U32 chip, T_U32 BlockNum, T_U32 nChipCnt, UINT StartBlock)
{
	UINT ooblen = MTD_OOB_LEN;
	UINT i = 0;
	UINT Blocklen = BlockNum * nChipCnt;
	UINT freebuflen = ooblen * Blocklen;
	UCHAR *ret_val = NULL;
	UINT  *ptr;
	T_MEDIUM_RW_SECTOR_INFO info = {0};
	
	info.chip  = 0;          //chip id
	info.block = StartBlock;  //块
	info.page  = 0;           //第0页
    info.oob_len = MTD_OOB_LEN; //oob 长度
	
	m_pBuf_freeBlk[nID-1] = (UCHAR *)malloc(freebuflen);
    if (AK_NULL == m_pBuf_freeBlk[nID-1])
    {
        return FALSE;
    }

	ret_val = (UCHAR *)malloc(Blocklen*4);
    if (NULL == ret_val)
    {
        return FALSE;
    }

	memset(m_pBuf_freeBlk[nID-1], BLOCK_PAGE0_FLAG_INVALID, freebuflen);
	memset(ret_val, 0, Blocklen*4);

	if (BT_Medium_Read_FreeBlockBuf(nID, m_pBuf_freeBlk[nID-1], &info, ooblen, ret_val, BlockNum * nChipCnt - StartBlock) != BT_SUCCESS)
	{
		free(m_pBuf_freeBlk[nID-1]);
		m_pBuf_freeBlk[nID-1] = NULL;
		free(ret_val);
		ret_val = NULL;
		return FALSE;
	}

	ptr = (UINT *)ret_val;
	for ( i = StartBlock; i < BlockNum * nChipCnt; i++)
	{
		//判断返回值是否成功
		if (ptr[i] != NF_SUCCESS)
		{
			memset((m_pBuf_freeBlk[nID-1] + i*ooblen), BLOCK_PAGE0_FLAG_INVALID, ooblen);
		}
	}

	free(ret_val);
	ret_val = NULL;
	
	return TRUE;
}

//释放内存
void fNand_Free_BadBlockBuf(UINT nID)
{

	if (m_pBuf_BadBlk[nID-1] != NULL)
	{
		free(m_pBuf_BadBlk[nID-1]);
		m_pBuf_BadBlk[nID-1] = NULL;
	}

	if (m_pBuf_freeBlk[nID-1] != NULL)
	{
		free(m_pBuf_freeBlk[nID-1]);
		m_pBuf_freeBlk[nID-1] = NULL;
	}
}

//建立缓冲区，进行把所有的bab block个数读取出来
T_BOOL fNand_Get_BadBlockBuf(UINT nID, T_U32 BlockNum, T_U32 nChipCnt, T_MODE_CONTROL *ModeCtrl)
{
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) 
		&& ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)) 
		&& (TRANSC_MEDIUM_NAND == ModeCtrl->eMedium || TRANSC_MEDIUM_SPI_NAND == ModeCtrl->eMedium))
	{
		UINT badbuflen = ((BlockNum * nChipCnt) >> 3);

		m_pBuf_BadBlk[nID-1] = (UCHAR *)malloc(badbuflen);
		if (AK_NULL == m_pBuf_BadBlk[nID-1])
		{
			return FALSE;
		}
		memset(m_pBuf_BadBlk[nID-1], 0, badbuflen);
		
		if (BT_Medium_Get_BadBlockBuf(nID, m_pBuf_BadBlk[nID-1],  badbuflen) != BT_SUCCESS)
		{
			return FALSE;
		}
	}
	
	return TRUE;
}


T_BOOL Reset_fNand_ExIsBadBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block )
{
	//T_MEDIUM_RW_SECTOR_INFO info = {0};
	T_U32 byte_loc, byte_offset;
	UINT ID = (UINT)nand->BufStart[0];  //哪一个通道
/*
	info.chip      = chip;
    info.block     = block;

	if (BT_MediumIsBadBlock((UINT)nand->BufStart[0], &info, &ret_val) != BT_SUCCESS)
	{
		return FALSE;
	}
*/
	if(AK_NULL != m_pBuf_BadBlk[ID-1])
	{
		byte_loc = block / 8;
		byte_offset = 7 - block % 8;
		
		if(m_pBuf_BadBlk[ID-1][byte_loc] & (1 << byte_offset))
		{
			return TRUE;
		}
	}

	return FALSE;
}

T_BOOL Reset_fNand_ExSetBadBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block)
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	T_U32 byte_loc, byte_offset;
	UINT ID = (UINT)nand->BufStart[0];

	info.chip      = chip;  // chip id
    info.block     = block; // 块

	if (BT_MediumSetBadBlock(ID, &info, &ret_val) != BT_SUCCESS)
	{
		return FALSE;
	}
	
	//记录坏块到m_pBuf_BadBlk下
	if(AK_NULL != m_pBuf_BadBlk[ID-1])
    {
        byte_loc = block / 8;
        byte_offset = 7 - block % 8;
        m_pBuf_BadBlk[ID-1][byte_loc] |= 1 << byte_offset;
    }
	

	return ret_val;
}
/******************************************************************
NFTL 专用的函数
******************************************************************/
E_NANDERRORCODE Reset_Nand_WriteSector(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page,const T_U8 data[], T_U8* spare_tbl,T_U32 oob_len)    //SpareTbl将是包含MutiPlaneNum个T_MTDOOB缓冲的指针
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;          //chip id
//	info.plane_num = plane_num;
    info.block     = block;         //块
	info.page      = page;          //页
	info.data      = (BYTE *)data;  //data buf
	info.spare_tbl = spare_tbl;  //oob buf
	info.oob_len   = oob_len;   //oob len
//    info.page_num  = page_num;
	info.lib_type  =0;

	tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;

	if (BT_MediumWriteSector(ID, &info, nand->BytesPerSector, &ret_val) != BT_SUCCESS)
	{
		//如果写失败的情况下，把这块设为无用
		if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
		{
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, oob_len);
		}
		return NF_FAIL;
	}
	
	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		/***原则上置为无效，原因是因为可能会存在写入后会读取马上失败***/
		memcpy(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], spare_tbl, oob_len);
	}

	return (E_NANDERRORCODE)ret_val;
}

E_NANDERRORCODE Reset_Nand_ReadSector(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page,T_U8 data[], T_U8* spare_tbl,T_U32 oob_len)//SpareTbl将是包含MutiPlaneNum个T_MTDOOB缓冲的指针
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;
//	info.plane_num = plane_num;
    info.block     = block;
	info.page      = page;
	info.data      = data;
	info.spare_tbl = spare_tbl;
	info.oob_len   = oob_len;
//    info.page_num  = page_num;
	info.lib_type  =0;    //1表示exnftl ， 0表示nftl

	tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;

	if (BT_MediumReadSector((UINT)nand->BufStart[0], &info, nand->BytesPerSector, &ret_val) != BT_SUCCESS)
	{
		//如果读失败的情况下，把这块设为无用
		if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
		{
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, oob_len);
		}
		return NF_FAIL;
	}

	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		/***原则上置为无效，原因是因为可能会存在写入后会读取马上失败***/
		memcpy(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], spare_tbl, oob_len);
	}

	return (E_NANDERRORCODE)ret_val;
}

E_NANDERRORCODE Reset_Nand_EraseBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block)
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;
	//info.plane_num = 1;
    info.block     = block;
	info.lib_type  =0;  //1表示exnftl ， 0表示nftl

	

	if (BT_MediumEraseBlock(ID, &info, &ret_val, 1) != BT_SUCCESS)
	{
		//如果擦失败的情况下，把这块设为无用
		if (m_pBuf_freeBlk[ID-1] != NULL)
		{
			tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, MTD_OOB_LEN);
		}
		return NF_FAIL;
	}

	//如果擦成功的情况下，把这块设为可用
	if (m_pBuf_freeBlk[ID-1] != NULL)
	{
		tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;
		memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_VALID, MTD_OOB_LEN);
	}

	return (E_NANDERRORCODE)ret_val;
}


T_U32 Reset_Medium_ReadSector(T_PMEDIUM medium, T_U8* buf, T_U32 start, T_U32 size)
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	T_PC_MEDIUM *p_medium = (T_PC_MEDIUM *)((UCHAR *)medium+sizeof(T_MEDIUM));
	UINT ID = p_medium->ID;//(UINT)nand->BufStart[0];
//	UINT tmpBlock;
	
	//info.chip      = chip;
	//info.plane_num = plane_num;
   // info.block     = block;
	info.page      = start; // 页
	info.data      = buf;   //BUF
	//info.spare_tbl = spare_tbl;
	//info.oob_len   = oob_len;
    info.page_num  = size;   //页数
	//info.lib_type  = 1;
	

	if (BT_MediumReadSector(ID, &info, (1<<medium->SecBit) * size, &ret_val) != BT_SUCCESS)
	{
		return NF_FAIL;
	}
	
	return (T_U32)ret_val;
}

T_U32 Reset_Medium_WriteSector(T_PMEDIUM medium, const T_U8 *buf, T_U32 start, T_U32 size)
{
	
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	T_PC_MEDIUM *p_medium = (T_PC_MEDIUM *)((UCHAR *)medium+sizeof(T_MEDIUM));
	UINT ID = p_medium->ID; //(UINT)nand->BufStart[0];
	UINT max_idex = 0;
	UINT writelen = 0;
	UINT idex = 1;
	UINT temp_ret = 0;
	UINT ret_secnt = 0;
	UINT i = 0;
	BOOL flag = FALSE;//当扇区大于16，但又不是16的位数时
	//	UINT tmpBlock;
	
	//	info.chip      = chip;
	//	info.plane_num = plane_num;
    //info.block     = block;
	info.page      = start;        //页
	info.data      = (BYTE *)buf;   //BUF
	//	info.spare_tbl = spare_tbl;
	//info.oob_len   = oob_len;
    info.page_num  = size;           //页数
	//info.lib_type  = 1;


	//if (BT_MediumWriteSector(ID, &info, (1<<medium->SecBit)*size, &ret_val) != BT_SUCCESS)
	if (BT_MediumWriteSector(ID, &info, (1<<medium->SecBit) * size, &ret_val) != BT_SUCCESS)
	{
		return NF_FAIL;
	}
	

	return (T_U32)ret_val;
}


T_VOID Reset_Medium_DeleteSec(T_PMEDIUM medium,T_U32 StartSce,T_U32 SecSize)
{
	return;
}
T_BOOL Reset_Medium_Flush(T_PMEDIUM medium)
{
	return AK_TRUE;
}

VOID Burn_ResetNandBase(UINT nID, T_PNANDFLASH nand)
{
	nand->BufStart[0]  = nID;/*目前用这位来替代通道，如果以后结构体删除，那么可以定义增加一个变量来替代*/
	nand->ExReadFlag   = Reset_Nand_ExReadFlag;
	nand->ExRead       = Reset_Nand_ExReadSector;
	nand->ExWrite      = Reset_Nand_ExWriteSector;
	nand->ExEraseBlock = Reset_Nand_ExEraseBlock;
	nand->ExIsBadBlock = Reset_fNand_ExIsBadBlock;
	nand->ExSetBadBlock = Reset_fNand_ExSetBadBlock;
	/**由于低级格式化的时候是调用这两个接口所以就直接应用了**/
	nand->ReadFlag   = Reset_Nand_ExReadFlag;
	nand->ReadSector   = Reset_Nand_ReadSector;
	nand->WriteSector  = Reset_Nand_WriteSector;
	nand->EraseBlock   = Reset_Nand_EraseBlock;
	nand->IsBadBlock   = Reset_fNand_ExIsBadBlock;
	nand->SetBadBlock  = Reset_fNand_ExSetBadBlock;
	
}
/*
VOID Burn_ResetMedium(UINT nID, T_PMEDIUM medium)
{
	medium->read = Reset_Medium_ReadSector;
	medium->write = Reset_Medium_WriteSector;
	medium->DeleteSec = AK_NULL;
	medium->flush = Reset_Medium_Flush;
}
*/

T_VOID Burn_Free_Medium(T_PMEDIUM medium)
{
	if (medium != AK_NULL)
	{
		free((T_PMEDIUM)medium);
		medium = NULL;
	}
}

T_PMEDIUM Burn_Malloc_Medium(UINT ID, UINT secsize, UINT capacity, UINT SecPerPg)
{
    T_PMEDIUM medium = NULL;
	T_PC_MEDIUM *pc_medium = NULL;
    int i = 0;
    
    medium = (T_PMEDIUM)malloc(sizeof(T_MEDIUM) + sizeof(T_PC_MEDIUM));
    if (medium == NULL)
    {
        return NULL;
    }

	pc_medium = (T_PC_MEDIUM *)((UCHAR *)medium + sizeof(T_MEDIUM));
	pc_medium->medium = medium;
	pc_medium->ID = ID;

    memset(medium,0,sizeof(T_MEDIUM));

    
    i = 0;
    while (secsize > 1)
    { 
        secsize >>= 1;
        i++;
    }
    
    medium->SecBit =(T_U8) i;
    
    i = 0;
    while (SecPerPg > 1)
    {
        SecPerPg >>= 1;
        i++;
    }
    medium->SecPerPg = i;	
    medium->PageBit =(T_U8) (i + medium->SecBit);
    //((T_POBJECT)media)->destroy = NULL;//(F_DESTROY)Medium_Destroy;
    //((T_POBJECT)media)->type = TYPE_MEDIUM;
    medium->read = Reset_Medium_ReadSector;
    medium->write = Reset_Medium_WriteSector;	
    medium->flush = Reset_Medium_Flush;
    medium->DeleteSec = NULL;
    
    medium->capacity = capacity;    //扇区为单位
    medium->type = MEDIUM_SD;
    medium->msg = NULL;
   
	return (T_PMEDIUM)medium;
}
