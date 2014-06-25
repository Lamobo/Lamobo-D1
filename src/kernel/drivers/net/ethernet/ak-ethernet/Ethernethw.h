#ifndef _MAC_REG_DEFINE_H_
#define _MAC_REG_DEFINE_H_
#define MAC_REG_BASE				0x60000000

#define REG_MASTER_CTRL                        ( 0x1400)  /* WORD reg */
    #define MASTER_CTRL_MAC_SOFT_RST_OFF        0       /* hw sc */
    #define MASTER_CTRL_MAC_SOFT_RST_BITS       1
    #define MASTER_CTRL_PCIE_SOFT_RST_OFF       1       /* hw sc */
    #define MASTER_CTRL_PCIE_SOFT_RST_BITS      1
    #define MASTER_CTRL_PCIE_TEST_MOD_OFF       2
    #define MASTER_CTRL_PCIE_TEST_MOD_BITS      2
    #define MASTER_CTRL_BERT_START_OFF          4
    #define MASTER_CTRL_BERT_START_BITS         1
    #define MASTER_CTRL_OOB_DIS_OFF             6
    #define MASTER_CTRL_OOB_DIS_BITS            1       
    #define MASTER_CTRL_SA_TIMER_EN_OFF         7
    #define MASTER_CTRL_SA_TIMER_EN_BITS        1
    #define MASTER_CTRL_MANUAL_TIME_EN_OFF      8
    #define MASTER_CTRL_MANUAL_TIMER_EN_BITS    1
    #define MASTER_CTRL_MANUAL_INT_OFF          9
    #define MASTER_CTRL_MANUAL_INT_BITS         1
    #define MASTER_CTRL_IRQ_MODRT_EN_OFF        10      /* tx if 2 timer */
    #define MASTER_CTRL_IRQ_MODRT_EN_BITS       1
    #define MASTER_CTRL_IRQ_MODRT_RXEN_OFF      11      /* rx if 2 timer */
    #define MASTER_CTRL_IRQ_MODRT_RXEN_BITS     1
    #define MASTER_CTRL_PCLK_SEL_DIS_OFF        12      /* ps may set it */
    #define MASTER_CTRL_PCLK_SEL_DIS_BITS       1
    #define MASTER_CTRL_CLKSW_MODE_OFF          13
    #define MASTER_CTRL_CLKSW_MODE_BITS         1
    #define MASTER_CTRL_INT_RCLR_EN_OFF         14
    #define MASTER_CTRL_INT_CLCR_EN_BITS        1
    #define MASTER_CTRL_OTP_SEL_OFF             31
    #define MASTER_CTRL_OTP_SEL_BITS            1

#define REG_DEV_REV_NUM                         (0x1402)  /* BYTE reg */
#define REG_DEV_ID_NUM                          (0x1403)  /* BYTE reg */

#define REG_MANUAL_TIMER_INIT                   (0x1404)  /* DWORD reg */

#define REG_IRQ_MODRT_INIT                      (0x1408)  /* WORD reg */
#define REG_IRQ_MODRT_RX_INIT                   (0x140A)  /* WORD reg */

#define REG_PHY_CTRL                            (0x140C)  /* DWORD reg */
    #define PHY_CTRL_EXIT_RST_OFF               0
    #define PHY_CTRL_EXIT_RST_BITS              1
    #define PHY_CTRL_RTL_MOD_OFF                1
    #define PHY_CTRL_RTL_MOD_BITS               1
    #define PHY_CTRL_LED_MOD_OFF                2
    #define PHY_CTRL_LED_MOD_BITS               1
    #define PHY_CTRL_ANEG_NOW_OFF               3
    #define PHY_CTRL_ANEG_NOW_BITS              1
    #define PHY_CTRL_REV_ANEG_OFF               4
    #define PHY_CTRL_REV_ANEG_BITS              1
    #define PHY_CTRL_GATE25M_EN_OFF             5
    #define PHY_CTRL_GATE25M_EN_BITS            1
    #define PHY_CTRL_LPW_EXIT_OFF               6
    #define PHY_CTRL_LPW_EXIT_BITS              1
    #define PHY_CTRL_PHY_IDDQ_OFF               7
    #define PHY_CTRL_PHY_IDDQ_BITS              1
    #define PHY_CTRL_PHY_IDDQ_DIS_OFF           8
    #define PHY_CTRL_PHY_IDDQ_DIS_BITS          1
    #define PHY_CTRL_GIGA_DIS_OFF               9
    #define PHY_CTRL_GIGA_DIS_BITS              1
    #define PHY_CTRL_HIB_EN_HW_OFF              10
    #define PHY_CTRL_HIB_EN_HW_BITS             1
    #define PHY_CTRL_HIB_PULSE_HW_OFF           11
    #define PHY_CTRL_HIB_PULSE_HW_BITS          1
    #define PHY_CTRL_SEL_ANA_RST_OFF            12
    #define PHY_CTRL_SEL_ANA_RST_BITS           1
    #define PHY_CTRL_PHY_PLL_ON_OFF             13
    #define PHY_CTRL_PHY_PLL_ON_BITS            1
    #define PHY_CTRL_POWERDOWN_HW_OFF           14
    #define PHY_CTRL_POWERDOWN_HW_BITS          1
    #define PHY_CTRL_PHY_PLL_BYPASS_OFF         15
    #define PHY_CTRL_PHY_PLL_BYPASS_BITS        1

#ifdef EN_HIB
    #define PHY_CTRL_DEFAULT    (\
        FLAG(PHY_CTRL_SEL_ANA_RST_OFF)  |\
        FLAG(PHY_CTRL_HIB_EN_HW_OFF)    |\
        FLAG(PHY_CTRL_HIB_PULSE_HW_OFF) )
#else
    #define PHY_CTRL_DEFAULT    (\
        FLAG(PHY_CTRL_SEL_ANA_RST_OFF)  |\
        FLAG(PHY_CTRL_HIB_PULSE_HW_OFF) )
#endif/*EN_HIB*/
        
    #define PHY_CTRL_POWER_SAVING   (\
        FLAG(PHY_CTRL_SEL_ANA_RST_OFF)  |\
        FLAG(PHY_CTRL_HIB_EN_HW_OFF)    |\
        FLAG(PHY_CTRL_HIB_PULSE_HW_OFF) |\
        FLAG(PHY_CTRL_POWERDOWN_HW_OFF) |\
        FLAG(PHY_CTRL_PHY_IDDQ_OFF) )
    
    
#define REG_IDLE_STATUS                         (0x1410)  /* BYTE reg */
    #define IDLE_STATUS_RXMAC_OFF               0
    #define IDLE_STATUS_RXMAC_BITS              1
    #define IDLE_STATUS_TXMAC_OFF               1
    #define IDLE_STATUS_TXMAC_BITS              1
    #define IDLE_STATUS_RXQ_OFF                 2
    #define IDLE_STATUS_RXQ_BITS                1
    #define IDLE_STATUS_TXQ_OFF                 3
    #define IDLE_STATUS_TXQ_BITS                1

#define REG_MDIO_CTRL                           (0x1414)  /* DWORD reg */
    #define MDIO_CTRL_DATA(val)             (val&0xFFFF)    
    #define MDIO_CTRL_REG_ADDR(val)         ((val&0x1f) << 16)
    #define MDIO_CTRL_REG_ADDR_BITS             5
	#define MDIO_CTRL_WRITE					(0<<21)
	#define MDIO_CTRL_READ					(1<<21)/*read:0, write:1*/
    #define MDIO_CTRL_RW_OFF                    21  
    #define MDIO_CTRL_RW_BITS                   1
    #define MDIO_CTRL_SUP_PREAMBLE          (1<<22)
    #define MDIO_CTRL_SUP_PREAMBLE_BITS         1
    #define MDIO_CTRL_START                 (1<<23)
    #define MDIO_CTRL_START_BITS                1   /* sc */
    #define MDIO_CTRL_CLK_SEL_OFF               24
    #define MDIO_CTRL_CLK_SEL_BITS              3
    #define MDIO_CTRL_BUSY_OFF                  27
    #define MDIO_CTRL_BUSY_BITS                 1
    #define MDIO_CTRL_AP_EN_OFF                 28
    #define MDIO_CTRL_AP_EN_BITS                1
    #define MDIO_CLK_25_4                       0
    #define MDIO_CLK_25_6                       2
    #define MDIO_CLK_25_8                       3
    #define MDIO_CLK_25_10                      4
    #define MDIO_CLK_25_14                      5
    #define MDIO_CLK_25_20                      6
    #define MDIO_CLK_25_28                      7
    #define MDIO_MAX_AC_TIMER                   100 /* 1 ms */
    #define MDIO_CTRL_POST_READ_OFF             29
    #define MDIO_CTRL_POST_READ_BITS            1
    #define MDIO_CTRL_MODE_OFF                  30
    #define MDIO_CTRL_MODE_BITS                 1
    #define MDIO_CTRL_MODE_OLD                  0
    #define MDIO_CTRL_MODE_EXTENSION            1

    
#define REG_PHY_STATUS                          (0x1418)  /* DWORD reg */
    #define PHY_STATUS_PHY_STATUS_OFF           0
    #define PHY_STATUS_PHY_STATUS_BITS          16
    #define PHY_STATUS_OE_PWSTRP_OFF            16
    #define PHY_STATUS_OE_PWSTRP_BITS           11
    #define PHY_STATUS_LPW_STATUS_OFF           31
    #define PHY_STATUS_LPW_STATUS_BITS          1
    
#define REG_BIST0_CTRL                          (0x141C)  /* DWORD reg */
    #define BIST0_CTRL_NOW_OFF                  0
    #define BIST0_CTRL_NOW_BITS                 1
    #define BIST0_CTRL_SRAM_FAIL_OFF            1
    #define BIST0_CTRL_SRAM_FAIL_BITS           1
    #define BIST0_CTRL_FUSE_FLAG_OFF            2
    #define BIST0_CTRL_FUSE_FLAG_BITS           1
    #define BIST0_CTRL_FUSE_PAT_OFF             4
    #define BIST0_CTRL_FUSE_PAT_BITS            3
    #define BIST0_CTRL_FUSE_STEP_OFF            8
    #define BIST0_CTRL_FUSE_SETP_BITS           4
    #define BIST0_CTRL_FUSE_ROW_OFF             12
    #define BIST0_CTRL_FUSE_ROW_BITS            12
    #define BIST0_CTRL_FUSE_COL_OFF             24
    #define BIST0_CTRL_FUSE_COL_BITS            6

#define REG_BIST1_CTRL                          (0x1420)  /* DWORD reg */
    #define BIST1_CTRL_NOW_OFF                  0
    #define BIST1_CTRL_NOW_BITS                 1
    #define BIST1_CTRL_SRAM_FAIL_OFF            1
    #define BIST1_CTRL_SRAM_FAIL_BITS           1
    #define BIST1_CTRL_FUSE_FLAG_OFF            2
    #define BIST1_CTRL_FUSE_FLAG_BITS           1
    #define BIST1_CTRL_FUSE_PAT_OFF             4
    #define BIST1_CTRL_FUSE_PAT_BITS            3
    #define BIST1_CTRL_FUSE_STEP_OFF            8
    #define BIST1_CTRL_FUSE_SETP_BITS           4
    #define BIST1_CTRL_FUSE_ROW_OFF             12
    #define BIST1_CTRL_FUSE_ROW_BITS            7
    #define BIST1_CTRL_FUSE_COL_OFF             24
    #define BIST1_CTRL_FUSE_COL_BITS            5

#define REG_SERDES_CTRL_STS                     (0x1424)
#define     SERDES_CTRL_STS_SELFB_PLL_SEL_OFF   14
#define     SERDES_CTRL_STS_SELFB_PLL_SEL_BITS  2
#define     SERDES_OVCLK_18_25                  0
#define     SERDES_OVCLK_12_18                  1
#define     SERDES_OVCLK_0_4                    2
#define     SERDES_OVCLK_4_12                   3
#define     SERDES_MAC_CLK_SLOWDOWN_OFF         17
#define     SERDES_MAC_CLK_SLOWDOWN_BITS        1
#define     SERDES_PHY_CLK_SLOWDOWN_OFF         18
#define     SERDES_PHY_CLK_SLOWDOWN_BITS        1


#define REG_LED_CTRL                            (0x1428)  /* DWORD reg */
    #define LED_CTRL_DC_CTRL_OFF                0
    #define LED_CTRL_DC_CTRL_BITS               2
    #define LED_CTRL_D3_MODE_CTRL_OFF           2
    #define LED_CTRL_D3_MODE_CTRL_BITS          2
    #define LED_CTRL_0_PAT_MAP_OFF              4
    #define LED_CTRL_0_PAT_MAP_BITS             2
    #define LED_CTRL_1_PAT_MAP_OFF              6
    #define LED_CTRL_1_PAT_MAP_BITS             2
    #define LED_CTRL_2_PAT_MAP_OFF              8
    #define LED_CTRL_2_PAT_MAP_BITS             2
    
#define REG_LED_PAT0                            (0x142C)  /* WORD reg */
#define REG_LED_PAT1                            (0x142E)  /* WORD reg */
#define REG_LED_PAT2                            (0x1430)  /* WORD reg */

#define REG_SYS_ALIVE                           (0x1434) 
    #define SYS_ALIVE_FLAG_OFF                  0
    #define SYS_ALIVE_FLAG_BITS                 1
    
#define REG_LPI_TD                              (0x143C)

#define REG_LPI_CTRL                            (0x1440)
    #define LPI_CTRL_EN_OFF                     0
    #define LPI_CTRL_EN_BITS                    1
    #define LPI_CTRL_CTRL_OFF                   1
    #define LPI_CTRL_CTRL_BITS                  1
    #define LPI_CTRL_GMII_OFF                   2
    #define LPI_CTRL_GMII_BITS                  1
    #define LPI_CTRL_CHK_STATE_OFF              3
    #define LPI_CTRL_CHK_STATE_BITS             1
    #define LPI_CTRL_CHK_RX_OFF                 4
    #define LPI_CTRL_CHK_RX_BITS                1
    
#define REG_LPI_TW                              (0x1444)

    
#define REG_MDIO_EXT_CTRL                       (0x1448)
    #define MDIO_EXT_CTRL_REG_ADDR_OFF          0
    #define MDIO_EXT_CTRL_REG_ADDR_BITS         16
    #define MDIO_EXT_CTRL_DEVADDR_OFF           16
    #define MDIO_EXT_CTRL_DEVADDR_BITS          5
    #define MDIO_EXT_CTRL_PTADDR_OFF            21
    #define MDIO_EXT_CTRL_PTADDR_BITS           5 

#define REG_MAC_CTRL                            (0x1480)  /* DWORD reg */
    #define MAC_CTRL_TXEN_OFF                   0
    #define MAC_CTRL_TXEN_BITS                  1
    #define MAC_CTRL_RXEN_OFF                   1
    #define MAC_CTRL_RXEN_BITS                  1
    #define MAC_CTRL_TXFC_OFF                   2
    #define MAC_CTRL_TXFC_BITS                  1
    #define MAC_CTRL_RXFC_OFF                   3
    #define MAC_CTRL_RXFC_BITS                  1
    #define MAC_CTRL_LOOPBACK_OFF               4
    #define MAC_CTRL_LOOPBACK_BITS              1
    #define MAC_CTRL_FULLD_OFF                  5
    #define MAC_CTRL_FULLD_BITS                 1
    #define MAC_CTRL_CRCE_OFF                   6
    #define MAC_CTRL_CRCE_BITS                  1
    #define MAC_CTRL_PCRCE_OFF                  7
    #define MAC_CTRL_PCRCE_BITS                 1
    #define MAC_CTRL_FLCHK_OFF                  8
    #define MAC_CTRL_FLCHK_BITS                 1
    #define MAC_CTRL_HUGEN_OFF                  9
    #define MAC_CTRL_HUGEN_BITS                 1
    #define MAC_CTRL_PRLEN_OFF                  10
    #define MAC_CTRL_PRLEN_BITS                 4
    #define MAC_CTRL_PRLEN_DEF                  7
    #define MAC_CTRL_VLAN_STRIP_OFF             14
    #define MAC_CTRL_VLAN_STRIP_BITS            1
    #define MAC_CTRL_PROM_MODE_OFF              15
    #define MAC_CTRL_PROM_MODE_BITS             1
    #define MAC_CTRL_TPAUSE_OFF                 16
    #define MAC_CTRL_TPAUSE_BITS                1
    #define MAC_CTRL_SSTCT_OFF                  17
    #define MAC_CTRL_SSTCT_BITS                 1
    #define MAC_CTRL_SRTFN_OFF                  18
    #define MAC_CTRL_SRTFN_BITS                 1
    #define MAC_CTRL_SIMR_OFF                   19
    #define MAC_CTRL_SIMR_BITS                  1
    #define MAC_CTRL_SPEED_OFF                  20
    #define MAC_CTRL_SPEED_BITS                 2
    #define MAC_CTRL_SPEED_10_100               1
    #define MAC_CTRL_SPEED_1000                 2                   
    #define MAC_CTRL_MBOF_OFF                   22
    #define MAC_CTRL_MBOF_BITS                  1
    #define MAC_CTRL_HUGE_OFF                   23
    #define MAC_CTRL_HUGE_BITS                  1
    #define MAC_CTRL_RX_XSUM_EN_OFF             24
    #define MAC_CTRL_RX_XSUM_EN_BITS            1
    #define MAC_CTRL_MUTI_ALL_OFF               25
    #define MAC_CTRL_MUTI_ALL_BITS              1
    #define MAC_CTRL_BROAD_EN_OFF               26
    #define MAC_CTRL_BROAD_EN_BITS              1
    #define MAC_CTRL_DEBUG_MODE_OFF             27
    #define MAC_CTRL_DEBUG_MODE_BITS            1
    #define MAC_CTRL_SINGLE_PAUSE_OFF           28
    #define MAC_CTRL_SINGLE_PAUSE_BITS          1   
    #define MAC_CTRL_HASH_ALG_MODE_OFF          29
    #define MAC_CTRL_HASH_ALG_MODE_BITS         1
    #define MAC_CTRL_HASH_ALG_CRC32             1
    #define MAC_CTRL_HASH_ALG_CRC16             0
    #define MAC_CTRL_SPEED_MODE_OFF             30
    #define MAC_CTRL_SPEED_MODE_BITS            1
    #define MAC_CTRL_SPEED_MODE_PHY             0
    #define MAC_CTRL_SPEED_MODE_SW              1


#define REG_MAC_STA_ADDR                        (0x1488)  /* QWORD reg */
/* [1488]=0x749dc320, [148c]=0x00000013, mac-addr:00-13-74-9d-c3-20 */

#define REG_RX_HASH_TABLE                       (0x1490)  /* QWORD reg */

#define REG_MTU                                 (0x149C)  /* WORD reg */

#define REG_WOL_CTRL                            (0x14A0)  /* DWORD reg */
    #define WOL_CTRL_PATTERN_EN_OFF             0
    #define WOL_CTRL_PATTERN_EN_BITS            1
    #define WOL_CTRL_PATTERN_PME_EN_OFF         1
    #define WOL_CTRL_PATTERN_PME_EN_BITS        1
    #define WOL_CTRL_MAGIC_EN_OFF               2
    #define WOL_CTRL_MAGIC_EN_BITS              1
    #define WOL_CTRL_MAGIC_PME_EN_OFF           3
    #define WOL_CTRL_MAGIC_PME_EN_BITS          1
    #define WOL_CTRL_LINKCHG_EN_OFF             4
    #define WOL_CTRL_LINKCHG_EN_BITS            1
    #define WOL_CTRL_LINKCHG_PME_EN_OFF         5
    #define WOL_CTRL_LINKCHG_PME_EN_BITS        1
    #define WOL_CTRL_PATTERN_ST_OFF             8
    #define WOL_CTRL_PATTERN_ST_BITS            1
    #define WOL_CTRL_MAGIC_ST_OFF               9
    #define WOL_CTRL_MAGIC_ST_BITS              1
    #define WOL_CTRL_LINKCHG_ST_OFF             10
    #define WOL_CTRL_LINKCHG_ST_BITS            1
    #define WOL_CTRL_CLK_SWH_EN_OFF             15
    #define WOL_CTRL_CLK_SWH_EN_BITS            1
    #define WOL_CTRL_PT0_EN_OFF                 16
    #define WOL_CTRL_PT0_EN_BITS                1
    #define WOL_CTRL_PT1_EN_OFF                 17
    #define WOL_CTRL_PT1_EN_BITS                1
    #define WOL_CTRL_PT2_EN_OFF                 18
    #define WOL_CTRL_PT2_EN_BITS                1   
    #define WOL_CTRL_PT3_EN_OFF                 19
    #define WOL_CTRL_PT3_EN_BITS                1   
    #define WOL_CTRL_PT4_EN_OFF                 20
    #define WOL_CTRL_PT4_EN_BITS                1   
    #define WOL_CTRL_PT5_EN_OFF                 21
    #define WOL_CTRL_PT5_EN_BITS                1   
    #define WOL_CTRL_PT6_EN_OFF                 22
    #define WOL_CTRL_PT6_EN_BITS                1
    #define WOL_CTRL_PT0_MATCH_OFF              24
    #define WOL_CTRL_PT0_MATCH_BITS             1
    #define WOL_CTRL_PT1_MATCH_OFF              25
    #define WOL_CTRL_PT1_MATCH_BITS             1
    #define WOL_CTRL_PT2_MATCH_OFF              26
    #define WOL_CTRL_PT2_MATCH_BITS             1
    #define WOL_CTRL_PT3_MATCH_OFF              27
    #define WOL_CTRL_PT3_MATCH_BITS             1
    #define WOL_CTRL_PT4_MATCH_OFF              28
    #define WOL_CTRL_PT4_MATCH_BITS             1
    #define WOL_CTRL_PT5_MATCH_OFF              29
    #define WOL_CTRL_PT5_MATCH_BITS             1
    #define WOL_CTRL_PT6_MATCH_OFF              30
    #define WOL_CTRL_PT6_MATCH_BITS             1
            
#define REG_WOL_PT0_LEN                         (0x14A4)  /* BYTE reg */
#define REG_WOL_PT1_LEN                         0x14A5  /* BYTE reg */
#define REG_WOL_PT2_LEN                         0x14A6  /* BYTE reg */
#define REG_WOL_PT3_LEN                         0x14A7  /* BYTE reg */
#define REG_WOL_PT4_LEN                         0x14A8  /* BYTE reg */
#define REG_WOL_PT5_LEN                         0x14A9  /* BYTE reg */
#define REG_WOL_PT6_LEN                         0x14AA  /* BYTE reg */

#define REG_SRAM_RFD0                           (0x1500)  /* DWORD reg */
    #define SRAM_RFD0_HDRADDR_OFF               0
    #define SRAM_RFD0_HDRADDR_BITS              12
    #define SRAM_RFD0_TALADDR_OFF               16
    #define SRAM_RFD0_TALADDR_BITS              12
    
#define REG_SRAM_RFD1                           (0x1504)  /* DWORD reg */
    #define SRAM_RFD1_HDRADDR_OFF               0
    #define SRAM_RFD1_HDRADDR_BITS              12
    #define SRAM_RFD1_TALADDR_OFF               16
    #define SRAM_RFD1_TALADDR_BITS              12
    
#define REG_SRAM_RFD2                           (0x1508)  /* DWORD reg */
    #define SRAM_RFD2_HDRADDR_OFF               0
    #define SRAM_RFD2_HDRADDR_BITS              12
    #define SRAM_RFD2_TALADDR_OFF               16
    #define SRAM_RFD2_TALADDR_BITS              12
    
#define REG_SRAM_RFD3                           (0x150C)  /* DWORD reg */
    #define SRAM_RFD3_HDRADDR_OFF               0
    #define SRAM_RFD3_HDRADDR_BITS              12
    #define SRAM_RFD3_TALADDR_OFF               16
    #define SRAM_RFD3_TALADDR_BITS              12
    
#define REG_SRAM_RFD_NICLEN                     (0x1510)  /* DWORD reg */

#define REG_SRAM_TRD                            0x1518  /* DWORD reg */
    #define SRAM_TRD_HDRADDR_OFF                0
    #define SRAM_TRD_HDRADDR_BITS               12
    #define SRAM_TRD_TALADDR_OFF                16
    #define SRAM_TRD_TALADDR_BITS               12
    
#define REG_SRAM_TRD_NICLEN                     (0x151C)  /* DWORD reg */

#define REG_SRAM_RXF                            (0x1520)  /* DWORD reg */
    #define SRAM_RXF_HDRADDR_OFF                0
    #define SRAM_RXF_HDRADDR_BITS               12
    #define SRAM_RXF_TALADDR_OFF                16
    #define SRAM_RXF_TALADDR_BITS               12
    
#define REG_SRAM_RXF_NICLEN                     (0x1524)  /* DWORD reg */

#define REG_SRAM_TXF                            (0x1528)  /* DWORD reg */
    #define SRAM_TXF_HDRADDR_OFF                0
    #define SRAM_TXF_HDRADDR_BITS               12
    #define SRAM_TXF_TALADDR_OFF                16
    #define SRAM_TXF_TALADDR_BITS               12
    
#define REG_SRAM_TXF_NICLEN                     (0x152C)  /* DWORD reg */ 
        
#define REG_SRAM_TCP_HDRADDR                    (0x1530)  /* WORD reg */

#define REG_SRAM_PAT_HDRADDR                    (0x1532)  /* WORD reg */

#define REG_SRAM_LOAD_PTR                       (0x1534)  /* BYTE reg, sc */
    #define SRAM_LOAD_PTR_OFF                   0
    #define SRAM_LOAD_PTR_BITS                  1   


#define REG_RX_BASE_ADDR_HI                     (0x1540)  /* DWORD reg */

#define REG_TX_BASE_ADDR_HI                     (0x1544)  /* DWORD reg */

#define REG_SMB_BASE_ADDR_HI                    (0x1548)  /* DWORD reg */
#define REG_SMB_BASE_ADDR_LO                    (0x154C)  /* DWORD reg */

#define REG_RFD0_HDRADDR_LO                     (0x1550)  /* DWORD reg */
#define REG_RFD1_HDRADDR_LO                     (0x1554)  /* DWORD reg */
#define REG_RFD2_HDRADDR_LO                     (0x1558)  /* DWORD reg */
#define REG_RFD3_HDRADDR_LO                     (0x155C) /* DWORD reg */
#define REG_RFD_RING_SIZE                       (0x1560) /* DWORD reg */
#define REG_RFD_BUFFER_SIZE                     (0x1564)  /* DWORD reg */

#define REG_RRD0_HDRADDR_LO                     (0x1568)  /* DWORD reg */
#define REG_RRD1_HDRADDR_LO                     (0x156C)  /* DWORD reg */
#define REG_RRD2_HDRADDR_LO                     (0x1570)  /* DWORD reg */
#define REG_RRD3_HDRADDR_LO                     (0x1574)  /* DWORD reg */
#define REG_RRD_RING_SIZE                       (0x1578)  /* DWORD reg */

#define REG_HTPD_HDRADDR_LO                     (0x157C)  /* DWORD reg */
#define REG_NTPD_HDRADDR_LO                     (0x1580)  /* DWORD reg */
#define REG_TPD_RING_SIZE                       (0x1584)  /* DWORD reg */

#define REG_CMB_BASE_ADDR_LO                    (0x1588)  /* DWORD reg */

#define REG_RSS_KEY0                            (0x14B0)  /* DWORD reg */
#define REG_RSS_KEY1                            (0x14B4)  /* DWORD reg */
#define REG_RSS_KEY2                            (0x14B8)  /* DWORD reg */
#define REG_RSS_KEY3                            (0x14BC)  /* DWORD reg */
#define REG_RSS_KEY4                            (0x14C0)  /* DWORD reg */
#define REG_RSS_KEY5                            (0x14C4)  /* DWORD reg */
#define REG_RSS_KEY6                            (0x14C8)  /* DWORD reg */
#define REG_RSS_KEY7                            (0x14CC)  /* DWORD reg */
#define REG_RSS_KEY8                            (0x14D0)  /* DWORD reg */
#define REG_RSS_KEY9                            (0x14D4)  /* DWORD reg */

#define REG_RSS_IDT_TABLE0                      (0x14E0)  /* DWORD reg */
#define REG_RSS_IDT_TABLE1                      (0x14E4)  /* DWORD reg */
#define REG_RSS_IDT_TABLE2                      (0x14E8)  /* DWORD reg */
#define REG_RSS_IDT_TABLE3                      (0x14EC)  /* DWORD reg */
#define REG_RSS_IDT_TABLE4                      (0x14F0)  /* DWORD reg */
#define REG_RSS_IDT_TABLE5                      (0x14F4)  /* DWORD reg */
#define REG_RSS_IDT_TABLE6                      (0x14F8)  /* DWORD reg */
#define REG_RSS_IDT_TABLE7                      (0x14FC)  /* DWORD reg */

#define REG_RSS_HASH_VAL                        (0x15B0)  /* DWORD reg */
#define REG_RSS_HASH_FLAG                       (0x15B4)  /* DOWRD reg */

#define REG_RSS_BASE_CPU_NUMBER                 (0x15B8)  /* DWORD reg */

#define REG_TXQ_CTRL                            (0x1590)  /* DWORD reg */
    #define TXQ_CTRL_NUM_TPD_BURST_OFF          0
    #define TXQ_CTRL_NUM_TPD_BURST_BITS         4
    #define TXQ_CTRL_NUM_TPD_BURST_DEF          5
    #define TXQ_CTRL_IP_OPT_SP_OFF              4
    #define TXQ_CTRL_IP_OPT_SP_BITS             1
    #define TXQ_CTRL_EN_OFF                     5
    #define TXQ_CTRL_EN_BITS                    1
    #define TXQ_CTRL_MODE_OFF                   6
    #define TXQ_CTRL_MODE_BITS                  1
    #define TXQ_CTRL_MODE_ENH                   1
    #define TXQ_CTRL_EN_SNAP_LSO_OFF            7
    #define TXQ_CTRL_EN_SNAP_LSO_BITS           1
    #define TXQ_CTRL_NUM_TXF_BURST_OFF          16
    #define TXQ_CTRL_NUM_TXF_BURST_BITS         16
    
    
#define REG_TXQ_JUMBO_TSO_THRESHOLD             (0x1594)  /* DWORD reg */

#define REG_TXQ_TXF_BURST_L1                    (0x1598)  /* DWORD reg */
    #define TXQ_TXF_BURST_L1_LWM_OFF            0
    #define TXQ_TXF_BURST_L1_LWM_BITS           12
    #define TXQ_TXF_BURST_L1_HWM_OFF            16
    #define TXQ_TXF_BURST_L1_HWM_BITS           12
    #define TXQ_TXF_BURST_L1_EN_OFF             31
    #define TXQ_TXF_BURST_L1_EN_BITS            1


#define REG_THRUPUT_MON_CTRL                    (0x159C)  /* DWORD reg */
    #define THRUPUT_MON_CTRL_RATE_OFF           0
    #define THRUPUT_MON_CTRL_RATE_BITS          2
    
    #define THRUPUT_MON_CTRL_EN_OFF             7
    #define THRUPUT_MON_CTRL_EN_BITS            1
    
#define REG_RXQ_CTRL                            (0x15A0)  /* DWORD reg */
    #define RXQ_CTRL_ASPM_THRUPUT_LIM_OFF       0
    #define RXQ_CTRL_ASPM_THRUPUT_LIM_BITS      2
    #define RXQ_CTRL_ASPM_THRUPUT_LIM_NO        0
    #define RXQ_CTRL_ASPM_THRUPUT_LIM_1MB       1
    #define RXQ_CTRL_ASPM_THRUPUT_LIM_10MB      2
    #define RXQ_CTRL_ASPM_THRUPUT_LIM_100MB     3
    #define RXQ_CTRL_Q1_EN_OFF                  4
    #define RXQ_CTRL_Q1_EN_BITS                 1
    #define RXQ_CTRL_Q2_EN_OFF                  5
    #define RXQ_CTRL_Q2_EN_BITS                 1
    #define RXQ_CTRL_Q3_EN_OFF                  6
    #define RXQ_CTRL_Q3_EN_BITS                 1
    #define RXQ_CTRL_IPV6_XSUM_EN_OFF           7
    #define RXQ_CTRL_IPV6_XSUM_EN_BITS          1
    #define RXQ_CTRL_RSS_HASH_BITS_OFF          8
    #define RXQ_CTRL_RSS_HASH_BITS_BITS         8
    #define RXQ_CTRL_RSS_HASH_TYPE_IPV4_OFF     16
    #define RXQ_CTRL_RSS_HASH_TYPE_IPV4_TCP_OFF 17
    #define RXQ_CTRL_RSS_HASH_TYPE_IPV6_OFF     18
    #define RXQ_CTRL_RSS_HASH_TYPE_IPV6_TCP_OFF 19
    #define RXQ_CTRL_NUM_RFD_PREF_OFF           20
    #define RXQ_CTRL_NUM_RFD_PREF_BITS          6
    #define RXQ_CTRL_NUM_RFD_PREF_DEF           8
    #define RXQ_CTRL_RSS_MODE_OFF               26
    #define RXQ_CTRL_RSS_MODE_BITS              2
    #define RXQ_CTRL_RSS_MODE_DIS               0
    #define RXQ_CTRL_RSS_MODE_SQSI              1
    #define RXQ_CTRL_RSS_MODE_MQSI              2
    #define RXQ_CTRL_RSS_MODE_MQMI              3
    #define RXQ_CTRL_NIP_QUEUE_SEL_OFF          28
    #define RXQ_CTRL_NIP_QUEUE_SEL_BITS         1
    #define RXQ_CTRL_RSS_HASH_EN_OFF            29
    #define RXQ_CTRL_RSS_HASH_EN_BITS           1
    #define RXQ_CTRL_CUT_THRU_OFF               30
    #define RXQ_CTRL_CUT_THRU_BITS              1
    #define RXQ_CTRL_EN_OFF                     31
    #define RXQ_CTRL_EN_BITS                    1
    
#define REG_RFD_PREF_CTRL                       (0x15A4)
    #define RFD_PREF_CTRL_UP_TH_OFF             0
    #define RFD_PREF_CTRL_UP_TH_BITS            6
    #define RFD_PREF_CTRL_UP_TH_DEF             16
    #define RFD_PREF_CTRL_LOW_TH_OFF            6
    #define RFD_PREF_CTRL_LOW_TH_BITS           6
    #define RFD_PREF_CTRL_LOW_TH_DEF            8
        
    
#define REG_FC_RXF_HI                           (0x15A8)  /* WORD reg */
#define REG_FC_RXF_LO                           0x15AA  /* WORD reg */

#define REG_RXD_CTRL                            (0x15AC)  /* DWORD reg */
    #define RXD_CTRL_THRESHOLD_OFF              0
    #define RXD_CTRL_THRESHOLD_BITS             12
    #define RXD_CTRL_TIMER_OFF                  16
    #define RXD_CTRL_TIMER_BITS                 16


#define REG_DMA_CTRL                            (0x15C0)  /* DWORD reg */
    #define DMA_CTRL_ORDER_MODE_OFF             0
    #define DMA_CTRL_ORDER_MODE_BITS            3
    #define DMA_CTRL_ORDER_MODE_IN              1
    #define DMA_CTRL_ORDER_MODE_ENH             2
    #define DMA_CTRL_ORDER_MODE_OUT             4    
    #define DMA_CTRL_RCB_VAL_OFF                3
    #define DMA_CTRL_RCB_VAL_BITS               1
    #define DMA_CTRL_REGRDBLEN_OFF              4
    #define DMA_CTRL_REGRDBLEN_BITS             3
    #define DMA_CTRL_REGWRBLEN_OFF              7
    #define DMA_CTRL_REGWRBLEN_BITS             3
    #define DMA_CTRL_DMAR_REQ_PRI_OFF           10
    #define DMA_CTRL_DMAR_REQ_PRI_BITS          1
    #define DMA_CTRL_DMAR_DLY_CNT_OFF           11
    #define DMA_CTRL_DMAR_DLY_CNT_BITS          5
    #define DMA_CTRL_DMAR_DLY_CNT_DEF           15
    #define DMA_CTRL_DMAW_DLY_CNT_OFF           16
    #define DMA_CTRL_DMAW_DLY_CNT_BITS          4
    #define DMA_CTRL_DMAW_DLY_CNT_DEF           4
    #define DMA_CTRL_CMB_EN_OFF                 20
    #define DMA_CTRL_CMB_EN_BITS                1
    #define DMA_CTRL_SMB_DMA_SP_OFF             21 /* enable SMB DMA */
    #define DMA_CTRL_SMB_DMA_SP_BITS            1
    #define DMA_CTRL_CMB_NOW_OFF                22
    #define DMA_CTRL_CMB_NOW_BITS               1
    #define DMA_CTRL_SMB_DIS_OFF                24
    #define DMA_CTRL_SMB_DIS_BITS               1
    #define DMA_CTRL_SMB_NOW_OFF                31
    #define DMA_CTRL_SMB_NOW_BITS               1
 
#define REG_SMB_DIS                             (0x15C3)  /* BYTE reg */    
    
#define REG_SMB_TIMER                           (0x15C4 ) /* DWORD reg */

#define REG_CMB_TPD_THRESHOLD                   (0x15C8)  /* WORD reg */
#define REG_CMB_TIMER                           (0x15CC)  /* WORD reg */

#define REG_RFD0_PROD_INDEX                     (0x15E0)  /* WORD reg */
#define REG_RFD1_PROD_INDEX                     (0x15E4)  /* WORD reg */
#define REG_RFD2_PROD_INDEX                     (0x15E8)  /* WORD reg */
#define REG_RFD3_PROD_INDEX                     (0x15EC) /* WORD reg */

#define REG_HTPD_PROD_INDEX                     (0x15F0)  /* WORD reg */
#define REG_NTPD_PROD_INDEX                     (0x15F2)  /* WORD reg */
#define REG_HTPD_CONS_INDEX                     (0x15F4)  /* WORD reg, ro */
#define REG_NTPD_CONS_INDEX                     (0x15F6)  /* WORD reg, ro */

#define REG_RFD0_CONS_INDEX                     (0x15F8)  /* WORD reg, ro */
#define REG_RFD1_CONS_INDEX                     (0x15FA)  /* WORD reg, ro */
#define REG_RFD2_CONS_INDEX                     (0x15FC)  /* WORD reg, ro */
#define REG_RFD3_CONS_INDEX                     (0x15FE)  /* WORD reg, ro */


#define REG_ISR                                 (0x1600)  /* DWORD reg */
    #define ISR_SMB_OFF                         0
  
    #define ISR_TIMER_OFF                       1
 
    #define ISR_SW_MANUAL_OFF                   2
    #define ISR_SW_MANUAL_BITS                  1
    #define ISR_RXF_OV_OFF                      (1<<3)
    #define ISR_RXF_OV_BITS                     1
    #define ISR_RFD0_UR_OFF                     (1<<4)
    #define ISR_RFD0_UR_BITS                    1
    #define ISR_RFD1_UR_OFF                     5
    #define ISR_RFD1_UR_BITS                    1   
    #define ISR_RFD2_UR_OFF                     6
    #define ISR_RFD2_UR_BITS                    1
    #define ISR_RFD3_UR_OFF                     7
    #define ISR_RFD3_UR_BITS                    1
    #define ISR_TXF_UR_OFF                      8
    #define ISR_TXF_UR_BITS                     1
    #define ISR_DMAR_OFF                        (1<<9)
    #define ISR_DMAR_BITS                       1
    #define ISR_DMAW_OFF                        (1<<10)
    #define ISR_DMAW_BITS                       1
    #define ISR_TX_CREDIT_OFF                   11
    #define ISR_TX_CREDIT_BITS                  1
    #define ISR_GPHY_OFF                        (1<<12)
    #define ISR_GPHY_BITS                       1
    #define ISR_GPHY_LPW_OFF                    (1<<13)
    #define ISR_GPHY_LPW_BITS                   1
    #define ISR_TXQ_OFF                         (1<<14)
    #define ISR_TXQ_BITS                        1
    #define ISR_TX_PKT_OFF                      (1<<15)
    #define ISR_TX_PKT_BITS                     1
    #define ISR_RX0_PKT_OFF                     (1<<16)
    #define ISR_RX0_PKT_BITS                    1
    #define ISR_RX1_PKT_OFF                     17
    #define ISR_RX1_PKT_BITS                    1
    #define ISR_RX2_PKT_OFF                     18
    #define ISR_RX2_PKT_BITS                    1
    #define ISR_RX3_PKT_OFF                     19
    #define ISR_RX3_PKT_BITS                    1
    #define ISR_MAC_RX_OFF                      20
    #define ISR_MAC_RX_BITS                     1
    #define ISR_MAC_TX_OFF                      21
    #define ISR_MAC_TX_BITS                     1
    #define ISR_PCIE_UR_OFF                     22
    #define ISR_PCIE_UR_BITS                    1
    #define ISR_PCIE_FERR_OFF                   23
    #define ISR_PCIE_FERR_BITS                  1
    #define ISR_PCIE_NFERR_OFF                  24
    #define ISR_PCIE_NFERR_BITS                 1
    #define ISR_PCIE_CERR_OFF                   25
    #define ISR_PCIE_CERR_BITS                  1
    #define ISR_PCIE_LINKDOWN_OFF               26
    #define ISR_PCIE_LINKDOWN_BITS              1
    #define ISR_DIS_OFF                         31
    #define ISR_DIS_BITS                        1   

#define REG_IMR                                 (0x1604)  /* DWORD reg */


#define INT_FATAL_MASK          (\
    FLAG(ISR_DMAR_OFF)          |\
    FLAG(ISR_DMAW_OFF)          |\
    FLAG(ISR_PCIE_FERR_OFF)     |\
    FLAG(ISR_PCIE_LINKDOWN_OFF) )

    
#define INT_TX_MASK             (\
    FLAG(ISR_MAC_TX_OFF)        |\
    FLAG(ISR_TX_PKT_OFF)        |\
    FLAG(ISR_TXF_UR_OFF)        )
    
#define INT_RX_MASK             (\
    FLAG(ISR_RXF_OV_OFF)        |\
 /*   FLAG(ISR_RFD0_UR_OFF)       |*/\
    FLAG(ISR_RFD1_UR_OFF)       |\
    FLAG(ISR_RFD2_UR_OFF)       |\
    FLAG(ISR_RFD3_UR_OFF)       |\
    FLAG(ISR_RX0_PKT_OFF)       |\
    FLAG(ISR_RX1_PKT_OFF)       |\
    FLAG(ISR_RX2_PKT_OFF)       |\
    FLAG(ISR_RX3_PKT_OFF)       |\
    FLAG(ISR_MAC_RX_OFF)        )

#define INT_MASK                (\
    INT_RX_MASK                 |\
    INT_TX_MASK                 |\
    INT_FATAL_MASK              |\
    FLAG(ISR_SMB_OFF)           |\
    FLAG(ISR_SW_MANUAL_OFF)     |\
    FLAG(ISR_GPHY_OFF)          |\
    FLAG(ISR_GPHY_LPW_OFF)      )
    

    

#define REG_INT_RETRIG_TIMER                    (0x1608)  /* WORD reg */

#define REG_HDS_CTRL                            (0x160C)  /* DWORD reg */
    #define HDS_CTRL_EN_OFF                     0
    #define HDS_CTRL_EN_BITS                    1
    #define HDS_CTRL_BACKFILLSIZE_OFF           8
    #define HDS_CTRL_BACKFILLSIZE_BITS          12
    #define HDS_CTRL_MAX_HDRSIZE_OFF            20
    #define HDS_CTRL_MAX_HDRSIZE_BITS           12

#endif
