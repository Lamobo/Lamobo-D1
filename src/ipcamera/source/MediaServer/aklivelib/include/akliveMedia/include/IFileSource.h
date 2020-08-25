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
             * ���캯��
             * ������
             * ��
             * ����ֵ��
             * ��
             */
            CFileSource();

            /* 
             * ���캯��
             * ������
             * path[in]:�ļ���
             * ����ֵ��
             * ��
             */
            CFileSource(const char *path);

            /* 
             * ���캯��
             * ������
             * fd[in]:�ļ����
             * ����ֵ��
             * ��
             */
            CFileSource(FILE * fd);

            /* 
             * �����������ر��ļ�������ͷ��ڴ�
             * ������
             * ��
             * ����ֵ��
             * ��
             */
            ~CFileSource( );

            /* 
             * ��ȡ�ļ���
             * ������
             * ��
             * ����ֵ��
             * NULL: ʧ��
             * �������ļ���ָ��
             * ע�⣺�û�Ҫ�������ַ������ݣ����Ҳ���free��ָ��
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

