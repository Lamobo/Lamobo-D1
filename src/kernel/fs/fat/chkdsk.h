#ifndef _CHKDSK_H_
#define _CHKDSK_H_
#include <plat-anyka/anyka_types.h>
#include "fat.h"


#define FAT16_MAX_USER_CLUSTER    0XFFF
#define FAT32_MAX_USER_CLUSTER    0XFFFFFF8
typedef    struct tag_Medium  T_MEDIUM;

typedef    struct tag_Medium *T_PMEDIUM;


typedef    T_U32 (*F_ReadSector)(T_PMEDIUM medium, T_U8** buf, T_U32 start, T_U32 size);
typedef    T_U32 (*F_WriteSector)(T_PMEDIUM medium, const T_U8 *buf, T_U32 start, T_U32 size);

struct tag_Medium
{
	struct super_block *sb;

    T_U8  SecBit;
    T_U8  PageBit;  // 2^n = SecPerPage.
    T_U8  SecPerPg; //PageBit -SecBit .
};


#define CLUSTER_MAP_NUMBER  100
typedef struct _CLUSTER_MAP_ARRAY_INFO *PSECTOR_MAP_ARRAY_INFO;

typedef struct _CLUSTER_MAP_INFO
{
	T_U32 sectorAddr;	//此缓冲区所对应的SECOTR,因为FAT表不可能太大,所以定义为T_U16以便节省内存,如果此值为-1,表示这个缓冲区没有被人使用.
	T_U32 bitNum;		//记录位图中有多少个位被置1了,如果它等于CluserPerSector,将表示位图为全1状态,可释放此缓冲区.
	T_U8 *bitMap;		//位图数据,当有人指向它,或者为空时就置1,否则为0,如果全为1时,将释放这个缓冲区(将sectorAddr = T_U16_MAX),以便其它扇区使用.这个缓冲区的大小为BytesPerSector/8.
} CLUSTER_MAP_INFO, *PCLUSTER_MAP_INFO;

typedef struct _CLUSTER_MAP_ARRAY_INFO
{
	T_U32 index;		//簇缓冲区,每次分配100个.
	PSECTOR_MAP_ARRAY_INFO next;		//指向下一个大缓冲区
	PCLUSTER_MAP_INFO pMapArray;		//簇缓冲区,每次分配100个.
	PSECTOR_MAP_ARRAY_INFO pCurSecMap;		//指向当前正在使用的小缓冲区
}SECTOR_MAP_ARRAY_INFO;


enum{MARK_FAT_OK, FAT_LINK_ERROR, MARK_MALLOC_ERROR, FAT_READ_ERROR};

#define TestBitMap(BitMap, item)    (((BitMap)[(item)>>3]&(1<<((item)&7))))
#define SetBitMap(BitMap, item)     ((BitMap)[(item)>>3] |= (1<<((item)&7)))
#define ClrBitMap(BitMap, item)     ((BitMap)[(item)>>3] &= ~(1<<((item)&7)))

typedef void F_ChkDskCallback(T_VOID *pData, T_U32 percent);
T_BOOL Fat_ChkDsk(struct super_block *sb, F_ChkDskCallback pCallBack, T_VOID *CallbackData);

//T_U32 FAT_GetFatLinkInfo(struct msdos_sb_info *sbi, T_U8 * pFatBuf, T_U16 offset);
//T_U32 FAT_GetFatLinkInfo_chkdsk(struct msdos_sb_info *sbi, T_U8 * pFatBuf, T_U16 offset);
//T_VOID FAT_SetFatLinkInfo(struct msdos_sb_info *sbi, T_U8 * pFatBuf, T_U16 offset, T_U32 newValue);


#endif

