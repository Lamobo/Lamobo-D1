#ifndef __FAT_FSCK_H__
#define __FAT_FSCK_H__

/*
 * sample of fat filesystem check and recovery tools.
 * base of bmptree reference to bmp_tree.c.
 *
 * author: lixinhai
 * date  : 2013-11-02
 *
 * Copyright (C) Anyka corporation, Inc. All rights reserved.
 * */


#include "bmp_tree.h"

enum fat_fsck_sel
{
	FAT_FSCK_UNSUPPORT = 0, /*unsupport the filesystem check.*/
	FAT_FSCK_BMPTREE, /*Base of bitmap tree.*/
	FAT_FSCK_NORMAL, /*use for RTOS Alg.*/
};

struct fat_chkdsk
{
	struct bmptree *bmptree;
	struct super_block *sb;
};

#define FAT32_MAX_USER_CLUSTER    0XFFFFFF8

static inline void show_fat_fsck_process(unsigned long total,
	   	unsigned long curr)
{
	if((curr % (total/50)) == 0)
		printk(".");
}

#if defined(CONFIG_FAT_FSCK)
static inline int support_fat_fsck(void)
{
	return FAT_FSCK_BMPTREE;
}

static inline int fat_fsck_check_cluster_status(struct fat_chkdsk *chkdsk, unsigned long cluster)
{
	return bmptree_check(chkdsk->bmptree, cluster);
}

/*
 * set cluster flags when search to cluster
 * */
static inline int fat_fsck_set_cluster(struct fat_chkdsk *chkdsk, int cluster)
{
	return bmptree_set(chkdsk->bmptree, cluster);
}

/*
 * filesystem check module initilize.
 * */
static inline int fat_fsck_init(struct super_block *sb, struct fat_chkdsk *chkdsk)
{
#if 0
	int err;
	err = bmptree_testmyself();
	printk("bmptree test myself %s.\n", err ? "fail":"success");
#endif

	chkdsk->bmptree = bmptree_init("fsck", chkdsk);
	chkdsk->sb = sb;
	return 0;
}

static inline void fat_fsck_release(struct fat_chkdsk *chkdsk)
{
	bmptree_release(chkdsk->bmptree);
}


#elif defined(CONFIG_FAT_CHK_DISK)
#include "chkdsk.h"
static inline int support_fat_fsck(void)
{
	return FAT_FSCK_NORMAL;
}

static inline int fat_fsck_set_cluster(struct fat_chkdsk *chkdsk, int cluster)
{
	return 0;
}

static inline int fat_fsck_init(struct super_block *sb, struct fat_chkdsk *chkdsk)
{
	Fat_ChkDsk(sb, NULL, NULL);
	return 1; /*check success, compatible for new alg*/
}

static inline void fat_fsck_release(struct fat_chkdsk *chkdsk)
{
}

#else /*FAT_NOT_CHKDSK*/
static inline int support_fat_fsck(void)
{
	return FAT_FSCK_UNSUPPORT;
}

static inline int fat_fsck_set_cluster(struct fat_chkdsk *chkdsk, int cluster)
{
	return 0;
}

static inline int fat_fsck_init(struct super_block *sb,
	   	struct fat_chkdsk *chkdsk)
{
	return -EINVAL;
}

static inline void fat_fsck_release(struct fat_chkdsk *chkdsk)
{
}

#endif  /*CONFIG_FAT_FSCK*/


#endif
