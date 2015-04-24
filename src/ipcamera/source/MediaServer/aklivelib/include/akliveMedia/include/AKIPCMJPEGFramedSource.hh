#ifndef _AKIPC_MJPEG_FRAMED_SOURCE_
#define _AKIPC_MJPEG_FRAMED_SOURCE_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <liveMedia.hh>
#include <sys/types.h>
#include <sys/syscall.h>


typedef int (*GetFrameFunc)(void* pbuf, unsigned* size, int nNeedIFrameCont, struct timeval* tv, int vsIndex);
typedef void (*SetLed)(int index);
class AKIPCMJPEGFramedSource: public JPEGVideoSource
{
	int m_started;
	int m_nNeedIFrameCount;
	GetFrameFunc m_getframefunc;
	int findex;
	int m_nWidth;
	int m_nHeight;
	int m_nType;
	int m_nQFactor;
	
	SetLed m_setledstart;
	SetLed m_setledexit;
	
public:
	void setGetFrameFunc(GetFrameFunc func);
	void setSetLed(SetLed funcstart, SetLed funcexit);
	void setIndex(int index);
	void setIFrameCount(int n);

	void setWidth(int nWidth)
	{
		m_nWidth = nWidth;
	};
	void setHeight(int nHeight)
	{
		m_nHeight = nHeight;
	};
	AKIPCMJPEGFramedSource(UsageEnvironment &env, int nType)
		:JPEGVideoSource(env)
	{
		m_started = 0;
		m_nNeedIFrameCount = 0;
		m_nType = 1;
		m_nQFactor = 128;
		m_nWidth = 640;
		m_nHeight = 480;
		findex = 0;
	};
	virtual u_int8_t type()
	{
		return m_nType; 
	};
	virtual u_int8_t qFactor()
	{
		return m_nQFactor;
	};
	virtual u_int8_t width()
	{
		return m_nWidth / 8;	
	};
	virtual u_int8_t height()
	{
		return m_nHeight / 8;
	};
	~AKIPCMJPEGFramedSource()
	{
		if(m_setledexit)
		{
			printf("*****led exit:%d*****\n",findex);
			m_setledexit(findex);
		}
	};
protected:
	virtual void doGetNextFrame();
	virtual unsigned maxFrameSize() const;

private:
	static void getNextFrame(void* ptr);
	void getNextFrame1();
};
#endif
