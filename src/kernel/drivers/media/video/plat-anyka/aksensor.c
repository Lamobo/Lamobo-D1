/*
 * ak sensor Driver
 *
 * Copyright (C) 2012 Anyka
 *
 * Based on anykaplatform driver,
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>

#include <media/soc_camera.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ctrls.h>

#include <plat-anyka/aksensor.h>
#include <mach/gpio.h>

//#define SENSOR_DEBUG
#ifdef SENSOR_DEBUG
#define sensor_dbg(fmt...) 			printk(KERN_INFO "Sensor: " fmt)
#else
#define sensor_dbg(fmt, args...) 	do{}while(0)
#endif 

#define SENDBG(fmt, args...) 		do{}while(0)

static struct sensor_info *cur_sensor_info; 
static const struct aksensor_color_format *cur_sensor_cfmts;

T_CAMERA_WORKMODE g_mode = CAMERA_WMODE_REC;
static struct i2c_client *g_client;

struct aksensor_priv {
	struct v4l2_subdev                	subdev;
	struct v4l2_ctrl_handler	  		hdl;	
	struct aksensor_camera_info        	*info;
	const struct aksensor_color_format 	*cfmt;
	struct aksensor_win_size     	win;
	int                               		model;
};

static struct sensor_info *sensor_info_array[SENSOR_MAX_SUPPORT];

/**
 * @brief: register sensor device
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *si: sensor_info structure, indicate sensor info
 */
int register_sensor(struct sensor_info *si)
{
	int i, ret;

	if (!si)
		return -1;

	if ((si->sensor_id <=0) || (si->handler == NULL))
		return -1;

	ret = -1;
	for (i = 0; i < SENSOR_MAX_SUPPORT; i++) {
		if (sensor_info_array[i] != NULL)
			continue;

		sensor_info_array[i] = si;
		ret = 0;
		SENDBG("register sensor(id=0x%x) successfully\n", si->sensor_id);
		break;
	}

	if (i == SENSOR_MAX_SUPPORT)
		SENDBG("register sensor(id=0x%x) failed!(no enough space)\n", si->sensor_id);

	return ret;
}
EXPORT_SYMBOL(register_sensor);

/**
 * @brief: get sensor device name
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] void: none
 */
const char *get_sensor_name(void)
{
	return cur_sensor_info->sensor_name;
}
EXPORT_SYMBOL(get_sensor_name);

/**
 * @brief camera probe pointer
 * @author dengzhou
 * @date 2012-03-16
 * @param
 * @return sensor_info * camera device pointer
 * @retval
 */ 
static struct sensor_info *probe_sensors(struct i2c_client *client)
{
	int i, read_id;
	
	for (i = 0; i < SENSOR_MAX_SUPPORT; i++)
	{
		if (sensor_info_array[i])
		{
			sensor_info_array[i]->handler->cam_open_func();
			read_id = sensor_info_array[i]->handler->cam_read_id_func();

			if (!(strcmp(sensor_info_array[i]->sensor_name, "hm1375")))
				read_id = read_id & 0x0fff;
			
			if ((read_id == sensor_info_array[i]->sensor_id)
				&& ((read_id != 0xffff) || (read_id != 0xff))) {
				
				dev_info(&client->dev, "Probing %s Sensor ID: 0x%x\n",
					sensor_info_array[i]->sensor_name, read_id);
				
				return sensor_info_array[i];
			} else {
                sensor_info_array[i]->handler->cam_close_func();
			}
		}
	}
   
	return NULL;
}

/**
 * @brief: write sensor register by i2c bus
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] daddr: sensor device address
 * @param [in] raddr: sensor device register(cmd)
 * @param [in] *data: pointer to data, the data writed to sensor register
 * @param [in] size: data number
 */
s32 aksensor_i2c_write_byte_short(u8 daddr, u16 raddr, u8 *data, u32 size)
{
	unsigned char msg[3];
	msg[0] = raddr >> 8;
	msg[1] = raddr & 0xff;
	msg[2] = *data;
	
//	printk("msg=0x%02x%02x, 0x%02x(write)\n", msg[0], msg[1], msg[2]);
	return i2c_master_send(g_client, msg, 3);
}

/**
 * @brief: read sensor register value by i2c bus
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] daddr: sensor device address
 * @param [in] raddr: sensor device register(cmd)
 */
s32 aksensor_i2c_read_byte_short(u8 daddr, u16 raddr)
{
	unsigned char msg[2];
	unsigned char data;

	g_client->addr = daddr/2;
	msg[0] = raddr >> 8;
	msg[1] = raddr & 0xff;

	i2c_master_send(g_client, msg, 2);

	i2c_master_recv(g_client, &data, 1);

	//	printk("msg=0x%02x%02x, 0x%02x(read)\n", msg[0], msg[1], data);

	return data;
}

/**
 * @brief: write sensor register by i2c bus
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] daddr: sensor device address
 * @param [in] raddr: sensor device register(cmd)
 * @param [in] *data: pointer to data, the data writed to sensor register
 * @param [in] size: data number
 */
s32 aksensor_i2c_write_word_data(u8 daddr, u16 raddr, u16 *data, u32 size)
{
	unsigned char msg[4];

	msg[0] = raddr >> 8;	//high 8bit first send
	msg[1] = raddr & 0xff;	//low 8bit second send
	msg[2] = *data & 0xff;	//low 8bit first send
	msg[3] = *data >> 8;	//high 8bit second send
	
	g_client->addr = daddr >> 1;
	
	//printk("(cmd): 0x%02x %02x, (data): %02x %02x\n", msg[0], msg[1], msg[2], msg[3]);
	return i2c_master_send(g_client, msg, 4);
}

/**
 * @brief: read sensor register value by i2c bus
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] daddr: sensor device address
 * @param [in] raddr: sensor device register(cmd)
 */
s32 aksensor_i2c_read_word_data(u8 daddr, u16 raddr)
{
	unsigned char msg[4];
	unsigned char buf[2];
	
	msg[0] = raddr >> 8;	//high 8bit first send
	msg[1] = raddr & 0xff;	//low 8bit second send
	
	g_client->addr = daddr >> 1;
	
	i2c_master_send(g_client, msg, 2);
	
	i2c_master_recv(g_client, buf, 2);

	return (buf[1] << 8)|buf[0];
}

/**
 * @brief: write sensor register by i2c bus
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] daddr: sensor device address
 * @param [in] raddr: sensor device register(cmd)
 * @param [in] *data: pointer to data, the data writed to sensor register
 * @param [in] size: data number
 */
s32 aksensor_i2c_write_byte_data(u8 daddr, u8 raddr, u8 *data, u32 size)
{
	return i2c_smbus_write_byte_data(g_client, raddr, *data);
}

/**
 * @brief: read sensor register value by i2c bus
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] daddr: sensor device address
 * @param [in] raddr: sensor device register(cmd)
 */
s32 aksensor_i2c_read_byte_data(u8 daddr, u8 raddr)
{
	g_client->addr = daddr/2;
	return i2c_smbus_read_byte_data(g_client,raddr);
}

/*****************************************/
static struct aksensor_priv *to_aksensor(const struct i2c_client *client)
{
	return container_of(i2c_get_clientdata(client), struct aksensor_priv, subdev);
}

/**
 * @brief: initial sensor device, open clock
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *sd: v4l2_subdev struct, v4l2 sub-device info
 * @param [in] val: none
 */
static int aksensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	SENDBG("entry %s\n", __func__);
	if (cur_sensor_info->handler != NULL)
	{
		cur_sensor_info->handler->cam_open_func();
		ret = cur_sensor_info->handler->cam_mclk;
		return ret;
	}

	return 0;
}

/**
 * @brief: read sensor register value by i2c bus
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *sd: v4l2_subdev struct, v4l2 sub-device info
 */
static int aksensor_loadfw(struct v4l2_subdev *sd)
{
	SENDBG("entry %s\n", __func__);
	if ((cur_sensor_info->handler != NULL) 
		&& (cur_sensor_info->handler->cam_init_func != NULL)) 
	{
		if (cur_sensor_info->handler->cam_init_func())
			return AK_TRUE;
	}
	return AK_FALSE;
}

/**
 * @brief: close sensor device, close clock
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *sd: v4l2_subdev struct, v4l2 sub-device info
 * @param [in] val: none
 */
static int aksensor_reset( struct v4l2_subdev *sd, u32 val )
{
//	struct i2c_client *client = v4l2_get_subdevdata(sd);
//	struct aksensor_priv *priv = i2c_get_clientdata(client);
	
	SENDBG("entry %s\n", __func__);

	//priv->win.width  = VGA_WIDTH;
	//priv->win.height = VGA_HEIGHT;
	
	if (NULL != cur_sensor_info->handler) {
		cur_sensor_info->handler->cam_close_func();
	}
	
	return 0;
}

static int aksensor_g_chip_ident(struct v4l2_subdev *sd,
			       struct v4l2_dbg_chip_ident *id)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct aksensor_priv *priv = to_aksensor(client);

	id->ident    = priv->model;
	id->revision = 0;

	return 0;
}

/**
 * @brief: query v4l2 sub-device ctroller function
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *sd: v4l2_subdev struct, v4l2 sub-device info
 * @param [in] *qc: v4l2_queryctrl struct
 */
static int aksensor_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int ret = 0;
	sensor_dbg("entry %s\n", __func__);
	//if (cur_sensor_info->ctrls)
	//	ret = cur_sensor_info->ctrls->ops->queryctrl(ctrl);
	return ret;
}
static int aksensor_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
//	struct v4l2_ctrl v4l2ctrl;
	
	sensor_dbg("entry %s\n", __func__);
#if 0
	if (cur_sensor_info->ctrls) {
		cur_sensor_info->ctrls->ops->g_volatile_ctrl(&v4l2ctrl);
		ctrl->id = v4l2ctrl.id;
		ctrl->val = v4l2ctrl.val;
	}
#endif
	return 0;
}

/**
 * @brief: query v4l2 sub-device ctroller function
 * 
 * @author: caolianming
 * @date: 2014-01-09
 * @param [in] *sd: v4l2_subdev struct, v4l2 sub-device info
 * @param [in] *ctrl: v4l2_control struct
 */
static int aksensor_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int ret = 0;
	struct v4l2_ctrl v4l2ctrl;
	
	sensor_dbg("entry %s  ctrl->id=%d, ctrl->value=%d\n", 
			__func__, ctrl->id, ctrl->value);

	if (cur_sensor_info->ctrls) {
		v4l2ctrl.id = ctrl->id;
		v4l2ctrl.val = ctrl->value;
		ret = cur_sensor_info->ctrls->ops->s_ctrl(&v4l2ctrl);
	}
	return ret;
}

static struct v4l2_subdev_core_ops aksensor_subdev_core_ops = {
	.init			= aksensor_init,
	.load_fw		= aksensor_loadfw,	
	.reset			= aksensor_reset,
	.g_chip_ident	= aksensor_g_chip_ident,
	.queryctrl		= aksensor_queryctrl,
	.g_ctrl			= aksensor_g_ctrl,
	.s_ctrl			= aksensor_s_ctrl,
};


/*
 * soc_camera_ops function
 */
static int aksensor_s_stream(struct v4l2_subdev *sd, int enable)
{
	SENDBG("entry %s\n", __func__);
	SENDBG("%s==>enable=%d\n", __func__, enable);
	SENDBG("leave %s\n", __func__);
	return 0;
}

static int aksensor_get_params(struct i2c_client *client,
			enum v4l2_mbus_pixelcode code)
{
	struct aksensor_priv *priv = to_aksensor(client);
	int ret = -EINVAL;
	int i;

	SENDBG("entry %s\n", __func__);
 
	/*
	 * select format
	 */
	priv->cfmt = NULL;
	for (i = 0; i < cur_sensor_info->num_formats; i++) {
		if (code == cur_sensor_cfmts[i].code) {
			priv->cfmt = cur_sensor_cfmts + i;
			break;
		}
	}
	if (!priv->cfmt)
		goto aksensor_set_fmt_error;
 
	return 0;

aksensor_set_fmt_error:
	priv->cfmt = NULL;
	return ret;
}

/* first called by soc_camera_prove to initialize icd->user_width... */
static int aksensor_g_fmt(struct v4l2_subdev *sd, 
			struct v4l2_mbus_framefmt *mf)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct aksensor_priv *priv = container_of(sd, struct aksensor_priv, subdev);
	int ret;

	SENDBG("entry %s\n", __func__);

	if (!priv->cfmt) {
		SENDBG("select VGA for first time\n");
		ret = aksensor_get_params(client, V4L2_MBUS_FMT_YUYV8_2X8);
		if (ret < 0)
			return ret;
	}

	mf->width	= priv->win.width;
	mf->height	= priv->win.height;
	mf->code	= priv->cfmt->code;
	mf->colorspace	= priv->cfmt->colorspace;
	mf->field		= V4L2_FIELD_NONE;

	return 0;
}


static int aksensor_try_fmt(struct v4l2_subdev *sd,
			  struct v4l2_mbus_framefmt *mf)
{
	SENDBG("entry %s\n", __func__);
	SENDBG("leave %s\n", __func__);
	
	return 0;
}

static int aksensor_s_fmt(struct v4l2_subdev *sd, 
			struct v4l2_mbus_framefmt *mf)
{
	struct aksensor_priv *priv = container_of(sd, struct aksensor_priv, subdev);
//	struct v4l2_pix_format *pix = &f->fmt.pix;

	T_BOOL bRet = AK_FALSE;
	int ret = -EINVAL;

	sensor_dbg("entry %s\n", __func__);

	//切换模式前必须设置一次mode
	if ( V4L2_BUF_TYPE_PRIVATE == mf->reserved[0])
	{
		g_mode = mf->reserved[1];
	}
	priv->win.width = mf->width;
	priv->win.height =mf->height;

	sensor_dbg("---%s. g_mode=%d mf->width=%d mf->height=%d\n", 
			__func__, g_mode, mf->width, mf->height);
	
	switch (g_mode)
	{
		case CAMERA_WMODE_PREV:
			if (cur_sensor_info->handler->cam_set_to_prev_func != NULL)
				bRet = cur_sensor_info->handler->cam_set_to_prev_func(mf->width, mf->height);
			break;
		case CAMERA_WMODE_CAP:
			if (cur_sensor_info->handler->cam_set_to_cap_func != NULL)
				bRet = cur_sensor_info->handler->cam_set_to_cap_func(mf->width, mf->height);
			break;
		case CAMERA_WMODE_REC:
			if (cur_sensor_info->handler->cam_set_to_record_func != NULL)
				bRet = cur_sensor_info->handler->cam_set_to_record_func(mf->width, mf->height);
			break;
		default :
			if (cur_sensor_info->handler->cam_set_to_record_func != NULL)
				bRet = cur_sensor_info->handler->cam_set_to_record_func(mf->width, mf->height);
			break;
	}

	if ( bRet )
		ret = 0;

	SENDBG("leave %s\n", __func__);
	return ret;
}



static int aksensor_cropcap(struct v4l2_subdev *sd, struct v4l2_cropcap *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct aksensor_priv *priv = to_aksensor(client);
 	int i;

	sensor_dbg("entry %s. priv=%p\n", __func__, priv);

	for (i = 0; i < cur_sensor_info->num_resolution; ++i) {
		if (!strcmp(cur_sensor_info->resolution[i].name, "720P")) 
			break;
	}
	// the resolution is 720P or larger
	if (i == cur_sensor_info->num_resolution)
		--i;
	a->bounds.width	= cur_sensor_info->resolution[i].width;
	a->bounds.height = cur_sensor_info->resolution[i].height;
	a->bounds.left			= 0;
	a->bounds.top			= 0;
		
	a->defrect.width		= priv->win.width;
	a->defrect.height		= priv->win.height;
	a->defrect.left			= 0;
	a->defrect.top			= 0;
	
	a->type					= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	a->pixelaspect.numerator	= 1;
	a->pixelaspect.denominator	= 1;

	sensor_dbg("%s.\n"
				"	a->bounds.width=%d, a->bounds.height=%d\n"
				"	a->defrect.width=%d, a->defrect.height=%d\n",
				__func__, 
				a->bounds.width, a->bounds.height,
				a->defrect.width, a->defrect.height);
	return 0;
}

static int aksensor_g_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct aksensor_priv *priv = to_aksensor(client);

	sensor_dbg("entry %s\n", __func__);

	a->c.left	= 0;
	a->c.top	= 0;
	a->c.width	= priv->win.width;
	a->c.height	= priv->win.height;
	
	return 0;
}

static int aksensor_s_crop(struct v4l2_subdev *sd, struct v4l2_crop *a)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct aksensor_priv *priv = to_aksensor(client);
	int ret = -EINVAL;
	T_BOOL bRet = AK_FALSE;
	
	sensor_dbg("entry %s\n", __func__);
	
	priv->win.width = a->c.width;
	priv->win.height =a->c.height;
		
	switch (g_mode)
	{
		case CAMERA_WMODE_PREV:
			if (cur_sensor_info->handler->cam_set_to_prev_func != NULL)
				bRet = cur_sensor_info->handler->cam_set_to_prev_func(a->c.width, a->c.height);
			break;
		case CAMERA_WMODE_CAP:
			if (cur_sensor_info->handler->cam_set_to_cap_func != NULL)
				bRet = cur_sensor_info->handler->cam_set_to_cap_func(a->c.width, a->c.height);
			break;
		case CAMERA_WMODE_REC:
			if (cur_sensor_info->handler->cam_set_to_record_func != NULL)
				bRet = cur_sensor_info->handler->cam_set_to_record_func(a->c.width, a->c.height);
			break;
		default :
			if (cur_sensor_info->handler->cam_set_to_record_func != NULL)
				bRet = cur_sensor_info->handler->cam_set_to_record_func(a->c.width, a->c.height);
			break;
	}

	if ( bRet )
		ret = 0;
	
	return ret;
}

static int aksensor_video_probe(struct i2c_client *client)
{
	struct aksensor_priv *priv = to_aksensor(client);
	const char         *devname;
		
	SENDBG("entry %s\n", __func__);

	/*
	 * check and show product ID and manufacturer ID
	 */
	g_client = client;
	if (cur_sensor_info != NULL) {
		dev_info(&client->dev, "Probing %s Sensor ID 0x%x\n",
				cur_sensor_info->sensor_name, 
				cur_sensor_info->sensor_id);
	} else {
		cur_sensor_info = probe_sensors(client);
		if (cur_sensor_info == NULL) {
			dev_err(&client->dev,  "Sensor ID error\n");
			return -ENODEV;	
		}
	}
	cur_sensor_cfmts = cur_sensor_info->formats;
	
	devname 	= "aksensor";
	priv->model = cur_sensor_info->sensor_id;

	return 0;
}

static int aksensor_enum_fmt(struct v4l2_subdev *sd, unsigned int index,
			   enum v4l2_mbus_pixelcode *code)
{
	if (index >= cur_sensor_info->num_formats)
		return -EINVAL;

	*code = cur_sensor_cfmts[index].code;
	return 0;
}

static int aksensor_g_mbus_config(struct v4l2_subdev *sd,
				struct v4l2_mbus_config *cfg)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct soc_camera_link *icl = soc_camera_i2c_to_link(client);

	SENDBG("entry %s\n", __func__);
	cfg->flags = V4L2_MBUS_PCLK_SAMPLE_RISING | V4L2_MBUS_MASTER |
		V4L2_MBUS_VSYNC_ACTIVE_HIGH | V4L2_MBUS_HSYNC_ACTIVE_HIGH |
		V4L2_MBUS_DATA_ACTIVE_HIGH;
	cfg->type = V4L2_MBUS_PARALLEL;
	cfg->flags = soc_camera_apply_board_flags(icl, cfg);
	SENDBG("leave %s\n", __func__);

	return 0;
}

static struct v4l2_subdev_video_ops aksensor_subdev_video_ops = {
	.s_stream	= aksensor_s_stream,
	.g_mbus_fmt	= aksensor_g_fmt,
	.s_mbus_fmt	= aksensor_s_fmt,
	.try_mbus_fmt	= aksensor_try_fmt,
	.cropcap	= aksensor_cropcap,
	.g_crop		= aksensor_g_crop,
	.s_crop		= aksensor_s_crop,
	.enum_mbus_fmt	= aksensor_enum_fmt,
	.g_mbus_config	= aksensor_g_mbus_config,
};

static struct v4l2_subdev_ops aksensor_subdev_ops = {
	.core	= &aksensor_subdev_core_ops,
	.video	= &aksensor_subdev_video_ops,
};

void aksensor_set_param(unsigned int cmd, unsigned int data)
{
	cur_sensor_info->handler->cam_set_sensor_param_func(cmd, data);
}
EXPORT_SYMBOL(aksensor_set_param);

unsigned short aksensor_get_param(unsigned int cmd)
{
	return cur_sensor_info->handler->cam_get_sensor_param_func(cmd);
}
EXPORT_SYMBOL(aksensor_get_param);

/*
 * i2c_driver function
 */
static int aksensor_probe(struct i2c_client *client,
			const struct i2c_device_id *did)
{
	struct aksensor_priv        *priv;
	struct soc_camera_link	*icl = soc_camera_i2c_to_link(client);
	struct i2c_adapter        *adapter = to_i2c_adapter(client->dev.parent);
	int i, ret;

	SENDBG("entry %s\n", __func__);

	if (!icl || !icl->priv) {
		dev_err(&client->dev, "AKSENSOR: missing platform data!\n");
		return -EINVAL;
	}

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&adapter->dev,
			"I2C-Adapter doesn't support "
			"I2C_FUNC_SMBUS_BYTE_DATA\n");
		return -EIO;
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		return -ENOMEM;
	}	

	priv->info = icl->priv;
	v4l2_i2c_subdev_init(&priv->subdev, client, &aksensor_subdev_ops);

	ret = aksensor_video_probe(client);
	
	if (ret) {
		kfree(priv);
		return ret;
	}

	v4l2_ctrl_handler_init(&priv->hdl, cur_sensor_info->nr_ctrls);
	for (i = 0; i < cur_sensor_info->nr_ctrls; i++)
		v4l2_ctrl_new_custom(&priv->hdl, &cur_sensor_info->ctrls[i], NULL);
	priv->subdev.ctrl_handler = &priv->hdl;
	if (priv->hdl.error) {
		int err = priv->hdl.error;
		v4l2_ctrl_handler_free(&priv->hdl);
		kfree(priv);
		return err;
	}

	// init sensor resolution, default VGA
	for (i = 0; i < cur_sensor_info->num_resolution; i++)
		if (!strcmp(cur_sensor_info->resolution[i].name, "VGA")) {
		priv->win.width = cur_sensor_info->resolution[i].width; 
		priv->win.height = cur_sensor_info->resolution[i].height;
	}
	sensor_dbg("%s: priv->win.width=%d priv->win.height=%d\n",
			__func__, priv->win.width, priv->win.height);
	return ret;
}

static int aksensor_remove(struct i2c_client *client)
{
	struct aksensor_priv *priv = to_aksensor(client);

	if (NULL != cur_sensor_info->handler) {
		cur_sensor_info->handler->cam_close_func();
		cur_sensor_info->handler = NULL; 
	}
	
	v4l2_device_unregister_subdev(&priv->subdev);
	v4l2_ctrl_handler_free(&priv->hdl);
	kfree(priv);
	return 0;
}

static const struct i2c_device_id aksensor_id[] = {
	{ "aksensor", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, aksensor);

static struct i2c_driver aksensor_i2c_driver = {
	.driver = {
		.name = "aksensor",
	},
	.probe    = aksensor_probe,
	.remove   = aksensor_remove,
	.id_table = aksensor_id,
};

/**
 * @brief get GPIO pin value
 * @author dengzhou
 * @date 2012-03-16
 * @param GPIO pin type
 * @return GPIO pin value
 * @retval
 */
T_U32 cam_getpin(T_CAMERA_PINTYPE pin_type)
{
	T_U32 pin = INVALID_GPIO;

//	SENDBG("entry %s\n", __func__);

	if (AK_NULL != g_client)
	{
		struct aksensor_priv *priv = to_aksensor(g_client);
		
		switch (pin_type)
		{
			case PIN_AVDD:
				pin = priv->info->pin_avdd;
				break;
			case PIN_POWER:
				pin = priv->info->pin_power;
				break;
			case PIN_RESET:
				pin = priv->info->pin_reset;
				break;
			default :
				break;
		}
	}
	
	return pin;
}

/*
 * module function
 */

static int __init aksensor_module_init(void)
{
	SENDBG("entry %s\n", __func__);

	return i2c_add_driver(&aksensor_i2c_driver);
}

static void __exit aksensor_module_exit(void)
{
	SENDBG("entry %s\n", __func__);

	i2c_del_driver(&aksensor_i2c_driver);
}

module_init(aksensor_module_init);
module_exit(aksensor_module_exit);

MODULE_DESCRIPTION("SoC Camera driver for aksensor");
MODULE_AUTHOR("dengzhou");
MODULE_LICENSE("GPL v2");
