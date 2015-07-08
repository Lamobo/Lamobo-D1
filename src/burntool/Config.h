
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "transc.h"
#include "anyka_types.h"


//////////////////////////////////////////////////////////////////////////
//Macro
#define MAX_ERASE_COUNT			5
#define MAX_FORMAT_COUNT		20

#define MAX_MAC_SEQU_ADDR_COUNT 30

#define MAX_PROJECT_NAME		64
#define MAX_PASSWD_LENGTH		20

#define MAX_DEVICE_NUM			8

#define ERROR_CHIP_ID			0xFFFFFFFF

#define INVALID_GPIO			0xFE

#define NANDBOOT_CMDLINE_POS	72
#define CMDLINE_LEN				128
#define CMDLINE_FLAG			"CMDL"

//#define MODE_SFLASH	 0
//#define MODE_NEWBURN 1
//#define MODE_UPDATE  2
//#define MODE_DEBUG	 3

#define RAM_TYPE_MCP           0
#define RAM_TYPE_SDR           1
#define RAM_TYPE_DDR16         2
#define RAM_TYPE_DDR2_32       3
#define RAM_TYPE_DDR2_16       4
#define RAM_TYPE_DDR32         5
#define RAM_TYPE_DDR_16_MOBILE 6
#define RAM_TYPE_DDR_32_MOBILE 7

#define BURNED_NONE      1
#define BURNED_RESET     2
#define BURNED_POWER_OFF 3


//此定义是针对10blue的producek中的MODE_DEBUG定为5而定义,主要是作为进行spi的bin回读功能用.
#define MODE_DEBUG       4  
//ram配置文件路径
#define RAM_CFG_DDR    ".\\ramconfig\\ramconfig_ddr.txt"
#define RAM_CFG_DDR2   ".\\ramconfig\\ramconfig_ddr2.txt"
#define RAM_CFG_mDDR   ".\\ramconfig\\ramconfig_m_ddr.txt"
#define RAM_CFG_MCP    ".\\ramconfig\\ramconfig_mcp.txt"
#define RAM_CFG_SDR    ".\\ramconfig\\ramconfig_sdram.txt"

//////////////////////////////////////////////////////////////////////////
//Usual Enum & Struct
typedef enum
{
	E_DOWNLOAD_AK_ONLY,
	E_DOWNLOAD_IFX_ONLY,
	E_DOWNLOAD_BOTH,
}
E_DOWNLOAD_MODE;

typedef enum
{
	E_CONFIG_NAND,
	E_CONFIG_SFLASH,
	E_CONFIG_SD,
	E_CONFIG_DEBUG,
	E_CONFIG_JTAG,
	E_CONFIG_SPI_NAND,
}
E_MODE_CONFIG_INDEX;


typedef enum
{
	DISK_TYPE_NORFLASH = 0,
	DISK_TYPE_NANDFLASH,	
	DISK_TYPE_U_DISK,
	DISK_TYPE_RESERVER,
}E_DISK_TYPE;

typedef enum
{
	MEMORY_TYPE_SDRAM = 0,		//SDRAM
	MEMORY_TYPE_SRAM ,			//SRAM
}E_MEMORY_TYPE;

typedef enum
{
	INIT_USB_GPIO = 0,			//初始化USB方式：GPIO
	INIT_USB_REGISTER,			//初始化USB方式：寄存器
}E_INIT_USB_TYPE;
/*
typedef enum
{
    MODE_NEWBURN = 1,
    MODE_UPDATE,
}E_BURN_MODE;
*/
typedef struct
{
	BOOL bCompare;
	BOOL  bBackup;
	TCHAR pc_path[MAX_PATH];
	UINT ld_addr;
	TCHAR file_name[MAX_PATH];
	FLOAT bin_revs_size;
}T_DOWNLOAD_NAND;

typedef struct
{
	BOOL bCompare;
	TCHAR pc_path[MAX_PATH];
	TCHAR udisk_path[MAX_PATH];
}T_DOWNLOAD_UDISK;

typedef struct
{
	BOOL bCompare;
	TCHAR pc_path[MAX_PATH];
	TCHAR disk_name[MAX_PATH];
}T_DOWNLOAD_MTD;

#define  maccurrentlow       _T("mac_current_low")
#define  sequencecurrentlow  _T("serial_current_low")

#pragma pack(1)
//NandFlash物理特性
typedef T_NAND_PHY_INFO T_NAND_PHY_INFO_TRANSC;
typedef T_SFLASH_PHY_INFO T_SFLASH_PHY_INFO_TRANSC;
//片选
typedef struct
{
	BYTE	b_chip_select_loop;
	BYTE	b_chip_select[4];
}T_CHIP_SELECT_TRANSC;
/*
//格式化
typedef struct 
{
	char Disk_Name;				//盘符名
    BOOL bOpenZone;				//
    UINT ProtectType;			//	
    UINT ZoneType;				//
	UINT Size;
}T_FORMAT_DATA;
*/
typedef struct 
{
    char Disk_Name;				//盘符名
	char volume_lable[12];
}T_VOLUME_LABLE;

//写数据
typedef struct 
{
	E_DISK_TYPE Disk_Type;		//
	int Start_Address;			//
	int End_Address;			//	
	int Backup_Start_Address;	//	
	int Backup_End_Address;		//	
	int Data_Length;			//
	int Backup_Map_Address;		//
	int map_index;
}T_DATA_WRITE_TRANSC;

//写文件
typedef struct 
{
	TCHAR File_Path[MAX_PATH+1];
	int File_length;
	int File_mode;
	int File_time;
}T_FILE_WRITE_TRANSC;

#pragma pack()

typedef struct 
{
	int main_version;
	int sub_version1;
	int sub_version2;
	int version_reserve;
}T_VERSION_DATA;

typedef struct
{
	TCHAR pm_password[MAX_PASSWD_LENGTH+1];	
	TCHAR rd_password[MAX_PASSWD_LENGTH+1];	
}T_SEC_CTRL;


typedef struct
{
	BYTE type;		//RAM类型
	UINT size;					//RAM大小
	UINT banks;					//RAM Banks
	UINT row;					//RAM row
	UINT column;				//RAM Column
	UINT control_addr;			//RAM Control register address
	UINT control_value;			//RAM Control value
}T_RAM_PARAM;

typedef struct
{
	int address;				//寄存器地址
	int value;					//寄存器值
}T_SET_REGISTER_DATA;

typedef struct 
{
	BOOL bCmdLine;
	UINT cmdAddr;
	char CmdData[CMDLINE_LEN+1];
}T_CMD_LINE;

class CConfig
{
public:
	CConfig();
	~CConfig();

public:
	void DefaultConfig();
	BOOL ReadConfig(LPCTSTR file_path);
	BOOL WriteConfig(LPCTSTR file_path);
	void Cansel_Config_init(void);

	BOOL StorePassword();
	BOOL GetPassword();

	UINT hex2int(const char *str);
//	TCHAR * ConverPathToAbsolute(TCHAR *path);

protected:
	BOOL get_download_udisk(UINT index, CString subRight);
	BOOL get_download_nand(UINT index, CString subRight);
	BOOL get_download_mtd(UINT index, CString subRight);

	BOOL get_format_data(UINT index, CString subRight);
	BOOL get_spi_format_data(UINT index, CString subRight); //linux spi
	BOOL get_nand_data(UINT index, CString subRight);
	BOOL get_spinand_data(UINT index, CString subRight);

	BOOL get_sflash_data(UINT index, CString subRight);
	BOOL get_burned_gpio(CString subRight);
    BOOL get_burned_rtc(CString subRight);
public:
	BOOL ConfigNandbootCmdline(BOOL bClear);
    BOOL write_config_addr(LPCTSTR file_path, TCHAR *pBufname, TCHAR *pBuffalg, TCHAR *pBufaddr, UINT channelID);
	BOOL read_config_addr(LPCTSTR file_path, TCHAR *pBufAddr, TCHAR *pGAddr, TCHAR *pCurAddr);
	BOOL read_currentdevicenum_addr(LPCTSTR file_path, TCHAR *pBufname, TCHAR *pBuffalg, TCHAR *pBufaddr, UINT channelID);
	VOID lower_to_upper(TCHAR *surbuf, TCHAR *dstbuf);
	VOID ChangeBurnMode(T_MODE_CONTROL *pModeCtrl);
	INT get_float_num(const char *str);
	INT float_num(const char *str);

	//bios version control
	T_VERSION_DATA version_ctrl;

	//project name
	TCHAR			project_name[MAX_PROJECT_NAME+1];

	//UI Control
	BOOL			bUI_ctrl;

	//Sec ctrl	
	T_SEC_CTRL		passwd_ctrl;

	//device number
	UINT			device_num;

	// com mode
	UINT			com_mode;

	//burn mode
	UINT			init_burn_mode;
	UINT			burn_mode;
	BOOL			init_bUpdate;
	BOOL			bUpdate;
	BOOL            init_bUDiskUpdate;
	BOOL            bUDiskUpdate;
	
	//com concerned parameter
	BOOL			bOpenCOM;
	UINT			com_base;
	UINT			com_count;
	UINT			com_baud_rate;
	
	// chip concerned parameter
	E_CHIP_TYPE		init_chip_type;
	E_CHIP_TYPE		chip_type;
	BOOL			bUboot;
//	BOOL			bAutoDownload;
//	UINT			chip_clock;
	BYTE			power_off_gpio;
	BOOL			init_bUsb2;
	BOOL			bUsb2;
	BOOL            bPLL_Frep_change;
	BOOL            init_bPLL_Frep_change;
	BOOL			init_bUpdateself;
	BOOL			bUpdateself;
	BOOL			bTest_RAM;
	UINT			chip_snowbirdsE;
	BYTE            *pCheckExport;

	//online make image
	BOOL			bonline_make_image;
	
	//MAC ADDR
	BOOL			fore_write_mac_addr;  //是否强制烧录的标志
	TCHAR			mac_start_high[MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			mac_start_low[MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			mac_end_high[MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			mac_end_low[MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			mac_current_high[MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			mac_current_low[MAX_MAC_SEQU_ADDR_COUNT+1];
	BOOL			macaddr_flag;   //是否烧录标志

	//SEQUENCE ADDR
	BOOL			fore_write_serial_addr; //是否强制烧录的标志
	TCHAR			sequence_start_high[MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			sequence_start_low[MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			sequence_end_high[MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			sequence_end_low[MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR	        sequence_current_high[MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			sequence_current_low[MAX_MAC_SEQU_ADDR_COUNT+1];

//	TCHAR			g_sequence_current_high[32][MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			g_sequence_current_low[32][MAX_MAC_SEQU_ADDR_COUNT+1];
//	TCHAR			g_mac_current_high[32][MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			g_mac_current_low[32][MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			g_mac_show_current_low[32][MAX_MAC_SEQU_ADDR_COUNT+1];
	TCHAR			g_sequence_show_current_low[32][MAX_MAC_SEQU_ADDR_COUNT+1];


	BOOL			sequenceaddr_flag; //是否烧录标志
	BOOL			ShowImageMaker_flag; //是否显示制作镜像菜单标志

	BYTE			gpio_pin;
    BYTE			gpio_pin_select;

	T_BURNED_PARAM  init_burned_param;
    T_BURNED_PARAM  burned_param;

	//burn planform
	UINT	planform_tpye;
	TCHAR	init_planform_name[MAX_PROJECT_NAME+1];
	TCHAR	planform_name[MAX_PROJECT_NAME+1];

	//nand concerned parameter
	T_CHIP_SELECT_DATA chip_select_ctrl;

	//freq
	UINT	m_freq;
	
	//memory concerned parameter
	T_RAM_PARAM     init_ram_param;
	T_RAM_PARAM		ram_param;

	//usb concerned
	E_INIT_USB_TYPE	init_usb;						//初始化USB
	UINT			init_usb_gpio_number;			//初始化USB需使用的GPIO口
	UINT			init_usb_register;				//初始化USB需使用的寄存器
	UINT			init_usb_register_bit;			//初始化USB需使用的寄存器的BIT位

	//path concerned
	TCHAR			path_module[MAX_PATH+1];
	TCHAR			path_producer[MAX_PATH+1];
	TCHAR			path_nandboot[MAX_PATH+1];
	TCHAR			path_nandboot_new[MAX_PATH+1];
	TCHAR			path_bios[MAX_PATH+1];


	TCHAR			udisk_info[MAX_PATH];           //由于11平台使用镜像文件不同，需要记录每个文件的盘符信息
	UINT            udiskfile_drivernum;

	//producer concerned
	UINT			producer_run_addr;

	//register concerned
	T_SET_REGISTER_DATA	*set_registers;				//寄存器设置列表

	//erase concerned ============= 可能不再需要了，暂保留
//	int				erase_count;
//	T_ERASE_DATA	*erase_data;

	//module burn concerned
	UINT			DownloadMode;
	BOOL			bDownloadFLS;
	BOOL			bDownloadEEP;
	UINT			baudrate_module_burn;
	UINT			gpio_dtr;
	UINT			gpio_module_igt;
	UINT			gpio_module_reset;
	TCHAR			path_fls[MAX_PATH+1];
	TCHAR			path_eep[MAX_PATH+1];

	//fat system start address
	UINT			fs_res_blocks;						//文件系统保留区
	UINT			nonfs_res_size;						//非文件系统保留区

	T_CMD_LINE      cmdLine;

    UINT            event_wait_time;
	UINT            PID;
	UINT            VID;
	//format concerned
	UINT			format_count;
	T_PARTION_INFO	*format_data;
	T_SPIFLASH_PARTION_INFO *spi_format_data; //此变量是专门为linux平台的spi分区烧录时使用的

	T_VOLUME_LABLE	*pVolumeLable;
    
	//download concern
	//download to nandflash
	UINT				download_nand_count;
	T_DOWNLOAD_NAND		*download_nand_data;
	//download to udisk
	UINT				download_udisk_count;
	T_DOWNLOAD_UDISK	*download_udisk_data;			
	//download to udisk
	UINT				download_mtd_count;
	T_DOWNLOAD_MTD		*download_mtd_data;
	
	UINT			file_download_total_len;

	//nand list	
	UINT				nandflash_parameter_count;		//NANDFLASH参数个数
	T_NAND_PHY_INFO_TRANSC *nandflash_parameter;			//NANDFLASH参数列表
	
	//spinand list	
	UINT				spi_nandflash_parameter_count;		//spi NANDFLASH参数个数
	T_NAND_PHY_INFO_TRANSC *spi_nandflash_parameter;			//spi NANDFLASH参数列表		


	UINT				spiflash_parameter_count;		//NANDFLASH参数个数
	T_SFLASH_PHY_INFO_TRANSC *spiflash_parameter;			//NANDFLASH参数列表		

    HANDLE          m_hMutex;
	HANDLE          m_hMutGetAddr;
};

#endif
