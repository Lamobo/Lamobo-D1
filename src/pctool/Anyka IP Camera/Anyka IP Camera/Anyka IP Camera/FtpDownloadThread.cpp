#include "StdAfx.h"
#include "afxinet.h"
#include "FtpDownloadThread.h"


#define FTP_SERVER_URL_ILLAGE		0x8000ff00
#define FTP_SERVER_PORT_ILLAGE		0x8000ff01
#define FTP_SERVER_CONNECT_FAILED	0x8000ff02
#define FTP_SERVER_FILE_OPEN_ERR	0x8000ff03
#define ERR_OPEN_LOACAL_SAVE_FILE	0x8000ff04
#define ERR_WRITE_LOCAL_FILE		0x8000ff05

CFtpDownloadThread::CFtpDownloadThread(void)
:m_nftpPort(-1), m_nErrorCode(0), m_nFtpFileLen(0), m_nID(-1), m_nDownloadLen(0), m_bIsStart(FALSE)
{
	return;
}

CFtpDownloadThread::CFtpDownloadThread(int iID, char * fptURL, int nftpPort, TCHAR * strFileName, 
										 TCHAR * strSaveFileDir, TCHAR * usr, WCHAR * pwd)
: m_nftpPort(nftpPort), m_nErrorCode(0), m_nFtpFileLen(0), m_nID(iID), m_nDownloadLen(0), m_bIsStart(FALSE)
{
	if (fptURL) m_strFptURL = fptURL;
	if (strFileName) m_strFileName = strFileName;
	if (strSaveFileDir) m_strSaveDir = strSaveFileDir;
	if (usr) m_strUsr = usr;
	if (pwd) m_strPwd = pwd;
}

CFtpDownloadThread::CFtpDownloadThread(const CFtpDownloadThread & _Val)
{
	m_strFptURL = _Val.m_strFptURL;
	m_strFileName = _Val.m_strFileName;
	m_strSaveDir = _Val.m_strSaveDir;
	m_strUsr = _Val.m_strUsr;
	m_strPwd = _Val.m_strPwd;
	m_nftpPort = _Val.m_nftpPort;
	m_nErrorCode = _Val.m_nErrorCode;
	m_nID = _Val.m_nID;
	m_nFtpFileLen = _Val.m_nFtpFileLen;
	m_nDownloadLen = _Val.m_nDownloadLen;
	m_bIsStart = _Val.m_bIsStart;
	m_handle = _Val.m_handle;
	m_ThreadID =_Val.m_ThreadID;
	m_ThreadName = _Val.m_ThreadID;
	m_bRun = _Val.m_bRun;
}

CFtpDownloadThread::~CFtpDownloadThread(void)
{
	Stop();
}

int CFtpDownloadThread::SetFtpServer(char * ftpURL, int nftpPort)
{
	m_strFptURL.clear();
	m_strFptURL = ftpURL;

	m_nftpPort = nftpPort;
	return 0;
}

int CFtpDownloadThread::SetFtpUser(TCHAR * usr, TCHAR * pwd)
{
	if (usr) m_strUsr = usr;
	if (pwd) m_strPwd = pwd;

	return 0;
}

int CFtpDownloadThread::SetDownloadFile(TCHAR * strFileName, TCHAR * strSaveFileDir)
{
	if (strFileName) m_strFileName = strFileName;
	if (strSaveFileDir) m_strSaveDir = strSaveFileDir;
	return 0;
}

int CFtpDownloadThread::SetID(int nID)
{
	m_nID = nID;
	return 0;
}

int CFtpDownloadThread::GetID()
{
	return m_nID;
}

BOOL CFtpDownloadThread::IsStart()
{
	return m_bIsStart;
}

int CFtpDownloadThread::Start()
{
	if (m_bIsStart) return 0;
	
	m_nErrorCode = 0;

	if (CheckFtpServerSet() < 0) return -1;
	
	m_bIsStart = TRUE;
	m_nFtpFileLen = 0;
	m_nDownloadLen = 0;

	CThreadBase::Start();

	return 0;
}

int CFtpDownloadThread::Stop()
{
	if (!m_bIsStart) return 0;
	
	m_bIsStart = FALSE;
	Join();

	return 0;
}

uint64_t CFtpDownloadThread::GetDownloadLen(int & nErrorCode)
{
	nErrorCode = m_nErrorCode;
	return m_nDownloadLen;
}

uint64_t CFtpDownloadThread::GetFtpFileLen(int & nErrorCode)
{
	nErrorCode = m_nErrorCode;
	return m_nFtpFileLen;
}

uint64_t CFtpDownloadThread::GetDownloadLen()
{
	return m_nDownloadLen;
}

uint64_t CFtpDownloadThread::GetFtpFileLen()
{
	return m_nFtpFileLen;
}

int CFtpDownloadThread::GetLastErrorCode()
{
	return m_nErrorCode;
}

int CFtpDownloadThread::GetLastErrorMsg(int nErrorCode, string & strErrMsg)
{
	switch (nErrorCode) {
		case FTP_SERVER_URL_ILLAGE:
			strErrMsg = "the ftp server is ip that you set is illage!\n";
			break;
		case FTP_SERVER_PORT_ILLAGE:
			strErrMsg = "the ftp server is port that you set is illage!\n";
			break;
		case FTP_SERVER_CONNECT_FAILED:
			strErrMsg = "can't connect to the ftp server!\n";
			break;
		case FTP_SERVER_FILE_OPEN_ERR:
			strErrMsg = "can't open the ftp server is file that you set!\n";
			break;
		case ERR_OPEN_LOACAL_SAVE_FILE:
			strErrMsg = "can't open the local file to save the download data!\n";
			break;
		case ERR_WRITE_LOCAL_FILE:
			strErrMsg = "write loacal file error!\n";
			break;
		default:
			strErrMsg = "no error\n";
			break;
	}

	return 0;
}

void CFtpDownloadThread::Run()
{
	USES_CONVERSION;
	CInternetSession InternetSession;
	CFtpConnection * pFtpConn = NULL;
	CInternetFile * pFtpFile = NULL;
	CString strFtpUrl, fileDir;
	CFileException fileException;
	CFile file;
	UINT nFtpReadSize = 0;
	BYTE buffer[1024] = {0};
	BOOL bIsOpenFile = FALSE;
	CFtpFileFind * pFtpFind = NULL;
	
	strFtpUrl.Format(_T("%s"), A2W(m_strFptURL.c_str()));
	
	try {
		pFtpConn = InternetSession.GetFtpConnection(strFtpUrl, 
			m_strUsr.c_str()?NULL:m_strUsr.c_str(), m_strPwd.c_str()?NULL:m_strPwd.c_str(), m_nftpPort);
	}
	catch (CInternetException * pEx)
	{
		TCHAR sz[1024];
		pEx->GetErrorMessage(sz, 1024);
		AfxMessageBox(sz);
		pEx->Delete();
		m_nErrorCode = FTP_SERVER_CONNECT_FAILED;
		goto end;
	}
	
	if (pFtpConn == NULL) {
		m_nErrorCode = FTP_SERVER_CONNECT_FAILED;
		goto end;
	}

	fileDir.Format(_T("%s\\%s"), m_strSaveDir.c_str(), m_strFileName.c_str());
	
	UINT nMode = 0;
	
	//CInternetFile不支持FTP协议的端点续传，需要自己实现。
	/*if (PathFileExists(fileDir)) nMode = CFile::modeWrite;
	else*/ nMode = CFile::modeCreate | CFile::modeReadWrite | CFile::typeBinary;

	if (!file.Open(fileDir, nMode, &fileException)){
		m_nErrorCode = ERR_OPEN_LOACAL_SAVE_FILE;
		goto end;
	}

	bIsOpenFile = TRUE;

	file.SeekToEnd();
	m_nDownloadLen = file.GetLength();
	
	pFtpFind = new CFtpFileFind(pFtpConn);
	if (!pFtpFind->FindFile(m_strFileName.c_str())) {
		m_nErrorCode = FTP_SERVER_FILE_OPEN_ERR;
		pFtpFind->Close();
		delete pFtpFind;
		goto end;
	}
	pFtpFind->FindNextFile();
	m_nFtpFileLen = pFtpFind->GetLength();
	pFtpFind->Close();
	delete pFtpFind;
 
	pFtpFile = pFtpConn->OpenFile(m_strFileName.c_str());
	if (pFtpFile == NULL) {
		m_nErrorCode = FTP_SERVER_FILE_OPEN_ERR;
		goto end;
	}
	
	/*//CInternetFile不支持FTP协议的断点续传，需要自己实现。
	if (m_nFtpFileLen != 0)
		pFtpFile->Seek(m_nDownloadLen, CInternetFile::begin);*/
	
	while(TRUE) {
		if (!m_bIsStart) break;
		
		if (m_nDownloadLen >= m_nFtpFileLen) break;
		
		try {
			nFtpReadSize = pFtpFile->Read(buffer, sizeof(buffer));
		}
		catch (CInternetException * pEx)
		{
			TCHAR sz[1024];
			pEx->GetErrorMessage(sz, 1024);
			AfxMessageBox(sz);
			pEx->Delete();
			m_nErrorCode = FTP_SERVER_CONNECT_FAILED;
			goto end;
		}

		if (nFtpReadSize <= 0) continue;
		
		try {
			file.Write(buffer, sizeof(buffer));
		}catch(CFileException * pEx) {
			TCHAR sz[1024];
			pEx->GetErrorMessage(sz, 1024);
			AfxMessageBox(sz);
			pEx->Delete();
			m_nErrorCode = ERR_WRITE_LOCAL_FILE;
			goto end;
		}

		m_nDownloadLen += nFtpReadSize;
	}

end:
	if (pFtpFile) {
		pFtpFile->Close();
		delete pFtpFile;
	}

	if (pFtpConn) {
		pFtpConn->Close();
		delete pFtpConn;
	}

	InternetSession.Close();
	
	if (bIsOpenFile) file.Close();

	m_bIsStart = FALSE;
}

int CFtpDownloadThread::CheckFtpServerSet()
{
	if (m_strFptURL.empty()) {
		fprintf(stderr, "please set ftp server is url!\n");
		m_nErrorCode = FTP_SERVER_URL_ILLAGE;
		return -1;
	}
	
	if (m_nftpPort > 65535 || m_nftpPort < 0) {
		fprintf(stderr, "ftp server port illage!\n");
		m_nErrorCode = FTP_SERVER_PORT_ILLAGE;
		return -1;	
	}
	
	return 0;
}