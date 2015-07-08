#include "stdafx.h"
#include "ServerInfo.h"

WCHAR * strFunction[COMM_TYPE_CNT][10] = 
			{{L"Record", L"Talkback", L"Photo", L"Motion Detection", L"Stop Recording", NULL, NULL, NULL, NULL, NULL},
			 {L"Image Settings", L"ISP Preferences", L"Privacy Block", L"Zoom Settings", L"Volume Settings", L"PTZ", NULL, NULL, NULL, NULL},
			 {L"Get Video Files", L"Obtain Basic Information For Service", L"Get Privacy Masking", L"Get Motion Detection Area", L"Get Image Settings", L"Get Video Files Lists", L"Get ISP Preferences", NULL, NULL, NULL}};

WCHAR * strError[ERROR_MAX] = {L"Unknown error", L"No Storage Devices", L"No Capture Device", L"No Output Devices", L"SD Card Mount Failure",
							   L"SD Card Read Only", L"File Open Error", L"File Write Error", L"File Read Error"};

int GetStringFromRetInfo(RETINFO & retInfo, CString & strServerInfo)
{
	strServerInfo.Empty();

	if ((retInfo.nCommandType >= COMM_TYPE_CNT) || (retInfo.nSubCommandType >= 10))
		return -1;

	WCHAR * pstrFun = strFunction[retInfo.nCommandType][retInfo.nSubCommandType];
	if (pstrFun) {
		strServerInfo.Format(L"%s", pstrFun);
	}

	if (retInfo.retType == RET_WARN_CODE) {
		strServerInfo.Append(L"[Warning]:");
	}else if (retInfo.retType == RET_SUCCESS) {
		strServerInfo.Append(L":Success!");
		return 0;
	}else if (retInfo.retType == RET_ERROR_CODE) {
		strServerInfo.Append(L"[Error]:");
	}

	if (retInfo.retType == RET_ERROR_CODE) {
		if (retInfo.nCode >= ERROR_MAX)
			return -1;

		strServerInfo.Append(strError[retInfo.nCode]);
	}

	return 0;
}