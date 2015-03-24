#ifndef __FC_BMP_H__
#define __FC_BMP_H__
/*
 * Free cluster bitmap
 * use for bitmap tree record the cluster allocation status.
 * base of bmptree.
 *
 * author: lixinhai
 * date  : 2013-11-02
 *
 * Copyright (C) Anyka corporation, Inc. All rights reserved.
 * */

#include "bmp_tree.h"
#include <linux/vfs.h>

struct fat_fcbmp 
{
	struct bmptree *bmptree;
	struct super_block *sb;
};

#if defined(CONFIG_FAT_FCBMP)

/*
 *fast of find next cluster in bitmap tree.
 * */
static inline int fcbmp_find_next_free_cluster(struct fat_fcbmp *fcbmp, 
		unsigned long max_cluster, int start)
{
	return bmptree_find_next_free_pos(fcbmp->bmptree, start, FAT_START_ENT, max_cluster);
}


static inline int fcbmp_check_cluster_status(struct fat_fcbmp *fcbmp, unsigned long cluster)
{
	return bmptree_check(fcbmp->bmptree, cluster);
}

/*
 * set cluster flags when alloc the cluster.
 * */
static inline int fcbmp_set_cluster(struct fat_fcbmp *fcbmp, int cluster)
{
	return bmptree_set(fcbmp->bmptree, cluster);
}

/*
 * clear cluster flags when free the cluster.
 * */
static inline int fcbmp_clear_cluster(struct fat_fcbmp *fcbmp, int cluster)
{
	return bmptree_clear(fcbmp->bmptree, cluster);
}

/*
 * fat free cluster bitmap initialize.
 * */
static inline int fcbmp_init(struct super_block *sb, struct fat_fcbmp *fcbmp)
{
#if 0
	int err;
	err = bmptree_testmyself();
	printk("bmptree test myself %s.\n", err ? "fail":"success");
#endif
	
	fcbmp->bmptree = bmptree_init("fcbmp", fcbmp);
	fcbmp->sb = sb;
	return 0;
}

static inline void fcbmp_release(struct fat_fcbmp *fcbmp)
{
	bmptree_release(fcbmp->bmptree);
}

static inline int support_fcbmp(void)
{
	return 1;
}

#else
static inline int fcbmp_find_next_free_cluster(struct fat_fcbmp *fcbmp, 
		unsigned long max_cluster, int start)
{
	return 0;
}

static inline int fcbmp_check_cluster_status(struct fat_fcbmp *fcbmp, unsigned long cluster)
{
	return 0;
}

static inline int fcbmp_set_cluster(struct fat_fcbmp *fcbmp, int cluster)
{
	return 0;
}
static inline int fcbmp_clear_cluster(struct fat_fcbmp *fcbmp, int cluster)
{
	return 0;
}

static inline int fcbmp_init(struct super_block *sb, struct fat_fcbmp *fcbmp)
{
	return 0;
}

static inline void fcbmp_release(struct fat_fcbmp *fcbmp)
{
}

static inline int support_fcbmp(void)
{
	return 0;
}
#endif

#endif
