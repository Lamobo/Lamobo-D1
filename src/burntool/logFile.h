
#ifndef  _LOG_FILE_H_
#define  _LOG_FILE_H_

#define  LOG_LINE_TIME  1
#define  LOG_LINE_DATE  (1<<1)

class CLogFile
{
public:
    CLogFile();   
    CLogFile(TCHAR *name);   
    ~CLogFile();

protected:
    HANDLE  hLogFile;
    TCHAR   file_name[MAX_PATH];

public:
    BOOL    InitFile();//实始化
    DWORD   WriteLogFile(UCHAR wFlag, const char *fmt, ...);//写数据
    UINT    ReadLogFile();//读LOG
    VOID    SetFileName(TCHAR *name);//设置文件名
    BOOL    CheckFileSize(DWORD file_max_size);//检查文件大小

protected:
    BOOL    GetCurPCPath(PTCHAR curPCPath, int buf_len);//获取当前路径
};

#endif