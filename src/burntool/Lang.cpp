// Lang.cpp: implementation of the CLang class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "burntool.h"
#include "Lang.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define UNICODE_TXT_HEAD 0xFEFF//

extern CBurnToolApp theApp;//

//注意
//如果是中文
//编写时要注间位置,
//长度不要超过50

TCHAR *g_strCN[] = 
{
_T("烧录工具"),//
_T("升级模式"),//

_T("文件(&F)"),//
_T("导入文件(&I)"),//
_T("导出文件(&O)"),//
_T("设置(&S)"),//
_T("参数设置"),//
_T("修改密码"),//
_T("视图(&V)"),//
_T("语言选择"),//
_T("COM窗口"),
_T("工具(&T)"),
_T("镜像制作(&I)"),
_T("SPI镜像(&S)"),
_T("BIN回读(&B)"),
_T("配置文件(&C)"),
_T("帮助(&H)"),
_T("关于BurnTool (&A)"),

_T("开始"),
_T("设置"),
_T("导入"),
_T("导出"),
_T("低格"),

_T("状态"),
_T("编号"),
_T("进度"),
_T("状态"),
_T("时间"),
_T("序列号"),
_T("MAC地址"),
_T("设备"),

_T("当前连接:"),
_T("可用连接:"),
_T("烧录次数:"),
_T("累计通过:"),
_T("累计失败:"),
_T("清除"),
_T("烧录状态"),
_T("USB设备:"),
_T("U盘:"),
_T("自动烧录"),
_T("U盘烧录"),
_T("在线制作镜像(只在设备0,并仅制作一次)"),

_T("生产者"),
_T("研发者"),
_T("请输入用户名和密码:"),
_T("用户名:"),
_T("密码:"),
_T("修改密码"),
_T("用户:"),
_T("旧密码:"),
_T("新密码:"),
_T("确认密码:"),
_T("密码错误!"),
_T("确认密码不一样"),
_T("修改密码成功"),


_T("准备就绪"),
_T("磁盘升级准备就绪"),
_T("开始烧录"),
_T("开始设置RAM参数"),
_T("设置RAM参数失败"),
_T("开始设置寄存器"),
_T("设置寄存器失败"),
_T("获取寄存器失败"),
_T("设置寄存器成功"),
_T("开始初始化USB"),
_T("初始化USB失败"),
_T("烧录完成"),
_T("开始测试RAM"),
_T("测试RAM失败"),
_T("开始下载producer"),
_T("下载producer失败"),
_T("下载producer后usb连接失败"),
_T("下载producer等待超时"),
_T("下载producer成功"),
_T("分区信息为空"),
_T("获取媒介数据信息失败"),
_T("获取空闲块信息失败"),
_T("开始创建分区"),
_T("创建分区失败"),
_T("构造媒介失败"),
_T("开始测试"),
_T("测试失败"),
_T("开始获取Nandflash芯片id"),
_T("获取Nandflash芯片id失败"),
_T("开始设置Nandflash芯片参数"),
_T("设置Nandflash芯片参数失败"),

_T("开始获取spiflash芯片id"),//
_T("获取spiflash芯片id失败"),//
_T("开始设置spiflash芯片参数"),//
_T("设置spiflash芯片参数失败"),//

_T("开始擦除"),
_T("擦除失败"),
_T("开始格式化"),
_T("格式化失败"),
_T("开始下载文件"),
_T("下载文件失败"),
_T("开始获取激活文件"),
_T("获取激活文件失败"),

_T("项目名称："),
_T("下载通道数："),
_T("   平台类型："),
_T("打开串口"),
_T("串口数："),
_T("第一个串口号："),
_T("波特率："),
_T("配置命令行"),
_T("   命令行地址："),
_T("   命令行数据："),
_T("烧录MAC地址"),
_T("强制写入mac地址"),
_T("当前的mac地址复位"),
_T("mac起始地址(0x):"),
_T("mac结束地址(0x):"),
_T("烧录序列号"),
_T("强制写入序列号"),
_T("当前的序列号复位"),
_T("序列号起始:"),
_T("序列号结束:"),
_T("mac起始前缀(0x):"),//
_T("mac结束前缀(0x):"),//
_T("序列号起始前缀:"),//
_T("序列号结束前缀:"),//

_T("浏览"),
_T("Producer: "),
_T("boot: "),
_T("boot(37L): "),
_T("Bios: "),

_T("下载到U盘路径: "),
_T("下载到flash路径："),
_T("MTD层烧录："),
_T("序号"),
_T("是否比较"),
_T("是"),
_T("否"),
_T("文件在PC路径"),
_T("下载文件"),
_T("下载目录"),
_T("文件下载路径"),
_T("起始地址"),
_T("结束地址"),
_T("备份起始位置"),
_T("备份结束位置"),
_T("盘符"),

_T("链接地址"),
_T("文件名"),
_T("是否备份"),
_T("是"),
_T("否"),
_T("bin区大小(MB)"),

_T("芯片类型："),
_T("USB2.0"),
_T("变频(pll)"),
_T("读取激活文件"),
_T("升级模式"),
//_T("支持自动下载"),
//_T("系统频率(M):"),
_T("随机存储器"),
_T("设置GPIO"),
_T("模式"),
_T("烧录"),
_T("导入..."),
_T("导出..."),
_T("  行:"),
_T("  列:"),
_T("支持USB Boot方式"),
_T("Power_off GPIO:"),
_T("BIOS运行地址:"),//
_T("文件系统保留区:"),//
_T("RAM类型:"),
_T("大小(M):"),
_T("Bank："),
_T("Row Length:"),//
_T("Col Address:"),//
_T("片选:"),
_T("自定义"),
_T("片选0"),
_T("片选1"),
_T("片选2"),
_T("片选3"),
_T("轮循所有"),
_T("初始化USB"),
_T("使用GPIO:"),
_T("写寄存器"),
_T("寄存器:"),
_T("位:"),

_T("格式化列表："),
_T("序号"),
_T("盘符"),
_T("用户盘"),
_T("资料信息"),
_T("分区属性"),
_T("大小 (M)"),
_T("卷标名"),
_T("文件系统保留区(block):"),
_T("非文件系统保留区(M)："),

_T("Nandflash 列表:"),
_T("SpiNandflash 列表:"),
_T("SPI 列表:"),

_T("开始下载Infineon模块程序"),//
_T("下载Infineon模块程序失败"),//
_T("设备同步成功"),//
_T("驱动目标设备失败"),//
_T("开始下载fls文件"),
_T("开始下载eep文件"),//
_T("下载fls文件失败"),//
_T("下载eep文件失败"),//

_T("下载模式"),
_T("只做AK下载"),
_T("只做Infineon下载"),
_T("二者都做"),
_T("下载相关设置"),
_T("波特率："),
_T("下载fls文件："),
_T("下载eep文件："),
_T("浏览"),
_T("GPIO设置"),
_T("GPIO设置失败"),//
_T("设置参数到producer失败"),//

_T("开始下载bin"),
_T("下载bin失败"),//
_T("开始下载img"),
_T("下载img失败"),
_T("开始下载boot"),//
_T("下载boot失败"),//


_T("开始写mac地址入安全区"),//
_T("写mac地址入安全区失败"),
_T("开始写序列号地址入安全区"),//
_T("写序列号地址入安全区失败"),//

_T("开始下载变频小程序"),//
_T("下载变频小程序失败"),//
_T("下载变频小程序超时"),//
_T("下载变频小程序后usb连接失败"),//

_T("设置烧录模式"),
_T("设置烧录模式失败"),
_T("设置擦除模式"),
_T("设置擦除模式失败"),
_T("设置Nand参数"),
_T("设置Nand参数失败"),
_T("设置安全区"),
_T("设置安全区失败"),
_T("bin回读失败"),
_T("获取坏块信息失败"),

_T("读安全区中的mac地址"),
_T("读安全区中的mac地址是无效的"),//
_T("读安全区中的序列号地址"),
_T("读安全区中的序列号地址是无效的"),//


_T("烧录mac地址比较"),
_T("烧录mac地址超出最大值"),
_T("烧录序列号地址比较"),
_T("烧录序列号地址超出最大值"),	

_T("设置保留区"),
_T("设置保留区失败"),
_T("创建分区表"),
_T("创建分区表失败"),
_T("终止烧录线程"),
_T("终止烧录线程失败"),

_T("请输入16进制的数字"),
_T("MAC地址格式必须是00:00:00"),
_T("MAC起始地址高位的字符长度少于8!!"),
_T("MAC起始地址低位的字符长度少于8!!"),
_T("MAC结束地址高位的字符长度少于8!!"),
_T("MAC起始地址低位的字符长度少于8!!"),
_T("字符串中有不是16进制的!!!!"),
_T("MAC起始高位和结束地址的高位不相同!!"),//
_T("MAC起始低位大于结束地址的低位!!"),//
_T("序列号起始高位的字符长度少于10!!"),//
_T("序列号起始低位的字符长度少于6!!"),//
_T("序列号结束高位的字符长度少于10!!"),//
_T("序列号结束低位的字符长度少于6!!"),//
_T("序列号起始的高位与结束的高位不相同!!"),//
_T("序列号起始的低位大于结束的低位!!"),//
_T("当前的mac地址已复位成功，当前值等于开始的值!"),//
_T("当前的序列号已复位成功，当前值等于开始的值!"),//

_T("设置通道的地址"),
_T("通道的地址错误"),

_T("镜像制作"),
_T("源文件夹:"),
_T("目标文件夹:"),
_T("浏览"),
_T("浏览"),
_T("镜像文件名:"),
_T("总容量(MByte):"),
_T("页大小(Byte):"),
_T("扇区大小(Byte):"),
_T("制作信息:"),
_T("准备镜像制作.."),
_T("正在进行镜像制作中..."),
_T("创建"),
_T("结束"),
_T("镜像制作成功!!"),
_T("镜像制作失败!!"),
_T("正在获取文件夹信息!!"),
_T("获取文件夹信息失败!!"),
_T("总容量大小不够!!"),
_T("配置信息出错!!"),

_T("源文件夹是空的!"),
_T("目标文件夹是空的!"),
_T("镜像文件名是空的!"),
_T("总容量大小是空的!"),
_T("扇区大小是空的!"),
_T("页大小是空的!"),

_T("设置nand gpioce失败!"),//
_T("下载channel地址失败!"),//
_T("设置串口类型失败!"),//
_T("下载串口类型失败!"),//

_T("正在制作镜像!"),
_T("镜像制作成功!"),
_T("镜像制作失败!"),
_T("完成镜像制作后清空选框!"),//

_T("程序正在进行中,是否要关闭？"),//

_T("输入校验字符串"),
_T("文件导入成功!"),
_T("文件导入失败!"),//
_T("请稍等..."),
_T("文件导出成功!"),//
_T("文件导出失败!"),//

_T("低格将破坏之前的信息,确定要对设备进行格式化吗?"),//
_T("SPI 镜像"),
_T("请选择SPIFLASH型号："),
_T("型号："),
_T("没有可选择的spi型号"),
_T("打开帮助文档失败"),

_T("设备低格"),
_T("确定"),
_T("取消"),
_T("分区的卷标名不能包含如下的9个字符"),//

_T("此mac地址是组播地址，请修改"),//
_T("此mac地址不能全是0"),//
_T("获取highID失败, produce版本是否太旧了"),//
_T("创建分区前统计所有的空闲块"),//
_T("个别通道的介质(nand/sd)容量不一致"),//
//_T("各通道的介质容量大小相同,才可以多台烧录,请确认"),
_T("检测每个通道的介质容量大小是否一样"),
_T("bin回读完成"),
_T("bin回读, 先进入massboot, 再按开始"),
_T("spiflash回读完成"),
_T("spiflash回读失败"),
_T("spiflash回读, 先进入massboot, 再按开始"),
_T("开始回读整个spiflash数据"),


NULL
};

//注意
//如果是英文
//编写时要注间位置,
//长度不要超过50

TCHAR *g_strEN[] = 
{
_T("Burn Tool"),
_T("Update Mode"),

_T("File(&F)"),
_T("Import File(&I)"),
_T("Export File(&O)"),
_T("Setting(&S)"),
_T("Config Setting"),
_T("Modify Password"),
_T("View(&V)"),
_T("Language"),
_T("COM Window"),
_T("Tool(&T)"),
_T("Creat Image(&I)"),
_T("Spi Image(&S)"),
_T("Bin Upload(&B)"),
_T("ConfigFiles(&C)"),
_T("Help(&H)"),
_T("About BurnTool (&A)"),

_T("Start"),//
_T("Setting"),//
_T("Import"),//
_T("Export"),
_T("Format"),//

_T("Status"),
_T("Number"),
_T("Progress"),//
_T("Status"),
_T("Timer"),//
_T("serial NO"),//
_T("mac addr"),//
_T("Device"),//

_T("Current link:"),
_T("Available link:"),//
_T("Burn count:"),///
_T("Total pass:"),//
_T("Total fail:"),//
_T("Clear"),
_T("Burn State"),//
_T("USBNum:"),//
_T("Udisk:"),
_T("Auto Burn"),//
_T("UDiskBurn"),//
_T("online make_image(only device0 && make once)"),//

_T("Producer"),
_T("Researcher"),
_T("Input the User Name and Password:"),
_T("User Name:"),//
_T("Password:"),
_T("Change Password"),//
_T("User:"),
_T("Old Password:"),
_T("New Password:"),
_T("Confirm Password:"),
_T("Error Password!"),
_T("Confirm Password Error!"),//
_T("Modify Password Success"),//


_T("Ready"),
_T("Udisk update Ready"),
_T("start burn"),//
_T("start to set RAM parameter"),//
_T("Fail to set RAM parameter"),
_T("Begin to set registers"),
_T("Fail to set registers"),
_T("Fail to get registers"),
_T("Set registers Sucessfully"),
_T("Begin to init USB"),
_T("Fail to init USB"),
_T("Burn task complete"),//
_T("start test RAM"),
_T("test RAM fail"),
_T("Begin to download producer"),//
_T("Fail to download producer"),
_T("Fail to connet usb after download producer"),//
_T("loading producer time out"),
_T("Download producer Succcessfully"),
_T("partition info is null"),
_T("get medium data info fail"),
_T("get free block info fail"),
_T("Start to mount disk"),
_T("Fail to mount disk"),
_T("malloc medium fail"),
_T("Start to Test"),
_T("Fail to test"),//
_T("Begin to get nandflash chip id"),
_T("Fail to get nandflash chip id"),//
_T("Begin to set nandflash chip parameter"),
_T("Fail to set nandflash chip parameter"),//

_T("Begin to get spiflash chip id"),
_T("Fail to get spiflash chip id"),
_T("Begin to set spiflash chip parameter"),
_T("Fail to set spiflash chip parameter"),

_T("Begin to erase"),
_T("Fail to erase"),
_T("Begin to format"),
_T("Fail to format"),//
_T("Begin to download file"),
_T("Fail to download file"),//
_T("Start to get activation file"),//
_T("Fail to get activation file"),

_T("Project Name: "),
_T("Channel Number: "),
_T("Planform Tpye: "),
_T("Open COM"),
_T("COM Number: "),
_T("COM Base: "),
_T("Baudrate: "),
_T("Config Command line"),
_T("Command line addr:"),
_T("Command line data:"),//

_T("burn MAC addr"),
_T("force write mac addr"),//
_T("current mac reset"),
_T("mac start addr(0x):"),
_T("mac end addr(0x):"),
_T("burn serial"),
_T("force write serial"),
_T("current serial reset"),//
_T("serial NO start:"),
_T("serial NO end:"),//
_T("mac startprefix(0x):"),
_T("mac endprefix(0x):"),//
_T("serial startprefix:"),
_T("serial endprefix:"),

_T("Browse"),
_T("Producer: "),
_T("boot: "),
_T("boot(37L): "),
_T("Bios:"),

_T("Download to U Disk"),
_T("Download to flash"),//
_T("Download to MTD"),
_T("N.O."),//
_T("bCompare"),
_T("YES"),//
_T("NO"),//
_T("PC Path"),//
_T("Download File"),//
_T("Download Directory"),
_T("UDisk Path"),
_T("Start Address"),
_T("End Address"),
_T("Backup Start Address"),
_T("Backup End Address"),//
_T("Disk Name"),

_T("Link Address"),
_T("File Name"),
_T("bBackup"),//
_T("YES"),
_T("NO"),
_T("bin area size(MB)"),//

_T("Chip Type: "),
_T("USB2.0"),
_T("PLLFrep_change"),
_T("Get Activation File"),
_T("Update Mode"),
//_T("Auto Download"),
//_T("Chip Clock(M):"),
_T("RAM"),
_T("SET GPIO"),
_T("MODE"),
_T("BURNED"),
_T("Import..."),
_T("Export..."),
_T("Row:"),
_T("Column:"),
_T("USB Boot"),
_T("Power off GPIO"),//
_T("BIOS Start Address"),
_T("FS Reserver Blocks"),//
_T("RAM Type: "),
_T("Size(M): "),
_T("Bank: "),//
_T("Row Length: "),//
_T("Col Address: "),
_T("ChipSel:"),
_T("Manual"),
_T("Chip0"),
_T("Chip1"),
_T("Chip2"),//
_T("Chip3"),
_T("All"),//
_T("Init USB"),//
_T("Plug GPIO"),
_T("Write Register"),//
_T("Register: "),
_T("Bit: "),

_T("Driver List: "),
_T("N.O."),
_T("Symbol"),
_T("User Disk"),
_T("Infor"),
_T("Attibute"),//
_T("Size(M)"),
_T("volume"),
_T("FS Reserver Blocks"),
_T("NonFS Reserve Size"),

_T("Nandflash List: "),//
_T("SpiNandflash List: "),//
_T("Spiflash list:"),//

_T("Start Infineon module burn"),
_T("Infineon module burn fail"),
_T("device_synchronized!"),
_T("failed to boot target board"),//
_T("Start to download fls file"),
_T("Start to download eep file"),//
_T("failed to download fls file"),//
_T("failed to download eep file"),//

_T("download mode"),
_T("AK only"),
_T("Infineon only"),
_T("Both"),
_T("Burn Setting"),
_T("Baudrate: "),
_T("download fls:"),
_T("download eep:"),//
_T("Browse"),
_T("GPIO Setting"),//
_T("GPIO Setting_fail"),//
_T("fail to set param to producer"),

_T("start to download bin"),
_T("fail to download bin"),
_T("start to download img"),
_T("fail to download img"),
_T("start to download boot"),
_T("fail to download boot"),


_T("start write mac addr into asa"),
_T("write mac addr into asa fail"),
_T("start write serial addr into asa"),
_T("write serial addr into asa fail"),

_T("Begin to download change clk"),//
_T("Fail to download change clk"),//
_T("loading change clk time out"),//
_T("Fail to connet usb after download change clk"),//

_T("set burn mode"),
_T("set burn mode fail"),//
_T("set burn erase mode"),//
_T("set burn erase mode fail"),//
_T("set nand para"),
_T("set nand para fail"),//
_T("set security area"),
_T("set security area fail"),//
_T("bin upload fail"),
_T("get bad block information fail"),//

_T("read mac addr in asafile"),
_T("read mac addr  error"),
_T("read serial addr in asafile"),
_T("read serial addr error"),	

_T("burn mac addr compare"),//
_T("burn mac addr beyond the most"),//
_T("burn serial addr compare"),//
_T("burn serial addr beyond the most"),	//

_T("set reserve area"),//
_T("set reserve area fail"),//
_T("create partition"),//
_T("create partition fail"),//
_T("close"),//
_T("close fail"),

_T("pls input the num must be 0~9，A~F，a~f"),//
_T("MAC Addr format must be 00:00:00"),
_T("mac start addr high Length less than 8 !!"),
_T("mac start addr low Length less than 8 !!"),
_T("mac end addr high Length less than 8 !!"),
_T("mac end addr low Length less than 8 !!"),
_T("the string is not 0~9，A~F，a~f !!!!"),
_T("h_mac_start_addr and h_end_addr is differnt!!"),//
_T("l_mac_start addr is more than l_mac_end_addr !!"),//
_T("h_serial_start_addr str Length less than 10 !!"),///
_T("l_serial_start_addr str Length less than 6 !!"),//
_T("h_serial_end_addr str Length less than 10 !!"),//
_T("l_serial_end_addr str Length less than 6 !!"),//
_T("h_serial_start_addr and h_end_addr is differnt!!"),//
_T("l_serial_start_addr is more than l_end_addr!!"),//
_T("mac addr  reset succeed, current = start !!"),//
_T("current serial  reset succeed, current = start !!"),//


_T("start set channel id"),
_T("get the channel id is error"),//

_T("Image Create Box"),
_T("sourcefolder:"),
_T("destfolder:"),//
_T("Browse"),//
_T("Browse"),//
_T("Imagename:"),//
_T("capacity(MByte):"),//
_T("pagesize(Byte):"),//
_T("sectorsize(Byte):"),//
_T("make information:"),//
_T("Ready Image make..."),//
_T("Image making..."),//
_T("Begin"),//
_T("End"),//
_T("Image make success!!"),//
_T("Image make fail!!"),//
_T("getting folder info!!"),//
_T("get folder info fail!!"),//
_T("capacity size is no enough!!"),//
_T("param error!!"),//

_T("source folder empty!"),//
_T("Destination folder empty!"),//
_T("image name empty!"),
_T("capacity empty!"),
_T("sector empty!"),
_T("page size empty!"),


_T("set nand gpioce fail!"),
_T("download channel addr fail!"),
_T("set com mode fail!"),
_T("download com mode fail!"),

_T("image making"),
_T("make image success!"),
_T("make image fail!"),
_T("finish make image and reset!"),

_T("Process is running, make sure to close or no!"),

_T("Input verify string"),
_T("Import file success!"),
_T("Import file fail!"),
_T("wait..."),
_T("Export file success!"),
_T("Export file fail!"),

_T("All data will be lost, make sure to format or not"),
_T("Spiflash Mirror"),
_T("Choose the spi,please!"),
_T("SPI:"),
_T("No SPI can choose"),
_T("Open help file error"),

_T("Physical Format"),
_T("OK"),
_T("Cancel"),
_T("driver volume name must not be"),

_T("the mac addr is special addr，please modification"),//
_T("the mac addr can not be zero"),//
_T("get highID fail,produce is not too old"),//
_T("before creat disk to get all free block"),//
_T("the capacity of the nand/sd is differnt"),//
//_T("the capacity must be same and to burn,please check"),
_T("check the capacity of ervery device"),
_T("bin upload complete"),
_T("bin upload, and pelease enter massboot to start"),
_T("spiflash upload complete"),
_T("spiflash upload fail"),
_T("spiflash upload, and pelease enter massboot to start"),
_T("start upload spiflash all data"),

NULL
};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLang::CLang()
{
	m_stringAlloc = NULL;//
	m_offsetAlloc = NULL;//

	m_langCount = 2;
	m_langName[0] = _T("中文");//初始化中文
	m_langName[1] = _T("English");//初始化英文

	m_activeLang = 0xFFFFFFFF;//
}

CLang::~CLang()
{	
	if(m_stringAlloc)
	{
		delete[] m_stringAlloc;//释放
		m_stringAlloc = NULL;//置空
	}

	if(m_offsetAlloc)//
	{
		delete[] m_offsetAlloc;//释放
		m_offsetAlloc = NULL;//置空
	}
}

BOOL CLang::Init(CString langName)
{
	ScanLang(theApp.ConvertAbsolutePath(_T("Lang")));//文件名
	
	UINT i;
	UINT activeLang = 1;//

	for(i = 0; i < m_langCount; i++)
	{
		if(m_langName[i] == langName)//
		{
			activeLang = i;//
		}
	}

	if(FALSE == ChangeLang(activeLang))//变换语言
	{
		LoadDefaultString(FALSE);//
		m_activeLang = 1;
	}

	return TRUE;
}

CString CLang::GetString(UINT strID)
{
	if(NULL==m_offsetAlloc || NULL==m_stringAlloc || strID >= IDS_MAX)
	{
		return CString(_T(""));//
	}

	CString theStr;
	theStr.Format(_T("%s"), m_stringAlloc+m_offsetAlloc[strID]);//

	return theStr;
}

BOOL CLang::LoadDefaultString(BOOL bCN)
{
	TCHAR **pStr = NULL;
	UINT strCount = IDS_MAX;

	if(bCN)
	{
		pStr = g_strCN;//中
	}
	else
	{
		pStr = g_strEN;//英
	}

	m_offsetAlloc = new	UINT[strCount];//分配
	if(NULL == m_offsetAlloc)
	{
		return FALSE;//
	}

	m_stringAlloc = new TCHAR[strCount * LANG_CHINA_ENGLISH_LEN];//分配
	if(NULL == m_stringAlloc)//
	{
		return FALSE;
	}

	memset(m_stringAlloc, 0, sizeof(TCHAR)*strCount*LANG_CHINA_ENGLISH_LEN);//清0

	UINT i, offset;//
	UINT len;

	offset = 0;//
	i = 0;
	while(i < strCount && pStr[i])
	{
		len = _tcslen(pStr[i])*sizeof(TCHAR);
		memcpy(m_stringAlloc+offset, pStr[i], len);//复制
		m_stringAlloc[offset+len] = 0;

		m_offsetAlloc[i] = offset;//偏移

		offset += len+1;//
		i++;//
	}

	if(i < strCount)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CLang::LoadStringFromFile(CString strPath)
{
	HANDLE hFile;
	DWORD dwSize;
	DWORD read_count;
	UINT len;
	
	if(0xFFFFFFFF == GetFileAttributes(strPath))//属性
	{
		return FALSE;
	}

	hFile = CreateFile(strPath, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, 0, NULL);//创建
	if(INVALID_HANDLE_VALUE == hFile)//
	{
		return FALSE;
	}

	dwSize = GetFileSize(hFile, NULL);//获取文件长度
	if(0xFFFFFFFF == dwSize)
	{
		CloseHandle(hFile);//关闭
		return FALSE;
	}

	if(m_stringAlloc)//
	{
		delete[] m_stringAlloc;//
		m_stringAlloc = NULL;
	}
	
	// Alloc Space for String buffer
	len = dwSize/sizeof(TCHAR) + 2;//大		
	m_stringAlloc = new TCHAR[len];
	if(NULL == m_stringAlloc)
	{
		CloseHandle(hFile);//
		return FALSE;
	}
	memset(m_stringAlloc, 0, len);//清0

	if(NULL == m_offsetAlloc)//
	{
		m_offsetAlloc = new UINT[IDS_MAX];//
		if(NULL == m_offsetAlloc)
		{
			CloseHandle(hFile);//
			return FALSE;
		}
	}

	ReadFile(hFile, m_stringAlloc, dwSize, &read_count, NULL);//读取

	CloseHandle(hFile);//关闭

	UINT indexString = 0, indexOffset = 0;//
	while(indexOffset < IDS_MAX)//
	{
		while(indexString<len && '"' != m_stringAlloc[indexString++]);//
		m_offsetAlloc[indexOffset++] = indexString;//
		
		while(indexString<len && '"' != m_stringAlloc[indexString++]);//
		m_stringAlloc[indexString-1] = 0;//

		if(indexString > len)
		{
			break;
		}
	}

	if(indexOffset < IDS_MAX)
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL CLang::WriteString()
{
	CString str;
	UINT i;

	if(NULL == m_stringAlloc || NULL == m_offsetAlloc)//
	{
		return FALSE;
	}

	CreateDirectory(theApp.ConvertAbsolutePath(_T("Lang")), NULL);//创建文件夹

	str.Format(_T("Lang\\%s.txt"), m_langName[m_activeLang]);
	if(0xFFFFFFFF != GetFileAttributes(theApp.ConvertAbsolutePath(str)))//属性
	{
		return TRUE;
	}

	//open file
	CStdioFile *pFile;
	pFile = new CStdioFile(theApp.ConvertAbsolutePath(str), 
		CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);
	if(NULL == pFile)
	{
		return FALSE;
	}

	USHORT head = UNICODE_TXT_HEAD;
	pFile->Write(&head, 2);
	
	pFile->WriteString(_T("Lang"));//
	pFile->WriteString(_T("{\r\n"));//

	for(i = 0; i < IDS_MAX; i++)
	{
		str.Format(_T("\"%s\",\r\n"), m_stringAlloc+m_offsetAlloc[i]);//
		pFile->WriteString(str);
	}

	pFile->WriteString(_T("}\r\n"));//

	pFile->Close();
	delete pFile;

	return TRUE;
}

UINT CLang::ScanLang(CString dirPath)
{
	CString langName;
	CString fileName;
	BOOL bFound;
	UINT pos;
	CFileFind cFF;
	
	bFound = cFF.FindFile(dirPath+"\\*.txt");
	if(bFound)
	{
		do
		{
			bFound = cFF.FindNextFile();//查下一个
			
			fileName = cFF.GetFileTitle();//
			if(fileName != _T("中文") && fileName != _T("English"))//不等时
			{
				pos = fileName.Find('.');//
				langName = fileName.Left(pos);//
				m_langName[m_langCount] = fileName;//
				m_langCount++;
			}
			
		}
		while(bFound);//
	}
	else
	{
		return 0;
	}

	return m_langCount-2;//
}

BOOL CLang::ChangeLang(UINT langID)
{
	if(langID == m_activeLang || langID >= m_langCount)//
	{
		return FALSE;
	}

	CString strLangFile;

	strLangFile.Format(theApp.ConvertAbsolutePath(_T("lang\\%s.txt")), m_langName[langID]);
	if(LoadStringFromFile(strLangFile))//加载
	{
		m_activeLang = langID;//
	}
	else
	{
		if(langID < 2)//
		{
			LoadDefaultString(langID==0);//
			m_activeLang = langID;
		}
		else
		{
			return FALSE;
		}
	}
	
	return TRUE;
}
