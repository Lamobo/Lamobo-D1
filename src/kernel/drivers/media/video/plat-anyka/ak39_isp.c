#include <linux/slab.h>
#include <linux/hardirq.h>

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <plat-anyka/aksensor.h>


#include "ak39_isp.h"
#include "isp_param.h"

//#define ISP_DEBUG
#ifdef ISP_DEBUG
#define isp_dbg(stuff...)		printk(KERN_INFO " ISP: " stuff)
#else
#define isp_dbg(fmt, args...)	do{}while(0)
#endif 
#define isp_info(stuff...)		printk(KERN_INFO " ISP: " stuff)


#define BRIGHTNESS_CHART_SIZE	64
#define HISTOGRAM_SIZE			(1024*3)

#define ABS_DIFF(a, b)   (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define AWB_LOOP_FRAME_NUM  5
#define G_TOTAL_THRESH 50000  
#define CT_STABLE_CNT	2
#define OUT_CYCLE_END  5
#define AWB_CALC_PARA_NUM 5
struct awb_rgb_param {
	unsigned short r[3];
	unsigned short b[3];
	unsigned short g[3];
};

struct awb_para_screen
{
	unsigned long channel_g_total[4];//4              
	unsigned short r_gain[4];
	unsigned short b_gain[4];
};


unsigned int   updata_auto_exposure_num = 0;
#define  UPDATA_AUTO_EXPOSURE_FREQUENCE 1

static unsigned int brightness_average(struct isp_struct *isp);
int isp_aec_time_calc(struct isp_struct  *isp);
static unsigned long histo_arr[256] = {0};

//the gamma_table by called build_gamma_table()
unsigned long gamma_table[7][BRIGHTNESS_CHART_SIZE] = {
	{ 0x15141312,  0x18171615,  0x1b1b1a19,  0x1f1e1d1c,  0x22212120, //disable gamma
	0x26252423,  0x29282727,  0x2d2c2b2a,  0x302f2e2d,  0x33333231,
	0x37363534,  0x3a393838,  0x3e3d3c3b,  0x41403f3e,  0x44444342,
	0x48474645,  0x4b4a4a49,  0x4f4e4d4c,  0x52515050,  0x56555453,
	0x59585756,  0x5c5c5b5a,  0x605f5e5d,  0x63626261,  0x67666564,
	0x6a696868,  0x6e6d6c6b,  0x71706f6e,  0x74747372,  0x78777675,
	0x7b7a7a79,  0x7f7e7d7c,  0x82818080,  0x85858483,  0x89888786,
	0x8c8b8b8a,  0x908f8e8d,  0x93929191,  0x97969594,  0x9a999897,
	0x9d9d9c9b,  0xa1a09f9e,  0xa4a3a3a2,  0xa8a7a6a5,  0xabaaa9a9,
	0xafaeadac,  0xb2b1b0af,  0xb5b5b4b3,  0xb9b8b7b6,  0xbcbbbbba,
	0xc0bfbebd,  0xc3c2c1c1,  0xc7c6c5c4,  0xcac9c8c7,  0xcdcccccb,
	0xd1d0cfce,  0xd4d3d2d2,  0xd8d7d6d5,  0xdbdad9d8,  0xdededddc,
	0xe2e1e0df,  0xe5e4e4e3,  0xe9e8e7e6,  0xecebeaea},
	{ 0x0c080400,  0x201c1810,  0x26252422,  0x24252626,  0x27262525, //ycx
	0x29282827,  0x2a2a2929,  0x2d2c2c2b,  0x31302f2e,  0x35343332,
	0x39383736,  0x3d3c3b3a,  0x41403f3e,  0x46454342,  0x4a494847,
	0x4f4e4c4b,  0x53525150,  0x58575554,  0x5c5b5a59,  0x5f5e5e5d,
	0x62616160,  0x65646363,  0x69676766,  0x6d6c6b6a,  0x7371706f,
	0x78777574,  0x7c7b7a79,  0x807f7e7d,  0x85838281,  0x89888786,
	0x8d8c8b8a,  0x92918f8e,  0x97959493,  0x9b9a9998,  0x9f9e9d9c,
	0xa3a2a1a0,  0xa7a6a5a4,  0xaaa9a9a8,  0xaeadacab,  0xb1b0b0af,
	0xb5b4b3b2,  0xb8b7b6b5,  0xbcbbbab9,  0xc0bfbebd,  0xc3c2c1c1,
	0xc6c5c4c4,  0xc8c7c7c6,  0xcacac9c9,  0xcbcbcbca,  0xcecdcccc,
	0xd1d0cfce,  0xd5d4d3d2,  0xd9d8d7d6,  0xdddcdbda,  0xe1e0dfde,
	0xe4e4e3e2,  0xe8e7e6e5,  0xecebeae9,  0xf0efeeed,  0xf3f2f1f1,
	0xf6f5f5f4,  0xf9f8f8f7,  0xfcfbfaf9,  0xfefefdfc},
	{ 0x10101010, 0x11111111, 0x12121212, 0x14131313, 0x15151514, 0x17171616, 0x19191818, 0x1b1b1a1a, 
	0x1e1e1d1c, 0x20201f1f, 0x23222121, 0x25252423, 0x28272726, 0x2b2a2a29, 0x2e2d2c2c, 0x31302f2f, 
	0x34333232, 0x37363635, 0x3a393938, 0x3d3c3c3b, 0x403f3f3e, 0x44434241, 0x47464544, 0x4a494948, 
	0x4d4d4c4b, 0x51504f4e, 0x54535251, 0x57565655, 0x5b5a5958, 0x5f5e5d5c, 0x62616060, 0x65646362, 
	0x68676665, 0x6b6a6969, 0x6e6e6d6c, 0x7271706f, 0x76757473, 0x7a797877, 0x7e7d7c7b, 0x8281807f, 
	0x87868583, 0x8c8b8988, 0x91908e8d, 0x95949492, 0x99989796, 0x9d9c9b9a, 0xa09f9f9e, 0xa4a3a2a1, 
	0xa8a7a6a5, 0xadabaaa9, 0xb1b0afae, 0xb5b4b3b2, 0xb9b8b7b6, 0xbebdbbba, 0xc2c1c0bf, 0xc7c5c4c3, 
	0xcbcac9c8, 0xd0cfcdcc, 0xd4d3d2d1, 0xd9d8d7d5, 0xdedcdbda, 0xe2e1e0df, 0xe7e6e5e3, 0xebebe9e8},
	{ 0x4020101, 0xc0a0806, 0x1412100e, 0x1c1a1816, 0x2422201e, 0x2d2b2926, 0x3533312f, 0x3d3b3937, 
	0x4543413f, 0x4e4b4947, 0x5251504f, 0x57555453, 0x5b5a5958, 0x605e5d5c, 0x64636261, 0x69686665, 
	0x6d6c6b6a, 0x7271706e, 0x76757473, 0x7b7a7978, 0x807e7d7c, 0x84838281, 0x89878685, 0x8d8c8b8a, 
	0x91908f8e, 0x96959493, 0x9a999897, 0x9e9e9c9b, 0xa1a1a09f, 0xa4a4a3a2, 0xa7a7a6a5, 0xaaaaa9a8,
	0xadadacab, 0xb0b0afae, 0xb3b2b2b1, 0xb6b5b5b4, 0xb9b8b7b7, 0xbbbbbab9, 0xbebdbdbc, 0xc1c0bfbf, 
	0xc3c3c2c1, 0xc6c5c5c4, 0xc8c8c7c6, 0xcbcac9c9, 0xcdcccccb, 0xcfcfcecd, 0xd1d1d0d0, 0xd3d3d2d2, 
	0xd5d5d4d4, 0xd7d7d6d6, 0xd9d9d8d8, 0xdbdadad9, 0xdddcdcdb, 0xe0e0dfde, 0xe3e2e2e1, 0xe5e5e4e4, 
	0xe7e7e6e6, 0xe9e9e8e8, 0xeaeaeae9, 0xecebebeb, 0xededecec, 0xeeeeeded, 0xefefeeee, 0xf0f0efef},
	{ 0x10101010, 0x12121111, 0x13131312, 0x15151414, 0x17161615, 0x19181817, 0x1b1b1a19, 0x1e1d1d1c, 
	0x21201f1f, 0x25242322, 0x29282726, 0x2e2c2b2a, 0x3332302f, 0x37363534, 0x3c3b3a38, 0x41403f3d, 
	0x47464443, 0x4d4b4a48, 0x5351504e, 0x59575654, 0x5f5d5c5a, 0x65636260, 0x6b6a6867, 0x71706e6d, 
	0x77767473, 0x7d7c7a79, 0x8382807f, 0x89878684, 0x8e8d8b8a, 0x9392908f, 0x98969594, 0x9c9b9a99, 
	0xa09f9e9d, 0xa4a3a2a1, 0xa8a7a6a5, 0xacabaaa9, 0xafaeadad, 0xb2b1b1b0, 0xb5b5b4b3, 0xb8b8b7b6, 
	0xbbbabab9, 0xbebdbdbc, 0xc1c0bfbf, 0xc3c3c2c1, 0xc6c5c5c4, 0xc8c7c7c6, 0xcac9c9c8, 0xcccbcbca, 
	0xcecdcdcc, 0xd0cfcfce, 0xd2d1d1d0, 0xd4d3d3d2, 0xd6d5d5d4, 0xd8d7d7d6, 0xdad9d9d8, 0xdcdbdbda, 
	0xdedddddc, 0xe0dfdfde, 0xe1e1e0e0, 0xe3e3e2e2, 0xe5e5e4e4, 0xe7e7e6e6, 0xe9e9e8e8, 0xebebeaea},
	{ 0x10101010, 0x10101010, 0x11111111, 0x12121211, 0x13131312, 0x15151414, 0x17171616, 0x1a191918, 
	0x1e1d1c1b, 0x2221201f, 0x27262523, 0x2e2c2a29, 0x32302f2e, 0x36353433, 0x3b393837, 0x3f3e3d3c, 
	0x45434241, 0x4a494746, 0x504e4d4b, 0x55545251, 0x5b5a5857, 0x615f5e5c, 0x67656462, 0x6d6b6a68, 
	0x7371706e, 0x79777674, 0x7f7d7c7a, 0x85838280, 0x8b898886, 0x908f8d8c, 0x96949392, 0x9b9a9897, 
	0xa09f9e9c, 0xa5a4a2a1, 0xa9a8a7a6, 0xaeadacaa, 0xb2b1b0af, 0xb6b5b4b3, 0xb9b9b8b7, 0xbdbcbbba, 
	0xc0c0bfbe, 0xc4c3c2c1, 0xc7c6c5c4, 0xc9c9c8c7, 0xcccbcbca, 0xcfcecdcd, 0xd1d0d0cf, 0xd3d3d2d1, 
	0xd5d5d4d4, 0xd7d7d6d6, 0xd9d8d8d8, 0xdbdadad9, 0xdcdcdbdb, 0xdfdedddd, 0xe1e0e0df, 0xe3e2e2e1, 
	0xe5e4e4e3, 0xe6e6e5e5, 0xe7e7e7e6, 0xe8e8e8e7, 0xe9e9e8e8, 0xe9e9e9e9, 0xeaeaeaea, 0xebebeaea},	
	{ 0x15131110, 0x1c1a1817, 0x23211f1e, 0x2a282725, 0x32302e2c, 0x39383634, 0x3d3c3b3a, 0x42413f3e, 
	0x46454443, 0x4b4a4948, 0x504f4e4c, 0x55545251, 0x5a595756, 0x5f5e5c5b, 0x64636260, 0x68676666, 
	0x6c6b6a69, 0x706f6e6d, 0x74737271, 0x78777675, 0x7c7b7a79, 0x807f7e7d, 0x84838281, 0x89888785, 
	0x8d8c8b8a, 0x91908f8e, 0x95949392, 0x99989796, 0x9c9b9a99, 0x9f9e9d9c, 0xa2a1a0a0, 0xa5a4a4a3, 
	0xa8a7a7a6, 0xabaaaaa9, 0xaeadadac, 0xb1b0b0af, 0xb4b3b3b2, 0xb7b6b5b5, 0xbab9b8b8, 0xbcbcbbba, 
	0xbfbfbebd, 0xc2c1c1c0, 0xc5c4c3c3, 0xc7c7c6c5, 0xc9c9c8c8, 0xcbcacaca, 0xcdcccccb, 0xcfcececd, 
	0xd1d0d0cf, 0xd3d2d2d1, 0xd4d4d3d3, 0xd6d6d5d5, 0xd8d8d7d7, 0xdad9d9d8, 0xdcdbdbda, 0xdddddcdc, 
	0xdfdfdede, 0xe1e0e0df, 0xe2e2e2e1, 0xe4e4e3e3, 0xe6e5e5e5, 0xe8e7e7e6, 0xe9e9e8e8, 0xebebeaea},
};

/* YUV 640x480, continuous mode, temporarily used */
void setup_yuv_video_out(struct isp_struct *pcdev)
{
	pcdev->isp_ctrltbl[3] = 0x00000000;
	pcdev->isp_ctrltbl[4] = 0x00000000;
	pcdev->isp_ctrltbl[5] = 0x00000000;
	pcdev->isp_ctrltbl[6] = 0x00000000;
	pcdev->isp_ctrltbl[7] = 0x00000000;
	pcdev->isp_ctrltbl[8] = 0x00000000;
	pcdev->isp_ctrltbl[9] = 0x00000000;
	pcdev->isp_ctrltbl[10] = 0x00000000;
	pcdev->isp_ctrltbl[11] = 0x00000000;
	pcdev->isp_ctrltbl[12] = 0x00000000;
	pcdev->isp_ctrltbl[13] = 0x00001c68;
	pcdev->isp_ctrltbl[14] = 0x00005538;
	pcdev->isp_ctrltbl[15] = 0x00008e08;
	pcdev->isp_ctrltbl[16] = 0x4000c6d8;
	pcdev->isp_ctrltbl[23] = 0x00000000;
	pcdev->isp_ctrltbl[24] = 0x00000000;
	pcdev->isp_ctrltbl[27] = 0x00022bb0;
	pcdev->isp_ctrltbl[28] = 0x0027bbb0;
	pcdev->isp_ctrltbl[29] = 0x00000000;
	pcdev->isp_ctrltbl[30] = 0x00000000;
	pcdev->isp_ctrltbl[31] = 0x00000000;

	pcdev->img_ctrltbl1[15] = 0x00000000;

	pcdev->osd_chktbl[0] = 0x00008080;
	pcdev->osd_chktbl[1] = 0x00ff8080;
	pcdev->osd_chktbl[2] = 0x00c08080;
	pcdev->osd_chktbl[3] = 0x00266ac0;
	pcdev->osd_chktbl[4] = 0x0071408a;
	pcdev->osd_chktbl[5] = 0x004b554a;
	pcdev->osd_chktbl[6] = 0x00599540;
	pcdev->osd_chktbl[7] = 0x000ec075;
	pcdev->osd_chktbl[8] = 0x0034aab5;
	pcdev->osd_chktbl[9] = 0x00786085;
	pcdev->osd_chktbl[10] = 0x002c8aa0;
	pcdev->osd_chktbl[11] = 0x0068d535;
	pcdev->osd_chktbl[12] = 0x0034aa5a;
	pcdev->osd_chktbl[13] = 0x0043e9ab;
	pcdev->osd_chktbl[14] = 0x004b55a5;
	pcdev->osd_chktbl[15] = 0x00008080;
	pcdev->osd_chktbl[16] = 0x00000000;
	pcdev->osd_chktbl[17] = 0x00000000;
	pcdev->osd_chktbl[18] = 0x00000000;
	pcdev->osd_chktbl[19] = 0x00000000;
	pcdev->osd_chktbl[20] = 0x00000000;
	pcdev->osd_chktbl[21] = 0x00000000;
	pcdev->osd_chktbl[22] = 0x00000000;
	pcdev->osd_chktbl[23] = 0x00000000;
	pcdev->osd_chktbl[24] = 0x00000000;
	pcdev->osd_chktbl[25] = 0x00000000;
	pcdev->osd_chktbl[26] = 0x00000000;
	pcdev->osd_chktbl[27] = 0x00000000;
	pcdev->osd_chktbl[28] = 0x00000000;
	pcdev->osd_chktbl[29] = 0x00000000;
	pcdev->osd_chktbl[30] = 0x00000000;
	pcdev->osd_chktbl[31] = 0x00000000;

	pcdev->img_ctrltbl2[25] = 0x00000000;
	pcdev->img_ctrltbl2[26] = 0x00009000;
}


void setup_yuv_video_bypass(struct isp_struct *pcdev)
{
	pcdev->isp_ctrltbl[3] = 0x00000000;
	pcdev->isp_ctrltbl[4] = 0x00000000;
	pcdev->isp_ctrltbl[5] = 0x00000000;
	pcdev->isp_ctrltbl[6] = 0x00000000;
	pcdev->isp_ctrltbl[7] = 0x00000000;
	pcdev->isp_ctrltbl[8] = 0x00000000;
	pcdev->isp_ctrltbl[9] = 0x00000000;
	pcdev->isp_ctrltbl[10] = 0x00000000;
	pcdev->isp_ctrltbl[11] = 0x00000000;
	pcdev->isp_ctrltbl[12] = 0x00000000;
	pcdev->isp_ctrltbl[13] = 0x00001c68;
	pcdev->isp_ctrltbl[14] = 0x00005538;
	pcdev->isp_ctrltbl[15] = 0x00008e08;
	pcdev->isp_ctrltbl[16] = 0x4000c6d8;
	pcdev->isp_ctrltbl[23] = 0x00000000;
	pcdev->isp_ctrltbl[24] = 0x00000000;
	pcdev->isp_ctrltbl[27] = 0x0001e0f8;
	pcdev->isp_ctrltbl[28] = 0x002770f8;
	pcdev->isp_ctrltbl[29] = 0x00000000;
	pcdev->isp_ctrltbl[30] = 0x00000000;
	pcdev->isp_ctrltbl[31] = 0x00000000;

	pcdev->img_ctrltbl1[15] = 0x00000000;

	//0x2000002c
//	memset(pcdev->osd_chktbl, 0 , sizeof(pcdev->osd_chktbl));	
	pcdev->osd_chktbl[0] = 0x00008080;
	pcdev->osd_chktbl[1] = 0x00ff8080;
	pcdev->osd_chktbl[2] = 0x00c08080;
	pcdev->osd_chktbl[3] = 0x00266ac0;
	pcdev->osd_chktbl[4] = 0x0071408a;
	pcdev->osd_chktbl[5] = 0x004b554a;
	pcdev->osd_chktbl[6] = 0x00599540;
	pcdev->osd_chktbl[7] = 0x000ec075;
	pcdev->osd_chktbl[8] = 0x0034aab5;
	pcdev->osd_chktbl[9] = 0x00786085;
	pcdev->osd_chktbl[10] = 0x002c8aa0;
	pcdev->osd_chktbl[11] = 0x0068d535;
	pcdev->osd_chktbl[12] = 0x0034aa5a;
	pcdev->osd_chktbl[13] = 0x0043e9ab;
	pcdev->osd_chktbl[14] = 0x004b55a5;
	pcdev->osd_chktbl[15] = 0x00008080;
	pcdev->osd_chktbl[16] = 0x00000000;
	pcdev->osd_chktbl[17] = 0x00000000;
	pcdev->osd_chktbl[18] = 0x00000000;
	pcdev->osd_chktbl[19] = 0x00000000;
	pcdev->osd_chktbl[20] = 0x00000000;
	pcdev->osd_chktbl[21] = 0x00000000;
	pcdev->osd_chktbl[22] = 0x00000000;
	pcdev->osd_chktbl[23] = 0x00000000;
	pcdev->osd_chktbl[24] = 0x00000000;
	pcdev->osd_chktbl[25] = 0x00000000;
	pcdev->osd_chktbl[26] = 0x00000000;
	pcdev->osd_chktbl[27] = 0x00000000;
	pcdev->osd_chktbl[28] = 0x00000000;
	pcdev->osd_chktbl[29] = 0x00000000;
	pcdev->osd_chktbl[30] = 0x00000000;
	pcdev->osd_chktbl[31] = 0x00000000;

	pcdev->img_ctrltbl2[25] = 0x00000000;
	pcdev->img_ctrltbl2[26] = 0x00000000;
}

void setup_rgb_video(struct isp_struct *pcdev)
{
	pcdev->isp_ctrltbl[3] = 0x00000000;
	pcdev->isp_ctrltbl[4] = 0x00000000;
	pcdev->isp_ctrltbl[5] = 0x00000000;
	pcdev->isp_ctrltbl[6] = 0x00000000;
	pcdev->isp_ctrltbl[7] = 0x00000000;
	pcdev->isp_ctrltbl[8] = 0x00000000;
	pcdev->isp_ctrltbl[9] = 0x00000000;
	pcdev->isp_ctrltbl[10] = 0x00000000;
	pcdev->isp_ctrltbl[11] = 0x00000000;
	pcdev->isp_ctrltbl[12] = 0x00000000;
	pcdev->isp_ctrltbl[13] = 0x00005fbd;
	pcdev->isp_ctrltbl[14] = 0x00011f37;
	pcdev->isp_ctrltbl[15] = 0x0001deb1;
	pcdev->isp_ctrltbl[16] = 0x40029e2b;
	pcdev->isp_ctrltbl[23] = 0x00000000;
	pcdev->isp_ctrltbl[24] = 0x00000000;
	pcdev->isp_ctrltbl[27] = 0x0001ed68;
	pcdev->isp_ctrltbl[28] = 0x01537168;
	pcdev->isp_ctrltbl[29] = 0x00000000;
	pcdev->isp_ctrltbl[30] = 0x00000000;
	pcdev->isp_ctrltbl[31] = 0x00000000;
	
	pcdev->img_ctrltbl1[15] = 0x00000000;

	//0x2000002c
	pcdev->osd_chktbl[0] = 0x00008080;
	pcdev->osd_chktbl[1] = 0x00ff8080;
	pcdev->osd_chktbl[2] = 0x00c08080;
	pcdev->osd_chktbl[3] = 0x00266ac0;
	pcdev->osd_chktbl[4] = 0x0071408a;
	pcdev->osd_chktbl[5] = 0x004b554a;
	pcdev->osd_chktbl[6] = 0x00599540;
	pcdev->osd_chktbl[7] = 0x000ec075;
	pcdev->osd_chktbl[8] = 0x0034aab5;
	pcdev->osd_chktbl[9] = 0x00786085;
	pcdev->osd_chktbl[10] = 0x002c8aa0;
	pcdev->osd_chktbl[11] = 0x0068d535;
	pcdev->osd_chktbl[12] = 0x0034aa5a;
	pcdev->osd_chktbl[13] = 0x0043e9ab;
	pcdev->osd_chktbl[14] = 0x004b55a5;
	pcdev->osd_chktbl[15] = 0x00008080;
	pcdev->osd_chktbl[16] = 0x00000000;
	pcdev->osd_chktbl[17] = 0x00000000;
	pcdev->osd_chktbl[18] = 0x00000000;
	pcdev->osd_chktbl[19] = 0x00000000;
	pcdev->osd_chktbl[20] = 0x00000000;
	pcdev->osd_chktbl[21] = 0x00000000;
	pcdev->osd_chktbl[22] = 0x00000000;
	pcdev->osd_chktbl[23] = 0x00000000;
	pcdev->osd_chktbl[24] = 0x00000000;
	pcdev->osd_chktbl[25] = 0x00000000;
	pcdev->osd_chktbl[26] = 0x00000000;
	pcdev->osd_chktbl[27] = 0x00000000;
	pcdev->osd_chktbl[28] = 0x00000000;
	pcdev->osd_chktbl[29] = 0x00000000;
	pcdev->osd_chktbl[30] = 0x00000000;
	pcdev->osd_chktbl[31] = 0x00000000;

	pcdev->img_ctrltbl2[25] = 0x00000000;
	pcdev->img_ctrltbl2[26] = 0x00008800;
}

void isp_restart_update_param(struct isp_struct *isp)
{
	// disable the status, then enable
	REG32(isp->base + ISP_IRQ_STATUS) |= 
			(1 << (12+ISP_FRAME_UPDATE))|(1 << ISP_FRAME_UPDATE);
}

/**
 * @brief: update regtable before comming
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] force: force update if 1.
 */
void isp_update_regtable(struct isp_struct *isp, int force)
{
	if (force)
		REG32(isp->base + ISP_ORDER_CTRL) = ((3<<30) | (isp->addr & 0x3fffffff));
	else
		REG32(isp->base + ISP_ORDER_CTRL) = ((1<<31) | (isp->addr & 0x3fffffff));
}

/**
 * @brief: change ISP current mode if user change sensor
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 */
int	update_cur_mode_class(struct isp_struct *isp)
{
	switch(isp->cur_mode) {
	case ISP_JPEG_MODE:
	case ISP_JPEG_VIDEO:
		isp->cur_mode_class = ISP_JPEG_CLASS;
		break;
	case ISP_YUV_OUT:
	case ISP_YUV_BYPASS:
	case ISP_YUV_MERGER_OUT:
	case ISP_YUV_BIG:
	case ISP_YUV_VIDEO_OUT:
	case ISP_YUV_VIDEO_BYPASS:
	case ISP_YUV_VIDEO_MERGER_OUT:
		isp->cur_mode_class = ISP_YUV_CLASS;
		break;
	case ISP_RGB_OUT:
	case ISP_RGB_VIDEO_OUT:
	case ISP_RGB_BIG:
		isp->cur_mode_class = ISP_RGB_CLASS;
		break;
	}
	return 0;
}

/**
 * @brief: setting cutter window size for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] co_g:  G component 
 * @param [in] co_r, R component
 * @param [in] co_b, B component
 */
static void __isp_set_white_pram(struct isp_struct *isp, 
	unsigned int co_g, unsigned int co_r, unsigned int co_b)
{
	isp->img_ctrltbl1[0] &= ~(0xffffff);
	isp->img_ctrltbl1[1] &= ~(0xfff);
	isp->img_ctrltbl1[0] |= ((co_g & 0xfff) << 12)|(co_r & 0xfff);
	isp->img_ctrltbl1[1] |= (co_b & 0xfff);
}

static int isp_set_auto_wb_support(struct isp_struct *isp, 
		struct isp_auto_white_balance *ctrl)
{
	int i;
	
	for (i = 0; i < 6; i++)
		isp->isp_ctrltbl[17+i] &= ~(0x3ff << 22);
	isp->isp_ctrltbl[25] = 0x0;
	isp->isp_ctrltbl[26] = 0x0;
	
	isp->isp_ctrltbl[17] |= ((ctrl->r_high & 0x3ff) << 22);
	isp->isp_ctrltbl[18] |= ((ctrl->r_low & 0x3ff) << 22);
	isp->isp_ctrltbl[19] |= ((ctrl->g_high & 0x3ff) << 22);
	isp->isp_ctrltbl[20] |= ((ctrl->g_low & 0x3ff) << 22);
	isp->isp_ctrltbl[21] |= ((ctrl->b_high & 0x3ff) << 22);
	isp->isp_ctrltbl[22] |= ((ctrl->b_low & 0x3ff) << 22);

	// enable white balance calculate
	isp->isp_ctrltbl[25] = (1 << 31)|((ctrl->grb_high & 0x3ff) << 20)
				|((ctrl->grb_low & 0x3ff) << 10) 
				|(ctrl->gr_low & 0x3ff);
	isp->isp_ctrltbl[26] = ((ctrl->gb_high & 0x3ff) << 20)
				|((ctrl->gb_high & 0x3ff) << 10) 
				|(ctrl->gb_low & 0x3ff);


	//isp->wb_param.co_g = 1024;
	//isp->wb_param.co_r = 1024;
	//isp->wb_param.co_b = 1024;

	__isp_set_white_pram(isp, isp->wb_param.co_g, 
				isp->wb_param.co_r, isp->wb_param.co_b);
	return 0;
}

static int __enable_brightness_enhance(struct isp_struct *isp)
{
	isp->img_ctrltbl1[12] = 0x0;
	if ((isp->chl2_enable)
		&& ((isp->fmt_def_width == 640)||(isp->fmt_def_height == 480))
		&& ((isp->chl2_width == isp->fmt_def_width)||(isp->chl2_height == isp->fmt_def_height))) {
	
		// disable edge(brightness) adjust
		isp->img_ctrltbl1[11] &= ~(1 << 29);
		isp->yedge_en = 0;
		
		return 0;
	}
	return 1;
}

static void __isp_set_color_crr(struct isp_struct *isp, struct isp_color_correct *ctrl,
	int row, int col)
{
	int min[3][3];
	unsigned long cmd = 0;
	int value, i, j;
	
	memset(min, 0, 9 * sizeof(int));
	min[0][0] = min[1][1] = min[2][2] = (1 << 10);

	if (ctrl->cc_thrs_high - ctrl->cc_thrs_low <= 1) {
		ctrl->cc_thrs_high = ctrl->cc_thrs_low + 1;
	}

	for (i = 0; i < row; i++) {
		for (j= 0; j<col; j++)
			;//isp_dbg("	ctrl->ccMtrx[%d][%d] = %d\n", i, j, ctrl->ccMtrx[i][j]);
	}

	for (i = 0; i < row; i++) {
		ctrl->ccMtrx[i][0] = (ctrl->ccMtrx[i][0] > 2560)? 2560 
				: ((ctrl->ccMtrx[i][0] < -2560) ? (-2560) : ctrl->ccMtrx[i][0]);
		ctrl->ccMtrx[i][1] = (ctrl->ccMtrx[i][1] > 2560)? 2560 
				: ((ctrl->ccMtrx[i][1] < -2560) ? (-2560) : ctrl->ccMtrx[i][1]);
		ctrl->ccMtrx[i][2] = (ctrl->ccMtrx[i][2] > 2560)? 2560 
				: ((ctrl->ccMtrx[i][2] < -2560) ? (-2560) : ctrl->ccMtrx[i][2]);

		value = (ctrl->ccMtrx[i][0] - min[i][0])/(ctrl->cc_thrs_high - ctrl->cc_thrs_low);
		cmd = ((value & 0x1fff) << CC_CALVAL_BIT) | ((ctrl->ccMtrx[i][0] & 0x1fff) << CC_TMPVAL_BIT);
		isp->img_ctrltbl1[2+3*i] = cmd;
			
		value = (ctrl->ccMtrx[i][1] - min[i][1])/(ctrl->cc_thrs_high - ctrl->cc_thrs_low);
		cmd = ((value & 0x1fff) << CC_CALVAL_BIT) | ((ctrl->ccMtrx[i][1] & 0x1fff) << CC_TMPVAL_BIT);
		isp->img_ctrltbl1[2+3*i+1] = cmd;

		value = (ctrl->ccMtrx[i][2] - min[i][2])/(ctrl->cc_thrs_high - ctrl->cc_thrs_low);
		cmd = ((value & 0x1fff) << CC_CALVAL_BIT) | ((ctrl->ccMtrx[i][2] & 0x1fff) << CC_TMPVAL_BIT);
		isp->img_ctrltbl1[2+3*i+2] = cmd;
	}

	// enable color correction
	isp->img_ctrltbl1[11] |= ((1 << 31)|((ctrl->cc_thrs_high & 0x7ff) << 11) 
				|(ctrl->cc_thrs_low & 0x7ff));
}

static void __isp_set_uv_saturation(struct isp_struct *isp, struct isp_saturation *ctrl)
{
	ctrl->Chigh = (ctrl->Chigh > 255) ? 255 : (ctrl->Chigh < 0) ? 0 : ctrl->Chigh;
	ctrl->Clow = (ctrl->Clow > 255) ? 255 : (ctrl->Clow < 0) ? 0 : ctrl->Clow;

	if (ctrl->Chigh <= ctrl->Clow) 
		ctrl->Chigh = ctrl->Clow + 1;

	if (ctrl->Khigh <= ctrl->Klow)
		ctrl->Khigh = ctrl->Klow + 1;
	
	// enable saturation adjust
	ctrl->Kslope = (ctrl->Khigh - ctrl->Klow) * 256 / (ctrl->Chigh - ctrl->Clow);
	isp->img_ctrltbl1[13] |= (1 << 31)|(ctrl->Klow << 20)|(ctrl->Khigh << 10)|ctrl->Kslope;
	isp->img_ctrltbl1[14] |= (ctrl->Chigh << 24)|(ctrl->Clow << 16);
}

static void __isp_set_sharpness(struct isp_struct *isp,  
		struct isp_brightness_enhance *ctrl)
{
	ctrl->y_thrs = (ctrl->y_thrs > 2048) ? 2048 : ctrl->y_thrs;
	ctrl->y_edgek = (ctrl->y_edgek > 2048) ? 2048 : ctrl->y_edgek;

	isp->img_ctrltbl1[12] &= ~(0xffffff);
	isp->img_ctrltbl1[12] |= (((ctrl->y_thrs * 9) & 0x7ff) << Y_EDGE_THRS_BIT)
			|((ctrl->y_edgek & 0x7ff) << Y_EDGE_K_BIT);
}

static void __isp_set_brigtness(struct isp_struct *isp,  
		struct isp_brightness_enhance *ctrl)
{
	ctrl->ygain = (ctrl->ygain > 64) ? 63 : 
					((ctrl->ygain < -64) ? -63 : ctrl->ygain);

	isp->img_ctrltbl1[12] &= ~(0xff << Y_GAIN_BIT);
	isp->img_ctrltbl1[12] |= (ctrl->ygain << Y_GAIN_BIT);

	// enable edge(brightness) adjust
	isp->img_ctrltbl1[11] |= (1 << 29);
	// enable iso filter 
	//isp->img_ctrltbl1[13] |= (1 << 30);
}

static void __isp_set_gamma(struct isp_struct *isp, unsigned long *gamma)
{
	int i;
	
	for(i = 0; i < BRIGHTNESS_CHART_SIZE; i++) {
		isp->img_lumitbl[i] = gamma[i];
	}
}

static void __isp_clear_lens_param(struct isp_struct *isp)
{
	int i;
	
	for (i = 0; i < 10; i++)
		isp->isp_ctrltbl[3+i] = 0x0;

	for (i = 0; i < 10; i++)
		isp->isp_ctrltbl[13+i] &= ~(0x3fffff);

	isp->isp_ctrltbl[23] = 0x0;
	isp->isp_ctrltbl[24] = 0x0;
}

static void __isp_set_lens_correct(struct isp_struct *isp, struct isp_lens_correct *ctrl)
{
	int i;
	
	// lens coef setting
	for (i = 0; i < 10; i++) {	
		ctrl->lens_coefa[i] = (ctrl->lens_coefa[i] > 1023)?1023
			: ((ctrl->lens_coefa[i] < -1023) ? (-1023): ctrl->lens_coefa[i]);
		ctrl->lens_coefb[i] = (ctrl->lens_coefb[i] > 1023)?1023
			: ((ctrl->lens_coefb[i] < -1023) ? (-1023): ctrl->lens_coefb[i]);
		ctrl->lens_coefc[i] = (ctrl->lens_coefc[i] > 511)?511
			: ((ctrl->lens_coefc[i] < -511) ? (-511): ctrl->lens_coefc[i]);
		
		isp->isp_ctrltbl[3+i] = ((ctrl->lens_coefa[i] & 0x7ff) << LENS_COEFA_BIT) 
				|((ctrl->lens_coefb[i] & 0x7ff) << LENS_COEFB_BIT) 
				|(ctrl->lens_coefc[i] & 0x3ff);
	}

	for (i = 0; i < 10; i++) {
		isp->isp_ctrltbl[13+i] = (ctrl->lens_range[i] < 0) ? 0 : ctrl->lens_range[i];
	}
	
	isp->isp_ctrltbl[23] = ((ctrl->lens_yref & 0x1fff) << LENS_YREF_BIT) 
				|(ctrl->lens_xref & 0x1fff);
	isp->isp_ctrltbl[24] = (1 << 31)|(((ctrl->lens_xref * ctrl->lens_xref)
				+ (ctrl->lens_yref * ctrl->lens_yref)) & 0x7ffffff);
}

static void __set_special_effect(struct isp_struct *isp, struct isp_special_effect *ctrl)
{
	if (ctrl->solar_enable) 
		isp->img_ctrltbl1[0] |= (ctrl->solar_thrs & 0xff) << YUV_SOLAR_THRD_BIT;

	isp->img_ctrltbl1[14] |= ((ctrl->y_eff_coefa < 0) << (SPEC_YCOEFA_BIT+7))
		|(((abs(ctrl->y_eff_coefa)) & 0x7f) << SPEC_YCOEFA_BIT) 
		|((ctrl->y_eff_coefb & 0xff) << SPEC_YCOEFB_BIT);

	isp->img_ctrltbl1[15] = ((ctrl->u_eff_coefa < 0) << (SPEC_UCOEFA_BIT+7)) 
		|(((abs(ctrl->u_eff_coefa)) & 0x7f) << SPEC_UCOEFA_BIT) 
		|(ctrl->u_eff_coefb << SPEC_UCOEFB_BIT) 
		|((ctrl->v_eff_coefa < 0) << (SPEC_VCOEFA_BIT+7)) 
		|(((abs(ctrl->v_eff_coefa)) & 0x7f) << SPEC_VCOEFA_BIT) 
		|(ctrl->v_eff_coefb << SPEC_VCOEFB_BIT);

	return;
}

/**
 * @brief: checking irq status and handler.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 */
unsigned int his_frame_cnt = 0;
int isp_check_irq(struct isp_struct *isp)
{
	unsigned int irq_status;
	int retval = 0;
	//int ch_flag = 0;
	
	irq_status = REG32(isp->base + ISP_IRQ_STATUS);
	//printk("isp irq: %#x, %d\n", irq_status, ((irq_status >> 27) & 0x1f));
	if (!(irq_status & (1 << ISP_FRAME_UPDATE))) {
		isp_restart_update_param(isp);
		isp_update_regtable(isp, 1);
		retval = -EINTR;
	}

	if (irq_status & (1 << ISP_FRAME_UPDATE)) {
		if(isp->cur_mode_class == ISP_RGB_CLASS)
		{	
				if(1==his_frame_cnt)
				{
				    isp_get_histogram(isp);
					isp_set_histogram(isp, NULL);				
					his_frame_cnt = 0;
				}
			    else
			    {
			    	
					  his_frame_cnt=1;
					
			    }
		}
	}

#if 0
	if (irq_status & (1 << ISP_CH1_UPDMA_OVERFLOW)) {
		printk("ch1 updma overflow\n");
	}

	if (!(irq_status & (1<<ISP_CH1_ONE_FRAME_END))) {
		REG32(isp->base + ISP_IRQ_STATUS) |= 
			(1<<(12+ISP_CH1_ONE_FRAME_END))|(1<<ISP_CH1_ONE_FRAME_END);
		ch_flag = 1;		
	}
	
	if (isp->chl2_enable) {
		if (!(irq_status & (1<<ISP_CH2_ONE_FRAME_END))) {
			REG32(isp->base + ISP_IRQ_STATUS) |= 
				(1<<(12+ISP_CH2_ONE_FRAME_END))|(1<<ISP_CH2_ONE_FRAME_END);
			ch_flag = 1;
		}
	}
	
	if (ch_flag) {
		isp_stop_capturing(isp);
		//udelay(100);
		isp_start_capturing(isp);
		retval = -EINTR;
	}
	
	if (irq_status & (1<<ISP_BIG_PIC_MODE_DONE))
		retval = 0;
#endif

	if (retval < 0) {
		printk("ISP interrupt exception. irq status=0x%08x\n", irq_status);
	}
	
	return retval;
}

/**
 * @brief: clear irq status
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 */
int isp_clear_irq_status(struct isp_struct *isp)
{
	REG32(isp->base + ISP_IRQ_STATUS) |= 0x7ff;
	return 0;
}

/**
 * @brief: disable irq
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 */
int isp_clear_irq(struct isp_struct *isp)
{
	REG32(isp->base + ISP_IRQ_STATUS) = 0;
	return 0;
}

/**
 * @brief: image frame(even frame mean first frame) for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] yaddr_chl1: main channel image size
 * @param [in] yaddr_chl2: sub channel image size
 */
void isp_set_even_frame(struct isp_struct *isp, 
				unsigned long yaddr_chl1, unsigned long yaddr_chl2)
{
	int size;

	size = isp->chl1_width * isp->chl1_height;
	isp->img_ctrltbl2[12] = yaddr_chl1;
	isp->img_ctrltbl2[14] = yaddr_chl1 + size;
	isp->img_ctrltbl2[16] = yaddr_chl1 + size * 5 / 4;

	if (isp->chl2_enable) {
		size = isp->chl2_width * isp->chl2_height;
		//isp->img_ctrltbl2[18] &= (3 << 30);
		isp->img_ctrltbl2[18] = yaddr_chl2 & ~(3 << 30);
		isp->img_ctrltbl2[20] = yaddr_chl2 + size;
		isp->img_ctrltbl2[22] = yaddr_chl2 + size * 5 /4;
	}
}

/**
 * @brief: image frame(odd frame mean next frame) for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] yaddr_chl1: main channel image size
 * @param [in] yaddr_chl2: sub channel image size
 */
void isp_set_odd_frame(struct isp_struct *isp, 
				unsigned long yaddr_chl1, 	unsigned long yaddr_chl2)
{
	int size;

	size = isp->chl1_width * isp->chl1_height;
	isp->img_ctrltbl2[13] = yaddr_chl1;
	isp->img_ctrltbl2[15] = yaddr_chl1 + size;
	isp->img_ctrltbl2[17] = yaddr_chl1 + size * 5 / 4;

	if (isp->chl2_enable) {
		size = isp->chl2_width * isp->chl2_height;
		isp->img_ctrltbl2[19] = yaddr_chl2;
		isp->img_ctrltbl2[21] = yaddr_chl2 + size;
		isp->img_ctrltbl2[23] = yaddr_chl2 + size * 5 /4;
	}	
}

/**
 * @brief: handler image boundary for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 */
static int update_image_size(struct isp_struct *isp)
{
	int	iwidth = isp->cut_width;
	int	iheight = isp->cut_height;

	// the follow setup parameter only ov9712 
	if (isp->cur_mode_class == ISP_RGB_CLASS) {
		iwidth -= 4;
		iheight -= 4;
	}
	
	iwidth  = (isp->demo_sac_en)?(iwidth-8):iwidth;
	iheight = (isp->demo_sac_en)?(iheight-6):iheight;
	
	if (isp->uv_isoflt_en || isp->yedge_en) {
		iwidth  = iwidth - 6;
		iheight = iheight - 6;
	}
	
	iwidth  = (isp->rgb_filter_en)?(iwidth-4):iwidth;
	iheight = (isp->rgb_filter_en)?(iheight-4):iheight;

	// here is set isp OSD size, handler edge
	isp->img_ctrltbl2[24] = ((iheight & 0x1fff) << OSD_SIZE_V_BIT)
							|(iwidth & 0x1fff);

	// save is here, used for lens_range
	isp->lens_use_width = iwidth;
	isp->lens_use_height = iheight;
	
	isp_dbg("%s: osd total size[w=%d h=%d]\n", __func__, iwidth, iheight);
	return 0;
}

/**
 * @brief: Settting OSD for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *osd_info: struct isp_osd_info.
 */
int isp_set_osd(struct isp_struct *isp, struct isp_osd_info *osd_info)
{
	int base = 1;

	if (osd_info->channel == 2)
		base = 4;

	isp->img_ctrltbl2[base] = 0;
	isp->img_ctrltbl2[base + 1] = 0;
	isp->img_ctrltbl2[base + 2] = 0;
	
	isp->img_ctrltbl2[base + 1] = ((osd_info->color_transparency & 0xf) << 28)
				|(osd_info->start_ypos << 13) | osd_info->start_xpos;
	isp->img_ctrltbl2[base + 2] = (osd_info->end_ypos << 13) | osd_info->end_xpos;

	if (osd_info->enable)
		isp->img_ctrltbl2[base] = (1 << 31) | (osd_info->phys_addr & 0x3fffffff);
	else 
		isp->img_ctrltbl2[base] = (0 << 31) | (osd_info->phys_addr & 0x3fffffff);

	return 0;
}

/**
 * @brief: Enable main channel default for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] width, height: output image size
 */
int isp_set_channel1_scale(struct isp_struct *isp, int width, int height)
{
	int xrate, yrate;
	
	if (height < 0 || width < 0) {
		printk(KERN_ERR "%s: channel1 scale remain the same\n", __func__);
		return -EINVAL;
	}
	
	xrate = (65536)/(width - 1);
	yrate = (65536)/(height - 1);

	isp_dbg("%s: xrate=%d yrate=%d width=%d height=%d\n", __func__, 
			xrate, yrate, width, height);

	isp->img_ctrltbl2[8] &= ~((0x1fff << 13)|0x1fff);
	
	isp->img_ctrltbl2[7] = (yrate << 16)|xrate;	
	isp->img_ctrltbl2[8] |= (height << 13)|width;
	isp->img_ctrltbl2[9] |= (0xff << UPSCALE_FRAME_CTRL);

	isp->chl1_width = width;
	isp->chl1_height = height;

	return 0;
}

/**
 * @brief: setting cutter window size for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] xpos, xpos:  Image orignal position
 * @param [in] width, height: needed image size
 */
int isp_set_cutter_window(struct isp_struct *isp, int xpos, int ypos, int width, int height)
{
    struct isp_demosaic demosac;
    struct isp_rgb_filter rgb_filter;
    int iwidth, iheight;
    
    iwidth = width;
    iheight = height;
    isp->cut_width = width;
    isp->cut_height = height;

    if (!strcmp(get_sensor_name(), "ov9712")) {
        if ((width == 1280)&&(height == 720)) {
            iwidth = width;
            iheight = height + 20;  // 740
            isp->cut_width = width;
            isp->cut_height = iheight;
        }
    } else {
		if ((width == 640)&&(height == 480)) {
            iwidth = width;
            iheight = height-120; // 360
            isp->cut_width = width;
            isp->cut_height = iheight;
        }
	}

	isp_dbg("%s. entry. iwidth=%d iheight=%d\n", __func__, 
			iwidth, iheight);
#if 0
	if (width > isp->fmt_width || height > isp->fmt_height) {
		printk(KERN_ERR"%s: invalid cutter window size\n", __func__);
		return -EINVAL;
	}
#endif

	if (xpos > isp->fmt_width || ypos > isp->fmt_height) {
		printk(KERN_ERR"%s: invalid cutter window position\n", __func__);
		return -EINVAL;
	}

	if (width < 0 || height < 0 || xpos < 0 || ypos < 0) {
		printk(KERN_WARNING"%s: cutter window remain the same\n", __func__);
		return -EINVAL;
	}

	isp->sub_sample = 0;
	xpos -= xpos % 4;

	// the follow setup parameter only ov9712 
	if (isp->cur_mode_class == ISP_RGB_CLASS) {
		xpos += 3;
		ypos += 3;
		iwidth -= 4;
		iheight -= 4;
	}

	isp_dbg("cutter_window: xpos=%d, ypos=%d, iwidth=%d, iheight=%d\n",
			xpos, ypos, iwidth, iheight);
	
	// here is set isp output size
	isp->isp_ctrltbl[0] = (isp->sub_sample << DSMP_RTO_BIT)
				|(ypos << CUT_STRY_BIT)|xpos;
	isp->isp_ctrltbl[1] = (iheight << CUT_WINH_BIT) | iwidth;

	if (isp->cur_mode_class == ISP_RGB_CLASS) {
		demosac.enable = 1;
		rgb_filter.enable = 1;
	} else {
		demosac.enable = 0;
		rgb_filter.enable = 0;
	}
	
	// should enable demo saic in RGB module
	if (isp->demo_sac_thres <= 0) {
		demosac.threshold = 0;
		isp->demo_sac_thres = demosac.threshold;
	} else
		demosac.threshold = isp->demo_sac_thres;
	isp_set_demosaic(isp, &demosac);

	// enable rgb filter (noise remove)
	if (isp->rgb_filter_thres <= 0) {
		rgb_filter.threshold = 31;
		isp->rgb_filter_thres = rgb_filter.threshold;
	} else
		rgb_filter.threshold = isp->rgb_filter_thres;
	isp_set_rgb_filter(isp, &rgb_filter);
	
	update_image_size(isp);
	return 0;
}

/**
 * @brief: Enable channel 2 for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *isp:  struct isp_channel2_info.
 */
int isp_set_channel2(struct isp_struct *isp, struct isp_channel2_info *chl2_info)
{
	int width = chl2_info->width;
	int height = chl2_info->height;

	isp_dbg("%s:\n\twidth=%d height=%d isp->chl1_width=%d isp->chl1_height=%d chl2_info->enable=%d\n",
			__func__, width, height, isp->chl1_width, isp->chl1_height, chl2_info->enable);

	if ((isp->cur_mode_class == ISP_RGB_CLASS)
		&&((isp->fmt_def_width == 640)||(isp->fmt_def_height == 480))
		&&((width == 640)||(height == 480))) {
		printk(KERN_ERR "isp: chl2 isn't support VGA output as sensor VGA input\n");
		return -EINVAL;
	}

	/* CH2 support output size up to as follow */
	if ((width > 720) || (height > 576)) {
		printk(KERN_ERR "isp: size is higher, chl2 isn't support.\n");
		return -EINVAL;
	}

	if ((isp->chl1_width < width)||(isp->chl1_height < height)) {
		printk(KERN_ERR "%s: wrong channel2 width and height.\n", __func__);
		return -EINVAL;
	}

	isp->img_ctrltbl2[10] = (65536 / (height-1) << 16) | (65536 / (width -1));
	isp->img_ctrltbl2[11] =  (height << 10) | width;

	/* Enable channel 2 */
	if (chl2_info->enable) {
		isp->img_ctrltbl2[9] |= 0xff << 24;
	} else {
		isp->img_ctrltbl2[9] &= ~(0xff << 24);
	}

	isp->chl2_width = width;
	isp->chl2_height = height;
	isp->chl2_enable = chl2_info->enable;

	return 0;
}

/**
 * @brief: Setting occlustion area for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *isp:  struct isp_occlusion_info.
 */
int isp_set_occlusion_area(struct isp_struct *isp, struct isp_occlusion_info *oclu_info)
{
	int target_area;
	
	if (oclu_info->number > 4 || oclu_info->number < 1)
		return -EINVAL;

	target_area = (oclu_info->number - 1) * 2 + 16;
	if (oclu_info->channel == 2)
		target_area += 4 * 2;

	isp->img_ctrltbl1[target_area] = (oclu_info->start_ypos << 16) | oclu_info->start_xpos;
	isp->img_ctrltbl1[target_area + 1] = (oclu_info->end_ypos << 16) | oclu_info->end_xpos;

	/* Enable the occlusion for target area */
	if (!oclu_info->enable) {
		isp->img_ctrltbl1[target_area] &= ~(1 << 31);
	} else {
		isp->img_ctrltbl1[target_area] |= (1 << 31);
	}

	isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: Setting occlustion color for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *isp:  struct isp_occlusion_color.
 */
int isp_set_occlusion_color(struct isp_struct *isp, struct isp_occlusion_color *color_info)
{
	isp->img_ctrltbl2[0] = 0x0;
	
	isp->img_ctrltbl2[0] = ((color_info->color_type & 3) << 30)
			|((color_info->transparency & 0xf) << 24)
			|((color_info->y_component & 0xff) << 16)
			|((color_info->u_component & 0xff) << 8)
			|(color_info->v_component & 0xff);
	
	isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: Setting Zoom for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *isp:  struct isp_zoom_info.
 */
int isp_set_zoom(struct isp_struct *isp, struct isp_zoom_info *info)
{
	struct isp_channel2_info chl2;
	int offset_w = 0, offset_h = 0;

	isp_dbg("%s: entry. \ncut_xpos=%d cut_ypos=%d cut_width=%d cut_height=%d\n"
			"out_width=%d out_height=%d channel=%d chl2_enable=%d\n", __func__, 
			info->cut_xpos, info->cut_ypos,	info->cut_width, info->cut_height,
			info->out_width, info->out_height, info->channel, isp->chl2_enable);

	if ((info->out_width > 1280) || (info->out_height > 720)) {
		printk(KERN_ERR "isp: output scale is too big\n");
		return -EINVAL;
	}

	if ((info->cut_width < 6 || info->cut_height < 6)
		||(info->out_width < 18 || info->out_height < 18)) {
		printk(KERN_ERR "isp: the scale of ch1 remain the same\n");
		return -EINVAL;
	}

	if (isp->cur_mode_class == ISP_RGB_CLASS) {
		offset_w = 22;
		offset_h = 20;
	} else if (isp->cur_mode_class == ISP_YUV_CLASS) {
		offset_w = 6;
		offset_h = 6;
	}

	if (isp->chl2_enable) {
		// the second channel is not support up scale
		if (info->channel == 2) {
			if ((info->cut_width < info->out_width+offset_w)||(info->cut_height < info->out_height+offset_h)) {
				printk(KERN_ERR "isp: ch2 isn't support up-scale.\n");
				return -EINVAL;
			}
		} else if ((info->channel == 1)||(info->channel == 0)) {
			if ((info->cut_width < isp->chl2_width+offset_w)||(info->cut_height < isp->chl2_height+offset_h)) {
				printk(KERN_ERR "isp: cut window should be larger than ch2 output.\n");
				return -EINVAL;
			}
		}
	}
	
	// enable master channel default, 0 also indicate ch1
	if ((info->channel == 1) || (info->channel == 0)) {
		// the master channel support up-scale ratio up to is 3
		if (info->out_width > info->cut_width * 3 || info->out_height > info->cut_height * 3) {
			printk(KERN_ERR "isp: ch1 up-scale ratio too big\n");
			return -EINVAL;
		}

		// the master channel support down-scale up to is 1/2
		if ((info->out_width < (info->cut_width >> 1))
			|| (info->out_height < (info->cut_height >> 1))) {
			printk(KERN_ERR "isp: ch1 down-scale ratio too big\n");
			return -EINVAL;
		}

		if (isp_set_channel1_scale(isp, info->out_width, info->out_height) < 0)
			return -EINVAL;
		
		
	} else if ((info->channel == 2) && (isp->chl2_enable)) {
		// the second channel support down-scale up to is 1/8
		if ((info->out_width < (info->cut_width >> 3))
			|| (info->out_height < (info->cut_height >> 3))) {
			printk(KERN_ERR "isp: ch2 down-scale ratio too big\n");
			return -EINVAL;
		}

		chl2.width = info->out_width;
		chl2.height = info->out_height;
		chl2.enable = 1;
		if (isp_set_channel2(isp, &chl2) < 0)
			return -EINVAL;
	}
	
	if (isp_set_cutter_window(isp, info->cut_xpos, info->cut_ypos, info->cut_width, info->cut_height) < 0)
		return -EINVAL;
	return 0;
}

int isp_set_crop(struct isp_struct *isp, struct v4l2_rect rect)
{
	if (rect.width > isp->fmt_width || rect.height > isp->fmt_height) {
		printk(KERN_ERR"%s: invalid cutter window size\n", __func__);
		return -EINVAL;
	}

	isp->isp_ctrltbl[0] &= (3 << DSMP_RTO_BIT);
	if (1/*YUV_DATA*/) {	//align four byte when YUV data input
		rect.left = ALIGN(rect.left, 4);
	}
	isp->isp_ctrltbl[0] |= (rect.top << ISP_WIN_HEIGHT_OFFSET) | rect.left;
	isp->isp_ctrltbl[1] = (rect.height << ISP_WIN_HEIGHT_OFFSET) | rect.width;

	//update_image_size(isp);

	return isp_set_channel1_scale(isp, rect.width, rect.height);
}

/* *******this interface response to set image param ****** */

/**
 * @brief: black balance for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_black_balance
 */
int isp_set_black_balance(struct isp_struct *isp, struct isp_black_balance *ctrl)
{
	int i;
	
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);

	for (i = 0; i < 3; i++)
		isp->isp_ctrltbl[13+i] &= ~(0x3ff << BB_PARM_BIT);

	if (ctrl->enable == 0) {
		isp->blkb_en = 0;
	} else {
		isp->isp_ctrltbl[13] |= (ctrl->r_offset << BB_PARM_BIT);
		isp->isp_ctrltbl[14] |= (ctrl->g_offset << BB_PARM_BIT);
		isp->isp_ctrltbl[15] |= (ctrl->b_offset << BB_PARM_BIT);

		isp->blkb_en = 1;
	}

	//isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: lens correct for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_lens_correct
 */
int isp_set_lens_correct(struct isp_struct *isp, struct isp_lens_correct *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);
	
	__isp_clear_lens_param(isp);

	if (ctrl->enable == 0) {
		isp->lens_crr_en = 0;
	} else {
		__isp_set_lens_correct(isp, ctrl);
		isp->isp_ctrltbl[24] |= (1 << 31);
		isp->lens_crr_en = 1;
	}
	
	//isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: set demosaic for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_demosaic
 */
int isp_set_demosaic(struct isp_struct *isp, struct isp_demosaic *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	BUG_ON(ctrl == NULL);

	if ((ctrl->threshold < 0)||(ctrl->threshold > 4095)) {
		printk(KERN_ERR "invalid parameter. error\n");
		return -EINVAL;
	}
	
	isp->isp_ctrltbl[2] = 0x0;	

	if (isp->cur_mode_class != ISP_RGB_CLASS) {
		isp->demo_sac_en = 0;
		goto out;
	}

	// always enable demo saic for rgb
	if (ctrl->enable == 0) {
		isp->isp_ctrltbl[2] = (1 << 12)|(isp->demo_sac_thres & 0xfff);
	} else {
		isp->isp_ctrltbl[2] = (1 << 12)|(ctrl->threshold & 0xfff);
		isp->demo_sac_thres = ctrl->threshold;	
	}
	isp->demo_sac_en = 1;
	
out:
	update_image_size(isp);
	isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: noise remove for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_rgb_filter
 */
int isp_set_rgb_filter(struct isp_struct *isp, struct isp_rgb_filter *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	BUG_ON(ctrl == NULL);

	if ((ctrl->threshold < 0)||(ctrl->threshold > 1024)) {
		printk("invalid parameter. error\n");
		return -EINVAL;
	}

	isp->img_ctrltbl1[1] &= ~(0x3ff << 12);

	//disable rgb filter
	if (isp->cur_mode_class != ISP_RGB_CLASS) {
		isp->img_ctrltbl1[1] &= ~(1 << 31);
		isp->rgb_filter_en = 0;
		isp->rgb_filter_en_flag = 0;
		goto out;
	}
	
	if (ctrl->enable == 0) {
		if (isp->defect_pixel_en == 0) {
			isp->img_ctrltbl1[1] &= ~(1 << 31);
			isp->rgb_filter_en = 0;
		}
		isp->rgb_filter_en_flag = 0;
	} else { //enable rgb filter and set threshold
		isp->img_ctrltbl1[1] |= (1 << 31)|((ctrl->threshold & 0x3ff) << 12);
		isp->rgb_filter_thres = ctrl->threshold;
		isp->rgb_filter_en = 1;
		isp->rgb_filter_en_flag = 1;
	}
out:
	update_image_size(isp);
	isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: uv iso filter for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_uv_filter
 */
int isp_set_uv_iso_filter(struct isp_struct *isp, struct isp_uv_filter *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);

	if (ctrl->enable == 0) {
		//if (isp->yedge_en == 0) {
			isp->img_ctrltbl1[13] &= ~(1 << 30);
			isp->uv_isoflt_en = 0;
		//}
		//isp->uv_isoflt_en_flag = 0;
	} else {
		isp->img_ctrltbl1[13] |= (1 << 30);
		isp->uv_isoflt_en = 1;
		//isp->uv_isoflt_en_flag = 1;
	}

	//isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: bad pixel defect and fixed for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_defect_pixel
 */
int isp_set_defect_pixel(struct isp_struct *isp, struct isp_defect_pixel *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);
	
	isp->img_ctrltbl1[1] &= ~(0x3 << 28);

	if (ctrl->enable == 0) {
		//disable defect pixel remove
		isp->img_ctrltbl1[1] &= ~(1 << 30);
		isp->defect_pixel_en = 0;
		
		//disable rgb filter
		if (isp->rgb_filter_en_flag == 0) {
			isp->img_ctrltbl1[1] &= ~(1 << 31);
			isp->rgb_filter_en = 0;
		}
	} else {
		//enable defect pixel remove and set dfp_cf_sel
		isp->img_ctrltbl1[1] |= (0x3 << 30)|((ctrl->threshold & 0x3) << 28);
		isp->defect_pixel_en = 1;
		isp->rgb_filter_en = 1;
	}
	
	update_image_size(isp);
	isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: set manu wb for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_white_balance
 */
int isp_set_manu_wb(struct isp_struct *isp, struct isp_white_balance *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);

	if (ctrl->enable == 0) {
		// disable color correction
		ctrl->co_r = 1024;
		ctrl->co_g = 1024;
		ctrl->co_b = 1024;

		isp->wb_param_en = 0;
	} else {
		//enable manu while balance, disable auto while balance
		isp->wb_param_en = 1;
		isp->auto_wb_param_en = 0;
	}

	__isp_set_white_pram(isp, ctrl->co_g, ctrl->co_r, ctrl->co_b);
	
	//isp_update_regtable(isp, 0);

	//save the value
	isp->wb_param.co_r = ctrl->co_r;
	isp->wb_param.co_g = ctrl->co_g;
	isp->wb_param.co_b = ctrl->co_b;

	return 0;
}
/**
 * @brief: set auto wb for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_auto_white_balance
 */
int isp_set_auto_wb(struct isp_struct *isp, struct isp_auto_white_balance *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
		
	BUG_ON(ctrl == NULL);
	
	// no use auto while balance
	if (ctrl->enable == 0) {
		isp->wb_param.co_r = 1024;
		isp->wb_param.co_g = 1024;
		isp->wb_param.co_b = 1024;
		
		__isp_set_white_pram(isp, isp->wb_param.co_g, 
				isp->wb_param.co_r, isp->wb_param.co_b);

		// disable white balance calculate
		isp->isp_ctrltbl[25] &= ~(1 << 31);
		isp->auto_wb_param_en = 0;		
	} else {
		// enable white balance calculate
		//isp_set_auto_wb_support(isp, ctrl);
		struct awb_colortemp_set *colortemp;

		isp->awb_param.auto_wb_step = ctrl->awb_step;

	

		if(ctrl->index < 0 || ctrl->index >ISP_COLORTEMP_MODE_COUNT) {
			return -EINVAL;
		}
		colortemp = &isp->awb_param.colortemp_set[ctrl->index];

		colortemp->gr_low = ctrl->gr_low;
		colortemp->gr_high = ctrl->gr_high;
		colortemp->gb_high = ctrl->gb_high;
		colortemp->gb_low = ctrl->gb_low;
		colortemp->grb_high = ctrl->grb_high;
		colortemp->grb_low = ctrl->grb_low;

		colortemp->r_low =  ctrl->r_low;	
		colortemp->r_high = ctrl->r_high;
		colortemp->g_low =  ctrl->g_low;	
		colortemp->g_high = ctrl->g_high;
		colortemp->b_low =  ctrl->b_low;	
		colortemp->b_high = ctrl->b_high;

		colortemp->channel_r_total = ctrl->co_r;
		colortemp->channel_b_total = ctrl->co_g;
		colortemp->channel_g_total = ctrl->co_b;

		isp->wb_param_en = 0;
		isp->auto_wb_param_en = 1;
	}
	
	//isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: color correct for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_color_correct
 */
int isp_set_color_correct(struct isp_struct *isp, struct isp_color_correct *ctrl)
{
	int i;
	
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);

	isp->img_ctrltbl1[11] &= ~(0x3fffff);
	for (i = 0; i < 9; i++)
		;
		//isp->img_ctrltbl1[2+i] = 0x0;
	
	if (ctrl->enable == 0) {
		// disable color correction
		isp->img_ctrltbl1[11] &= ~(1 << 31);
		isp->color_crr_en = 0;	
	} else {
		__isp_set_color_crr(isp, ctrl, 3, 3);
		isp->color_crr_en = 1;		
	}
	
	//isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: set gamma for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_gamma_calculate
 */
int isp_set_gamma_calc(struct isp_struct *isp, struct isp_gamma_calculate *ctrl)
{
	int i;
	
	isp_dbg("%s enter.\n", __func__);
	
	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);
	
	if (ctrl->enable == 0) {
		memset(isp->img_lumitbl, 0, sizeof(unsigned long)*BRIGHTNESS_CHART_SIZE);
		// disable brightness chart table
		isp->img_ctrltbl1[11] &= ~(1 << 30);
		isp->gamma_calc_en = 0;
		
		//isp_update_regtable(isp, 0);
	} else {
		if (ctrl->is_sync == 0) {
			for(i = 0; i < 32; i++)				
				isp->img_lumitbl[i] = ctrl->gamma[i];
		}

		if (ctrl->is_sync == 1) {
			for(i = 0; i < 32; i++)
				isp->img_lumitbl[32+i] = ctrl->gamma[i];
			
			// enable brightness adjust
			isp->img_ctrltbl1[11] |= (1 << 30);
			isp->gamma_calc_en = 1;
			
			//isp_update_regtable(isp, 0);
		}
	}
	
	return 0;
}

/**
 * @brief: brightness enhance for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_brightness_enhance
 */
int isp_set_brightness_enhance(struct isp_struct *isp, struct isp_brightness_enhance *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);

	if (!__enable_brightness_enhance(isp)) 
		goto out;
	
	if (ctrl->enable == 0) {
		// disable edge(brightness) adjust
		isp->img_ctrltbl1[11] &= ~(1 << 29);
		isp->yedge_en = 0;
	#if 0	
		// disable isofilter 
		if (isp->uv_isoflt_en_flag == 0) {
			isp->img_ctrltbl1[13] &= ~(1 << 30);
			isp->uv_isoflt_en = 0;
		}
	#endif
	} else {
		__isp_set_brigtness(isp, ctrl);
		__isp_set_sharpness(isp, ctrl);

		isp->yedge_en = 1;
		//isp->uv_isoflt_en = 1;
	}
out:	
	update_image_size(isp);
	isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: uv saturation for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_saturation
 */
int isp_set_uv_saturation(struct isp_struct *isp, struct isp_saturation *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);

	isp->img_ctrltbl1[13] &= ~0x3fffffff;
	isp->img_ctrltbl1[14] &= ~(0xffff << 16);
	
	if (ctrl->enable == 0) {
		// disable saturation adjust
		isp->img_ctrltbl1[13] &= ~(1 << 31);
		isp->uv_saturate_en = 0;
	} else {
		__isp_set_uv_saturation(isp, ctrl);
		
		isp->uv_saturate_en = 1;
	}
	
	//isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: set histogram for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_histogram
 */
int isp_set_histogram(struct isp_struct *isp, struct isp_histogram *ctrl)
{
	unsigned long regval;
	
	isp_dbg("%s enter.\n", __func__);
	
	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
#if 1

	do {
		regval = REG32(isp->base + ISP_HISTO_CFG);
	} while(regval >> 31);
	
	REG32(isp->base + ISP_HISTO_CFG) = (1 << 31)
					|(isp->histo_phyaddr & 0x3fffffff);
	isp->histo_en = 1;
#else

	if (ctrl->enable == 0) {
		REG32(isp->base + ISP_HISTO_CFG) = 0;
		isp->histo_en = 0;
	} else {

		do {
			regval = REG32(isp->base + ISP_HISTO_CFG);
		} while(regval >> 31);
		
		REG32(isp->base + ISP_HISTO_CFG) = (1 << 31)
						|(isp->histo_phyaddr & 0x3fffffff);
		isp->histo_en = 1;
	}
#endif

	return 0;
}

/**
 * @brief: color effect for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_special_effect
 */
int isp_set_special_effect(struct isp_struct *isp, struct isp_special_effect *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);
	
	isp->img_ctrltbl1[0] &= ~(0xff << 24);
	isp->img_ctrltbl1[14] &= ~(0xffff);
	isp->img_ctrltbl1[15] = 0x0;

	if (ctrl->enable == 0) {
		//disable yuv and yuv_solar
		isp->img_ctrltbl1[11] &= ~(0x3 << 27);

		isp->yuv_effect_en = 0;
		isp->yuv_solar_en = 0;
	} else {
		__set_special_effect(isp, ctrl);	

		//enable yuv and yuv_solar
		isp->img_ctrltbl1[11] |= (0x1 << 28);
		isp->yuv_effect_en = 1;
		
		if (ctrl->solar_enable) {
			isp->img_ctrltbl1[11] |= (0x1 << 27);
			isp->yuv_solar_en = 1;
		} else {
			isp->img_ctrltbl1[11] &= ~(0x1 << 27);
			isp->yuv_solar_en = 0;
		}
		
	}

	return 0;
}

/**
 * @brief:  remove fog for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_image_fog
 */
int isp_remove_fog(struct isp_struct *isp, struct isp_image_fog *ctrl)
{
	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);
	
	if (ctrl->enable == 0) 
		isp->fog_en = 0;
	else 
		isp->fog_en = 1;

	return 0;
}


/* ~~~~~~~~~~~~~~~~ ISP control ~~~~~~~~~~~~~~~ */

/**
 * @brief: set brightness for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct v4l2_ctrl - The control structure.
 */
int isp_set_brightness(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{
	struct isp_brightness_enhance bl_edge;

	isp_dbg("%s. enter. ctrl->val=%d\n", __func__, ctrl->val);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 1;
	
	if (!__enable_brightness_enhance(isp)) 
		goto out;
	
	memset(&bl_edge, 0, sizeof(struct isp_brightness_yedge));
	
#if 0
	// ctrl->val is 0 indicate edge enhance closed
	if (ctrl->val == ISP_BRIGHTNESS_0) {
		// disable edge(brightness) adjust
		isp->img_ctrltbl1[11] &= ~(1 << 29);
		isp->yedge_en = 0;
		
		// disable isofilter 
		if (isp->uv_isoflt_en_flag == 0) {
			isp->img_ctrltbl1[13] &= ~(1 << 30);
			isp->uv_isoflt_en = 0;
		}
	} else {

#endif
		bl_edge.y_edgek = 300;
		bl_edge.y_thrs = 2;

		switch(ctrl->val) {
		case ISP_BRIGHTNESS_0:
			bl_edge.ygain = -20;
			break;
		case ISP_BRIGHTNESS_1:
			bl_edge.ygain = -15;
			break;
		case ISP_BRIGHTNESS_2:
			bl_edge.ygain = -10;
			break;
		case ISP_BRIGHTNESS_3:
			bl_edge.ygain = 0;
			break;
		case ISP_BRIGHTNESS_4:
			bl_edge.ygain = 10;
			break;
		case ISP_BRIGHTNESS_5:
			bl_edge.ygain = 20;
			break;
		case ISP_BRIGHTNESS_6:
			bl_edge.ygain = 35;
			break;
		}

		__isp_set_brigtness(isp, &bl_edge);
		
		isp->yedge_en = 1;
		//isp->uv_isoflt_en = 1;
	//}
out:	
	update_image_size(isp);
	isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: set gamma for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct v4l2_ctrl - The control structure.
 */
int isp_set_gamma(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{
	isp_dbg("%s. enter. ctrl->val=%d\n", __func__, ctrl->val);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 1;
	
	#if 0
	if (ctrl->val == ISP_GAMMA_0) {
		memset(isp->img_lumitbl, 0, sizeof(unsigned long)*BRIGHTNESS_CHART_SIZE);
		// disable brightness chart table
		isp->img_ctrltbl1[11] &= ~(1 << 30);
		isp->gamma_calc_en = 0;
	} else {
	#endif
		switch(ctrl->val) {
		case ISP_GAMMA_0:
			__isp_set_gamma(isp, gamma_table[0]);
			break;
		case ISP_GAMMA_1:
			__isp_set_gamma(isp, gamma_table[1]);
			break;
		case ISP_GAMMA_2:
			__isp_set_gamma(isp, gamma_table[2]);
			break;
		case ISP_GAMMA_3:
			__isp_set_gamma(isp, gamma_table[3]);
			break;
		case ISP_GAMMA_4:
			__isp_set_gamma(isp, gamma_table[4]);
			break;
		case ISP_GAMMA_5:
			__isp_set_gamma(isp, gamma_table[5]);
			break;
		case ISP_GAMMA_6:
			__isp_set_gamma(isp, gamma_table[6]);
			break;
		
		}
	
		// enable brightness adjust
		isp->img_ctrltbl1[11] |= (1 << 30);
		isp->gamma_calc_en = 1;
	//}
	
	//isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: set saturation for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct v4l2_ctrl - The control structure.
 */
int isp_set_saturation(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{
	struct isp_saturation satu;

	isp_dbg("%s. enter. ctrl->val=%d\n", __func__, ctrl->val);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
			return 1;
	
	memset(&satu, 0, sizeof(struct isp_saturation));
	isp->img_ctrltbl1[13] &= ~0x3fffffff;
	isp->img_ctrltbl1[14] &= ~(0xffff << 16);
	
	if (ctrl->val == ISP_SATURATION_0) {
		// disable saturation adjust
		isp->img_ctrltbl1[13] &= ~(1 << 31);
		isp->uv_saturate_en = 0;
	} else {
		switch(ctrl->val) {
		case ISP_SATURATION_1:
			satu.Chigh = 5;
			satu.Clow = 0;
			//tmp->Khigh should be 300-800
			satu.Khigh = 300;
			satu.Klow = 249;//280;
			break;
		case ISP_SATURATION_2:
			satu.Chigh = 5;
			satu.Clow = 0;
			satu.Khigh = 350;
			satu.Klow = 249;
			break;
		case ISP_SATURATION_3:
			satu.Chigh = 5;
			satu.Clow = 0;
			satu.Khigh = 354;
			satu.Klow = 254;
			break;
		case ISP_SATURATION_4:
			satu.Chigh = 5;
			satu.Clow = 0;
			satu.Khigh = 450;
			satu.Klow = 249;
			break;
		case ISP_SATURATION_5:
			satu.Chigh = 5;
			satu.Clow = 0;
			satu.Khigh = 500;
			satu.Klow = 249;
			break;
		case ISP_SATURATION_6:
			satu.Chigh = 5;
			satu.Clow = 0;
			satu.Khigh = 550;
			satu.Klow = 249;
			break;
		}

		__isp_set_uv_saturation(isp, &satu);
		
		isp->uv_saturate_en = 1;
	}
	
	//isp_update_regtable(isp, 0);
	return 0;
}

/**
 * @brief: set sharpness for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct v4l2_ctrl - The control structure.
 */
int isp_set_sharpness(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{
	struct isp_brightness_enhance bl_edge;

	isp_dbg("%s. enter. ctrl->val=%d\n", __func__, ctrl->val);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 1;
	
	memset(&bl_edge, 0, sizeof(struct isp_brightness_yedge));
	
	switch(ctrl->val) {
	case ISP_SHARPNESS_0:
		bl_edge.y_edgek = 200;
		bl_edge.y_thrs = 2;
		break;
	case ISP_SHARPNESS_1:
		bl_edge.y_edgek = 300;
		bl_edge.y_thrs = 2;
		break;
	case ISP_SHARPNESS_2:
		bl_edge.y_edgek = 400;
		bl_edge.y_thrs = 2;
		break;
	case ISP_SHARPNESS_3:
		bl_edge.y_edgek = 500;
		bl_edge.y_thrs = 2;
		break;
	case ISP_SHARPNESS_4:
		bl_edge.y_edgek = 600;
		bl_edge.y_thrs = 2;
		break;
	case ISP_SHARPNESS_5:
		bl_edge.y_edgek = 700;
		bl_edge.y_thrs = 2;
		break;
	case ISP_SHARPNESS_6:
		bl_edge.y_edgek = 800;
		bl_edge.y_thrs = 2;
		break;
	}
	
	__isp_set_sharpness(isp, &bl_edge);
	return 0;
}

/**
 * @brief: set color effects for ISP controller.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct v4l2_ctrl - The control structure.
 */
int isp_set_uspecial_effect(struct isp_struct *isp, struct v4l2_ctrl *ctrl, int isp_wb)
{
	struct isp_special_effect effect;
	
//	printk("%s enter. ctrl->val=%d\n", __func__, ctrl->val);
	if(0 == isp_wb)
		{
		
	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 1;
		}
	
	BUG_ON(ctrl == NULL);

	memset(&effect, 0, sizeof(struct isp_special_effect));
	isp->img_ctrltbl1[0] &= ~(0xff << 24);
	isp->img_ctrltbl1[14] &= ~(0xffff);
	isp->img_ctrltbl1[15] = 0x0;

	switch(ctrl->val) {
	case V4L2_COLORFX_NONE:
		effect.solar_thrs = 0;
		effect.y_eff_coefa = 64;
		effect.y_eff_coefb = 0;
		effect.u_eff_coefa = 64;
		effect.u_eff_coefb = 0;
		effect.v_eff_coefa = 64;
		effect.v_eff_coefb = 0;
		break;
	case V4L2_COLORFX_BW:
		effect.solar_thrs = 0;
		effect.y_eff_coefa = 64;
		effect.y_eff_coefb = 0;
		effect.u_eff_coefa = 0;
		effect.u_eff_coefb = 128;
		effect.v_eff_coefa = 0;
		effect.v_eff_coefb = 128;
		break;
	case V4L2_COLORFX_SEPIA:
		effect.solar_thrs = 0;
		effect.y_eff_coefa = 64;
		effect.y_eff_coefb = 0;
		effect.u_eff_coefa = 0;
		effect.u_eff_coefb = 64;
		effect.v_eff_coefa = 0;
		effect.v_eff_coefb = 160;
		break;
	case V4L2_COLORFX_NEGATIVE:
		effect.solar_thrs = 0;
		effect.y_eff_coefa = -64;
		effect.y_eff_coefb = 255;
		effect.u_eff_coefa = -64;
		effect.u_eff_coefb = 255;
		effect.v_eff_coefa = -64;
		effect.v_eff_coefb = 255;
		break;
	case V4L2_COLORFX_EMBOSS:
		break;
	case V4L2_COLORFX_SKETCH:
		break;
	case V4L2_COLORFX_SKY_BLUE:
		break;
	case V4L2_COLORFX_GRASS_GREEN:
		break;
	case V4L2_COLORFX_SKIN_WHITEN:
		break;
	case V4L2_COLORFX_VIVID:
		break;
	}

	__set_special_effect(isp, &effect);
	//enable yuv and yuv_solar
	isp->img_ctrltbl1[11] |= (0x1 << 28);
	isp->yuv_effect_en = 1;

	if (isp->yuv_solar_en) {
		isp->img_ctrltbl1[11] |= (0x1 << 27);
	} else {
		isp->img_ctrltbl1[11] &= ~(0x1 << 27);
	}
	
	return 0;
}
static int awb_para_update(struct isp_struct *isp,
								struct isp_auto_white_balance *auto_screen_wb_param)
{
	 	auto_screen_wb_param->r_high = isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].r_high;
		auto_screen_wb_param->r_low =  isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].r_low;	
		auto_screen_wb_param->g_high = isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].g_high;
		auto_screen_wb_param->g_low =  isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].g_low;	
		auto_screen_wb_param->b_high = isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].b_high;
		auto_screen_wb_param->b_low =  isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].b_low;	
		
		auto_screen_wb_param->grb_high = isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].grb_high;
		auto_screen_wb_param->grb_low = isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].grb_low;
		
		auto_screen_wb_param->gr_high = isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].gr_high;
		auto_screen_wb_param->gr_low = isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].gr_low;
		
		auto_screen_wb_param->gb_high = isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].gb_high;
		auto_screen_wb_param->gb_low = isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].gb_low;
	 
	 return 0;
    
}

static int estimate_color_temperture( struct isp_struct *isp,
									  unsigned long  r_total,
									  unsigned long b_total,
									  unsigned long  g_total)
{
	//
	unsigned int i;
	unsigned long max_g_total;
	unsigned int max_index=-1;
	
	if (0<isp->awb_param.frame_cnt && isp->awb_param.frame_cnt<AWB_LOOP_FRAME_NUM)
	{
	   if(r_total!=0 && b_total!= 0 && g_total!=0)
	   	{
	   		isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].channel_r_total += (r_total>>2);
	   		isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].channel_b_total += (b_total>>2);
			isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_seq].channel_g_total += (g_total>>2);
	   	}
	}

	isp->awb_param.frame_cnt++;
	//isp_dbg("the isp->awb_param.frame_cnt = %d\n",isp->awb_param.frame_cnt);
	if(isp->awb_param.frame_cnt>=AWB_LOOP_FRAME_NUM)	//statics one color tempor
	{
		isp->awb_param.frame_cnt = 0;
		isp->awb_param.colortemp_set_seq++;
		if(isp->awb_param.colortemp_set_seq>=isp->awb_param.colortemp_set_num+1)	//last color tempor
		{
			isp->awb_param.colortemp_set_seq = 0;
			
			max_index = -1;
			max_g_total = 0;
		   for(i=0;i<isp->awb_param.colortemp_set_num;i++)
		   {
		   		isp_dbg("g_total[%d]=%ld\n", i, isp->awb_param.colortemp_set[i].channel_g_total);
		   	  if(isp->awb_param.colortemp_set[i].channel_g_total>=max_g_total)
		   	  {
		   	  	max_index = i;
				max_g_total = isp->awb_param.colortemp_set[i].channel_g_total;
		   	  }
		   }

		   if(max_g_total<=G_TOTAL_THRESH)	//use default AWB statics if no color tempor avaliable
		   	{
		   		max_index = isp->awb_param.colortemp_set_num;
				max_g_total = isp->awb_param.colortemp_set[isp->awb_param.colortemp_set_num].channel_g_total;
		   	}

		   if(max_g_total >G_TOTAL_THRESH && 
	   	 		(isp->awb_param.colortemp_set[max_index].channel_r_total!=0) && 
	   	 		(isp->awb_param.colortemp_set[max_index].channel_b_total!=0))
		   	{
		   		long r_gain,g_gain, b_gain, max_rgb;
				if(isp->awb_param.colortemp_set[max_index].channel_r_total>
				   isp->awb_param.colortemp_set[max_index].channel_b_total)
				   max_rgb = isp->awb_param.colortemp_set[max_index].channel_r_total;
				else
					max_rgb = isp->awb_param.colortemp_set[max_index].channel_b_total;
				
				if(max_rgb<
				   isp->awb_param.colortemp_set[max_index].channel_g_total)
				{
				   max_rgb = isp->awb_param.colortemp_set[max_index].channel_g_total;
				}								  				
				
				r_gain = max_rgb/(isp->awb_param.colortemp_set[max_index].channel_r_total>>10);
				b_gain = max_rgb/(isp->awb_param.colortemp_set[max_index].channel_b_total>>10);
				g_gain = max_rgb/(isp->awb_param.colortemp_set[max_index].channel_g_total>>10);

				if(max_index!=isp->awb_param.current_colortemp_index)
				{
					isp->awb_param.colortemp_set[max_index].time_cnt++;
					for(i=0;i<isp->awb_param.colortemp_set_num+1;i++)
					{
					   	if(i!= max_index)
							isp->awb_param.colortemp_set[i].time_cnt = 0;
					}
					if(isp->awb_param.colortemp_set[max_index].time_cnt>CT_STABLE_CNT)
					{
						//change color temp scene
						isp->awb_param.target_r_gain = r_gain;
				   		isp->awb_param.target_b_gain = b_gain;
						isp->awb_param.target_g_gain = g_gain;
						isp->awb_param.current_colortemp_index = max_index;

						for(i=0;i<isp->awb_param.colortemp_set_num+1;i++)
					    {
					   	  isp->awb_param.colortemp_set[i].time_cnt = 0;
					    }
					}
				}
				else
				{
					isp->awb_param.target_r_gain = r_gain;
			   		isp->awb_param.target_b_gain = b_gain;
					isp->awb_param.target_g_gain = g_gain;
					isp->awb_param.current_colortemp_index = max_index;

					for(i=0;i<isp->awb_param.colortemp_set_num+1;i++)
				    {
				   	  isp->awb_param.colortemp_set[i].time_cnt = 0;
				    }
				}
		   	}
		    else
				max_index = -1;

			for(i=0;i<isp->awb_param.colortemp_set_num+1;i++)
		   {  
		   	  isp->awb_param.colortemp_set[i].channel_r_total= 0;
	   		  isp->awb_param.colortemp_set[i].channel_b_total = 0;
			  isp->awb_param.colortemp_set[i].channel_g_total = 0;
		   }
		   
		}
	}
	
	return max_index;	
}


static int isp_update_wb_param_slowness(struct isp_struct *isp)
{
        int  deltar,deltab,deltag;
		int  step = 0;

		step = isp->awb_param.auto_wb_step;

		if(step == 0)
		{
			step = 1;
		}
		
		deltar = isp->awb_param.target_r_gain - isp->awb_param.current_r_gain;
		if(deltar>0)
		{
			deltar= (deltar+step-1)/step;
		}
		if(deltar<0)
		{
		   deltar= (deltar-step+1)/step;
		}
		isp->awb_param.current_r_gain += deltar;

		deltab =isp->awb_param.target_b_gain - isp->awb_param.current_b_gain;

		if(deltab>0)
		{
		   deltab = (deltab+step-1)/step;
		}
		if(deltab<0)
		{
			deltab = (deltab-step+1)/step;
		}
		
		isp->awb_param.current_b_gain += deltab;

		deltag =isp->awb_param.target_g_gain - isp->awb_param.current_g_gain;

		if(deltag>0)
		{
		   deltag= (deltag+step-1)/step;
		}
		if(deltag<0)
		{
			deltag= (deltag-step+1)/step;
		}
		
		isp->awb_param.current_g_gain += deltag;
		
	    return 0;

}
static int _isp_awb_cc_para_update(struct isp_struct *isp,struct isp_color_correct *color_correct)
{
	short color_temperture = isp->awb_param.current_colortemp_index;
   	color_correct->enable = 1;
   	//isp_dbg("isp->awb_param.current_colortemp_index = %d\n",isp->awb_param.current_colortemp_index);
   	color_correct->cc_thrs_low = isp->awb_param.color_correct_set[color_temperture].cc_thrs_low;
   	color_correct->cc_thrs_high = isp->awb_param.color_correct_set[color_temperture].cc_thrs_high;
   	color_correct->ccMtrx[0][0] = isp->awb_param.color_correct_set[color_temperture].ccMtrx[0][0];
	color_correct->ccMtrx[0][1] = isp->awb_param.color_correct_set[color_temperture].ccMtrx[0][1];
   	color_correct->ccMtrx[0][2] = isp->awb_param.color_correct_set[color_temperture].ccMtrx[0][2];
   
   	color_correct->ccMtrx[1][0] = isp->awb_param.color_correct_set[color_temperture].ccMtrx[1][0];
   	color_correct->ccMtrx[1][1] = isp->awb_param.color_correct_set[color_temperture].ccMtrx[1][1];
   	color_correct->ccMtrx[1][2] = isp->awb_param.color_correct_set[color_temperture].ccMtrx[1][2];
   
   	color_correct->ccMtrx[2][0] = isp->awb_param.color_correct_set[color_temperture].ccMtrx[2][0];
   	color_correct->ccMtrx[2][1] = isp->awb_param.color_correct_set[color_temperture].ccMtrx[2][1];
   	color_correct->ccMtrx[2][2] = isp->awb_param.color_correct_set[color_temperture].ccMtrx[2][2];

	return 0;
}





int isp_set_cc_with_awb(struct isp_struct *isp, struct isp_color_correct_awb *ctrl)
{

	struct color_correct_set *cctemp;

	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
		
	BUG_ON(ctrl == NULL);
		

	if(ctrl->color_temperture_index > 2)
		ctrl->color_temperture_index = 2;

	isp->awb_param.current_colortemp_index = ctrl->color_temperture_index;

		

	if(ctrl->color_temperture_index < 0 || ctrl->color_temperture_index >ISP_COLORTEMP_MODE_COUNT) {
		return -EINVAL;
	}

	
	cctemp = &isp->awb_param.color_correct_set[ctrl->color_temperture_index];
	cctemp->cc_thrs_high = ctrl->cc_thrs_high;
	cctemp->cc_thrs_low = ctrl->cc_thrs_low;

	

	cctemp->ccMtrx[0][0] = ctrl->ccMtrx[0][0];
	cctemp->ccMtrx[0][1] = ctrl->ccMtrx[0][1];
	cctemp->ccMtrx[0][2] = ctrl->ccMtrx[0][2];
		
	cctemp->ccMtrx[1][0] = ctrl->ccMtrx[1][0];
	cctemp->ccMtrx[1][1] = ctrl->ccMtrx[1][1];
	cctemp->ccMtrx[1][2] = ctrl->ccMtrx[1][2];

	cctemp->ccMtrx[2][0] = ctrl->ccMtrx[2][0];
	cctemp->ccMtrx[2][1] = ctrl->ccMtrx[2][1];
	cctemp->ccMtrx[2][2] = ctrl->ccMtrx[2][2];
	
	
	return  0;
}


//xph
#if 1
	int isp_update_auto_wb_param(struct isp_struct *isp)
	{

    unsigned long co_g = 0, co_r = 0, co_b = 0;	
	int   awb_index;
	struct isp_auto_white_balance auto_wb_param;
	struct isp_color_correct      color_correct;
	//first update register
	REG32(isp->base + ISP_RGB_DATA) = 1;
	
	//then read value from register
	co_r = REG32(isp->base + ISP_RGB_DATA);
	co_g = REG32(isp->base + ISP_RGB_DATA);
	co_b = REG32(isp->base + ISP_RGB_DATA);
	//isp_dbg("[%ld %ld %ld]\n", co_r, co_g, co_b);
	//ak_gpio_setpin(0, 1);//by xc

	//if (!co_r || !co_g || !co_b)
		//printk("|");
	//return 0;

    awb_index = estimate_color_temperture( isp,
									  co_r,
									  co_b,
									  co_g);          //find the max_value g_tatol as the awb para
	if(awb_index>=0)
  	{
		isp_dbg("ct set=%d, r_gain=%d,  b_gain =%d\n", 
			awb_index, isp->awb_param.target_r_gain, isp->awb_param.target_b_gain);
  	}
   isp_update_wb_param_slowness(isp);
   #if 0
	isp->wb_param.co_r = isp->awb_param.target_r_gain;
	isp->wb_param.co_b =  isp->awb_param.target_b_gain;
	isp->wb_param.co_g = isp->awb_param.target_g_gain;
   #endif
    isp->wb_param.co_r = isp->awb_param.current_r_gain;
	isp->wb_param.co_b =  isp->awb_param.current_b_gain;
	isp->wb_param.co_g = isp->awb_param.current_g_gain;
	
    __isp_set_white_pram(isp, isp->wb_param.co_g, 
				isp->wb_param.co_r, isp->wb_param.co_b);
     
     //isp_dbg("isp->wb_param.co_r %d, isp->wb_param.co_b %d\n", 
		  //   isp->wb_param.co_r,  isp->wb_param.co_b);
	 awb_para_update(isp,&auto_wb_param);
     isp_set_auto_wb_support(isp, &auto_wb_param);

	// color correct
	 _isp_awb_cc_para_update(isp,&color_correct);
	 isp_set_color_correct(isp, &color_correct);
	
	
	isp_update_regtable(isp, 0);
	return 0;
}

#endif


//caolianming
#if  0
int isp_update_auto_wb_param(struct isp_struct *isp)
{
	unsigned long co_g = 0, co_r = 0, co_b = 0;
	unsigned short co_r_tmp, co_b_tmp;
	unsigned short nr1, nr2, nr3;
	unsigned short nb1, nb2, nb3;
	static struct awb_rgb_param rb_arr;
	static int count = 0;
	
	//first update register
	REG32(isp->base + ISP_RGB_DATA) = 1;
	
	//then read value from register
	co_r = REG32(isp->base + ISP_RGB_DATA);
	co_g = REG32(isp->base + ISP_RGB_DATA);
	co_b = REG32(isp->base + ISP_RGB_DATA);
	//isp_dbg("[%ld %ld %ld]\n", co_r, co_g, co_b);

	if ((co_r > 0) && (co_g > 0) && (co_b > 0)) {
		co_r >>= 10;
		co_b >>= 10;
		
		if ((co_r > 0) && (co_b > 0)) {
			co_r_tmp = co_g/co_r;
			co_b_tmp = co_g/co_b;
						
			rb_arr.r[count] = co_r_tmp;
			rb_arr.b[count] = co_b_tmp;
			if (count >= 2) {
				//isp_dbg("[r[0]:%d r[1]:%d r[2]:%d]\n", 
				//	rb_arr.r[0],rb_arr.r[1],rb_arr.r[2]);
				
				nr1 = ABS_DIFF(rb_arr.r[0], rb_arr.r[1]);
				nr2 = ABS_DIFF(rb_arr.r[0], rb_arr.r[2]);
				nr3 = ABS_DIFF(rb_arr.r[1], rb_arr.r[2]);

				nb1 = ABS_DIFF(rb_arr.b[0], rb_arr.b[1]);
				nb2 = ABS_DIFF(rb_arr.b[0], rb_arr.b[2]);
				nb3 = ABS_DIFF(rb_arr.b[1], rb_arr.b[2]);
								
				if (((co_r_tmp > 0) && (co_r_tmp < 2560)) &&
					((nr1+nr2)<30 || (nr1+nr3)<30 || (nr2+nr3)<30))
					isp->wb_param.co_r = co_r_tmp;
				
				if (((co_b_tmp > 0) && (co_b_tmp < 2560)) &&
					((nb1+nb2)<30 || (nb1+nb3)<30 || (nb2+nb3)<30))
					isp->wb_param.co_b = co_b_tmp;

				rb_arr.r[0] = rb_arr.r[1];
				rb_arr.r[1] = rb_arr.r[2];
				
				rb_arr.b[0] = rb_arr.b[1];
				rb_arr.b[1] = rb_arr.b[2];
			} else {
				isp->wb_param.co_r = co_r_tmp;
				isp->wb_param.co_b = co_b_tmp;
				count = count+1;
			}		
			
			isp->wb_param.co_g = 1024;
		}
	}
	
	//isp_dbg("co_r %d, co_g %d, co_b %d\n", 
	//		isp->wb_param.co_r, isp->wb_param.co_g, isp->wb_param.co_b);

	__isp_set_white_pram(isp, isp->wb_param.co_g, 
				isp->wb_param.co_r, isp->wb_param.co_b);

	isp_update_regtable(isp, 0);
	return 0;
}
#endif

// brief: return -1 is indicate the function is not support YUV data
int isp_auto_set_wb_param(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{
	struct isp_auto_white_balance auto_wb_param;
	
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS) 
		return 1;
	
	memset(&auto_wb_param, 0, sizeof(struct isp_auto_white_balance));
	memset(&isp->wb_param, 0, sizeof(struct isp_wb_param));
	
	// no use auto while balance
	if (ctrl->val == 0) {
		isp->wb_param.co_r = 1024;
		isp->wb_param.co_g = 1024;
		isp->wb_param.co_b = 1024;

		__isp_set_white_pram(isp, isp->wb_param.co_g, 
				isp->wb_param.co_r, isp->wb_param.co_b);

		// disable white balance calculate
		isp->isp_ctrltbl[25] &= ~(1 << 31);
		isp->auto_wb_param_en = 0;
	} else {
		isp_set_auto_wb_support(isp, &auto_wb_param);
		
		//disable manu while balance, enable auto while balance
		isp->wb_param_en = 0;
		isp->auto_wb_param_en = 1;
	}
	
	//isp_update_regtable(isp, 0);
	return 0;
}

// brief: return -1 is indicate the function is not support YUV data
int isp_manu_set_wb_param(struct isp_struct *isp, struct v4l2_ctrl *ctrl, int isp_wb)
{
	isp_dbg("%s enter.\n", __func__);
if(0 == isp_wb)
{
	if (isp->cur_mode_class != ISP_RGB_CLASS) 
		return 0;
}
	memset(&isp->wb_param, 0, sizeof(struct isp_wb_param));

	if (ctrl->val == ISP_MANU_WB_0) {
		// disable color correction
		isp->wb_param.co_r = 1024;
		isp->wb_param.co_g = 1024;
		isp->wb_param.co_b = 1024;

		isp->wb_param_en = 0;
	} else {
		switch(ctrl->val) {
		case ISP_MANU_WB_1:
			isp->wb_param.co_r = 1137;
			isp->wb_param.co_g = 1024;
			isp->wb_param.co_b = 1108;
			break;
		case ISP_MANU_WB_2:
			isp->wb_param.co_r = 1235;
			isp->wb_param.co_g = 1024;
			isp->wb_param.co_b = 1345;
			break;
		case ISP_MANU_WB_3:
			isp->wb_param.co_r = 1489;
			isp->wb_param.co_g = 1024;
			isp->wb_param.co_b = 1427;
			break;
		case ISP_MANU_WB_4:
		case ISP_MANU_WB_5:
		case ISP_MANU_WB_6:
			break;
		}

		//enable manu while balance, disable auto while balance
		isp->wb_param_en = 1;
		isp->auto_wb_param_en = 0;
	}

	__isp_set_white_pram(isp, isp->wb_param.co_g, 
				isp->wb_param.co_r, isp->wb_param.co_b);

	//isp_update_regtable(isp, 0);
	return 0;
}

#if 0 
void update_exposure_value(struct isp_struct *isp)
{
	unsigned int value;
	static int flags_max = 0, flags_min = 0;
	
	value = brightness_average(isp);

	if (!strcmp(get_sensor_name(), "ov9712")) {
		if ((value > 0xB4) && (flags_max < 2)) {
			//isp_dbg(" >0xB4 -> 0x48\n");
			flags_max++;
			flags_min = 0;
			aksensor_set_param(0x14, 0x48);
		} else if ((value < 0x60) && (flags_min < 2)) {
			//isp_dbg(" <0x60 -> 0x40\n");
			flags_max = 0;
			flags_min++;
			aksensor_set_param(0x14, 0x40);
		} 
	}
}
#endif
#if 1
int isp_recalc_gamma_from_histogram(struct isp_struct *isp)
{
	unsigned char gamma_tmp[256] = {0};
	unsigned char y_lut[256] = {0};
	unsigned char Lum_Tab[256] = {0};
	unsigned int slope;
	unsigned int sum = 0;
	unsigned int lutTmp = 0;
	int i;
	
	isp->fog_thrs = 7372;  // = 1280 * 720 * 0.008
	
	for (i = 0; i < 256; i++) {
		sum += (histo_arr[i]);
		if (sum > isp->fog_thrs) {
			break;
		}
	}
	isp->fog_thrs = i;

	for (i = 0; i <= isp->fog_thrs; i++) {
		gamma_tmp[i] = 0x0;
	}

	// float --> int
	slope = (255 << 16) / (255 - isp->fog_thrs);
	for (i = isp->fog_thrs+1; i < 256; i++) 		
		gamma_tmp[i] = (slope*(i-isp->fog_thrs)) >> 16;

	for(i = 0; i < 256; i++) {
		lutTmp = (219 * (i - 128) + (128 << 8)) >> 8;
		Lum_Tab[i] = (unsigned char)lutTmp;
	}

	for(i = 0; i < 256; i++) {
		y_lut[i] = Lum_Tab[gamma_tmp[i]];	
	}
	
	//copy to isp->img_lumitbl, write gamma table
	for(i = 0; i < 256; i+=4) {
		isp->img_lumitbl[i>>2] = y_lut[i]|(y_lut[i+1] << 8)
					|(y_lut[i+2] << 16)|(y_lut[i+3] << 24);
	}
	
	isp_update_regtable(isp, 0);
	return 0;
}
#endif



/**
 * @brief: set auto AE for ISP controller.
 * 
 * @author: yanchunxiang
 * @date: 2014-03-13
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 * @param [in] *ctrl: struct isp_ae_attr
 */

int isp_set_ae_attr (struct isp_struct *isp, struct isp_ae_attr *ctrl) 
{

	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
		
	BUG_ON(ctrl == NULL);
	
	// no use auto explore control
	if (ctrl->enable == 0)
	{
		
		isp->aec_param.bByPassAE = 1;
	} 
	else
	{
		
		isp->aec_param.exposure_time_max = ctrl->ae_timemax;
		isp->aec_param.exposure_time_min = ctrl->ae_timemin;
		isp->aec_param.stable_range = ctrl->ae_tolerance;
		isp->aec_param.target_lumiance = ctrl->ae_target;
		
		isp->aec_param.exposure_step = ctrl->ae_step;
		isp->aec_param.bLinkage = ctrl->enableExpLinkage;
		isp->aec_param.bCompensation = ctrl->enableExpCompensation;
		isp->aec_param.bGama = ctrl->enableExpGama;

		isp->aec_param.bByPassAE = 0;	
		
	}	

	return 0;

}




unsigned int aec_time_flag = 0;
unsigned int tmp_exposure_time;
 int tmp_a_gain;
unsigned int  laec_flag = 0;
unsigned int laec_time  =0xffff;

int calc_aec_updata_step(struct isp_struct *isp,int target_lumiance, int calc_lumi_average)
{
	int  lumi_deta =  0 ;
	int  target_lumi = isp->aec_param.target_lumiance;
	int high_lock = 0, low_lock = 0;
	int stepCoef = 0;
	

	high_lock = isp->aec_param.target_lumiance +
					isp->aec_param.stable_range;
	low_lock  =  isp->aec_param.target_lumiance -
					isp->aec_param.stable_range;

	stepCoef = isp->aec_param.exposure_step;


	if( low_lock>calc_lumi_average)
	{
		lumi_deta = low_lock - calc_lumi_average;
		
	}
	else if(high_lock< calc_lumi_average)
	{
	    	
	    	lumi_deta = calc_lumi_average-high_lock;
		
	}
	else
	{ 
	    lumi_deta= 0;
	}
	
	 
	
	

	 if (lumi_deta == 0)// lumi_deta = 0
	{
	       isp->aec_param.aec_step.exposure_time_updata_step = 0;	 	           
		   isp->aec_param.aec_step.a_gain_updata_step = 0;
		   isp->aec_param.aec_step.laec_time_step=
		  	isp->aec_param.current_laec_time>>1;

		   // printk("calc_aec_updata_step 0000000:lumi_deta!\n");
		   
			       // isp->aec_param.current_a_gain>>1;
	}
	else  if(lumi_deta<=(target_lumi>>4))
    {
     	isp->aec_param.aec_step.exposure_time_updata_step = 
	 	            (isp->aec_param.current_exposure_time>>5) * stepCoef;
		isp->aec_param.aec_step.a_gain_updata_step =  1;
		isp->aec_param.aec_step.laec_time_step=isp->aec_param.current_laec_time>>5;
			        //isp->aec_param.current_a_gain>>5;
	 
    
    }
				   
	else if(lumi_deta<=(target_lumi>>3))
	{
	    isp->aec_param.aec_step.exposure_time_updata_step = 
	 	            (isp->aec_param.current_exposure_time>>4) * stepCoef;
		isp->aec_param.aec_step.a_gain_updata_step = 1;
		isp->aec_param.aec_step.laec_time_step=isp->aec_param.current_laec_time>>4;
			        //isp->aec_param.current_a_gain>>4;
	}
	else if(lumi_deta<=(target_lumi>>2))
	{
	    isp->aec_param.aec_step.exposure_time_updata_step = 
	 	            (isp->aec_param.current_exposure_time>>3) * stepCoef;
		isp->aec_param.aec_step.a_gain_updata_step = 1;
		isp->aec_param.aec_step.laec_time_step=isp->aec_param.current_laec_time>>3;
			       // isp->aec_param.current_a_gain>>3;
	}
	else if (lumi_deta<=target_lumi>>1)
	{
		  isp->aec_param.aec_step.exposure_time_updata_step = 
	 	            (isp->aec_param.current_exposure_time>>2) * stepCoef;
		   isp->aec_param.aec_step.a_gain_updata_step = 1;
		   isp->aec_param.aec_step.laec_time_step=isp->aec_param.current_laec_time>>2;
			       // isp->aec_param.current_a_gain>>2;
	}
	else if (lumi_deta<=target_lumi)
	{
		  isp->aec_param.aec_step.exposure_time_updata_step = 
	 	            (isp->aec_param.current_exposure_time>>1) * stepCoef;
		   isp->aec_param.aec_step.a_gain_updata_step =  2;
		   isp->aec_param.aec_step.laec_time_step=isp->aec_param.current_laec_time>>2;
			       // isp->aec_param.current_a_gain>>2;
	}
	else if (lumi_deta>target_lumi && lumi_deta <= 255)
	{
	      isp->aec_param.aec_step.exposure_time_updata_step = 
	 	            (isp->aec_param.current_exposure_time>>1) * stepCoef; 
		  isp->aec_param.aec_step.a_gain_updata_step = 2;
		  isp->aec_param.aec_step.laec_time_step=
		  	isp->aec_param.current_laec_time>>1;
			        //isp->aec_param.current_a_gain>>1;
	}
	else 
	{
	      printk("lumi_deta:%d, targed:%d\n",lumi_deta,target_lumi);
	}
	
	if(isp->aec_param.current_exposure_time<=32)
	{
		isp->aec_param.aec_step.exposure_time_updata_step = 1;
		isp->aec_param.aec_step.a_gain_updata_step  =1;
		isp->aec_param.aec_step.laec_time_step = 2;

#if 0
		if((isp->aec_param.current_exposure_time<=6)&&
			(aec_time_flag==1))
	    {
	       isp->aec_param.stable_range = 4*isp->aec_param.stable_range;
		   aec_time_flag = 1;
		   printk("stable range increase\n");
	    }

		if((isp->aec_param.current_exposure_time>6) 
		&&(aec_time_flag==1))
	    {
		  aec_time_flag = 0;
		  isp->aec_param.stable_range = isp->aec_param.stable_range>2;
		  printk("stable range decrease\n");
	    }
#endif
	
		
	}


	
	
	
	return  0;
	
}

unsigned int agc_smooth_num = 0;
unsigned int agc_aec_update_flag = 0;
unsigned int agc_smooth_max = 0;
int isp_luminance_aec_time_calc(struct isp_struct *isp)
{
	

  	unsigned int calc_lumi_average = isp->aec_param.hist_feature.mean_brightness;
	int target_lumiance = isp->aec_param.target_lumiance;
	unsigned int high_lock = 0;
 	unsigned int low_lock =  0; 
	unsigned int high_hold = 0;	  
	unsigned int low_hold  = 0; 
	int current_exposure_time = 0;
	int tempvalue = 0;
	
	tempvalue = isp->aec_param.target_lumiance + isp->aec_param.stable_range;
	high_lock = (tempvalue > 256) ? 256 : tempvalue;

	tempvalue = isp->aec_param.target_lumiance - isp->aec_param.stable_range;
	low_lock = (tempvalue < 0) ? 0 : tempvalue;

	tempvalue = isp->aec_param.target_lumiance + isp->aec_param.stable_range + 6;        
	high_hold = (tempvalue > 256) ? 256 : tempvalue;

	tempvalue = isp->aec_param.target_lumiance - isp->aec_param.stable_range - 6;
	low_hold = (tempvalue < 0) ? 0 : tempvalue;

	calc_aec_updata_step(isp,target_lumiance,calc_lumi_average);
   
	if((calc_lumi_average<=high_lock)&&(calc_lumi_average>=low_lock))
	{

	 isp->aec_param.aec_locked  = 1;                              //no work to do
	 isp->aec_param.aec_status  = 0;  // 稳定

	  return 0;

	}
   

	if((isp->aec_param.aec_locked == 1) &&
		  (((calc_lumi_average<high_hold)&&(calc_lumi_average>high_lock)) ||

		  ((calc_lumi_average < low_lock)&&(calc_lumi_average>low_hold) ))	)

   
   {
	 //isp->aec_param.aec_locked  = 0;
	 isp->aec_param.aec_locked  = 1; 
	 isp->aec_param.aec_status  = 0;                              //no work to do

	  return 0;
    
   }
   else
   {
	 isp->aec_param.aec_locked  = 0; 
     isp->aec_param.aec_status  = 1;                              //不稳定 
   }
   

 if(isp->aec_param.aec_status == 1)

 {

	// 过曝
 	 if(calc_lumi_average > high_lock) 
  
    {


		if(isp->aec_param.current_exposure_time<=8)
		{
           
               if(isp->aec_param.current_a_gain > isp->aec_param.a_gain_min)
			    {
			        isp->aec_param.current_exposure_time = isp->aec_param.current_exposure_time;
						isp->aec_param.current_a_gain--;

						if(isp->aec_param.current_a_gain<isp->aec_param.a_gain_min)
						{
							printk("current_a_gain<0: %d\n",isp->aec_param.current_a_gain);
				
							isp->aec_param.current_a_gain = isp->aec_param.a_gain_min;
						}
				}
                else
				{
					

					if(isp->aec_param.current_exposure_time > 1)
					{

						agc_smooth_max = (isp->aec_param.current_exposure_time<<4)/(isp->aec_param.current_exposure_time-1);
						agc_smooth_max -= 16;

						if(agc_smooth_max > 7)
						{
							agc_smooth_max = 7;
						}

						agc_smooth_max += isp->aec_param.a_gain_min;


						isp->aec_param.current_a_gain = agc_smooth_max;				   
					    

						isp->aec_param.current_exposure_time = isp->aec_param.current_exposure_time - 1;
						
					}
					else if(isp->aec_param.current_exposure_time == 1)
					{
						isp->aec_param.current_exposure_time = 0;
						isp->aec_param.current_a_gain = isp->aec_param.a_gain_min+7; // 是否匹配
						
					}
					else if(isp->aec_param.current_exposure_time == 0 && isp->aec_param.current_a_gain == isp->aec_param.a_gain_min)
					{
						printk("time =0 && a_gain = 0; \n");
				
						return 0;
					}
					else
					{
						printk(" < 8 ERROR \n");
					}
	

					
					
				}
				return 0;

                
				
           }
		 
		



 
		/*
		if((isp->aec_param.current_d_gain <= isp->aec_param.d_gain_max)&&
			(isp->aec_param.current_d_gain >= isp->aec_param.d_gain_min))
	   	 {
	   	 	isp->aec_param.current_d_gain -=isp->aec_param.aec_step.d_gain_updata_step;
			if(isp->aec_param.current_d_gain<=0)
			{
				isp->aec_param.current_d_gain = 0;
				
			}
			return 0;
	   	}*/

		if((isp->aec_param.current_a_gain <= isp->aec_param.a_gain_max)&&
			(isp->aec_param.current_a_gain > isp->aec_param.a_gain_min))
		{
			isp->aec_param.current_a_gain -= isp->aec_param.aec_step.a_gain_updata_step;

			
			if(isp->aec_param.current_a_gain <= isp->aec_param.a_gain_min)
			{
				isp->aec_param.current_a_gain = isp->aec_param.a_gain_min;
				
			}
			return 0;
		}

		// 下面是a_gain=0时，减少曝光时间
		current_exposure_time = isp->aec_param.current_exposure_time-
			                    isp->aec_param.aec_step.exposure_time_updata_step;	 		
		if(current_exposure_time < isp->aec_param.exposure_time_min)
		{
			isp->aec_param.current_exposure_time = isp->aec_param.exposure_time_min;
		}
		else
		{
		  isp->aec_param.current_exposure_time = current_exposure_time;
		}

		
	        
	//	 printk("in the light envi,the exposure_time is %d\n",isp->aec_param.current_exposure_time);
	//	 printk("in the light envi,the a_gain is %d\n",isp->aec_param.current_a_gain);
		
 	}
  else if(calc_lumi_average<low_lock)
  {  

	if(isp->aec_param.current_exposure_time<=8)
	{
		if(isp->aec_param.current_exposure_time == 0)// 0 -> 1
		{
			agc_smooth_max = isp->aec_param.a_gain_min + 7;
		}
		else
		{
			// 计算agc_smooth_max		
			
			agc_smooth_max = ((isp->aec_param.current_exposure_time+1)<<4)/isp->aec_param.current_exposure_time;
			agc_smooth_max = agc_smooth_max - 16;
			
			if(agc_smooth_max > 7)
			{
				agc_smooth_max = 7;
			}
			
			agc_smooth_max += isp->aec_param.a_gain_min;
		}
				
		if(isp->aec_param.current_a_gain < agc_smooth_max)
		{
			isp->aec_param.current_a_gain++;

		}
		else //if(isp->aec_param.current_a_gain >= agc_smooth_max)
		{
			isp->aec_param.current_exposure_time++;
			isp->aec_param.current_a_gain = isp->aec_param.a_gain_min;
			
		}
		
        return 0;   
           
	}	   
				


     isp->aec_param.current_exposure_time += isp->aec_param.aec_step.exposure_time_updata_step; 

	if(isp->aec_param.current_exposure_time >=	isp->aec_param.exposure_time_max)
	 {
		isp->aec_param.current_exposure_time = isp->aec_param.exposure_time_max;
		isp->aec_param.current_a_gain = isp->aec_param.current_a_gain+
			                          isp->aec_param.aec_step.a_gain_updata_step;
		if(isp->aec_param.current_a_gain > isp->aec_param.a_gain_max)
		{
		    isp->aec_param.current_a_gain = isp->aec_param.a_gain_max;
			isp->aec_param.current_d_gain = isp->aec_param.current_d_gain +
				        isp->aec_param.current_d_gain;
			if (isp->aec_param.current_d_gain > isp->aec_param.d_gain_max)
			{
				isp->aec_param.current_d_gain  = isp->aec_param.d_gain_max;
			}
		}
		
	 }
	 
	// printk("int the dark envi, the exposure_time%d\n",isp->aec_param.current_exposure_time);
	// printk("in  the dark envi,the a_gain =%d\n",isp->aec_param.current_a_gain);
  }
  else
  {
       isp->aec_param.aec_status = 0;
  }
 
  
 }
 
  return 0;
	
}




static unsigned int _isp_set_para_with_aec(struct isp_struct *isp)
{

#if 0

		isp->aec_param.para_with_aec.y_edgek[0] = 750;
		isp->aec_param.para_with_aec.y_edgek[1] = 740;
		isp->aec_param.para_with_aec.y_edgek[2] = 730;
		isp->aec_param.para_with_aec.y_edgek[3] = 720;
		isp->aec_param.para_with_aec.y_edgek[4] = 710;
		isp->aec_param.para_with_aec.y_edgek[5] = 600;
		isp->aec_param.para_with_aec.y_edgek[6] = 690;
		isp->aec_param.para_with_aec.y_edgek[7] = 680;
#endif	


        // 东舜客户
        isp->aec_param.para_with_aec.y_edgek[0] = 550;
		isp->aec_param.para_with_aec.y_edgek[1] = 540;
		isp->aec_param.para_with_aec.y_edgek[2] = 530;
		isp->aec_param.para_with_aec.y_edgek[3] = 520;
		isp->aec_param.para_with_aec.y_edgek[4] = 510;
		isp->aec_param.para_with_aec.y_edgek[5] = 500;
		isp->aec_param.para_with_aec.y_edgek[6] = 490;
		isp->aec_param.para_with_aec.y_edgek[7] = 480;

		isp->aec_param.para_with_aec.y_thrs[0]  = 2 ; 
		isp->aec_param.para_with_aec.y_thrs[1]  = 2 ; 
		isp->aec_param.para_with_aec.y_thrs[2]  = 2 ; 
		isp->aec_param.para_with_aec.y_thrs[3]  = 2 ; 
		isp->aec_param.para_with_aec.y_thrs[4]  = 2 ; 
		isp->aec_param.para_with_aec.y_thrs[5]  = 2 ;
		isp->aec_param.para_with_aec.y_thrs[6]  = 2 ; 
		isp->aec_param.para_with_aec.y_thrs[7]  = 2 ;

		isp->aec_param.para_with_aec.rgb_filter_thres[0]  = 20 ; 
		isp->aec_param.para_with_aec.rgb_filter_thres[1]  = 30 ; 
		isp->aec_param.para_with_aec.rgb_filter_thres[2]  = 40 ; 
		isp->aec_param.para_with_aec.rgb_filter_thres[3]  = 50 ; 
		isp->aec_param.para_with_aec.rgb_filter_thres[4]  = 60 ; 
		isp->aec_param.para_with_aec.rgb_filter_thres[5]  = 70 ;
		isp->aec_param.para_with_aec.rgb_filter_thres[6]  = 80 ; 
		isp->aec_param.para_with_aec.rgb_filter_thres[7]  = 100;

		isp->aec_param.para_with_aec.demo_sac_thres[0] = 4;
		isp->aec_param.para_with_aec.demo_sac_thres[1] = 4;
		isp->aec_param.para_with_aec.demo_sac_thres[2] = 4;
		isp->aec_param.para_with_aec.demo_sac_thres[3] = 4;
		isp->aec_param.para_with_aec.demo_sac_thres[4] =4 ;
		isp->aec_param.para_with_aec.demo_sac_thres[5] = 4;
		isp->aec_param.para_with_aec.demo_sac_thres[6] = 20;
		isp->aec_param.para_with_aec.demo_sac_thres[7] = 52;
		return 0;
		
}


static unsigned int isp_param_updata_with_ae(struct isp_struct *isp)
{
  //  unsigned int exposure_time = isp->aec_param.current_exposure_time+1;
	unsigned int gain = isp->aec_param.current_a_gain-
		                isp->aec_param.a_gain_min + 1;
	unsigned int intensity_light = gain;//exposure_time*gain;
	unsigned int intensity_light_max = //isp->aec_param.exposure_time_max*
		                              isp->aec_param.a_gain_max;
	unsigned int intensity_level_step = intensity_light_max/8;
	unsigned int intensity_level = intensity_light/intensity_level_step;
	struct isp_brightness_enhance bl_edge;

	printk("intensity is %d\n",intensity_level);


	_isp_set_para_with_aec(isp);

	bl_edge.y_edgek = isp->aec_param.para_with_aec.y_edgek[intensity_level];      //edge
    bl_edge.y_thrs  = isp->aec_param.para_with_aec.y_thrs[intensity_level];

    __isp_set_sharpness(isp, &bl_edge);


	if (isp->cur_mode_class != ISP_RGB_CLASS) {
		isp->img_ctrltbl1[1] &= ~(1 << 31);
		isp->rgb_filter_en = 0;
		isp->rgb_filter_en_flag = 0;
		goto out;
	}

	isp->rgb_filter_thres = 
		isp->aec_param.para_with_aec.rgb_filter_thres[intensity_level];
	isp->rgb_filter_en = 1;
	
    isp->demo_sac_thres = isp->aec_param.para_with_aec.demo_sac_thres[intensity_level];
   __isp_set_sharpness(isp, &bl_edge);	
   
out:
   // printk("y_edgek = %d demo_sac_thres=%d rgb_filter_thres=%d\n",bl_edge.y_edgek,
	//	 isp->demo_sac_thres,isp->rgb_filter_thres);
	update_image_size(isp);
	isp_update_regtable(isp, 0);
	return 0;
   		
}





#define LOW_VALUE 16
#define HIGH_VALUE 240


int isp_compensation_with_ae(struct isp_struct *isp)
{
	int i=0;
 	int histo_sum[16], pixel_sum[16];
	int new_avg = 0,sum_num=0,value = 0;
	int weight_ceff[8] = {1,2,3,4,5,6,7,8};
	int weight = 0;
	 
	memset(histo_sum,0,sizeof(histo_sum));
	memset(pixel_sum,0,sizeof(pixel_sum));
	 
	for(i=0; i<LOW_VALUE;i++)
	{
		histo_sum[0] += i *(histo_arr[i]);
		pixel_sum[0] += (histo_arr[i]);
	}
	 
	
	 
	// 数据较少时，亮度增加
	if(pixel_sum[0] < LOW_VALUE*3600/4 )// 14400
	{
		// histo_sum[0] = histo_sum[0] *3/4;
		// pixel_sum[0] = pixel_sum[0] * 4/3;//3/4;
	}
	 
	 
	for(i=LOW_VALUE; i<HIGH_VALUE;i++)
	{
		histo_sum[1] += i *(histo_arr[i]);
		pixel_sum[1] += (histo_arr[i]);
	}
	 
	//printk(">16&&<240:%d\n",pixel_sum[1]);	 
	
	
	for(i=HIGH_VALUE; i<256;i++)
	{
		histo_sum[2] += i *(histo_arr[i]);
		pixel_sum[2] += (histo_arr[i]);
	}
	//printk(">240:%d\n",pixel_sum[2]);
	
	 weight=pixel_sum[2]/((256-HIGH_VALUE)*3600)-1;
	 if(weight>7)
	 	weight = 7;
	 else if(weight<0)
	 	weight = 0;
	 else
	 	weight = weight_ceff[weight];

	 histo_sum[2] = (histo_sum[2]/16)*(weight+16);
	 
		 
	value = histo_sum[0] + histo_sum[1] + histo_sum[2];
	sum_num = pixel_sum[0] + pixel_sum[1] + pixel_sum[2];

	if (sum_num == 0)
		sum_num = 1;
	
	new_avg = value/sum_num;
 
	//printk("new value:%d, sum_num:%d, average:%d\n",value, sum_num, new_avg);
	return new_avg;
}



T_U32 g_uLum_Tab[4][BRIGHTNESS_CHART_SIZE] = {
	// default
		{
	0x4020101, 0xc0a0806, 0x1412100e, 0x1c1a1816, 0x2422201e, 0x2d2b2926, 0x3533312f, 0x3d3b3937, 
	0x4543413f, 0x4e4b4947, 0x5251504f, 0x57555453, 0x5b5a5958, 0x605e5d5c, 0x64636261, 0x69686665, 
	0x6d6c6b6a, 0x7271706e, 0x76757473, 0x7b7a7978, 0x807e7d7c, 0x84838281, 0x89878685, 0x8d8c8b8a, 
	0x91908f8e, 0x96959493, 0x9a999897, 0x9e9e9c9b, 0xa1a1a09f, 0xa4a4a3a2, 0xa7a7a6a5, 0xaaaaa9a8,		
	0xadadacab, 0xb0b0afae, 0xb3b2b2b1, 0xb6b5b5b4, 0xb9b8b7b7, 0xbbbbbab9, 0xbebdbdbc, 0xc1c0bfbf, 
	0xc3c3c2c1, 0xc6c5c5c4, 0xc8c8c7c6, 0xcbcac9c9, 0xcdcccccb, 0xcfcfcecd, 0xd1d1d0d0, 0xd3d3d2d2, 
	0xd5d5d4d4, 0xd7d7d6d6, 0xd9d9d8d8, 0xdbdadad9, 0xdddcdcdb, 0xe0e0dfde, 0xe3e2e2e1, 0xe5e5e4e4, 
	0xe7e7e6e6, 0xe9e9e8e8, 0xeaeaeae9, 0xecebebeb, 0xededecec, 0xeeeeeded, 0xefefeeee, 0xf0f0efef,

		},
	//直线
		{
	0x3020100, 0x7060504, 0xb0a0908, 0xf0e0d0c, 0x13121110, 0x17161514, 0x1b1a1918, 0x1f1e1d1c, 
	0x23222120, 0x27262524, 0x2b2a2928, 0x2f2e2d2c, 0x33323130, 0x37363534, 0x3b3a3938, 0x3f3e3d3c, 
	0x43424140, 0x47464544, 0x4b4a4948, 0x4f4e4d4c, 0x53525150, 0x57565554, 0x5b5a5958, 0x5f5e5d5c, 
	0x63626160, 0x67666564, 0x6b6a6968, 0x6f6e6d6c, 0x73727170, 0x77767574, 0x7b7a7978, 0x7f7e7d7c,
	0x83828180, 0x87868584, 0x8b8a8988, 0x8f8e8d8c, 0x93929190, 0x97969594, 0x9b9a9998, 0x9f9e9d9c, 
	0xa3a2a1a0, 0xa7a6a5a4, 0xabaaa9a8, 0xafaeadac, 0xb3b2b1b0, 0xb7b6b5b4, 0xbbbab9b8, 0xbfbebdbc, 
	0xc3c2c1c0, 0xc7c6c5c4, 0xcbcac9c8, 0xcfcecdcc, 0xd3d2d1d0, 0xd7d6d5d4, 0xdbdad9d8, 0xdfdedddc, 
	0xe3e2e1e0, 0xe7e6e5e4, 0xebeae9e8, 0xefeeedec, 0xf3f2f1f0, 0xf7f6f5f4, 0xfbfaf9f8, 0xfffefdfc,
		},
	//凸
		{
	0x6040200, 0xf0d0b08, 0x18161412, 0x211f1d1b, 0x2b282624, 0x34322f2d, 0x3e3b3936, 0x47454240, 
	0x514f4c4a, 0x5b595654, 0x6663605e, 0x6e6e6b68, 0x7171706f, 0x74747372, 0x77777675, 0x7b7a7978, 
	0x7e7d7c7b, 0x81807f7e, 0x84838282, 0x87868585, 0x8a898988, 0x8d8c8c8b, 0x90908f8e, 0x94939291, 
	0x97969594, 0x9a999897, 0x9d9c9b9b, 0xa09f9e9e, 0xa3a2a1a1, 0xa6a5a5a4, 0xa9a8a8a7, 0xacababaa,
	0xafaeaead, 0xb2b1b0b0, 0xb5b4b3b3, 0xb8b7b6b6, 0xbbbab9b8, 0xbdbdbcbb, 0xc0bfbfbe, 0xc3c2c1c1, 
	0xc5c5c4c3, 0xc8c7c7c6, 0xcbcac9c9, 0xcdcccccb, 0xcfcfcece, 0xd2d1d1d0, 0xd4d4d3d2, 0xd6d6d5d5, 
	0xd9d8d7d7, 0xdbdadad9, 0xdddcdcdb, 0xdfdededd, 0xe3e1e0df, 0xe8e7e6e4, 0xedecebea, 0xf1f0efee, 
	0xf4f3f3f2, 0xf7f6f6f5, 0xf9f8f8f7, 0xfbfafaf9, 0xfcfcfbfb, 0xfdfdfcfc, 0xfefefdfd, 0xfffefefe,

		},
		
	// bright
		{
	0x5030100, 0xd0b0907, 0x1412100e, 0x1c1a1816, 0x23211f1e, 0x2b292725, 0x34312f2d, 0x3c3a3836, 
	0x4543403e, 0x4e4c4947, 0x57555350, 0x615f5c5a, 0x68676764, 0x6a6a6968, 0x6d6c6c6b, 0x6f6f6e6d, 
	0x72717170, 0x75747373, 0x77767675, 0x7a797878, 0x7c7c7b7a, 0x7f7f7e7d, 0x82818180, 0x85848383, 
	0x87878685, 0x8a8a8988, 0x8d8c8c8b, 0x908f8e8e, 0x93929191, 0x96959493, 0x98989796, 0x9b9b9a99,
	0x9e9e9d9c, 0xa1a1a09f, 0xa4a3a3a2, 0xa7a6a6a5, 0xaaa9a9a8, 0xadacacab, 0xb0afafae, 0xb3b2b2b1, 
	0xb6b5b5b4, 0xb9b9b8b7, 0xbcbcbbba, 0xbfbfbebd, 0xc3c2c1c0, 0xc6c5c4c3, 0xc9c8c7c6, 0xcccbcaca, 
	0xcfcecdcd, 0xd2d1d1d0, 0xd5d4d4d3, 0xd8d8d7d6, 0xdcdbdad9, 0xdfdedddc, 0xe2e1e0df, 0xe5e4e3e3, 
	0xe8e7e7e6, 0xebebeae9, 0xefeeedec, 0xf2f1f0ef, 0xf5f4f3f3, 0xf8f7f7f6, 0xfbfbfaf9, 0xfffefdfc,
		},

};

	
int isp_gamma_with_ae(struct isp_struct * isp)
{
	int i = 0;
	T_U32 step = 0;	
	T_U32 gamma_temp[BRIGHTNESS_CHART_SIZE];
	T_U8 *gammaT = AK_NULL, *gamma = AK_NULL;

	int pixel_sum = 0;
	int value_avg = 0;
		 
	for(i=0; i<LOW_VALUE;i++)
	{
		
		pixel_sum += (histo_arr[i]);
	} 

	value_avg = LOW_VALUE*3600/2;
	
	gammaT = (T_U8 *)g_uLum_Tab[0];
	gamma = (T_U8*)gamma_temp;	

	if(value_avg == 0)
	{
		printk("isp_gamma_with_ae() ERROR!\n");
		return 0;
	}


	for(i=0; i<64; i++)
	{
				
		step = pixel_sum / value_avg; 

		if(step > 10) step = 10;
		
		gamma[i] = gammaT[i] + step ;		
		
	}

	// printk("gama[10]:old:%d, new:%d\n", gammaT[10],gamma[10]);

	for(i=64; i<256; i++)
	{
		gamma[i] = gammaT[i];
	}		
	
	__isp_set_gamma(isp, gamma_temp);
	
	isp->img_ctrltbl1[11] |= (1 << 30);
	isp->gamma_calc_en = 1;	
	return 0;
}






unsigned int pre_brightness_average = 0;

static unsigned int brightness_average(struct isp_struct *isp)
{
	unsigned int sum = 0, tmp = 0;
	unsigned int average;
	int i;

    
	for (i = 0; i < 256; i++) {
		sum += i *(histo_arr[i]);
		tmp += (histo_arr[i]);
	}
	
	if (tmp == 0)
		tmp = 1;
	if(tmp>1000)
	{
	    average = sum/tmp;
		
	
	}
	else
	{
	    average = pre_brightness_average;
	}

	if(isp->aec_param.bCompensation)
	{
		 average = isp_compensation_with_ae(isp);
	}

	if(isp->aec_param.bGama)
	{
		isp_gamma_with_ae(isp);	
	
	}
	
	
	
	pre_brightness_average = average;
	//printk("the sum =%d\n tmp = %d\n",sum,tmp);

    if(average<255)
    {
	    isp->aec_param.hist_feature.mean_brightness = average;
    }
	else
	{
		isp->aec_param.hist_feature.mean_brightness = 255;
	}
    //printk("average is %d\n",isp->aec_param.hist_feature.mean_brightness);
	return 0;
}



#define OV_MAX(a, b)            (((a) < (b) ) ?  (b) : (a))
#define OV_MIN(a, b)            (((a) > (b) ) ?  (b) : (a))
#define OV_CLIP3(low, high, x)  (OV_MAX(OV_MIN((x), high), low))
/*
 * This function applies the new gains to the ISP registers.
 */
static __inline unsigned int cmos_gains_update(struct isp_struct *isp,unsigned int p_gains)
{

    unsigned int ag_interget_table[17] = {0,  1, 2, 2, 4, 4, 4, 4, 8, 8, 8, 8,
        8, 8, 8, 8, 16};
    unsigned int ag_integer, ag_fraction, tmp;
	unsigned int ag_return;
 
    tmp =  p_gains / 16;
	if(tmp == 0)
	{
		tmp = 1;
		printk("Div ERROR 000!\n");
	}
	
    tmp =  OV_CLIP3(0, 16, tmp);
    ag_integer =  ag_interget_table[tmp];
 
    ag_fraction = (p_gains / ag_integer) - 16;
    ag_fraction =  OV_CLIP3(0, 15, ag_fraction);
 
    if (((ag_fraction + 16) * ag_integer) < p_gains)
    {
        if (ag_fraction < 15)
        {
            ag_fraction++;
        }
        else if (ag_integer < 16)
        {
            tmp++;
            ag_integer  =  ag_interget_table[tmp];
            ag_fraction = 0;
        }
        else
        {
        }
    }
 
    switch (ag_integer)
    {
        case 1 :
            ag_integer = 0x00;
            break;
        case 2 :
            ag_integer = 0x10;
            break;
        case 4 :
            ag_integer = 0x30;
            break;
        case 8 :
            ag_integer = 0x70;
            break;
        case 16 :
            ag_integer = 0xf0;
            break;
        default:
            ag_integer = 0x00;
            break;
    }

    ag_return = ag_integer|ag_fraction;
 
	return ag_return;
}
 



int isp_update_auto_exposure_param(struct isp_struct *isp)
{
    unsigned char exposure_time_msb = 0;
	unsigned char exposure_time_lsb = 0;
	unsigned int ag_value = 0;
	


		if (!isp->aec_param.bByPassAE) // start AE
		{
			//get isp_exposure_hist_feature 
		   	tmp_exposure_time = isp->aec_param.current_exposure_time;
		   	tmp_a_gain = isp->aec_param.current_a_gain;

		   	brightness_average(isp);
	      	isp_luminance_aec_time_calc(isp);		

				    
      		if((isp->aec_param.aec_status!=0))    	//need to update
       		{
       			if(isp->aec_param.bLinkage)
       			{
					isp_param_updata_with_ae(isp);		// sharpen
				}			


				// 每两帧写一次
				if(updata_auto_exposure_num == UPDATA_AUTO_EXPOSURE_FREQUENCE)
		       	{	
		       		if ((isp->aec_param.current_exposure_time>=0)&&
				   		(isp->aec_param.current_a_gain>=0))
				   
	       	   	    {	
	       	   	    
			           	exposure_time_msb =(isp->aec_param.current_exposure_time>>8)&0xff;
		    			exposure_time_lsb = isp->aec_param.current_exposure_time&0xff;
		  
						aksensor_set_param(0x16,exposure_time_msb);
						aksensor_set_param(0x10,exposure_time_lsb);

						//ag_value = cmos_gains_update(isp,isp->aec_param.current_a_gain);
						// aksensor_set_param(0x00,((isp->aec_param.current_a_gain)&0x7f) | (1<<0x7) );
						//aksensor_set_param(0x00, ag_value );

						isp->aec_param.current_a_gain = tmp_a_gain;
						

						updata_auto_exposure_num = 0;
						agc_aec_update_flag = 0;
						//printk("exp_time %d,  aver_L %d, a_gain %d,target:%d \n",isp->aec_param.current_exposure_time,
						//	isp->aec_param.hist_feature.mean_brightness,isp->aec_param.current_a_gain,isp->aec_param.target_lumiance);
			            
	       	   	    }
					else
					{
					    
						printk("ERROR time:%d,again:%d\n",isp->aec_param.current_exposure_time,isp->aec_param.current_a_gain);
					}

			   

       	   		}
			    else
			    {
			    
			         ag_value = cmos_gains_update(isp,isp->aec_param.current_a_gain);
					 aksensor_set_param(0x00, ag_value );
			         updata_auto_exposure_num++;
					 isp->aec_param.current_exposure_time = tmp_exposure_time;
		             //isp->aec_param.current_a_gain = tmp_a_gain;
			    }			
       	}
	   
	}



	return 0;

	
}







/* compress histogram */
int isp_get_histogram(struct isp_struct *isp)
{
	int i, j;
	
	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;

	for (i = 0, j = 0; i < HISTOGRAM_SIZE; i = i+12, j++) {
		histo_arr[j] = ((isp->histo_base[i+11]<<16)|(isp->histo_base[i+10]<<8)|(isp->histo_base[i+9]))
					+((isp->histo_base[i+8]<<16)|(isp->histo_base[i+7]<<8)|(isp->histo_base[i+6]))
					+((isp->histo_base[i+5]<<16)|(isp->histo_base[i+4]<<8)|(isp->histo_base[i+3]))
					+((isp->histo_base[i+2]<<16)|(isp->histo_base[i+1]<<8)|(isp->histo_base[i]));
	}	
	
	return 0;
}

/**
 * @brief: initial ISP parameter about image effects.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 */
static void isp_initial_effects(struct isp_struct *isp)
{
	// following called support RGB only
	isp_set_demosaic(isp, &stDemosaic);
	
	isp_set_rgb_filter(isp, &stRGBFilter);
	
	isp_set_defect_pixel(isp, &stDefectPixel);
	
	isp_set_lens_correct(isp, &stLensCorrect);
	
	isp_set_color_correct(isp, &stColorCorrect);
	
	isp_set_special_effect(isp, &stSpecialEffect);
	
	isp_set_black_balance(isp, &stBlackBlance);
	
	if (isp->wb_param_en)
		isp_set_manu_wb(isp, &stWhiteBlance);

	isp_set_ae_attr(isp, &stAeAttr);	

	// following called support RGB and YUV
	isp_set_gamma_calc(isp, &stGammaCalc[0]);
	isp_set_gamma_calc(isp, &stGammaCalc[1]);
	
	isp_set_brightness_enhance(isp, &stBrigtnessEnhance);
	
	isp_set_uv_iso_filter(isp, &stUVFilter);
	
	isp_set_uv_saturation(isp, &stSaturation);
}

/**
 * @brief: set ISP controller for startting data collection.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 */
int isp_apply_mode(struct isp_struct *isp)
{
	int width = isp->fmt_width;
	int height = isp->fmt_height;
	int ret = 0;

	isp_dbg("%s: width=%d, height=%d\n", __func__, width, height);
	switch(isp->cur_mode_class) {
	case ISP_YUV_CLASS:
		switch(isp->cur_mode) {
		
		// ISP_YUV_OUT mode is direct output,  and is not support minor channel output
		case ISP_YUV_OUT:
			REG32(isp->base + ISP_FMT_CHK) = (width * 2 | (height << 14));
			REG32(isp->base + ISP_IMG_PARA) = 0x80001000;
			REG32(isp->base + ISP_IRQ_STATUS) = 0x040017ff;
			REG32(isp->base + ISP_PERI_PARA) = 0x200340c0;
			break;
			
		// ISP_YUV_BYPASS mode is support minor channel output
		case ISP_YUV_BYPASS:
			setup_yuv_video_bypass(isp);

			REG32(isp->base + ISP_FMT_CHK) = (width * 2 | (height << 14));
			REG32(isp->base + ISP_IMG_PARA) = 0x80001000;
			REG32(isp->base + ISP_IRQ_STATUS) = 0x040017ff;
			REG32(isp->base + ISP_PERI_PARA) = 0x000104c0;
			break;
			
		case ISP_YUV_VIDEO_OUT:
			setup_yuv_video_out(isp);

			REG32(isp->base + ISP_FMT_CHK) = (width * 2 | (height << 14));
			REG32(isp->base + ISP_IMG_PARA) = 0x80001000;
			REG32(isp->base + ISP_IRQ_STATUS) = 0x044007ff;
			REG32(isp->base + ISP_PERI_PARA) = 0x000340c5;
			break;
			
		case ISP_YUV_VIDEO_BYPASS:
			setup_yuv_video_bypass(isp);

			REG32(isp->base + ISP_FMT_CHK) = (width * 2 | (height << 14));
			REG32(isp->base + ISP_IMG_PARA) = 0x80001000;
			REG32(isp->base + ISP_IRQ_STATUS) = 0x000017ff;
			REG32(isp->base + ISP_PERI_PARA) = 0x000104c5;
			break;
			
		case ISP_YUV_MERGER_OUT:
		case ISP_YUV_VIDEO_MERGER_OUT:
		case ISP_YUV_BIG:
			break;
			
		default:
			break;
		}
		
		break;
		
	case ISP_RGB_CLASS:
		setup_rgb_video(isp);

		REG32(isp->base + ISP_FMT_CHK) = (width | (height<<14));
		REG32(isp->base + ISP_IMG_PARA) = 0x80000800;
		//REG32(isp->base + ISP_HISTO_CFG) = 0x00000000;
		REG32(isp->base + ISP_IRQ_STATUS) = 0x000017ff;
		
		switch(isp->cur_mode) {
		case ISP_RGB_OUT:
			REG32(isp->base + ISP_PERI_PARA) = 0x000120c0; 
			break;
			
		case ISP_RGB_VIDEO_OUT:
			REG32(isp->base + ISP_PERI_PARA) = 0x000120c5;
			break;
			
		case ISP_RGB_BIG:
			break;
			
		default:
			break;
		}
		
		break;
		
	case ISP_JPEG_CLASS:
		switch(isp->cur_mode) {
			case ISP_JPEG_MODE:
			case ISP_JPEG_VIDEO:
			break;
			
		default:
			break;
		}
		break;
		
	default:
		printk("Unrecognized mode %d\n", isp->cur_mode);
	}

	isp_initial_effects(isp);
	isp_update_regtable(isp, 1);
	return ret;
}

/**
 * @brief: start data collection.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 *
 * Note: To enable raw upload, the 14th bit of peripheral parameter register must be 
 * set, otherwise the data cannot be captured
 */
void isp_start_capturing(struct isp_struct *isp)
{
	unsigned long peri_status;

	//REG32(isp->base + ISP_PERI_PARA) &= ~(1 << 28);
	
	switch(isp->cur_mode) {
	case ISP_YUV_OUT:
	case ISP_YUV_VIDEO_OUT:
	case ISP_YUV_BIG:
	case ISP_JPEG_MODE:
	case ISP_JPEG_VIDEO:
	case ISP_RGB_BIG:
		peri_status = REG32(isp->base + ISP_PERI_PARA);
		peri_status |= (3 << 30) | (1 << 14); 
		REG32(isp->base + ISP_PERI_PARA) = peri_status;
		break;
	default:
		REG32(isp->base + ISP_PERI_PARA) |= (3 << 30);
	}
}

/**
 * @brief: stop data collection.
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 */
void isp_stop_capturing(struct isp_struct *isp)
{
	unsigned long peri_status;
	
	peri_status = REG32(isp->base + ISP_PERI_PARA);
	peri_status |= (1 << 30);
	peri_status &= ~(1 << 31);
	REG32(isp->base + ISP_PERI_PARA) = peri_status;
}

static void isp_init_awb_parms(struct isp_struct *isp)
{
	int i;
	struct awb_colortemp_set *colortemp;
	struct isp_auto_white_balance *awb;
	struct isp_color_correct_awb *cc_awb;
	struct color_correct_set *color_correct_temp;

	//init AWB
	isp->awb_param.frame_cnt = 0;
	isp->awb_param.current_b_gain = 1024;
	isp->awb_param.current_g_gain = 1024;
	isp->awb_param.current_r_gain = 1024;
	
	isp->awb_param.target_b_gain = 1024;
	isp->awb_param.target_g_gain = 1024;
	isp->awb_param.target_r_gain = 1024;
	
	isp->awb_param.auto_wb_step = 5;
	isp->awb_param.colortemp_set_num = ISP_COLORTEMP_MODE_COUNT;
	isp->awb_param.colortemp_set_seq = 0;

	BUG_ON(ISP_COLORTEMP_MODE_COUNT != ARRAY_SIZE(stAwb));

	/*one by one*/
	for(i=ISP_COLORTEMP_MODE_A; i<ISP_COLORTEMP_MODE_COUNT; i++) {
		colortemp = &isp->awb_param.colortemp_set[i];
		awb = &stAwb[i];

		colortemp->gr_low = awb->gr_low;
		colortemp->gr_high = awb->gr_high;
		colortemp->gb_high = awb->gb_high;
		colortemp->gb_low = awb->gb_low;
		colortemp->grb_high = awb->grb_high;
		colortemp->grb_low = awb->grb_low;

		colortemp->r_low =  awb->r_low;	
		colortemp->r_high = awb->r_high;
		colortemp->g_low =  awb->g_low;	
		colortemp->g_high = awb->g_high;
		colortemp->b_low =  awb->b_low;	
		colortemp->b_high = awb->b_high;

		colortemp->channel_r_total = awb->co_r;
		colortemp->channel_b_total = awb->co_g;
		colortemp->channel_g_total = awb->co_b;
	}

	// init color_correct_set
	isp->awb_param.current_colortemp_index = 2;


	// ISP_A_LIGHT---ISP_D75_LIGHT
	for(i=0; i<3; i++ )
	{
		color_correct_temp = &isp->awb_param.color_correct_set[i];
		cc_awb = &stCCwithAwb[i];

		color_correct_temp->cc_thrs_low = cc_awb->cc_thrs_low;
		color_correct_temp->cc_thrs_high = cc_awb->cc_thrs_high;

		color_correct_temp->ccMtrx[0][0] = cc_awb->ccMtrx[0][0];
		color_correct_temp->ccMtrx[0][1] = cc_awb->ccMtrx[0][1];
		color_correct_temp->ccMtrx[0][2] = cc_awb->ccMtrx[0][2];
		
		color_correct_temp->ccMtrx[1][0] = cc_awb->ccMtrx[1][0];
		color_correct_temp->ccMtrx[1][1] = cc_awb->ccMtrx[1][1];
		color_correct_temp->ccMtrx[1][2] = cc_awb->ccMtrx[1][2];

		color_correct_temp->ccMtrx[2][0] = cc_awb->ccMtrx[2][0];
		color_correct_temp->ccMtrx[2][1] = cc_awb->ccMtrx[2][1];
		color_correct_temp->ccMtrx[2][2] = cc_awb->ccMtrx[2][2];
	
	}
	// default
	isp->awb_param.color_correct_set[3].ccMtrx[0][0] = stCCwithAwb[2].ccMtrx[0][0];
	isp->awb_param.color_correct_set[3].ccMtrx[0][1] = stCCwithAwb[2].ccMtrx[0][1];
	isp->awb_param.color_correct_set[3].ccMtrx[0][2] = stCCwithAwb[2].ccMtrx[0][2];
		

	
}


void isp_dump_register(struct isp_struct *isp)
{
	int i;

	printk("0x20000000=0x%08lx\n", REG32(isp->base + ISP_PERI_PARA));
	printk("0x20000004=0x%08lx\n", REG32(isp->base + ISP_FMT_CHK));
	printk("0x20000008=0x%08lx\n", REG32(isp->base + ISP_IMG_PARA));
	printk("0x200000014=0x%08lx\n", REG32(isp->base + ISP_IRQ_STATUS));
	printk("0x20000001c=0x%08lx\n", REG32(isp->base + ISP_ORDER_CTRL));

	printk("0x200000020:\n");
	REG32(isp->base + 0x20) = 0x00000001;
	for (i = 0; i <28; i++) {
		printk("%02d: 0x%08lx\t", i, REG32(isp->base + 0x20));
		if (i % 5 == 0)
			printk("\n");
	}
	printk("\n");

	printk("0x200000024:\n");
	REG32(isp->base + 0x24) = 0x00000001;
	for (i = 0; i <28; i++) {
		printk("%02d: 0x%08lx\t", i, REG32(isp->base + 0x24));
		if (i % 5 == 0)
			printk("\n");
	}
	printk("\n");

	printk("0x200000028:\n");
	REG32(isp->base + 0x28) = 0x00000001;
	for (i = 0; i <64; i++) {
		printk("%02d: 0x%08lx\t", i, REG32(isp->base + 0x28));
		if (i % 5 == 0)
			printk("\n");
	}
	printk("\n");

	printk("0x20000002c:\n");
	REG32(isp->base + 0x2c) = 0x00000001;
	for (i = 0; i <28; i++) {
		printk("%02d: 0x%08lx\t", i, REG32(isp->base + 0x2c));
		if (i % 5 == 0)
			printk("\n");
	}
	printk("\n");

	printk("0x200000030:\n");
	REG32(isp->base + 0x30) = 0x00000001;
	for (i = 0; i <27; i++) {
		printk("%02d: 0x%08lx\t", i, REG32(isp->base + 0x30));
		if (i % 5 == 0)
			printk("\n");
	}
	printk("\n");	
}


/**
 * @brief: prepare resource for initial ISP controller
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 */
int isp_module_init(struct isp_struct *isp)
{
	//register table total size is (32 * 4 * 6) = 768byte. here is more than real size.
	isp->bytes = 1024;	
	isp->area = dma_alloc_coherent(NULL, isp->bytes, &isp->addr, GFP_KERNEL);
	if (!isp->area) {
		printk(KERN_ERR"Failed to allocate memory for register table\n");
		return -1;
	}

	memset(isp->area, 0, isp->bytes);
	isp_dbg("isp device init: isp->area=0x%p, isp->addr=0x%p, isp->bytes=%d\n", 
		isp->area, (void *)isp->addr, isp->bytes);

	isp->isp_ctrltbl = (unsigned long *)isp->area;	//0x20, register table
	isp->img_ctrltbl1 = isp->isp_ctrltbl + 32;		//0x24
	isp->img_lumitbl = isp->img_ctrltbl1 + 32;		//0x28
	isp->osd_chktbl = isp->img_lumitbl + 64;		//0x2c
	isp->img_ctrltbl2 = isp->osd_chktbl + 32;		//0x30

	isp->histo_base = dma_alloc_coherent(NULL, HISTOGRAM_SIZE, &isp->histo_phyaddr, GFP_KERNEL);
	if (isp->histo_base == NULL) {
		printk("no memory for histopram %d\n", -ENOMEM);
	}
	memset(isp->histo_base, 0, HISTOGRAM_SIZE);

	isp_init_awb_parms(isp);
	
	isp_dbg("Allocate %d bytes for register table\n", isp->bytes);

    isp->aec_param.current_exposure_time = 0xf0;	
	isp->aec_param.current_d_gain = 0;
	isp->aec_param.a_gain_max = 127;
	isp->aec_param.a_gain_min = 16;
	isp->aec_param.current_a_gain = isp->aec_param.a_gain_min;
	isp->aec_param.d_gain_max = 4;
	isp->aec_param.d_gain_min = 0;
	isp->aec_param.exposure_time_max = 0xffff;
	isp->aec_param.exposure_time_min = 0x00;
	isp->aec_param.stable_range = 10;
	isp->aec_param.target_lumiance =0x50;
	isp->aec_param.exposure_step = 1;
	isp->aec_param.current_laec_time=0xffff;

	
	isp->aec_param.bLinkage = 1;
	isp->aec_param.bCompensation = 1;
	isp->aec_param.bGama = 1;
	isp->aec_param.bByPassAE = 0;	
	
	return 0;
}

/**
 * @brief: release resource for ISP controller
 * 
 * @author: caolianming
 * @date: 2014-01-06
 * @param [in] *isp: isp_struct structure, indicate ISP hard device information
 */
void isp_module_fini(struct isp_struct *isp)
{
	dma_free_coherent(NULL, HISTOGRAM_SIZE, isp->histo_base, isp->histo_phyaddr);
	dma_free_coherent(NULL, isp->bytes, isp->area, isp->addr);
}


