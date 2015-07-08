// Update.h: interface for the CUpdate class.
//
//* @date 2009/11/11
//
// * @version 1.0
//
// * @author Lu Qiliu.
//
// * Copyright 2009 Anyka corporation, Inc. All rights reserved.
//
// * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
// */
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPDATE_H__55137F11_4698_4E0D_8E3A_4B119C49AFF8__INCLUDED_)
#define AFX_UPDATE_H__55137F11_4698_4E0D_8E3A_4B119C49AFF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//format信息个数
//tagUpdateFileHead
//
//


//T_UPDATE_FILE_HEAD
//文件头信息，固定为"anyka106"
//format信息个数
//format信息偏移量
//nand信息偏移量
 //spi信息个数
//spi信息偏移量
//producer偏移量
//producer偏移量
//producer文件大小
//bios偏移量
//bios文件大小
//udisk目录信息个数
//udisk目录信息偏移量
//udisk文件个数
typedef struct tagUpdateFileHead
{
	BYTE head_info[8];			//文件头信息，固定为"anyka106"
	UINT format_count;			//format信息个数
    UINT format_offset;			//format信息偏移量
	UINT nand_count;			//nand信息个数
    UINT nand_offset;			//nand信息偏移量
	//UINT spi_count;			    //spi信息个数
    //UINT spi_offset;			//spi信息偏移量
	UINT producer_offset;		//producer偏移量
	DWORD producer_size;		//producer文件大小
	UINT bios_offset;			//bios偏移量
	DWORD bios_size;			//bios文件大小
	UINT udisk_info_count;		//udisk目录信息个数
	UINT udisk_info_offset;		//udisk目录信息偏移量
	UINT udisk_file_count;		//udisk文件个数
	UINT udisk_file_info_offset;//udisk文件信息偏移量
	UINT file_count;			//nand文件个数
    UINT file_info_offset;		//nand信息偏移量
	UINT config_tool[8][2];		//配置工具文件信息使用,config_tool[0]--ImageRse,config_tool[1]--PROG
	//烧录配置信息使用: 
	//Other_config[0]:Usb mode
	//Other_config[1]:NonFS reserve size
	//Other_config[2]:fs reserve size
	//Other_config[3]:volumelable data offset
	//Other_config[4]:bUpdateself
	//Other_config[5]:snowbirdsE
    //Other_config[6]:burn_mode
	UINT Other_config[8];	//	
	UINT check_sum;	//			
}
T_UPDATE_FILE_HEAD;

//T_FILE_INFO
//是否比较
//链接地址
//文件长度
//文件偏移量
//文件名（存储在设备上文件名）
typedef struct tagFileInfo
{
	BYTE bCompare;				//是否比较
	BYTE bBack;                 //是否备份
	BYTE bin_len;               //bin区大小
	BYTE resev;                 //未用，保留
	UINT ld_addr;				//链接地址
	UINT file_length;			//文件长度
    UINT file_offset;			//文件偏移量
	CHAR file_name[16];			//文件名（存储在设备上文件名）
	UINT check_sum;//
}
T_FILE_INFO;

//T_FILE_INFO
//是否比较
//链接地址
//文件长度
//文件偏移量
//文件名（存储在设备上文件名）
typedef struct tagFileInfo_linux
{
	BYTE bCompare;				//是否比较
	BYTE bBack;                 //是否备份
	BYTE resev1;               
	BYTE resev2;                 //未用，保留
	UINT ld_addr;				//链接地址
	UINT file_length;			//文件长度
    UINT file_offset;			//文件偏移量
	CHAR file_name[16];			//文件名（存储在设备上文件名）
	FLOAT bin_len;              //bin区大小
	UINT check_sum;              
}
T_FILE_INFO_LINUX;

//T_UDISK_INFO
//是否比较
//pc路径
//udisk路径
typedef struct tagUdiskInfo
{
	UINT bCompare;				//是否比较
	CHAR pc_path[MAX_PATH];		//pc路径
	CHAR udisk_path[MAX_PATH];	//udisk路径
	UINT check_sum;//
}
T_UDISK_INFO;

//T_UDISK_UPDATE_FILE_INFO
//pc上整个文件路径
//文件长度
//文件偏移量
typedef struct tagUdiskFileInfo
{
	CHAR pc_file_path[MAX_PATH];//pc上整个文件路径
	UINT file_length;			//文件长度
	UINT file_offset;			//文件偏移量
	UINT check_sum;//
}
T_UDISK_UPDATE_FILE_INFO;

//如下CUpdate

class CUpdate  
{
public:
	CUpdate();//
	virtual ~CUpdate();//

    BOOL ExportUpdateFile(CConfig *pCFG, CString file_path, CString strCheck);//
    BOOL ImportUpdateFile(CConfig *pCFG, CString file_path);//

protected:
	BOOL CreateUpdateDir();//创建文件夹
	BOOL ReadBuffer(HANDLE hFile, LPVOID buf, UINT begin, UINT size);//读
	BOOL WriteBuffer(HANDLE hFile, LPVOID buf, UINT begin, UINT size);//写
    UINT Cal_CheckSum(PBYTE data, UINT len); //检测
	UINT GetFileCnt(CString path, UINT* pFileCnt);//获取文件个数
	UINT GetUdiskFileCount(CConfig *pCFG);//获取u盘文件
	BOOL StoreFile(HANDLE hPacketFile, TCHAR* file_path, UINT file_offset, UINT file_len);//存储文件
    BOOL StoreProducer(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead);//produce
	BOOL StoreBios(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead);//bois
	BOOL StoreNandFile(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead);//nand
	BOOL StoreNandFile_linux(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead);
	BOOL StoreUdiskFile(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead);//u盘
	void SetUdiskFileInfo(CString strPath, T_UDISK_UPDATE_FILE_INFO *pUdisFileInfo, UINT *pFileCnt);
	
	BOOL Del_UpdateFiles(LPCTSTR file_path);
	BOOL UnpacketFile(HANDLE hSourceFile, LPCTSTR file_path, UINT file_offset, UINT file_length);//升级文件
	BOOL UnpacketUdiskFile(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_UDISK *pDownloadUdisk);//升级nand
	BOOL UnpacketNandFile(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_NAND* pDownloadNand);
	BOOL UnpacketNandFile_linux(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_NAND* pDownloadNand);

	BOOL WriteCheckExport(HANDLE hFile, CString strCheck);
    BOOL CUpdate::UnpacketCheckExport(HANDLE hSourceFile, BYTE str[]);
private:
	UINT m_nand_file_offset;//
	CString m_update_folder;//
};

#endif // !defined(AFX_UPDATE_H__55137F11_4698_4E0D_8E3A_4B119C49AFF8__INCLUDED_)
