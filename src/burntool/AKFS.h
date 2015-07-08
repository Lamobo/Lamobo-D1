// AKFS.h: interface for the CAKFS class.
//
//////////////////////////////////////////////////////////////////////
extern "C"
{
	#include "fsa.h"
//	#include "fha_test.h"
}

#if !defined(AFX_AKFS_H__C04685B0_6A04_4B58_B43B_2568B17632DD__INCLUDED_)
#define AFX_AKFS_H__C04685B0_6A04_4B58_B43B_2568B17632DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CAKFS  
{
public:
	BOOL LowFormat(PBYTE PartitionInfo,     UINT nNumPartion, UINT resvSize, UINT  StartBlock,  UINT MediumType, PBYTE MediumInfo, UINT *StartID, UINT *IDCnt);
	BOOL DownloadFile(UINT obj);//обтьнд╪Ч
	VOID Destroy(VOID);//Destroy
	VOID UnMountNandFlash(UCHAR DriverID);//UnMountNandFlash
	VOID UnMountMemDev(UCHAR DriverID);//UnMountMemDev
	UINT MountNandFlash(UINT NandBase, UINT StartBlock, UCHAR DriverList[], UCHAR *DriverCnt);
	BOOL GetDriverInfo(UINT StartID, UINT DriverCnt, UINT *DriverNum, UCHAR *Info, UINT MediumType);
	BOOL Init(VOID);
	BOOL DownloadImg(UINT nID, HANDLE hFile, T_IMG_INFO *img_info, UINT img_buf_len);
	CAKFS();
	virtual ~CAKFS();

};

#endif // !defined(AFX_AKFS_H__C04685B0_6A04_4B58_B43B_2568B17632DD__INCLUDED_)
