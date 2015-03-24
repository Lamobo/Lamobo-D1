/*
 * @(#)chkdsk.c
 * @date 2010/07/10
 * @version 1.0
 * @author AiJun.
 * @Leader:xuchuang
 * @modify: lixinhai
 * Copyright 2015 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
    
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include "fat_fsck.h"

//#define FAT_CHKDSK_DBG
#ifdef FAT_CHKDSK_DBG
#define PDEBUG(fmt, args...) 	printk(KERN_INFO "chkdsk:" fmt, ##args)
#else
#define PDEBUG(fmt, args...) 
#endif

#ifdef FAT_CHKDSK_DBG 
void dump_data(const unsigned long *addr, unsigned long size) 
{
	const unsigned long *p = addr;
	int i = 0;

	while (size--) {
		printk("%08lx ", *(p++));
		if((++i)%8 == 0)
			printk("\n");
	}
	printk("\n");
}
#else
void dump_data(const unsigned long *addr, unsigned long size) 
{
}
#endif



#define MAX_CHKDSK_RAM_NUMBER   10     // chkdsk use the max big buffer number

/************************************************************************
 * NAME:        FAT_GetFatLinkInfo
 * FUNCTION  it will get the fat link value in fat table with offset and fstype
 * PARAM:      T_U8 * pFatBuf
                       T_U16 offset
                       T_U8 FSType
 * RETURN:     NULL
**************************************************************************/
T_U32 FAT_GetFatLinkInfo_chkdsk(struct msdos_sb_info *sbi, T_U8 * pFatBuf, T_U16 offset)
{
    T_U32 ret;
    
    if(sbi->fat_bits == 16)
    {
        ret = (((T_U16 *) pFatBuf)[offset]);
    }
    else
    {
        ret = (((T_U32 *) pFatBuf)[offset]);
    }
    return ret;
}


T_U32 FAT_GetFatLinkInfo(struct msdos_sb_info *sbi, T_U8 * pFatBuf, T_U16 offset)
{
    T_U32 ret;
    
    if(sbi->fat_bits == 16)
    {
        ret = (((T_U16 *) pFatBuf)[offset]);
        if(ret >= FAT16_MAX_USER_CLUSTER)
            ret = FAT32_MAX_USER_CLUSTER;
    }
    else
    {
        ret = (((T_U32 *) pFatBuf)[offset]);
    }
    return ret;
}


T_VOID FAT_SetFatLinkInfo(struct msdos_sb_info *sbi, T_U8 * pFatBuf, T_U16 offset, T_U32 newValue)
{
    if(sbi->fat_bits == 16)
    {
        (((T_U16 *) pFatBuf)[offset]) = (T_U16)newValue;
    }
    else
    {
        (((T_U32 *) pFatBuf)[offset]) = newValue;
    }
}

/************************************************************************
 * NAME:        FAT_MallocChkDskBuf
 * FUNCTION  it will malloc some chkdsk buffer, and add it to the link
 * PARAM:      PSECTOR_MAP_ARRAY_INFO pSectorMapBuf, 
                       T_U16 SectorSize
 * RETURN:     the new buffer ptr
**************************************************************************/

PSECTOR_MAP_ARRAY_INFO FAT_MallocChkDskBuf(PSECTOR_MAP_ARRAY_INFO pSectorMapBuf, T_U16 SectorSize)
{
    PSECTOR_MAP_ARRAY_INFO next;
    T_U16 i, BufSize, BytesPerClusMap;
    
    BytesPerClusMap = (SectorSize >> 3);
    BufSize = sizeof(SECTOR_MAP_ARRAY_INFO) + CLUSTER_MAP_NUMBER * (sizeof(CLUSTER_MAP_INFO) + BytesPerClusMap);
    next = kmalloc(BufSize, GFP_KERNEL);
    if(next == AK_NULL)
        return AK_NULL;
    
    next->pMapArray = (PCLUSTER_MAP_INFO)&next[1];
    next->next = AK_NULL;
    next->pCurSecMap = AK_NULL;
    for(i = 0; i < CLUSTER_MAP_NUMBER; i++)
    {
        next->pMapArray[i].sectorAddr = T_U32_MAX;
        next->pMapArray[i].bitNum = 0;
        next->pMapArray[i].bitMap = (T_U8 *)(&next->pMapArray[CLUSTER_MAP_NUMBER]) + i * BytesPerClusMap;
		memset(next->pMapArray[i].bitMap, 0, BytesPerClusMap);
    }
    if(pSectorMapBuf == AK_NULL)
        return next;
    while(pSectorMapBuf->next)
    {
        pSectorMapBuf = pSectorMapBuf->next;
    }
    pSectorMapBuf->next = next;
    return next;
}


/************************************************************************
 * NAME:        FAT_CheckChkDskRamNumber
 * FUNCTION  it will check whether system can malloc more ram
 * PARAM:      PSECTOR_MAP_ARRAY_INFO pSectorMapBuf, 
 * RETURN:     return ak_true if system can
**************************************************************************/

T_BOOL FAT_CheckChkDskRamNumber(PSECTOR_MAP_ARRAY_INFO pSectorMapBuf)
{
    PSECTOR_MAP_ARRAY_INFO cur = pSectorMapBuf;
    T_U32 RamNum = 0;

    while(cur->next)
    {
        RamNum ++;
        cur = cur->next;
    }
    return RamNum < MAX_CHKDSK_RAM_NUMBER;
}

/************************************************************************
 * NAME:        FAT_FIndChkDskClusMap
 * FUNCTION  it will search a empty cluster map ram
 * PARAM:      T_U8 *SectorBitMap
                      PSECTOR_MAP_ARRAY_INFO pSectorMapBuf
                      PSECTOR_MAP_ARRAY_INFO *pFindPos
 * RETURN:     return the find item
**************************************************************************/

PCLUSTER_MAP_INFO FAT_FIndChkDskClusMap(T_U8 *SectorBitMap, PSECTOR_MAP_ARRAY_INFO pSectorMapBuf, PSECTOR_MAP_ARRAY_INFO *pFindPos)
{
    PCLUSTER_MAP_INFO pMaxClusMap = AK_NULL;
    PSECTOR_MAP_ARRAY_INFO pCurMap = pSectorMapBuf;
    T_U32 i, MaxBitNum = 0;
    
    while(pCurMap != AK_NULL)
    {
        for(i = 0; i < CLUSTER_MAP_NUMBER; i++)
        {
            if(pCurMap->pMapArray[i].sectorAddr == T_U32_MAX)
            {
                *pFindPos = pCurMap;
                return pCurMap->pMapArray + i;
            }
            
            if(pCurMap->pMapArray[i].bitNum > MaxBitNum)
            {
                *pFindPos = pCurMap;
                pMaxClusMap = pCurMap->pMapArray + i;
                MaxBitNum = pCurMap->pMapArray[i].bitNum;            
            }
        }
        pCurMap = pCurMap->next;
    }
    SetBitMap(SectorBitMap, pMaxClusMap->sectorAddr);
    return pMaxClusMap;
}

/************************************************************************
 * NAME:        FAT_AddMarkFatItemToBuf
 * FUNCTION  it will search a empty cluster map ram
 * PARAM:      T_U8 *SectorBitMap
                      PSECTOR_MAP_ARRAY_INFO pSectorMapBuf
                      T_U32 MarkItem
                      T_U16 ClusPerSector
 * RETURN:     return the find item
**************************************************************************/

T_U8 FAT_AddMarkFatItemToBuf(T_U8 *SectorBitMap, PSECTOR_MAP_ARRAY_INFO pSectorMapBuf, T_U32 MarkItem, T_U16 ClusPerSector)
{
    T_U16 i;
    T_U32 SectorAddr, SectorOff;
    PSECTOR_MAP_ARRAY_INFO pCurMap = pSectorMapBuf;
    PCLUSTER_MAP_INFO pFreeClusMap = AK_NULL;
    
    SectorAddr = MarkItem / ClusPerSector;
    SectorOff = MarkItem % ClusPerSector;
    if(TestBitMap(SectorBitMap, SectorAddr))
    {
        return FAT_LINK_ERROR;
    }
    if(pCurMap->pCurSecMap && (pCurMap->pCurSecMap->pMapArray[pCurMap->index].sectorAddr == SectorAddr))
    {    
        i = (T_U16)pSectorMapBuf->index;
        pCurMap = pCurMap->pCurSecMap;
    }
    else
    {
        while(pCurMap != AK_NULL)
        {
            for(i = 0; i < CLUSTER_MAP_NUMBER; i++)
            {
                if(pFreeClusMap == AK_NULL)
                {
                    if(pCurMap->pMapArray[i].sectorAddr == T_U32_MAX)
                        pFreeClusMap = &pCurMap->pMapArray[i];
                }
                if(pCurMap->pMapArray[i].sectorAddr == SectorAddr)
                    break;
            }
            if(i != CLUSTER_MAP_NUMBER)
                break;
            pCurMap = pCurMap->next;
        }
    }

    if(pCurMap != AK_NULL)
    {
        pSectorMapBuf->pCurSecMap = pCurMap;
        pSectorMapBuf->index = i;
        if(TestBitMap(pCurMap->pMapArray[i].bitMap, SectorOff))
        { 
            return FAT_LINK_ERROR;
        }
        else
        {
            SetBitMap(pCurMap->pMapArray[i].bitMap, SectorOff);
            pCurMap->pMapArray[i].bitNum ++ ;
            if(ClusPerSector == pCurMap->pMapArray[i].bitNum)
            {
                // we will free the small buffer
                pSectorMapBuf->pCurSecMap = AK_NULL;
                SetBitMap(SectorBitMap, SectorAddr);
                pCurMap->pMapArray[i].sectorAddr = T_U32_MAX;
                pCurMap->pMapArray[i].bitNum = 0;
                if(pCurMap != pSectorMapBuf)
                {
                    // we will check if the big buffer need free
                    for(i = 0; i < CLUSTER_MAP_NUMBER; i++)
                    {
                        if(pCurMap->pMapArray[i].sectorAddr != T_U32_MAX)
                            break;
                    }
                    if(i == CLUSTER_MAP_NUMBER)
                    {
                        while(pSectorMapBuf->next != pCurMap)
                            pSectorMapBuf = pSectorMapBuf->next;
                        pSectorMapBuf->next = pCurMap->next;
                        kfree(pCurMap);
                    }
                }
            }
        }
    }
    else
    {
        if(pFreeClusMap == AK_NULL)
        {
            if(FAT_CheckChkDskRamNumber(pSectorMapBuf) == AK_TRUE)
            {
                pCurMap = FAT_MallocChkDskBuf(pSectorMapBuf,ClusPerSector);
                if(pCurMap == AK_NULL)
                    return MARK_MALLOC_ERROR;
                
                pFreeClusMap = &pCurMap->pMapArray[0];
            }
            else
            {
                pFreeClusMap = FAT_FIndChkDskClusMap(SectorBitMap, pSectorMapBuf, &pCurMap);
            }
        }
        pSectorMapBuf->pCurSecMap = pCurMap;
        pSectorMapBuf->index = 0;
        pFreeClusMap->sectorAddr = SectorAddr;
        pFreeClusMap->bitNum = 1;
        memset(pFreeClusMap->bitMap, 0, ClusPerSector >> 3);
        SetBitMap(pFreeClusMap->bitMap, SectorOff);
    }
    return MARK_FAT_OK;
}

/************************************************************************
 * NAME:        FAT_FreeChkDskBuf
 * FUNCTION  it will free all buffer with chkdsk
 * PARAM:      PSECTOR_MAP_ARRAY_INFO pSectorMapBuf
 * RETURN:     NONE
**************************************************************************/

void FAT_FreeChkDskBuf(PSECTOR_MAP_ARRAY_INFO pSectorMapBuf)
{
    PSECTOR_MAP_ARRAY_INFO next;
    
    while(pSectorMapBuf != AK_NULL)
    {
        next = pSectorMapBuf->next;
        kfree(pSectorMapBuf);
        pSectorMapBuf = next;
    }
}

/************************************************************************
 * NAME:        FAT_DowithFatError
 * FUNCTION  it will fix fat error
 * PARAM:      T_PDRIVER  driver
                      T_U32 ErrFat
 * RETURN:     NONE
**************************************************************************/

T_BOOL FAT_DowithFatError(struct super_block *sb, T_U32 ErrFat, T_U32 ClusPerBuf)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct buffer_head *bh = NULL;
    T_U8 *FatDataBuf;
	T_U32 PageNum, PageOff;
	int err;
    
	printk("FSLIB: we will fix the cluser:%08lx!, set the end flag.\r\n",ErrFat);

    PageNum = ErrFat / ClusPerBuf;
    PageOff = ErrFat % ClusPerBuf;

	PDEBUG("pagenum:%lu, page_off:%lu, ErrFat:%lu, ClusPerBuf:%lu.\n", PageNum, PageOff, ErrFat, ClusPerBuf);
	bh = sb_bread(sb, sbi->fat_start + PageNum);
	if(!bh) {        
        return AK_FALSE;
    }

	FatDataBuf = bh->b_data;

    if(sbi->fat_bits == 16)
    {
        ((T_U16 *)FatDataBuf)[PageOff] = EOF_FAT16;
    }
    else
    {
        ((T_U32 *)FatDataBuf)[PageOff] = EOF_FAT32;
    }

	mark_buffer_dirty(bh);
	err = sync_dirty_buffer(bh);
	
	brelse(bh);
    if(err) {        
        return AK_FALSE;
    }
    return AK_TRUE;
    
}


/************************************************************************
 * NAME:        FAT_CheckFatLinkError
 * FUNCTION  it will check the fat error
 * PARAM:      T_PDRIVER driver, 
                      T_U32 MarkItem
                      T_U32 ClusPerSector
 * RETURN:     NONE
**************************************************************************/

T_U8 FAT_CheckFatLinkError(struct super_block *sb, T_U32 MarkItem, T_U32 ClusPerSector)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct buffer_head *bh;
    T_U8 *FatDataBuf, ret;
    T_U32 SectorAddr, SectorOff;
    
    SectorAddr = MarkItem / ClusPerSector;
    SectorOff = MarkItem % ClusPerSector;
    
	PDEBUG("SectorAddr:%lu, SectorOff:%lu, MarkItem:%lu, ClusPerSector:%lu.\n", 
			SectorAddr, SectorOff, MarkItem, ClusPerSector);
  	bh = sb_bread(sb, sbi->fat_start + SectorAddr);
	if(!bh)
  	{
        return FAT_READ_ERROR;
    }

	FatDataBuf = bh->b_data;

    if(sbi->fat_bits == 16)
    {
        ret = (((T_U16 *)FatDataBuf)[SectorOff] == 0);
    }
    else
    {
        ret = (((T_U32 *)FatDataBuf)[SectorOff] == 0);
    }

	brelse(bh);
    if(ret == AK_TRUE)
        return FAT_LINK_ERROR;
    else
        return MARK_FAT_OK;
    
}


/* 128kb is the whole sectors for FAT12 and FAT16 */
#define FAT_READA_SIZE		(128 * 1024)


/************************************************************************
 * NAME:        FAT_CheckFatLinkError
 * FUNCTION  it will fix the fat error when system poweroff without closing file
 * PARAM:      T_U32 DriverID  , 
                      F_ChkDskCallback pCallBack
                      T_VOID *CallbackData
 * RETURN:     NONE
**************************************************************************/

T_BOOL Fat_ChkDsk(struct super_block *sb, F_ChkDskCallback pCallBack, T_VOID *CallbackData)
{
	struct msdos_sb_info *sbi = MSDOS_SB(sb);
	struct buffer_head *bh = NULL;
    T_U8 *FatDataBuf, *SectorBitMap,percent, ret;
    T_U32 i, j, ClusPerBuf, ClusPerSector;
    T_U32 CurCluster, NextFatItem, MarkFatItem;
    PSECTOR_MAP_ARRAY_INFO pSectorMapBuf = AK_NULL;
	T_U32 sector_bit;
	int diff_time; /*used for test.*/
   
	unsigned long reada_blocks, reada_mask;

	reada_blocks = FAT_READA_SIZE >> sb->s_blocksize_bits;
	reada_mask = reada_blocks - 1;

	sector_bit = sbi->cluster_bits - (ffs(sbi->sec_per_clus) - 1);
    
	diff_time = jiffies; /*record start time.*/

    SectorBitMap = kmalloc((sbi->fat_length >> 3) + 1,  GFP_KERNEL);
    if(SectorBitMap == AK_NULL)
        return AK_FALSE;
    memset(SectorBitMap, 0, (sbi->fat_length >> 3) + 1);
    
    ClusPerSector = (1 << (sector_bit - 1));
    ClusPerBuf = sb->s_blocksize >> 1;

	PDEBUG("sector_bit:%lu, ClusPerBuf:%lu, ClusPerSector:%lu, sbi->fat_length=%lu, s_blocksize:%lu.\n", 
			sector_bit,  ClusPerBuf, ClusPerSector, sbi->fat_length, sb->s_blocksize);

    if(sbi->fat_bits == 32)
    {
        ClusPerBuf >>= 1;
        ClusPerSector >>= 1;
    }
    
    pSectorMapBuf = FAT_MallocChkDskBuf(AK_NULL, (T_U16)ClusPerSector);
    if(pSectorMapBuf == AK_NULL)
        return AK_FALSE;
    
    percent = 0;

    for(i = 0, CurCluster = 0; i < sbi->fat_length && CurCluster <= sbi->max_cluster; i++)
	{
		if((i * 50) / sbi->fat_length != percent)
		{
			percent = (T_U8)((i * 50) /sbi->fat_length);
			printk(".");
		}
		/* readahead of fat blocks , current block equals i.*/
		if ((i & reada_mask) == 0) {
			int tmp;
			unsigned long rest = sbi->fat_length - i;
			rest = min(reada_blocks, rest);

			for (tmp = 0; tmp < rest; tmp++)
				sb_breadahead(sb, i + tmp);
		}

		bh = sb_bread(sb, sbi->fat_start + i);
        if(!bh)
        {
            CurCluster += ClusPerBuf;
            continue;
        }
		FatDataBuf = bh->b_data;

        for(j = 0; j < ClusPerBuf && CurCluster <= sbi->max_cluster; j ++, CurCluster ++)
        {
            NextFatItem = FAT_GetFatLinkInfo(sbi, FatDataBuf, (T_U16)j);
            if(NextFatItem == 0)
            {
                MarkFatItem = CurCluster;
            }
            else if(NextFatItem >= FAT32_MAX_USER_CLUSTER)
            {
                continue;
            }
            else
            {
                MarkFatItem = NextFatItem;
            }
			if((MarkFatItem > sbi->max_cluster) && (MarkFatItem != EOF_FAT32))
			{
			//	printk("the cluster is error:%lu-->%lu\r\n",CurCluster, NextFatItem);
				continue;
			}
            ret = FAT_AddMarkFatItemToBuf(SectorBitMap, pSectorMapBuf, MarkFatItem, (T_U16)ClusPerSector);
            if(ret == MARK_MALLOC_ERROR)
            {
                // if it fails to malloc ram , we will exit
                CurCluster = sbi->max_cluster;
                break;
            }
            else if(ret == FAT_LINK_ERROR)
            {
                // if we find the fat error, we will fix it , 
                ret = FAT_CheckFatLinkError(sb, MarkFatItem, ClusPerSector);
                if(ret == FAT_LINK_ERROR)
                {
                    if(FAT_DowithFatError(sb, MarkFatItem, ClusPerBuf) == AK_FALSE)
                    {
                        //it fails to fix error, we will exit
                        CurCluster = sbi->max_cluster;
                    }
                }
            }
        }        
		brelse(bh);
    }
    FAT_FreeChkDskBuf(pSectorMapBuf);
    kfree(SectorBitMap);

	diff_time = jiffies - diff_time;
	printk("Check disk finish, spend time:%dms.\n", (diff_time*1000)/HZ);

	return AK_TRUE;
}



