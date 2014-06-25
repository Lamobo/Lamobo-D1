#ifndef __AK39_UART_H_
#define __AK39_UART_H_

#define AK39_UART0_TXBUF_BASE	REG_VA_ADDR(AK_VA_L2MEM, 0x1000)
#define AK39_UART0_RXBUF_BASE	REG_VA_ADDR(AK_VA_L2MEM, 0x1080)
#define AK39_UART1_TXBUF_BASE	REG_VA_ADDR(AK_VA_L2MEM, 0x1100)
#define AK39_UART1_RXBUF_BASE	REG_VA_ADDR(AK_VA_L2MEM, 0x1180)

#define AK39_UART0_BASE			REG_VA_ADDR(AK_VA_UART, 0x0000)
#define AK39_UART1_BASE			REG_VA_ADDR(AK_VA_UART, 0x8000)

#define AK39_UART0_PA_BASE		REG_PA_ADDR(AK_PA_UART, 0x0000)
#define AK39_UART1_PA_BASE		REG_PA_ADDR(AK_PA_UART, 0x8000)


#define UART_CONF1			    0x00
#define	UART_CONF2			    0x04
#define	DATA_CONF			    0x08
#define	BUF_THRESHOLD		    0x0C
#define	UART_RXBUF			    0x10
#define UART_STOPBIT_TIMEOUT    (0x18)

#define RX_THR_INT_ENABLE		(28)
#define TX_END_INTERRUPT		(27)
#define TXBUF_EMP_INT_ENABLE	(24)
#define RECVDATA_ERR_INT_ENABLE	(23)
#define RX_TIMEOUT_INT_ENABLE	(22)
#define	MEM_RDY_INT_ENABLE		(21)
#define TX_THR_INT			(31)
#define RX_THR_INT			(30)
#define	TX_END_STATUS		(19)
#define	RX_OV				(18)
#define	MEM_RDY_INT			(17)
#define TX_BYT_CNT_VLD		(16)
#define RECVDATA_ERR_INT	(3)
#define	RX_TIMEOUT			(2)
#define RXBUF_FULL			(1)
#define TXFIFO_EMPTY		(0)

#define TX_INTTERUPT		(29)
#define RX_INTTERUPT		(28)

#define	TX_STATUS		    (1)
#define	RX_STATUS	    	(0)
#define DISABLE		    	(0)
#define ENABLE			    (1)
#define	AKUART_INT_MASK		0x3FE00000
#define UART_RX_FIFO_SIZE	128

// Configuration Register 1 of UARTn
#define UARTN_CONFIG1_DIV_CNT(cnt)          ((cnt) & 0xffff)
#define UARTN_CONFIG1_UTD_INVERSELY         (1 << 16)
#define UARTN_CONFIG1_URD_INVERSELY         (1 << 17)
#define UARTN_CONFIG1_CTS_INVERSELY         (1 << 18)
#define UARTN_CONFIG1_RTS_INVERSELY         (1 << 19)
#define UARTN_CONFIG1_RTS_EN_BY_CIRCUIT     (1 << 20)
#define UARTN_CONFIG1_EN                    (1 << 21)
#define UARTN_CONFIG1_DIV_ADJ_EN            (1 << 22)
#define UARTN_CONFIG1_TIMEOUT_EN            (1 << 23)
#define UARTN_CONFIG1_PAR_EVEN              (1 << 25)
#define UARTN_CONFIG1_PAR_EN                (1 << 26)
#define UARTN_CONFIG1_ENDIAN_BIG            (1 << 27)
#define UARTN_CONFIG1_TX_STA_CLR            (1 << 28)
#define UARTN_CONFIG1_RX_STA_CLR            (1 << 29)
#define UARTN_RX_ADDR_CLR					(1 << 30)
#define UARTN_TX_ADDR_CLR					(1 << 31)

// Configuration Register 2 of UARTn
#define UARTN_CONFIG2_TX_FIFO_EMPTY         (1 << 0)  // read only
#define UARTN_CONFIG2_RX_BUF_FULL           (1 << 1)  // write clear
#define UARTN_CONFIG2_TIMEOUT               (1 << 2)  // write clear
#define UARTN_CONFIG2_R_ERR                 (1 << 3)  // write clear
#define UARTN_CONFIG2_TX_BYT_CNT(cnt)       (cnt << 4)
#define UARTN_CONFIG2_TX_BYT_CNT_MASK       (0xfff << 4)
#define UARTN_CONFIG2_TX_BYT_CNT_VLD        (1 << 16) // auto clear
#define UARTN_CONFIG2_MEM_RDY               (1 << 17) // read only

#define UARTN_CONFIG2_TX_END                (1 << 19) // read only

#define UARTN_CONFIG2_RX_BUF_FULL_INT_EN    (1 << 21)
#define UARTN_CONFIG2_TIMEOUT_INT_EN        (1 << 22)
#define UARTN_CONFIG2_R_ERR_INT_EN          (1 << 23)
#define UARTN_CONFIG2_TX_BUF_EMP_INT_EN     (1 << 24)

#define UARTN_CONFIG2_TX_END_INT_EN         (1 << 27)
#define UARTN_CONFIG2_RX_TH_INT_EN          (1 << 28)
#define UARTN_CONFIG2_TX_TH_INT_EN          (1 << 29)

#define UARTN_CONFIG2_RX_TH_STA             (1 << 30) // write clear
#define UARTN_CONFIG2_TX_TH_STA             (1 << 31) // write clear

// Data Configuration Register of UARTn
#define UARTN_DATACONFIG_TX_BYT_SUM(rval)   	((rval) & 0x1FFF)
#define UARTN_DATACONFIG_RX_ADR(rval)       	(((rval) >> 13) & 0x7F)
#define UARTN_DATACONFIG_TX_ADR(rval)       	(((rval) >> 20) & 0x1F)
#define UARTN_RX_TH_CFG_H_MASK              	(0x7f >> 25)
#define UARTN_RX_TH_CFG_H(value)                (((value & 0x7f) >> 25))


// TX RX Data Threshold Resgister
#define UARTN_RX_TH_CFG_L_MASK              (0x1f)
#define UARTN_RX_TH_CFG_L(value)            (value)

#define UARTN_RX_TH_CLR                     (1 << 5)
#define UARTN_TX_TH_CFG(value)              (((value) & 0x1f) << 6)
#define UARTN_TX_TH_CLR                     (1 << 11)
#define UARTN_RX_TH_CNT(rvalue)             (((rvalue) >> 12) & 0xFFF)   // read only
#define UARTN_TX_TH_CNT(rvalue)             (((rvalue) >> 24) & 0x1F)   // read only
#define UARTN_BFIFO_BYTE_NUM(rvalue)		(((rvalue) >> 29) & 0x03)   // read only
#define UARTN_RX_START						(1 <<31)


//Stop Bit Timeout Configuration Register

#define UARTN_BIT_CFG(value)				((value) & 0x1ff)
#define UARTN_TIME_OUT_CFG(value)			(((value) >> 16) & 0x0ffff)


#undef REG_VA_VAL
#define REG_VA_VAL(base_addr, offset)	(*(volatile unsigned long *)((base_addr) + (offset)))

#define rL2_FRACDMAADDR         	REG_VA_VAL(AK_VA_L2CTRL, 0x84)
#define rL2_CONBUF8_15          	REG_VA_VAL(AK_VA_L2CTRL, 0x8C) 

#endif  /* end __AK39_UART_H_ */
