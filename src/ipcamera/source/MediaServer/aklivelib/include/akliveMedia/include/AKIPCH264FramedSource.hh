#ifndef _AKIPC_H264_FRAMED_SOURCE_
#define _AKIPC_H264_FRAMED_SOURCE_
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <liveMedia.hh>
#include <sys/types.h>
#include <sys/syscall.h>


typedef int (*GetFrameFunc)(void* pbuf, unsigned* size, int nNeedIFrameCont, struct timeval* ptv, int vsIndex);
typedef void (*SetLed)(int index);
class AKIPCH264FramedSource: public FramedSource
{
	int m_started;
	int m_nNeedIFrameCount;
	GetFrameFunc m_getframefunc;
	int findex;
	SetLed m_setledstart;
	SetLed m_setledexit;
public:
	void setGetFrameFunc(GetFrameFunc func);
	void setSetLed(SetLed funcstart, SetLed funcexit);
	void setIndex(int index);
	void setIFrameCount(int n);

	AKIPCH264FramedSource(UsageEnvironment &env, int nType)
		:FramedSource(env)
	{
		m_setledexit = NULL;
		m_started = 0;
		m_nNeedIFrameCount = 0;

	};
	~AKIPCH264FramedSource()
	{
		if(m_setledexit)
		{
			printf("*****led exit:%d*****\n", findex);
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
