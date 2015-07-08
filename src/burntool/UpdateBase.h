// UpdateBase.h: interface for the CUpdateBase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPDATEBASE_H__B7F8B122_65C5_433F_A5C1_F69EE318A0B8__INCLUDED_)
#define AFX_UPDATEBASE_H__B7F8B122_65C5_433F_A5C1_F69EE318A0B8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CUpdateBase  
{
public:
	CUpdateBase();
	virtual ~CUpdateBase();

protected:
	HANDLE m_hDisk;

public:
	int GetUDisk(char pDiskName[], int len);

	BOOL OpenDisk(char diskName);
	void CloseDisk();
	BOOL SendCommand();
};

#endif // !defined(AFX_UPDATEBASE_H__B7F8B122_65C5_433F_A5C1_F69EE318A0B8__INCLUDED_)
