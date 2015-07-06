#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "IPCameraCommand.h"
#include "AkFontLib.h"
#include "log.h"
#include "Tool.h"
#include "isp_interface.h"
#include "Thread.h"
#include "font.h"
#include "cgi_anyka.h"
#include "muxer.h"
#include "camera.h"
#include "anyka_types.h"
#include "akuio.h"
#include "inisetting.h"
#include "tb.h"

#define FONTDSP_TEST_API
#define FONTDSP_TAG  "[F_DSP]"
#define FONT_DATA_START_IDX    (4)
#define BUFF_NUM 	4
#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct tagRECORD_VIDEO_FONT_CTRL {
	T_U8 *y;
	T_U8 *u;
	T_U8 *v;
	T_U32 color;
	T_U32 width;
};

typedef struct {
    T_U32             timeStampColor;
    struct tm         curTime;
    char              timeStr[25];
    T_U8              timeStrLen;
	T_U32             color;
	T_eFONT_SIZE      fontSize;
	T_POS             timePos_X;
	T_POS             timePos_Y;
} T_VIDEO_OSD_REC_ICON;

typedef T_VOID (*FONT_DISP_SET_PIXEL_CB)(T_pVOID param, T_U16 pos_x, T_U16 pos_y);

typedef struct {
	FUNC_DMA_MALLOC dma_malloc;
	FUNC_DMA_FREE	dma_free;
    T_pVOID pframebuf;
	T_U32 width;
	T_U32 height;
} T_CAMERA_STRC;

struct buffer {
        void   *start;
        size_t  length;
};

T_CAMERA_STRC g_camera;
struct v4l2_buffer buf;


static char            *dev_name = "/dev/video0";
static int              fd = -1;
struct buffer          *buffers;
static int              force_format = 1;
static void *ion_mem;
static int ion_size;
static char *osd_buff = AK_NULL;
static char *osd_buff1 = AK_NULL;
static char *osd_buff2 = AK_NULL;
static char *osd2_buff = AK_NULL;
static char *osd2_buff1 = AK_NULL;
static char *osd2_buff2 = AK_NULL;

nthread_t	 ThredOsdID;
int 		 g_osd_exit = 0;
int 		g_time_osd = 1;
int 		g_time = 1;
static int  g_size = 32;
static int  g_size2 = 16;
static int  g_osd_name = 0;

static T_U8 *timefont[21];
static T_U8 *timefont2[21];
static int	fontnum = 0;
static char strfont[30];

//RGB 2 YUV (8bit) 
#define RGB2Y(R,G,B)   ((306*R + 601*G + 117*B)>>10)
#define RGB2U(R,G,B)   ((-173*R - 339*G + 512*B + 131072)>>10)
#define RGB2V(R,G,B)   ((512*R - 428*G - 84*B + 131072)>>10)

extern PRIVACY_AREA priAreaSave[4];
static int xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}

static void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

static int read_frame(void)
{
    unsigned int i;
	CLEAR(buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_USERPTR;
	buf.m.userptr = 0;
	if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) 
	{
			switch (errno)
			{
			case EAGAIN:
					return 0;

			case EIO:
					/* Could ignore EIO, see spec. */

					/* fall through */

			default:
					errno_exit("VIDIOC_DQBUF");
			}
	}

	for (i = 0; i < BUFF_NUM; ++i) 
	{
		   if (buf.m.userptr == (unsigned long)buffers[i].start)
		   {
				return 1;
		   }
	}				

    return 0;
}

static void init_userp(unsigned int buffer_size)
{
        struct v4l2_requestbuffers req;
		int n_buffers;
        CLEAR(req);

        req.count  = BUFF_NUM;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

	  printf("init_userp +\n");

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
		   printf("init_userp==>REQBUFS failed!\n");
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support "
                                 "user pointer i/o\n", dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }
	  printf("init_userp==>REQBUFS succeedded!userptr[count=%d]\n", req.count);
		
        buffers = calloc(BUFF_NUM, sizeof(struct buffer));

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

	printf("init_userp==>buffer_size=%d\n", buffer_size);

	ion_size = buffer_size * BUFF_NUM;
//	if (akuio_pmem_init() != 0) {
//		fprintf(stderr, "akuio_pmem_init failed!\n");
//		exit(EXIT_FAILURE);
//	}
	ion_mem = akuio_alloc_pmem(ion_size);
	if (!ion_mem) {
		fprintf(stderr, "Allocate %d bytes failed\n", ion_size);
	}
	//printf("init_userp==>ion_mem=0x%x\n", ion_mem);

	T_U32 temp;
	T_U8 *ptemp;
	
	temp = akuio_vaddr2paddr(ion_mem) & 7;
	//编码buffer 起始地址必须8字节对齐
	ptemp = ((T_U8 *)ion_mem) + ((8-temp)&7);

    for (n_buffers = 0; n_buffers < BUFF_NUM; ++n_buffers) {
            buffers[n_buffers].length = buffer_size;
//                buffers[n_buffers].start = malloc(buffer_size);
   	   buffers[n_buffers].start = ptemp + buffer_size * n_buffers;
	 //  printf("init_userp==>[%d]start=0x%08x, length=%d\n", 
	  // 		n_buffers, buffers[n_buffers].start, buffers[n_buffers].length);

            if (!buffers[n_buffers].start) {
                    fprintf(stderr, "Out of memory\n");
                    exit(EXIT_FAILURE);
            }
    }
	  printf("init_userp -\n");
}


static int Zoom(int x, int y, int width, int heigth, int outw, int outh, int channel);

extern init_parse parse;
int Set_Zoom(int z)
{
	int width = 0;
	int height = 0;
	int outwidth = 0;
	int outheight = 0;
	int stepwidth = 0;
	int stepheight = 0;
	static int zoom = 0;
//	static int zoom2 = 0;
	int x, y, w, h;
	int channel = z >> 1;
	channel++;
	if(channel == 2)
		return 0;
	if(parse.width == 1280 && channel == 1 )
	{
		width = 1280;
		height = 720;
		outwidth = 1280;
		outheight = 720;
		stepwidth = 180;
		stepheight = 120;
	}
	#if 0
	else if(parse.width == 1280 && channel == 1 &&parse.width2 == 640)
	{
		width = 1280;
		height = 720;
		outwidth = 1280;
		outheight = 720;
		stepwidth = 140;
		stepheight = 90;
	}
	else if(parse.width == 1280 && channel == 1 &&parse.width2 == 720)
	{
		width = 1280;
		height = 720;
		outwidth = 1280;
		outheight = 720;
		stepwidth = 96;
		stepheight = 60;
	}
	#endif
	#if 0
	else if(parse.width == 640 && channel == 1 )
	{
		width = 640;
		height = 480;
		outwidth = 640;
		outheight = 480;
		stepwidth = 110;
		stepheight = 90;
	}
	else if(parse.width == 320 && channel == 1 )
	{
		width = 640;
		height = 480;
		outwidth = 320;
		outheight = 240;
		stepwidth = 100;
		stepheight = 80;
	}
	#endif
	#if 0
	if(parse.width2 == 1280 && channel == 2)
	//if(zoom < 0)
	{
		width = 1280;
		height = 720;
		outwidth = 1280;
		outheight = 720;
		stepwidth = 188;
		stepheight = 120;
	}
	else if(parse.width2 == 640 && channel == 2 )
	{
		width = 640;
		height = 480;
		outwidth = 640;
		outheight = 480;
		stepwidth = 110;
		stepheight = 90;
	}
	else if(parse.width2 == 320 && channel == 2 )
	{
		width = 640;
		height = 480;
		outwidth = 320;
		outheight = 240;
		stepwidth = 100;
		stepheight = 80;
	}
	#endif
	
	if( z & (int)0x01 && zoom < 3 && channel == 1)
	{
		zoom++;
		x = zoom*(stepwidth/2);
		y = zoom*(stepheight/2);
		w = width - x;
		h = height - y;
		
	}
	else if( !(z & (int)0x01) && zoom > 0 && channel == 1)
	{
		zoom--;
		x = zoom*(stepwidth/2);
		y = zoom*(stepheight/2);
		w = width - x;
		h = height - y;
	}
#if 0
	if( z & (int)0x01 && zoom < 3 && channel == 2)
	{
		zoom2++;
		x = zoom*(stepwidth/2);
		y = zoom*(stepheight/2);
		w = x;
		h = y;
	}
	else if( !(z & (int)0x01) && zoom > 0 && channel == 2)
	{
		zoom2--;
		x = zoom*(stepwidth/2);
		y = zoom*(stepheight/2);
		w = w;
		h = y;
	}
#endif
	printf("zoom is %d channel = %d\n", zoom, channel );
#if 0	
	x = zoom*(stepwidth/2);
	y = zoom*(stepheight/2);
	w = width - zoom*stepwidth;
	h = height - zoom*stepheight;
#endif

	{
		Zoom(x, y, w, h, outwidth, outheight, channel );
	}
	return 0;
}

static int Zoom(int x, int y, int width, int heigth, int outw, int outh, int channel)
{
	struct v4l2_streamparm parm;
	
	printf("zoom x=%d,y=%d, width=%d, height=%d, w = %d, h = %d \n",
			x, y, width, heigth, outw, outh);
	//if(channel == 2 && width < outw)
	{
	//	printf("set parm zoom err \n");
	}
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	struct isp_zoom_info *zoom = (struct isp_zoom_info *) parm.parm.raw_data;
	zoom->type = ISP_PARM_ZOOM;
	zoom->channel = channel;
	zoom->cut_xpos = x;
	zoom->cut_ypos = y;
	zoom->cut_width = width;
	zoom->cut_height = heigth;
	zoom->out_width = outw;
	zoom->out_height = outh;
	
	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	 {
		printf("Set occ zoom err \n");
		return -1;
	 }
	else
	{
		printf("Set occ zoom success \n");
	}
	return 0;

}

void * p_draw[11]=
{
	osd_buff_0,
	osd_buff_1,
	osd_buff_2,
	osd_buff_3,
	osd_buff_4,
	osd_buff_5,
	osd_buff_6,
	osd_buff_7,
	osd_buff_8,
	osd_buff_9,
	osd_buff_d
};

void draw(int n, int num, int size, T_U8 * time[], char *buff)
{

//	char t,m;
	char *p;
//	int i, j;
	T_U8 m1;
	T_U8 m2;
	int temp;
	int step;
	int width = 10*size;
	switch(size)
	{
		case 48:
			//width = 384+fontnum*size;
			step = n*24;
			temp = 6;
			break;
		case 32:
			//width = 256+fontnum*size;
			step = n*16;
			temp = 4;
			break;
		case 16:
			//width = 128+fontnum*size;
			step = n*8;
			temp = 2;
			break;
	}

	int startx = 0;
	int endx = size;
	if( n >= 10 )
	{
		startx = size;
		endx = 2*size;
	}
	if( n < 10 )
	{
	for(int i = 0; i< size; i++)
		{
			p = buff+(i*(width/2))+step;
			//char *k = osd[i];
			for(int j = temp*i; j<=temp*i+(temp-1); j++ )
			{
				
				T_U8 t = *(time[num]+j);//a[j];
				for(int k =0; k<8; k+=2)
				{
					
					if(t & (0x80 >> k))
					{
						m1 = 3;
					
					}
					else
					{
						m1 = 0;
					
					}
					if( t & (0x80 >> (k+1)))
					{	
						m2 = 3;
					}
					else
					{
						m2 = 0;
					}
				//*p = (m1 & 0xf) | ( (m2 & 0xf)<<4);
				if( 0 != m1 || 0 != m2 )
				{
					m1 = 3;
					m2 = 3;
				}
				
				*p = m1 | (m2 <<4);
				p++;
				
				}
			}
			
		}

	}
	else
	{
		for(int i = 0; i< size; i++)
		{
			p = buff+((i+size)*(width/2))+step;
			//char *k = osd[i];
			for(int j = temp*i; j<=temp*i+(temp-1); j++ )
			{
				
				T_U8 t = *(time[num]+j);//a[j];
				for(int k =0; k<8; k+=2)
				{
					
					if(t & (0x80 >> k))
					{
						m1 = 3;
					
					}
					else
					{
						m1 = 0;
					
					}
					if( t & (0x80 >> (k+1)))
					{	
						m2 = 3;
					}
					else
					{
						m2 = 0;
					}
				//*p = (m1 & 0xf) | ( (m2 & 0xf)<<4);
				if( 0 != m1 || 0 != m2 )
				{
					m1 = 3;
					m2 = 3;
				}
				
				*p = m1 | (m2 <<4);
				p++;
				
				}
			}
			
		}
	}

	
}
void draw_sec(int sec, int size, T_U8 *font[], char * buff)
{
	if(sec < 10)
	{
		draw(6, 0, size, font, buff);
	}
	else
	{
		draw(6, sec/10, size, font, buff);
	}
	draw(7, sec%10, size, font, buff);
}

void draw_min(int min, int size, T_U8 *font[], char * buff)
{
	if(min < 10)
	{
		draw(3, 0, size, font, buff);
	}
	else
	{
		draw(3, min/10, size, font, buff);
	}
	draw(4, min%10, size, font, buff);
}

void draw_hour(int hour, int size, T_U8 *font[], char * buff)
{
	if(hour < 10)
	{
		draw(0, 0, size, font, buff);
	}
	else
	{
		draw(0, hour/10, size, font, buff);
	}
	draw(1, hour%10, size, font, buff);
}

void draw_ymd(struct tm *tt, int size, T_U8 *font[], char *buff){
    int year = tt->tm_year + 1900;
    int mon = tt->tm_mon + 1;
    int day = tt->tm_mday;
    draw(10, (year/1000)    , size, font, buff);
    draw(11, (year/100) % 10, size, font, buff);
    draw(12, (year/10)  % 10, size, font, buff);
    draw(13, (year)     % 10, size, font, buff);
    draw(14, (mon)      / 10, size, font, buff);
    draw(15, (mon)      % 10, size, font, buff);
    draw(16, (day)      / 10, size, font, buff);
    draw(17, (day)      % 10, size, font, buff);
}
static int floag2 = 0;

static int create_osd_picture2(void *pcdev, int size)
{
	
	int startX, startY, endX, endY;
	int picsize;
	int osd_width, osd_height;
//	int i,j;
	//char *p;
	//char (* osd)[20];
	startX = startY = 0;
	endX = size * 10;
	endY = size * 2;
	#if 0
	switch(size)
	{
		case 48:
			endX = 384+fontnum*size;
			endY = 48;
			break;
		case 32:
			endX = 256+fontnum*size;
			endY = 32;
			break;
		case 16:
			endX = 128+fontnum*size;
			endY = 16;
			break;
	}
	#endif
	osd_width = endX - startX + 1;
	osd_height = endY - startY + 1;
	picsize = osd_width * osd_height;

	if (AK_NULL == osd2_buff1) 
	{
		osd2_buff1 = akuio_alloc_pmem(picsize / 2 + picsize %2);
		
		if (!osd2_buff1)
			return -1;
		memset(osd2_buff1, 0x00, (picsize / 2 + picsize % 2));
	}
	
	if (AK_NULL == osd2_buff2) 
	{
		osd2_buff2 = akuio_alloc_pmem(picsize / 2 + picsize %2);
		if (!osd2_buff2)
			return -1;
		memset(osd2_buff2, 0x00, (picsize / 2 + picsize % 2));
	}
	
	if(floag2 == 0)
	{
		osd2_buff = osd2_buff1;
		floag2 = 1;
	}
	else
	{
		osd2_buff = osd2_buff2;
		floag2 = 0;
    }
    time_t ta;
    time(&ta);
    static int sec=-1, min=-1, hour=-1;
    struct tm * tt = localtime( &ta );
	if(5 != g_time )
	{
		draw(2, 10, g_size2, timefont2, osd2_buff);
		draw(5, 10, g_size2, timefont2, osd2_buff);



	//printf("time is %d, %d, %d,", tt->tm_sec, tt->tm_min, tt->tm_hour);
	
	
	//if(sec != tt->tm_sec)
		{
			draw_sec(tt->tm_sec, g_size2, timefont2, osd2_buff);
		}
		//if(min != tt->tm_min)
		{
			draw_min(tt->tm_min, g_size2, timefont2, osd2_buff);
		}
		//if(hour != tt->tm_hour)
		{
			draw_hour(tt->tm_hour, g_size2, timefont2, osd2_buff);
		}
	
		sec = tt->tm_sec;
		min = tt->tm_min;
		hour = tt->tm_hour;
	}
	if(g_osd_name == 1)
	{   
        /*
		for( int i = 0; i< fontnum; i++ )
		{
			draw(i+10, 11+i, g_size2, timefont2, osd2_buff);
		}
		//draw(8, 11, g_size2, timefont2, osd2_buff);
		//draw(9, 12, g_size2, timefont2, osd2_buff);
        */
        draw_ymd(tt, g_size2, timefont2, osd2_buff);
	}
	return 0;
}



static int floag = 0;

static int create_osd_picture(void *pcdev, int size)
{
	
	int startX, startY, endX, endY;
	int picsize;
	int osd_width, osd_height;
//	int i,j;
	//char *p;
	//char (* osd)[20];
	startX = startY = 0;
	endX = size * 10;
	endY = size * 2;
	#if 0
	switch(size)
	{
		case 48:
			endX = 384+fontnum*size;
			endY = 48;
			break;
		case 32:
			endX = 256+fontnum*size;
			endY = 32;
			break;
		case 16:
			endX = 128+fontnum*size;
			endY = 16;
			break;
	}
	#endif
	
	osd_width = endX - startX + 1;
	osd_height = endY - startY + 1;
	picsize = osd_width * osd_height;

	if (AK_NULL == osd_buff1) 
	{
		osd_buff1 = akuio_alloc_pmem(picsize / 2 + picsize %2);
		
		if (!osd_buff1)
			return -1;
		memset(osd_buff1, 0x00, (picsize / 2 + picsize % 2));
	}
	
	if (AK_NULL == osd_buff2) 
	{
		osd_buff2 = akuio_alloc_pmem(picsize / 2 + picsize %2);
		if (!osd_buff2)
			return -1;
		memset(osd_buff2, 0x00, (picsize / 2 + picsize % 2));
	}
	
	if(floag == 0)
	{
		osd_buff = osd_buff1;
		floag = 1;
	}
	else
	{
		osd_buff = osd_buff2;
		floag = 0;
	}
    time_t ta;
    time(&ta);
    static int sec=-1, min=-1, hour=-1;
    struct tm * tt = localtime( &ta );
	if(5 != g_time )
	{
		draw(2, 10, g_size, timefont, osd_buff);
		draw(5, 10, g_size, timefont, osd_buff);


	//printf("time is %d, %d, %d,", tt->tm_sec, tt->tm_min, tt->tm_hour);
	
	
	//if(sec != tt->tm_sec)
		{
			draw_sec(tt->tm_sec, g_size, timefont, osd_buff);
		}
		//if(min != tt->tm_min)
		{
			draw_min(tt->tm_min, g_size, timefont, osd_buff);
		}
		//if(hour != tt->tm_hour)
		{
			draw_hour(tt->tm_hour, g_size, timefont, osd_buff);
		}
	
		sec = tt->tm_sec;
		min = tt->tm_min;
		hour = tt->tm_hour;
	}
	if(g_osd_name == 1)
	{
        draw_ymd(tt, g_size, timefont, osd_buff);
	    /*	
		for(int i= 0; i< fontnum; i++)
		{
			draw(i+10, 11+i, g_size, timefont, osd_buff);
			//draw(i+10, san+10, g_size, timefont, osd_buff);
		}
	
		//draw(8, 11, g_size, timefont, osd_buff);
		//draw(9, 12, g_size, timefont, osd_buff);
        */
	}
	return 0;
}

static int read_ini()
{
	IniSetting_init();
	struct picture_info *picture = IniSetting_GetPictureInfo();

	if( !strcmp(picture->osd_place, "right_up"))
	{
		g_time_osd = 1;
	}
	else if(!strcmp(picture->osd_place, "right_down"))
	{
		g_time_osd = 3;
	}
	else if(!strcmp(picture->osd_place, "left_up"))
	{
		g_time_osd = 0;
	}
	else if(!strcmp(picture->osd_place, "left_down"))
	{
		g_time_osd = 2;
	}

	if(!strcmp(picture->osd_time, "show"))
	{
		g_time = 4;
	}
	else if(!strcmp(picture->osd_time, "hide"))
	{
		g_time = 5;
	}
	//printf("strfont %s %d\n", picture->osd_name, strlen(picture->osd_name));
	if(strlen(picture->osd_name) == 0)
	{
		g_osd_name = 0;
		
	}
	else
	{
		g_osd_name = 1;
		memset(strfont, 0x00, 30);
		//strcmp(strfont, picture->osd_name );
		memcpy(strfont, picture->osd_name, strlen(picture->osd_name));
		printf("strfont %s \n", strfont);
	}
	
	IniSetting_destroy();
	return 0;
}

static int u2u(char *inbuf, char *outbuf)
{	int len, i;
	unsigned char firstch = inbuf[0];
	unsigned long code;
	if (firstch > 0xc0)
	{
		if ((firstch & 0xe0) == 0xc0)
		{
			len = 2;
			code = firstch & 0x1f;
		} else if ((firstch & 0xf0) == 0xe0)
		{
			len = 3;
			code = firstch & 0xf;
		} else if ((firstch & 0xf8) == 0xf0)
		{
			len = 4;
			code = firstch & 0x7;
		} else if ((firstch & 0xfc) == 0xf8)
		{
			len = 5;
			code = firstch & 0x3;
		} else if ((firstch & 0xfe) == 0xfc)
		{
			len = 6;
			code = firstch & 0x1;
		} else {
			outbuf[0] = firstch;
			return 1;
		}

		for (i = 1; i < len; i++)
		{
			if ((inbuf[i] & 0xc0) != 0x80)
			{
				fprintf(stderr, "error utf8 code\n");
				return 0;
			}
			code <<= 6;
			code |= (unsigned char)inbuf[i] & 0x3F;
		}
		*(unsigned long *)outbuf = code;
		return len;
	}
	else
	{
		/* ASCII鐮?*/
		outbuf[0] = (unsigned char)firstch;
		return 1;
	}
}


static int search_table(unsigned char *in, unsigned char *out)
{	
	//int i = 0;	
	int head, tail, step;
	int len = sizeof(table)/sizeof(table[0]);
	unsigned short key = *(unsigned short *)in;	

	head = 0;	
	tail = len;	
	step = len / 2;	
	while(1) 
	{		
		if (key > table[step][0]) 
		{			
			head = step;
			step = head + (tail - head) / 2;
		} 
		else if (key < table[step][0])
		{
			tail = step;
			step = head + (tail - head) / 2;
		} else 
		{
			break;
		}
		/* FIXME: 瀵逛簬鏈煡缂栫爜锛岃繖閲岃繑鍥炰腑鏂団�滐紵鈥濆瓧绗?*/
		if (head == step)
		{
			out[0] = 0xa3;
			out[1] = 0xbf;
			return -1;		
		}
	}	
	printf("gb2312: %04x\n", table[step][1]);
	out[0] = table[step][1] & 0xff;
	out[1] = table[step][1] >> 8 ;
	return 0;
}

#if 0
static void display_ch(char *ch, int len)
{	
	int i, j, k;
	int bpl = len / 8;
	for(i = 0; i < len; i++)
	{
		for(j = 0; j < bpl; j++)
		{
			for(k = 0; k < 8; k++)
			{
				if (ch[bpl*i + j] & (0x80 >> k))
					printf("*");
				else
					printf(" ");
			}
		}
		printf("\n");
	}
}
#endif



#define fontfile "/usr/share/hzk/HZK"
#define OUTLEN	8
static int Init_Font(int size, T_U8 * font[])
{
	long fd;
	char file[256] = {0};
	int leng = 0;
	int i = 0;
	int n = 0;
	unsigned char uout[OUTLEN] = {0};	
	unsigned char gout[OUTLEN] = {0};
	
	switch(size)
	{
		case 16:
			sprintf(file, "%s%d", fontfile, size);
			break;
		case 24:
			sprintf(file, "%s%d", fontfile, size);
			break;
		case 32:
			sprintf(file, "%s%d", fontfile, size);
			break;
		case 48:
			sprintf(file, "%s%d", fontfile, size);
			break;
		default:
			sprintf(file, "%s%d", fontfile, 32);
	}
	
	fd = open( file, O_RDONLY );
	if( fd <= 0 )
	{	
		printf("open font file %s err \n", file);
		return -1;
	}
	
	T_U8 qh= 0xa3;
	T_U8 wh= 0xb0;

	if( qh == 0xa3 )
	{
		qh -= 0x2;
	}
	else if( qh >= 0xb0 && qh <= 0xd7 )
	{
		qh -= 0xe;
	}
	else
	{
		printf( "Invalid codec \n" );
		return -1;
	}

	if(wh < 0xa1 || wh > 0xfe)
	{
		printf( "Invalid codec \n" );
		return -1;
	}

	qh -= 0xa0;
	wh -= 0xa0;
	
	int  offset = 0;
	leng = size /8 * size;
	
	for( i = 0; i< 11; i++, wh++)
	{
		font[i] = malloc(leng);
		memset(font[i], 0x00, leng);
		offset = (94 * (qh - 1) + (wh - 1)) * leng;
		lseek(fd, offset, SEEK_SET);
		read(fd, font[i], leng);
	}


	n = strlen(strfont);

	int outlen;
	int fleng = 0;
	fontnum = 0;
	while( n )
	{
		memset(uout, 0x00, OUTLEN);
		outlen = u2u( strfont+fleng, (char *)uout );

		printf("uout = %04x \n", *(unsigned short *) uout);

		search_table(uout, gout);

		
		qh= gout[0];
		wh= gout[1];

		printf("n = %d i = %d, qh =%d, wh =%d outlen=%d \n", n,  i, qh, wh, outlen);

		if( qh == 0xa3 )
		{
			qh -= 0x2;
		}
		else if( qh >= 0xb0 && qh <= 0xd7 )
		{
			qh -= 0xe;
		}
		else
		{
			printf( "Invalid codec \n" );
			continue;
		}

		if(wh < 0xa1 || wh > 0xfe)
		{
			printf( "Invalid codec \n" );
			continue;
		}

		qh -= 0xa0;
		wh -= 0xa0;

		font[i] = malloc(leng);
		memset(font[i], 0x00, leng);
		offset = (94 * (qh - 1) + (wh - 1)) * leng;
		lseek(fd, offset, SEEK_SET);
		read(fd, font[i], leng);

		//display_ch(font[i], size);

		i++;
		n -= outlen;
		fontnum ++;
		
		if(fontnum >= 10)
			break;
		
		fleng += outlen;
		
	}

	close(fd);

	return 0;
}



void Take_out_osd()
{
	struct v4l2_streamparm parm;
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_osd_info *p_osd =(struct isp_osd_info *) parm.parm.raw_data;
	
	p_osd->type = ISP_PARM_OSD;
	p_osd->channel = 1;
	p_osd->color_depth = 16;
	p_osd->color_transparency = 0;
	p_osd->start_xpos = 0;
	p_osd->start_ypos = 0;
	p_osd->end_xpos = 200;
	p_osd->end_ypos = 20;
	p_osd->enable = 0;

	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	 {
		printf("take out osd err \n");
	 }
	else
	{
		//printf("Set osd success \n");
	}

	p_osd->type = ISP_PARM_OSD;
	p_osd->channel = 2;
	p_osd->color_depth = 16;
	p_osd->color_transparency = 0;
	p_osd->start_xpos = 0;
	p_osd->start_ypos = 0;
	p_osd->end_xpos = 200;
	p_osd->end_ypos = 20;
	p_osd->enable = 0;

	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	{
		printf("take out osd err \n");
	}
	else
	{
		//printf("Set osd success \n");
	}
	
}


int SetOsd2( int size )
{
	int start_x;
	int start_y;
	int end_x;
	int end_y;

	int width = size*10;
	int height = size*2;
	#if 0
	switch(size)
	{
		case 48:
			width = 384+fontnum*size;
			height = 48;
			break;
		case 32:
			width = 256+fontnum*size;
			height = 32;
			break;
		case 16:
			width = 128+fontnum*size;
			height = 16;
			break;
	}
	#endif
	//第二路图像必须是偶数, 并不能设置到边源。
	switch(g_time_osd)
	{
		case 0:
		{
			start_x = 0;
			start_y = 0;
			end_x = width-1;
			end_y = height;
			break;
		}
		case 1:
		{
			start_x = parse.width2-width-2;
			start_y = 0;
			end_x = parse.width2-3;
			end_y = height;
			break;
		}
		case 2:
		{
			start_x = 0;
			start_y = parse.height2 - height-1;
			end_x = width-1;
			end_y = parse.height2 -1;
			break;
		}
		case 3:
		{
			start_x = parse.width2-width -2;
			start_y = parse.height2 - height -1;
			end_x = parse.width2-3;
			end_y = parse.height2 -1;
			break;
		}
		case 4:
		{
			return 0;
		}
	}
	struct v4l2_streamparm parm;

	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_osd_info *p_osd = (struct isp_osd_info *)parm.parm.raw_data;
	
	//Take_out_osd();
	if(create_osd_picture2(AK_NULL, size))
	{
		printf("create osd picture err \n");
		return -1;
	}


	//printf("start_x =%d, start_y = %d, end_x=%d end_y=%d \n", start_x, start_y, end_x, end_y);
	p_osd->type = ISP_PARM_OSD;
	p_osd->channel = 2;
	p_osd->color_depth = 16;
	p_osd->color_transparency = 0;
	p_osd->start_xpos = start_x;
	p_osd->start_ypos = start_y;
	p_osd->end_xpos = end_x;
	p_osd->end_ypos = end_y;
	p_osd->enable = 1;

	//printf("color_transparency = %d", p_osd->color_transparency);
	if(osd2_buff != AK_NULL)
	{
		p_osd->phys_addr = akuio_vaddr2paddr(osd2_buff);
	}
	else
	{
		return -1;
	}
	
	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	 {
		printf("Set osd err \n");
	 }
	else
	{
		//printf("Set osd success \n");
	}
	return 0;
}


int SetOsd1( int size )
{
	int start_x;
	int start_y;
	int end_x;
	int end_y;

	int width = size*10;
	int height = size*2;
	#if 0
	switch(size)
	{
		case 48:
			//width = 384+fontnum*size;
			//height = 48;
			break;
		case 32:
			width = 256+fontnum*size;
			height = 32;
			break;
		case 16:
			width = 128+fontnum*size;
			height = 16;
			break;
	}
	#endif
	switch(g_time_osd)
	{
		case 0:
		{
			start_x = 26;
			start_y = 26;
			end_x = width+26;
			end_y = height+26;
			break;
		}
		case 1:
		{
			start_x = parse.width-width;
			start_y = 1;
			end_x = parse.width;
			end_y = height;
			break;
		}
		case 2:
		{
			start_x = 0;
			start_y = parse.height - height;
			end_x = width;
			end_y = parse.height;
			break;
		}
		case 3:
		{
			start_x = parse.width-width;
			start_y = parse.height - height;
			end_x = parse.width;
			end_y = parse.height;
			break;
		}
		case 4:
		{
			return 0;
		}
	}
	struct v4l2_streamparm parm;

	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_osd_info *p_osd = (struct isp_osd_info *)parm.parm.raw_data;
	
	//Take_out_osd();
	if(create_osd_picture(AK_NULL, size))
	{
		printf("create osd picture err \n");
		return -1;
	}


	
	p_osd->type = ISP_PARM_OSD;
	p_osd->channel = 1;
	p_osd->color_depth = 16;
	p_osd->color_transparency = 0;
	p_osd->start_xpos = start_x;
	p_osd->start_ypos = start_y;
	p_osd->end_xpos = end_x;
	p_osd->end_ypos = end_y;
	p_osd->enable = 1;

	//printf("color_transparency = %d", p_osd->color_transparency);
	if(osd_buff != AK_NULL)
	{
		p_osd->phys_addr = akuio_vaddr2paddr(osd_buff);
	}
	else
	{
		return -1;
	}
	
	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	 {
		printf("Set osd err \n");
	 }
	else
	{
		//printf("Set osd success \n");
	}
	return 0;
}

int SetChannel(int width, int height, int enable)
{
	struct v4l2_streamparm parm;
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_channel2_info *p_mode = (struct isp_channel2_info *)parm.parm.raw_data;
	printf("SetChannel width=%d, height=%d enable = %d\n", width, height, enable);
	p_mode->type = ISP_PARM_CHANNEL2;
	p_mode->width = width;
	p_mode->height = height;
	p_mode->enable = enable;

	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	{
		printf("Set channel2 err \n");
	}
	else
	{
		printf("Set channel2 ok \n");
	}

	return 0;
}

static T_pVOID thread_enc( T_pVOID user )
{

	while(1)
	{
		if ( 1 == g_osd_exit)
		{
			break;
		}
		//if(g_time_osd != 4 )
		{
			SetOsd1( g_size );
			SetOsd2( g_size2 );
		}
		sleep(1);
		
	}

	return NULL;
}


int SetOcc(int num, int x1, int y1, int x2, int y2, int enable)
{
	struct v4l2_streamparm parm;
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_occlusion_info *p_occ = (struct isp_occlusion_info *)parm.parm.raw_data;
	printf("num=%d, x1=%d, y1=%d, x2=%d, y2=%d, enable=%d", 
			num, x1, y1, x2, y2, enable);
	p_occ->type = ISP_PARM_OCCLUSION;
	p_occ->channel = 1;
	p_occ->number = num;
	p_occ->start_xpos = x1;
	p_occ->start_ypos = y1;
	p_occ->end_xpos = x2;
	p_occ->end_ypos = y2;
	p_occ->enable = enable;
	
	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	 {
		printf("Set occ err \n");
	 }
	else
	{
		printf("Set occ success \n");
	}
	return 0;
}

int InitOcc()
{
	int x, y, end_x, end_y;
	if( IniSetting_aki() < 0 )
	{
		printf("Init OCC fail \n");
		return -1;
	}

	struct occ_info * occ = IniSetting_GetOccInfo();
	if(NULL == occ)
	{
		return -1;
	}
	if( atoi(occ->enable1) == 1 )
	{
		x = atoi(occ->start_xpos1);
		y = atoi(occ->start_ypos1);
		end_x = atoi(occ->end_xpos1);
		end_y = atoi(occ->end_ypos1);
		SetOcc(1, x, y, end_x, end_y, 1);
		priAreaSave[0].nNumber = 1;
		priAreaSave[0].nLeftTopX = x;
		priAreaSave[0].nLeftTopY = y;
		priAreaSave[0].nWidth = end_x - x;
		priAreaSave[0].nHeight = end_y - y;
		printf("width = %d \n", priAreaSave[0].nWidth);
	}
	if( atoi(occ->enable2) == 1 )
	{
		x = atoi(occ->start_xpos2);
		y = atoi(occ->start_ypos2);
		end_x = atoi(occ->end_xpos2);
		end_y = atoi(occ->end_ypos2);
		SetOcc(2, x, y, end_x, end_y, 1);
		priAreaSave[1].nNumber = 2;
		priAreaSave[1].nLeftTopX = x;
		priAreaSave[1].nLeftTopY = y;
		priAreaSave[1].nWidth = end_x - x;
		priAreaSave[1].nHeight = end_y - y;
		printf("width = %d \n", priAreaSave[1].nWidth);
	}
	if( atoi(occ->enable3) == 1 )
	{
		x = atoi(occ->start_xpos3);
		y = atoi(occ->start_ypos3);
		end_x = atoi(occ->end_xpos3);
		end_y = atoi(occ->end_ypos3);
		SetOcc(3, x, y, end_x, end_y, 1);
		priAreaSave[2].nNumber = 3;
		priAreaSave[2].nLeftTopX = x;
		priAreaSave[2].nLeftTopY = y;
		priAreaSave[2].nWidth = end_x - x;
		priAreaSave[2].nHeight = end_y - y;
		printf("width = %d \n", priAreaSave[2].nWidth);
	}
	if( atoi(occ->enable4) == 1 )
	{
		x = atoi(occ->start_xpos4);
		y = atoi(occ->start_ypos4);
		end_x = atoi(occ->end_xpos4);
		end_y = atoi(occ->end_ypos4);
		SetOcc(4, x, y, end_x, end_y, 1);
		priAreaSave[3].nNumber = 4;
		priAreaSave[3].nLeftTopX = x;
		priAreaSave[3].nLeftTopY = y;
		priAreaSave[3].nWidth = end_x - x;
		priAreaSave[3].nHeight = end_y - y;
		printf("width = %d \n", priAreaSave[3].nWidth);
	}
	//IniSetting_akidestroy();
	return 0;
}

int SaveOcc(int num, int x, int y, int endx, int endy, int enable)
{
	if( IniSetting_aki() < 0 )
	{
		printf("Init OCC fail \n");
		return -1;
	}
	struct occ_info * occ = IniSetting_GetOccInfo();
	if(NULL == occ)
	{
		return -1;
	}
	switch( num )
	{
		case 1:
		{
			sprintf(occ->start_xpos1, "%d", x);
			sprintf(occ->start_ypos1, "%d", y);
			sprintf(occ->end_xpos1, "%d", endx);
			sprintf(occ->end_ypos1, "%d", endy);
			sprintf(occ->enable1, "%d", enable);
			printf("set x %s y %s endx %s endt %s enable %s \n", 
					occ->start_xpos1, occ->start_ypos1, occ->end_xpos1, occ->end_ypos1, occ->enable1);
			break;
		}
		case 2:
		{
			sprintf(occ->start_xpos2, "%d", x);
			sprintf(occ->start_ypos2, "%d", y);
			sprintf(occ->end_xpos2, "%d", endx);
			sprintf(occ->end_ypos2, "%d", endy);
			sprintf(occ->enable2, "%d", enable);
			printf("set x %s y %s endx %s endt %s enable %s \n", 
					occ->start_xpos2, occ->start_ypos2, occ->end_xpos2, occ->end_ypos2, occ->enable2);
			break;
		}
		case 3:
		{
			sprintf(occ->start_xpos3, "%d", x);
			sprintf(occ->start_ypos3, "%d", y);
			sprintf(occ->end_xpos3, "%d", endx);
			sprintf(occ->end_ypos3, "%d", endy);
			sprintf(occ->enable3, "%d", enable);
			printf("set x %s y %s endx %s endt %s enable %s \n", 
					occ->start_xpos3, occ->start_ypos3, occ->end_xpos3, occ->end_ypos3, occ->enable3);
			break;
		}
		case 4:
		{
			sprintf(occ->start_xpos4, "%d", x);
			sprintf(occ->start_ypos4, "%d", y);
			sprintf(occ->end_xpos4, "%d", endx);
			sprintf(occ->end_ypos4, "%d", endy);
			sprintf(occ->enable4, "%d", enable);
			printf("set x %s y %s endx %s endt %s enable %s \n", 
					occ->start_xpos4, occ->start_ypos4, occ->end_xpos4, occ->end_ypos4, occ->enable4);
			break;
		}
	}

	IniSetting_akisave();
	IniSetting_akidestroy();
	return 0;
}

int SetBrightness(int bright)
{
	struct v4l2_control control;
	memset (&control, 0, sizeof (control));
	control.id = V4L2_CID_BRIGHTNESS;
	control.value = bright;// + 2;
	
	if (-1 == ioctl (fd, VIDIOC_S_CTRL, &control)) 
	{
		perror ("SetBrightness err \n");
		return -1;
	}

	return 0;
}

int SetGAMMA(int cid)
{
	struct v4l2_control control;
	memset (&control, 0, sizeof (control));
	control.id = V4L2_CID_GAMMA;
	control.value = cid;
	if (-1 == ioctl (fd, VIDIOC_S_CTRL, &control))
	{
		perror ("Set cid err \n");
		return -1;
	}

	return 0;
}


int SetSATURATION(int sat)
{
	struct v4l2_control control;
	memset (&control, 0, sizeof (control));
	control.id = V4L2_CID_SATURATION;
	control.value = sat;

	
	if (-1 == ioctl (fd, VIDIOC_S_CTRL, &control))
	{
		printf("set Saturation \n");
		return -1;
	}
	return 0;
}

int SetIsp(unsigned char *pbuf, int size)
{
	struct v4l2_streamparm parm;

	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	memcpy(parm.parm.raw_data, pbuf, size);

	if (-1 == xioctl(fd, VIDIOC_S_PARM, &parm)) 
	{
        printf("VIDIOC_S_PARM(ISP_CID_ISP_PARM)");

         return -1;
   } else

   printf("ISP_CID_RGB_FILTER succeedded\n");

   return 0;
}

int GetIsp_awb(void* awbbuf, int* awbbuflen)
{
	if(awbbuf == NULL)
		return -1;
	if(*awbbuflen < sizeof(struct isp_white_balance))
	{
		printf("getisp awb buf len is not enough\n");
		return -1;	
	}
	
	*awbbuflen = sizeof(struct isp_white_balance);
	struct v4l2_streamparm parm;
	struct isp_white_balance *co_rgb_info;
	CLEAR(parm);

	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	co_rgb_info = (struct isp_white_balance *)parm.parm.raw_data;
	co_rgb_info->type = ISP_CID_AUTO_WHITE_BALANCE;
    
	if (-1 == xioctl(fd, VIDIOC_G_PARM, &parm)) {
		printf("VIDIOC_G_PARM(ISP_CID_AUTO_WHITE_BALANCE)");
		return -1;
	}
	printf("read AWB paramter succeedded,%d,%d,%d\n", 
		co_rgb_info->co_r, co_rgb_info->co_g, co_rgb_info->co_b);
	memcpy(awbbuf, co_rgb_info, *awbbuflen);
	return 0;
}
#if 0
static int isp_set_rgb_filter(void)
{        

         struct v4l2_streamparm parm;

         struct isp_rgb_filter *rgb_filter;

         
         CLEAR(parm);

    	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

         rgb_filter = (struct isp_rgb_filter *) parm.parm.raw_data;  

         rgb_filter->type = ISP_CID_RGB_FILTER;

         rgb_filter->enable = 1;

         rgb_filter->threshold = 142;

 
         if (-1 == xioctl(fd, VIDIOC_S_PARM, &parm)) 
		 {
        	printf("VIDIOC_S_PARM(ISP_CID_RGB_FILTER)");

                   return -1;
    	} else

        printf("ISP_CID_RGB_FILTER succeedded\n");

         return 0;

}
#endif

/**
* @brief open camera to get picture
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_open(unsigned long width, unsigned long height)
{

    struct stat st;

    if (-1 == stat(dev_name, &st)) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n",
                 dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", dev_name);
        exit(EXIT_FAILURE);
    }

    fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (-1 == fd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n",
                 dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }


	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n",
					 dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n",
				 dev_name);
		exit(EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf(stderr, "%s does not support streaming i/o\n",
				 dev_name);
		exit(EXIT_FAILURE);
	}
	/* Select video input, video standard and tune here. */

	CLEAR(cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
			crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			crop.c = cropcap.defrect; /* reset to default */

			if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
				switch (errno) {
				case EINVAL:
						/* Cropping not supported. */
						break;
				default:
						/* Errors ignored. */
						break;
				}
			} else {
				printf("init_device==>S_CROP succeedded!reset to defrect[%d, %d]\n", 
					crop.c.width, crop.c.height);
			}
			
	} else {
			/* Errors ignored. */
	}

	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (force_format) {
			int m_width = width;
			int m_height = height;
			if( 320 == width )
			{
				m_width = 640;
				m_height = 480;
			}
			fmt.fmt.pix.width       = m_width;
			fmt.fmt.pix.height      = m_height;
			fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
			fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

			if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
					errno_exit("VIDIOC_S_FMT");
	   else
			printf("init_device==>S_FMT succeedded!\n");
			/* Note VIDIOC_S_FMT may change width and height. */
	} else {
			/* Preserve original settings as set by v4l2-ctl for example */
			if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
					errno_exit("VIDIOC_G_FMT");
	}

	if(320 == width)
	{
		Zoom(0, 0, 640, 480, 320, 240, 1);
	}

	//isp_set_rgb_filter();

	InitOcc();
    /*
	switch( width )
	{
		case 1280:
			g_size = 48;
			break;
		case 640:
		case 720:
			g_size = 32;
			break;
		case 320:
			g_size = 16;
			break;
		default:
			g_size = 32;
			break;
	}
	switch( parse.width2 )
	{
		case 1280:
			g_size2 = 48;
			break;
		case 640:
		case 720:
			g_size2 = 32;
			break;
		case 320:
			g_size2 = 16;
			break;
		default:
			g_size2 = 32;
			break;
	}
    */
    g_size = 16;
    g_size2 = 16;
	read_ini();
	Init_Font(g_size, timefont);
	Init_Font(g_size2, timefont2);
	if(g_time_osd != 4)
	{
		SetOsd1( g_size );
		SetOsd2( g_size2 );
	}
	else
	{
		Take_out_osd();
	}
	
	if ( pthread_create( &ThredOsdID, NULL, thread_enc, NULL ) != 0 ) 
	{
			
		printf( "unable to create a thread for osd !\n" );
			//return -1;	
	}

	SetChannel(parse.real_width2, parse.real_height2, 1);


	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
			fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
			fmt.fmt.pix.sizeimage = min;

	
	fmt.fmt.pix.sizeimage += (parse.real_width2*parse.real_height2*3/2);
	//fmt.fmt.pix.sizeimage = (parse.width*parse.height*3/2)+(parse.width2*parse.height2*3/2);
	init_userp(fmt.fmt.pix.sizeimage);


        unsigned int i;
        enum v4l2_buf_type type;

        for (i = 0; i < BUFF_NUM; ++i) {
            struct v4l2_buffer buf;

            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long)buffers[i].start;
            buf.length = buffers[i].length;

            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
  			else
				printf("start_capturing==>QBUF succeedded!\n");
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                errno_exit("VIDIOC_STREAMON");
		else		
  		printf("start_capturing==>STREAMON succeedded!\n");

	struct v4l2_streamparm parm;
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_occlusion_color *p_occ_color = (struct isp_occlusion_color *) parm.parm.raw_data;
	p_occ_color->type = ISP_PARM_OCCLUSION_COLOR;
	p_occ_color->color_type = 0;
	p_occ_color->transparency = 0;
	p_occ_color->y_component = RGB2Y(0, 0, 0);
	p_occ_color->u_component = RGB2U(0, 0, 0);
	p_occ_color->v_component = RGB2V(0, 0, 0);
	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	{
		printf("Set occ color err \n");
	}
	else
	{
		printf("Set occ color success \n");
	}


	return 1;
}

/**
* @brief get one frame from camera
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_getframe(void **pbuf, long *size, unsigned long *timeStamp)
{
	//T_VIDEO_OSD_REC_ICON mVideoStamp;

	while(1)
	{
	    fd_set fds;
	    struct timeval tv;
	    int r;

	    FD_ZERO(&fds);
	    FD_SET(fd, &fds);

	    /* Timeout. */
	    tv.tv_sec = 2;
	    tv.tv_usec = 0;

	    r = select(fd + 1, &fds, NULL, NULL, &tv);

	    if (-1 == r) {
			fprintf(stderr, "select err\n");
	       return 0;
	    }

	    if (0 == r) {
	            fprintf(stderr, "select timeout\n");
	          return 0;
	    }

	    if ( read_frame() )
		{
			*pbuf = (void*)buf.m.userptr;
			*size = buf.length;//g_camera.width*g_camera.height*3/2;
			*timeStamp = buf.timestamp.tv_sec * 1000ULL + buf.timestamp.tv_usec / 1000ULL;
	        break;
		}
	    /* EAGAIN - continue select loop. */
	}
	return 1;
}

/**
* @brief call this when camera buffer unused
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_usebufok(void *pbuf)
{
/*	int i;

	for (i = 0; i < BUFF_NUM; ++i) 
	{
	   if (buf.m.userptr == (unsigned long)pbuf)
				break;
	}
*/
	if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");

	return 1;
}

/**
* @brief close camera
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_close(void)
{
	if (AK_NULL != g_camera.pframebuf)
	{
		g_camera.dma_free(g_camera.pframebuf);
	}
	printf("osd exit \n");
	g_osd_exit = 1;

	{
		pthread_join(ThredOsdID , NULL);
		ThredOsdID	= thread_zeroid();
	}
	printf("osd exit2 \n");
	{
	        enum v4l2_buf_type type;
			type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
				printf("VIDIOC_STREAMOFF");
			
	        if (ion_mem) {
				akuio_free_pmem(ion_mem);
				ion_mem = NULL;
			}

			free(buffers);

			if( AK_NULL != osd_buff1 )
			{
				akuio_free_pmem(osd_buff1);
				osd_buff1 = AK_NULL;
			}

			if( AK_NULL != osd_buff2 )
			{
				akuio_free_pmem(osd_buff2);
				osd_buff2 = AK_NULL;
			}
			osd_buff = AK_NULL;


			if( AK_NULL != osd2_buff1 )
			{
				akuio_free_pmem(osd2_buff1);
				osd2_buff1 = AK_NULL;
			}

			if( AK_NULL != osd2_buff2 )
			{
				akuio_free_pmem(osd2_buff2);
				osd2_buff2 = AK_NULL;
			}
			osd2_buff = AK_NULL;
			printf("close fd \n");
	        if (-1 == close(fd))
	                printf("close");
			printf("close fd exit \n");

	        fd = -1;
	}

	return 1;
}
