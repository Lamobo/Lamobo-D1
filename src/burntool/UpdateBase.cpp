// UpdateBase.cpp: implementation of the CUpdateBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UpdateBase.h"
#include "Config.h"
#include "USBTransc.h"

#define MAX_SEND_SCSI_LEN 6 //MAX 6 
BYTE m_send_SCSI_cmd[MAX_SEND_SCSI_LEN] =
{
	'A', 'N', 'Y', 'K', 'A', 0
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern CConfig theConfig;

CUpdateBase::CUpdateBase()
{
	m_hDisk = INVALID_HANDLE_VALUE;
}

CUpdateBase::~CUpdateBase()
{

}

int CUpdateBase::GetUDisk(char pDiskName[], int len)
{
	DWORD	MaxDriveSet;
	DWORD	drive;
	TCHAR	szDrvName[33];
	int		count = 0;

	MaxDriveSet = GetLogicalDrives();//GetLogicalDrives

	for (drive = 0; drive < 32; drive++)  
	{
		if ( MaxDriveSet & (1 << drive) )  
		{
			DWORD temp = 1<<drive;
			_stprintf(szDrvName, _T("%c:\\"), 'A'+drive);//szDrvName
			
			if(GetDriveType(szDrvName)== DRIVE_REMOVABLE && (drive > 1))
			{
				pDiskName[count++] = 'A'+ (char)drive;
				if(count >= len)
				{
					break;
				}
			}
		}
	}

	return count;
}

BOOL CUpdateBase::OpenDisk(char diskName)
{
	DWORD	accessMode = 0, shareMode = 0;//
	TCHAR	szBuf[260];//

    shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;  // default
    accessMode = GENERIC_WRITE | GENERIC_READ;       // default

	_stprintf(szBuf, _T("\\\\?\\%c:"), diskName);//
	m_hDisk = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);//CreateFile

	if(INVALID_HANDLE_VALUE == m_hDisk)//
	{
		return FALSE;
	}

	return TRUE;

}

void CUpdateBase::CloseDisk()
{
	if(m_hDisk != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDisk);//CloseHandle
	}
}

BOOL CUpdateBase::SendCommand()
{
	if(INVALID_HANDLE_VALUE == m_hDisk)
	{
		return FALSE;
	}
	//·¢ËÍCC
	return BT_SendExSCSICommand(m_hDisk, 0xCC, m_send_SCSI_cmd, MAX_SEND_SCSI_LEN - 1);
}
