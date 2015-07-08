/**
 * @file burn_lib.h
 * @brief API of burn lib 
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Zhijun Liao
 * @date 
 * @version 1.0
 * @ref 
 */

#ifndef __BURN_TYPE_H__
#define __BURN_TYPE_H__

typedef enum
{
	MEMORY_TYPE_SDRAM = 0,		//SDRAM
	MEMORY_TYPE_SRAM ,			//SRAM
}E_MEMORY_TYPE;

typedef enum
{
	CHIP_3223 = 0,				//芯片型号：AK_3223
	CHIP_3224,					//芯片型号：AK_3224
	CHIP_3225,					//芯片型号：AK_3225
	CHIP_322L,					//芯片型号：AK_322L
	CHIP_36XX,					//芯片型号：AK_36XX
	CHIP_3631,					//芯片型号：AK_3631
	CHIP_RESERVER				//保留位
}E_CHIP_TYPE;

typedef enum
{
	NULL_TRANSC = 0x0,						//NULL事务
	CONN_TEST_TRANSC,						//测试传输事务
	ERASE_TRANSC,							//擦除事务
	FORMAT_TRANSC,							//格式化
	WRITE_DATA_TRANSC,						//写数据
	WRITE_FILE_TRANSC,						//写文件
	SET_REGISTER_TRANS,						//设置寄存器
	FAT_TRANS,								//格式化								
	COMPARE_TRANS,							//比较
	WRITE_CONFIG,							//写Config信息
	GET_CHIPPARA_TRANS,						//获取Chip参数
	SET_CHIPPARA_TRANS,						//设置Chip参数
	SET_INFOR_TRANS,						//set information
	RESET_USB_TRANS,						//Reset USB
}E_TRANSC_TYPE;

typedef struct
{
	E_MEMORY_TYPE	type;		//RAM类型
	UINT size;					//RAM大小
	UINT banks;					//RAM Banks
	UINT row;					//RAM row
	UINT column;				//RAM Column
	UINT control_addr;			//RAM Control register address
	UINT control_value;			//RAM Control value
}T_RAM_PARAM;


#pragma pack(1)

//NandFlash物理特性
typedef struct 
{
	UINT32  chip_id;			//芯片ID号
	USHORT  page_size;			//Page大小
	USHORT  page_per_blk;		//一个block的总Page数
	USHORT  blk_num;			//总block数目
	USHORT  group_blk_num;		//if size is 512M, A29 must the same when copy back, we thought the A29=0 is a group,A29=1 is another group
	USHORT  plane_blk_num;		//
	UCHAR   spare_size;			//spare size
	UCHAR   col_cycle;			//column address cycle
	UCHAR   lst_col_mask;		//last column address cycle mask bit
	UCHAR   row_cycle;			//row address cycle
	UCHAR   last_row_mask;		//last row address cycle mask bit
	UCHAR   custom_nd;			//if is a custom nandflash(other not the typic nandflash), set bit, use this would fast
    UINT32	flag;				//
	UINT32  cmd_len;			//nandflash command length
    UINT32  data_len;			//nandflash data length
	UCHAR   des_str[255];		//描述符
}T_NAND_PHY_INFO_TRANSC;

#pragma pack()


class IComm
{
public:
	virtual BOOL Read(BYTE buf[], int size, int *upload_count) = 0;
	virtual BOOL Write(BYTE buf[], int size) = 0;

	virtual BOOL write_transc_packet(BYTE transc_type, BYTE data[], int data_length) = 0;
	virtual BYTE read_transc_ack() = 0;
};

#endif