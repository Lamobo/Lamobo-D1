#pragma once

#include "threadbase.h"
#include "IPCameraCommand.h"
#include <string>
using namespace std;

#ifdef UNICODE
#define TString wstring
#else
#define TString string
#endif

class CFtpDownloadThread :
	public CThreadBase
{
public:
	CFtpDownloadThread(void);
	CFtpDownloadThread(const CFtpDownloadThread & _Val);
	CFtpDownloadThread(int iID, char * fptURL, int nftpPort, TCHAR * strFileName, 
					   TCHAR * strSaveFileDir, TCHAR * usr = NULL, WCHAR * pwd = NULL);
	virtual ~CFtpDownloadThread(void);

	int SetFtpServer(char * ftpURL, int nftpPort);
	int SetFtpUser(TCHAR * usr = NULL, TCHAR * pwd = NULL);
	int SetDownloadFile(TCHAR * strFileName, TCHAR * strSaveFileDir);
	int SetID(int nID);

	int GetLastErrorCode();
	int GetLastErrorMsg(int nErrorCode, string & strErrMsg);
	
	BOOL IsStart();

	int Start();
	int Stop();
	
	uint64_t GetDownloadLen(int & nErrorCode);
	uint64_t GetFtpFileLen(int & nErrorCode);
	uint64_t GetDownloadLen();
	uint64_t GetFtpFileLen();
	int GetID();

protected:
	virtual void Run();

	int CheckFtpServerSet();

private:
	string m_strFptURL;
	TString m_strFileName, m_strSaveDir, m_strUsr, m_strPwd;
	int m_nftpPort, m_nErrorCode, m_nID;

	uint64_t m_nFtpFileLen;
	uint64_t m_nDownloadLen;
	
	BOOL m_bIsStart;
};
