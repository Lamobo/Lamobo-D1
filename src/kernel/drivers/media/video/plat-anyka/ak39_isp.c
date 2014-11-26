#include <linux/slab.h>
#include <linux/hardirq.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <plat-anyka/aksensor.h>

#include "ak39_isp.h"

//#define ISP_DEBUG
#ifdef ISP_DEBUG
#define isp_dbg(stuff...)		printk(KERN_INFO " ISP: " stuff)
#else
#define isp_dbg(fmt, args...)	do{}while(0)
#endif 

#define isp_info(stuff...)		printk(KERN_INFO " ISP: " stuff)

#define BRIGHTNESS_CHART_SIZE	64
#define HISTOGRAM_SIZE			(1024*3)

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
	{ 0x11111010, 0x14131212, 0x16151514, 0x18181717, 0x1b1a1a19, 0x1e1d1c1c, 0x20201f1e, 0x24232221, 
	0x27262524, 0x2a292928, 0x2e2d2c2b, 0x3231302f, 0x37363533, 0x3b3b3938, 0x403f3e3d, 0x44434241, 
	0x49474645, 0x4d4c4b4a, 0x5251504e, 0x57565553, 0x5c5b5a58, 0x61605f5d, 0x66656462, 0x6b6a6968, 
	0x716f6e6d, 0x76747372, 0x7b797877, 0x807e7d7c, 0x84838281, 0x89888786, 0x8e8d8b8a, 0x9291908f, 
	0x96959493, 0x9b9a9897, 0x9e9d9c9c, 0xa2a1a09f, 0xa6a5a4a3, 0xa9a8a8a7, 0xadacabaa, 0xb0afaead, 
	0xb3b2b1b1, 0xb6b5b4b4, 0xb9b8b7b7, 0xbbbbbab9, 0xbebebdbc, 0xc1c0c0bf, 0xc4c3c2c2, 0xc6c5c5c4, 
	0xc8c8c7c7, 0xcbcacac9, 0xcdcdcccc, 0xd0cfcfce, 0xd2d2d1d0, 0xd4d4d3d3, 0xd7d6d6d5, 0xd9d8d8d7, 
	0xdbdbdada, 0xdedddcdc, 0xe0dfdfde, 0xe2e2e1e0, 0xe4e4e3e3, 0xe7e6e5e5, 0xe9e8e8e7, 0xebebeae9},
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

void isp_update_regtable(struct isp_struct *isp, int force)
{
	if (force)
		REG32(isp->base + ISP_ORDER_CTRL) = ((3<<30) | (isp->addr & 0x3fffffff));
	else
		REG32(isp->base + ISP_ORDER_CTRL) = ((1<<31) | (isp->addr & 0x3fffffff));
}


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


	isp->wb_param.co_g = 1024;
	isp->wb_param.co_r = 1024;
	isp->wb_param.co_b = 1024;

	__isp_set_white_pram(isp, isp->wb_param.co_g, 
				isp->wb_param.co_r, isp->wb_param.co_b);
	return 0;
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
			isp_dbg("	ctrl->ccMtrx[%d][%d] = %d\n", i, j, ctrl->ccMtrx[i][j]);
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

static void __isp_set_brigtness(struct isp_struct *isp,  
		struct isp_brightness_enhance *ctrl)
{
	ctrl->y_thrs = (ctrl->y_thrs > 2048) ? 2048 : ctrl->y_thrs;
	ctrl->y_edgek = (ctrl->y_edgek > 2048) ? 2048 : ctrl->y_edgek;
	ctrl->ygain = (ctrl->ygain > 64) ? 63 : 
					((ctrl->ygain < -64) ? -63 : ctrl->ygain);

	isp->img_ctrltbl1[12] = (ctrl->ygain << Y_GAIN_BIT)
			|(((ctrl->y_thrs * 9) & 0x7ff) << Y_EDGE_THRS_BIT)
			|((ctrl->y_edgek & 0x7ff) << Y_EDGE_K_BIT);

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
	int width = isp->lens_use_width;
	int height = isp->lens_use_height;
	unsigned long cmd;
	int value = 0, i;
	
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

	if ((width <= 0) || (height <= 0)) {
		width = isp->fmt_width;
		height = isp->fmt_height;		
	}
		
	// lens range setting
	value = (width/2) * (width/2) + (height/2) * (height/2);
	cmd = value / 22;
	for (i = 0; i < 10; i++) {
		isp->isp_ctrltbl[13+i] |= (cmd * (2*i+1)) & 0x3fffff;
	}
	
	isp->isp_ctrltbl[23] = ((ctrl->lens_yref & 0x1fff) << LENS_YREF_BIT) 
				|(ctrl->lens_xref & 0x1fff);
	isp->isp_ctrltbl[24] = (1 << 31)|(((ctrl->lens_xref * ctrl->lens_xref)
				+ (ctrl->lens_yref * ctrl->lens_yref)) & 0x7ffffff);
}


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
		if (isp->auto_wb_param_en)
			isp_update_auto_wb_param(isp);
		
		//isp_set_histogram(isp, NULL);
		//isp_get_histogram(isp);
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

int isp_clear_irq_status(struct isp_struct *isp)
{
	REG32(isp->base + ISP_IRQ_STATUS) |= 0x7ff;
	return 0;
}

int isp_clear_irq(struct isp_struct *isp)
{
	REG32(isp->base + ISP_IRQ_STATUS) = 0;
	return 0;
}

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
	
	isp->img_ctrltbl2[24] = ((iheight & 0x1fff) << OSD_SIZE_V_BIT)
							|(iwidth & 0x1fff);

	// save is here, used for lens_range
	isp->lens_use_width = iwidth;
	isp->lens_use_height = iheight;
	
	isp_dbg("%s: osd total size[w=%d h=%d]\n", __func__, iwidth, iheight);
	return 0;
}

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


int isp_set_cutter_window(struct isp_struct *isp, int xpos, int ypos, int width, int height)
{
	struct isp_demosaic demosac;
	struct isp_rgb_filter rgb_filter;
	int iwidth = width;
	int iheight = height;
	
	isp_dbg("%s. entry. iwidth=%d iheight=%d\n", __func__, 
			iwidth, iheight);

	if (width > isp->fmt_width || height > isp->fmt_height) {
		printk(KERN_ERR"%s: invalid cutter window size\n", __func__);
		return -EINVAL;
	}

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
		xpos = 3;
		ypos = 3;
		iwidth -= 4;
		iheight -= 4;
	}
	
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
		demosac.threshold = 1300;
		isp->demo_sac_thres = demosac.threshold;
	} else
		demosac.threshold = isp->demo_sac_thres;
	isp_set_demosaic(isp, &demosac);

	// enable rgb filter (noise remove)
	if (isp->rgb_filter_thres <= 0) {
		rgb_filter.threshold = 142;
		isp->rgb_filter_thres = rgb_filter.threshold;
	} else
		rgb_filter.threshold = isp->rgb_filter_thres;
	isp_set_rgb_filter(isp, &rgb_filter);
	
	isp->cut_width = width;
	isp->cut_height = height;
	
	update_image_size(isp);
	return 0;
}

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
		offset_w = 18;
		offset_h = 16;
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

// noise remove
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

int isp_set_uv_iso_filter(struct isp_struct *isp, struct isp_uv_filter *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

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

// bad pixel defect and fixed
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
		isp_set_auto_wb_support(isp, ctrl);
		
		isp->wb_param_en = 0;
		isp->auto_wb_param_en = 1;
	}
	
	//isp_update_regtable(isp, 0);
	return 0;
}

int isp_set_color_correct(struct isp_struct *isp, struct isp_color_correct *ctrl)
{
	int i;
	
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	BUG_ON(ctrl == NULL);

	isp->img_ctrltbl1[11] &= ~(0x3fffff);
	for (i = 0; i < 9; i++)
		isp->img_ctrltbl1[2+i] = 0x0;
	
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

int isp_set_gamma_calc(struct isp_struct *isp, struct isp_gamma_calculate *ctrl)
{
	int i;
	
	isp_dbg("%s enter.\n", __func__);
	
	BUG_ON(ctrl == NULL);
	
	if (ctrl->enable == 0) {
		memset(isp->img_lumitbl, 0, sizeof(unsigned long)*BRIGHTNESS_CHART_SIZE);
		// disable brightness chart table
		isp->img_ctrltbl1[11] &= ~(1 << 30);
		isp->gamma_calc_en = 0;
		
		//isp_update_regtable(isp, 0);
	} else {
		if (ctrl->is_sync == 0) {
			for(i = 0; i < 32/*BRIGHTNESS_CHART_SIZE >> 1*/; i++)				
				isp->img_lumitbl[i] = ctrl->gamma[i];
		}

		if (ctrl->is_sync == 1) {
			for(i = 0; i < 32/*BRIGHTNESS_CHART_SIZE >> 1*/; i++)
				isp->img_lumitbl[32+i] = ctrl->gamma[i];
			
			// enable brightness adjust
			isp->img_ctrltbl1[11] |= (1 << 30);
			isp->gamma_calc_en = 1;
			
			//isp_update_regtable(isp, 0);
		}
	}
	
	return 0;
}

int isp_set_brightness_enhance(struct isp_struct *isp, struct isp_brightness_enhance *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	BUG_ON(ctrl == NULL);
	
	isp->img_ctrltbl1[12] = 0x0;
	
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

		isp->yedge_en = 1;
		//isp->uv_isoflt_en = 1;
	}
	
	update_image_size(isp);
	isp_update_regtable(isp, 0);
	return 0;
}

int isp_set_uv_saturation(struct isp_struct *isp, struct isp_saturation *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

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

int isp_set_histogram(struct isp_struct *isp, struct isp_histogram *ctrl)
{
	unsigned long regval;
	
	isp_dbg("%s enter.\n", __func__);
	
	if (isp->cur_mode_class != ISP_RGB_CLASS)
		return 0;
	
	//BUG_ON(ctrl == NULL);
#if 0
	if (ctrl->enable == 0) {

		isp->histo_en = 0;
	} else {
#endif
		do {
			regval = REG32(isp->base + ISP_HISTO_CFG);
		} while(regval >> 31);
		
		REG32(isp->base + ISP_HISTO_CFG) = (1 << 31)
						|(isp->histo_phyaddr & 0x3fffffff);
		isp->histo_en = 1;
//	}

	return 0;
}

int isp_set_special_effect(struct isp_struct *isp, struct isp_special_effect *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	BUG_ON(ctrl == NULL);
	
	isp->img_ctrltbl1[0] &= ~(0xff) << 24;
	isp->img_ctrltbl1[14] &= ~(0xffff);
	isp->img_ctrltbl1[15] = 0x0;

	if (ctrl->enable == 0) {
		//disable yuv and yuv_solar
		isp->img_ctrltbl1[11] &= ~(0x3 << 27);

		isp->yuv_effect_en = 0;
		isp->yuv_solar_en = 0;
	} else {
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
		
		//enable yuv and yuv_solar
		isp->img_ctrltbl1[11] |= (0x3 << 27);
		isp->yuv_effect_en = 1;
		isp->yuv_solar_en = 1;
	}

	//isp_update_regtable(isp, 0);
	return 0;
}


/* ~~~~~~~~~~ ISP control ~~~~~~~~~ */
int isp_set_brightness(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{
	struct isp_brightness_enhance bl_edge;

	isp_dbg("%s. enter. ctrl->val=%d\n", __func__, ctrl->val);

	memset(&bl_edge, 0, sizeof(struct isp_brightness_yedge));
	isp->img_ctrltbl1[12] = 0x0;
	
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
		bl_edge.y_edgek = 1000;
		bl_edge.y_thrs = 2;

		switch(ctrl->val) {
		case ISP_BRIGHTNESS_0:
			bl_edge.ygain = -30;
			break;
		case ISP_BRIGHTNESS_1:
			bl_edge.ygain = -15;
			break;
		case ISP_BRIGHTNESS_2:
			bl_edge.ygain = 0;
			break;
		case ISP_BRIGHTNESS_3:
			bl_edge.ygain = 10;
			break;
		case ISP_BRIGHTNESS_4:
			bl_edge.ygain = 20;
			break;
		case ISP_BRIGHTNESS_5:
			bl_edge.ygain = 30;
			break;
		case ISP_BRIGHTNESS_6:
			bl_edge.ygain = 40;
			break;
		}

		__isp_set_brigtness(isp, &bl_edge);
		
		isp->yedge_en = 1;
		//isp->uv_isoflt_en = 1;
	//}
	
	update_image_size(isp);
	isp_update_regtable(isp, 0);
	return 0;
}

int isp_set_gamma(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{
	isp_dbg("%s. enter. ctrl->val=%d\n", __func__, ctrl->val);
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

int isp_set_saturation(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{
	struct isp_saturation satu;

	isp_dbg("%s. enter. ctrl->val=%d\n", __func__, ctrl->val);
	
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
			satu.Khigh = 400;
			satu.Klow = 249;
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

//颜色校正每个sensor都不一样
// brief: return -1 is indicate the function is not support YUV data
int isp_set_sharpness(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{

	struct isp_color_correct colcon;
	int i;

	isp_dbg("%s enter. ctrl->val=%d\n", __func__, ctrl->val);

	if (isp->cur_mode_class != ISP_RGB_CLASS) {
		return 0;
	}
	
	memset(&colcon, 0, sizeof(struct isp_color_correct));
	isp->img_ctrltbl1[11] &= ~(0x3fffff);
	for (i = 0; i < 9; i++)
		isp->img_ctrltbl1[2+i] = 0x0;
	
	if (ctrl->val == ISP_SHARPNESS_0) {
		// disable color correction
		isp->img_ctrltbl1[11] &= ~(1 << 31);
		isp->color_crr_en = 0;
	} else {
		switch(ctrl->val) {
		case ISP_SHARPNESS_1:
			colcon.cc_thrs_low = 40;
			colcon.cc_thrs_high = 41;
			colcon.ccMtrx[0][0] = 1024;
			colcon.ccMtrx[0][1] = 150;
			colcon.ccMtrx[0][2] = -150;
			colcon.ccMtrx[1][0] = 150;
			colcon.ccMtrx[1][1] = 1024;
			colcon.ccMtrx[1][2] = -150;
			colcon.ccMtrx[2][0] = 301;
			colcon.ccMtrx[2][1] = -301;
			colcon.ccMtrx[2][2] = 1024;
			break;
		case ISP_SHARPNESS_2:
		case ISP_SHARPNESS_3:
		case ISP_SHARPNESS_4:
		case ISP_SHARPNESS_5:
		case ISP_SHARPNESS_6:
			break;
		}

		__isp_set_color_crr(isp, &colcon, 3, 3);

		isp->color_crr_en = 1;
	}
	
	//isp_update_regtable(isp, 0);
	return 0;
}

#define ABS_DIFF(a, b)   (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))

struct awb_rgb_param {
	unsigned short r[3];
	unsigned short b[3];
};

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
								
				if (((co_r_tmp > 700) && (co_r_tmp < 1700)) &&
					((nr1+nr2)<30 || (nr1+nr3)<30 || (nr2+nr3)<30))
					isp->wb_param.co_r = co_r_tmp;
				
				if (((co_b_tmp > 700) && (co_b_tmp < 1700)) &&
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

// brief: return -1 is indicate the function is not support YUV data
int isp_auto_set_wb_param(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{
	struct isp_auto_white_balance auto_wb_param;
	
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS) {
		return 0;
	}
	
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
		auto_wb_param.r_high = 984;
		auto_wb_param.r_low = 555;	
		auto_wb_param.g_high = 988;
		auto_wb_param.g_low = 718;	
		auto_wb_param.b_high = 979;
		auto_wb_param.b_low = 542;	
		
		auto_wb_param.grb_high = 826;
		auto_wb_param.grb_low = 437; 
		auto_wb_param.gr_high = 822;
		auto_wb_param.gr_low = 433;
		auto_wb_param.gb_high = 818;
		auto_wb_param.gb_low = 420;
		
		isp_set_auto_wb_support(isp, &auto_wb_param);
		
		//disable manu while balance, enable auto while balance
		isp->wb_param_en = 0;
		isp->auto_wb_param_en = 1;
	}
	
	//isp_update_regtable(isp, 0);
	return 0;
}

// brief: return -1 is indicate the function is not support YUV data
int isp_manu_set_wb_param(struct isp_struct *isp, struct v4l2_ctrl *ctrl)
{
	isp_dbg("%s enter.\n", __func__);

	if (isp->cur_mode_class != ISP_RGB_CLASS) {
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
	
	average = sum/tmp;
	//isp_dbg("[%d] %d %d\n", average, sum, tmp);
	return average;
}

void update_exposure_value(struct isp_struct *isp)
{
	unsigned int value;
	static int flags_max = 0, flags_min = 0;
	
	value = brightness_average(isp);
	
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
	
	schedule_delayed_work(&isp->work, msecs_to_jiffies(20));
	return 0;
}

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
		
		isp_update_regtable(isp, 1);
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
		
		isp_update_regtable(isp, 1);
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

	return ret;
}

/*
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

void isp_stop_capturing(struct isp_struct *isp)
{
	unsigned long peri_status;
	
	peri_status = REG32(isp->base + ISP_PERI_PARA);
	peri_status |= (1 << 30);
	peri_status &= ~(1 << 31);
	REG32(isp->base + ISP_PERI_PARA) = peri_status;
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

	isp_dbg("Allocate %d bytes for register table\n", isp->bytes);

	return 0;
}


void isp_module_fini(struct isp_struct *isp)
{
	dma_free_coherent(NULL, HISTOGRAM_SIZE, isp->histo_base, isp->histo_phyaddr);
	dma_free_coherent(NULL, isp->bytes, isp->area, isp->addr);
}


#if 0
void build_gamma_table(struct isp_struct *isp)
{
    int i;
	unsigned char lum_table[256] = {0};

	// generate lum correction parameter
	for (i=0; i<256; i++) {   
	    if(i<=13)
			lum_table[i]=(unsigned char)(0.46154*i +0);
		if (13<i && i<=21)
			lum_table[i]=(unsigned char)(2*i -20);
		if (21<i&&i<=26)
			lum_table[i]=(unsigned char)(4*i -62);
		if (26<i && i<=36)
			lum_table[i]=(unsigned char)(2.4*i -20.4);
		if (36<i && i<=48)
			lum_table[i]=(unsigned char)(2.1667*i -12);
		if (48<i && i<=61)
			lum_table[i]=(unsigned char)(1.5385*i +18.154);
		if (61<i && i<=73)
			lum_table[i]=(unsigned char)(1.5*i +20.5);
	    if (73<i && i<=93)
			lum_table[i]=(unsigned char)(0.95*i +60.65);
	    if (93<i && i<=114)
			lum_table[i]=(unsigned char)(0.80952*i +73.714);
	    if (114<i && i<=135)
			lum_table[i]=(unsigned char)(0.7619*i +79.091);
	    if (135<i && i<=157)
			lum_table[i]=(unsigned char)(0.63636*i +96.091);
		if (157<i && i<=190)
			lum_table[i]=(unsigned char)(0.36364*i +138.91);
		if (190<i && i<=221)
			lum_table[i]=(unsigned char)(0.25806*i +158.97);
		if (221<i && i<=241)
			lum_table[i]=(unsigned char)(0.95*i +6.05);
		if (241<i && i<=255) {
			lum_table[i]=(unsigned char)((1.4286*i -109.29)>255?254:(1.4286*i -109.29));
		}		
	}
	
	for (i=5; i<247; i++) {
		lum_table[i]=(unsigned char)((lum_table[i-5]+lum_table[i-4]
			+lum_table[i-3]+lum_table[i-2]
			+lum_table[i-1]+lum_table[i]
			+lum_table[i+1]+lum_table[i+2]
			+lum_table[i+3]+lum_table[i+4]
			+lum_table[i+5]+lum_table[i+6]
			+lum_table[i+7])/13);
	}

	//copy to isp->img_lumitbl
	for(i = 0; i < 256; i+=4) {
		isp->img_lumitbl[i>>2] = lum_table[i]|(lum_table[i+1] << 8)
					|(lum_table[i+2] << 16)|(lum_table[i+3] << 24);
	}
	
	printk("\n");
	for(i = 0; i < 64; i++) {
		printk("%08x  ", isp->img_lumitbl[i]);
		if ((i != 0) && (i % 5 == 0))
			printk("\n");
	}
} 
#endif

