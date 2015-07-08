/***
*Burn.h - This is the head file for Burn.
*
*       Copyright (c) 2000-2007, Anyka (GuangZhou) Software Technology Co., Ltd. All rights reserved.
*
*Purpose:
*       declare the CBurnBase class and Subclass.
*
* @author	Anyka
* @date		2007-10
* @version	1.0
*
*******************************************************************************/
#ifndef __BURN_H__
#define __BURN_H__

#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////////
//Macro
#define ON_BURNFLASH_MESSAGE		((WM_USER) + (100))
#define ON_BURNFLASH_DEVICE_ARRIVE	((WM_USER) + (101))
#define ON_BURNFLASH_DEVICE_REMOVE	((WM_USER) + (102))
#define ON_MODULE_BURN_MESSAGE		((WM_USER) + (103))
#define ON_MODULE_BURN_PROGRESS		((WM_USER) + (104))
#define ON_BURNFLASH_PROCESS		((WM_USER) + (105))

#define MAX_DOWNLOAD_NAND			50
#define MAX_DOWNLOAD_MTD			50
#define MAX_DOWNLOAD_FILES			1000

//////////////////////////////////////////////////////////////////////////
//Usual enum


typedef struct
{
	char diskName;
	UINT sectorCount;
    UINT sectorSize;
    UINT PageSize;
	BOOL bWritten;
	BOOL bCompare;
	UINT nID;
}T_DISK_INFO;

typedef struct
{
	T_DISK_INFO g_disk_info[16];
	UINT nID;
}T_nID_DISK_INFO;


typedef enum
{
	MESSAGE_SET_STANDBY  = 0,				//设置为准备状态
	MESSAGE_SET_UDISK_UPDATE_STANDBY,		//设置为准备状态
	MESSAGE_START_BURN,                     //开始烧录线程
	MESSAGE_RESET_STANDBY,					//设置为Reset状态
	MESSAGE_START_SET_REGISTER,				//设置寄存器消息

	MESSAGE_START_SET_CHANNELID,		    //设置通道的地址
	MESSAGE_START_CHANNELID_FAIL,		    //通道的地址错误

	MESSAGE_START_SET_RAMPARAM,             //开始设置ram参数
	MESSAGE_SET_RAMPARAM_FAIL,				//设置ram参数失败
	MESSAGE_SET_REGISTER_FAIL,				//设置寄存器失败
	MESSAGE_GET_REGVALUE_FAIL,				//获取寄存器值失败
	MESSAGE_DOWNLOAD_CHANNELID_FAIL,        //下载channel地址失败
	MESSAGE_SET_COMMODE_FAIL,               //设置串口类型失败
	MESSAGE_DOWNLOAD_COMADDR_FAIL,			//下载串口类型失败
	MESSAGE_SET_REGISTER_SUCCESS,			//设置寄存器成功
	MESSAGE_START_DOWNLOAD_LOADFILE,		//开始DownLoad文件
	MESSAGE_DOWNLOAD_LOADFILE_FAIL,			//DownLoad文件失败
	MESSAGE_DOWNLOAD_LOADFILE_SUCCESS,		//DownLoad文件成功
	MESSAGE_START_INIT_USB,					//开始初始化USB
	MESSAGE_INIT_USB_FAIL,					//初始化USB失败
	MESSAGE_INIT_USB_SUCCESS,				//初始化USB成功
	MESSAGE_START_TEST_TRANSC,				//开始测试通信
	MESSAGE_TEST_TRANSC_FAIL,				//测试通信失败
	MESSAGE_START_ERASE_TRANSC,				//开始擦除
	MESSAGE_ERASE_TRANSC_FAIL,				//擦除失败
	MESSAGE_START_FORMAT_TRANSC,			//开始格式化
	MESSAGE_FORMAT_TRANSC_FAIAL,			//格式化失败
	MESSAGE_START_DOWNLOAD_FILE,			//开始DownLoad文件
	MESSAGE_DOWNLOAD_FILE_FAIL,				//DownLoad文件失败
	MESSAGE_DOWNLOAD_FILE_SUCCESS,			//DownLoad文件成功
	MESSAGE_TASK_COMPLETE,					//下载任务完毕
	MESSAGE_DOWNLOAD_FILE_LEN,				//
	MESSAGE_DOWNLOADING_FILE,				//
	MESSAGE_START_DOWNLOAD_FAT_IMAGE,		//
	MESSAGE_DOWNLOAD_FAT_IMAGE_FAIL,		//	
	MESSAGE_DOWNLOADING_FAT_IMAGE,			//
	MESSAGE_DOWNLOAD_FAT_IMAGE_SUCCESS,		//
	MESSAGE_START_DOWNLOAD_RESOURCE,		//开始下载资源文件
	MESSAGE_DOWNLOAD_RESOURCE_SUCCESS,		//下载资源文件失败
	MESSAGE_DOWNLOAD_RESOURCE_FAIL,			//下载资源文件成功
	MESSAGE_COMPARE_FILE,					//开始比较文件
	MESSAGE_COMPARE_FILE_FAIL,				//比较文件失败
	MESSAGE_COMPARE_FILE_SUCCESS,			//比较文件成功
    MESSAGE_GET_CHIP_PARA,					//获取NandFlash芯片参数
    MESSAGE_GET_CHIP_PARA_FAIL,				//获取参数失败
    MESSAGE_GET_CHIP_PARA_SUCCESS,			//获取参数成功
    MESSAGE_SET_CHIP_PARA,					//设置NandFlash芯片参数
    MESSAGE_SET_CHIP_PARA_FAIL,				//设置参数失败
    MESSAGE_SET_CHIP_PARA_SUCCESS,			//设置参数成功
	MESSAGE_CHECK_FORMAT_DATA_FAIL,			//
	MESSAGE_BEGIN_GET_AID,                  //
	MESSAGE_GET_AID_FAIL,                   //
	MESSAGE_START_IMAGE_CREATE,             //正在制作镜像
	MESSAGE_IMAGE_CREATE_SUCCESS,           //镜像制作成功
	MESSAGE_IMAGE_CREATE_FAIL,              //镜像制作失败
	MESSAGE_IMAGE_CREATE_RESET,             //完成镜像制作后清空选框
	MESSAGE_START_MODULE_BURN,              //
	MESSAGE_MODULE_BURN_FAIL,               //
	MESSAGE_BASEBAND_GPIO_SETTING,
	MESSAGE_BASEBAND_GPIO_SETTING_FAIL,
	MESSAGE_SET_PARAM_TO_PRODUCER_FAIL,     //设置参数到producer失败
	MESSAGE_DOWNLOAD_PRODUCER_START,        //开始下载producer
	MESSAGE_DOWNLOAD_PRODUCER_FAIL,         //下载producer失败
	MESSAGE_DOWNLOAD_AND_TESTRAM_START,        //开始下载测试RAM
	MESSAGE_DOWNLOAD_AND_TESTRAM_FAIL,         //下载下载测试RAM失败
	MESSAGE_DOWNLOAD_PRODUCER_USB_FAIL,     //下载producer后usb连接失败
	MESSAGE_DOWNLOAD_PRODUCER_TIMEOUT_FAIL, //下载producer超时
	MESSAGE_DOWNLOAD_PRODUCER_SUCCESS,      //下载producer成功
	MESSAGE_DOWNLOAD_BIN_START,             //开始下载bin
	MESSAGE_DOWNLOAD_BIN_FAIL,              //下载bin失败
	MESSAGE_DOWNLOAD_BIN_SUCCESS,           //下载bin成功
	MESSAGE_DOWNLOAD_IMG_START,             //开始下载img
	MESSAGE_DOWNLOAD_IMG_FAIL,              //下载img失败
	MESSAGE_DOWNLOAD_IMG_SUCCESS,           //下载img成功
	MESSAGE_DOWNLOAD_BOOT_START,            //开始下载BOOT
	MESSAGE_DOWNLOAD_BOOT_FAIL,             //下载BOOT失败
	MESSAGE_DOWNLOAD_BOOT_SUCCESS,          //下载BOOT成功
	MESSAGE_PARTTION_INFORMATION_IS_NULL,   //分区信息为空
	MESSAGE_GET_MEDIUM_DATAINFO_FAIL,       //获取媒介数据信息失败
	MESSAGE_GET_FREE_BLOCK_FAIL,            //获取空闲块信息失败
	MESSAGE_LOW_FORMAT_START,				//开始低格
	MESSAGE_LOW_FORMAT_FAIL,				//低格失败
	MESSAGE_MALLOC_MEDIUM_FAIL,				//构造媒介失败

	MESSAGE_WRITE_MAC_ADDR_ASA_START,       //开始写mac地址入安全区
	MESSAGE_WRITE_MAC_ADDR_ASA_FAIL,        //写mac地址入安全区失败
	MESSAGE_WRITE_SERIAL_ADDR_ASA_START,    //开始写序列号地址入安全区
	MESSAGE_WRITE_SERIAL_ADDR_ASA_FAIL,     //写序列号地址入安全区失败


    MESSAGE_SET_MODE_START,                 //设置烧录模式
	MESSAGE_SET_MODE_FAIL,                  //设置烧录模式失败
	MESSAGE_SET_ERASEMODE_START,            //设置擦除模式
	MESSAGE_SET_ERASEMODE_FAIL,             //设置擦除模式失败
	MESSAGE_SET_NAND_GPIOCE_FAIL,           //设置nand gpioce 失败
	MESSAGE_SET_NAND_PARA_START,            //设置nand参数
	MESSAGE_SET_NAND_PARA_FAIL,             //设置nand参数失败
	MESSAGE_SET_SEC_AREA_START,             //设置安全区
	MESSAGE_SET_SEC_AREA_FAIL,              //设置安全区失败
	MESSAGE_BIN_UPLOAD_FAIL,                //bin回读失败
	MESSAGE_GET_BAD_BLOCK_FAIL,             //获取坏块信息失败

	MESSAGE_BURN_MAC_ADDR_READ,              //读安全区中的mac地址
	MESSAGE_BURN_MAC_ADDR_READ_ERROR,        //读安全区中的mac地址是无效的
	MESSAGE_BURN_SERIAL_ADDR_READ,           //读安全区中的序列号地址
	MESSAGE_BURN_SERIAL_ADDR_READ_ERROR,     //读安全区中的序列号地址是无效的

	MESSAGE_BURN_MAC_ADDR_COMPARE,          //烧录mac地址比较
	MESSAGE_BURN_MAC_ADDR_FAIL,             //烧录mac地址超出最大值
	MESSAGE_BURN_MAC_ADDR_SHOW,             //显示当前烧录的mac地址	
	MESSAGE_BURN_SERIAL_ADDR_COMPARE,       //烧录序列号地址比较
	MESSAGE_BURN_SERIAL_ADDR_FAIL,          //烧录序列号地址超出最大值
	MESSAGE_BURN_SERIAL_ADDR_SHOW,          //显示当前烧录的序列号地址
	
	MESSAGE_SET_RESV_AREA_START,            //设置保留区
	MESSAGE_SET_RESV_AREA_FAIL,             //设置保留区失败
	MESSAGE_CREATE_PARTITION_START,         //创建分区表
	MESSAGE_CREATE_PARTITION_FAIL,          //创建分区表失败
	
	MESSAGE_DOWNLOAD_CHANGE_CLK_START,      //下载变频小程序
	MESSAGE_DOWNLOAD_CHANGE_CLK_FAIL,       //下载变频小程序失败
	MESSAGE_DOWNLOAD_CHANGECLK_TIMEOUT_FAIL,    //下载变频小程序超时
	MESSAGE_DOWNLOAD_CHANGE_CLK_USB_FAIL,   //下载变频小程序usb连接失败
	MESSAGE_CLOSE_START,                    //终止烧录线程
	MESSAGE_CLOSE_FAIL,                     //终止烧录线程失败


	MESSAGE_SEND_CMD_ANYKA,                 //发送anyka命令到小机端，连接MASS boot
	MESSAGE_GET_HIGHID_FAIL,				//获取highID失败
	MESSAGE_GET_ALL_FREE_BLOCK,             //统计所以空闲块
	MESSAGE_MEDIUM_CAPACITY_FAIL,           //每个nand或sd的容量不一致
	MMESSAGE_MEDIUM_CAPACITY_CHECK,         //检测每个nand或sd的容量
	MESSAGE_UPLAOD_SPIFLASH_FAIL,           //spiflash回读失败
	MESSAGE_UPLOAD_SPIFLASH_START,          //正在回读spiflash的数据

}MESSAGE_TYPE;


typedef enum
{
	E_IMG_INIT,
	E_IMG_CREATING,
	E_IMG_SUCCESS,
	E_IMG_FAIL
}E_IMAGE_STATUS;

#endif