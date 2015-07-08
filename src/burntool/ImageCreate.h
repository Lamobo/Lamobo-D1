// ImageCreate.h: interface for the CImageCreate class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGECREATE_H__1354C64A_4164_4FF9_9878_A4D5007B66DD__INCLUDED_)
#define AFX_IMAGECREATE_H__1354C64A_4164_4FF9_9878_A4D5007B66DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef CRC_CHECK
#include "Config.h"
#define CHECK_IMG_FILE_POS	200
#define CHECK_FLAG_SIZE		4
extern BYTE CHECK_FLAG[CHECK_FLAG_SIZE];
#endif

extern "C"
{
	#include "anyka_types.h"
	#include "mtdlib.h"
    #include "medium.h"
    #include "fs.h"
    #include "eng_dataconvert.h"
}

#pragma pack(1)
typedef struct
{
    T_U8    BS_jmpBoot[3];//
    T_U8    BS_OEMName[8];
    T_U16   BPB_BytsPerSec;//BPB_BytsPerSec
    T_U8    BPB_SecPerClus;//BPB_SecPerClus
    T_U16   BPB_RsvdSecCnt;//BPB_RsvdSecCnt
    T_U8    BPB_NumFATs;//BPB_NumFATs
    T_U16   BPB_RootEntCnt;//BPB_RootEntCnt
    T_U16   BPB_TotSec16;//BPB_TotSec16
    T_U8    BPB_Media;//BPB_Media
    T_U16   BPB_FATsz16;//BPB_FATsz16
    T_U16   BPB_SecPerTrk;//BPB_SecPerTrk
    T_U32   BPB_HiddSec;//BPB_HiddSec
    T_U32   BPB_TotSec32;
}
T_BPB;

#pragma pack()

class CImageCreate  
{
public:
	CImageCreate();
	virtual ~CImageCreate();

public:
	typedef struct image
	{
		char diskname;
		HANDLE hFile;
		void *driver;
#ifdef CRC_CHECK
		T_U8 FileCheckValue[CHECK_XOR_LEN];
#endif
	}T_IMAGE;

	UINT DriverCapacity;
	UINT TotalFCapacity;
	DWORDLONG FileSize;
	BOOL ExitFlag;
	UINT nID;

protected:
	CArray<T_IMAGE, T_IMAGE> m_image_array;
	BOOL bAddVolumeLable;

public:
	BOOL fslib_init();//文件系统初始化

	BOOL img_create(LPTSTR strPath, T_U32 capacity, T_U32 BytsPerSec, T_U32 BytsPerPage, T_U32 MediumType, char driverName);
	void img_destroy();//销毁

	BOOL img_add_file(LPTSTR pathPC, LPSTR pathDisk);//加文件
	BOOL img_add_dir(LPTSTR pathPC, LPSTR pathDisk);//加文件夹
			
	BOOL img_add_volume_lable(T_U8* volume_name);//增加卷标
protected:
	BOOL FindFileInDir(LPTSTR pathPC, LPSTR pathImg);//找文件

};

#endif // !defined(AFX_IMAGECREATE_H__1354C64A_4164_4FF9_9878_A4D5007B66DD__INCLUDED_)
