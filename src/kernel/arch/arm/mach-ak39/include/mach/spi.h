/*
 * include/asm-arm/arch-ak37/spi.h
 */

#ifndef __SPI_H__
#define __SPI_H__

struct ak_spi_info {
	unsigned long* pin_cs;	
	unsigned short num_cs;
	unsigned long board_size;
	unsigned short bus_num;
	unsigned short mode_bits;
	char clk_name[20];
	int xfer_mode; /*use for dma or cpu*/
};

enum akspi_cs_num {	
	AKSPI_ONCHIP_CS = 0,	/*on chip control cs index*/
	/*AKSPI_CS1,*/
	/*AKSPI_CS2,*/
	AKSPI_CS_NUM, 		
};

enum akspi_bus_num {
	AKSPI_BUS_NUM1,
	AKSPI_BUS_NUM2,
	AKSPI_MAX_BUS_NUM,
};


#define AKSPI_1DATAWIRE 	(0b00<<16)
#define AKSPI_2DATAWIRE 	(0b01<<16)
#define AKSPI_4DATAWIRE 	(0b10<<16)

#define AKSPI_XFER_MODE_DMA 	(1)
#define AKSPI_XFER_MODE_CPU 	(2)


#define AK_SPICON		(0x00)
#define AK_SPICON_WIRE		(0x3<<16)
#define AK_SPICON_CLKDIV	(0x7F<<8)
#define AK_SPICON_EN	(1<<6)
#define AK_SPICON_CS	(1<<5)
#define AK_SPICON_MS	(1<<4)
#define AK_SPICON_CPHA	(1<<3)
#define AK_SPICON_CPOL	(1<<2)
#define AK_SPICON_ARRM	(1<<1)
#define AK_SPICON_TGDM	(1<<0)

#define AK_SPISTA		(0x04)
#define AK_SPISTA_TIMEOUT	(1<<10)
#define AK_SPISTA_MPROC	(1<<9)
#define AK_SPISTA_TRANSF	(1<<8)
#define AK_SPISTA_RXOVER	(1<<7)
#define AK_SPISTA_RXHFULL	(1<<6)
#define AK_SPISTA_RXFULL	(1<<5)
#define AK_SPISTA_RXEMP	(1<<4)
#define AK_SPISTA_TXUNDER	(1<<3)
#define AK_SPISTA_TXHEMP	(1<<2)
#define AK_SPISTA_TXFULL	(1<<1)
#define AK_SPISTA_TXEMP	(1<<0)

#define AK_SPIINT		(0x08)
#define AK_SPIINT_TIMEOUT	(1<<10)
#define AK_SPIINT_MPROC	(1<<9)
#define AK_SPIINT_TRANSF	(1<<8)
#define AK_SPIINT_RXOVER	(1<<7)
#define AK_SPIINT_RXHFULL	(1<<6)
#define AK_SPIINT_RXFULL	(1<<5)
#define AK_SPIINT_RXEMP	(1<<4)
#define AK_SPIINT_TXUNDER	(1<<3)
#define AK_SPIINT_TXHEMP	(1<<2)
#define AK_SPIINT_TXFULL	(1<<1)
#define AK_SPIINT_TXEMP	(1<<0)

#define AK_SPICNT		(0x0C)

#define AK_SPIEXTX		(0x10)
#define AK_SPIEXTX_BUFEN	(1<<0)
#define AK_SPIEXTX_DMAEN	(1<<16)


#define AK_SPIEXRX		(0x14)
#define AK_SPIEXRX_BUFEN	(1<<0)
#define AK_SPIEXRX_DMAEN	(1<<16)

#define AK_SPIOUT		(0x18)

#define AK_SPIIN		(0x1C)

#endif

