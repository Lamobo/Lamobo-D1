#include "StdAfx.h"
#include "Config.h"
#include "BurnTool.h"

#define DEFAULT_UART0_RATE 115200

extern CConfig theConfig;
extern CBurnToolApp theApp;

//global variables about ram registers config
//RAM TYPE MCP
static UINT reg_addr_mcp[] = {
    0x2002d004, 
    0x2002d000, 0x66668888,  
    0x2002d000, 0x2002d000, 
    0x66668888, 0x2002d000, 
    0x66668888, 0x88888888
};
static UINT reg_value_mcp[] = {
    0x09527214, 
    0xe0110000, 0x0007d000, 
    0xe0120400, 0xe0100033,
    0x000000c8, 0xe0108000, 
    0x000000c8, 0x00000000
};

//RAM TYPE SDRAM
static UINT reg_addr_sdr[] = {
    0x2002d008, 0x2002d004, 
    0x2002d000, 0x2002d000, 0x2002d000, 
    0x66668888, 
    0x2002d000, 0x2002d000, 0x2002d000, 
    0x88888888};
static UINT reg_value_sdr[] = {
    0x00067d00, 0x09527214, 
    0xe0170000, 0xe0120400, 0xe0100033, 
    0x000000c8,
    0xe0124400, 0xe0110000, 0xe0110000, 
    0x00000000
};

//RAM TYPE DDR
// ASIC = 124M
static UINT  reg_addr_ddr[] = 
{
    0x080000dc, 0x08000004, 
    0x66668888, 0x66668888, // pll = 248Mhz
    0x20026000, 
    0x66668888, 0x66668888, // set uart baudrate
    0x08000064, 0x080000a8, // use sstl2 , open ien, 
    0x2002d004, 0x66668888, 
    0x2002d000, 0x2002d000, 
    0x66668888, 0x2002d000, 
    0x66668888, 0x2002d000, 
    0x66668888, 0x2002d000, 
    0x66668888, 0x2002d000, 
    0x66668888, 0x2002d000, 
    0x66668888, 0x2002d000, 
    0x66668888, 0x2002d000, 
    0x2002d008
};
static UINT reg_value_ddr[] = 
{
    0x0000000c, 0x00021051, 
    0x000000c8, 0x000000c8, 
    0x30200433, 
    0x000000c8, 0x000000c8, 
    0x08000000, 0x44000000, 
    0x0f706b95, 0x000000c8, 
    0x40170000, 0x40120400, 
    0x000000c8, 0x40104000, 
    0x000000c8, 0x40100123, 
    0x000000c8, 0x40120400, 
    0x000000c8, 0x40110000, 
    0x000000c8, 0x40110000, 
    0x000000c8, 0x40100023, 
    0x000000c8, 0x60170000, 
    0x01027c58
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//aspen3s SDRAM CONFIG
static UINT reg_addr_sdr_aspen3s[] = {
    0x2000e000, 0x2000e010, 
    0x2000e010, 0x2000e010, 
    0x2000e010, 0x2000e010, 
    0x2000e010, 0x2000e010, 
    0x2000e00c, 0x88888888};
    
static UINT reg_value_sdr_aspen3s[] = {
    0x00004817, 0x02000000, 
    0x02f00000, 0x02a00400, 
    0x02c00000,    0x02c00000, 
    0x02800032, 0x02f00000, 
    0x000143a1, 0x00000000};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//aspen3s DDR CONFIG
// ASIC = 124M
static UINT  reg_addr_ddr_aspen3s[] = 
{
    0x2000e05c,
    0x2000e078,                       //io pad config
    0x08000004,                       //change frequency
    0x20026000,                       //uart initial
    0x66668888,                       //DELAY
    0x2000e004,                       // AC Timing Configure 1
    0x2000e008,                       // AC Timing Configure 2
    0x2000e000,                       //ROW COLUMN SET
    0x66668888,                       //DELAY
    0x2000e010,                       //cke enable
    0x2000e010,                       //nop
    0x2000e010,                       //precharge all
    0x2000e010,                       //extended mode reg 1
    0x2000e010,                       //master mode reg reset DLL
    0x2000e010,                       //precharge all
    0x2000e010,                       //auto refresh
    0x2000e010,                       //auto refresh
    0x2000e010,                       //master mode reg
    0x2000e00c,                       //open auto refresh
    0x2000e020,                       //open dll set
    0x66668888,                       //delay
    0x2000e024,                       //normal calibration
    0x66668888,                       //delay
    0x66668888,                       //delay
    0x66668888,                        //delay
    0x88888888
};

static UINT reg_value_ddr_aspen3s[] = 
{
    0x200,
    0x288040,//0x140a,                                                     //io pad config
    //124 MHZ
    0x00005011,                                                          //change frequence
    (0x0434 | (0x1<<21) | (0x1<<22) | (0x1<<28) | (0x1<<29) ),           //uart initial
    0x00000000,                                                          //delay
    0x08252209,                                                           // AC Timing Configure 1
    0x00144254,                                                           // AC Timing Configure 2    
    ( 0x1 | (0x1<<2) | (0x1<<3) | (0x1<<6) | (0x3<<10) | (0x2<<13) ),    //ROW COLUMN SET, CL = 2.5          
    0x00000000,                                                          //delay
    0x02000000,                                                          //cke enable
    ( (0x1<<25) | (0x1<<23) | (0x1<<22) | (0x1<<21) | (0x1<<20) ),       //nop
    ( (0x1<<25) | (0x1<<23) | (0x1<<21) | (0x1<<10) ),                   //precharge all
    ( (0x1<<25) | (0x1<<23) | (0x1<<16) ),                               //extended mode reg 1
    ( (0x1<<25) | (0x1<<23) | 0x162 ),                                   //master mode reg reset DLL
    ( (0x1<<25) | (0x1<<23) | (0x1<<21) | (0x1<<10) ),                   //precharge all
    ( (0x1<<25) | (0x1<<23) | (0x1<<22) ),                               //auto refresh
    ( (0x1<<25) | (0x1<<23) | (0x1<<22) ),                               //auto refresh
    ( (0x1<<25) | (0x1<<23) | 0x62 ),                                    //master mode reg
    (0x1 | (0x3e8<<1) ),                                                 //open auto refresh
    0x3,                                                                 //open dll set
    0x00000000,                                                          //delay
    (0x1<<0),                                                            //normal calibration
    0x00000000,                                                          //delay
    0x00000000,                                                          //delay
    0x00000000,                                                          //delay
    0x00000000                                                          
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* aspen3s mobile DDR CONFIG For Linux support */
// AK98: PLL 360 MHz, CPU Core 360 MHz, MEM 180 MHz, ASIC 90 MHz
static UINT  reg_addr_m_ddr_aspen3s[] = 
{
    /* update software, reset ddr2 and nand */
	0x66668888,
	0x0800000c,
	0x66668888,
	0x0800000c,
	
	0x2000e05c,
	0x2000e078,

	0x66668888,
	0x08000004,
	0x20026000,
	0x66668888,
    
	0x2000e004,
	0x2000e008,
	0x2000e000,
	0x66668888,
	
	0x2000e010,
	0x2000e020,
	0x66668888,

	0x2000e010,
	0x66668888,
	0x2000e010,
	0x2000e010,
	0x66668888,
	0x2000e010,
	0x2000e010,
	0x66668888,
	0x2000e010,
	0x2000e010,
	0x66668888, 
	0x2000e010,  
	0x2000e010,
	0x66668888,
	0x2000e010,
	0x2000e010,
	0x66668888,
	
	0x2000e024,
	0x66668888,
	0x2000e00c,
	0x66668888,
	0x88888888,
};

static UINT reg_value_m_ddr_aspen3s[] = 
{
	0x00010100,	/* delay */
	0x44004de7,	/* reset some module */
	0x00010100,	/* delay */
	0x000009e7,	/* complete reset operation */

	(1 << 9),	/* bypass DCC */
	0x003cc060,	/* IO pad configure */

	0x00010100,  	/* delay */
	0x0000d0ad,  	/* set clock timting CPU:360 MEM:180 ASIC:90 */
	0x3020030c,  	/* set UART 115200 */
	0x00008000,  	/* delay */

	((10 << 24)|(2 << 21)|(7 << 16)|(3 << 12)|(3 << 8)|17), 	/* set ac timing ? */
	((61 << 16)|(6 << 12)|(5 << 8)|(3 << 4)|4),  			/* set ac timing ? */
	((2 << 13)|(3 << 10)|(0 << 9)|(1 << 6)|(2 << 3)|(1 << 2)|2), 	/* set physical parameter ? */
	0x00000000,  	/* delay */

	(1 << 25),   	/* enable CKE */
	0x00000003,  	/* enable on-chip DLL? */
	0x00000000,  	/* delay */

	((1 << 25)|(1 << 23)|(1 << 22)|(1 << 21)|(1 << 20)), 	/* NOP */
	0x00001000,  	/* delay 200us */
	((1 << 25)|(1 << 23)|(1 << 21)|(1 << 10)),   	/* precharge all */
	((1 << 25)|(1 << 23)|(1 << 22)|(1 << 21)|(1 << 20)), 	/* NOP for at least tRP */
	0x00000040,  	/* delay */
	((1 << 25)|(1 << 23)|(1 << 22)),	/* auto refresh */
	((1 << 25)|(1 << 23)|(1 << 22)|(1 << 21)|(1 << 20)), 	/* NOP for at least tRFC */
	0x00000040,  	/* delay */
	((1 << 25)|(1 << 23)|(1 << 22)),     	/* auto refresh */
	((1 << 25)|(1 << 23)|(1 << 22)|(1 << 21)|(1 << 20)), 	/* NOP for at least tRFC */
	0x00000040,  	/* delay */
	((1 << 25)|(1 << 23)|(3 << 4)|(0 << 3)|2),   	/* MRS: burst length=4, sequential burst type, CAS latency=3 */
	((1 << 25)|(1 << 23)|(1 << 22)|(1 << 21)|(1 << 20)), 	/* NOP for at least tMRD */
	0x00000040,  	/* delay */
	((1 << 25)|(1 << 23)|(1 << 17)),     /* EMRS: normal driver strength, refresh all banks during self-refresh */
	((1 << 25)|(1 << 23)|(1 << 22)|(1 << 21)|(1 << 20)), 	/* NOP for at least tMRD */
	0x00000040,  	/* delay */

	0x00000001,	/* calibrate read timing */
	0x00001000,	/* delay */
	((1000 << 1)|1),	/* tck = 5.5ns(MEM=180M), trfei = 7.8us */
	0x00001000,	/* delay */
	0x00000000,	/* bye */
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//aspen3s DDR2 CONFIG
// ASIC = 124M
//#ifndef SUPPORT_LINUX
static UINT  reg_addr_ddr2_aspen3s_rtos[] = 
{
    0x66668888,          //DELAY
    0x08000004,          //change frequence
    0x20026000,          //uart initial
    0x66668888,          //DELAY

    0x2000e004,
    0x2000e008,
    
    0x2000e05c,          //0 bypass dcc
    0x2000e078,          //io pad config, 1.8 ddr2 sstl
    0x08000064,          //ien 

    0x2000e004,          //AC Timing Configure 1
    0x2000e008,          //AC Timing Configure 2

    0x2000e000,          //ROW COLUMN SET
    0x66668888,          //DELAY
    0x2000e010,          //cke enable

    0x2000e010,          //nop
    0x2000e010,          //precharge all
    0x2000e010,          //emrs2 enable hirgtep selfrefresh 
    0x2000e010,          //emrs3 
    0x2000e010,          //emrs1  enable dell
    0x2000e010,          //master mode reg ,reset dell
    0x2000e010,          //precharge all

    0x2000e010,          //auto refresh
    0x2000e010,          //auto refresh
    0x2000e010,          //master mode reg,without dell 

    //   0x2000e010,          //emrs1  calibration
    0x66668888,          //delay
    //   0x2000e010,          //emrs1  exit calibration
    0x2000e010,          //nop

    0x2000e00c,          //open auto refresh
    0x2000e020,          //open dll set

    0x2000e024,          //normal calibration
    0x66668888,          //delay

    0x88888888           //ending
};        

static UINT reg_value_ddr2_aspen3s_rtos[] = 
{
    0x00010100,            //delay, 0x66668888 value
    0x00005011,                                                  //change frequence, 0x08000004 value
    (0x0434 | (0x1<<21) | (0x1<<22) | (0x1<<28) | (0x1<<29)),    //uart initial, 0x20026000 value
    0x00010100,            //delay, 0x66668888 value

    ((0x20<<0) | (0xA<<8) | (0x5<<12) | (0xE<<16) | (0x5<<21)),  //=0x00ae5A20, set to slower, 0x2000e004 value
                                                                 //[7:0] tRFC_cfg, [11:8] tRCD_cfg, [15:12] tRP_cfg
                                                                 //[20:16] tRAS_cfg, [23:21] tRRD_cfg
    
    ((0xF<<0) | (0xF<<4) | (0x7<<8) | (0xF<<12) | (0x14<<16)),   //=0x0014f7ff, set to slower, 0x2000e008 value
                                                                 //[3:0] tWR_cfg, [7:4] tWTR_cfg, [10:8] tRTP_cfg
                                                                 //[15:12] tRTW_cfg, [23:16] tDQSQ_tQHS_cfg

	
    0x200,                 //0 bypass dcc, 0x2000e05c value
    (0x0<<5) | (0x0<<14) | (0x0<<18) | (0x0<<20) | (0x1<<30),    //io pad config, 1.8 ddr2 sstl output 12mA, cmd_fifo=16, 0x2000e078 value
    (1<<27),               //ien, 0x08000064 value

    ((0x11<<0) | (0x1<<8) | (0x1<<12) | (0x6<<16) | (0x1<<21)),  //=0x00261111, AC Timing Configure 1, 0x2000e004 value
                                                                 //[7:0] tRFC_cfg, [11:8] tRCD_cfg, [15:12] tRP_cfg
                                                                 //[20:16] tRAS_cfg, [23:21] tRRD_cfg

    ((0x2<<0) | (0x5<<4) | (0x2<<8) | (0x3<<12) | (0x14<<16)),   //=0x00143252, AC Timing Configure 2, 0x2000e008 value
                                                                 //[3:0] tWR_cfg, [7:4] tWTR_cfg, [10:8] tRTP_cfg
                                                                 //[15:12] tRTW_cfg, [23:16] tDQSQ_tQHS_cfg
    
    (0x0 | (0x2<<3)  | (0x2<<6) | (0x1<<9) | (0x3<<10) | (0x2<<13)), //ROW COLUMN SET, 32bit width, 0x2000e000 value
    0x00000100,                //delay, 0x66668888 value
    ((0x0<<26) | (0x1<<25)),   //=0x02000000, cke enable, 0x2000e010 value

    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<22) | (0x1<<21) | (0x1<<20)),  //nop, 0x2000e010 value
    // 0x00000100,                       //delay 400ns, 0x66668888 value
    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<21) | (0x1<<10)),    //precharge all, 0x2000e010 value
    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<17) | 0x00),         //emrs2 enable hirgtep selfrefresh, 0x2000e010 value
    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<17) | (0x1<<16)),    //emrs3, 0x2000e010 value
    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<16) | (0x1<<10) | ((0x0<<6) | (0x0<<2)) | (0x1<<1)),  //emrs1  enable dll, disable DQS#(AK98 only support single-end DQS!), disable ODT Rtt, Output reduced strength, 0x2000e010 value
    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<9)  | (0x1<<8) | (0x0<<7) | (0x3<<4) | (0x0<<3) | (0x2<<0)),  //low=0x332, master mode reg, reset dll, 0x2000e010 value
    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<21) | (0x1<<10) ),    //precharge all, 0x2000e010 value
    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<22)),      //auto refresh, 0x2000e010 value
    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<22)),      //auto refresh, 0x2000e010 value
    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<9)  | (0x0<<8) | (0x0<<7) | (0x3<<4) | (0x0<<3) | (0x2<<0)),  //low=0x232, master mode reg, without resetting dll, 0x2000e010 value
    // ( (0x1<<25) | (0x1<<23) | (0x1<<16) | (0x0384)),    //emrs1  calibration, 0x2000e010 value
    0x00000100,            //delay, 0x66668888 value
    // ( (0x1<<25) | (0x1<<23) | (0x1<<16) | (0x0004)),    //emrs1  exit calibration, 0x2000e010 value
    ((0x0<<27) | (0x1<<25) | (0x1<<23) | (0x1<<22) | (0x1<<21) | (0x1<<20)),  //nop, 0x2000e010 value

    ((0x3e8<<1) | (1<<0)), //open auto refresh, AC Timing Configure 3, 0x2000e00c value
    0x3,                   //open dll set, 0x2000e020 value

    (0x1<<0),              //normal calibration, 0x2000e024 value
    0x00010100,            //delay, 0x66668888 value

    0x00000000             //ending
};
//#else

//	0x0800000c,
//	0x66668888,
//	0x0800000c,
//	0x66668888,

static UINT  reg_addr_ddr2_aspen3s_linux[] = 
{
    /* update software, reset ddr2 and nand */
    0x66668888,                       //DELAY
    0x0800000c,
    0x66668888,
    0x0800000c,

    /* initializing system clock and uart baudrate */
    0x66668888,                       // DELAY
    0x08000004,
    0x20026000, 
    0x66668888,
    
    0x2000e018,
    0x2000e01c,
    
    /* initializing ram controller */
    0x2000e078,                       // RAM I/O controller
    0x2000e000,                       // memory controller register1: col, row, address width
    0x2000e004,                       // memory controller register2: timming
    0x2000e008,                       // memory controller register3: timming
    0x2000f084,                        // l2 cache data abort
    0x66668888,                       // delay register initializing finished

    /* enable and reset on-chip dll and wait until its output is stable */
    0x2000e05c,         // bypass dcc
    0x66668888,                       // wait dll stable
    0x2000e020,         // enable and reset dll
    0x66668888,         // wait dll output stable 
    0x2000e020,         // enable dll and set dll in normal state
    0x66668888,         // wait dll output stable

    /* initializing ddr2 timming */
    0x66668888,                       // delay at least 200us
    0x2000e010,         // take cke high
    0x2000e010,          
    0x2000e010,          
    0x2000e010,          
    0x2000e010,                       // send precharge all banks          
    0x2000e010,                     // emrs3       
    0x2000e010,         // emrs2
    0x2000e010,                       // emrs1 and enable dll         
    0x2000e010,             // mrs with reset dll
    0x2000e010,                       // send precharge all banks again          
    0x2000e010,          
    0x2000e010,
    0x2000e010,
    0x2000e010,                       // mrs without reset dll
    0x2000e010,                       // ocd calibration default mode
    0x2000e010,                       // ocd calibration default mode exit

    /* start normal calibration and wait until the operation is finished */
    0x2000e024,         // start normal calibration
    0x66668888,         // delay until wait for normaltion finished
    0x2000e00c,         // enable auto-refresh
    0x66668888,         // delay
    0x88888888,         // nand boot config finish
};

static UINT reg_value_ddr2_aspen3s_linux[] = 
{
    /* update software, reset ddr2 and nand */
    0x00010100,              //delay, 0x66668888 value
    0x44004de7,
    0x00010100,
    0x000009e7,

    /* initializing system clock and uart baudrate */
    0x00010100,              // delay
    0x0000d0ad,              // (cpu, mem, asic) = (360, 180, 90)MHz
    0x3020030c,              // asic90MHz
    0x00008000,
    
    0x000b0908,
    0xff3f1f0f,

    /* initializing ram controller */
    0x431540a0,              // master: hd control odt and set 75ohm, 16mA
    0x00004e90,              // ROW=13, COL=10, WL=2, RL=3, 32bit addr buss
    0x00282219,              // timming
    0x00143122,              // timming
    0x00000001,              // l2 cache data abort
    0x00001000,
    
    /* enable and reset on-chip dll and wait until its output is stable */
    0x00000200,              // start clock, bypass dcc
    0x00001000,
    0x00000001,              // enable and reset dll
    0x00001000,
    0x00000003,              // enable dll and set dll in normal state
    0x00008000,    

    /* initializing ddr2 timming */
    0x00010000,              // delay at least 200us
    0x02000000,              // take cke high
    0x0af00000,               // nop
    0x0af00000,               // nop
    0x0af00000,               // nop
    0x0aa00400,              // send precharge all banks
    0x0a820000,              // emrs2
    0x0a830000,              // emrs3
    0x0a810406,              // emrs1 and enable dll, odt=75ohm
    0x0a800532,              // mrs with reset dll, WR=3, CAS=3, BL=4
    0x0aa00400,              // send precharge all banks again
    0x0ac00000,              // send refresh(not auto-refresh)
    0x0ac00000,              // send refresh
    0x0ac00000,              // send refresh
    0x0a800432,              // mrs without reset dll
    0x0a810786,              // ocd calibration default mode, [a9:a7] = 111B
    0x0a810406,              // ocd calibration default mode exit, [a9:a7] = 000B
   
    /* start normal calibration and wait until the operation is finished */
    0x00000001,              // start auto calibration
    0x00001000,              // wait calibration finished
    0x00000ad5,              // tck = 5.5ns(mem=180M), trfei = 7.7us
    0x00001000,              // wait ddr2 initializing finished
    0x00000000,

};
//#endif


//linux
//AK39芯版本的内存参数地址
static UINT reg_addr_ddr_sunh[] =
{
	0x08000004,
    0x66668888,
    0x21000078,
    0x21000000,
    0x21000010,
    0x21000010,
    0x21000010,
    0x21000010,
    0x21000010,
    0x21000010,
    0x21000010,
    0x21000010,
    0x21000010,
    0x2100000C,
    0x21000020,
    0x66668888,
    0x21000024,
    0x66668888,
	0x88888888,
};

//linux
//AK39芯版本的内存参数值
static UINT reg_value_ddr_sunh[] =
{
	((1 << 24)|(1 << 20)|(1 << 17)|(1 << 14)|(2 << 12)|(2 << 8)|240),
	0xC8,
	((2 << 20)|(2 << 18)|(2 << 14)|(2 << 5)),
	((1 << 20)|(2 << 13)|(3 << 10)|(1 << 6)|(1 << 3)|1),
	(1 << 25),
	((1 << 25)|(1 << 23)|(1 << 22)|(1 << 21)|(1 << 20)),
	((1 << 25)|(1 << 23)|(1 << 21)|(1 << 10)),
	((1 << 25)|(1 << 23)|(1 << 16)),
	((1 << 25)|(1 << 23)|0x162),
	((1 << 25)|(1 << 23)|(1 << 21)|(1 << 10)),
	((1 << 25)|(1 << 23)|(1 << 22)),
	((1 << 25)|(1 << 23)|(1 << 22)),
	((1 << 25)|(1 << 23)|0x62),
	((0x464 << 1)|1),
	0x3,
	0x200,
	0x1,
	0x400,
	0x88888888,
};

static UINT reg_addr_ddr2_sunh[] =
{
	//0x08000008,
	//0x66668888,
	0x08000004,
	0x66668888,
	//0x20130000,
	
	0x21000078,
	0x21000004,
	0x21000008,
	0x21000000,
	
	0x21000010,
	0x21000010,
	0x21000010,
	0x21000010,
	
	0x21000010,
	0x21000010,
	0x21000010,
	0x21000010,
	
	0x21000010,
	0x21000010,
	0x21000010,
	0x66668888,
	
	0x21000010,
	0x21000010,
	0x21000010,
	0x2100000C,
	
	0x21000020,
	0x66668888,
	0x21000024,
	0x66668888,
	0x88888888,
};

static UINT reg_value_ddr2_sunh[] =
{
	//((1 << 23)|(1 <<17)|(2 << 12)|(2 << 8)|120),
	//0xC8,
	((1 << 24)|(1 << 20)|(1 << 17)|(1 << 14)|(1 << 12)|(3 << 8)|200),
	0xC8,
	//((1 << 29)|(1 << 28)|(1 << 21)|780),
	
	((3 << 20)|(3 << 18)|(2 << 16)|(3 << 14)|(2 << 7)|(3 << 5)),
	((0xf << 24)|(5 << 21)|(0xe << 6)|(5 << 12)|(3 << 8)|0x20),
	((0x14<<16)|(0xf<<12)|(0x5<<8)|(0x5<<4)|0x5),
	((0x1<<20)|(0x3<<16)|(0x2<<13)|(0x3<<10)|(0x1<<9)|(0x5<<6)|(0x5<<3)),
	(1 << 25),
	((1 << 25)|(1 << 23)|(1 << 22)|(1 << 21)|(1 << 20)),
	((1 << 25)|(1 << 23)|(1 << 21)|(1 << 10)),
	((1 << 25)|(1 << 23)|(1 << 17)),
	
	((0x1<<25)|(0x1<<23)|(0x1<<17)|(0x1<<16)),
	((0x1<<25)|(0x1<<23)|(0x1<<16)|(0x1<<10)|(0x1<<6)),
	((1 << 25)|(1 << 23)|0x162),
	((1 << 25)|(1 << 23)|(1 << 21)|(1 << 10)),
	
	((1 << 25)|(1 << 23)|(1 << 22)),
	((1 << 25)|(1 << 23)|(1 << 22)),
	((1 << 25)|(1 << 23)|0x62),
	0xC8,
	
	((0x1<<25)|(0x1<<23)|(0x1<<16)|(0x1<<10)|(0x7<<7)|(0x1<<6)),
	((0x1<<25)|(0x1<<23)|(0x1<<16)|(0x1<<10)|(0x1<<6)),
	((0x1<<25)|(0x1<<23)|(0x1<<22)|(0x1<<21)|(0x1<<20)),
	((0x545 << 1)|1),
	0x3,
	0xC8,
	0x1,
	0x7a120,
	0x88888888,
};



static UINT reg_addr_sdram_sun3[] = 
{
   0x08000078,
   0x2002d004, // ac row
   0x2002d000, // no operation, open mclk , 0x2002e010
   0x2002d000, // precharge all banks
   0x2002d000, // 8 auto refresh1
   0x2002d000, // 8 auto refresh2
   0x2002d000, // 8 auto refresh3
   0x2002d000, // 8 auto refresh4
   0x2002d000, // 8 auto refresh5
   0x2002d000, // 8 auto refresh6
   0x2002d000, // 8 auto refresh7
   0x2002d000, // 8 auto refresh8
   0x2002d000,  // load mode register
   0x2002d000,  // no operation
   0x66668888,
   0x88888888
};


//之前RTOS一直没出问题可能是烧录跑的60Mhz，系统跑起来有用sdram_on_change函数又对sdram时序做了调整。
//而Aimer37的Linux内核一直都是跑132Mhz，且没有sdram变频函数。
//所以，烧录工具里面还是要按最大跑140Mhz来算，CLK cycle=7ns。
//tWTR=2, tRAS=7, tWR=2, tRCD=2, tRFC=9, tRP=2
//寄存器默认值用0x12789253。

static UINT reg_value_sdram_sun3[] = 
{ 
   0x18401F7E,
   0x12789253|(1<<26), // ac row, AHB burst read number MUST BE 32Byte(会影响性能)
   0xc0170000, // no operation 
   0xc0120400, // precharge all banks
   0xc0110000,  // 8 auto refresh1
   0xc0110000,  // 8 auto refresh2
   0xc0110000,  // 8 auto refresh3
   0xc0110000,  // 8 auto refresh4
   0xc0110000,  // 8 auto refresh5
   0xc0110000,  // 8 auto refresh6
   0xc0110000,  // 8 auto refresh7
   0xc0110000,  // 8 auto refresh8
   0xc0100033, // lode mode register
   0xe0170000, // no operation
   0x0000000a,
   0x00000000
};

//pll1 = 4 * M / N; 
//baud rate = asic freq / (div_cnt + 1);
VOID config_aspen3_ddr_freq(T_RAM_REG *ram, UINT regCnt)
{
    UINT i;
    UINT tmpN = 0, tmpM = 0;    //pll1 & asic_freq change
    UINT div_cnt;       //uart change
    
    for(i=0; i<regCnt; i++)
    {
        //1: calculate frequence register
        if (0x080000dc == ram[i].reg_addr)
        {
            tmpN = (ram[i].reg_value >> 12) & 0xF;  //mark N, bit[15:12] of 0x080000dc
            tmpN += 1;     //000000:1 000001:2 ...
        }

        if (0x08000004 == ram[i].reg_addr)
        {
            //tmpM = (ram[i].value >> 0) & 0x3F;  //mark M, bit[5:0] of 0x08000004
            //tmpM += 45;     //000000:45 000001:46 ...
            tmpM = (theConfig.m_freq * 2 * tmpN / 4) - 45;
            ram[i].reg_value &= ~(0x3F);
            ram[i].reg_value |= tmpM;
        }

        //2: calculate baud rate
        if (0x20026000 == ram[i].reg_addr)
        {
            div_cnt = (theConfig.m_freq * 1000 * 1000 / DEFAULT_UART0_RATE) - 1;  //default baud rate is 115200
            ram[i].reg_value &= ~(0xFFFF);
            ram[i].reg_value |= div_cnt;
        }
    }
}

UINT config_aspen3s_ram_size(UINT ram_size)
{
    UINT ram_size_modify = ram_size;

    ram_size_modify &= (~(0x7f << 9));

    ram_size_modify |= ((theConfig.ram_param.row - 11) << 13);
    ram_size_modify |= ((theConfig.ram_param.column - 7) << 10);
    ram_size_modify |= ((theConfig.ram_param.banks - 4) / 4 << 9);

    return ram_size_modify;
}

//ddr and ddr2 sdram use the same configuration for bus width
void config_ddr_ddr2_data_width(BYTE ram_type, UINT *reg_value)
{
    if (RAM_TYPE_DDR2_16 == ram_type || RAM_TYPE_DDR16 == ram_type || RAM_TYPE_DDR_16_MOBILE  == ram_type )
    {
        *reg_value |= (1 << 2);
    }
    else if (RAM_TYPE_DDR2_32 == ram_type || RAM_TYPE_DDR32 == ram_type || RAM_TYPE_DDR_32_MOBILE == ram_type)
    {
        *reg_value &= (~(1 << 2));
    }


}

VOID config_aspen3s_ddr_freq(UINT *freq, UINT *uart)
{
    UINT pll, asic_freq;
    UINT uart_div;

    *freq = 0;                //clear clock register value
    
    pll = (theConfig.m_freq * 2 - 180) / 4;
    
    //if mem freq higher than 132Mhz, asic freq = mem_freq/2
    //else asic_freq = mem_freq
    if (theConfig.m_freq > 132)
    {
        asic_freq = theConfig.m_freq / 2;
        *freq |= (0x2 << 6);    //asic div is 4
    }
    else
    {
        asic_freq = theConfig.m_freq;
    }

    *freq |=(0x5 << 12);    //enable pll, asic clock and mem clock to be change
    *freq |= pll;

    uart_div = asic_freq * 1000000 / DEFAULT_UART0_RATE;

    *uart &= (~0xffff);
    *uart |= uart_div;
}

void modify_ram_size(UINT *reg_value)
{
    UINT ram_size = theConfig.ram_param.size;

    *reg_value &= ~0x7;

    if(4 == ram_size)
    {
        *reg_value |= 0x1;
    }
    else if(8 == ram_size)
    {
        *reg_value |= 0x2;
    }
    else if(16 == ram_size)
    {
        *reg_value |= 0x3;
    }
    else if(32 == ram_size)
    {
        *reg_value |= 0x4;
    }
    else if(64 == ram_size)
    {
        *reg_value |= 0x5;
    }

  
}


#if 0
#define CONFIG_VALUE_LENGTH 10  //length of both 0x2000e008 and 0x0014f7ff is 10

//import ram initial config from "init_ramconfig.txt"
//return regCnt
unsigned long import_ram_config(T_RAM_REG *ram)
{
    char buf[128];
    unsigned long regCnt, i, k;
    CString str;
    HANDLE hFile;
    
    hFile = CreateFile(theApp.ConvertAbsolutePath(_T("init_ramconfig.txt")), GENERIC_READ, FILE_SHARE_READ, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if(INVALID_HANDLE_VALUE == hFile)
    {
	    CloseHandle(hFile);
	    return 0;
    }


    k = GetFileSize(hFile, NULL);

    if ((k % 24) != 0)
    {
        str.Format(_T("config文件不符合标准，长度为%d"), k);
        AfxMessageBox(str);

	    CloseHandle(hFile);
	    return 0;
    }
    else
    {
        regCnt = k / 24;
        str.Format(_T("config文件符合标准，寄存器个数为%d"), regCnt);
        AfxMessageBox(str);
    }

    for(i=0; i<regCnt; i++)
    {
        unsigned long read_len;
        BOOL bResult = ReadFile(hFile, buf, CONFIG_VALUE_LENGTH, &read_len, NULL);

        if(!bResult)
        {
            CloseHandle(hFile);
            return 0;
        }

        //str.Format(_T("rl:%d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x"), read_len, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
        //AfxMessageBox(str);

        buf[10] = '\0';

        //1: get register address
        k = theConfig.hex2int(buf);
        ram[i].reg_addr = k;

        //str.Format(_T("buf1 as 0x%08x"), k);
        //AfxMessageBox(str);

        SetFilePointer(hFile, 2, NULL, FILE_CURRENT);

        bResult = ReadFile(hFile, buf, CONFIG_VALUE_LENGTH, &read_len, NULL);

        if(!bResult)
        {
            CloseHandle(hFile);
            return 0;
        }
        
        //str.Format(_T("al:%d, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x"), read_len, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], buf[9]);
        //AfxMessageBox(str);

        buf[10] = '\0';

        //2: get register value
        k = theConfig.hex2int(buf);
        ram[i].reg_value = k;

        //str.Format(_T("buf2 as 0x%08x"), k);
        //AfxMessageBox(str);

        SetFilePointer(hFile, 2, NULL, FILE_CURRENT);
    }

    CloseHandle(hFile);
    return regCnt;
}

//export ram final config to "final_ramconfig.txt"
void export_ram_config(int regCnt, T_RAM_REG *ram)
{
	FILE *pWFile;
    char buf[128];
    int i;

    pWFile = fopen("final_ramconfig.txt", "w");
	if(pWFile == NULL)
	{
        return;
	}

    for(i=0; i<regCnt; i++)
    {
        sprintf(buf, "0x%08x, 0x%08x\n", ram[i].reg_addr, ram[i].reg_value);
        fwrite(buf, 1, strlen(buf), pWFile);

    }
    fclose(pWFile);
}

//#define MEMORY_CONFIG_BY_TXT  //use *.txt to import and export ram config

#else

#include <string.h>

typedef enum 
{
	RAM_CONFIG_FILE_END = 0,
	RAM_CONFIG_DATA,
	RAM_CONFIG_BRAND,
	RAM_CONFIG_REMARK,
	RAM_CONFIG_INANITION,
	RAM_CONFIG_FILE_ERROR
}E_PARSE_LINE_FLAG;

typedef struct
{
	char				* str_op;
	E_PARSE_LINE_FLAG	flag;
}T_CHECK_OPERATION;

static T_CHECK_OPERATION check_op[] =
	{
		{"CONFIG_DATA", RAM_CONFIG_DATA},
		{"CONFIG_BRAND", RAM_CONFIG_BRAND}
	};

#define RAM_REG_COUNT				128
#define RAM_CONFIG_FILE_LENGTH_MAX	(1024*10)  //10K bytes


static struct 
{
	T_RAM_REG ram[RAM_REG_COUNT] ;
	UINT	  uRegCount;
	BOOL      bValid;
}stRamData;

static char * get2line_end(char *buf)
{
	while((0 != buf[0]) && ((0x0d != buf[0]) &&(0x0a != buf[1])))
	{
		++buf;
	}
	if(0 != buf[0])
	{
		buf += 2;
	}
	
	return buf;
}

static BOOL check_operation(char *buf, E_PARSE_LINE_FLAG * flag)
{
	char	tmpbuf[64];
	int		i;


	++buf;  //pass '['
	while((0x20 == buf[0]) || (0x09 == buf[0])) ++buf;    //remove space & tab

	for(i=0 ; i < 64; ++i)
	{
		if((']' == buf[i]) || (0 == buf[i]) || (0x0d == buf[i]))
			break;
		tmpbuf[i] = buf[i];
	}

	if((64 == i) || (']' != buf[i]))
	{
		*flag = RAM_CONFIG_FILE_ERROR;
		return FALSE;
	}
	
	tmpbuf[i] = 0;
	
	for(i=0 ; i < sizeof(check_op)/sizeof(check_op[0]); ++i)
	{
		if(!strncmp(tmpbuf, check_op[i].str_op, strlen(check_op[i].str_op)))
		{
			*flag = check_op[i].flag;
			return TRUE;
		}
	}

	*flag = RAM_CONFIG_FILE_ERROR;
	return FALSE;
}


static char * parseline(char *buf, E_PARSE_LINE_FLAG * flag) //zero end buf
{
	switch(buf[0])
	{
	case '#':
	case '/':
		*flag = RAM_CONFIG_REMARK;
		break;
	case '[':
		check_operation(buf, flag);
		break;
	case 0:
		*flag = RAM_CONFIG_FILE_END;
		break;
	case 0x0d:								//enter line
		*flag = RAM_CONFIG_INANITION;
		break;
	default:
		*flag = RAM_CONFIG_FILE_ERROR;
		break;
	}

	return get2line_end(buf);
}


static char * get_ram_config_data(char *buf, BOOL * bRet)
{
	char tmpbuf[32];
	int  i;

	*bRet = FALSE;
	stRamData.bValid = FALSE;
	stRamData.uRegCount = 0;

	while('[' != buf[0])
	{
		ZeroMemory(tmpbuf,32);
		for(i=0;i<32;++i)
		{
			if((0x0d == buf[0]) || (0 == buf[0]))
				break;
			tmpbuf[i] = buf[0];
			++buf;
		}

		if((',' != tmpbuf[10]) &&(',' != tmpbuf[22]))
		{
			return buf;
		}
		
		tmpbuf[10] = tmpbuf[22] = 0;

		stRamData.ram[stRamData.uRegCount].reg_addr = theConfig.hex2int(&tmpbuf[0]);
		stRamData.ram[stRamData.uRegCount].reg_value = theConfig.hex2int(&tmpbuf[11]);
		stRamData.uRegCount ++ ;
		
		if(stRamData.uRegCount >= RAM_REG_COUNT)
			return buf;
		
		buf = get2line_end(buf);		
	}

	buf = get2line_end(buf);            //TO next line
	stRamData.bValid = TRUE;			//stRamData.uRegCount maybe need check
	*bRet = TRUE;

	return buf;
}



BOOL import_ram_config(LPCTSTR lpFileName)
{
	HANDLE	hFile;
	DWORD	dFileLen;
	char	FileBuf[RAM_CONFIG_FILE_LENGTH_MAX+4] = {0}; //must init
	char	*pBuf;
	E_PARSE_LINE_FLAG  flag;
	BOOL	bRet;


	hFile = CreateFile(lpFileName, GENERIC_READ,
					0, (LPSECURITY_ATTRIBUTES) NULL,
					OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
					(HANDLE) NULL);

	if(INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	dFileLen = GetFileSize(hFile, NULL);
	if((INVALID_FILE_SIZE == dFileLen) || (dFileLen > RAM_CONFIG_FILE_LENGTH_MAX))
	{
		CloseHandle(hFile);
		return FALSE;
	}

	DWORD LenRead = 0;

	if(!(ReadFile(hFile,FileBuf,dFileLen, &LenRead, NULL) && (dFileLen == LenRead)))
	{
		CloseHandle(hFile);
		return FALSE;		
	}

	CloseHandle(hFile);  //no need , close

	
	pBuf = FileBuf;
	do
	{
		bRet = TRUE;
		pBuf = parseline(pBuf, &flag);
		switch(flag)
		{
		case RAM_CONFIG_INANITION:

			break;
		case RAM_CONFIG_REMARK:

			break;
		case RAM_CONFIG_DATA:
			pBuf = get_ram_config_data(pBuf,&bRet);
			break;
		case RAM_CONFIG_FILE_ERROR:
			bRet = FALSE;
			break;
		case RAM_CONFIG_FILE_END:

			break;
		default:
			break;
		}

	}while(bRet && (RAM_CONFIG_FILE_END != flag));


	return bRet;
}



BOOL export_ram_config(LPCTSTR lpFileName, UINT ui_data)
{
	FILE  *pFile;
	UINT  i;
	T_RAM_REG ram_reg[64];

	if(!stRamData.bValid)
	{
		UINT *pAddr = NULL;
		UINT *pValue = NULL;
		UINT p = ui_data & 0x03;
#if 0
		switch(ui_data & 0x03)
		{
		case 0:
			//LINUX PLANFORM
			if (theConfig.planform_tpye == E_LINUX_PLANFORM)
			{
				stRamData.uRegCount = 
					sizeof(reg_addr_ddr2_aspen3s_linux)/sizeof(reg_addr_ddr2_aspen3s_linux[0]);
				pAddr = reg_addr_ddr2_aspen3s_linux;
				pValue = reg_value_ddr2_aspen3s_linux;
			}

			//RTOS PLANFORM
			if (theConfig.planform_tpye == E_ROST_PLANFORM)
			{
				stRamData.uRegCount = 
					sizeof(reg_addr_ddr2_aspen3s_rtos)/sizeof(reg_addr_ddr2_aspen3s_rtos[0]);
				pAddr = reg_addr_ddr2_aspen3s_rtos;
				pValue = reg_value_ddr2_aspen3s_rtos;
			}
			break;
		case 1:
			stRamData.uRegCount = 
				sizeof(reg_addr_ddr_aspen3s)/sizeof(reg_addr_ddr_aspen3s[0]);
			pAddr = reg_addr_ddr_aspen3s;
			pValue = reg_value_ddr_aspen3s;
			break;
		case 2:
		case 3:
/*
			stRamData.uRegCount = 
				sizeof(reg_addr_sdr_aspen3s)/sizeof(reg_addr_sdr_aspen3s[0]);
			pAddr = reg_addr_sdr_aspen3s;
			pValue = reg_value_sdr_aspen3s;
			break;
*/	
			stRamData.uRegCount = 
				sizeof(reg_addr_m_ddr_aspen3s)/sizeof(reg_addr_m_ddr_aspen3s[0]);
			pAddr = reg_addr_m_ddr_aspen3s;
			pValue = reg_value_m_ddr_aspen3s;
			break;

		default:
			return FALSE;
			break;
		}
#endif
		stRamData.uRegCount = config_ram_param(ram_reg);

		for(i=0; i < stRamData.uRegCount; ++i)
		{
			stRamData.ram[i].reg_addr = ram_reg[i].reg_addr; //pAddr[i];
			stRamData.ram[i].reg_value = ram_reg[i].reg_value; //pValue[i];
		}	
	}

	pFile = _wfopen(lpFileName, _T("w"));
	if(NULL ==  pFile)
	{
		return FALSE;
	}

	fprintf(pFile,"# '#' and '//' is the remark head.\n");
	fprintf(pFile,"// '#' and '//' is the remark head.\n");
	fprintf(pFile,"# blank line is allowable,but couldn't "	\
		"contain blank(space)\n\n\n\n");
	fprintf(pFile,
		"# Every section should be like the format followed:\n"	\
		"# [section name]       //no blank before '['\n"	\
		"# Your data...\n"	\
		"# Your data...\n"	\
		"# ...\n"	\
		"# [END]                //no blank before '['\n");
	
	fprintf(pFile,
		"# The following section name is supported:\n"	\
		"# [xxxx],[xxxx]\n\n");
	
	
	fprintf(pFile,"[CONFIG_DATA]\n");
	for(i=0 ; i < stRamData.uRegCount; ++i)
	{
		if(0x2000e000 == stRamData.ram[i].reg_addr)
		{
			stRamData.ram[i].reg_value &= 
				~(7 | (1<<9) | (7<<10) | (7<<13));
//			fprintf(pFile,"\n0x%08x\n0x%08x\n", stRamData.ram[i].reg_value,ui_data);
			stRamData.ram[i].reg_value |= ui_data;
		}

		fprintf(pFile,"0x%08x, 0x%08x,    //remark\n",
			stRamData.ram[i].reg_addr,
			stRamData.ram[i].reg_value);
	}
	fprintf(pFile,"[END]\n");


	fprintf(pFile,"\n\n\n\n# ^_^ ^_^\n");
	fclose(pFile);
	return TRUE;
}


UINT get_import_ram_config(T_RAM_REG *ram)
{
	UINT i;

	if(!stRamData.bValid)
	{
		return 0;
	}
	
	for(i=0; i < stRamData.uRegCount; ++i)
	{
		ram[i] = stRamData.ram[i];
	}

	return stRamData.uRegCount;
}

BOOL get_UI_data(UINT *data)
{
	UINT i;
	
	if(!stRamData.bValid)
	{
		return FALSE;
	}

	for(i=0; i < stRamData.uRegCount; ++i)
	{
		if(theConfig.chip_type == CHIP_980X && 0x2000e000 == stRamData.ram[i].reg_addr)  //JUST FOR AK98
		{
			*data = stRamData.ram[i].reg_value;
			return TRUE;
		}
		if(theConfig.chip_type == CHIP_39XX && 0x21000000 == stRamData.ram[i].reg_addr)  //JUST FOR AK98
		{
			*data = stRamData.ram[i].reg_value;
			return TRUE;
		}
	}

	return FALSE;
}


#endif

UINT config_ram_param(T_RAM_REG *ram)
{
    int i;
    int regCnt = 0;
    int indSize = 0, index1 = 0, index2 = 0;
    BYTE ram_type;
    UINT ram_size;
	CString strTmp;

    ram_type = theConfig.ram_param.type;
    ram_size = theConfig.ram_param.size;

	if (CHIP_980X == theConfig.chip_type || CHIP_39XX == theConfig.chip_type)
	{
	    regCnt = get_import_ram_config(ram);
	}
	
	if (0 == regCnt)
	{
		if((RAM_TYPE_DDR16 == ram_type) || (RAM_TYPE_DDR32 == ram_type))
		{
			strTmp.Format(_T(RAM_CFG_DDR));
		}
		else if ((RAM_TYPE_DDR2_16 == ram_type) || (RAM_TYPE_DDR2_32 == ram_type))
		{
			strTmp.Format(_T(RAM_CFG_DDR2));
		}
		else if ((RAM_TYPE_DDR_16_MOBILE == ram_type) || (RAM_TYPE_DDR_32_MOBILE == ram_type))
		{
			strTmp.Format(_T(RAM_CFG_mDDR));
		}
		else if (RAM_TYPE_MCP == ram_type)
		{
			strTmp.Format(_T(RAM_CFG_MCP));
		}
		else if (RAM_TYPE_SDR == ram_type)
		{
			strTmp.Format(_T(RAM_CFG_SDR));
		}

		if (import_ram_config(theApp.ConvertAbsolutePath(strTmp)))
		{
			regCnt = get_import_ram_config(ram);
		}
	}

    if (CHIP_37XX == theConfig.chip_type || CHIP_37XX_L == theConfig.chip_type)
    {
        if (0 == regCnt)
        {
            regCnt = sizeof(reg_addr_sdram_sun3) / sizeof(UINT);

            for(i=0; i<regCnt; i++)
            {
                ram[i].reg_addr = reg_addr_sdram_sun3[i]; 
                ram[i].reg_value = reg_value_sdram_sun3[i];
            }
        }

        for(i=0; i<regCnt; i++)
        {
            if (0x2002d004 == ram[i].reg_addr)
            {
                modify_ram_size(&ram[i].reg_value);
            }
        }
    }
	else if (CHIP_39XX == theConfig.chip_type)
    {
		 if(RAM_TYPE_DDR16 == ram_type || RAM_TYPE_DDR32 == ram_type) 
		 {
			if (0 == regCnt)
			{
				regCnt = sizeof(reg_addr_ddr_sunh) / sizeof(UINT);
				
				for(i=0; i<regCnt; i++)
				{
					ram[i].reg_addr = reg_addr_ddr_sunh[i]; 
					ram[i].reg_value = reg_value_ddr_sunh[i];
				}
			}
		 }
		 else if(RAM_TYPE_DDR2_16 == ram_type || RAM_TYPE_DDR2_32 == ram_type) 
		 {
			 if (0 == regCnt)
			 {
				 regCnt = sizeof(reg_value_ddr2_sunh) / sizeof(UINT);
				 
				 for(i=0; i<regCnt; i++)
				 {
					 ram[i].reg_addr = reg_addr_ddr2_sunh[i]; 
					 ram[i].reg_value = reg_value_ddr2_sunh[i];
				 }
			 }
		 }
		 for(i=0; i<regCnt; i++)
		 {
			 if (0x21000000 == ram[i].reg_addr)
			 {
				 indSize = i;
			 }
		 }
		 ram[indSize].reg_value = config_aspen3s_ram_size(ram[indSize].reg_value);
    }
    else if (CHIP_980X == theConfig.chip_type)
    {
       //copy ram param from global data depends on the ram type
        if(RAM_TYPE_DDR16 == ram_type || RAM_TYPE_DDR32 == ram_type)                //DDR
        {
            if (0 == regCnt)
            {
                regCnt = sizeof(reg_addr_ddr_aspen3s)/sizeof(UINT);
            
                for(i=0; i<regCnt; i++)
                {
                    ram[i].reg_addr = reg_addr_ddr_aspen3s[i]; 
                    ram[i].reg_value = reg_value_ddr_aspen3s[i];
                }
            }

            for(i=0; i<regCnt; i++)
            {
                if (0x08000004 == ram[i].reg_addr)
                {
                    index1 = i;
                }

                if (0x20026000 == ram[i].reg_addr)
                {
                    index2 = i;
                }
                
                if (0x2000e000 == ram[i].reg_addr)
                {
                    indSize = i;
                }
            }

            //index1 is mem freq config, index2 is uart config    
            config_aspen3s_ddr_freq(&(ram[index1].reg_value), &(ram[index2].reg_value));

			config_ddr_ddr2_data_width(ram_type, &(ram[indSize].reg_value));
        }

		else if(RAM_TYPE_DDR_16_MOBILE == ram_type || RAM_TYPE_DDR_32_MOBILE == ram_type)                //DDR
        {
            if (0 == regCnt)
            {
                regCnt = sizeof(reg_addr_m_ddr_aspen3s)/sizeof(UINT);
				
                for(i=0; i<regCnt; i++)
                {
                    ram[i].reg_addr = reg_addr_m_ddr_aspen3s[i]; 
                    ram[i].reg_value = reg_value_m_ddr_aspen3s[i];
                }
            }
			
            for(i=0; i<regCnt; i++)
            {
                if (0x08000004 == ram[i].reg_addr)
                {
                    index1 = i;
                }
				
                if (0x20026000 == ram[i].reg_addr)
                {
                    index2 = i;
                }
                
                if (0x2000e000 == ram[i].reg_addr)
                {
                    indSize = i;
                }
            }
			
            //index1 is mem freq config, index2 is uart config    
            //config_aspen3s_ddr_freq(&(ram[index1].reg_value), &(ram[index2].reg_value));
			
			config_ddr_ddr2_data_width(ram_type, &(ram[indSize].reg_value));
        }
        else if(RAM_TYPE_DDR2_32 == ram_type || RAM_TYPE_DDR2_16 == ram_type)                //DDR2
        {
            if (0 == regCnt)
            {
				//LINUX PLANFORM
				if (theConfig.planform_tpye == E_LINUX_PLANFORM)
				{
					regCnt = sizeof(reg_addr_ddr2_aspen3s_linux)/sizeof(UINT);
            
					for(i=0; i<regCnt; i++)
					{
						ram[i].reg_addr = reg_addr_ddr2_aspen3s_linux[i]; 
						ram[i].reg_value = reg_value_ddr2_aspen3s_linux[i];
					}
				}

				//rtos PLANFORM
				if (theConfig.planform_tpye == E_ROST_PLANFORM)
				{
					regCnt = sizeof(reg_addr_ddr2_aspen3s_rtos)/sizeof(UINT);
					
					for(i=0; i<regCnt; i++)
					{
						ram[i].reg_addr = reg_addr_ddr2_aspen3s_rtos[i]; 
						ram[i].reg_value = reg_value_ddr2_aspen3s_rtos[i];
					}
				}
            }

            for(i=0; i<regCnt; i++)
            {
                if (0x08000004 == ram[i].reg_addr)
                {
                    index1 = i;
                }

                if (0x20026000 == ram[i].reg_addr)
                {
                    index2 = i;
                }
                
                if (0x2000e000 == ram[i].reg_addr)
                {
                    indSize = i;
                }
            }

            //index1 is mem freq config, index2 is uart config
			if (theConfig.planform_tpye == E_ROST_PLANFORM)
			{
				 config_aspen3s_ddr_freq(&(ram[index1].reg_value), &(ram[index2].reg_value));
			}
            
            config_ddr_ddr2_data_width(ram_type, &(ram[indSize].reg_value));
        }
        else                                                    //SDRAM
        {
            if (0 == regCnt)
            {
                regCnt = sizeof(reg_addr_sdr_aspen3s)/sizeof(UINT);
            
                for(i=0; i<regCnt; i++)
                {
                    ram[i].reg_addr = reg_addr_sdr_aspen3s[i]; 
                    ram[i].reg_value = reg_value_sdr_aspen3s[i];
                }
            }

            for(i=0; i<regCnt; i++)
            {
                if (0x2000e000 == ram[i].reg_addr)
                {
                    indSize = i;
                }
            }
        }
        
        ram[indSize].reg_value = config_aspen3s_ram_size(ram[indSize].reg_value);

    }
    else    //for ak880x
    {
        //copy ram param from global data depends on the ram type
        if (RAM_TYPE_MCP == ram_type)                    //MCP
        {
            if (0 == regCnt)
            {
                regCnt = sizeof(reg_addr_mcp)/sizeof(UINT);
            
                for(i=0; i<regCnt; i++)
                {
                    ram[i].reg_addr = reg_addr_mcp[i]; 
                    ram[i].reg_value = reg_value_mcp[i];
                }
            }

            for(i=0; i<regCnt; i++)
            {
                if (0x2002d004 == ram[i].reg_addr)
                {
                    indSize = i;
                }
            }
        }
        else if(RAM_TYPE_DDR16 == ram_type)                //DDR
        {
            if (0 == regCnt)
            {
                regCnt = sizeof(reg_addr_ddr)/sizeof(UINT);

                for(i=0; i<regCnt; i++)
                {
                    ram[i].reg_addr = reg_addr_ddr[i]; 
                    ram[i].reg_value = reg_value_ddr[i];
                }
            }

            for(i=0; i<regCnt; i++)
            {
                if (0x08000004 == ram[i].reg_addr)
                {
                    index1 = i;
                }

                if (0x2002d004 == ram[i].reg_addr)
                {
                    indSize = i;
                }
            }
 
            config_aspen3_ddr_freq(ram, regCnt);
            
            if(CHIP_780X == theConfig.chip_type)
            {
                ram[index1].reg_value = 0x00021000;
            }
        }
        else                                                    //SDRAM
        {
            if (0 == regCnt)
            {
                regCnt = sizeof(reg_addr_sdr)/sizeof(UINT);
                indSize = 1;
            
                for(i=0; i<regCnt; i++)
                {
                    ram[i].reg_addr = reg_addr_sdr[i]; 
                    ram[i].reg_value = reg_value_sdr[i];
                }
            }
            
            for(i=0; i<regCnt; i++)
            {
                if (0x2002d008 == ram[i].reg_addr)
                {
                    index1 = i;
                }

                if (0x2002d004 == ram[i].reg_addr)
                {
                    indSize = i;
                }
            }

            if(CHIP_880X == theConfig.chip_type)
            {
                ram[index1].reg_value |= (1<<25);    //for AK880x, config ram type, default is DDR type
            }
        }

        modify_ram_size(&(ram[indSize].reg_value));
        
    }

    return regCnt;
}
