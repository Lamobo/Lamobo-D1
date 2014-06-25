#ifndef __L2_EXEBUF
#define __L2_EXEBUF


#include <linux/io.h>
#include <mach/gpio.h>
#include <mach/map.h>


#define	CPU_CHIP_ID					(AK_VA_SYSCTRL + 0x00)
#define	CLOCK_DIV_REG				(AK_VA_SYSCTRL + 0x04)
#define	CLOCK_CTRL_REG				(AK_VA_SYSCTRL + 0x0C)
#define	PHY_CLOCK_CTRL_REG			(AK_PA_SYSCTRL + 0x0C)
#define PHY_CLOCK_DIV_REG   		(AK_PA_SYSCTRL + 0x04)
#define PHY_RAM_CFG_REG4			(AK_PA_REGRAM + 0x0C)

#define RAM_CFG_REG1				(AK_VA_REGRAM + 0x00)
#define RAM_CFG_REG2				(AK_VA_REGRAM + 0x04)
#define RAM_CFG_REG3				(AK_VA_REGRAM + 0x08)
#define RAM_CFG_REG4				(AK_VA_REGRAM + 0x0C)
#define RAM_CPU_CMD					(AK_VA_REGRAM + 0x10)
#define PHY_RAM_CFG_REG1			(AK_PA_REGRAM + 0x00)
#define PHY_RAM_CFG_REG2			(AK_PA_REGRAM + 0x04)
#define PHY_RAM_CFG_REG3			(AK_PA_REGRAM + 0x08)
#define PHY_RAM_CFG_REG4			(AK_PA_REGRAM + 0x0C)
#define PHY_RAM_CPU_CMD				(AK_PA_REGRAM + 0x10)

#define FIFO_R_EMPTY				(1 << 16)
#define FIFO_CMD_EMPTY				(1 << 14)
#define AUTO_REFRESH_EN				(1 << 0)
#define RAM_CLOCK_DISABLE			(1 << 10)
#define ENTER_STANDBY				(1 << 13)
#define REFRESH_PERIOD_INTERVAL		(0x39f << 1)

#if 0
#define LED_INIT_DEBUG do {\
	unsigned long value;\
	REG32(AK_VA_SYSCTRL + 0x00a8) |= (1 << 16);\
	REG32(AK_VA_SYSCTRL + 0x0094) &= ~(1 << 16);\
	} while(0)
#define LED_PHY_ON	do {\
	REG32(AK_PA_SYSCTRL + 0x0098) &= ~(1 << 16);\
	} while(0)

#define LED_PHY_OFF	do {\
	REG32(AK_PA_SYSCTRL + 0x0098) |= (1 << 16);\
	} while(0)

#define LED_VIRT_ON	do {\
	REG32(AK_VA_SYSCTRL + 0x0098) &= ~(1 << 16);\
	} while(0)

#define LED_VIRT_OFF do {\
	REG32(AK_VA_SYSCTRL + 0x0098) |= (1 << 16);\
	} while(0)
#endif

#define DISABLE_CACHE_MMU()	do { \
	__asm__ __volatile__(	\
		"tci_loop: mrc  p15, 0, r15, c7, c14, 3\n\t" /* test,clean,invalidate D cache */\
		"bne  tci_loop\n\t"										\
		"mcr  p15, 0, %0, c8, c7, 0\n\t"	/* invalidate  I & D  TLBs */	\
		"mcr  p15, 0, %0, c7, c5, 0\n\t"	/* invalidate I caches */	\
		"mrc  p15, 0, %0, c1, c0, 0\n\t"							\
		"bic  %0, %0, #0x1000\n\t"		/* disable Icache */			\
		"bic  %0, %0, #0x0005\n\t"		/* disable Dcache,mmu*/		\
		"ldr  %1, =l2_phys_run\n\t"		/* load 0x480000xx address */	\
		"b  suspend_turn_off_mmu\n\t"								\
		"   .align 5\n\t"				/* 32 byte aligned */			\
		"suspend_turn_off_mmu:\n\t"\
		"mcr  p15, 0, %0, c1, c0, 0\n\t"\
		"mov  pc, %1\n\t"		/* jumpto 0x480000xx then run */		\
		"l2_phys_run:\n\t"		/* mark the real running addr--> L2 buff */\
		: : "r"(0),"r"(1));	\
	} while(0)

#define ENABLE_CACHE_MMU()	do { \
	__asm__ __volatile__(	\
		"mcr  p15, 0, %0, c8, c7, 0\n\t"	/* invalidate I & D TLBs */	\
		"mcr  p15, 0, %0, c7, c7, 0\n\t"	/* invalidate I & D caches */\
		"mcr  p15, 0, %0, c7, c10, 4\n\t"	/* Drain write buffer */		\
		"mrc  p15, 0, %0, c1, c0, 0\n\t"	\
		"orr  %0, %0, #0x1000\n\t"			\
		"orr  %0, %0, #0x0005\n\t"			\
		"b	resume_turn_on_mmu\n\t"			\
		"	.align 5\n\t"					\
		"resume_turn_on_mmu:\n\t"			\
		"mcr  p15, 0, %0, c1, c0, 0\n\t"	\
		::"r"(2));	\
	} while(0)

#define	PM_DELAY(time)	do { \
	__asm__ __volatile__(	\
		"1:\n\t"	\
		"subs %0, %0, #1\n\t"	\
		"bne 1b\n\t"			\
		::"r"(time));	\
	} while(0)


#define DDR2_ENTER_POWERDOWN() do {\
    /* send precharge all banks */\
    __raw_writel(0x0aa00400, PHY_RAM_CPU_CMD);\
    /* close odt and asserting low on cke ,close odt and send enter powerdown command */\
    __raw_writel(0x04f00000, PHY_RAM_CPU_CMD);\
    }while(0)

#define DDR2_EXIT_POWERDOWN() do {\
    /* exit precharge power-down mode after delay at least 3 tck */\
    /* by asserting high on cke and odt remain low */\
    __raw_writel(0x02f00000, PHY_RAM_CPU_CMD);\
    }while(0)


#define DDR2_ENTER_SELFREFRESH() do {\
    /* send precharge all banks */\
    __raw_writel(0x0aa00400, PHY_RAM_CPU_CMD);\
    /* close odt and delay taofd=2.5 tck */\
    __raw_writel(0x02f00000, PHY_RAM_CPU_CMD);\
    PM_DELAY(0x1);\
    /* asserting low on cke ,close odt and entry self-refresh mode */\
    __raw_writel(0x04c00000, PHY_RAM_CPU_CMD);\
    }while(0)

#define DDR2_EXIT_SELFREFRESH() do {\
    /* exit self-refresh by asserting high on cke and odt remain low */\
    __raw_writel(0x02f00000, PHY_RAM_CPU_CMD);\
    /* delay txsrd=200tck as send nop cmd */ \
    __raw_writel(0x02f00000, PHY_RAM_CPU_CMD);\
    PM_DELAY(0x8);\
    }while(0)

#define DDR2_ENTER_AUTOREFRESH() do {\
    /* send auto refresh and open odt high */\
    __raw_writel(0x0ac00000, PHY_RAM_CPU_CMD);\
    __raw_writel(0x0ac00000, PHY_RAM_CPU_CMD);\
    __raw_writel(0x0ac00000, PHY_RAM_CPU_CMD);\
    }while(0)


#define L2_LINK(flag)		__section(.l2mem_##flag)
#define L2FUNC_NAME(name)	l2_enter_##name

#define SPECIFIC_L2BUF_EXEC(flag, param1,param2,param3,param4) do {\
	extern char _end_##flag[], _start_##flag[];\
	int len;\
	len = _end_##flag - _start_##flag;\
	l2_exec_buf(_start_##flag,len, param1,param2,param3,param4);\
}while(0)

int l2_exec_buf(const char *vaddr, int len, unsigned long param1, 
	unsigned long param2,unsigned long param3, unsigned long param4);


#endif	/* L2_EXEBUF */


