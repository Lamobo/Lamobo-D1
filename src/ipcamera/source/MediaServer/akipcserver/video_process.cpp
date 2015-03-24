#include "video_process.h"
#include "Thread.h"
#include "encode.h"
#include "camera.h"
#include "Tool.h"
#include "akuio.h"
#include "encode.h"
#include "muxer.h"
#include "SDcard.h"
#include "AkMotionDetect.h"
#include "photograph.h"

#include <queue>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>

static unsigned char frameArray[][31] = 
{    //0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30 
      {0,0,0,0,0,0,0,0,0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/*1 */{1,0,0,0,0,0,0,0,0,0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/*2 */{1,0,0,0,0,0,0,0,0,0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
/*3 */{1,0,0,0,0,0,0,0,0,0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	
/*4 */{1,0,0,0,0,0,0,0,1,0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
/*5 */{1,0,0,0,0,0,1,0,0,0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
/*6 */{1,0,0,0,0,1,0,0,0,0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0},	
/*7 */{1,0,0,0,1,0,0,0,0,1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0},
/*8 */{1,0,0,0,1,0,0,0,1,0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0},
/*9 */{1,0,0,0,1,0,0,0,1,0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},	
/*10*/{1,0,0,1,0,0,1,0,0,1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0},
/*11*/{1,0,0,1,0,0,1,0,0,1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0},
/*12*/{1,0,0,1,0,1,0,1,0,1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0},	
/*13*/{1,0,0,1,0,1,0,1,0,1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0},
/*14*/{1,0,1,0,1,0,1,0,1,0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
/*15*/{1,0,1,0,1,0,1,0,1,0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},	
/*16*/{1,0,1,0,1,0,1,0,1,0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
     //0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30
/*17*/{1,0,1,0,1,0,1,0,1,1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
/*18*/{1,0,1,1,1,0,1,0,1,0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1},	
/*19*/{1,1,0,1,1,0,1,1,0,1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
/*20*/{1,1,0,1,1,0,1,1,0,1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
/*21*/{1,1,0,1,1,0,1,1,1,0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},	
/*22*/{1,1,0,1,1,0,1,1,1,0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1},
/*23*/{1,1,0,1,1,1,0,1,1,1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1},
/*24*/{1,0,1,1,1,1,0,1,1,1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1},	
/*25*/{1,0,1,1,1,1,1,0,1,1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1},
/*26*/{1,0,1,1,1,1,1,1,0,1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
/*27*/{1,0,1,1,1,1,1,1,1,1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1},	
/*28*/{1,0,1,1,1,1,1,1,1,1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*29*/{1,0,1,1,1,1,1,1,1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
/*30*/{1,1,1,1,1,1,1,1,1,1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},	
};

using namespace std;

static int videoStartFlag = 0;
extern init_parse parse;
static int ViewFlag = 0;//0-没有预览 1-第一路预览 2-第二路预览 3-第一、二路都预览
extern int Recordflag;//0-没有录像 1-第一路录像 2-第二路录像

#define VIDEOBUFLEN 256 * 1024//buf大小
#define MAX_QUEUE_SIZE 10//最大buf个数
const unsigned char throwX [][3] = //丢帧策略数组{x,y,z}表示free buf个数从x到y个每隔z帧丢一帧
  {{4 , 7, 5}, {0, 4, 3}, {0, 0, 0}};
static unsigned nal_offset[2] = {0, 0};//264码流中关键帧最前面sps，pps等信息的长度，live555需要解析
typedef struct
{
	int nflag;
	unsigned nlen;
	unsigned char buf[VIDEOBUFLEN];
	struct timeval tv;
	unsigned long ts;
	int nIsIFrame;
}T_VIDEOBUF;


static queue<T_VIDEOBUF*> queueBuf[2];//编码后的码流链表
static queue<T_VIDEOBUF*> queueBufFree[2];//空余链表

static Condition con[2], conw[2];
static T_VIDEOBUF* videobufTT[2];//保存第一个编码后的数据
//#define DBG_ENCODE_FRAMERATE
//#define DBG_GET_FRAMERATE

//#define DBG_ENCODE_FRAME1
#ifdef DBG_ENCODE_FRAME1
#define DBG_ENCODE_FRAME_COUNT1 5000
#define DBG_ENCODE_FRAME_FILENAME1 "testframe1.264"
#endif

//#define DBG_ENCODE_FRAME2
#ifdef DBG_ENCODE_FRAME2
#define DBG_ENCODE_FRAME_COUNT2 5000
#define DBG_ENCODE_FRAME_FILENAME2 "testframe2.264"
#endif

//#define DBG_GET_FRAME
static int nInit = 0;//初始化完成标志
extern int g_InitMotion;

static Condition conView;
static void LockViewFlag()
{
	Condition_Lock(conView);
}

static void UnLockViewFlag()
{
	Condition_Unlock(conView);
}

#if 0
static unsigned long gettimespan(struct timeval* before, struct timeval* after)
{
	unsigned long usec = (after->tv_sec - before->tv_sec) * 1000 * 1000 + after->tv_usec - before->tv_usec;
	return usec;
}
static void printBufSize(int index, int when)
{
	if(when == 0) //check
		printf("@@check:index[%d] buf size<%u>,<%u>\n", index, queueBuf[index].size(), queueBufFree[index].size());	
	else if(when == 1)//put
		printf("$$$$put:index[%d] buf size<%u>,<%u>\n", index, queueBuf[index].size(), queueBufFree[index].size());	
	else if(when == 2)//get
		printf("****get:index[%d] buf size<%u>,<%u>\n", index, queueBuf[index].size(), queueBufFree[index].size());	
}
#endif
static int video_process_clear_buf(int index)
{
	T_VIDEOBUF* pvideobuf = NULL;
	Condition_Lock(con[index]);
	unsigned int bufsize = queueBuf[index].size();
	for(unsigned int i = 0; i < bufsize; i ++)
	{
		pvideobuf = queueBuf[index].front();
		if(pvideobuf == NULL)
		{
			printf("clear pvideobuf = NULL\n");
			while(1);	
		}
		queueBuf[index].pop();
		queueBufFree[index].push(pvideobuf);
	}
	Condition_Unlock(con[index]);
	return 0;
}

static void initvideoBuf()
{
	T_VIDEOBUF* pbuf;
	for(int i = 0; i < 2; i ++)
	{
		Condition_Initialize(&con[i]);
		Condition_Initialize(&conw[i]);
		for(int j = 0; j < MAX_QUEUE_SIZE; j ++)
		{
			pbuf = new T_VIDEOBUF;
			queueBufFree[i].push(pbuf);
		}
		
		videobufTT[i] = new T_VIDEOBUF;
	}
	Condition_Initialize(&conView);
}

int video_process_get_buf(void* buf, unsigned* nlen, int nNeedIFrame, struct timeval* ptv, int index)
{
	T_VIDEOBUF* bufvideo;
	
	if(nNeedIFrame > 0)
	{
		if(nInit == 0)
		{
			printf("!!!!!init not finish\n");
			*nlen = 0;
			goto out1;
		}
		printf("need iFrame,size=%d\n", videobufTT[index]->nlen);
		memmove(buf, videobufTT[index]->buf, (nal_offset[index] == 0)?videobufTT[index]->nlen:nal_offset[index]);
		gettimeofday(ptv, NULL);
		*nlen = (nal_offset[index] == 0)?videobufTT[index]->nlen:nal_offset[index];
		return 0;
	}
	Condition_Lock(con[index]);
	if(queueBuf[index].empty())	
	{	
		Condition_Unlock(con[index]);
		*nlen = 0;
		goto out1;
	}
	bufvideo = queueBuf[index].front();
	queueBuf[index].pop();
	Condition_Unlock(con[index]);
	//memmove(buf, bufvideo->nIsIFrame?(bufvideo->buf + nal_offset[index]):(bufvideo->buf), (bufvideo->nIsIFrame)?(bufvideo->nlen - nal_offset[index]):bufvideo->nlen);
	memmove(buf, bufvideo->buf, bufvideo->nlen);
	*ptv = bufvideo->tv;
	*nlen = (bufvideo->nlen);
	//*nlen = (bufvideo->nIsIFrame)?(bufvideo->nlen - nal_offset[index]):(bufvideo->nlen);
	Condition_Lock(con[index]);
	queueBufFree[index].push(bufvideo);				
	Condition_Unlock(con[index]);
	
#ifdef DBG_GET_FRAMERATE //cal get framerate	
	static int count[2] = {0,0};	
	count[index] ++;	
	static struct timeval tv1[2], tv2[2];	
	gettimeofday(&tv2[index], NULL);
	static unsigned long bytepsec[2] = {0, 0};
	bytepsec[index] += *nlen;
	if(tv1[index].tv_sec != tv2[index].tv_sec)	
	{		
			printf("index:%d live555 get frame:%d, %u kbps\n", index, count[index], bytepsec[index] / 1000 * 8);		
			tv1[index] = tv2[index];		
			count[index] = 0;
			bytepsec[index] = 0;
	}
#endif
out1:
	return 0;
}

static int video_process_put_buf(void* buf, unsigned* nlen, int nIsIFrame, struct timeval* ptv, int index)
{
	T_VIDEOBUF* pvideobuf = NULL;
	Condition_Lock(con[index]);
	pvideobuf = queueBufFree[index].front();
	queueBufFree[index].pop();
	Condition_Unlock(con[index]);
	
	memcpy(pvideobuf->buf, buf, *nlen);
	pvideobuf->nlen = *nlen;
	pvideobuf->tv = *ptv;
	if(nIsIFrame)
		pvideobuf->nIsIFrame = 1;
	else
		pvideobuf->nIsIFrame = 0;
			
	Condition_Lock(con[index]);
	queueBuf[index].push(pvideobuf);
	Condition_Unlock(con[index]);
	Condition_Signal(&conw[index]);
	return 0;
}
#if 0
static int skipFrame_framerate(int framerate, int* curframenum)
{
	int ret = 1;
	if(frameArray[framerate][*curframenum] == 0)
	{
		ret = 0;
	}
	
	if(*curframenum == 29)
	{
		*curframenum = 0;	
	}
	else
	{
		(*curframenum) ++;	
	}
	return ret;
}

static int skipFrame_buffull(unsigned long ts, int framenum)
{
	int ret = 1;
	static int throwcount = 0;
	
	if(ViewFlag == 0)
	{
		if(Recordflag == 0)
		{
			ret = 0;
		}
	}
	else if(ViewFlag == 1 || ViewFlag == 2)
	{
		int index;
		if(ViewFlag == 1)
			index = 0;
		else
			index = 1;

		Condition_Lock(con[index]);
		if(queueBuf[index].size() < throwX[0][0])
			throwcount = 0;
		else if(queueBuf[index].size() >= throwX[0][0] && queueBuf[index].size() < throwX[0][1])
		{
			if(throwcount >= throwX[0][2])
			{
				printf("[%u,%u) ei %d ::full, throw frame,%lu,%d\n", throwX[0][0], throwX[0][1], index, ts,framenum);
				throwcount = 0;
				ret = 0;
			}
			else
				throwcount ++;
		}
		else if(queueBuf[index].size() >= throwX[1][0] && queueBuf[index].size() < throwX[1][1])
		{
			if(throwcount >= throwX[1][2])
			{
				printf("[%u,%u) ei %d::full, throw frame,%lu,%d\n", throwX[1][0], throwX[1][1],index, ts,framenum);
				throwcount = 0;
				ret = 0;	
			}
			else
				throwcount ++;
		}
		else if(queueBuf[index].size() >= throwX[2][0])
		{
			printf("[%u,+~) ei %d::full, throw frame,%lu,%d\n", throwX[2][0],index, ts,framenum);
			throwcount = 0 ;
			ret = 0;	
		}
		
		Condition_Unlock(con[index]);
	}
	else if(ViewFlag == 3)
	{
		Condition_Lock(con[0]);	
		Condition_Lock(con[1]);
	
		if(queueBuf[0].size() < throwX[0][0] && queueBuf[1].size() < throwX[0][0])
			throwcount = 0;
		else if((queueBuf[0].size() >= throwX[2][0]) || (queueBuf[1].size() >= throwX[2][0]))
		{
			printf("[%u,+~) ei 0 && 1::full, throw frame,%lu,%d\n",throwX[2][0], ts,framenum);
			throwcount = 0 ;
			ret = 0;	
		}
		else if((queueBuf[0].size() >= throwX[1][0] && queueBuf[0].size() < throwX[1][1]) || (queueBuf[1].size() >= throwX[1][0] && queueBuf[1].size() < throwX[1][1]))
		{
			if(throwcount >= throwX[1][2])
			{
				printf("[%u,%u) ei 0 && 1 ::full, throw frame,%lu,%d\n",throwX[1][0], throwX[1][1], ts,framenum);
				throwcount = 0;
				ret = 0;	
			}
			else
				throwcount ++;
		}
		else if((queueBuf[0].size() >= throwX[0][0] && queueBuf[0].size() < throwX[0][1]) || (queueBuf[1].size() >= throwX[0][0] && queueBuf[1].size() < throwX[0][1]))
		{
			if(throwcount >= throwX[0][2])
			{
				printf("[%u,%u) ei 0 && 1 ::full, throw frame,%lu,%d\n", throwX[0][0], throwX[0][1], ts,framenum);
				throwcount = 0;
				ret = 0;
			}
			else
				throwcount ++;
		}
		
		Condition_Unlock(con[1]);
		Condition_Unlock(con[0]);
	}
	
	return ret;
}
#endif
static int skipFrame(int framerate, unsigned long ts)
{
	static int framenum = 0;
	int ret = 1;//ret 为0的时候表示丢弃该帧
	int throwbyrate = 0;//记录由于帧率控制丢帧
	static int lastthrow = 0;//记录上一帧是否丢弃
	if(frameArray[framerate][framenum] == 0)
	{
		throwbyrate = 1;
	}
	
	static int throwcount = 0;
	
	if(ViewFlag == 0)//无预览
	{
		if(Recordflag == 0)//无录像
		{
			ret = 0;
		}
		else
		{
			if(throwbyrate == 1)
				ret = 0;
		}
	}
	else if(ViewFlag == 1 || ViewFlag == 2)//第一路或者第二路预览
	{
		int index;
		if(ViewFlag == 1)
			index = 0;
		else
			index = 1;

		Condition_Lock(con[index]);
		if(queueBufFree[index].size() > throwX[0][1])
		{
			if(throwbyrate)
			{
				ret = 0;
			}
			throwcount = 0;
		}
		else if(queueBufFree[index].size() > throwX[0][0] && queueBufFree[index].size() <= throwX[0][1])
		{
			if(throwbyrate)
			{
				ret = 0;
			}
			else if(lastthrow)//上一次丢了，这次就不丢了，下面就不解释了
			{
				throwcount ++;
			}
			else
			{
				if(throwcount >= throwX[0][2])
				{
					printf("(%u,%u] ei %d ::full, throw frame,%lu,%d\n", throwX[0][0], throwX[0][1], index, ts,framenum);
					throwcount = 0;
					ret = 0;
				}
				else
					throwcount ++;
			}
		}
		else if(queueBufFree[index].size() > throwX[1][0] && queueBufFree[index].size() <= throwX[1][1])
		{
			if(throwbyrate)
			{
				ret = 0;
			}
			else if(lastthrow)
			{
				throwcount ++;	
			}
			else
			{
				if(throwcount >= throwX[1][2])
				{
					printf("(%u,%u] ei %d::full, throw frame,%lu,%d\n", throwX[1][0], throwX[1][1],index, ts,framenum);
					throwcount = 0;
					ret = 0;	
				}
				else
					throwcount ++;
			}
		}
		else if(queueBufFree[index].size() <= throwX[2][0])
		{
			printf("[%u,0] ei %d::full, throw frame,%lu,%d\n", throwX[2][0],index, ts,framenum);
			throwcount = 0 ;
			ret = 0;	
		}
		
		Condition_Unlock(con[index]);
	}
	else if(ViewFlag == 3)//两路都预览
	{
		Condition_Lock(con[0]);	
		Condition_Lock(con[1]);
	
		if(queueBufFree[0].size() > throwX[0][1] && queueBufFree[1].size() > throwX[0][1])
		{
			if(throwbyrate)
			{
				ret = 0;
			}
			throwcount = 0;
		}
		else if((queueBufFree[0].size() <= throwX[2][0]) || (queueBufFree[1].size() <= throwX[2][0]))
		{
			printf("[%u,0) ei 0 && 1::full, throw frame,%lu,%d\n",throwX[2][0], ts,framenum);
			throwcount = 0 ;
			ret = 0;	
		}
		else if((queueBufFree[0].size() > throwX[1][0] && queueBufFree[0].size() <= throwX[1][1]) || (queueBufFree[1].size() > throwX[1][0] && queueBufFree[1].size() <= throwX[1][1]))
		{
			if(throwbyrate)
			{
				ret = 0;
			}
			else if(lastthrow)
			{
				throwcount ++;	
			}
			else
			{
				if(throwcount >= throwX[1][2])
				{
					printf("(%u,%u] ei 0 && 1 ::full, throw frame,%lu,%d\n",throwX[1][0], throwX[1][1], ts,framenum);
					throwcount = 0;
					ret = 0;	
				}
				else
					throwcount ++;
			}
		}
		else if((queueBufFree[0].size() > throwX[0][0] && queueBufFree[0].size() <= throwX[0][1]) || (queueBufFree[1].size() > throwX[0][0] && queueBufFree[1].size() <= throwX[0][1]))
		{
			if(throwbyrate)
			{
				ret = 0;
			}
			else if(lastthrow)
			{
				throwcount ++;
			}
			else
			{
				if(throwcount >= throwX[0][2])
				{
					printf("(%u,%u] ei 0 && 1 ::full, throw frame,%lu,%d\n", throwX[0][0], throwX[0][1], ts,framenum);
					throwcount = 0;
					ret = 0;
				}
				else
					throwcount ++;
			}
		}
		
		Condition_Unlock(con[1]);
		Condition_Unlock(con[0]);
	}
	
	if(ret == 0)
	{
		lastthrow = 1;	
	}
	else
	{
		lastthrow = 0;		
	}
	
	if(framenum == 29)
	{
		framenum = 0;	
	}
	else
	{
		framenum ++;	
	}
	
	return ret;
}

static unsigned get_nal_offset(void* pbuf, unsigned nlen)
{
	T_U8* tpbuf = (T_U8*)pbuf;
	int startcount = 0;
	unsigned i = 0;
	for(i = 0; i < nlen - 4; i ++)
	{
		if(*(tpbuf + i + 0) == 0x00 &&
			 *(tpbuf + i + 1) == 0x00 &&
			 *(tpbuf + i + 2) == 0x00 &&
			 *(tpbuf + i + 3) == 0x01)
		{	
			startcount ++;
			
			if(startcount == 3)
				break;
		}
	}
	
	return (i == nlen - 4) ? 0 : i;
}

int pic = 0;
extern T_MUX_INPUT mux_input;
static void* video_thread_entry(void* param)
{
	printf("thread_process video start\n");
	
	unsigned long timestamp = 0;	
	unsigned long ts;
	long size;	
	videoStartFlag = 1;
	struct timeval tvs;
	void* ppbuf[2], *pbuf;
	int nIsIFrame[2] = {0, 0};
	T_U8 time_flag = 0;
	T_ENC_INPUT encInput[2];
	//unsigned long lasttime = 0;
	long offset = (parse.height*parse.width*3/2);
	
#ifdef DBG_ENCODE_FRAME1
	int dbg_encode_frame_count1 = 0;
	FILE* fp1 = fopen(DBG_ENCODE_FRAME_FILENAME1, "w");
#endif

#ifdef DBG_ENCODE_FRAME2
	int dbg_encode_frame_count2 = 0;
	FILE* fp2 = fopen(DBG_ENCODE_FRAME_FILENAME2, "w");
#endif
	
	while (1)//获取第一帧
	{
		if (videoStartFlag == 0)
			return NULL;

		if (camera_getframe((void**)&pbuf, &size, &ts) == 1)
			break;
	}
	
	//编码第一帧
	encode_frame(&encInput[0], pbuf, &ppbuf[0], &nIsIFrame[0], &encInput[1], (T_U8*)pbuf+size, &ppbuf[1], &nIsIFrame[1]);
	camera_usebufok(pbuf);
	
	#ifdef DBG_ENCODE_FRAME1
	fwrite(ppbuf[0], encInput[0].size, 1, fp1);
	#endif
	
	#ifdef DBG_ENCODE_FRAME1
	fwrite(ppbuf[1], encInput[1].size, 1, fp2);
	#endif
	
	//把编码后的第一帧数据保存起来，live555需要
	Condition_Lock(con[0]);
	memcpy(videobufTT[0]->buf, ppbuf[0], encInput[0].size);
	videobufTT[0]->nlen = encInput[0].size;
	Condition_Unlock(con[0]);
	nal_offset[0] = get_nal_offset(videobufTT[0]->buf, videobufTT[0]->nlen);
	
	Condition_Lock(con[1]);
	memcpy(videobufTT[1]->buf, ppbuf[1], encInput[1].size);
	videobufTT[1]->nlen = encInput[1].size;
	Condition_Unlock(con[1]);
	nal_offset[1] = get_nal_offset(videobufTT[1]->buf, videobufTT[1]->nlen);
	
	printf("nal offset[0] = 0x%x,, nal offset[1] = 0x%x\n", nal_offset[0], nal_offset[1]);
	int num = 0;
	//int numvideo = 0;
	//T_U32 temp;
	//T_U32 x;
	//int y;
	int frame;

	if ( parse.fps1 > 30 || parse.fps1 <=0)
		frame = 30;
	else
		frame = parse.fps1;	
	
	nInit = 1;
	while (1)
	{
		if (videoStartFlag == 0)			
			return NULL;			

		gettimeofday(&tvs, NULL);
		
		if (camera_getframe((void**)&pbuf, &size, &ts) != 1)
			continue;

		if (pic == 1)
		{
			photograph(pbuf, offset);
			pic = 0;
		}
		//程序启动时音频可能还没启动采集需等待
		//当TS小于1000 就能确定音频正常启动。
		if (0 == time_flag)
		{
			if (ts > 1000)
			{
				printf("get ts is %lu\n",ts);
				camera_usebufok(pbuf);
				continue;
			}
			else
			{
				time_flag = 1;
			}
		}

#if 0
		//视频时间戳算法每采集一帧数据累加33毫秒
		numvideo++;
		if (lasttime != 0)
		{
			if(numvideo == 101 )
			{
				numvideo = 0;
				camera_usebufok(pbuf);
				lasttime = ts;
				continue;
			}
			//如果当前时间戳小于前一个时间则丢弃。
			if (ts < lasttime)
			{
				
				if(ts > 1000 && ts < 2000)
				{
					printf("del timestamp = %lu, ts =%lu lasttime =%lu \n", timestamp, ts, lasttime);
					
				}
				else
				{
					lasttime = ts;
				}
				camera_usebufok(pbuf);
				continue;
				//timestamp += 33;
			}
			else
			{
				//时间累加
				temp = ts - lasttime;
				if(temp < 33)
				{
					temp = 33;
					y -= 33 - temp;
				}
				x = temp / 33;
				if( y > 33)
				{
					x++;
					y -= 33;
				}
				
				y += temp % 33;
				timestamp += x * 33;
				//printf("x = %d, y =%d, times = %d ts = %d \n", x, y, timestamp, ts);
			}
		}
		lasttime = ts;
#else
		
		timestamp = ts;

#endif 

#if 1
		if( 1 == g_InitMotion ) 
		{
			if(num % 30 == 0)
			{

				MotionDetect_writedata(pbuf, parse.width*parse.height);

			}
		}
		num++;
#endif
		LockViewFlag();
		if(skipFrame(frame, ts) == 0)
		{
			UnLockViewFlag();
			camera_usebufok(pbuf);
			continue;
		}
		
		encode_frame(&encInput[0], pbuf, &ppbuf[0], &nIsIFrame[0], &encInput[1], (T_U8*)pbuf+size, &ppbuf[1], &nIsIFrame[1]);
		camera_usebufok(pbuf);
		if((ViewFlag == 1) && encInput[0].size != 0)
			video_process_put_buf(ppbuf[0], (unsigned*)&encInput[0].size, nIsIFrame[0], &tvs, 0);
		else if(ViewFlag == 2 && encInput[1].size != 0)
			video_process_put_buf(ppbuf[1], (unsigned*)&encInput[1].size, nIsIFrame[1], &tvs, 1);
		else if(ViewFlag == 3)
		{
			video_process_put_buf(ppbuf[0], (unsigned*)&encInput[0].size, nIsIFrame[0], &tvs, 0);
			video_process_put_buf(ppbuf[1], (unsigned*)&encInput[1].size, nIsIFrame[1], &tvs, 1);
		}
		
		UnLockViewFlag();
#if 1	
		if ((Recordflag == 1 || Recordflag == 2) && encInput[Recordflag - 1].size != 0)
		{
			mux_write_data(1, ppbuf[Recordflag - 1], encInput[Recordflag - 1].size, timestamp, nIsIFrame[Recordflag - 1]);
		}
#endif

#ifdef DBG_ENCODE_FRAME1
		dbg_encode_frame_count1 ++;
		if (dbg_encode_frame_count1 == DBG_ENCODE_FRAME_COUNT1)
		{
			printf("************111close dbg encode file ok******************\n");
			fclose(fp1);
			fp1 = NULL;
		}
		else if (dbg_encode_frame_count1 < DBG_ENCODE_FRAME_COUNT1)
		{
			fwrite(ppbuf[0], encInput[0].size, 1, fp1);
		}
#endif

#ifdef DBG_ENCODE_FRAME2
		dbg_encode_frame_count2 ++;
		if (dbg_encode_frame_count2 == DBG_ENCODE_FRAME_COUNT2)
		{
			printf("************222close dbg encode file ok******************\n");
			fclose(fp2);
			fp2 = NULL;
		}
		else if (dbg_encode_frame_count2 < DBG_ENCODE_FRAME_COUNT2)
		{
			fwrite(ppbuf[1], encInput[1].size, 1, fp2);
		}
#endif
		
#ifdef DBG_ENCODE_FRAMERATE ///cal framerate
		static struct timeval tv1, tv2;
		static int count = 0;
		gettimeofday(&tv1, NULL);
		if (tv2.tv_sec != tv1.tv_sec)
		{
			printf("****framerate:%d\n", count);
			count = 0 ;
			tv2 = tv1;	
		}
		count ++;
#endif

	}
	
	return NULL;
}

int video_process_start()
{
	//create the  thread
	int ret;
	pthread_attr_t SchedAttr;
	pthread_t ntid;
	struct sched_param	SchedParam;
	initvideoBuf();
	memset(&SchedAttr, 0, sizeof(pthread_attr_t));
	memset(&SchedParam, 0, sizeof(SchedParam));
				
	pthread_attr_init( &SchedAttr );				
	SchedParam.sched_priority = 60;	
	pthread_attr_setschedparam( &SchedAttr, &SchedParam );
	pthread_attr_setschedpolicy( &SchedAttr, SCHED_RR );
	if ( ( ret = pthread_create( &ntid, &SchedAttr, video_thread_entry, NULL) ) != 0 ) {
		pthread_attr_destroy(&SchedAttr);
		printf( "unable to create a thread for read data ret = %d!\n", ret );
		return -1;	
	}

	pthread_attr_destroy(&SchedAttr);
	
	return 0;

}
void video_process_stop()
{
	videoStartFlag = 0;
}


void video_process_SetViewFlag(int nFlag)
{
	LockViewFlag();
	static int lastflag = 0;
	if(lastflag == 1)
	{
		if(nFlag == 0)
		{
			video_process_clear_buf(0);
		}
	}
	else if(lastflag == 2)
	{
		if(nFlag == 0)
		{
			video_process_clear_buf(1);	
		}
	}
	else if(lastflag == 3)
	{
		if(nFlag == 1)
		{
			video_process_clear_buf(1);
		}
		else if(nFlag == 2)
		{
			video_process_clear_buf(0);
		}
	}
	
	ViewFlag = nFlag;	
	lastflag = nFlag;
	UnLockViewFlag();
}

void video_process_SetRecordFlag(int nFlag)
{
	Recordflag = nFlag;	
}
