// ModuleBurn.h: interface for the CModuleBurn class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODULEBURN_H__A095B235_620E_4EB1_8026_4A4343AA071E__INCLUDED_)
#define AFX_MODULEBURN_H__A095B235_620E_4EB1_8026_4A4343AA071E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_MODULE_BURN_NUM 32

class CModuleBurn
{

public:
	CModuleBurn();
	~CModuleBurn();

protected:
	enum
	{
		MBSTAT_initial,
		MBSTAT_booting_target,
		MBSTAT_device_synchronized,
		MBSTAT_downloading_fls,
		MBSTAT_downloading_eep,
		MBSTAT_finished,
	}
	m_stat;

public:
	int m_channel;//m_channel
	int m_port;//
	int m_baudrate;//
	char m_ErrorText[1024];//

public:
	void HandleOutlineUpdate();//HandleOutlineUpdate

	void HandleOK();//HandleOK

	void HandleDetailUpdate();//HandleDetailUpdate

	void HandleError();//HandleError

public:
	static void ModuleInit(HWND hwnd, int BootTimeOut);//ModuleInit

	BOOL StartBurn(int port, int baudrate);//StartBurn

	void EndBurn();//EndBurn
};

#endif // !defined(AFX_MODULEBURN_H__A095B235_620E_4EB1_8026_4A4343AA071E__INCLUDED_)
