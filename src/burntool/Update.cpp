// Update.cpp: implementation of the CUpdate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "burntool.h"
#include "Update.h"
#include "anyka_types.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define FILE_HEAD_INFO			"ANYKA106"
#define ADJUST_512(x)			(((x-1)/512+1)*512)
#define UPDATE_FOLDER			_T("Update_Files")
#define WRITE_BUF_LEN			(512*1024)
#define IMAGE_RES_NAME			_T("ImageRes") 
#define SPOTLIGHT_NAME			_T("PROG") 
#define CHECK_EXPORT_ID			"ANYKA_ID"

extern CBurnToolApp theApp;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUpdate::CUpdate()
{
	m_nand_file_offset = 0;
}

CUpdate::~CUpdate()
{
}

//Caclutate check sum
UINT CUpdate::Cal_CheckSum(PBYTE data, UINT len)
{
    UINT sum = 0;
    UINT i;

     for(i = 0; i < len-2;)
    {
		T_U16 xor1 = (T_U16)data[i];
		T_U16 xor2 = (T_U16)data[i+2];
		
		sum += xor1 ^ xor2;
		i +=2;
    }

    return sum;
}
//read file to buffer
BOOL CUpdate::ReadBuffer(HANDLE hFile, LPVOID buf, UINT begin, UINT size)
{
	BOOL ret;
	DWORD read_len;

	if (INVALID_HANDLE_VALUE == hFile || NULL == buf)
	{
		return FALSE;
	}
	
	memset(buf, 0, size);
	SetFilePointer(hFile, begin, NULL, FILE_BEGIN);
	ret = ReadFile(hFile, buf, size, &read_len, NULL);	
	if(!ret || read_len != size)
	{
		return FALSE;
	}

	return TRUE;
}

//write buffer to file
BOOL CUpdate::WriteBuffer(HANDLE hFile, LPVOID buf, UINT begin, UINT size)
{
	BOOL ret;
	DWORD write_len;

	if (INVALID_HANDLE_VALUE == hFile || NULL == buf)
	{
		return FALSE;
	}

	SetFilePointer(hFile, begin, NULL, FILE_BEGIN);
	ret = WriteFile(hFile, buf, size, &write_len, NULL);	
	if(!ret || write_len != size)
	{
		return FALSE;
	}

	FlushFileBuffers(hFile);

	return TRUE;
}

// delelte files of update folder
BOOL CUpdate::Del_UpdateFiles(LPCTSTR file_path)
{	
	CString strPCPath = file_path;
	strPCPath = theApp.ConvertAbsolutePath(strPCPath);
	BOOL bFind;
	CFileFind* fd = new CFileFind();
	
	strPCPath += _T("\\*.*");
	bFind = fd->FindFile(strPCPath);
	while(bFind)
	{
		bFind = fd->FindNextFile();	
		if(!fd->IsDots())
		{
			strPCPath = fd->GetFilePath();
			if(!fd->IsDirectory())
			{
				if(!DeleteFile(strPCPath))
				{
					delete fd;
					fd = NULL;
					return FALSE;
				}
			}
			else
			{
				Del_UpdateFiles(strPCPath);
			}
		}
		RemoveDirectory(strPCPath);
	}

		
	
	delete fd;
	fd = NULL;
	return TRUE;
}

// create update folder
BOOL CUpdate::CreateUpdateDir()
{
	WIN32_FIND_DATA fd;
	CString strPCUpdatePath = theApp.ConvertAbsolutePath(UPDATE_FOLDER);	

	if(INVALID_HANDLE_VALUE != FindFirstFile(strPCUpdatePath, &fd))
	{
		if(FILE_ATTRIBUTE_DIRECTORY == GetFileAttributes(LPCTSTR(strPCUpdatePath)))
		{
			if(!Del_UpdateFiles(strPCUpdatePath))
			{
				return FALSE;
			}
		}
	}
	else
	{
		if(!CreateDirectory(strPCUpdatePath, NULL))
		{
			return FALSE;
		}
	}

	m_update_folder = strPCUpdatePath;
	m_update_folder += _T("\\");
	
	return TRUE;
}

// Unpacket update file
BOOL CUpdate::UnpacketFile(HANDLE hSourceFile, LPCTSTR file_path, UINT file_offset, UINT file_length)
{
	HANDLE hDstFile;
	BOOL ret;
	UINT i=0;
	BYTE* dataBuffer;
//	WIN32_FIND_DATA fd;
	TCHAR pathTemp[MAX_PATH+1] = {0};
	
	if(INVALID_HANDLE_VALUE == hSourceFile || NULL == file_path)
	{
		return FALSE;
	}

	// create path
	while('\0'!=file_path[i])
	{
		while('\\' != file_path[i])
		{
			if(_T('\0')==file_path[i])
			{
				break;
			}
			i++;
		}
		if('\0' !=file_path[i])
		{	
			wcsncpy(pathTemp, file_path, i);

		//	if(INVALID_HANDLE_VALUE == FindFirstFile(pathTemp, &fd))
		//	{
				CreateDirectory(pathTemp,NULL);
		//	}
			i++;
		}
	}
	
	//create file 
	hDstFile = CreateFile(file_path, GENERIC_WRITE, FILE_SHARE_WRITE, 
		NULL, CREATE_ALWAYS, 0, NULL);
	if(INVALID_HANDLE_VALUE == hDstFile)
	{
		return FALSE;
	}
	
	dataBuffer = new BYTE[WRITE_BUF_LEN];
	if (NULL == dataBuffer)
	{
		CloseHandle(hDstFile);
		return FALSE;
	}
	memset(dataBuffer, 0, WRITE_BUF_LEN);
	
	//SetFilePointer
	SetFilePointer(hSourceFile, file_offset, NULL, FILE_BEGIN);

	DWORD read_len, write_len;
	UINT nSpare = file_length % WRITE_BUF_LEN;
	UINT nTimes = file_length / WRITE_BUF_LEN;

	while(nTimes >0)
	{
		ret = ReadFile(hSourceFile, dataBuffer, WRITE_BUF_LEN, &read_len, NULL);
		if(!ret || read_len != WRITE_BUF_LEN)
		{
			delete[] dataBuffer;
			dataBuffer = NULL;
			CloseHandle(hDstFile);
			return FALSE;	
		}
		
		ret = WriteFile(hDstFile, dataBuffer, WRITE_BUF_LEN, &write_len, NULL);
		if (!ret || write_len != WRITE_BUF_LEN)
		{
			delete[] dataBuffer;
			dataBuffer = NULL;
			CloseHandle(hDstFile);
			return FALSE;
		}
		FlushFileBuffers(hDstFile);
		nTimes--;	
	}
	//ReadFile
	ret = ReadFile(hSourceFile, dataBuffer, nSpare, &read_len, NULL);
	if(!ret || read_len != nSpare)
	{
		delete[] dataBuffer;
		dataBuffer = NULL;
		CloseHandle(hDstFile);
		return FALSE;	
	}
	//WriteFile
	ret = WriteFile(hDstFile, dataBuffer, nSpare, &write_len, NULL);
	if (!ret || write_len != nSpare)
	{
		delete[] dataBuffer;
		dataBuffer = NULL;
		CloseHandle(hDstFile);
		return FALSE;
	}
	FlushFileBuffers(hDstFile);//FlushFileBuffers
	
	delete[] dataBuffer;
	dataBuffer = NULL;
	
	CloseHandle(hDstFile);//CloseHandle
	return TRUE;
}

//Unpacket nand file
BOOL CUpdate::UnpacketNandFile_linux(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_NAND* pDownloadNand)
{	
	T_FILE_INFO_LINUX* pFileInfo = NULL;
	UINT data_size, i;
	CString strFilePath;
	
	//read nand files info
	pFileInfo = new T_FILE_INFO_LINUX[pFileHead->file_count];
	data_size = sizeof(T_FILE_INFO_LINUX) * pFileHead->file_count;
	if (NULL == pFileInfo)
	{
		return FALSE;
	}
	
	//ReadBuffer
	if (!ReadBuffer(hSourceFile,pFileInfo,pFileHead->file_info_offset,data_size))
	{
		delete[] pFileInfo;
		pFileInfo = NULL;
		return FALSE;
	}
	
	USES_CONVERSION;
	
	// unpacket nand files
	for(i=0; i<pFileHead->file_count; i++)
	{
		if(pFileInfo[i].check_sum != Cal_CheckSum((PBYTE)(&pFileInfo[i]), sizeof(pFileInfo[i])-sizeof(pFileInfo[i].check_sum)))
		{
			delete[] pFileInfo;
			pFileInfo = NULL;
			return FALSE;
		}
		
		//get download to nand info
		pDownloadNand[i].bCompare = (BOOL)pFileInfo[i].bCompare;
		pDownloadNand[i].bBackup = (BOOL)pFileInfo[i].bBack;
		pDownloadNand[i].bin_revs_size = (FLOAT)pFileInfo[i].bin_len;
		
		pDownloadNand[i].ld_addr = pFileInfo[i].ld_addr;
		
		wcsncpy(pDownloadNand[i].file_name, A2T(pFileInfo[i].file_name), 16);
		
		strFilePath = m_update_folder + pDownloadNand[i].file_name;
		strFilePath += ".bin";
		wcscpy(pDownloadNand[i].pc_path, strFilePath);
		
		//UnpacketFile
		if(!UnpacketFile(hSourceFile, pDownloadNand[i].pc_path, 
			pFileInfo[i].file_offset, pFileInfo[i].file_length))
			
		{
			delete[] pFileInfo;
			pFileInfo = NULL;
			return FALSE;
		}
	}
	
	delete[] pFileInfo;
	pFileInfo = NULL;
	return TRUE;
}


//Unpacket nand file
BOOL CUpdate::UnpacketNandFile(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_NAND* pDownloadNand)
{	
	T_FILE_INFO* pFileInfo = NULL;
	UINT data_size, i;
	CString strFilePath;

	//read nand files info
	pFileInfo = new T_FILE_INFO[pFileHead->file_count];
	data_size = sizeof(T_FILE_INFO) * pFileHead->file_count;
	if (NULL == pFileInfo)
	{
		return FALSE;
	}

	//ReadBuffer
	if (!ReadBuffer(hSourceFile,pFileInfo,pFileHead->file_info_offset,data_size))
	{
		delete[] pFileInfo;
		pFileInfo = NULL;
		return FALSE;
	}

	USES_CONVERSION;

	// unpacket nand files
	for(i=0; i<pFileHead->file_count; i++)
	{
		if(pFileInfo[i].check_sum != Cal_CheckSum((PBYTE)(&pFileInfo[i]), sizeof(pFileInfo[i])-sizeof(pFileInfo[i].check_sum)))
		{
			delete[] pFileInfo;
			pFileInfo = NULL;
			return FALSE;
		}
		
		//get download to nand info
		pDownloadNand[i].bCompare = (BOOL)pFileInfo[i].bCompare;
		pDownloadNand[i].bBackup = (BOOL)pFileInfo[i].bBack;
		pDownloadNand[i].bin_revs_size = (FLOAT)pFileInfo[i].bin_len;

		pDownloadNand[i].ld_addr = pFileInfo[i].ld_addr;

		wcsncpy(pDownloadNand[i].file_name, A2T(pFileInfo[i].file_name), 16);
		
		strFilePath = m_update_folder + pDownloadNand[i].file_name;
		strFilePath += ".bin";
		wcscpy(pDownloadNand[i].pc_path, strFilePath);
	
		//UnpacketFile
		if(!UnpacketFile(hSourceFile, pDownloadNand[i].pc_path, 
			                pFileInfo[i].file_offset, pFileInfo[i].file_length))

		{
			delete[] pFileInfo;
			pFileInfo = NULL;
			return FALSE;
		}
	}

	delete[] pFileInfo;
	pFileInfo = NULL;
	return TRUE;
}

//unpacket udisk files
BOOL CUpdate::UnpacketUdiskFile(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_UDISK * pDownloadUdisk)
{
	UINT i, data_size, file_offset, file_len;
	TCHAR file_path[MAX_PATH];
	T_UDISK_UPDATE_FILE_INFO * pUdiskFileInfo = NULL;
	T_UDISK_INFO *pUdiskInfo = NULL;
	CString strUpdatePath;

	if(INVALID_HANDLE_VALUE == hSourceFile || NULL == pFileHead)
	{
		return FALSE;
	}

	pUdiskInfo = new T_UDISK_INFO[pFileHead->udisk_info_count];//pUdiskInfo分配
	data_size = pFileHead->udisk_info_count * sizeof(T_UDISK_INFO);
	if (NULL == pUdiskInfo)
	{
		return FALSE;
	}
	//ReadBuffer
	if(!ReadBuffer(hSourceFile, pUdiskInfo, pFileHead->udisk_info_offset, data_size))
	{
		delete[] pUdiskInfo;
		pUdiskInfo = NULL;
		return FALSE;
	}

	USES_CONVERSION;

	for(i=0; i<pFileHead->udisk_info_count; i++)
	{
		memset(file_path, 0, MAX_PATH);
		
		if(pUdiskInfo[i].check_sum != Cal_CheckSum(PBYTE(pUdiskInfo + i), sizeof(pUdiskInfo[i]) - sizeof(pUdiskInfo[i].check_sum)))
		{
			delete[] pUdiskInfo;
			pUdiskInfo = NULL;
			return FALSE;
		}

		pDownloadUdisk[i].bCompare = pUdiskInfo[i].bCompare;
		wcscpy(pDownloadUdisk[i].udisk_path, A2T(pUdiskInfo[i].udisk_path));//udisk_path

		wcscpy(file_path, A2T(pUdiskInfo[i].pc_path));//pc_path
		strUpdatePath = m_update_folder + file_path;
		wcscpy(file_path, strUpdatePath);
		wcscpy(pDownloadUdisk[i].pc_path, file_path);	
	}

	delete[] pUdiskInfo;//释放
	pUdiskInfo = NULL;

	// read udisk files info
	pUdiskFileInfo = new T_UDISK_UPDATE_FILE_INFO[pFileHead->udisk_file_count];
	data_size = pFileHead->udisk_file_count * sizeof(T_UDISK_UPDATE_FILE_INFO);
	if (NULL == pUdiskFileInfo)
	{
		return FALSE;
	}
	//ReadBuffer
	if (!ReadBuffer(hSourceFile, pUdiskFileInfo, pFileHead->udisk_file_info_offset, data_size))
	{	
		delete[] pUdiskFileInfo;
		pUdiskFileInfo = NULL;
		return FALSE;
	}

	//unpacket udisk files
	for(i=0; i<pFileHead->udisk_file_count; i++)
	{
		memset(file_path, 0, MAX_PATH);
		
		if(pUdiskFileInfo[i].check_sum != Cal_CheckSum(PBYTE(&pUdiskFileInfo[i]),
										sizeof(pUdiskFileInfo[i])-sizeof(pUdiskFileInfo[i].check_sum)))
		{
			delete[] pUdiskFileInfo;
			pUdiskFileInfo = NULL;
			return FALSE;
		}
		wcsncpy(file_path, A2T(pUdiskFileInfo[i].pc_file_path), MAX_PATH);
		strUpdatePath = m_update_folder + file_path;
		wcscpy(file_path, strUpdatePath);
		
		file_offset = pUdiskFileInfo[i].file_offset;
		file_len = pUdiskFileInfo[i].file_length;
		
		if(!UnpacketFile(hSourceFile, file_path, file_offset, file_len))//UnpacketFile
		{
			delete[] pUdiskFileInfo;
			pUdiskFileInfo = NULL;
			return FALSE;
		}
	}
	delete[] pUdiskFileInfo;
	pUdiskFileInfo = NULL;
	return TRUE;

}

//get file count
UINT CUpdate::GetFileCnt(CString path, UINT* pFileCnt)
{
	BOOL bFind;
	CString strPath = path;
	CFileFind *fd = new CFileFind();

	bFind = fd->FindFile(strPath);

	while(bFind)
	{
		bFind = fd->FindNextFile();//FindNextFile
		strPath = fd->GetFilePath();//GetFilePath
		if(!fd->IsDots())
		{
			if(fd->IsDirectory())//IsDirectory
			{		
				strPath +=_T("\\*.*");	
				GetFileCnt(strPath, pFileCnt);
			}
			else
			{
				(*pFileCnt)++;
			}
		}
	}

	delete fd;
	fd = NULL;
	return *pFileCnt;
}

//get download to udisk file count 
UINT CUpdate::GetUdiskFileCount(CConfig *pCFG)
{
	UINT i, nFileCnt=0;
	
	for (i=0; i<pCFG->download_udisk_count; i++)
	{
		CString strPath = theApp.ConvertAbsolutePath(pCFG->download_udisk_data[i].pc_path);
		GetFileCnt(strPath, &nFileCnt);		//GetFileCnt
	}
	return nFileCnt;
}

//set downdload to udisk file info
void CUpdate::SetUdiskFileInfo(CString strPath, T_UDISK_UPDATE_FILE_INFO *pUdisFileInfo, UINT *pFileCnt)
{
	CString strFilePath = theApp.ConvertAbsolutePath(strPath);
//	static i = 0;
	BOOL bFind;
	CFileFind fd;

	USES_CONVERSION;
	
    bFind = fd.FindFile(strFilePath);
	while(bFind)
	{
		bFind = fd.FindNextFile();//FindNextFile
		strFilePath = fd.GetFilePath();//GetFilePath
		if (!fd.IsDots())
		{	
			if (fd.IsDirectory())
			{	
				strFilePath += _T("\\*.*");
				SetUdiskFileInfo(strFilePath, pUdisFileInfo, pFileCnt);//SetUdiskFileInfo
			}
			else
			{
				//get file info
				strcpy(pUdisFileInfo[*pFileCnt].pc_file_path, T2A((LPCTSTR)strFilePath)); 
		
				pUdisFileInfo[*pFileCnt].file_length = fd.GetLength();//GetLength

				if(0 == *pFileCnt)
				{
					pUdisFileInfo[*pFileCnt].file_offset = m_nand_file_offset;
				}
				else
				{
					pUdisFileInfo[*pFileCnt].file_offset = pUdisFileInfo[*pFileCnt-1].file_offset 
						                                 + ADJUST_512(pUdisFileInfo[*pFileCnt-1].file_length);
				}

				(*pFileCnt)++;
			}
		}
	}
}

//store file
BOOL CUpdate::StoreFile(HANDLE hPacketFile, TCHAR* file_path, UINT file_offset, UINT file_len)
{
	HANDLE hDstFile = NULL;
	BOOL ret;
	PBYTE pBuf = NULL;

	if(NULL == file_path || NULL == hPacketFile)
	{
		return FALSE;
	}

	SetFilePointer(hPacketFile, file_offset, NULL, CFile::begin);//SetFilePointer
	hDstFile = CreateFile(theApp.ConvertAbsolutePath(file_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
		    		OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
	if(INVALID_HANDLE_VALUE == hDstFile)
	{
		return FALSE;
	}

	DWORD read_len, write_len;
	UINT nTimes = file_len / WRITE_BUF_LEN;
	UINT nSpare = file_len % WRITE_BUF_LEN;

	pBuf = new BYTE[WRITE_BUF_LEN];
    if(NULL == pBuf)
    {
		CloseHandle(hDstFile);//CloseHandle
		return FALSE;
    }
	memset(pBuf, 0, WRITE_BUF_LEN);
	
	while(nTimes >0)
	{
		ret = ReadFile(hDstFile, pBuf, WRITE_BUF_LEN, &read_len, NULL);//ReadFile
		if(!ret || read_len != WRITE_BUF_LEN)
		{
			delete[] pBuf;
			pBuf = NULL;
			CloseHandle(hDstFile);
			return FALSE;
		}

		ret = WriteFile(hPacketFile, pBuf, WRITE_BUF_LEN, &write_len,NULL);//WriteFile

		if (!ret || write_len != WRITE_BUF_LEN)
		{
			delete[] pBuf;
			pBuf = NULL;
			CloseHandle(hDstFile);
			return FALSE;
		}

		FlushFileBuffers(hPacketFile);
		nTimes--;
	}

	ret = ReadFile(hDstFile, pBuf, nSpare, &read_len, NULL);//ReadFile

	if(!ret || read_len != nSpare)
	{
		delete[] pBuf;
		pBuf = NULL;
		CloseHandle(hDstFile);
		return FALSE;
	}
	
	ret = WriteFile(hPacketFile, pBuf, nSpare,&write_len,NULL);//WriteFile
	
	if (!ret || write_len != nSpare)
	{
		delete[] pBuf;
		pBuf = NULL;
		CloseHandle(hDstFile);
		return FALSE;
	}
	
	FlushFileBuffers(hPacketFile);//FlushFileBuffers

	CloseHandle(hDstFile);//CloseHandle
	delete[] pBuf;
	pBuf = NULL;

	return TRUE;
}

//strore producer
BOOL CUpdate::StoreProducer(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
	pFileHead->producer_offset = pFileHead->file_info_offset + ADJUST_512(pFileHead->file_count * sizeof(T_FILE_INFO));

	HANDLE hProducerFile = CreateFile(theApp.ConvertAbsolutePath(pCFG->path_producer), GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//CreateFile

	if(hProducerFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hProducerFile, NULL);
		if(dwSize != 0xFFFFFFFF)
		{
			pFileHead->producer_size = dwSize;//producer_size
		}
		else
		{
			CloseHandle(hProducerFile);//CloseHandle
			return FALSE;
		}
		CloseHandle(hProducerFile);//CloseHandle
	}
	else
	{
		CString str;
		str.Format(_T("打开文件:%s失败"),theApp.ConvertAbsolutePath(pCFG->path_producer));
		AfxMessageBox(str);
	}

	if(!StoreFile(hPacketFile, pCFG->path_producer, pFileHead->producer_offset, pFileHead->producer_size))
	{
		return FALSE;
	}

	return TRUE;
	
}

// store bios
BOOL CUpdate::StoreBios(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
	pFileHead->bios_offset = pFileHead->producer_offset + ADJUST_512(pFileHead->producer_size);;
	
	HANDLE hBiosFile = CreateFile(theApp.ConvertAbsolutePath(pCFG->path_nandboot), GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL, NULL);//CreateFile

	if(hBiosFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = GetFileSize(hBiosFile, NULL);//GetFileSize
		if(dwSize != 0xFFFFFFFF)
		{
			pFileHead->bios_size = dwSize;//bios_size
		}
		else
		{
			CloseHandle(hBiosFile);//CloseHandle
			return FALSE;
		}
		CloseHandle(hBiosFile);//CloseHandle
	}
	else
	{
		CString str;
		str.Format(_T("打开文件:%s失败"),theApp.ConvertAbsolutePath(pCFG->path_nandboot));
		AfxMessageBox(str);
	}

	if(!StoreFile(hPacketFile, pCFG->path_nandboot, pFileHead->bios_offset, pFileHead->bios_size))
	{
		return FALSE;
	}

	return TRUE;
}

//store nand files info and files
BOOL CUpdate::StoreNandFile_linux(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
    T_FILE_INFO_LINUX *pfInfo = NULL;
    UINT file_offset = 0;
	UINT file_info_len;
    HANDLE hFile;
    UINT i;
    DWORD dwSize;

	if(NULL == hPacketFile)
    {
        return FALSE;
    }

    pfInfo = new T_FILE_INFO_LINUX[pFileHead->file_count];
    if(NULL == pfInfo)
    {
        return FALSE;
    }
	
	file_info_len = pFileHead->file_count * sizeof(T_FILE_INFO_LINUX);
	memset(pfInfo, 0, file_info_len);

    USES_CONVERSION;

    file_offset = pFileHead->bios_offset + ADJUST_512(pFileHead->bios_size);

    for(i = 0; i < pFileHead->file_count; i++)
    {
        pfInfo[i].bCompare = (BYTE)pCFG->download_nand_data[i].bCompare;//bCompare
		pfInfo[i].bBack = (BYTE)pCFG->download_nand_data[i].bBackup;   //bBackup
		pfInfo[i].resev1 = 0;   //bin_revs_size
        pfInfo[i].resev2 = 0;
		pfInfo[i].ld_addr = pCFG->download_nand_data[i].ld_addr;//ld_addr
		pfInfo[i].bin_len = (FLOAT)pCFG->download_nand_data[i].bin_revs_size;
        
        hFile = CreateFile(theApp.ConvertAbsolutePath(pCFG->download_nand_data[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
		    		OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pfInfo[i].file_length = dwSize;        // 
                pfInfo[i].file_offset = file_offset;//
                file_offset += ADJUST_512(dwSize);//
		    }
            else
            {
                CloseHandle(hFile);
                delete[] pfInfo;
				pfInfo = NULL;
                return FALSE;
            }
        }
        else
        {		
			CString str;
			str.Format(_T("打开文件:%s失败"),theApp.ConvertAbsolutePath(pCFG->download_nand_data[i].pc_path));
			AfxMessageBox(str);
			
            delete[] pfInfo;
            pfInfo = NULL;
            return FALSE;
        }

        CloseHandle(hFile);

        //file name

		if(0 == wcscmp(pCFG->download_nand_data[i].file_name, IMAGE_RES_NAME))
		{
			pFileHead->config_tool[0][0] = pfInfo[i].file_offset;//
			pFileHead->config_tool[0][1] = dwSize;//
		}

		if(0 == wcscmp(pCFG->download_nand_data[i].file_name, SPOTLIGHT_NAME))
		{
			pFileHead->config_tool[1][0] = pfInfo[i].file_offset;//
			pFileHead->config_tool[1][1] = dwSize;//
		}
		
        strncpy(pfInfo[i].file_name, T2A(pCFG->download_nand_data[i].file_name), 15);
		pfInfo[i].check_sum = Cal_CheckSum((PBYTE)(pfInfo+i), sizeof(T_FILE_INFO_LINUX)-sizeof(UINT));
    }

	//get all nand file offset
	m_nand_file_offset = file_offset;

    //write file info
	//
	if (!WriteBuffer(hPacketFile, pfInfo, pFileHead->file_info_offset, file_info_len))
	{
		delete[] pfInfo;
		pfInfo = NULL;
		return FALSE;
	}

    //write all nand file

    for(i = 0; i < pFileHead->file_count; i++)
    {
		//StoreFile
		if(!StoreFile(hPacketFile, pCFG->download_nand_data[i].pc_path, pfInfo[i].file_offset, pfInfo[i].file_length))
		{
			delete[] pfInfo;
			pfInfo = NULL;
			return FALSE;
		}
	}
    delete[] pfInfo;
	pfInfo = NULL;
	
    return TRUE;
}

//store nand files info and files
BOOL CUpdate::StoreNandFile(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
    T_FILE_INFO *pfInfo = NULL;
    UINT file_offset = 0;
	UINT file_info_len;
    HANDLE hFile;
    UINT i;
    DWORD dwSize;

	if(NULL == hPacketFile)
    {
        return FALSE;
    }

    pfInfo = new T_FILE_INFO[pFileHead->file_count];
    if(NULL == pfInfo)
    {
        return FALSE;
    }
	
	file_info_len = pFileHead->file_count * sizeof(T_FILE_INFO);
	memset(pfInfo, 0, file_info_len);

    USES_CONVERSION;

    file_offset = pFileHead->bios_offset + ADJUST_512(pFileHead->bios_size);

    for(i = 0; i < pFileHead->file_count; i++)
    {
        pfInfo[i].bCompare = (BYTE)pCFG->download_nand_data[i].bCompare;//bCompare
		pfInfo[i].bBack = (BYTE)pCFG->download_nand_data[i].bBackup;   //bBackup
		pfInfo[i].bin_len = (BYTE)pCFG->download_nand_data[i].bin_revs_size;   //bin_revs_size
        pfInfo[i].resev = 0;
		pfInfo[i].ld_addr = pCFG->download_nand_data[i].ld_addr;//ld_addr
        
        hFile = CreateFile(theApp.ConvertAbsolutePath(pCFG->download_nand_data[i].pc_path), GENERIC_READ , FILE_SHARE_READ , NULL , 
		    		OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			dwSize = GetFileSize(hFile, NULL);
			if(dwSize != 0xFFFFFFFF)
			{
				pfInfo[i].file_length = dwSize;        // 
                pfInfo[i].file_offset = file_offset;//
                file_offset += ADJUST_512(dwSize);//
		    }
            else
            {
                CloseHandle(hFile);
                delete[] pfInfo;
				pfInfo = NULL;
                return FALSE;
            }
        }
        else
        {		
			CString str;
			str.Format(_T("打开文件:%s失败"),theApp.ConvertAbsolutePath(pCFG->download_nand_data[i].pc_path));
			AfxMessageBox(str);
			
            delete[] pfInfo;
            pfInfo = NULL;
            return FALSE;
        }

        CloseHandle(hFile);

        //file name

		if(0 == wcscmp(pCFG->download_nand_data[i].file_name, IMAGE_RES_NAME))
		{
			pFileHead->config_tool[0][0] = pfInfo[i].file_offset;//
			pFileHead->config_tool[0][1] = dwSize;//
		}

		if(0 == wcscmp(pCFG->download_nand_data[i].file_name, SPOTLIGHT_NAME))
		{
			pFileHead->config_tool[1][0] = pfInfo[i].file_offset;//
			pFileHead->config_tool[1][1] = dwSize;//
		}
		
        strncpy(pfInfo[i].file_name, T2A(pCFG->download_nand_data[i].file_name), 15);
		pfInfo[i].check_sum = Cal_CheckSum((PBYTE)(pfInfo+i), sizeof(T_FILE_INFO)-sizeof(UINT));
    }

	//get all nand file offset
	m_nand_file_offset = file_offset;

    //write file info
	//
	if (!WriteBuffer(hPacketFile, pfInfo, pFileHead->file_info_offset, file_info_len))
	{
		delete[] pfInfo;
		pfInfo = NULL;
		return FALSE;
	}

    //write all nand file

    for(i = 0; i < pFileHead->file_count; i++)
    {
		//StoreFile
		if(!StoreFile(hPacketFile, pCFG->download_nand_data[i].pc_path, pfInfo[i].file_offset, pfInfo[i].file_length))
		{
			delete[] pfInfo;
			pfInfo = NULL;
			return FALSE;
		}
	}
    delete[] pfInfo;
	pfInfo = NULL;
	
    return TRUE;
}


//write download_to_udisk info and files
BOOL CUpdate::StoreUdiskFile(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead)
{
	UINT i, dwSize, nUdiskFileTotal = 0;
	T_UDISK_INFO *pUdiskInfo = NULL;            //download_to_udisk info
	T_UDISK_UPDATE_FILE_INFO* pUdiskFileInfo = NULL;   //download_to_udisk file info
	T_DOWNLOAD_UDISK* pudisk = pCFG->download_udisk_data;

	if(NULL == hPacketFile)
	{
		return FALSE;
	}

	pUdiskFileInfo = new T_UDISK_UPDATE_FILE_INFO[pFileHead->udisk_file_count];//pUdiskFileInfo
	if(NULL == pUdiskFileInfo)
	{
		return FALSE;
	}
	memset(pUdiskFileInfo, 0, pFileHead->udisk_file_count * sizeof(T_UDISK_UPDATE_FILE_INFO));

	USES_CONVERSION;
	
	//fill struct pUdiskFileInfo to store udisk file
	for (i=0; i<pFileHead->udisk_info_count; i++)
	{
		SetUdiskFileInfo(pudisk[i].pc_path, pUdiskFileInfo, &nUdiskFileTotal);//SetUdiskFileInfo
	
		if ((pFileHead->udisk_info_count != 0) && (0 == nUdiskFileTotal))
		{
			delete[] pUdiskFileInfo;
			pUdiskFileInfo = NULL;
	//		AfxMessageBox(_T("没找到下载的优盘文件"));
            return FALSE;
		}
	}

	//write all file of download_to_udisk 
	for (i=0; i<pFileHead->udisk_file_count; i++)
	{
		//StoreFile
		if(!StoreFile(hPacketFile, A2T(pUdiskFileInfo[i].pc_file_path), pUdiskFileInfo[i].file_offset, pUdiskFileInfo[i].file_length))
        {
			delete[] pUdiskFileInfo;
			pUdiskFileInfo = NULL;
            return FALSE;
		}
    }

	pUdiskInfo = new T_UDISK_INFO[pFileHead->udisk_info_count];//pUdiskInfo
	if(NULL == pUdiskInfo)
	{
		delete[] pUdiskFileInfo;
		pUdiskFileInfo = NULL;
		return FALSE;
	}
	memset(pUdiskInfo, 0, pFileHead->udisk_info_count * sizeof(T_UDISK_INFO));

	
	UINT nindex, j, nFileCnt, nFileTotal=0;

	//fill struct pUdiskInfo and modify pc_file_path of struct pUdiskFileInfo to store
	for (i=0; i<pFileHead->udisk_info_count; i++)
	{
		CString strUdiskInfo, strFront, strtemp;
		BOOL bSame = FALSE;
		
		strFront.Format(_T("UDISK%d_"), i+1);

		CFileFind fd;
		if (fd.FindFile(theApp.ConvertAbsolutePath(pudisk[i].pc_path)))//FindFile
		{
			fd.FindNextFile();
			strUdiskInfo = fd.GetFilePath();
		}

		nFileCnt = 0;
		nFileCnt = GetFileCnt(strUdiskInfo, &nFileCnt);//GetFileCnt
			
		nindex = strUdiskInfo.ReverseFind('\\');
		strUdiskInfo = strUdiskInfo.Mid(nindex+1);
		
		if(i>0)
		{
			for(UINT k=0; k<i; k++)
			{
				strtemp = A2T(pUdiskInfo[i-1].pc_path);
				if(0 == strtemp.Compare(strUdiskInfo))
				{
					strUdiskInfo = strFront + strUdiskInfo;
					bSame = TRUE;
					break;
				}		
			}		
		}
	
		pUdiskInfo[i].bCompare = pudisk[i].bCompare;//bCompare
		strcpy(pUdiskInfo[i].udisk_path, T2A(pudisk[i].udisk_path));//udisk_path
		strcpy(pUdiskInfo[i].pc_path, T2A((LPCTSTR)strUdiskInfo)/*, strUdiskInfo.GetLength()*/);
		pUdiskInfo[i].check_sum = Cal_CheckSum(PBYTE(pUdiskInfo + i), sizeof(T_UDISK_INFO)-sizeof(UINT));
			
		for (j = 0; j<nFileCnt; j++)
		{	
			CString strUdiskFile;
			strUdiskFile = A2T(pUdiskFileInfo[j + nFileTotal].pc_file_path);//strUdiskFile
			strUdiskFile = strUdiskFile.Mid(nindex+1);
			
			if (bSame)
			{
				strUdiskFile = strFront + strUdiskFile;//strUdiskFile
			}
			
		
			memset(pUdiskFileInfo[j + nFileTotal].pc_file_path, 0, MAX_PATH);
			strcpy(pUdiskFileInfo[j + nFileTotal].pc_file_path, T2A((LPCTSTR)strUdiskFile)/*, strUdiskFile.GetLength()*/);

			pUdiskFileInfo[j + nFileTotal].check_sum = 0;
			pUdiskFileInfo[j + nFileTotal].check_sum	= Cal_CheckSum((PBYTE)(pUdiskFileInfo + j + nFileTotal), sizeof(T_UDISK_UPDATE_FILE_INFO)-sizeof(UINT));//
		}
		
		nFileTotal += nFileCnt;	
	}


	//write download_to_udisk info

	dwSize = pFileHead->udisk_info_count * sizeof(T_UDISK_INFO);//
	if(!WriteBuffer(hPacketFile, pUdiskInfo, pFileHead->udisk_info_offset, dwSize))
	{
		delete[] pUdiskInfo;
		pUdiskInfo = NULL;
		return FALSE;
	}

	//write download_to_udisk files info

	dwSize = pFileHead->udisk_file_count * sizeof(T_UDISK_UPDATE_FILE_INFO);//dwSize
	if(!WriteBuffer(hPacketFile, pUdiskFileInfo, pFileHead->udisk_file_info_offset, dwSize))
	{
		delete[] pUdiskInfo;
		delete[] pUdiskFileInfo;
		pUdiskInfo = NULL;
		pUdiskFileInfo = NULL;
		return FALSE;
	}


	return TRUE;	
}

BOOL CUpdate::ExportUpdateFile(CConfig *pCFG, CString file_path, CString strCheck)
{
    if(NULL == pCFG || file_path.IsEmpty())
    {
        return FALSE;
    }
	
	HANDLE hPacketFile = NULL;
	T_UPDATE_FILE_HEAD file_head;
    UINT tmp, data_size, i;

	hPacketFile = CreateFile(file_path, GENERIC_WRITE, FILE_SHARE_WRITE, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (NULL == hPacketFile)
	{
		return FALSE;
	}

    //prepare for file head struct
    memset(&file_head, 0, sizeof(T_UPDATE_FILE_HEAD));
    memcpy(file_head.head_info, FILE_HEAD_INFO, 8);

	file_head.Other_config[0] = (UINT)pCFG->bUsb2;//bUsb2
	file_head.Other_config[1] = pCFG->nonfs_res_size;//nonfs_res_size
	file_head.Other_config[2] = pCFG->fs_res_blocks;//fs_res_blocks
	file_head.Other_config[4] = pCFG->bUpdateself;//bUpdateself
	file_head.Other_config[5] = pCFG->chip_snowbirdsE;//chip_snowbirdsE
    file_head.Other_config[6] = pCFG->burn_mode;//burn_mode
	
    //format list
    file_head.format_count = pCFG->format_count;
    file_head.format_offset = sizeof(T_UPDATE_FILE_HEAD);

    //nand list
	if (pCFG->burn_mode == E_CONFIG_SPI_NAND)
	{
		file_head.nand_count = pCFG->spi_nandflash_parameter_count;
	}
	else
	{
		file_head.nand_count = pCFG->nandflash_parameter_count;
	}
    file_head.nand_offset = 512;

    tmp = file_head.nand_count * sizeof(T_NAND_PHY_INFO_TRANSC);
/*
#if 0
	//spi list
    file_head.spi_count = pCFG->spiflash_parameter_count;
    file_head.spi_offset = file_head.nand_offset + ADJUST_512(tmp);
	
    tmp = file_head.spi_count * sizeof(T_SFLASH_PHY_INFO_TRANSC);
#endif
*/
	//udisk info
	file_head.udisk_info_count = pCFG->download_udisk_count;
	file_head.udisk_info_offset = file_head.nand_offset + ADJUST_512(tmp);

	tmp = file_head.udisk_info_count * sizeof(T_UDISK_INFO);
	
	//udisk file
	file_head.udisk_file_count = GetUdiskFileCount(pCFG);
	file_head.udisk_file_info_offset = file_head.udisk_info_offset + ADJUST_512(tmp);
 
	tmp = file_head.udisk_file_count * sizeof(T_UDISK_UPDATE_FILE_INFO);

//	USES_CONVERSION;
//	char strayy[10];
//	AfxMessageBox(A2T(itoa(file_head.udisk_file_count, strayy, 10)));
//	return false;

	file_head.Other_config[3] = file_head.udisk_file_info_offset + ADJUST_512(tmp);
	tmp = file_head.format_count * sizeof(T_VOLUME_LABLE);
	

	//file list
    file_head.file_count = pCFG->download_nand_count;
    file_head.file_info_offset = file_head.Other_config[3] + ADJUST_512(tmp);
	
     //store format list
	data_size = file_head.format_count * sizeof(T_PARTION_INFO);

	//linux平台中的spi烧录
	//获取spi_format_data
	//获取spi分区的信息
	if (pCFG->planform_tpye == E_LINUX_PLANFORM && pCFG->burn_mode == E_CONFIG_SFLASH)
	{
		for (i = 0; i < pCFG->format_count; i++)
		{
			pCFG->spi_format_data[i].Disk_Name = pCFG->format_data[i].Disk_Name;//盘符
			pCFG->spi_format_data[i].bOpenZone= pCFG->format_data[i].bOpenZone;//用户盘
			pCFG->spi_format_data[i].ProtectType= pCFG->format_data[i].ProtectType;//信息
			pCFG->spi_format_data[i].ZoneType= pCFG->format_data[i].ZoneType;//属性
			pCFG->spi_format_data[i].Size = pCFG->spi_format_data[i].Size;   //此变量不相同
			pCFG->spi_format_data[i].EnlargeSize= pCFG->format_data[i].EnlargeSize;//扩容
			pCFG->spi_format_data[i].HideStartBlock= pCFG->format_data[i].HideStartBlock;//开始块
			pCFG->spi_format_data[i].FSType= pCFG->format_data[i].FSType;//文件系统类型
			pCFG->spi_format_data[i].resv[0]= pCFG->format_data[i].resv[0];//保留
		}

		//WriteBuffer
		if (!WriteBuffer(hPacketFile, pCFG->spi_format_data, file_head.format_offset, data_size))
		{
			CloseHandle(hPacketFile);
			return FALSE;
		}

	}
	else
	{
		//WriteBuffer
		if (!WriteBuffer(hPacketFile, pCFG->format_data, file_head.format_offset, data_size))
		{
			CloseHandle(hPacketFile);
			return FALSE;
		}
	}

	//store format list
	data_size = file_head.format_count * sizeof(T_VOLUME_LABLE);
	if (!WriteBuffer(hPacketFile, pCFG->pVolumeLable, file_head.Other_config[3], data_size))
	{
		CloseHandle(hPacketFile);
		return FALSE;
	}

    //store nand list
	data_size = file_head.nand_count * sizeof(T_NAND_PHY_INFO_TRANSC);
	if (pCFG->burn_mode == E_CONFIG_SPI_NAND)
	{
		if (!WriteBuffer(hPacketFile, pCFG->spi_nandflash_parameter, file_head.nand_offset, data_size))
		{
			CloseHandle(hPacketFile);
			return FALSE;
		}
	}
	else
	{
		if (!WriteBuffer(hPacketFile, pCFG->nandflash_parameter, file_head.nand_offset, data_size))
		{
			CloseHandle(hPacketFile);
			return FALSE;
		}
	}
	
/*
#if 0
	//store spi list
	data_size = file_head.spi_count * sizeof(T_SFLASH_PHY_INFO_TRANSC);
	if (!WriteBuffer(hPacketFile, pCFG->spiflash_parameter, file_head.spi_offset, data_size))
	{
		CloseHandle(hPacketFile);
		return FALSE;
	}
#endif
	*/
		//store file
	if (pCFG->planform_tpye == E_LINUX_PLANFORM)
	{
		if(!StoreProducer(hPacketFile, pCFG, &file_head) 
			|| !StoreBios(hPacketFile, pCFG, &file_head)
			|| !StoreNandFile_linux(hPacketFile, pCFG, &file_head)
			|| !StoreUdiskFile(hPacketFile, pCFG, &file_head))
		{
			CloseHandle(hPacketFile);
			return FALSE;
		}
	}
	else
	{
		if(!StoreProducer(hPacketFile, pCFG, &file_head) 
			|| !StoreBios(hPacketFile, pCFG, &file_head)
			|| !StoreNandFile(hPacketFile, pCFG, &file_head)
			|| !StoreUdiskFile(hPacketFile, pCFG, &file_head))
		{
			CloseHandle(hPacketFile);
			return FALSE;
		}
	}
	

	//check sum
    file_head.check_sum = Cal_CheckSum((PBYTE)(&file_head), sizeof(file_head)-sizeof(file_head.check_sum));

	//store file head
	if (!WriteBuffer(hPacketFile, &file_head, 0, sizeof(T_UPDATE_FILE_HEAD)))
	{
		CloseHandle(hPacketFile);
		return FALSE;
	}

	//write user check string
	if (!WriteCheckExport(hPacketFile, strCheck))
	{
		CloseHandle(hPacketFile);
		return FALSE;
	}
	CloseHandle(hPacketFile);

    return TRUE;
}

BOOL CUpdate::ImportUpdateFile(CConfig *pCFG, CString file_path)
{
	HANDLE hSourceFile;
	UINT data_size;
	T_UPDATE_FILE_HEAD file_head;
	T_PARTION_INFO* pFormatData = NULL;
	T_SPIFLASH_PARTION_INFO* spi_pFormatData = NULL;
	T_VOLUME_LABLE* pVolumeLable = NULL;
	T_DOWNLOAD_NAND* pDownloadNand = NULL;
	T_NAND_PHY_INFO_TRANSC *pNandInfo = NULL;
	T_SFLASH_PHY_INFO_TRANSC *pspiInfo = NULL;
	
	T_DOWNLOAD_UDISK* pDownloadUdisk = NULL;
	T_UDISK_INFO *pUdiskInfo = NULL;
    BYTE *pCheckExport = NULL;

	if(pCFG == NULL || file_path.IsEmpty())
	{
		return FALSE;
	}

	hSourceFile = CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//CreateFile
	if(INVALID_HANDLE_VALUE == hSourceFile)
	{
		return FALSE;
	}
	
	//read file_head and check
	if(!ReadBuffer(hSourceFile, &file_head, 0, sizeof(T_UPDATE_FILE_HEAD)))
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}

	USES_CONVERSION;

	if((wcsncmp(_T(FILE_HEAD_INFO), A2T((LPSTR)file_head.head_info), 8) != 0) || (file_head.nand_offset > 512))
	{	
		CloseHandle(hSourceFile);
		return FALSE;
	}
	
	if(file_head.check_sum != Cal_CheckSum((PBYTE)(&file_head), sizeof(file_head)-sizeof(file_head.check_sum)))//Cal_CheckSum
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}

	//create empty update folder
	if(!CreateUpdateDir())//CreateUpdateDir
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}

	pCFG->bUsb2 = file_head.Other_config[0];//bUsb2
	pCFG->nonfs_res_size = file_head.Other_config[1];//nonfs_res_size
	pCFG->fs_res_blocks = file_head.Other_config[2];//fs_res_blocks
	pCFG->bUpdateself = file_head.Other_config[4];//bUpdateself
	pCFG->chip_snowbirdsE = file_head.Other_config[5];//chip_snowbirdsE
    pCFG->burn_mode = file_head.Other_config[6];//burn_mode

	CString strPath = m_update_folder + _T("producer.bin");
	if(!UnpacketFile(hSourceFile, strPath, file_head.producer_offset, file_head.producer_size))
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}

	memset(pCFG->path_producer, 0, MAX_PATH+1);
	wcscpy(pCFG->path_producer, strPath);//produce

	strPath = m_update_folder + _T("BOOT.bin");
	if(!UnpacketFile(hSourceFile, strPath, file_head.bios_offset, file_head.bios_size))
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}
	memset(pCFG->path_nandboot, 0, MAX_PATH+1);
	wcscpy(pCFG->path_nandboot, strPath);//boot path

	//download_to_nandflash struct
	pDownloadNand = new T_DOWNLOAD_NAND[file_head.file_count];//分配
	if(NULL == pDownloadNand )
	{
		CloseHandle(hSourceFile);
		return FALSE;
	}	
	memset(pDownloadNand, 0, file_head.file_count * sizeof(T_DOWNLOAD_NAND));
		
	if (pCFG->planform_tpye == E_LINUX_PLANFORM)
	{
		//unpacket download_to_nandflash files and fill pDownloadNand
		if(!UnpacketNandFile_linux(hSourceFile, &file_head, pDownloadNand))
		{
			goto FAIL;
		}
	}
	else
	{
		//unpacket download_to_nandflash files and fill pDownloadNand
		if(!UnpacketNandFile(hSourceFile, &file_head, pDownloadNand))
		{
			goto FAIL;
		}
	}
	

	//download_to_udisk struct
	pDownloadUdisk = new T_DOWNLOAD_UDISK[file_head.udisk_info_count];
	if (NULL == pDownloadUdisk)
	{
		goto FAIL;
	}
	memset(pDownloadUdisk, 0, file_head.udisk_info_count * sizeof(T_DOWNLOAD_UDISK));

	//unpacket Udisk files	
	if (!UnpacketUdiskFile(hSourceFile, &file_head, pDownloadUdisk))//UnpacketUdiskFile
	{
		goto FAIL;
	}

	//read FORMAT info
	pFormatData = new T_PARTION_INFO[file_head.format_count];//分配
	if (NULL == pFormatData)
	{
		goto FAIL;
	}

	//read FORMAT info
	spi_pFormatData = new T_SPIFLASH_PARTION_INFO[file_head.format_count];//分配
	if (NULL == spi_pFormatData)
	{
		goto FAIL;
	}

	data_size = file_head.format_count * sizeof(T_PARTION_INFO);
	//由于pFormatData是需要的，所以二个都要进行读取
	if(!ReadBuffer(hSourceFile, pFormatData, file_head.format_offset, data_size))
	{
		goto FAIL;
	}
	
	if(!ReadBuffer(hSourceFile, spi_pFormatData, file_head.format_offset, data_size))
	{
		goto FAIL;
	}
	
	pVolumeLable = new T_VOLUME_LABLE[file_head.format_count];//pVolumeLable
	if (NULL == pVolumeLable)
	{
		goto FAIL;
	}
	data_size = file_head.format_count * sizeof(T_VOLUME_LABLE);//data_size
	
	if(0 != file_head.Other_config[3])
	{
		if(!ReadBuffer(hSourceFile, pVolumeLable, file_head.Other_config[3], data_size))
		{
			goto FAIL;
		}	
	}
	else
	{
		memset(pVolumeLable, 0, data_size);
	}

/*
#if 0  //导入时去掉nand参数导入
	//read nand param info
	pNandInfo = new T_NAND_PHY_INFO_TRANSC[file_head.nand_count];
	if (NULL ==pNandInfo)
	{
		goto FAIL;
	}
	data_size = sizeof(T_NAND_PHY_INFO_TRANSC) * file_head.nand_count;

	if (!ReadBuffer(hSourceFile, pNandInfo, file_head.nand_offset, data_size)) 
	{
		goto FAIL;
	}
#endif

#if 0  //导入时去掉spi参数导入
	//read spi param info
	pspiInfo = new T_SFLASH_PHY_INFO_TRANSC[file_head.spi_count];
	if (NULL ==pspiInfo)
	{
		goto FAIL;
	}
	data_size = sizeof(T_SFLASH_PHY_INFO_TRANSC) * file_head.spi_count;
	
	if (!ReadBuffer(hSourceFile, pspiInfo, file_head.spi_offset, data_size)) 
	{
		goto FAIL;
	}
#endif
*/
    pCheckExport = new BYTE[64];
	if (NULL ==pCheckExport)
	{
		goto FAIL;
	}
    memset(pCheckExport, 0, 64);
    if (!UnpacketCheckExport(hSourceFile, pCheckExport))
    {
        delete [] pCheckExport;//释放
        pCheckExport = NULL;
    }
	CloseHandle(hSourceFile);


	//write config 
	pCFG->format_count = file_head.format_count;
	if(pCFG->format_data != NULL)
	{
		delete [] pCFG->format_data;//释放
	}
	pCFG->format_data = pFormatData;

	if(pCFG->spi_format_data != NULL)
	{
		delete [] pCFG->spi_format_data;//释放
	}
	pCFG->spi_format_data = spi_pFormatData;

	if(pCFG->pVolumeLable != NULL)
	{
		delete [] pCFG->pVolumeLable;//释放
	}
	pCFG->pVolumeLable = pVolumeLable;
/*
#if 0 //#if 0导入时去掉nand参数导入
	pCFG->nandflash_parameter_count = file_head.nand_count;
	if(pCFG->nandflash_parameter != NULL)
	{
		delete [] pCFG->nandflash_parameter;
	}
	pCFG->nandflash_parameter = pNandInfo;
#endif

#if 0 //#if 0 导入时去掉spi参数导入
	pCFG->spiflash_parameter_count = file_head.spi_count;
	if(pCFG->spiflash_parameter != NULL)
	{
		delete [] pCFG->spiflash_parameter;
	}
	pCFG->spiflash_parameter = pspiInfo;
#endif
*/
	pCFG->download_nand_count = file_head.file_count;
	if (pCFG->download_nand_data != NULL)
	{
		delete [] pCFG->download_nand_data;//释放
	}
	pCFG->download_nand_data = pDownloadNand;


	pCFG->download_udisk_count = file_head.udisk_info_count;
	if(pCFG->download_udisk_data != NULL)
	{
		delete [] pCFG->download_udisk_data;//释放
	}
	pCFG->download_udisk_data = pDownloadUdisk;

	if(pCFG->pCheckExport != NULL)
	{
		delete [] pCFG->pCheckExport;//释放
	}
	pCFG->pCheckExport = pCheckExport;
/*
	if (USER_MODE_RESEARCHER == theApp.GetUserMode())
	{
		pCFG->WriteConfig(CONFIG_FILE_NAME_RESEARCHER);
	}
	else
	{
		pCFG->WriteConfig(CONFIG_FILE_NAME_PRODUCER);
	}
	*/
    pCFG->WriteConfig(theApp.m_config_file[theApp.m_config_index]);

	return true;

//release memory and exit
FAIL:
	if(pDownloadNand != NULL)
	{
		delete [] pDownloadNand;
		pDownloadNand = NULL;
	}
	if(pFormatData != NULL)
	{
		delete [] pFormatData;
		pFormatData = NULL;
	}
	if(spi_pFormatData != NULL)
	{
		delete [] spi_pFormatData;
		spi_pFormatData = NULL;
	}

	if(pVolumeLable != NULL)
	{
		delete [] pVolumeLable;
		pVolumeLable = NULL;
	}
/*
#if 0 //导入时去掉nand参数导入
	if(pNandInfo != NULL)
	{
		delete pNandInfo;
		pNandInfo = NULL;
	}
#endif

#if 0 //导入时去掉nand参数导入
	if(pspiInfo != NULL)
	{
		delete pspiInfo;
		pspiInfo = NULL;
	}
#endif
*/	
	if(pDownloadUdisk != NULL)
	{
		delete [] pDownloadUdisk;
		pDownloadUdisk = NULL;
	}
	CloseHandle(hSourceFile);
	return FALSE;
}

BOOL CUpdate::WriteCheckExport(HANDLE hFile, CString strCheck)
{
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}
	
#ifndef CRC_CHECK
	UINT nLen = 0;
	BYTE strBuf[64] = {0};
	BOOL ret;
	DWORD write_len;
    UINT IDLen = strlen(CHECK_EXPORT_ID);

	USES_CONVERSION;
    
    if (IDLen > 64)
    {
        return FALSE;
    }

	nLen = strCheck.GetLength();//GetLength
    if ((IDLen + nLen) > 64)
    {
        nLen = 64 - IDLen;
    }

    memcpy(strBuf, CHECK_EXPORT_ID, IDLen);
	memcpy(&strBuf[IDLen], T2A(strCheck), nLen);

	SetFilePointer(hFile, 0, NULL, FILE_END);//SetFilePointer
	ret = WriteFile(hFile, strBuf, 64, &write_len, NULL);	//WriteFile
	if(!ret || write_len != 64)
	{
		return FALSE;
	}
#else
	BOOL ret;
	DWORD write_len;
 
	SetFilePointer(hFile, 0, NULL, FILE_END);//SetFilePointer
	ret = WriteFile(hFile, (PBYTE)&g_crc_check, sizeof(g_crc_check), &write_len, NULL);	
	if(!ret || write_len != sizeof(g_crc_check))
	{
		return FALSE;
	}
#endif

	FlushFileBuffers(hFile);//FlushFileBuffers

	return TRUE;
}

BOOL CUpdate::UnpacketCheckExport(HANDLE hSourceFile, BYTE str[])
{
	if (INVALID_HANDLE_VALUE == hSourceFile || NULL == str)
	{
		return FALSE;
	}
	
    BOOL ret;
	DWORD read_len;
    BYTE buf[64];
    UINT IDLen = strlen(CHECK_EXPORT_ID);//长度

    if (IDLen > 64)
    {
        return FALSE;
    }  
    
	memset(buf, 0, 64);
	SetFilePointer(hSourceFile, -64, NULL, FILE_END);//SetFilePointer
	ret = ReadFile(hSourceFile, buf, 64, &read_len, NULL);	//ReadFile
	if(!ret || read_len != 64)
	{
		return FALSE;
	}
    
    if (memcmp(buf, CHECK_EXPORT_ID, IDLen))//比较
    {
        return FALSE;
    }
    
    memcpy(str, &buf[IDLen], (64 - IDLen));
	return TRUE;
}
