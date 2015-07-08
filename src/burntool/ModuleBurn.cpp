// ModuleBurn.cpp: implementation of the CModuleBurn class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Burn.h"
#include "BurnTool.h"
#include "ModuleBurn.h"

extern "C"
{
#include "IFWD_DownloadDll.h"
}

extern CConfig theConfig;//
extern CBurnToolApp theApp;//

CModuleBurn g_MB[MAX_MODULE_BURN_NUM];//

static HWND m_hwnd = (HWND)NULL;//

void CallBack(unsigned char channel, IFWD_DL_status_enum status, char *status_text);


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModuleBurn::CModuleBurn()
{
	m_channel = 1;

	m_port = 1;
	m_baudrate = 115200;//m_baudrate

	memset(m_ErrorText, 0, sizeof(m_ErrorText));//memset

	m_stat = MBSTAT_initial;
}

CModuleBurn::~CModuleBurn()
{
}

void CallBack(unsigned char channel, IFWD_DL_status_enum status, char *status_text)
{
	int progress;
	char strTrace[1024];

	switch(status)
	{
	case IFWD_DL_ProgressUpdated:
		progress = atoi(status_text);//progress
		PostMessage(m_hwnd, ON_MODULE_BURN_PROGRESS, WPARAM(channel+100), LPARAM(progress));//PostMessage
		break;

	case IFWD_DL_ProcessOutlineUpdated:
		sprintf(strTrace, "OutlineUpdate in channel %d: %s", channel, status_text);
		theTrace.TraceInfo(strTrace);//TraceInfo
		PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(channel+100), LPARAM(MODULE_MESSAGE_OUTLINE_UPDATED));//PostMessage
		break;

	case IFWD_DL_OK:
		sprintf(strTrace, "OK in channel %d: %s", channel, status_text);
		theTrace.TraceInfo(strTrace);
		PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(channel+100), LPARAM(MODULE_MESSAGE_DL_OK));
		break;

	case IFWD_DL_ProcessDetailUpdated:  //
		sprintf(strTrace, "DetailedUpdate in channel %d: %s", channel, status_text);
		theTrace.TraceInfo(strTrace);//
		break;

	case IFWD_DL_Error://
		sprintf(strTrace, "Error in channel %d: %s", channel, status_text);
		theTrace.TraceInfo(strTrace);//
		PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(channel+100), LPARAM(MODULE_MESSAGE_DL_ERROR));
		break;

	case IFWD_DL_AT_Command://
		break;

	default:
		break;
	}
}


void CModuleBurn::ModuleInit(HWND hwnd, int BootTimeOut)
{
	int i;
	char ErrorText[1024];

	m_hwnd = hwnd;

	for(i = 0; i < MAX_MODULE_BURN_NUM; i++)//
	{
		g_MB[i].m_channel = i+1;
	}

	//set parameter for DLL
	IFWD_DL_init_callback(CallBack, ErrorText);//
	
	// set to 1: so we don't need timers here
	IFWD_DL_set_dll_parameter(IFWD_DL_dll_parameter_stay_in_function, 0, ErrorText);//

	//set boot timer out
	IFWD_DL_set_dll_parameter(IFWD_DL_dll_parameter_boot_process_timeout, BootTimeOut, ErrorText);//

//	IFWD_DL_set_dll_parameter(IFWD_DL_dll_parameter_comm_timeout, 10000, ErrorText);

}


BOOL CModuleBurn::StartBurn(int port, int baudrate)
{
	IFWD_DL_status_enum Status;
	char *port_name_base = "\\\\.\\COM";
	char port_name[15];
	IFWD_DL_modem_control_signals_type OnBoot_modem_control_signals;//

	char strTrace[1024];//

	//store parameters
	m_port = port;//

	m_baudrate = baudrate;//

	m_stat = MBSTAT_initial;//

	//open COM port
	sprintf(port_name,"%s%i",port_name_base, m_port);//
	//
	sprintf(strTrace, "try to open COM%d in channel %d", m_port, m_channel);
	theTrace.TraceInfo(strTrace);//

	m_ErrorText[0] = 0;
	Status = IFWD_DL_open_comm_port(m_channel, port_name, port_name, m_baudrate, m_ErrorText);//
    if(Status == IFWD_DL_Error)
    {
		sprintf(strTrace, "Error in open COM%d in channel %d", m_port, m_channel);//
		theTrace.TraceInfo(strTrace);//
		return FALSE;//
	}

	sprintf(strTrace, "try to open boot target in channel %d", m_channel);//
	theTrace.TraceInfo(strTrace);//

    m_ErrorText[0] = 0;
    Status = IFWD_DL_boot_target(m_channel, theApp.ConvertAbsolutePath_ASC(theConfig.path_fls), 
		&OnBoot_modem_control_signals, m_ErrorText); // use first file in list to boot with
    if(Status == IFWD_DL_Error)//
    {
		sprintf(strTrace, "Error in boot target in channel %d", m_channel);//
		theTrace.TraceInfo(strTrace);
		return FALSE;
	}

	m_stat = MBSTAT_booting_target;//

//	Sleep(5000);

	return TRUE;
}

void CModuleBurn::HandleOutlineUpdate()
{
	if(MBSTAT_booting_target == m_stat)
	{
		m_stat = MBSTAT_device_synchronized;//

		PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(m_channel+100), //
			LPARAM(MODULE_MESSAGE_DEVICE_SYNCHRONIZED));//
	}
}

void CModuleBurn::HandleOK()
{
	IFWD_DL_status_enum Status;//

	if(MBSTAT_device_synchronized == m_stat)//
	{
		if(theConfig.bDownloadFLS)
		{
			PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(m_channel+100), 
				LPARAM(MODULE_MESSAGE_DL_FLS_START));

			//start downloading fls
			m_stat = MBSTAT_downloading_fls;

			Sleep(1000);
			
			Status = IFWD_DL_download_fls_file(m_channel, theApp.ConvertAbsolutePath_ASC(theConfig.path_fls), m_ErrorText);
			if(Status == IFWD_DL_Error)
			{
				theTrace.TraceInfo(m_ErrorText);		
			}
		}
		else if(theConfig.bDownloadEEP)
		{
			PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(m_channel+100), 
				LPARAM(MODULE_MESSAGE_DL_EEP_START));

			//start downloading eep
			m_stat = MBSTAT_downloading_eep;
			
			Status = IFWD_DL_download_eep_file(m_channel, theApp.ConvertAbsolutePath_ASC(theConfig.path_eep), IFWD_DL_target_boot_mode_normal, m_ErrorText);
			if(Status == IFWD_DL_Error)
			{
				theTrace.TraceInfo(m_ErrorText);		
			}
		}
		else
		{
			//download successful
			PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(m_channel+100), 
				LPARAM(MODULE_MESSAGE_DL_SUCCESS));
			
			IFWD_DL_close_comm_port(m_channel, m_ErrorText);
			m_stat = MBSTAT_finished;
		}
		return;
	}

	if(MBSTAT_downloading_fls == m_stat)
	{
		if(theConfig.bDownloadEEP)
		{
			PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(m_channel+100), 
				LPARAM(MODULE_MESSAGE_DL_EEP_START));
			
			//start downloading eep
			m_stat = MBSTAT_downloading_eep;
			
			Status = IFWD_DL_download_fls_file(m_channel, theApp.ConvertAbsolutePath_ASC(theConfig.path_eep), m_ErrorText);
			if(Status == IFWD_DL_Error)
			{
				theTrace.TraceInfo(m_ErrorText);		
			}
		}
		else
		{
			//download successful
			PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(m_channel+100), 
				LPARAM(MODULE_MESSAGE_DL_SUCCESS));
			
			IFWD_DL_close_comm_port(m_channel, m_ErrorText);
			m_stat = MBSTAT_finished;
		}
		return;
	}

	if(MBSTAT_downloading_eep == m_stat)
	{
		//download successful
		PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(m_channel+100), 
			LPARAM(MODULE_MESSAGE_DL_SUCCESS));
		
		IFWD_DL_close_comm_port(m_channel, m_ErrorText);
		m_stat = MBSTAT_finished;

		return;
	}
}

void CModuleBurn::HandleDetailUpdate()
{
}

void CModuleBurn::HandleError()
{
	//Error in Booting
	if(MBSTAT_booting_target == m_stat || MBSTAT_device_synchronized == m_stat)
	{
		PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(m_channel+100), 
			LPARAM(MODULE_MESSAGE_BOOT_FAIL));

		IFWD_DL_close_comm_port(m_channel, m_ErrorText);
		m_stat = MBSTAT_finished;

		return;
	}

	//Error in downloading fls
	if(MBSTAT_downloading_fls == m_stat)
	{
		PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(m_channel+100), 
			LPARAM(MODULE_MESSAGE_DL_FLS_FAIL));

		IFWD_DL_close_comm_port(m_channel, m_ErrorText);
		m_stat = MBSTAT_finished;

		return;
	}

	//Error in download eep
	if(MBSTAT_downloading_eep == m_stat)
	{
		PostMessage(m_hwnd, ON_MODULE_BURN_MESSAGE, WPARAM(m_channel+100), 
			LPARAM(MODULE_MESSAGE_DL_EEP_FAIL));

		IFWD_DL_close_comm_port(m_channel, m_ErrorText);
		m_stat = MBSTAT_finished;

		return;
	}
}

void CModuleBurn::EndBurn()
{
	IFWD_DL_close_comm_port(m_channel, m_ErrorText);
	m_stat = MBSTAT_finished;
}
