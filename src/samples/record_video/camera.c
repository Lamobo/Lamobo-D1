#include "camera.h"
#include "anyka_types.h"
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

#include "AkFontLib.h"
#include "log.h"
#include "Tool.h"
#include "isp_interface.h"
#include "font.h"
#include "Mutex.h"
#include "Thread.h"
#include "akuio.h"

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
struct v4l2_buffer v4l2Buf;
static char            *dev_name = "/dev/video0";
static int              fd = -1;
struct buffer          *buffers;
static int              force_format = 1;
static void *ion_mem;
static char *osd_buff = AK_NULL;
pthread_mutex_t g_fd_Mutex;
nthread_t	 ThredOsdID;
int 	g_osd_exit = 0;
extern int g_width;
extern int g_height;

static int xioctl(int fh, int request, void *arg)
{
    int r;
	Mutex_Lock(&g_fd_Mutex);
    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);
	
	Mutex_Unlock(&g_fd_Mutex);
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
	CLEAR(v4l2Buf);

	v4l2Buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2Buf.memory = V4L2_MEMORY_USERPTR;
	v4l2Buf.m.userptr = 0;
	if (-1 == xioctl(fd, VIDIOC_DQBUF, &v4l2Buf)) 
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
		if (v4l2Buf.m.userptr == (unsigned long)buffers[i].start)
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
            fprintf(stderr, "%s does not support user pointer i/o\n", dev_name);
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

	ion_mem = akuio_alloc_pmem(buffer_size*BUFF_NUM);
	memset(ion_mem, 0x00, buffer_size*BUFF_NUM);
	if (!ion_mem) {
		fprintf(stderr, "Allocate %d bytes failed\n", buffer_size*BUFF_NUM);
	}
	printf("init_userp==>ion_mem=0x%p\n", ion_mem);

	T_U32 temp;
	T_U8 *ptemp;
	
	temp = akuio_vaddr2paddr(ion_mem) & 7;
	//编码buffer 起始地址必须8字节对齐
	ptemp = ((T_U8 *)ion_mem) + ((8-temp)&7);

    for (n_buffers = 0; n_buffers < BUFF_NUM; ++n_buffers) {
        buffers[n_buffers].length = buffer_size;
		// buffers[n_buffers].start = malloc(buffer_size);
   	   	buffers[n_buffers].start = ptemp + buffer_size * n_buffers;
	   	printf("init_userp==>[%d]start=0x%p, length=%d\n", 
	   	n_buffers, buffers[n_buffers].start, buffers[n_buffers].length);

        if (!buffers[n_buffers].start) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
	printf("init_userp -\n");
}

int Zoom(int x, int y, int width, int heigth, int outw, int outh)
{
	struct v4l2_streamparm parm;

	printf("zoom x=%d,y=%d, sw=%d, sh=%d, dw = %d, dh = %d\n", x, y, width, heigth, outw, outh);
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	struct isp_zoom_info *zoom = (void*)parm.parm.raw_data;
	zoom->type 			= ISP_PARM_ZOOM;
	zoom->cut_xpos 		= x;
	zoom->cut_ypos 		= y;
	zoom->cut_width 	= width;
	zoom->cut_height 	= heigth;
	zoom->out_width 	= outw;
	zoom->out_height 	= outh;
	
	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	{
		printf("Set occ zoom err\n");
		return -1;
	}
	
	printf("Set occ zoom success\n");
	
	return 0;

}

int Set_Zoom(int z)
{
	static int width = 0;
	static int height = 0;
	static int stepwidth = 0;
	static int stepheight = 0;
	int x, y, w, h;
	
	width 		= g_width;
	stepwidth 	= ((int)(width*0.14) >> 4) << 4;

	height 		= g_height;
	stepheight 	= ((int)(height*0.14) >> 4) << 4;

	if ( z < 0 || z > 4)
	{
		printf("ZOOM Level Is Out of Define, Z: %d\n", z);
		return -1;
	}
	
	x = z*(stepwidth/2);
	y = z*(stepheight/2);
	w = width - z*stepwidth;
	h = height - z*stepheight;

	Zoom(x, y, w, h, width, height);

	return 0;	
}


int SetChannel(int width, int height, int enable)
{
	struct v4l2_streamparm parm;
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_channel2_info *p_mode = (void*)parm.parm.raw_data;
	printf("SetChannel width=%d, height=%d enable = %d\n", width, height, enable);
	p_mode->type 	= ISP_PARM_CHANNEL2;
	p_mode->width 	= width;
	p_mode->height 	= height;
	p_mode->enable 	= enable;

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

static void SetOcc(int x1, int y1, int x2, int y2, int enable)
{
	struct v4l2_streamparm parm;
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_occlusion_info *p_occ = (void*)parm.parm.raw_data;

	p_occ->type 		= ISP_PARM_OCCLUSION;
	p_occ->channel 		= 1;
	p_occ->number 		= 1;
	p_occ->start_xpos 	= x1;
	p_occ->start_ypos 	= y1;
	p_occ->end_xpos 	= x2;
	p_occ->end_ypos 	= y2;
	p_occ->enable 		= enable;
	
	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))	{
		printf("Set occ err \n");
	}
	else {
		printf("Set occ success \n");
	}
}

void * p_draw[11]=
{
	osd_buff_0,	// 0
	osd_buff_1, // 1
	osd_buff_2, // 2
	osd_buff_3, // 3
	osd_buff_4, // 4
	osd_buff_5, // 5
	osd_buff_6, // 6
	osd_buff_7, // 7
	osd_buff_8, // 8
	osd_buff_9, // 9
	osd_buff_d  // : 
};

void draw(int n, int num)
{
	char (* osd)[20];
	osd = p_draw[num];
	char t,m;
	char *p;
	int i, j;
	
	for (i=0; i< 20; i++)
	{
		p = osd_buff+(i*(200/2))+n*10;
		char *k = osd[i];
		for (j = 0; j<20; )
		{
			t = *k++;
			m = *k++;
			
			*p = (t & 0xf) | ((m&0xf)<<4);
			
			p++;
			j += 2;
		}
	}
}

void draw_sec(int sec)
{
	if (sec < 10)
	{
		draw(7, 0);
	}
	else
	{
		draw(7, sec/10);
	}
	draw(8, sec%10);
}

void draw_min(int min)
{
	if (min < 10)
	{
		draw(4, 0);
	}
	else
	{
		draw(4, min/10);
	}
	draw(5, min%10);
}

void draw_hour(int hour)
{
	if (hour < 10)
	{
		draw(1, 0);
	}
	else
	{
		draw(1, hour/10);
	}
	draw(2, hour%10);
}

static int create_osd_picture(void *pcdev)
{
	int startX 		= 0;
	int startY 		= 0;
	int endX 		= 200;
	int endY 		= 20;	
	int osd_width 	= endX - startX + 1;
	int osd_height 	= endY - startY + 1;
	int picsize 	= osd_width * osd_height;
	
	int i,j;
	char *p;
	char (*osd)[20];

	if (!osd_buff) {
		osd_buff = akuio_alloc_pmem(picsize / 2 + picsize %2);
		
		if (!osd_buff)
			return -1;
		memset(osd_buff, 0x00, (picsize / 2 + picsize % 2));

		//printf("osd_buff is %p \n", osd_buff);
		osd = p_draw[10];
		char t,m;
		for (i=0; i< 20; i++)
		{
			p = osd_buff+(i*(200/2))+30;
			char *k = osd[i];
			for(j = 0; j<20; )
			{
				
				t = *k++;			
				m = *k++;			
				*p = (t & 0xf) | ((m&0xf)<<4);	
				p++;
				j += 2;
			}
			
		}

		for (i=0; i< 20; i++)
		{
			p = osd_buff+(i*(200/2))+60;
			char *k = osd[i];
			for(j = 0; j<20; )
			{			
				t = *k++;			
				m = *k++;			
				*p = (t & 0xf) | ((m&0xf)<<4);			
				p++;
				j += 2;
			}
		}
	}
	
	time_t ta;
	static int ss = -1;
	static int mm = -1;
	static int hh = -1;
	
	time(&ta);
	struct tm * tt = localtime(&ta);
	if (ss != tt->tm_sec){
		draw_sec(tt->tm_sec);
		ss = tt->tm_sec;
	}
	if (mm != tt->tm_min){
		draw_min(tt->tm_min);
		mm = tt->tm_min;
	}
	if (hh != tt->tm_hour){
		draw_hour(tt->tm_hour);
		hh = tt->tm_hour;
	}

	return 0;
}

void Take_out_osd(int channel)
{
	struct v4l2_streamparm parm;
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_osd_info *p_osd = (void*)parm.parm.raw_data;
	
	p_osd->type 		= ISP_PARM_OSD;
	p_osd->channel 		= channel;
	p_osd->color_depth 	= 16;
	p_osd->color_transparency = 0;
	p_osd->start_xpos 	= 300;
	p_osd->start_ypos 	= 300;
	p_osd->end_xpos 	= 500;
	p_osd->end_ypos 	= 320;
	p_osd->enable 		= 0;

	if (0 != xioctl(fd, VIDIOC_S_PARM, &parm))	{
		printf("take out osd err \n");
	}

}

int SetOsd()
{
	struct v4l2_streamparm parm;
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_osd_info *p_osd = (void*)parm.parm.raw_data;
	
	//Take_out_osd();
	if (create_osd_picture(AK_NULL))
	{
		printf("create osd picture err \n");
		return -1;
	}
	
	p_osd->type 		= ISP_PARM_OSD;
	p_osd->channel 		= 1;
	p_osd->color_depth 	= 16;
	p_osd->color_transparency = 0;
	p_osd->start_xpos 	= g_width-200;
	p_osd->start_ypos 	= 0;
	p_osd->end_xpos 	= g_width;
	p_osd->end_ypos 	= 20;
	p_osd->enable 		= 1;

	//printf("color_transparency = %d", p_osd->color_transparency);
	if (osd_buff) {
		p_osd->phys_addr = akuio_vaddr2paddr(osd_buff);
	}
	else {
		return -1;
	}

	if ( 0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	{
		printf("Set osd err \n");
	}

	return 0;
}

static T_pVOID thread_setOSD( T_pVOID user )
{
	while (!g_osd_exit)
	{
		SetOsd();
		sleep(1);		
	}

	if (osd_buff) {
		akuio_free_pmem(osd_buff);
		osd_buff = AK_NULL;
	}

	return NULL;
}

#if 0
static int isp_set_rgb_filter(void)
{        

    struct v4l2_streamparm parm;
    struct isp_rgb_filter *rgb_filter;
     
    CLEAR(parm);

    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rgb_filter = parm.parm.raw_data;  
    rgb_filter->type = ISP_CID_RGB_FILTER;
    rgb_filter->enable = 1;
    rgb_filter->threshold = 142;
 
    if (-1 == xioctl(fd, VIDIOC_S_PARM, &parm)) 
	{
    	errno_exit("VIDIOC_S_PARM(ISP_CID_RGB_FILTER)");

        return -1;
    }
	else
    	printf("ISP_CID_RGB_FILTER succeedded\n");

    return 0;
}
#endif

int camera_start(void)
{
	unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < BUFF_NUM; ++i) 
	{
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

	return 0;
}

/**
* @brief open camera to get picture
* @author dengzhou 
* @date 2013-04-25
* @param[in] 
* @return T_S32
* @retval if return 0 success, otherwise failed 
*/
int camera_open(demo_setting* Setting)
{
    struct stat st;
	
    if (-1 == stat(dev_name, &st)) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", dev_name);
        exit(EXIT_FAILURE);
    }

    fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    if (-1 == fd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

	Mutex_Initialize(&g_fd_Mutex);
	
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	struct v4l2_streamparm parm;
	unsigned int min;

	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf(stderr, "%s is no V4L2 device\n", dev_name);
			exit(EXIT_FAILURE);
		} else {
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf(stderr, "%s is no video capture device\n", dev_name);
		exit(EXIT_FAILURE);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
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
		
	}
	else {
			/* Errors ignored. */
	}

	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (force_format) {
		fmt.fmt.pix.width       = Setting->width;
		fmt.fmt.pix.height      = Setting->height;
		fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
		fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

		if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
				errno_exit("VIDIOC_S_FMT");
		else
		printf("init_device==>S_FMT succeedded!\n");
		/* Note VIDIOC_S_FMT may change width and height. */
	}
	else {
		/* Preserve original settings as set by v4l2-ctl for example */
		if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
			errno_exit("VIDIOC_G_FMT");
	}

	//isp_set_rgb_filter();

	printf("mode is %ld\n", Setting->mode );

	SetOsd();
	Take_out_osd(2);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&ThredOsdID, &attr, thread_setOSD, NULL) != 0 ) 
	{		
		loge( "unable to create a thread for osd = %d!\n" );
		//return -1;	
	}
	pthread_attr_destroy(&attr);
	if (Setting->mode == 1)
	{
		//Zoom(Setting->x1, Setting->y1, Setting->width2, Setting->height2, Setting->times);
		Set_Zoom(Setting->times);
	}

	if (Setting->mode == 2)
	{
		SetChannel(Setting->width2, Setting->height2, 1);
	}
	else
	{
		SetChannel(Setting->width2, Setting->height2, 0);
	}

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;
	fmt.fmt.pix.sizeimage += (Setting->width2*Setting->height2*3/2);
	printf("set sizeimge = %d \n", fmt.fmt.pix.sizeimage);
	init_userp(fmt.fmt.pix.sizeimage);

	if (Setting->mode == 3)
	{
		SetOcc(Setting->x1, Setting->y1, Setting->width2, Setting->height2, 1);
	}
	else
	{
		SetOcc(Setting->x1, Setting->y1, Setting->width2, Setting->height2, 0);
	}

#if 1
	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	struct isp_occlusion_color *p_occ_color = (void*)parm.parm.raw_data;
	p_occ_color->type = ISP_PARM_OCCLUSION_COLOR;
	p_occ_color->color_type = 2;
	p_occ_color->transparency = 0;
	
	if (0 != xioctl(fd, VIDIOC_S_PARM, &parm))
	{
		printf("Set occ color err \n");
	}
	else
	{
		printf("Set occ color success \n");
	}
#endif
	
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
int camera_getframe(void **ppBuf, unsigned long *size, unsigned long *timeStamp)
{
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
	        printf("select \n");
			return 0;
	    }

	    if (0 == r) {
	        fprintf(stderr, "select timeout\n");
	        return 0;
	    }

	    if (read_frame())
		{
			*ppBuf = (void*)v4l2Buf.m.userptr;
			*size = v4l2Buf.length;//g_camera.width*g_camera.height*3/2;
			*timeStamp = v4l2Buf.timestamp.tv_sec * 1000ULL + v4l2Buf.timestamp.tv_usec / 1000ULL;
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
int camera_usebufok(void)
{
	if (-1 == xioctl(fd, VIDIOC_QBUF, &v4l2Buf))
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
	g_osd_exit = 1;
	if (AK_NULL != g_camera.pframebuf)
	{
		g_camera.dma_free(g_camera.pframebuf);
	}
	
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
		errno_exit("VIDIOC_STREAMOFF");

	if (ion_mem) {
		akuio_free_pmem(ion_mem);
		ion_mem = NULL;
	}

	free(buffers);

	if (-1 == close(fd))
	    errno_exit("close");

	fd = -1;
	
	Mutex_Destroy(&g_fd_Mutex);
	return 1;
}
