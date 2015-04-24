/* @file IFileSource.h
 * @brief declare IFileSource class
 *
 * Copyright (C) 2012 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author
 * @date 2012-7-18
 * @version 1.0
 */

#ifndef IFILE_SOURCE_H_
#define IFILE_SOURCE_H_
#include <stdio.h>
namespace akmedia {

    class IFileSource {
        public:
            virtual int  open(const char *path) = 0;
            virtual void close() = 0;
            virtual int  read(void *buf, long len) = 0;
            virtual int  write(const void *ptr, long size) = 0;
            virtual int  seek(long offset, int whence) = 0;
            virtual int  tell() = 0;
            virtual unsigned int getLength() = 0;
    };

    class CFileSource: public IFileSource
    {
        public:
            virtual int  open(const char *path);
            virtual void close();
            virtual int  read(void *buf, long len);
            virtual int  write(const void *ptr, long size);
            virtual int  seek(long offset, int whence);
            virtual int  tell();
            virtual unsigned int getLength();

            /* 
             * 构造函数
             * 参数：
             * 无
             * 返回值：
             * 无
             */
            CFileSource();

            /* 
             * 构造函数
             * 参数：
             * path[in]:文件名
             * 返回值：
             * 无
             */
            CFileSource(const char *path);

            /* 
             * 构造函数
             * 参数：
             * fd[in]:文件句柄
             * 返回值：
             * 无
             */
            CFileSource(FILE * fd);

            /* 
             * 析构函数，关闭文件句柄，释放内存
             * 参数：
             * 无
             * 返回值：
             * 无
             */
            ~CFileSource( );

            /* 
             * 获取文件名
             * 参数：
             * 无
             * 返回值：
             * NULL: 失败
             * 其它：文件名指针
             * 注意：用户要拷贝该字符串内容，而且不能free此指针
             */
            char * getName();

        private:
            /*
             * a CFileSoure can't copy
             */
            CFileSource(const CFileSource &);
            CFileSource & operator = (const CFileSource &);

            FILE * m_file;
            char * filename;

    };

}

#endif  // IFILE_SOURCE_H_

