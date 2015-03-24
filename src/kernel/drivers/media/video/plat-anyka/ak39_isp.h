#ifndef _AK39_ISP_H
#define _AK39_ISP_H

#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>

#include <plat-anyka/isp_interface.h>

#define 	ISP_PERI_PARA 		0x00
#define 	ISP_FMT_CHK 		0x04
#define 	ISP_IMG_PARA 		0x08
#define		ISP_HISTO_CFG		0x10
#define 	ISP_IRQ_STATUS 		0x14
#define 	ISP_RGB_DATA 		0x18
#define 	ISP_ORDER_CTRL 		0x1c

#define 	ISP_FRAME_UPDATE		(0)
#define 	ISP_RAW_UPDMA_OVERFLOW	(1)
#define 	ISP_BIG_PIC_MODE_DONE	(2)
#define 	ISP_CH1_UPDMA_OVERFLOW	(3)
#define 	ISP_CH1_ONE_FRAME_END	(4)
#define 	ISP_PCLK_VER_ERR		(5)
#define 	ISP_PCLK_HOR_ERR		(6)
#define 	ISP_CH2_ONE_FRAME_END	(7)
#define 	ISP_HIST_END			(8)
#define 	ISP_RAW_UPLOAD			(9)
#define 	ISP_ALL_FARME_END		(10)


//dsamp rto + cut pos
#define		DSMP_RTO_BIT			30
#define		CUT_STRY_BIT			13
#define		CUT_STRX_BIT			0

//cut winsize							
#define		CUT_WINH_BIT			13
#define		CUT_WINW_BIT			0

//demosaic
#define DEMO_ENABLE_BIT 		   12	// demosaic enable
#define DEMO_THRD_BIT				0	// [11:0] demosaic threshold, asic presice+2

//lens coef, 0~9, 10times  aa[10:0],bb[10:0],cc[9:0]
#define LENS_COEFA_BIT			   21
#define LENS_COEFB_BIT			   10
#define LENS_COEFC_BIT				0	// c=[9:0]+1'b0

//lens range 0~9 10 times + black level parm / white balance parm
#define LENS_RANGE_BIT				0	// [21:0] 
#define BB_PARM_BIT 			   22	// [31:22] black level parm, R,G,B
#define WB_SETPARM_BIT			   22	// [31:22] white balance set parm, r/g/b_high/low,
#define RAW_DMA_CYL_BIT 		   28	// [31:28] raw DMA download min cycle, min val = 2

//lens refpos center point{3¡¯b0, yref[12:0], 3¡¯b0, xref[12:0]}
#define LENS_YREF_BIT			   16	// [28:16]
#define LENS_XREF_BIT				0	// [12:0] 

//lens istsqr
#define LENS_ENABLE_BIT 		   31	// lens correction enable
#define LENS_1STSQR_BIT 			0	// [26:0](xref)^2 + (yref) ^2

//WB calculate parm  
#define WB_PARMCAL_EN_BIT		   31	//white balance parameter calculate enable
#define WB_GRB_HIGH_BIT 		   20	// [29:20] grb_high
#define WB_GRR_LOW_BIT			   10	// [19:10] grb_low
#define WB_GR_LOW_BIT				0	// [9:0]   gr_low
#define WB_GR_HIGH_BIT			   20	// [29:20] gr_high 
#define WB_GB_HIGH_BIT			   10	// [19:10] gb_high
#define WB_GB_LOW_BIT				0	// [9:0]   gb_low 


#define		ISP_WIN_HEIGHT_OFFSET	(13)
#define		ISP_WIN_WIDTH_OFFSET	(0)

#define		UPSCALE_FRAME_CTRL		(16)
#define		DOWNSCALE_FRAME_CTRL	(24)

#define 	CC_CALVAL_BIT			16   // [28:16]   (tmp(I, j) ¨Cmin(I, j) ) /( high-low)
#define 	CC_TMPVAL_BIT			0   // [12:0] tmp(i, j) 

// Y gain edge enhance
#define 	Y_GAIN_BIT 				24	 // [31:24]
#define 	Y_EDGE_THRS_BIT			12	 // [22:12] ythrs * 9
#define 	Y_EDGE_K_BIT			0	 // [10:0] k/9*2048

//uv saturation1
#define UVSATUR_EN_BIT			   31  
#define ISOFLT_EN_BIT			   30
#define UVSATUR_K_LOW_BIT		   20	// [29:20] 10bit, xx.xxxx xxxx klow*256, 0.1~3
#define UVSATUR_K_HIGH_BIT		   10	// [19:10] 10bit, khigh*256, 0.1~3
#define UVSATUR_K_SLOPE_BIT 		0	// [9:0]  (iHigh - iLow) * 256 / (CHigh - CLow)

// uv saturation 2	
#define UVSATUR_CHIGH_BIT		   24	// [31:24] 0~255
#define UVSATUR_CLOW_BIT		   16	// [23:16] 0~255
#define SPEC_YCOEFA_BIT 			8	// [15:8] Y effect A -64~64
#define SPEC_YCOEFB_BIT 			0	// [7:0] B 0~255

// uv special effect
#define YUV_SOLAR_THRD_BIT         24   // [31:24] yuv_eff_solar_thd 
#define SPEC_UCOEFA_BIT 		   24	// [31:24] U effect -64~64
#define SPEC_UCOEFB_BIT 		   16	// [23:16] B 0~255
#define SPEC_VCOEFA_BIT 			8	// [15:8] V effect -64~64
#define SPEC_VCOEFB_BIT 			0	// [7:0] B 0~255


#define OSD_SIZE_H_BIT				0	  
#define OSD_SIZE_V_BIT				16

enum {
	ISP_BRIGHTNESS_0 = 0,
	ISP_BRIGHTNESS_1,
	ISP_BRIGHTNESS_2,
	ISP_BRIGHTNESS_3,
	ISP_BRIGHTNESS_4,
	ISP_BRIGHTNESS_5,
	ISP_BRIGHTNESS_6
};

enum {
	ISP_GAMMA_0 = 0,
	ISP_GAMMA_1,
	ISP_GAMMA_2,
	ISP_GAMMA_3,
	ISP_GAMMA_4,
	ISP_GAMMA_5,
	ISP_GAMMA_6
};

// isp saturation enum defined
enum {
	ISP_SATURATION_0 = 0,
	ISP_SATURATION_1,
	ISP_SATURATION_2,
	ISP_SATURATION_3,
	ISP_SATURATION_4,
	ISP_SATURATION_5,
	ISP_SATURATION_6
};

enum {
	ISP_SHARPNESS_0 = 0,
	ISP_SHARPNESS_1,
	ISP_SHARPNESS_2,
	ISP_SHARPNESS_3,
	ISP_SHARPNESS_4,
	ISP_SHARPNESS_5,
	ISP_SHARPNESS_6
};

enum {
	ISP_MANU_WB_0 = 0,
	ISP_MANU_WB_1,
	ISP_MANU_WB_2,
	ISP_MANU_WB_3,
	ISP_MANU_WB_4,
	ISP_MANU_WB_5,
	ISP_MANU_WB_6
};

//enum for sub sample
typedef enum
{
    SUBSMP_1X,  /*no sub sample*/
    SUBSMP_2X,  /*sub sample 1/2 * 1/2 */
    SUBSMP_4X,  /*sub sample 1/4 * 1/4 */
    SUBSMP_8X   /*sub sample 1/8 * 1/8 */ 
} T_SUBSMP_RTO;



typedef enum 
{
	ISP_A_LIGHT= 0,
	ISP_TL84_LIGHT,
	ISP_D75_LIGHT,
	ISP_DEFAULT_LIGHT
} T_AWB_MODE;


//define the image sensor controller register address
#undef REG32
#define REG32(_reg_)  (*(volatile unsigned long *)(_reg_))

struct isp_brightness_yedge { 
    unsigned short	yedgeK;
    unsigned short	ythresold;
    signed char		ygain;
};

struct color_correct_set
{
   unsigned int cc_thrs_low;
   unsigned int cc_thrs_high;
   int ccMtrx[3][3];
	
};
struct isp_wb_param {
	unsigned short	co_r;
	unsigned short	co_g;
	unsigned short	co_b;
};

#define CT_SET_NUM	3

struct awb_colortemp_set
{
	long channel_r_total;
	long channel_b_total;
	long channel_g_total;//4              

	int  time_cnt;

	unsigned int gr_low;              
    unsigned int gr_high;             
    unsigned int gb_low;              
    unsigned int gb_high;             
    unsigned int grb_low;             
    unsigned int grb_high;            
    //range of pixel 
    unsigned int r_low;
    unsigned int r_high;
    unsigned int g_low;
    unsigned int g_high;
    unsigned int b_low;
    unsigned int b_high;
};

enum isp_colortemp_mode {
	ISP_COLORTEMP_MODE_A = 0,
	ISP_COLORTEMP_MODE_TL84,
	ISP_COLORTEMP_MODE_D75,
	ISP_COLORTEMP_MODE_D50,
	ISP_COLORTEMP_MODE_DEFAULT,
	ISP_COLORTEMP_MODE_COUNT,
};

struct isp_awb_param {
	unsigned int frame_cnt;
	short current_r_gain;
	short current_b_gain;
	short current_g_gain;
	
	short target_r_gain;
	short target_b_gain;
	short target_g_gain;

	short auto_wb_step;
	short colortemp_set_num;
	short colortemp_set_seq;
	short current_colortemp_index;	
	struct awb_colortemp_set colortemp_set[ISP_COLORTEMP_MODE_COUNT+1];	
	struct color_correct_set color_correct_set[ISP_COLORTEMP_MODE_COUNT];
};

struct isp_aec_hist_feature
{
	unsigned int   mean_brightness;
	unsigned char   puppet_mid_brightness;
	unsigned char   average_feature_distance;
};

struct isp_aec_step
{
	unsigned int a_gain_updata_step;
	unsigned int exposure_time_updata_step;
	unsigned int laec_time_step;
};

struct isp_para_with_aec
{
   unsigned int  rgb_filter_thres[8];    //filter threshold
   unsigned int  demo_sac_thres[8];      //demosaic threshold
   unsigned int  y_edgek[8];
   unsigned int  y_thrs[8];         
};


struct isp_aec_param
{
    int current_exposure_time;
	 int current_laec_time;
	int current_a_gain;
	int current_d_gain;
	 int exposure_time_max;
	 int exposure_time_min;
	//unsigned int target_time;
	unsigned int  target_lumiance;
	unsigned int  exposure_step;
	unsigned int  stable_range;
	int  a_gain_min;
	int  a_gain_max;
	int  d_gain_min;
	int  d_gain_max;
	unsigned int  aec_status;
	unsigned int  aec_locked;	

	int bByPassAE;
	int bLinkage;
	int bCompensation;
	int bGama;
	
	struct isp_aec_step aec_step;
	struct isp_aec_hist_feature hist_feature;
	struct isp_para_with_aec para_with_aec;
	
};



struct isp_struct {
	void __iomem	*base;
	enum isp_working_mode cur_mode;
	enum isp_mode_class cur_mode_class;

	/*the input size from sensor*/
	int fmt_width;
	int fmt_height;

	/* indicate VGA size */
	int fmt_def_width;
	int fmt_def_height;

	/*the input size from sensor*/
	int cut_width;
	int cut_height;

	/* master channel output size*/
	int chl1_width;
	int chl1_height;

	/* minor channel */
	int chl2_width;	
	int chl2_height;
	int chl2_enable;

	int lens_use_width;			// used for lens correct, width
	int lens_use_height;		// used for lens correct, height

	int demo_sac_en;
	unsigned int demo_sac_thres;
	int rgb_filter_en;
	int rgb_filter_en_flag;
	unsigned int rgb_filter_thres;
	
	int yedge_en;
	int sharpness_en;
	int uv_isoflt_en;
	int uv_isoflt_en_flag;
	int gamma_calc_en;
	int uv_saturate_en;
	int auto_wb_param_en;
	int wb_param_en;
	int color_crr_en;
	int yuv_effect_en;
	int yuv_solar_en;
	int defect_pixel_en;
	int blkb_en;				// black balance enable flag
	int lens_crr_en;
	int histo_en;
	int fog_en;
	int fog_thrs;
	
	T_SUBSMP_RTO sub_sample;             //0:1x,1:2x,2:4x,3:8x
	struct isp_wb_param wb_param;
	struct isp_awb_param awb_param;
	struct isp_aec_param aec_param;
	unsigned char	*area;			/* virtual pointer */
	dma_addr_t		addr;			/* physical address */
	size_t 			bytes;			/* buffer size in bytes */
	unsigned long		*isp_ctrltbl;		/* ISP control Information */
	unsigned long		*img_ctrltbl1;	/* Image backend process control information 1 */
	unsigned long		*img_lumitbl;	/* Image luminosity transformation table */
	unsigned long 		*osd_chktbl;		/* OSD color check table */
	unsigned long 		*img_ctrltbl2;	/* Image backend process control information 2 */
	dma_addr_t		histo_phyaddr;
	unsigned char 	*histo_base;

	int rfled_ison;
	struct delayed_work awb_work;	
	struct delayed_work ae_work;	
};

void isp_start_capturing(struct isp_struct *isp);

void isp_stop_capturing(struct isp_struct *isp);

static inline void isp_clear_irq_counter(struct isp_struct *isp)
{
	REG32(isp->base + ISP_IRQ_STATUS) |= 1 << 26;
}

static inline bool isp_capture_running(struct isp_struct *isp)
{
	return REG32(isp->base + ISP_PERI_PARA) & (1 << 31);
}

static inline bool isp_is_continuous(struct isp_struct *isp)
{
	return REG32(isp->base + ISP_PERI_PARA) & (1 << 2);
}

static inline bool isp_is_capturing_odd(struct isp_struct *isp)
{
	return REG32(isp->base + ISP_IRQ_STATUS) & (1 << 27);
}


void isp_update_regtable(struct isp_struct *isp, int force);
int	update_cur_mode_class(struct isp_struct *isp);

void isp_set_even_frame(struct isp_struct *isp, 
				unsigned long yaddr_chl1, unsigned long yaddr_chl2);
void isp_set_odd_frame(struct isp_struct *isp, 
				unsigned long yaddr_chl1, unsigned long yaddr_chl2);

int isp_check_irq(struct isp_struct *isp);
int isp_clear_irq(struct isp_struct *isp);
int isp_clear_irq_status(struct isp_struct *isp);

int isp_set_osd(struct isp_struct *isp, struct isp_osd_info *);
int isp_set_channel2(struct isp_struct *isp, struct isp_channel2_info *);
int isp_set_occlusion_area(struct isp_struct *isp, struct isp_occlusion_info *);
int isp_set_occlusion_color(struct isp_struct *isp, struct isp_occlusion_color *);
int isp_set_channel1_scale(struct isp_struct *isp, int width, int height);
int isp_set_cutter_window(struct isp_struct *isp, int xpos, int ypos, int width, int height);
int isp_set_zoom(struct isp_struct *isp, struct isp_zoom_info *info);
int isp_set_crop(struct isp_struct *isp, struct v4l2_rect rect);
int isp_set_brightness(struct isp_struct *isp, struct v4l2_ctrl *ctrl);
int isp_set_gamma(struct isp_struct *isp, struct v4l2_ctrl *ctrl);
int isp_set_saturation(struct isp_struct *isp, struct v4l2_ctrl *ctrl);
int isp_set_sharpness(struct isp_struct *isp, struct v4l2_ctrl *ctrl);
int isp_set_uspecial_effect(struct isp_struct *isp, struct v4l2_ctrl *ctrl, int isp_wb);
int isp_set_ucolor_correct(struct isp_struct *isp, struct v4l2_ctrl *ctrl);
int isp_update_auto_wb_param(struct isp_struct *isp);
int isp_auto_set_wb_param(struct isp_struct *isp, struct v4l2_ctrl *ctrl);
int isp_manu_set_wb_param(struct isp_struct *isp, struct v4l2_ctrl *ctrl, int isp_wb);
int isp_update_auto_exposure_param(struct isp_struct *isp);


/* *******this interface response to set image param ****** */
int isp_set_black_balance(struct isp_struct *isp, struct isp_black_balance *ctrl);
int isp_set_lens_correct(struct isp_struct *isp, struct isp_lens_correct *ctrl);
int isp_set_demosaic(struct isp_struct *isp, struct isp_demosaic *ctrl);
int isp_set_rgb_filter(struct isp_struct *isp, struct isp_rgb_filter *ctrl);
int isp_set_uv_iso_filter(struct isp_struct *isp, struct isp_uv_filter *ctrl);
int isp_set_defect_pixel(struct isp_struct *isp, struct isp_defect_pixel *ctrl);
int isp_set_manu_wb(struct isp_struct *isp, struct isp_white_balance *ctrl);
int isp_set_auto_wb(struct isp_struct *isp, struct isp_auto_white_balance *ctrl);
int isp_set_color_correct(struct isp_struct *isp, struct isp_color_correct *ctrl);
int isp_set_gamma_calc(struct isp_struct *isp, struct isp_gamma_calculate *ctrl);
int isp_set_brightness_enhance(struct isp_struct *isp, struct isp_brightness_enhance *ctrl);
int isp_set_uv_saturation(struct isp_struct *isp, struct isp_saturation *ctrl);
int isp_set_histogram(struct isp_struct *isp, struct isp_histogram *ctrl);
int isp_get_histogram(struct isp_struct *isp);
int isp_set_special_effect(struct isp_struct *isp, struct isp_special_effect *ctrl);
int isp_remove_fog(struct isp_struct *isp, struct isp_image_fog *ctrl);

int isp_apply_mode(struct isp_struct *isp);
void isp_start_capturing(struct isp_struct *isp);
void isp_dump_register(struct isp_struct *isp);

int isp_module_init(struct isp_struct *isp);
void isp_module_fini(struct isp_struct *isp);

int isp_set_histogram(struct isp_struct *isp, struct isp_histogram *ctrl);
int isp_set_ae_attr (struct isp_struct *isp, struct isp_ae_attr *ctrl) ;
int isp_set_cc_with_awb(struct isp_struct *isp, struct isp_color_correct_awb *ctrl);

#endif
