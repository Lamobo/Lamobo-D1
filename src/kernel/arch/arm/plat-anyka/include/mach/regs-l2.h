
/*
 * arch/arm/plat-anyka/include/mach/regs-l2.h
 */
#ifndef __REGS_L2_H_
#define __REGS_L2_H_

#include <mach/map.h>

#define vL2DMA_ADDRBUF0         REG_VA_ADDR(AK_VA_L2CTRL, 0x00)
#define vL2DMA_CONBUF0          REG_VA_ADDR(AK_VA_L2CTRL, 0x40)

#define L2_DMA_ADDR				(AK_VA_L2CTRL + 0x00)
#define L2_DMA_CON				(AK_VA_L2CTRL + 0x40)
#define L2_DMAREQ				(AK_VA_L2CTRL + 0x80)
#define L2_FRACDMAADDR			(AK_VA_L2CTRL + 0x84)
#define L2_CONBUF0_7			(AK_VA_L2CTRL + 0x88)  
#define L2_CONBUF8_15			(AK_VA_L2CTRL + 0x8C) 
#define L2_BUFASSIGN1			(AK_VA_L2CTRL + 0x90) 
#define L2_BUFASSIGN2			(AK_VA_L2CTRL + 0x94) 
#define L2_LDMACON				(AK_VA_L2CTRL + 0x98) 
#define L2_BUFINTEN				(AK_VA_L2CTRL + 0x9C) 
#define L2_BUFSTAT1				(AK_VA_L2CTRL + 0xA0) 
#define L2_BUFSTAT2				(AK_VA_L2CTRL + 0xA8) 

/*************************** L2 MEMORY CONTROL *********************/
#define rL2_DMAREQ              REG_VA_VAL(AK_VA_L2CTRL, 0x80)
#define rL2_FRACDMAADDR         REG_VA_VAL(AK_VA_L2CTRL, 0x84)
#define rL2_CONBUF0_7           REG_VA_VAL(AK_VA_L2CTRL, 0x88)  
#define rL2_CONBUF8_15          REG_VA_VAL(AK_VA_L2CTRL, 0x8C) 
#define rL2_BUFASSIGN1          REG_VA_VAL(AK_VA_L2CTRL, 0x90) 
#define rL2_BUFASSIGN2          REG_VA_VAL(AK_VA_L2CTRL, 0x94) 
#define rL2_LDMACON             REG_VA_VAL(AK_VA_L2CTRL, 0x98) 
#define rL2_BUFINTEN            REG_VA_VAL(AK_VA_L2CTRL, 0x9C) 
#define rL2_BUFSTAT1            REG_VA_VAL(AK_VA_L2CTRL, 0xA0) 
#define rL2_BUFSTAT2			REG_VA_VAL(AK_VA_L2CTRL, 0xA8) 

/*************************** L2 MEMORY BUFFER **********************/
#define rL2_ADDRBUF0            REG_VA_VAL(AK_VA_L2MEM, 0x0000)
#define rL2_ADDRBUF1            REG_VA_VAL(AK_VA_L2MEM, 0x0200)
#define rL2_ADDRBUF2            REG_VA_VAL(AK_VA_L2MEM, 0x0400)
#define rL2_ADDRBUF3            REG_VA_VAL(AK_VA_L2MEM, 0x0600)
#define rL2_ADDRBUF4            REG_VA_VAL(AK_VA_L2MEM, 0x0800)
#define rL2_ADDRBUF5            REG_VA_VAL(AK_VA_L2MEM, 0x0A00)
#define rL2_ADDRBUF6            REG_VA_VAL(AK_VA_L2MEM, 0x0C00)
#define rL2_ADDRBUF7            REG_VA_VAL(AK_VA_L2MEM, 0x0E00)

#define rL2_ADDRTX1_BUF8        REG_VA_VAL(AK_VA_L2MEM, 0x1000)
#define rL2_ADDRRX1_BUF9        REG_VA_VAL(AK_VA_L2MEM, 0x1080)
#define rL2_ADDRTX2_BUF10       REG_VA_VAL(AK_VA_L2MEM, 0x1100)
#define rL2_ADDRRX2_BUF11       REG_VA_VAL(AK_VA_L2MEM, 0x1180)

#endif  /* __REGS_L2_H_ */

