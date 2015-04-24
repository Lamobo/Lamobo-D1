#ifndef ISP_INITIALIZE_PARAMETER_DEFINE_H
#define ISP_INITIALIZE_PARAMETER_DEFINE_H

#include <plat-anyka/isp_interface.h>

struct isp_auto_white_balance stAwb[] = {
{
	0x67,
	1,
	5,
	0,
	328,
	550,
	700,
	972,
	550,
	800,
	140,
	920,
	140,
	920,
	140,
	750,
	0,
	0,
	0
 },
{
	0x67,
	1,
	5,
	1,
	461,
	650,
	650,
	850,
	563,
	717,
	140,
	920,
	140,
	920,
	140,
	750,
	0,
	0,
	0
 },
{
	0x67,
	1,
	5,
	2,
	600,
	900,
	400,
	800,
	500,
	850,
	0,
	750,
	140,
	920,
	140,
	750,
	0,
	0,
	0
 },
{
	0x67,
	0,
	5,
	3,
	0,
	1,
	0,
	1,
	0,
	1,
	0,
	1,
	0,
	1,
	0,
	1,
	0,
	0,
	0
 },
{
	0x67,
	0,
	5,
	4,
	0,
	1,
	0,
	1,
	0,
	1,
	0,
	1,
	0,
	1,
	0,
	1,
	0,
	0,
	0
 }
};

struct isp_black_balance stBlackBlance = {
	0x60,
	1,
	16,
	16,
	16
};

struct isp_brightness_enhance stBrigtnessEnhance = {
	0x70,
	1,
	0,
	2,
	500
};

struct isp_color_correct stColorCorrect = {
	0x68,
	1,
	0,
	1,
	{
		{1575, -323, -223},
		{-287, 1423, -111},
		{-239, -867, 2129},
	}
};

struct isp_demosaic stDemosaic = {
	0x62,
	1,
	53
};

struct isp_rgb_filter stRGBFilter = {
	0x63,
	1,
	136
};

struct isp_uv_filter stUVFilter = {
	0x64,
	0
};

struct isp_defect_pixel stDefectPixel = {
	0x65,
	0,
	0
};

struct isp_gamma_calculate stGammaCalc[] = {
	{
		0x69,
		1,
		0,
		{
			0x4020101, 0xc0a0806, 0x1412100e, 0x1c1a1816, 0x2422201e, 0x2d2b2926, 0x3533312f, 0x3d3b3937, 
			0x4543413f, 0x4e4b4947, 0x5251504f, 0x57555453, 0x5b5a5958, 0x605e5d5c, 0x64636261, 0x69686665, 
			0x6d6c6b6a, 0x7271706e, 0x76757473, 0x7b7a7978, 0x807e7d7c, 0x84838281, 0x89878685, 0x8d8c8b8a, 
			0x91908f8e, 0x96959493, 0x9a999897, 0x9e9e9c9b, 0xa1a1a09f, 0xa4a4a3a2, 0xa7a7a6a5, 0xaaaaa9a8
		}
	},
	{
		0x69,
		1,
		1,
		{
			0xadadacab, 0xb0b0afae, 0xb3b2b2b1, 0xb6b5b5b4, 0xb9b8b7b7, 0xbbbbbab9, 0xbebdbdbc, 0xc1c0bfbf, 
			0xc3c3c2c1, 0xc6c5c5c4, 0xc8c8c7c6, 0xcbcac9c9, 0xcdcccccb, 0xcfcfcecd, 0xd1d1d0d0, 0xd3d3d2d2, 
			0xd5d5d4d4, 0xd7d7d6d6, 0xd9d9d8d8, 0xdbdadad9, 0xdddcdcdb, 0xe0e0dfde, 0xe3e2e2e1, 0xe5e5e4e4, 
			0xe7e7e6e6, 0xe9e9e8e8, 0xeaeaeae9, 0xecebebeb, 0xededecec, 0xeeeeeded, 0xefefeeee, 0xf0f0efef
		}
	}
};

struct isp_lens_correct stLensCorrect = {
	0x61,
	0,
	{
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0
	},
	{
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0
	},
	0,
	0
};

struct isp_saturation stSaturation = {
	0x72,
	0,
	302,
	255,
	0,
	5,
	0
};

struct isp_special_effect stSpecialEffect = {
	0x74,
	0,
	0,
	0,
	-2,
	255,
	0,
	189,
	25,
	109
};

struct isp_white_balance stWhiteBlance = {
	0x66,
	0,
	0,
	0,
	0
};


struct isp_ae_attr stAeAttr = {

	0x77,
	1,
	0x45,
	10,
	1,
	0xffff,
	0,
	0,
	1,
	0
};


struct isp_color_correct_awb stCCwithAwb[]  = {

  // ISP_A_LIGHT
  {
	0x78,	
	0,
	0,
	1,	
	{
		{1575, -503, -44},
		{-483, 1689, -183},
		{-483, -951, 2463},
	}
  },

  //ISP_TL84_LIGHT
  {
	0x78,	
	1,
	0,
	1,	
	{
		{1182, -136, -22},
		{-359, 1523, -136},
		{-359, -707, 2089},
	}
  },
  
  //ISP_D75_LIGHT
  {
	0x78,	
	2,
	0,
	1,	
	{
		{1575, -323, -223},
		{-287, 1423, -111},
		{-239, -867, 2129},
	}
  },

  
};


#endif
