/**
 * @filename akfha_char.c
 * @brief AK fha char device driver
 * Copyright (C) 2010 Anyka (Guangzhou) Software Technology Co., LTD
 * @author zhangshenglin
 * @date 2012-12-7
 * @version 1.0
 * @
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/mtd/partitions.h>
#include <linux/gpio.h>
#include <mtd/mtd-abi.h>
#include <mach-anyka/nand_list.h>
#include <mach-anyka/fha.h>
#include <mach-anyka/fha_asa.h>
#ifdef CONFIG_MTD_NAND_ANYKA
#include <plat-anyka/wrap_nand.h>
#include <plat-anyka/anyka_cpu.h>
#include <plat-anyka/nand.h>
#endif

#include <mach/gpio.h>
#include <mach/clock.h>
#include <linux/platform_device.h>


#define FHA_CHAR_MAJOR                 	168
#define AK_FHA_UPDATE_BOOT_BEGIN	0xb1
#define AK_FHA_UPDATE_BOOT		0xb2
#define AK_FHA_UPDATE_BIN_BEGIN		0xb3
#define AK_FHA_UPDATE_BIN		0xb4
#define AK_FHA_UPDATE_MAC	0xb5
#define AK_FHA_UPDATE_SER	0xb6
#define AK_FHA_UPDATE_BSER	0xb7

#define MAX_BUF_LEN 64*1024
#define MAC_FILE_NAME "MACADDR"
#define SN_FILE_NAME "SERADDR"
#define BSN_FILE_NAME "BCADDR"

extern int ak_fha_init_for_update(int n);
extern T_U32 ak_fha_read_callback(T_U32 chip_num, T_U32 page_num, T_U8 *data,
		T_U32 data_len, T_U8 *oob, T_U32 oob_len, T_U32 eDataType);

typedef struct
{
	long filelen;
	char filename[256];//the bin name in flash
}T_BinInfo;

typedef struct
{
	long buflen;
	char buff[MAX_BUF_LEN];
	long ddrparcnt;
	unsigned int ddrpar[64][2];
}T_BufInfo;
static T_BufInfo* pBufInfo = NULL;

typedef struct
{
	char Disk_Name;				//盘符名
	char bOpenZone;				//
    	char ProtectType;			//	
    	char ZoneType;				//
	unsigned int Size;
	unsigned int EnlargeSize;         // set this value if enlarge capacity,otherwise set 0 
    	unsigned int HideStartBlock;      //hide disk start
    	unsigned int FSType;
    	unsigned int resv[1];				
}__attribute__((packed)) T_PARTION_INFO;

#define MAX_PARTS_COUNT			26
static unsigned char* spipart_info = NULL;
#define SPI_PAGESIZE 256
static int akfha_char_open(struct inode *inode, struct file *filp);
static int akfha_char_close(struct inode *inode, struct file *filp);
static long akfha_char_ioctl(/*struct inode *inode, */struct file *filp, unsigned int cmd, unsigned long arg);

static struct akfha_char_dev
{
    struct cdev c_dev;
}fha_c_dev;

static int fha_c_major = FHA_CHAR_MAJOR;

static const struct file_operations akfha_char_fops =
{
    .owner   = THIS_MODULE,
    .open    = akfha_char_open,
    .release = akfha_char_close,
    .unlocked_ioctl   = akfha_char_ioctl,
};

static int akfha_char_open(struct inode *inode, struct file *filp)
{
	pBufInfo = kmalloc(sizeof(T_BufInfo), GFP_KERNEL);
	if(pBufInfo == NULL)
	{
		printk("malloc buf error\n");
		return -1;
	}
    filp->private_data = &fha_c_dev;
	ak_fha_init_for_update(1);
    return 0;
}

static int akfha_char_close(struct inode *inode, struct file *filp)
{
	if(pBufInfo)
	{
		kfree(pBufInfo);
		pBufInfo = NULL;
	}
	if(spipart_info != NULL)
	{
		if(AK_FALSE == FHA_set_fs_part(spipart_info, 256))
		{
			printk("FHA_set_fs_part error\n");
		}
		kfree(spipart_info);
		spipart_info = NULL;
	}

	FHA_close();
    return 0;
}

static int fhachar_UpdateBootBegin(unsigned long arg)
{
	return 0;
}
static int fhachar_UpdateBoot(unsigned long arg)
{
	int ret = 0;
	//int size;
	//unsigned long partition_count = 0;
	struct partitions* parts = NULL;
	unsigned long *ori_mbyte = NULL;
	unsigned char *part_info = NULL;
	//int i = 0;

	T_U8* page0 = NULL;
	T_U8* pData = NULL;

	T_U8 ecc[4];
	T_U8 pagedata[512];
	long newBootLen;
	int pagesize;

#ifdef CONFIG_MTD_NAND_ANYKA
	/* get the old part_info */
	size = sizeof(partition_count) + sizeof(*parts) * MAX_PARTS_COUNT +
			sizeof(*ori_mbyte) * MAX_PARTS_COUNT;
	part_info = kmalloc(size, GFP_KERNEL);
	if (part_info == NULL) {
		printk("malloc part_info error\n");
		ret = -1;
		goto go_out;
	}

	if (AK_FALSE == FHA_get_fs_part(part_info, size)){
		ret = -1;
		goto go_out;
	}		
	
	partition_count = *(unsigned long *)(part_info);
	
	parts = kmalloc(partition_count * sizeof(*parts), GFP_KERNEL);
	ori_mbyte = kmalloc(partition_count * sizeof(*ori_mbyte), GFP_KERNEL);
	
	memcpy(parts, part_info + sizeof(partition_count), partition_count * sizeof(*parts));
	memcpy(ori_mbyte, part_info + sizeof(partition_count) + partition_count * sizeof(*parts),
		partition_count * sizeof(*ori_mbyte));
	/* end get the old part_info */
#endif

	if(copy_from_user(pBufInfo, (T_BufInfo*)arg, sizeof(T_BufInfo)) != 0)
	{
		ret = -1;
		goto go_out;
	}

	memset(pagedata, 0, 512);

#ifdef CONFIG_MTD_NAND_ANYKA
	ak_fha_read_callback(0, 0, pagedata, 512, ecc, 4, FHA_DATA_BOOT);
	
	pagesize = pagedata[0xc] * 512;//BOOT页大小
#else	
	pagesize = SPI_PAGESIZE;//pagedata[0xc] * 512;//BOOT页大小	
#endif
	page0 = (T_U8*)kmalloc(pagesize, GFP_KERNEL);
	if(page0 == NULL)
	{
		ret = -1;
		goto go_out;
	}
	memset(page0, 0, pagesize);

#ifdef CONFIG_MTD_NAND_ANYKA
	ak_fha_read_callback(0, 0, page0, pagesize, ecc, 4, FHA_DATA_BOOT);

	page0[0xd] = pBufInfo->buflen / pagesize + 2;//修改nandboot所占的页数
#else
	ak_fha_read_callback(0, 0, page0, 1, ecc, 4, FHA_DATA_BOOT);
	ak_fha_read_callback(0, 1, pagedata, 1, ecc, 4, FHA_DATA_BOOT);
	*((unsigned int*)(page0+0x0c)) = pBufInfo->buflen;
#endif
	//ddr param
	if(pBufInfo->ddrparcnt > 0 && pBufInfo->ddrparcnt < 53)
	{
#ifdef CONFIG_MTD_NAND_ANYKA
		for(i = 0x2c; i < 472 - sizeof(T_NAND_PHY_INFO) - 4; i ++)
		{
			if(page0[i] == 'N' && page0[i + 1] == 'A' && page0[i + 2] == 'N' && page0[i + 3] == 'D')
			{
				printk("NAND found\n");
				break;
			}
		}

		if(i == 472 - sizeof(T_NAND_PHY_INFO) - 4)
		{
				ret = -1;
				goto go_out;
		}

		memmove(page0 + 0x2c + pBufInfo->ddrparcnt * 8, page0 + i, sizeof(T_NAND_PHY_INFO)+ 4);
		
		memcpy(page0 + 0x2c, pBufInfo->ddrpar, pBufInfo->ddrparcnt * sizeof(unsigned int) * 2);
#endif
	}
	newBootLen = pBufInfo->buflen;
	if(newBootLen < 0)
	{
		ret = -1;
		goto go_out;
	}

#ifdef CONFIG_MTD_NAND_ANYKA
	newBootLen += pagesize;
#endif
	//printk("New Boot len:%ld\n", newBootLen);
	
	pData = (T_U8*)kmalloc(newBootLen, GFP_KERNEL);
	if(pData == NULL)
	{
		printk("malloc error\n");
		ret = -1;
		goto go_out;
	}

#ifndef CONFIG_MTD_NAND_ANYKA
	memcpy(pData, pBufInfo->buff, pBufInfo->buflen);
#endif
	memcpy(pData, page0, pagesize);
#ifndef CONFIG_MTD_NAND_ANYKA
	memcpy(pData+pagesize, pagedata, pagesize);
#endif

//ddr param
	if(pBufInfo->ddrparcnt > 0 && pBufInfo->ddrparcnt < 53)
	{
#ifndef CONFIG_MTD_NAND_ANYKA
		memcpy(pData + 0x18, pBufInfo->ddrpar, pBufInfo->ddrparcnt * sizeof(unsigned int) * 2);
#endif
	}

#ifdef CONFIG_MTD_NAND_ANYKA
	memcpy(pData+pagesize, pBufInfo->buff, pBufInfo->buflen);
	
	ak_fha_read_callback(0, 1, pagedata, 512, ecc, 4, FHA_DATA_BOOT);
	memcpy(pData + pagesize, pagedata, 208);
#endif
#ifdef CONFIG_MTD_NAND_ANYKA
	if (FHA_FAIL == FHA_set_fs_part(part_info, sizeof(int) 
			+ partition_count * sizeof(parts[0]) + partition_count * sizeof(ori_mbyte[0])))
	{
		ret = -1;
		goto go_out;
	}
#endif
	if(FHA_write_boot_begin(newBootLen) == AK_TRUE)
	{
		printk("write boot begin ok\n");
	}	
	else
	{
		printk("write boot begin error\n");
		ret = -1;
		goto go_out;
	}
	
	if(FHA_write_boot(pData, newBootLen) == AK_TRUE)
	{
		printk("write boot ok\n");
	}
	else
	{
		printk("write boot error\n");
		ret = -1;
		goto go_out;
	}

go_out:
	if(page0)
	{
		kfree(page0);
		page0 = NULL;
	}	
	if(pData)
	{	
		kfree(pData);
		pData = NULL;
	}	
	if(parts)
	{	
		kfree(parts);
		parts = NULL;
	}
	
	if(part_info)
	{
		kfree(part_info);
		part_info = NULL;
	}
	if(ori_mbyte)
	{
		kfree(ori_mbyte);
		ori_mbyte = NULL;
	}

	printk("*********************end to update nandboot:ret=%d\n", ret);
	return ret;
}
static int fhachar_UpdateBinBegin(unsigned long arg)
{
	T_BinInfo binInfo;
	T_FHA_BIN_PARAM binParam;
	memset(&binParam, 0, sizeof(binParam));
	printk("e00000\n");
	if(copy_from_user(&binInfo, (T_BinInfo*)arg, sizeof(T_BinInfo)) != 0)
	{
			printk("e11111\n");
		return -1;
	}
	strcpy(binParam.file_name, binInfo.filename);
	if(FHA_read_bin_begin(&binParam) == FHA_FAIL)
	{
			printk("e22222\n");
		return -1;
	}

	binParam.data_length = binInfo.filelen;
	if(FHA_write_bin_begin(&binParam) == FHA_FAIL)
	{
			printk("e33333\n");
		return -1;
	}
	printk("e444444\n");
	return 0;
}

static int fhachar_UpdateBin(unsigned long arg)
{
	if(copy_from_user(pBufInfo, (T_BufInfo*)arg, sizeof(T_BufInfo)) != 0)
	{
		return -1;
	}

#ifndef CONFIG_MTD_NAND_ANYKA
	if(spipart_info == NULL)
	{
		spipart_info = kmalloc(SPI_PAGESIZE, GFP_KERNEL);
		if(AK_FALSE == FHA_get_fs_part(spipart_info, SPI_PAGESIZE))
		{
			printk("fha get fs part error\n");
		}
	}
#endif
	if(FHA_write_bin(pBufInfo->buff, pBufInfo->buflen) != FHA_SUCCESS)
	{
		return -1;
	}
	return 0;
}
static int UpdateAsaFile(unsigned long arg, char* filename)
{
	unsigned char file_buf[64] = {0};
	unsigned char file_len[4] = {0};
	unsigned char ttbuf[64];
	printk("filename:%s\n", filename);
	if(FHA_asa_read_file(filename, file_len, 4) == AK_FALSE)
	{
		printk("no %s file in asa!!\n", filename);
		return -1;
	}

	printk("file_len:0x%x 0x%x 0x%x 0x%x\n", file_len[0], file_len[1], file_len[2], file_len[3]);

	if(FHA_asa_read_file(filename, file_buf, *(unsigned long*)file_len + 4) == AK_FALSE)
	{
		printk("read %s file in asa error!!!\n", filename);
		return -1;
	}

	if(copy_from_user(pBufInfo, (T_BufInfo*)arg, sizeof(T_BufInfo)) != 0)
	{
		return -1;
	}

	printk("new file:%s\n", pBufInfo->buff);

	memset(ttbuf, 0, 64);
	ttbuf[0] = pBufInfo->buflen;
	memcpy(ttbuf + 4, pBufInfo->buff, pBufInfo->buflen);
	if(FHA_asa_write_file(filename, ttbuf, pBufInfo->buflen + 4, ASA_MODE_CREATE) == AK_FALSE)
	{
		printk("write %s file in asa error!!\n", filename);
		return -1;
	}

	return 0;
}
static int fhachar_UpdateMac(unsigned long arg)
{
	return UpdateAsaFile(arg, MAC_FILE_NAME);
}
static int fhachar_UpdateSer(unsigned long arg)
{
	return UpdateAsaFile(arg, SN_FILE_NAME);
}
static int fhachar_UpdateBSer(unsigned long arg)
{
	return UpdateAsaFile(arg, BSN_FILE_NAME);
}
/**
 * @brief :Command Control interface for burntool .
 * 
 * @author zhangshenglin
 * @date 2012-12-07
 * @param  inode[in] nand char device inode.
 * @param  filp[in] nand char file id.
 * @param  cmd[in] burntool command.
 * @param  arg[in] parameter from user.
 * @return int
 * @retval  1: fail   0: success
 */
static long akfha_char_ioctl(/*struct inode *inode, */struct file *filp,
		                     unsigned int cmd, unsigned long arg)
{
    	int ret = 0;
	switch(cmd)
	{
		case AK_FHA_UPDATE_BOOT_BEGIN:
		{
			ret = fhachar_UpdateBootBegin(arg);
			break;
		}
		case AK_FHA_UPDATE_BOOT:
		{
			ret = fhachar_UpdateBoot(arg);
			break;
		}
		case AK_FHA_UPDATE_BIN_BEGIN:
		{
			ret = fhachar_UpdateBinBegin(arg);
			break;
		}
		case AK_FHA_UPDATE_BIN:
		{
			ret = fhachar_UpdateBin(arg);
			break;
		}
		case AK_FHA_UPDATE_MAC:
		{
			ret = fhachar_UpdateMac(arg);
			break;
		}
		case AK_FHA_UPDATE_SER:
		{
			ret = fhachar_UpdateSer(arg);
			break;
		}
		case AK_FHA_UPDATE_BSER:
		{
			ret = fhachar_UpdateBSer(arg);
			break;
		}
		default:
		{
			return -EINVAL;
		}

	}
    	return ret;
}

static struct class *fha_class;

static void fha_setup_cdev(void)
{
    int err = 0;
    dev_t devno = MKDEV(fha_c_major, 0);

    cdev_init(&(fha_c_dev.c_dev), &akfha_char_fops);
    fha_c_dev.c_dev.owner = THIS_MODULE;
    fha_c_dev.c_dev.ops = &akfha_char_fops;
    err = cdev_add(&(fha_c_dev.c_dev), devno, 1);
    if(err)
    {
        printk(KERN_NOTICE "Error %d adding anyka akfha char dev\n", err);
    }
	
	//automatic mknod device node
	fha_class = class_create(THIS_MODULE, "akfha_class");
	device_create(fha_class, NULL, devno, &fha_c_dev, "akfha_char");
}
static int akfha_char_probe(struct platform_device *pdev)
{
	int result = 0;
	//int existent_chips;
	//int nr_sets;
	dev_t devno;

#ifdef CONFIG_MTD_NAND_ANYKA
	struct ak_platform_nand_multice *plat = pdev->dev.platform_data;
	struct ak_nand_set *sets;
	sets = (plat != NULL) ? plat->sets : NULL;
	nr_sets = (plat != NULL) ? plat->nr_sets : 1;
	ak_nand_clock(AK_TRUE);
#ifndef CONFIG_MTD_DOWNLOAD_MODE
    existent_chips = ak_nand_CE_set(sets->max_chips, sets->nr_chips, &sets->ce2_gpio, &sets->ce3_gpio);
	if (!existent_chips) {
        	printk("Can not find one chip\n");
        	return -1;
    }

	if(FHA_FAIL == ak_fha_init(sets->nr_chips))
	{
		printk(KERN_INFO "%s, %d init fha lib error return!!!\n", __func__, __LINE__);
		return -1;
	}	
#endif
#endif
	devno = MKDEV(fha_c_major, 0);
	if(fha_c_major)
	{
		result = register_chrdev_region(devno, 1, "anyka fha char dev");
	}
	else
	{
		result = alloc_chrdev_region(&devno, 0, 1, "anyka fha char dev");
	}
	if(result < 0)
	{
		return result;
	}
	fha_setup_cdev();
    
	printk(KERN_INFO "akfha Char Device Initialize Successed!\n");
	return 0;
}
static int akfha_char_remove(struct platform_device *pdev)
{
	dev_t devno = MKDEV(fha_c_major, 0);
	
	//destroy device node
	device_destroy(fha_class, devno);
	class_destroy(fha_class); 
	
	//delete char device
    cdev_del(&(fha_c_dev.c_dev));
   	unregister_chrdev_region(devno, 1);
	
	return 0;
}

/* device driver for platform bus bits */
static struct platform_driver akfha_char_driver = {
	.probe		= akfha_char_probe,
	.remove		= akfha_char_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "ak-fhachar",
		.pm	= NULL,	
	},
};
static int __init akfha_char_init(void)
{
	printk("*********akfha_char init\n");
	return platform_driver_register(&akfha_char_driver);
}
                      
static void __exit akfha_char_exit(void)
{
	platform_driver_unregister(&akfha_char_driver);
	printk("*******akfha_char_exit");
}
                      
subsys_initcall(akfha_char_init);
module_exit(akfha_char_exit);            
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ZhangShenglin");
MODULE_DESCRIPTION("Direct character-device access to fha devices");

