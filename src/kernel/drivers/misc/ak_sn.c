#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#include <mach-anyka/anyka_types.h>
#include <mach-anyka/fha_asa.h>

#define BAR_CODE_FILE_NAME "BCADDR"
#define SN_FILE_NAME	"SERADDR"
#define SN_MAX_LEN	64

static ssize_t sn_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	unsigned char sn[SN_MAX_LEN] = {0};
	unsigned char file_len[4] = {0};

	if (FHA_asa_read_file(SN_FILE_NAME, file_len, 4) == AK_FALSE) {
		goto out;
	}

	if (FHA_asa_read_file(SN_FILE_NAME, sn, *(unsigned long*)file_len + 4) == AK_FALSE) {
		memset(sn, 0, SN_MAX_LEN);
		goto out;
	}
out:
	return sprintf(buf, "%s\n", sn + 4);
}

static ssize_t barcode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	unsigned char sn[SN_MAX_LEN] = {0};
	unsigned char file_len[4] = {0};

	if (FHA_asa_read_file(BAR_CODE_FILE_NAME, file_len, 4) == AK_FALSE) {
		goto out;
	}

	if (FHA_asa_read_file(BAR_CODE_FILE_NAME, sn, *(unsigned long*)file_len + 4) == AK_FALSE) {
		memset(sn, 0, SN_MAX_LEN);
		goto out;
	}
out:
	return sprintf(buf, "%s\n", sn + 4);
}


static struct kobj_attribute sn_attribute = 
	__ATTR(sn, 0666, sn_show, NULL);

static struct kobj_attribute barcode_attribute = 
	__ATTR(barcode, 0666, barcode_show, NULL);

static struct attribute *attrs[] = {
	&sn_attribute.attr,
	NULL
};

static struct attribute *attrsbarcode[] = {
	&barcode_attribute.attr,
	NULL
};


static struct kobject *sn_kobj;

static int __init serial_number_init(void)
{
	int ret;

	sn_kobj = kobject_create_and_add("serial_number", kernel_kobj);
	if (!sn_kobj) {
		printk("Create serial number kobject failed\n");
		return -ENOMEM;
	}

	ret = sysfs_create_file(sn_kobj, *attrs);
	if (ret) {
		printk("Create serial number sysfs file failed\n"); 
		kobject_put(sn_kobj);
	}

	ret = sysfs_create_file(sn_kobj, *attrsbarcode);
	if (ret) {
		printk("Create barcode sysfs file failed\n"); 
		kobject_put(sn_kobj);
	}

	return ret;
}

static void __exit serial_number_exit(void)
{
	kobject_put(sn_kobj);
}

module_init(serial_number_init);
module_exit(serial_number_exit);

MODULE_DESCRIPTION("Anyka Device Serial Number Interface");
MODULE_AUTHOR("Anyka");
MODULE_LICENSE("GPL");
