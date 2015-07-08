#include "StdAfx.h"
#include "BurnTool.h"
#include "Config.h"
#include "medium.h"
#include "TRANSC.h"
extern "C"
{
	#include "fha.h"
}

#define UNICODE_TXT_HEAD 0xFEFF

extern T_NAND_PHY_INFO_TRANSC SUPPORT_NAND_TABLE[];
extern CBurnToolApp theApp;

//对应的排列
//_T("AK_3224"), _T("AK_322L"), _T("AK_36XX"), _T("AK_780X"), _T("AK_880X"),  _T("AK_10X6"), _T("AK_3631"), 
//_T("AK_3671"), _T("AK_980X"), _T("AK_3671A"), _T("AK_1080A"), _T("AK_37XX"), _T("AK_11XX"), _T("AK_1080L"),_T("RESERVER"), NULL

TCHAR *g_strChipName[] = {
   _T("AK_3224"), _T("AK_322L"), _T("AK_36XX"), _T("AK_780X"), _T("AK_880X"),  _T("AK_10XX"), _T("AK_3631"), 
   _T("AK_3671"), _T("AK_980X"), _T("AK_3671A"), _T("AK_10XX"), _T("AK_37XX"), _T("AK_11XX"), _T("AK_10XX"),
   _T("AK_10XXC"), _T("AK_39XX"),_T("RESERVER"), NULL
};


#define SAFE_RELEASE(x) if(x != NULL) { delete[] x; x=NULL;}

//static TCHAR absolute_path[255];

CConfig::CConfig()
{
	DefaultConfig();
}

CConfig::~CConfig()
{
	SAFE_RELEASE(format_data);
	SAFE_RELEASE(spi_format_data);
	SAFE_RELEASE(download_nand_data);
	SAFE_RELEASE(download_udisk_data);
	SAFE_RELEASE(nandflash_parameter);
	SAFE_RELEASE(spi_nandflash_parameter);
}


//////////////////////////////////////////////////////////////////////////
//对于取消保存的一些初始值
void CConfig::Cansel_Config_init(void)
{
	_tcsncpy(init_planform_name, planform_name, MAX_PROJECT_NAME);
	
	init_chip_type = chip_type;   //芯片类型
	
	init_ram_param.type = ram_param.type;  //内存参数
	
	init_bUsb2 = bUsb2;             //usb 2.0 和1.1
	init_bPLL_Frep_change = bPLL_Frep_change;
	
	init_burn_mode = burn_mode;     //烧录模式
	
	init_bUpdate = bUpdate;         //是否升级
	
	init_bUDiskUpdate = bUDiskUpdate; //u盘烧录

	init_bUpdateself = bUpdateself;   //自升级
	
	init_burned_param.burned_mode = burned_param.burned_mode; //
	
	init_burned_param.bWakup = burned_param.bWakup;
}

//配置文件中的默认值
void CConfig::DefaultConfig()
{
	int i;
	CString sPath;
	int nPos;

	version_ctrl.main_version = 1;
	version_ctrl.sub_version1 = 13;
	version_ctrl.sub_version2 = 10;

	GetModuleFileName(NULL,sPath.GetBufferSetLength(MAX_PATH+1),MAX_PATH);
	sPath.ReleaseBuffer();
	nPos=sPath.ReverseFind ('\\');
    sPath=sPath.Left(nPos+1);
	//sprintf(path_module, "%s", sPath);
	_tcsncpy(path_module, sPath, MAX_PATH); 

	// set all the param to default value
	//the general setting page
	memset(project_name, 0, MAX_PROJECT_NAME+1); //工程名
	//strcpy(project_name, "Anyka");
	_tcscpy(project_name, _T("ASPEN"));

	memset(planform_name, 0, MAX_PROJECT_NAME+1);
	_tcscpy(planform_name, _T("RTOS"));    //平台类型
	planform_tpye = E_ROST_PLANFORM;


	_tcscpy(passwd_ctrl.pm_password, _T("anyka"));
	_tcscpy(passwd_ctrl.rd_password, _T("anyka")); //密码是anyka

	device_num = 3;                                //通道

	burn_mode = E_CONFIG_NAND;                     //烧录介质
	bUpdate = FALSE;                               //升级烧录
    bUDiskUpdate = FALSE;                          //U盘烧录选择
	bUpdateself = FALSE;                           //自升级选择
	bonline_make_image = FALSE;                    //在线制作镜像

	com_count = 1;                                  //波特率
	com_base = 1;
	com_baud_rate = 38400;

	producer_run_addr = 0x00800000;                  //produce 的运行地址

	memset(path_producer, 0, sizeof(path_producer));	
	memset(path_nandboot, 0, sizeof(path_nandboot));
	memset(path_nandboot_new, 0, sizeof(path_nandboot_new));
	memset(path_bios, 0, sizeof(path_bios));	

	_tcscpy(path_producer, _T("producer.bin"));            //produce
	_tcscpy(path_nandboot, _T("Nandboot_780x.bin"));       //nand boot
	_tcscpy(path_nandboot_new, _T("Nandboot_37XXL_new.bin"));
	_tcscpy(path_bios, _T("bios_t500_780x.bin"));          //bios


	//清0
	memset(mac_start_high, 0, sizeof(mac_start_high));
	memset(mac_start_low, 0, sizeof(mac_start_low));
	memset(mac_end_high, 0, sizeof(mac_end_high));
	memset(mac_end_low, 0, sizeof(mac_end_low));
	memset(mac_current_high, 0, sizeof(mac_current_high));
	memset(mac_current_low, 0, sizeof(mac_current_low));
	memset(sequence_start_high, 0, sizeof(sequence_start_high)); //序列号开始高位
	memset(sequence_start_low, 0, sizeof(sequence_start_low));   //序列号开始低位
	memset(sequence_end_high, 0, sizeof(sequence_end_high));     //序列号结束高位
	memset(sequence_end_low, 0, sizeof(sequence_end_low));       //序列号结束低位
	memset(sequence_current_high, 0, sizeof(sequence_current_high));//当前高位
	memset(sequence_current_low, 0, sizeof(sequence_current_low));  //当前低位
	memset(g_sequence_current_low, 0, sizeof(g_sequence_current_low));//全局当前低位，
	memset(g_mac_current_low, 0, sizeof(g_mac_current_low));          //全局MAC低位

	//附初始值	
	_tcscpy(mac_start_high, _T("01:02:03"));
	_tcscpy(mac_start_low, _T("00:00:00"));
	_tcscpy(mac_end_high, _T("01:02:03"));
	_tcscpy(mac_end_low, _T("FF:FF:FF"));
	_tcscpy(mac_current_high, mac_start_high);
	_tcscpy(mac_current_low,mac_start_low);
	_tcscpy(sequence_start_high, _T("AABBCCXXDD"));
	_tcscpy(sequence_start_low, _T("000000"));
	_tcscpy(sequence_end_high, _T("AABBCCXXDD"));
	_tcscpy(sequence_end_low, _T("999999"));
	_tcscpy(sequence_current_high, sequence_start_high);
	_tcscpy(sequence_current_low, sequence_start_low);

	macaddr_flag = FALSE;         //MAC标志
	sequenceaddr_flag = FALSE;    //序列号标志

	//区分是linux还是RTOS
	if (E_LINUX_PLANFORM == planform_tpye)
	{
		ShowImageMaker_flag = TRUE;
	}
	else 
	{
		ShowImageMaker_flag = FALSE;
	}
	fore_write_mac_addr = FALSE;  //强制烧录mac
	fore_write_serial_addr = FALSE; //强制烧录序列号
	
	//the hardware setting page
	chip_type = CHIP_10X6;//
	bUboot = TRUE;
//	bAutoDownload = FALSE;
	power_off_gpio = 0xFF;

    //chip_clock = 84;	
	bUsb2 = TRUE;
	bPLL_Frep_change = AK_FALSE;
	//bUpdateself = 0;
	chip_snowbirdsE = 0;

    m_hMutex = INVALID_HANDLE_VALUE;
	m_hMutGetAddr = INVALID_HANDLE_VALUE;

	//98/11/1080L//37: E101, 04D6
	//10BLUE : E301, 04D6
	
	PID = 0xE101;//58113;
	VID = 0x04D6;


	cmdLine.bCmdLine = TRUE;
	cmdLine.cmdAddr  = 0;
	memset(cmdLine.CmdData, 0, CMDLINE_LEN+1);

    chip_select_ctrl.chip_sel_num = 1;
    chip_select_ctrl.gpio_chip_2 = INVALID_GPIO;
    chip_select_ctrl.gpio_chip_3 = INVALID_GPIO;
    
    gpio_pin = INVALID_GPIO;
    gpio_pin_select = 0;//INVALID_GPIO;

    // burned param. add for linux
    burned_param.burned_mode = BURNED_NONE;
    burned_param.bGpio = TRUE;
    burned_param.pwr_gpio_param.level = 1;
    burned_param.pwr_gpio_param.num = INVALID_GPIO;
    burned_param.pwr_gpio_param.Pulldown = 1;
    burned_param.pwr_gpio_param.Pullup = 0;
    burned_param.bWakup = TRUE;
    burned_param.rtc_wakup_level = 1;

	m_freq = 124;

	ram_param.type = RAM_TYPE_SDR;
	ram_param.size = 32;
    ram_param.banks = 4;
    ram_param.row = 13;
    ram_param.column = 9;
	ram_param.control_addr = 0x200b0000;

	init_usb = INIT_USB_REGISTER;
	init_usb_gpio_number = 5;
	init_usb_register = 0x20063000;
	init_usb_register_bit = 0x1;

    event_wait_time = 30;
	//module burn setting
	DownloadMode = E_DOWNLOAD_AK_ONLY;
	bDownloadFLS = AK_TRUE;
	bDownloadEEP = AK_TRUE;
	baudrate_module_burn = 921600;
	gpio_dtr = 85;
	gpio_module_igt = 109;
	gpio_module_reset = 87;

	_tcscpy(path_fls, _T("LCG2.fls"));
	_tcscpy(path_eep, _T("LCG2.eep"));
	
	//the format setting page
	fs_res_blocks = 64;
	nonfs_res_size = 4;
    bTest_RAM = 0;
	
	//对分区的内存进行分配
	format_count = 5;
	format_data = new T_PARTION_INFO[format_count];
	spi_format_data = new T_SPIFLASH_PARTION_INFO[format_count]; //如下是专门为linux平台spi烧录分区时使用的信息
    pVolumeLable = new T_VOLUME_LABLE[format_count];
	memset(format_data, 0, sizeof(T_PARTION_INFO)*format_count);
	memset(spi_format_data, 0, sizeof(T_SPIFLASH_PARTION_INFO)*format_count); //如下是专门为linux平台spi烧录分区时使用的信息
    memset(pVolumeLable, 0, sizeof(T_VOLUME_LABLE)*format_count);

	format_data[0].Disk_Name = 'C';
	format_data[0].ProtectType = MEDIUM_PORTECT_LEVEL_NORMAL;
	format_data[0].Size = 20;
	format_data[0].bOpenZone = false;

	format_data[1].Disk_Name = 'D';
	format_data[1].ProtectType = MEDIUM_PORTECT_LEVEL_NORMAL;
	format_data[1].Size = 20;
	format_data[1].bOpenZone = false;

	format_data[2].Disk_Name = 'A';
	format_data[2].ProtectType = MEDIUM_PORTECT_LEVEL_NORMAL;
	format_data[2].Size = 10;
	format_data[2].bOpenZone = false;

	format_data[3].Disk_Name = 'E';
	format_data[3].ProtectType = MEDIUM_PORTECT_LEVEL_NORMAL;
	format_data[3].Size = 10;
	format_data[3].bOpenZone = false;

	format_data[4].Disk_Name = 'B';
	format_data[4].ProtectType = MEDIUM_PORTECT_LEVEL_NORMAL;
	format_data[4].Size = 4000;
	format_data[4].bOpenZone = true;
	
	//spi_format_data这结构体成员只用size ,其他成员在使用时会用format_data的成员
	spi_format_data[0].Size = 0.5;
	spi_format_data[1].Size = 10;
	spi_format_data[2].Size = 10;
	spi_format_data[3].Size = 20;

	//download setting page
	download_nand_count = 0;
	download_nand_data = NULL;

	download_udisk_count = 0;
	download_udisk_data = NULL;
	
	//init cansel
	Cansel_Config_init();

	//load nandlist from SUPPORT_NAND_TABLE
	i = 0;
	while(ERROR_CHIP_ID != SUPPORT_NAND_TABLE[i].chip_id)
	{
		i++;
	}
	//NAND参数
	nandflash_parameter_count = i;
	nandflash_parameter = new T_NAND_PHY_INFO_TRANSC[nandflash_parameter_count];

	memcpy(nandflash_parameter, SUPPORT_NAND_TABLE, 
		nandflash_parameter_count*sizeof(T_NAND_PHY_INFO_TRANSC));

	//spi NAND参数
	spi_nandflash_parameter_count = i;
	spi_nandflash_parameter = new T_NAND_PHY_INFO_TRANSC[spi_nandflash_parameter_count];
	
	memcpy(spi_nandflash_parameter, SUPPORT_NAND_TABLE, 
		spi_nandflash_parameter_count*sizeof(T_NAND_PHY_INFO_TRANSC));

	//spi参数
	spiflash_parameter_count = 0;
	spiflash_parameter = NULL;
    m_hMutGetAddr      = CreateMutex(NULL, FALSE, NULL);

}

//读取配置文件的参数
BOOL CConfig::ReadConfig(LPCTSTR file_path)
{
	CString str;
	int k;
	BOOL ret = TRUE;
	DWORD read_len = 1;
	TCHAR tempbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR surtempbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	//获取文件的属性
	if(0xFFFFFFFF == GetFileAttributes(theApp.ConvertAbsolutePath(file_path)))
	{
		return FALSE;
	}

	USES_CONVERSION;

	//打开配置文件
	HANDLE hFile = CreateFile(theApp.ConvertAbsolutePath(file_path), GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, 0, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

#ifdef _UNICODE
	USHORT head;
	ReadFile(hFile, &head, 2, &read_len, NULL);
#endif

	//进行一行一行读取数据
	while(read_len > 0)
	{
		int pos;
		CString subLeft, subRight;
		TCHAR ch = 0;
		TCHAR text[1024];
		int index = 0;

		while(read_len > 0 && ch != '\n')
		{
			ret = ReadFile(hFile, &ch, sizeof(TCHAR), &read_len, NULL);
			text[index++] = ch;
		}
		text[index] = 0;

		str = text;
		int len = str.GetLength();

		//discard the lines that is blank or begin with '#'
		str.TrimLeft();
		if(str.IsEmpty() || '#' == str[0])
		{
			continue;
		}

		pos = str.Find('=');

		subLeft = str.Left(pos);
		subRight = str.Right(str.GetLength() - pos - 1);

		subLeft.TrimLeft();
		subLeft.TrimRight();
		subRight.TrimLeft();
		subRight.TrimRight();

		//project name
		if(_T("project name") == subLeft)
		{
			_tcsncpy(project_name, subRight, MAX_PROJECT_NAME);
		}
		//device number
		else if(_T("device channel") == subLeft)
		{
			device_num = atoi(T2A(subRight));
		}


		//planform name
		else if(_T("planform name") == subLeft)
		{
			_tcsncpy(planform_name, subRight, MAX_PROJECT_NAME);
			if (_tcscmp(planform_name, A2T("LINUX")) == 0)
			{
				planform_tpye = E_LINUX_PLANFORM;
			}
			if (_tcscmp(planform_name, A2T("RTOS")) == 0)
			{
				planform_tpye = E_ROST_PLANFORM;
			}
		}
		//com
		else if(_T("com") == subLeft.Left(3))
		{
			if(_T("com bOpen") == subLeft)
			{
				bOpenCOM = atoi(T2A(subRight));
			}
			else if(_T("com base") == subLeft)
			{
				com_base = atoi(T2A(subRight));
			}
			else if(_T("com count") == subLeft)
			{
				com_count = atoi(T2A(subRight));
			}
			else if(_T("com baud rate") == subLeft)
			{
				com_baud_rate = atoi(T2A(subRight));
			}
		}
		//path
		else if(_T("path") == subLeft.Left(4))
		{
			if(_T("path producer") == subLeft)
			{
				_tcsncpy(path_producer, subRight, MAX_PATH);
			}
			else if(_T("path nandboot") == subLeft)
			{
				_tcsncpy(path_nandboot, subRight, MAX_PATH);
			}
			else if(_T("path nandboot new") == subLeft)
			{
				_tcsncpy(path_nandboot_new, subRight, MAX_PATH);
			}
//			else if(_T("path bios") == subLeft)
//			{
//				_tcsncpy(path_bios, subRight, MAX_PATH);
//			}
		}
		//produce run addr
		else if(_T("producer run addr") == subLeft)
		{
			producer_run_addr = hex2int(T2A(subRight));
		}
	
		//chip
		else if(_T("chip") == subLeft.Left(4))
		{
			if(_T("chip type") == subLeft)
			{
				chip_type = CHIP_RESERVER;
				
				int i = 0;
				while(g_strChipName[i])
				{
					if(g_strChipName[i] == subRight)
					{
						chip_type = (E_CHIP_TYPE)i;
						break;
					}

					i++;
				}
			}
			else if(_T("chip uboot") == subLeft)
			{
				bUboot = atoi(T2A(subRight));
			}
//			else if(_T("chip auto download") == subLeft)
//			{
//				bAutoDownload = atoi(T2A(subRight));
//			}
			else if(_T("chip clock") == subLeft)
			{
				m_freq = atoi(T2A(subRight));
			}
			else if(_T("chip power off gpio") == subLeft)
			{
				power_off_gpio = atoi(T2A(subRight));
			}
			else if(_T("chip usb2") == subLeft)
			{
				bUsb2 = atoi(T2A(subRight));  //2.0 or 1.1
			}
			else if(_T("chip pll_frep_change") == subLeft)
			{
				bPLL_Frep_change = atoi(T2A(subRight));  //2.0 or 1.1
			}
			else if(_T("chip mode") == subLeft)
			{
				burn_mode = atoi(T2A(subRight)); //nand sd,spi
			}
			else if(_T("chip update") == subLeft)
			{
				bUpdate = atoi(T2A(subRight));
			}
			else if(_T("chip updateself") == subLeft)
			{
				bUpdateself = atoi(T2A(subRight));
			}
			else if(_T("chip test ram") == subLeft)
			{
				bTest_RAM = atoi(T2A(subRight));
			}
            //#ifdef SUPPORT_LINUX
            else if(_T("chip udisk update") == subLeft)
			{
				//if (planform_tpye == E_LINUX_PLANFORM)
				{
					bUDiskUpdate = atoi(T2A(subRight));
				}
			}
            
            //#endif

            else if(_T("chip select number") == subLeft)
			{
				chip_select_ctrl.chip_sel_num = atoi(T2A(subRight));
			}
			else if(_T("chip select nand2") == subLeft)
			{
				chip_select_ctrl.gpio_chip_2 = atoi(T2A(subRight));
			}
			else if(_T("chip select nand3") == subLeft)
			{
				chip_select_ctrl.gpio_chip_3 = atoi(T2A(subRight));
			}
			else if(_T("chip select gpio") == subLeft)
			{
				gpio_pin_select = atoi(T2A(subRight));
			}
			else if(_T("chip gpio_pin") == subLeft)
			{
				gpio_pin = atoi(T2A(subRight));
			}
        }
        else if(_T("wait time") == subLeft)
        {
            event_wait_time = atoi(T2A(subRight));
        }
		else if(_T("USB PID") == subLeft)
        {
            PID = hex2int(T2A(subRight));  //pid,用于usb设备的个数
        }
		else if(_T("USB VID") == subLeft)
        {
            VID = hex2int(T2A(subRight));  //vid,用于usb设备的个数
        }

        else if(_T("burned mode") == subLeft)
            {
                burned_param.burned_mode = atoi(T2A(subRight));
            }
            else if (_T("burned gpio") == subLeft)
            {
                get_burned_gpio(subRight);
            }
            else if (_T("burned rtc") == subLeft)
            {
                get_burned_rtc(subRight);
            }
            
		//cmdline
		else if(_T("cmdline") == subLeft.Left(7))
		{
			if(_T("cmdline support") == subLeft)
			{
				//cmdLine.bCmdLine = (BOOL)atoi(T2A(subRight));
				cmdLine.bCmdLine = TRUE;
			}
			if(_T("cmdline address") == subLeft)
			{
				cmdLine.cmdAddr = hex2int(T2A(subRight));
			}
			else if(_T("cmdline data") == subLeft)
			{
				if (!subRight.IsEmpty())
				{
					strncpy(cmdLine.CmdData, T2A(subRight), CMDLINE_LEN);
				}
			}
		}
		//ram
		else if(_T("ram") == subLeft.Left(3))
		{
			if(_T("ram type") == subLeft)
			{
				ram_param.type = atoi(T2A(subRight));//内存类型
			}
			else if(_T("ram size") == subLeft)
			{
				ram_param.size = atoi(T2A(subRight));//大小
			}
			else if(_T("ram row") == subLeft)
			{
				ram_param.row = atoi(T2A(subRight));//行数
			}
			else if(_T("ram column") == subLeft)
			{
				ram_param.column = atoi(T2A(subRight));
			}
			else if(_T("ram bank") == subLeft)
			{
				ram_param.banks = atoi(T2A(subRight));
			}
/*			else if(_T("ram control address") == subLeft)
			{
				ram_param.control_addr = atoi(T2A(subRight));
			}
			else if(_T("ram control value") == subLeft)
			{
				ram_param.control_value = atoi(T2A(subRight));
			}*/
		}

		//MAC ADDR
		else if(_T("mac start high") == subLeft)//开始高位
		{
			//把读出mac start high地址转换出大写
			memset(tempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			memset(surtempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			_tcscpy(surtempbuf, subRight);
			lower_to_upper(surtempbuf, tempbuf);
			//记录开始mac地址的高位
			_tcscpy(mac_start_high, tempbuf);
			_tcscpy(mac_current_high, mac_start_high);
		}
		else if(_T("mac start low") == subLeft)//开始低位
		{
			//把读出mac start low地址转换出大写
			memset(tempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			memset(surtempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			_tcscpy(surtempbuf, subRight);
			lower_to_upper(surtempbuf, tempbuf);
			//记录开始mac地址的高位
			_tcscpy(mac_start_low, tempbuf);
			
		}
		else if(_T("mac end high") == subLeft)//结束的高位
		{
			//把读出mac end high地址转换出大写
			memset(tempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			memset(surtempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			_tcscpy(surtempbuf, subRight);
			lower_to_upper(surtempbuf, tempbuf);
			//记录结束mac地址的高位
			_tcscpy(mac_end_high, tempbuf);
		}
		else if(_T("mac end low") == subLeft)//MAC结束低位
		{
			//把读出mac end low地址转换出大写
			memset(tempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			memset(surtempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			_tcscpy(surtempbuf, subRight);
			lower_to_upper(surtempbuf, tempbuf);
			//记录结束mac地址的低位
			_tcscpy(mac_end_low, tempbuf);
		}
		else if(_T("mac addr flag") == subLeft)//MAC标志
		{
			macaddr_flag = atoi(T2A(subRight));
		}
		else if(_T("fore write mac addr") == subLeft)
		{
			fore_write_mac_addr = atoi(T2A(subRight));//强制写
		}
	
		/*
		else if(_T("mac current high") == subLeft)
		{
			_tcsncpy(mac_current_high, subRight, MAX_MAC_SEQU_ADDR_COUNT);
		}
		else if(_T("mac current low") == subLeft)
		{
			_tcsncpy(mac_current_low, subRight, MAX_MAC_SEQU_ADDR_COUNT);
		}
		*/
		
		//SEQUENCE ADDR
		else if(_T("serial start high") == subLeft)//序列号开始的高位
		{
			//把读出mac start low地址转换出大写
			memset(tempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			memset(surtempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			_tcscpy(surtempbuf, subRight);
			lower_to_upper(surtempbuf, tempbuf);

			_tcsncpy(sequence_start_high, tempbuf, MAX_MAC_SEQU_ADDR_COUNT);
			_tcsncpy(sequence_current_high, sequence_start_high, MAX_MAC_SEQU_ADDR_COUNT);
		}
		else if(_T("serial start low") == subLeft)//序列号开始低位
		{
			_tcsncpy(sequence_start_low, subRight, MAX_MAC_SEQU_ADDR_COUNT);
			
		}
		else if(_T("serial end high") == subLeft)//序列号结束的高位
		{
			memset(tempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			memset(surtempbuf, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
			_tcscpy(surtempbuf, subRight);
			lower_to_upper(surtempbuf, tempbuf);

			_tcsncpy(sequence_end_high, tempbuf, MAX_MAC_SEQU_ADDR_COUNT);
		}
		else if(_T("serial end low") == subLeft)//序列号的结束的低位
		{
			_tcsncpy(sequence_end_low, subRight, MAX_MAC_SEQU_ADDR_COUNT);
		}
		else if(_T("serial addr flag") == subLeft)//强制标志
		{
			sequenceaddr_flag = atoi(T2A(subRight));
		}
		else if(_T("fore write serial addr") == subLeft)
		{
			fore_write_serial_addr = atoi(T2A(subRight));
		}
		/*
		else if(_T("serial current high") == subLeft)
		{
			_tcsncpy(sequence_current_high, subRight, MAX_MAC_SEQU_ADDR_COUNT);
		}
		else if(_T("serial current low") == subLeft)
		{
			_tcsncpy(sequence_current_low, subRight, MAX_MAC_SEQU_ADDR_COUNT);
		}
		*/

		//online image make
		else if(_T("online make image") == subLeft)
		{
			bonline_make_image = (BOOL)atoi(T2A(subRight));
		}

		//module burn
		else if(_T("moduleburn") == subLeft.Left(10))
		{
			if(_T("moduleburn DownloadMode") == subLeft)
			{
				DownloadMode = atoi(T2A(subRight));
			}
			else if(_T("moduleburn bDownloadFLS") == subLeft)
			{
				bDownloadFLS = atoi(T2A(subRight));
			}
			else if(_T("moduleburn bDownloadEEP") == subLeft)
			{
				bDownloadEEP = atoi(T2A(subRight));
			}
			else if(_T("moduleburn baudrate") == subLeft)
			{
				baudrate_module_burn = atoi(T2A(subRight));
			}
			else if(_T("moduleburn gpio_dtr") == subLeft)
			{
				gpio_dtr = atoi(T2A(subRight));
			}
			else if(_T("moduleburn gpio_module_igt") == subLeft)
			{
				gpio_module_igt = atoi(T2A(subRight));
			}
			else if(_T("moduleburn gpio_module_reset") == subLeft)
			{
				gpio_module_reset = atoi(T2A(subRight));
			}
			else if(_T("moduleburn path_fls") == subLeft)
			{
				_tcsncpy(path_fls, subRight, MAX_PATH);
			}
			else if(_T("moduleburn path_eep") == subLeft)
			{
				_tcsncpy(path_eep, subRight, MAX_PATH);
			}
		}
		//fs
		else if(_T("fs") == subLeft.Left(2))
		{
			if(_T("fs reserver block") == subLeft)
			{
				fs_res_blocks = atoi(T2A(subRight));//以BLOCK为单位
			}
			else if(_T("fs nonfs reserve size") == subLeft)
			{
				nonfs_res_size = atoi(T2A(subRight));//以M为单位
			}
		}
		//format
		else if(_T("partition") == subLeft.Left(9))//分区
		{
			if(_T("partition count") == subLeft)//分区个数
			{
				format_count = atoi(T2A(subRight));//分区个数

				if(format_data)
				{
					delete[] format_data;
				}
				format_data = new T_PARTION_INFO[format_count];//分区空间
				memset(format_data, 0, format_count*sizeof(T_PARTION_INFO));//清0
				
				//如下是专门为linux平台spi烧录分区时使用的信息
				if(spi_format_data)
				{
					delete[] spi_format_data;
				}
				spi_format_data = new T_SPIFLASH_PARTION_INFO[format_count];
				memset(spi_format_data, 0, format_count*sizeof(T_SPIFLASH_PARTION_INFO));

			}
			else
			{
				str = subLeft.Mid(9);
				k = atoi(T2A(str));
				k--;
				if(k >= 0 && k < (int)format_count)
				{
					get_format_data(k, subRight);	
				}
			}
		}
        //format volume
		else if (_T("volume lable") == subLeft.Left(12))//volume
		{
			if(_T("volume lable count") == subLeft)
			{
				if (NULL != pVolumeLable)
				{
					delete[] pVolumeLable;
				}
				//卷标的内存分配
				pVolumeLable = new T_VOLUME_LABLE[format_count];
				memset(pVolumeLable, 0, format_count*sizeof(T_VOLUME_LABLE));
			}
			else
			{
				str = subLeft.Mid(12);
				k = atoi(T2A(str));
				k--;
				if(k >= 0 && k < (int)format_count)
				{
					memcpy(pVolumeLable[k].volume_lable, T2A(subRight.Left(11)), 11);
					//linux平台下的spi烧录时需要用spi_format_data
					if (planform_tpye == E_LINUX_PLANFORM && burn_mode == E_CONFIG_SFLASH)
					{
						if (spi_format_data != NULL)
						{
							pVolumeLable[k].Disk_Name = spi_format_data[k].Disk_Name;
						}
						else
						{
							pVolumeLable[k].Disk_Name = 'A';
						}
					}
					else
					{
						//其他平台
						if (format_data != NULL)
						{
							pVolumeLable[k].Disk_Name = format_data[k].Disk_Name;
						}
						else
						{
							pVolumeLable[k].Disk_Name = 'A';
						}
					}
				}
			}
		}        
		//dowload
		else if(_T("download") == subLeft.Left(8))
		{
			if(_T("download_to_udisk") == subLeft.Left(17))//udisk
			{
				int Len = subRight.GetLength();
				if(_T("download_to_udisk count") == subLeft)
				{
					download_udisk_count = atoi(T2A(subRight));
					
					if(download_udisk_data)
					{
						delete[] download_udisk_data;//如果非空，那么要进行删除内存
					}
					//u盘数据
					download_udisk_data = new T_DOWNLOAD_UDISK[download_udisk_count];
					memset(download_udisk_data, 0, download_udisk_count*sizeof(T_DOWNLOAD_UDISK));
				}else
				{
					str = subLeft.Mid(17);
					k = atoi(T2A(str));
					k--;
					if(k >= 0 && k < (int)download_udisk_count)
					{
						Len = subRight.GetLength();
						get_download_udisk(k, subRight);
					}
				}
			}
			else if(_T("download_to_nand") == subLeft.Left(16))//NANDFLASH
			{
				if(_T("download_to_nand count") == subLeft)
				{
					download_nand_count = atoi(T2A(subRight));
					
					if(download_nand_data)
					{
						delete[] download_nand_data;//如果非空，那么要进行删除内存
					}
					//nand flash数据
					download_nand_data = new T_DOWNLOAD_NAND[download_nand_count];
					memset(download_nand_data, 0, download_nand_count*sizeof(T_DOWNLOAD_NAND));
				}
				else
				{
					str = subLeft.Mid(16);
					k = atoi(T2A(str));
					k--;
					if(k >= 0 && k < (int)download_nand_count)
					{
						get_download_nand(k, subRight);
					}
				}
			}
			else if(_T("download_to_mtd") == subLeft.Left(15))//MTD
			{
				if(_T("download_to_mtd count") == subLeft)
				{
					download_mtd_count = atoi(T2A(subRight));
					
					if(download_mtd_data)
					{
						delete[] download_mtd_data;//如果非空，那么要进行删除内存
					}
					//mtd数据
					download_mtd_data = new T_DOWNLOAD_MTD[download_mtd_count];
					memset(download_mtd_data, 0, download_mtd_count*sizeof(T_DOWNLOAD_MTD));
				}
				else
				{
					str = subLeft.Mid(15);
					k = atoi(T2A(str));
					k--;
					if(k >= 0 && k < (int)download_mtd_count)
					{
						get_download_mtd(k, subRight);
					}
				}
			}
		}
		//nandflash
		else if(_T("nand") == subLeft.Left(4))
		{
			if(_T("nand supported count") == subLeft)
			{
				nandflash_parameter_count = atoi(T2A(subRight));

				if(nandflash_parameter)
				{
					delete[] nandflash_parameter;//如果非空，那么要进行删除内存
				}
				//nand参数的内存分配
				nandflash_parameter = new T_NAND_PHY_INFO_TRANSC[	nandflash_parameter_count];
				memset(nandflash_parameter, 0, 	nandflash_parameter_count*sizeof(T_NAND_PHY_INFO_TRANSC));
			}
			else
			{
				str = subLeft.Mid(4);
				str.TrimLeft();

				k = atoi(T2A(str));
				k--;
				//读取nand的参数的值
				if(k >= 0 && k < (int)nandflash_parameter_count)
				{
					get_nand_data(k, subRight);
				}
			}
		}
		//spi nandflash
		else if(_T("spinand") == subLeft.Left(7))
		{
			if(_T("spinand supported count") == subLeft)
			{
				spi_nandflash_parameter_count = atoi(T2A(subRight));
				
				if(spi_nandflash_parameter)
				{
					delete[] spi_nandflash_parameter;//如果非空，那么要进行删除内存
				}
				//nand参数的内存分配
				spi_nandflash_parameter = new T_NAND_PHY_INFO_TRANSC[spi_nandflash_parameter_count];
				memset(spi_nandflash_parameter, 0, 	spi_nandflash_parameter_count*sizeof(T_NAND_PHY_INFO_TRANSC));
			}
			else
			{
				str = subLeft.Mid(7);
				str.TrimLeft();
				
				k = atoi(T2A(str));
				k--;
				//读取nand的参数的值
				if(k >= 0 && k < (int)spi_nandflash_parameter_count)
				{
					get_spinand_data(k, subRight);
				}
			}
		}
		else if(_T("sflash") == subLeft.Left(6))
		{
			if(_T("sflash supported count") == subLeft)
			{
				spiflash_parameter_count = atoi(T2A(subRight));

				if(spiflash_parameter)
				{
					delete[] spiflash_parameter;//如果非空，那么要进行删除内存
				}
				//spi参数的内存分配
				spiflash_parameter = new T_SFLASH_PHY_INFO_TRANSC[spiflash_parameter_count];
				memset(spiflash_parameter, 0, 	spiflash_parameter_count*sizeof(T_SFLASH_PHY_INFO_TRANSC));
			}
			else
			{
				str = subLeft.Mid(6);
				str.TrimLeft();

				k = atoi(T2A(str));
				k--;
				//读取spi的参数的值
				if(k >= 0 && k < (int)spiflash_parameter_count)
				{
					get_sflash_data(k, subRight);
				}
			}			
		}

	}
	//在读config时先进行初始值
	Cansel_Config_init();

	CloseHandle(hFile);

	return TRUE;
}

//保存配置文件，
//在设完成，进行保存
//在关闭烧录工具时，进行保存
BOOL CConfig::WriteConfig(LPCTSTR file_path)
{
	CString str;
	UINT i;
	INT len = 0;
	char buf[12] = {0};

	//获取属性
	DWORD faConfig = GetFileAttributes(theApp.ConvertAbsolutePath(file_path)); 
	if(0xFFFFFFFF != faConfig)
	{
		faConfig &= ~FILE_ATTRIBUTE_READONLY;//如果文件是只读，需要设非只读
		faConfig &= ~FILE_ATTRIBUTE_SYSTEM;  //如果文件是系统，那么设非系统
		faConfig &= ~FILE_ATTRIBUTE_TEMPORARY;//如果存在临时，那么也要设非临时
		SetFileAttributes(theApp.ConvertAbsolutePath(file_path), faConfig);
	}

	//open config file
	CStdioFile *pFile;
	pFile = new CStdioFile(theApp.ConvertAbsolutePath(file_path), 
		CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
	if(NULL == pFile)
	{
		return FALSE;
	}

	USES_CONVERSION;

#ifdef _UNICODE
	USHORT head = UNICODE_TXT_HEAD;
	pFile->Write(&head, 2);
#endif
	
	//project name
	pFile->WriteString(_T("###Project Name\r\n"));
	str.Format(_T("project name = %s\r\n"), project_name);//项目名
	pFile->WriteString(str);
	pFile->WriteString(_T("\r\n"));

	//device num
	pFile->WriteString(_T("###Devie Number\r\n"));
	str.Format(_T("device channel = %d\r\n"), device_num);//通道
	pFile->WriteString(str);
	pFile->WriteString(_T("\r\n"));

	//planform name
	pFile->WriteString(_T("###planform name\r\n"));
	str.Format(_T("planform name = %s\r\n"), planform_name);//平台类型
	pFile->WriteString(str);
	pFile->WriteString(_T("\r\n"));

	//com
	pFile->WriteString(_T("###COM\r\n"));
	str.Format(_T("com bOpen = %d\r\n"), bOpenCOM);//串口
	pFile->WriteString(str);
	str.Format(_T("com base = %d\r\n"), com_base);//基率
	pFile->WriteString(str);
	str.Format(_T("com count = %d\r\n"), com_count);//串口个数
	pFile->WriteString(str);
	str.Format(_T("com baud rate = %d\r\n"), com_baud_rate);//波特率
	pFile->WriteString(str);
	pFile->WriteString(_T("\r\n"));

	//path
	str.Format(_T("path producer = %s\r\n"), path_producer);//路径
	pFile->WriteString(str);
	str.Format(_T("path nandboot = %s\r\n"), path_nandboot);//boot路径
	pFile->WriteString(str);
	str.Format(_T("path nandboot new = %s\r\n"), path_nandboot_new);
	pFile->WriteString(str);
//	str.Format(_T("path bios = %s\r\n"), path_bios);
//	pFile->WriteString(str);
	pFile->WriteString(_T("\r\n"));

	//producer run addr
	str.Format(_T("producer run addr = 0x%08x\r\n"), producer_run_addr);//produce运行地址
	pFile->WriteString(str);

	pFile->WriteString(_T("\r\n"));

	//chip
	str.Format(_T("chip type = %s\r\n"), g_strChipName[chip_type]);//芯片
	pFile->WriteString(str);
	str.Format(_T("chip uboot = %d\r\n"), bUboot);
	pFile->WriteString(str);
//	str.Format(_T("chip auto download = %d\r\n"), bAutoDownload);
//	pFile->WriteString(str);
	str.Format(_T("chip clock = %d\r\n"), m_freq);//频率
	pFile->WriteString(str);
	str.Format(_T("chip power off gpio = %d\r\n"), power_off_gpio);//gpio
	pFile->WriteString(str);
	str.Format(_T("chip usb2 = %d\r\n"), bUsb2);//usb2.0
	pFile->WriteString(str);
	str.Format(_T("chip pll_frep_change = %d\r\n"), bPLL_Frep_change);//change pll frep
	pFile->WriteString(str);
	str.Format(_T("chip mode = %d\r\n"), burn_mode);//烧录介质
	pFile->WriteString(str);
	str.Format(_T("chip update = %d\r\n"), bUpdate);//升级
	pFile->WriteString(str);
	
	str.Format(_T("chip test ram = %d\r\n"), bTest_RAM);//测试ram 
	pFile->WriteString(str);
	str.Format(_T("chip updateself = %d\r\n"), bUpdateself);//自升级
	pFile->WriteString(str);

	str.Format(_T("chip udisk update = %d\r\n"), bUDiskUpdate);//U盘烧录
	pFile->WriteString(str);

    str.Format(_T("chip select number = %d\r\n"), chip_select_ctrl.chip_sel_num);//chip个数
	pFile->WriteString(str);
	str.Format(_T("chip select nand2 = %d\r\n"), chip_select_ctrl.gpio_chip_2);//nand1
	pFile->WriteString(str);
	str.Format(_T("chip select nand3 = %d\r\n"), chip_select_ctrl.gpio_chip_3);//nand2
	pFile->WriteString(str);

	str.Format(_T("chip select gpio = %d\r\n"), gpio_pin_select);
	pFile->WriteString(str);
	str.Format(_T("chip gpio_pin = %d\r\n"), gpio_pin);
	pFile->WriteString(str);
	
	pFile->WriteString(_T("\r\n"));

    str.Format(_T("wait time = %d\r\n"), event_wait_time);//等待时间，如usb转换时的时间
    pFile->WriteString(str);
	str.Format(_T("USB PID = %04x\r\n"), PID);//pid
    pFile->WriteString(str);
	str.Format(_T("USB VID = %04x\r\n"), VID);//vid
    pFile->WriteString(str);

    str.Format(_T("burned mode = %d\r\n"), burned_param.burned_mode);//全新还是升级烧录
    pFile->WriteString(str);
    str.Format(_T("burned gpio = %d, %d, %d, %d, %d\r\n"), 
        burned_param.bGpio,
        burned_param.pwr_gpio_param.num,
        burned_param.pwr_gpio_param.level,
        burned_param.pwr_gpio_param.Pullup,
        burned_param.pwr_gpio_param.Pulldown);
    pFile->WriteString(str);
    str.Format(_T("burned rtc = %d, %d\r\n\r\n"), 
        burned_param.bWakup,
        burned_param.rtc_wakup_level);
    pFile->WriteString(str);

	//config cmdline
	str.Format(_T("cmdline support = %d\r\n"), cmdLine.bCmdLine);
	pFile->WriteString(str);
    
	str.Format(_T("cmdline address = 0x%x\r\n"), cmdLine.cmdAddr);
	pFile->WriteString(str);

    str.Format(_T("cmdline data = %s\r\n"), A2T(cmdLine.CmdData));
	pFile->WriteString(str);
    
	pFile->WriteString(_T("\r\n"));

	//ram
	str.Format(_T("ram type = %d\r\n"), ram_param.type);//内存的类型
	pFile->WriteString(str);
	str.Format(_T("ram size = %d\r\n"), ram_param.size);//大小
	pFile->WriteString(str);
	str.Format(_T("ram row = %d\r\n"), ram_param.row);//行数
	pFile->WriteString(str);
	str.Format(_T("ram column = %d\r\n"), ram_param.column);
	pFile->WriteString(str);
	str.Format(_T("ram bank = %d\r\n"), ram_param.banks);
	pFile->WriteString(str);
//	str.Format(_T("ram control address = 0x%x\r\n"), ram_param.control_addr);
//	pFile->WriteString(str);
//	str.Format(_T("ram control value = 0x%x\r\n"), ram_param.control_value);
//	pFile->WriteString(str);
	pFile->WriteString(_T("\r\n"));


	//MAC ADDR
	str.Format(_T("mac start high = %s\r\n"), mac_start_high);//mac高位开始
	pFile->WriteString(str);
	str.Format(_T("mac start low = %s\r\n"), mac_start_low);//mac低位开始
	pFile->WriteString(str);
	str.Format(_T("mac end high = %s\r\n"), mac_end_high);//mac高位结束
	pFile->WriteString(str);
	str.Format(_T("mac end low = %s\r\n"), mac_end_low);//mac低位开始
	pFile->WriteString(str);
	str.Format(_T("mac addr flag = %d\r\n"), macaddr_flag);//mac地址标志
	pFile->WriteString(str);
	str.Format(_T("fore write mac addr = %d\r\n"), fore_write_mac_addr);//强制写MAC标志
	pFile->WriteString(str);
	
	/*
	str.Format(_T("mac current high = %s\r\n"), mac_current_high);
	pFile->WriteString(str);
	str.Format(_T("mac current low = %s\r\n"), mac_current_low);
	pFile->WriteString(str);
	*/
	pFile->WriteString(_T("\r\n"));
	
	//SEQUENCE ADDR
	str.Format(_T("serial start high = %s\r\n"), sequence_start_high);
	pFile->WriteString(str);
	str.Format(_T("serial start low = %s\r\n"), sequence_start_low);
	pFile->WriteString(str);
	str.Format(_T("serial end high = %s\r\n"), sequence_end_high);
	pFile->WriteString(str);
	str.Format(_T("serial end low = %s\r\n"), sequence_end_low);
	pFile->WriteString(str);
	str.Format(_T("serial addr flag = %d\r\n"), sequenceaddr_flag);
	pFile->WriteString(str);
	str.Format(_T("fore write serial addr = %d\r\n"), fore_write_serial_addr);
	pFile->WriteString(str);
	/*
	str.Format(_T("serial current high = %s\r\n"), sequence_current_high);
	pFile->WriteString(str);
	str.Format(_T("serial current low = %s\r\n"), sequence_current_low);
	pFile->WriteString(str);
	*/
	pFile->WriteString(_T("\r\n"));


	//online image make
	str.Format(_T("online make image = %d\r\n"), bonline_make_image);
	pFile->WriteString(str);
	pFile->WriteString(_T("\r\n"));

	//module burn
	str.Format(_T("moduleburn DownloadMode = %d\r\n"), DownloadMode);
	pFile->WriteString(str);
	str.Format(_T("moduleburn bDownloadFLS = %d\r\n"), bDownloadFLS);
	pFile->WriteString(str);
	str.Format(_T("moduleburn bDownloadEEP = %d\r\n"), bDownloadEEP);
	pFile->WriteString(str);
	str.Format(_T("moduleburn baudrate = %d\r\n"), baudrate_module_burn);
	pFile->WriteString(str);
	str.Format(_T("moduleburn gpio_dtr = %d\r\n"), gpio_dtr);
	pFile->WriteString(str);
	str.Format(_T("moduleburn gpio_module_igt = %d\r\n"), gpio_module_igt);
	pFile->WriteString(str);
	str.Format(_T("moduleburn gpio_module_reset = %d\r\n"), gpio_module_reset);
	pFile->WriteString(str);
	str.Format(_T("moduleburn path_fls = %s\r\n"), path_fls);
	pFile->WriteString(str);
	str.Format(_T("moduleburn path_eep = %s\r\n"), path_eep);
	pFile->WriteString(str);
	pFile->WriteString(_T("\r\n"));
	
	//fs
	str.Format(_T("fs reserver block = %d\r\n"), fs_res_blocks);
	pFile->WriteString(str);
	str.Format(_T("fs nonfs reserve size = %d\r\n"), nonfs_res_size);
	pFile->WriteString(str);
	pFile->WriteString(_T("\r\n"));

	//format
	str.Format(_T("partition count = %d\r\n"), format_count);
	pFile->WriteString(str);
	for(i = 0; i < format_count; i++)
	{
		char c = (0 == format_data[i].Disk_Name) ? ' ': format_data[i].Disk_Name;

		if (planform_tpye == E_LINUX_PLANFORM && burn_mode == E_CONFIG_SFLASH)
		{
			//此专门为linux平台的spi分区烧录使用
			str.Format(_T("%f"), spi_format_data[i].Size);
			//获取format_data[i].Size盘的空间大小的小数点的位数
			len = float_num(T2A(str));
			str.Format(_T("\tpartition%d = %c, %d, %d, %d, %.*f\r\n"), i+1, c,
				format_data[i].bOpenZone, format_data[i].ProtectType, 
				format_data[i].ZoneType, len, spi_format_data[i].Size);
		}
		else
		{
			str.Format(_T("\tpartition%d = %c, %d, %d, %d, %d\r\n"), i+1, c,
				format_data[i].bOpenZone, format_data[i].ProtectType, 
				format_data[i].ZoneType, format_data[i].Size);
		}
		pFile->WriteString(str);
	}
	pFile->WriteString(_T("\r\n"));


	str.Format(_T("volume lable count = %d\r\n"), format_count);
	pFile->WriteString(str);
	
	for (i=0; i < format_count; i++)
	{
		str.Format(_T("volume lable%d =  %s\r\n"), i+1, A2T(pVolumeLable[i].volume_lable));
		pFile->WriteString(str);
	}
	
	pFile->WriteString(_T("\r\n"));

	//download to udisk
	str.Format(_T("download_to_udisk count = %d\r\n"), download_udisk_count);
	pFile->WriteString(str);
	for(i = 0; i < download_udisk_count; i++)
	{
		str.Format(_T("download_to_udisk%d = %d, %s, %s\r\n"), i+1, 
			download_udisk_data[i].bCompare,
			download_udisk_data[i].pc_path,
			download_udisk_data[i].udisk_path
			);
		pFile->WriteString(str);
	}
	pFile->WriteString(_T("\r\n"));

	//downlaod to nandflash
	str.Format(_T("download_to_nand count = %d\r\n"), download_nand_count);
	pFile->WriteString(str);
	for(i = 0; i < download_nand_count; i++)
	{
		if (planform_tpye == E_LINUX_PLANFORM)
		{
			//此专门为linux平台的spi分区烧录使用
			str.Format(_T("%f"), download_nand_data[i].bin_revs_size);
			//获取format_data[i].Size盘的空间大小的小数点的位数
			len = float_num(T2A(str));
			
			str.Format(_T("download_to_nand%d = %d, %s, 0x%x, %s, %d, %.*f\r\n"), i+1, 
				download_nand_data[i].bCompare,
				download_nand_data[i].pc_path,
				download_nand_data[i].ld_addr,
				download_nand_data[i].file_name,
				download_nand_data[i].bBackup,
				len,
				download_nand_data[i].bin_revs_size
				);
		}
		else
		{
			str.Format(_T("download_to_nand%d = %d, %s, 0x%x, %s, %d, %d\r\n"), i+1, 
				download_nand_data[i].bCompare,
				download_nand_data[i].pc_path,
				download_nand_data[i].ld_addr,
				download_nand_data[i].file_name,
				download_nand_data[i].bBackup,
				download_nand_data[i].bin_revs_size
				);	
		}
		pFile->WriteString(str);
	}
	pFile->WriteString(_T("\r\n"));

	//downlaod to mtd
	str.Format(_T("download_to_mtd count = %d\r\n"), download_mtd_count);
	pFile->WriteString(str);
	for(i = 0; i < download_mtd_count; i++)
	{
		str.Format(_T("download_to_mtd%d = %d, %s, %s\r\n"), i+1, 
			download_mtd_data[i].bCompare,
			download_mtd_data[i].pc_path,
			download_mtd_data[i].disk_name
			);
		pFile->WriteString(str);
	}
	pFile->WriteString(_T("\r\n"));

	//nandflash
	str.Format(_T("nand supported count = %d\r\n"), nandflash_parameter_count);
	pFile->WriteString(str);
	for(i = 0; i < nandflash_parameter_count; i++)
	{
		str.Format(_T("nand%2d = 0x%x, %4d, %3d, %d, %d, %d, %3d, %d, %d, %d, %3d, 0x%x, 0x%x, 0x%x, 0x%x, %s\r\n"), 
			i+1,
			nandflash_parameter[i].chip_id,
			nandflash_parameter[i].page_size,
			nandflash_parameter[i].page_per_blk,
			nandflash_parameter[i].blk_num,
			nandflash_parameter[i].group_blk_num,
			nandflash_parameter[i].plane_blk_num,
			nandflash_parameter[i].spare_size,
			nandflash_parameter[i].col_cycle,
			nandflash_parameter[i].lst_col_mask,
			nandflash_parameter[i].row_cycle,
			nandflash_parameter[i].last_row_mask,
			nandflash_parameter[i].custom_nd,
			nandflash_parameter[i].flag,
			nandflash_parameter[i].cmd_len,
			nandflash_parameter[i].data_len,
			A2T(nandflash_parameter[i].des_str)
			);
		pFile->WriteString(str);
	}
	pFile->WriteString(_T("\r\n"));

	//spi nandflash
	str.Format(_T("spinand supported count = %d\r\n"), spi_nandflash_parameter_count);
	pFile->WriteString(str);
	for(i = 0; i < spi_nandflash_parameter_count; i++)
	{
		str.Format(_T("spinand%2d = 0x%x, %4d, %3d, %d, %d, %3d, 0x%x, 0x%x, %d, %s\r\n"), 
			i+1,
			spi_nandflash_parameter[i].chip_id,
			spi_nandflash_parameter[i].page_size,
			spi_nandflash_parameter[i].page_per_blk,
			spi_nandflash_parameter[i].blk_num,
			//spi_nandflash_parameter[i].group_blk_num,
			spi_nandflash_parameter[i].plane_blk_num,
			spi_nandflash_parameter[i].spare_size,
			//spi_nandflash_parameter[i].col_cycle,
			//spi_nandflash_parameter[i].lst_col_mask,
			//spi_nandflash_parameter[i].row_cycle,
			//spi_nandflash_parameter[i].last_row_mask,
			spi_nandflash_parameter[i].custom_nd,
			spi_nandflash_parameter[i].flag,
			//spi_nandflash_parameter[i].cmd_len,
			spi_nandflash_parameter[i].data_len,
			A2T(spi_nandflash_parameter[i].des_str)
			);
		pFile->WriteString(str);
	}
	pFile->WriteString(_T("\r\n"));

	//nandflash
	str.Format(_T("sflash supported count = %d\r\n"), spiflash_parameter_count);
	pFile->WriteString(str);
	for(i = 0; i < spiflash_parameter_count; i++)
	{
		str.Format(_T("sflash%2d = 0x%x, %4d, %4d, %3d, %d, %d, 0x%x, 0x%x, %s, %d\r\n"), 
			i+1,
			spiflash_parameter[i].chip_id,
			spiflash_parameter[i].total_size,
			spiflash_parameter[i].page_size,
			spiflash_parameter[i].program_size,
			spiflash_parameter[i].erase_size,
			spiflash_parameter[i].clock,			
			spiflash_parameter[i].flag,
			spiflash_parameter[i].protect_mask,
			A2T(spiflash_parameter[i].des_str),
			spiflash_parameter[i].reserved1
			);
		pFile->WriteString(str);
	}
	//写完参数到文件重设一些初始值
	Cansel_Config_init();

	pFile->Close();
	delete pFile;

	return TRUE;
}
//保存密码
BOOL CConfig::StorePassword()
{
	HANDLE hFile;
	char buf[1024];
	DWORD dwWrite;
	int i;

	//prepare data
	memcpy(buf, &passwd_ctrl, sizeof(passwd_ctrl));
	for(i = 0; i < 1024; i++)
	{
		buf[i] = buf[i] ^ (i*i + 5*i +1);
	}

	//write file
	hFile = CreateFile(theApp.ConvertAbsolutePath(_T("password.cfg")) , GENERIC_WRITE | GENERIC_READ , 
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL , 
		CREATE_ALWAYS , FILE_ATTRIBUTE_HIDDEN , NULL);

	if (hFile == INVALID_HANDLE_VALUE) 
	{
		return FALSE;
	}	
	//写密码到文档上
	if(!WriteFile(hFile,buf, 1024, &dwWrite, NULL))
	{
		CloseHandle(hFile);
		return FALSE;
	}

	CloseHandle(hFile);
	return TRUE;
}
//获取密码
BOOL CConfig::GetPassword()
{
	HANDLE hFile;
	char buf[1024];
	DWORD dwRead;
	int i;
    
	//fetch data
	hFile = CreateFile(theApp.ConvertAbsolutePath(_T("password.cfg")), GENERIC_READ , FILE_SHARE_READ , NULL , 
					OPEN_EXISTING , FILE_ATTRIBUTE_HIDDEN , NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	//读取密码
	if(!ReadFile(hFile, buf, 1024, &dwRead, NULL))
	{
		CloseHandle(hFile);
		return FALSE;
	}

	//extract password
	for(i = 0; i < 1024; i++)
	{
		buf[i] = buf[i] ^ (i*i + 5*i +1);
	}

	memcpy(&passwd_ctrl, buf, sizeof(passwd_ctrl));

	CloseHandle(hFile);
	return TRUE;
}

//获取小数点后的位数
int CConfig::float_num(const char *str)
{
    int i;
    int number=0; 
    TCHAR ch;
	BOOL flag = FALSE;
	
    for(i = strlen(str)-1; i >= 0; i--)
    {
		ch=str[i];
		//判断字符是否0
		if((ch == 48) && (flag == FALSE))//如果字符是48flag == FALSE，那么就继续
		{
			continue;
		}
		else if(ch == 46)//如果字符是46，那么就退出去
		{
			break;
		}
		else//否则加1
		{
			number++;
			flag = TRUE;
		}
    }
    return number;
}
/*
//获取浮点型实际的位数
int CConfig::get_float_num(const char *str)
{
    int i;
    int number=strlen(str); //获取字符的长度
    TCHAR ch;
	
    for(i = strlen(str)-1; i>=0; i--)//从尾开始比较
    {
		ch=str[i];
		//判断字符是否0
		if(ch == 48)//如果字符是48，那么就减1
		{
			number--;		
		}
		else if(ch == 46)//如果字符是46，那么就减1
		{
			number--;
			break;
		}
		else
		{
			break;
		}
    }
    return number;
}
*/
//u型转整型
UINT CConfig::hex2int(const char *str)
{
    int i;
    UINT number=0; 
    int order=1;
    TCHAR ch;

    for(i=strlen(str)-1;i>=0;i--)
    {
		ch=str[i];
		if(ch=='x' || ch=='X')break;
		
		if(ch>='0' && ch<='9')
		{
			number+=order*(ch-'0');
			order*=16;
		}
		if(ch>='A' && ch<='F')
		{
			number+=order*(ch-'A'+10);
			order*=16;
		}
		if(ch>='a' && ch<='f')
		{
			number+=order*(ch-'a'+10);
			order*=16;
		}
    }
    return number;
}

//下载到u盘的文件
BOOL CConfig::get_download_udisk(UINT index, CString subRight)
{
	CString str;
	int nPos;

	if(index >= download_udisk_count)
	{
		return FALSE;
	}

	USES_CONVERSION;

	//bCompare
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	download_udisk_data[index].bCompare = atoi(T2A(str));//是否比较

	//pc path
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	_tcsncpy(download_udisk_data[index].pc_path, str, MAX_PATH);//pc路径

	//udisk path
	subRight.TrimLeft();
	subRight.TrimRight();
	_tcsncpy(download_udisk_data[index].udisk_path, subRight, MAX_PATH);//U盘数据

	return TRUE;
}
//下载到闪存的数据
BOOL CConfig::get_download_nand(UINT index, CString subRight)
{
	CString str;
	int nPos;

	if(index >= download_nand_count)
	{
		return FALSE;
	}

	USES_CONVERSION;

	//bCompare
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	download_nand_data[index].bCompare = atoi(T2A(str));//是否比较

	//pc path
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	_tcsncpy(download_nand_data[index].pc_path, str, MAX_PATH);//pc的路径

	//ld addr
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	download_nand_data[index].ld_addr = hex2int(T2A(str));//连接地址

	//file name
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		//一此老客户中，config.txt文件没有备份这一项。
		subRight.TrimLeft();
		subRight.TrimRight();
		_tcsncpy(download_nand_data[index].file_name, subRight, MAX_PATH);

		//预防CONFIG文档没有这个值，在此初始化。
		download_nand_data[index].bBackup = 0;
		return TRUE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	_tcsncpy(download_nand_data[index].file_name, str, MAX_PATH);//文件名

	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		//一此老客户中，config.txt文件没有bin exctern size这一项。
		subRight.TrimLeft();
		subRight.TrimRight();
		download_nand_data[index].bBackup = atoi(T2A(subRight));
		
		//预防CONFIG文档没有这个值，在此初始化。
		download_nand_data[index].bin_revs_size = 0;
		return TRUE;
	}

	//bBackup
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	download_nand_data[index].bBackup = atoi(T2A(str));//bBackup

	//bin exctern size
	subRight.TrimLeft();
	subRight.TrimRight();
	download_nand_data[index].bin_revs_size = (FLOAT)atof(T2A(subRight)); //hex2int(T2A(subRight));//bin_revs_size


	return TRUE;
}
//获取mtd的下载数据
BOOL CConfig::get_download_mtd(UINT index, CString subRight)
{
	CString str;
	int nPos;

	if(index >= download_mtd_count)
	{
		return FALSE;
	}

	USES_CONVERSION;

	//bCompare
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	download_mtd_data[index].bCompare = atoi(T2A(str));//bCompare

	//pc path
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	_tcsncpy(download_mtd_data[index].pc_path, str, MAX_PATH);

	//backup end address
	subRight.TrimLeft();
	subRight.TrimRight();
	_tcsncpy(download_mtd_data[index].disk_name, subRight, MAX_PATH);

	return TRUE;
}
//获取gpio
BOOL CConfig::get_burned_gpio(CString subRight)
{
    CString str;
	int nPos;

    USES_CONVERSION;

    nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);
	str.TrimLeft();
	str.TrimRight();
	burned_param.bGpio = atoi(T2A(str));

    nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);
	str.TrimLeft();
	str.TrimRight();
	burned_param.pwr_gpio_param.num = atoi(T2A(str));//num

    nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);
	str.TrimLeft();
	str.TrimRight();
	burned_param.pwr_gpio_param.level = atoi(T2A(str));//level

    nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);
    str.TrimLeft();
	str.TrimRight();
	burned_param.pwr_gpio_param.Pullup = atoi(T2A(str));//Pullup

	subRight.TrimLeft();
	subRight.TrimRight();
	burned_param.pwr_gpio_param.Pulldown = atoi(T2A(subRight));//Pulldown
    // burned_param.pwr_gpio_param.Pulldown = (~burned_param.pwr_gpio_param.Pullup)&0x01;

    return TRUE;
}

BOOL CConfig::get_burned_rtc(CString subRight)
{
    CString str;
	int nPos;

    USES_CONVERSION;

    nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);
	str.TrimLeft();
	str.TrimRight();
	burned_param.bWakup = atoi(T2A(str));//bWakup
/*
    nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);*/
	subRight.TrimLeft();
	subRight.TrimRight();
	burned_param.rtc_wakup_level = atoi(T2A(subRight));//rtc_wakup_level

    return TRUE;
}
//获取格式化的数据
BOOL CConfig::get_format_data(UINT index, CString subRight)
{
	CString str;
	int nPos;

	//是否超出范围
	if(index >= format_count)
	{
		return FALSE;
	}

	USES_CONVERSION;

	//Disk Name
	nPos = subRight.Find(',');
	if(0 > nPos)
	{
		return FALSE;
	}
	else if(0 == nPos)
	{
		subRight = subRight.Mid(nPos+1);
		format_data[index].Disk_Name = ' ';
	}
	else
	{
		str = subRight.Left(nPos);
		subRight = subRight.Mid(nPos+1);

		str.TrimLeft();
		str.TrimRight();
		format_data[index].Disk_Name = (BYTE)str[0];//Disk_Name
	}

	//bOpenZone
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	format_data[index].bOpenZone = atoi(T2A(str));//bOpenZone

	//Protect type
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	format_data[index].ProtectType = atoi(T2A(str));//ProtectType

	//zone type
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	format_data[index].ZoneType = atoi(T2A(str));

	//Size
	subRight.TrimLeft();
	subRight.TrimRight();
	if (planform_tpye == E_LINUX_PLANFORM && burn_mode == E_CONFIG_SFLASH)
	{
		//如下是专门为linux平台spi烧录分区时使用的信息
		spi_format_data[index].Size = (FLOAT)atof(T2A(subRight));
	}
	else
	{
		//其他的
		format_data[index].Size = atoi(T2A(subRight));
	}
	

	return TRUE;
}

BOOL CConfig::get_nand_data(UINT index, CString subRight)
{
	CString str;
	int nPos;

	//判断是否超出范围
	if(index >= nandflash_parameter_count)
	{
		return FALSE;
	}

	USES_CONVERSION;
	
	//chip id
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].chip_id = hex2int(T2A(str));//chip_id


	//page size
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].page_size = atoi(T2A(str));//page_size

	//page per block
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].page_per_blk = atoi(T2A(str));//page_per_blk

	//block number
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].blk_num = atoi(T2A(str));//blk_num

	//group block number
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].group_blk_num = atoi(T2A(str));//group_blk_num

	//plane block number
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].plane_blk_num = atoi(T2A(str));//plane_blk_num

	//spare size
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].spare_size = atoi(T2A(str));//spare_size

	//column cycle
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].col_cycle = atoi(T2A(str));//col_cycle
	
	//last column mask
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].lst_col_mask = atoi(T2A(str));//lst_col_mask

	//row cycle
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].row_cycle = atoi(T2A(str));//row_cycle

	//last row mask
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].last_row_mask = atoi(T2A(str));//last_row_mask

	//custom nandflash
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].custom_nd = hex2int(T2A(str));//custom_nd

	//flag
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].flag = hex2int(T2A(str));//flag

	//command length
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].cmd_len = hex2int(T2A(str));//cmd_len

	//data length
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	nandflash_parameter[index].data_len = hex2int(T2A(str));//nand的data_len

	//description
	subRight.TrimLeft();
	subRight.TrimRight();
	strncpy(nandflash_parameter[index].des_str, T2A(subRight), 31);//nand的描述符

	return TRUE;
}

BOOL CConfig::get_spinand_data(UINT index, CString subRight)
{
	CString str;
	int nPos;

	//判断是否超出范围
	if(index >= spi_nandflash_parameter_count)
	{
		return FALSE;
	}

	USES_CONVERSION;
	
	//chip id
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].chip_id = hex2int(T2A(str));//chip_id


	//page size
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].page_size = atoi(T2A(str));//page_size

	//page per block
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].page_per_blk = atoi(T2A(str));//page_per_blk

	//block number
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].blk_num = atoi(T2A(str));//blk_num

	//group block number
	/*
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].group_blk_num = atoi(T2A(str));//group_blk_num
	*/

	//plane block number
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].plane_blk_num = atoi(T2A(str));//plane_blk_num

	//spare size
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].spare_size = atoi(T2A(str));//spare_size

	//column cycle
	/*
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].col_cycle = atoi(T2A(str));//col_cycle
	
	//last column mask
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].lst_col_mask = atoi(T2A(str));//lst_col_mask

	//row cycle
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].row_cycle = atoi(T2A(str));//row_cycle

	//last row mask
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].last_row_mask = atoi(T2A(str));//last_row_mask
	*/

	//custom nandflash
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].custom_nd = hex2int(T2A(str));//custom_nd

	//flag
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].flag = hex2int(T2A(str));//flag

	//command length
	/*
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].cmd_len = hex2int(T2A(str));//cmd_len
	*/

	//data length
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spi_nandflash_parameter[index].data_len = atoi(T2A(str));//nand的data_len

	//description
	subRight.TrimLeft();
	subRight.TrimRight();
	strncpy(spi_nandflash_parameter[index].des_str, T2A(subRight), 31);//nand的描述符

	return TRUE;
}


//获取spi的参数
BOOL CConfig::get_sflash_data(UINT index, CString subRight)
{
	CString str;
	int nPos;

	//判断是否越出界
	if(index >= spiflash_parameter_count)
	{
		return FALSE;
	}

	USES_CONVERSION;
	
	//chip id
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spiflash_parameter[index].chip_id = hex2int(T2A(str));//spi的参数的chip_id


	//total_size
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spiflash_parameter[index].total_size = atoi(T2A(str));//spi的参数的容量大小


	//page size
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spiflash_parameter[index].page_size = atoi(T2A(str));//spi的参数的页大小

	//program_size
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spiflash_parameter[index].program_size = atoi(T2A(str));//spi的参数的program_size

	//erase_size
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spiflash_parameter[index].erase_size = atoi(T2A(str));//spi的参数的块大小

	//clock
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spiflash_parameter[index].clock = atoi(T2A(str));//spi的参数的clock


	//flag
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spiflash_parameter[index].flag = hex2int(T2A(str));//flag

	//protect_mask
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		return FALSE;
	}

	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	str.TrimLeft();
	str.TrimRight();
	spiflash_parameter[index].protect_mask = hex2int(T2A(str));//spi的参数的protect_mask


	//protect_mask
	nPos = subRight.Find(',');
	if(nPos <= 0 )
	{
		subRight.TrimLeft();
		subRight.TrimRight();
		//description
		strncpy(spiflash_parameter[index].des_str, T2A(subRight), 31);//spi的参数的描述符

		//一此老客户中，config.txt文件没有bin exctern size这一项。
		spiflash_parameter[index].reserved1 = 0;//spi disk erase size，for linux
		return TRUE;
	}
	
	str = subRight.Left(nPos);
	subRight = subRight.Mid(nPos+1);

	//description
	str.TrimLeft();
	str.TrimRight();
	strncpy(spiflash_parameter[index].des_str, T2A(str), 31);//spi的参数的描述符

	subRight.TrimLeft();
	subRight.TrimRight();
	spiflash_parameter[index].reserved1 = atoi(T2A(subRight));////spi disk erase size，for linux


	return TRUE;
}




BOOL CConfig::ConfigNandbootCmdline(BOOL bClear)
{
	const UINT datalen = NANDBOOT_CMDLINE_POS + CMDLINE_LEN + 100;
    char *ptr = NULL;

	//打开nandboot的文件
    HANDLE hFile = CreateFile(theApp.ConvertAbsolutePath(path_nandboot), GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE , NULL , 
					OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);

	if(hFile != INVALID_HANDLE_VALUE)
	{
		BYTE buftmp[datalen];
		DWORD read_len;
		DWORD dwSize = GetFileSize(hFile, NULL);
		
		//判断长度是否有效
		if(0xFFFFFFFF == dwSize || dwSize < datalen)
		{
			CloseHandle(hFile);
			return FALSE;
		}
		//读一定长度的数据
		ReadFile(hFile, buftmp, datalen, &read_len, NULL);

        ptr = (char *)(buftmp+NANDBOOT_CMDLINE_POS);
		//判断读出来的数据
		if (0 == strncmp(ptr, CMDLINE_FLAG, 4))
		{
            ptr += 4;
            if (0 != strncmp(ptr, (char *)(&cmdLine.cmdAddr), 4)
                || 0 != strncmp(ptr + 4, cmdLine.CmdData, CMDLINE_LEN))
            {
			    SetFilePointer(hFile, NANDBOOT_CMDLINE_POS + 4, NULL, FILE_BEGIN);//偏移
			    /*
			    if (bClear)
			    {
				    memset(buftmp, 0, datalen);
				    WriteFile(hFile, buftmp, 4, &read_len, NULL);
				    WriteFile(hFile, buftmp, CMDLINE_LEN, &read_len, NULL);
			    }
			    else*/
			    {
					//回写到文件中
				    WriteFile(hFile, (LPVOID)(&cmdLine.cmdAddr), 4, &read_len, NULL);
				    WriteFile(hFile, cmdLine.CmdData, CMDLINE_LEN, &read_len, NULL);
			    }
            }
		}

		CloseHandle(hFile);
	}

	return TRUE;
}

//烧录介质的变更
VOID CConfig::ChangeBurnMode(T_MODE_CONTROL *pModeCtrl)	
{
	switch (burn_mode)
	{
	case E_CONFIG_NAND:
		pModeCtrl->eMedium = TRANSC_MEDIUM_NAND; //nand
		break;
	case E_CONFIG_SPI_NAND:
		pModeCtrl->eMedium = TRANSC_MEDIUM_SPI_NAND; //spi nand
		break;
	case E_CONFIG_SFLASH:
		pModeCtrl->eMedium = TRANSC_MEDIUM_SPIFLASH;//spi
		break;
	case E_CONFIG_SD:
		pModeCtrl->eMedium = TRANSC_MEDIUM_EMMC;//spi
		break;
	case E_CONFIG_DEBUG://调试用，如只下载produce
	case E_CONFIG_JTAG:
		break;
	}

	if (bUpdate)
	{
		pModeCtrl->burn_mode = MODE_UPDATE;//升级
	}
	else
	{
		pModeCtrl->burn_mode = MODE_NEWBURN;//全新烧录
	}
}

//记录在完成或失败时的序列号和mac地址
BOOL CConfig::write_config_addr(LPCTSTR file_path, TCHAR *pBufname, TCHAR *pBuffalg, TCHAR *pBufaddr, UINT channelID)
{
	TCHAR tmpBufFlag[256] = {0};
	TCHAR tmpBufAddr[256] = {0};
	
	swprintf(tmpBufFlag,_T("%s_%d_F"), pBufname, channelID);	
	//写标志
	WritePrivateProfileString(_T("config_addr"),tmpBufFlag, pBuffalg, theApp.ConvertAbsolutePath(file_path));

	swprintf(tmpBufAddr,_T("%s_%d_A"), pBufname, channelID);	
	//写值
	WritePrivateProfileString(_T("config_addr"),tmpBufAddr, pBufaddr, theApp.ConvertAbsolutePath(file_path));


	return TRUE;
}
//小写转成大小
VOID CConfig::lower_to_upper(TCHAR *surbuf, TCHAR *dstbuf)
{
	UINT i = 0;
	UINT nlen = 0;
	
	//获取字符串的长度
	while (surbuf[nlen] != NULL)
	{
		nlen++;
	}

	for (i = 0; i < nlen; i++)
	{
		if(islower(surbuf[i]))//如果是小写
		{
			dstbuf[i] = toupper(surbuf[i]); //转大
		}
		else
		{
			dstbuf[i] = surbuf[i]; //直接附值
		}
	}
}
//读当前通道的地址
BOOL CConfig::read_currentdevicenum_addr(LPCTSTR file_path, TCHAR *pBufname, TCHAR *pBuffalg, TCHAR *pBufaddr, UINT channelID)
{
	TCHAR tmpBufAddr[256] = {0};
	TCHAR tmpBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	swprintf(tmpBufAddr, _T("%s_%d_A"), pBufname, channelID);//获取相应的标志
	//获取读到的值
	GetPrivateProfileString(_T("config_addr"), tmpBufAddr, NULL, tmpBuf, MAX_MAC_SEQU_ADDR_COUNT, theApp.ConvertAbsolutePath(file_path));
	
	_tcscpy(pBufaddr, tmpBuf);

	return TRUE;
}
//读配置文件中的地址
BOOL CConfig::read_config_addr(LPCTSTR file_path, TCHAR *pBufAddr, TCHAR *pGAddr, TCHAR *pCurAddr)
{
	UINT  j, tempmac, tempsequence;
	BOOL  flag = FALSE;
	TCHAR tmpBufFlag[256] = {0};
	TCHAR tmpBufAddr[256] = {0};
	TCHAR tmpBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR tmpBufMax[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR dstBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	CHAR macbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	CString str;

	USES_CONVERSION;
	
	memset(pGAddr, 0, MAX_MAC_SEQU_ADDR_COUNT);

	for (j=0; j < 32; j++)
	{
		swprintf(tmpBufFlag,_T("%s_%d_F"), pBufAddr, j);
		//从文件中读取MAC或序列号
		GetPrivateProfileString(_T("config_addr"), tmpBufFlag, NULL, tmpBuf, MAX_MAC_SEQU_ADDR_COUNT, theApp.ConvertAbsolutePath(file_path));
		if (tmpBuf[0] == 0)
		{
			if (j == 0)
			{
				//如果config_addr.txt文件不存在时，都从开头位置开始
				if (pBufAddr == maccurrentlow)
				{
					_tcscpy(pCurAddr, mac_start_low);//记录当前的MAC
				}
				if (pBufAddr == sequencecurrentlow)
				{
					_tcscpy(pCurAddr, sequence_start_low);//记录汉前的序列号
				}
				return FALSE;
			}

			break;
		}

		if (tmpBuf[0] == 'Y')
			flag = TRUE;//全都成功
		else
			flag = FALSE;//有存在失败的
		
		swprintf(tmpBufAddr, _T("%s_%d_A"), pBufAddr, j);
		GetPrivateProfileString(_T("config_addr"), tmpBufAddr, NULL, tmpBuf, MAX_MAC_SEQU_ADDR_COUNT, theApp.ConvertAbsolutePath(file_path));
	
		//把从config_addr.txt文件中读出的地址转换出大写
		lower_to_upper(tmpBuf, dstBuf);

		if (_tcscmp(dstBuf, tmpBufMax) > 0)
		{
			_tcscpy(tmpBufMax, dstBuf);
		}
		
		//如果上一次烧录有失败的，取失败的值
		if (!flag)
		{
			_tcscpy(pGAddr+(MAX_MAC_SEQU_ADDR_COUNT + 1)*j, tmpBuf);
		}
		
		//如果通道数大于最大的时，那么其他都清空
		if (j >= device_num)
		{
			WritePrivateProfileString(_T("config_addr"), tmpBufFlag, _T(""),theApp.ConvertAbsolutePath(file_path));
			WritePrivateProfileString(_T("config_addr"), tmpBufAddr, _T(""),theApp.ConvertAbsolutePath(file_path));
		}

	}
	//mac地址
	if (pBufAddr == maccurrentlow)
	{
		//对低位进行十六进制加一
		sprintf(macbuf, "%c%c%c%c%c%c", tmpBufMax[0], tmpBufMax[1], tmpBufMax[3], tmpBufMax[4], tmpBufMax[6], tmpBufMax[7]);
		//地址递增一
		sscanf(macbuf, "%x", &tempmac);
		tempmac++;
		sprintf(macbuf, "%06x", tempmac);
		swprintf(dstBuf, _T("%c%c:%c%c:%c%c"), macbuf[0],macbuf[1],macbuf[2],macbuf[3],macbuf[4],macbuf[5]);
		
		//把从当前的地址转换出大写
		lower_to_upper(dstBuf, pCurAddr);
		
		//比较二个值
		if (_tcscmp (mac_start_low, pCurAddr) > 0)
		{
			_tcscpy(pCurAddr, mac_start_low);
		}
	
	
	}
	else if (pBufAddr == sequencecurrentlow)//序列号
	{
		tempsequence = atoi(T2A(tmpBufMax));
		tempsequence++;//自增1
		str.Format(_T("%06d"), tempsequence);
		_tcscpy(pCurAddr, str);	
		//比较
		if (_tcscmp (sequence_start_low, pCurAddr) > 0 )
		{
			_tcscpy(pCurAddr, sequence_start_low);
		}
	}
	else
	{
		//出错提示
		MessageBox(NULL, _T("pBufAddr ERR!"), NULL,MB_OK);
	}

	return TRUE;
}