#ifndef _ANYKA_PHY_REG_H_
#define _ANYKA_PHY_REG_H_
/********************* PHY regs definition ***************************/
#define LC_10H      0x01
#define LC_10F      0x02
#define LC_100H     0x04
#define LC_100F     0x08
#define LC_1000F    0x10
#define LC_ALL     (LC_10H|LC_10F|LC_100H|LC_100F|LC_1000F)

/* PHY Control Register */
#define MII_BMCR                                0x00
    #define BMCR_SPEED_SELECT_MSB               0x0040  /* bits 6,13: 10=1000, 01=100, 00=10 */
    #define BMCR_COLL_TEST_ENABLE               0x0080  /* Collision test enable */
    #define BMCR_FULL_DUPLEX                    0x0100  /* FDX =1, half duplex =0 */
    #define BMCR_RESTART_AUTO_NEG               0x0200  /* Restart auto negotiation */
    #define BMCR_ISOLATE                        0x0400  /* Isolate PHY from MII */
    #define BMCR_POWER_DOWN                     0x0800  /* Power down */
    #define BMCR_AUTO_NEG_EN                    0x1000  /* Auto Neg Enable */
    #define BMCR_SPEED_SELECT_LSB               0x2000  /* bits 6,13: 10=1000, 01=100, 00=10 */
    #define BMCR_LOOPBACK                       0x4000  /* 0 = normal, 1 = loopback */
    #define BMCR_RESET                          0x8000  /* 0 = normal, 1 = PHY reset */
    #define BMCR_SPEED_MASK                     0x2040
    #define BMCR_SPEED_1000                     0x0040
    #define BMCR_SPEED_100                      0x2000
    #define BMCR_SPEED_10                       0x0000

/* PHY Status Register */    
#define MII_BMSR                                0x01 
    #define BMMSR_EXTENDED_CAPS                 0x0001  /* Extended register capabilities */
    #define BMSR_JABBER_DETECT                  0x0002  /* Jabber Detected */
    #define BMSR_LINK_STATUS                    0x0004  /* Link Status 1 = link */
    #define BMSR_AUTONEG_CAPS                   0x0008  /* Auto Neg Capable */
    #define BMSR_REMOTE_FAULT                   0x0010  /* Remote Fault Detect */
    #define BMSR_AUTONEG_COMPLETE               0x0020  /* Auto Neg Complete */
    #define BMSR_PREAMBLE_SUPPRESS              0x0040  /* Preamble may be suppressed */
    #define BMSR_EXTENDED_STATUS                0x0100  /* Ext. status info in Reg 0x0F */
    #define BMSR_100T2_HD_CAPS                  0x0200  /* 100T2 Half Duplex Capable */
    #define BMSR_100T2_FD_CAPS                  0x0400  /* 100T2 Full Duplex Capable */
    #define BMSR_10T_HD_CAPS                    0x0800  /* 10T   Half Duplex Capable */
    #define BMSR_10T_FD_CAPS                    0x1000  /* 10T   Full Duplex Capable */
    #define BMSR_100X_HD_CAPS                   0x2000  /* 100X  Half Duplex Capable */
    #define BMMII_SR_100X_FD_CAPS               0x4000  /* 100X  Full Duplex Capable */
    #define BMMII_SR_100T4_CAPS                 0x8000  /* 100T4 Capable */

#define MII_PHYSID1                             0x02     
#define MII_PHYSID2                             0x03  
#define L1D_MPW_PHYID1                          0xD01C  /* V7 */
#define L1D_MPW_PHYID2                          0xD01D  /* V1-V6 */
#define L1D_MPW_PHYID3                          0xD01E  /* V8 */
 

/* Autoneg Advertisement Register */
#define MII_ADVERTISE                           0x04
    #define ADVERTISE_SELECTOR_FIELD            0x0001  /* indicates IEEE 802.3 CSMA/CD */
    #define ADVERTISE_10T_HD_CAPS               0x0020  /* 10T   Half Duplex Capable */
    #define ADVERTISE_10T_FD_CAPS               0x0040  /* 10T   Full Duplex Capable */
    #define ADVERTISE_100TX_HD_CAPS             0x0080  /* 100TX Half Duplex Capable */
    #define ADVERTISE_100TX_FD_CAPS             0x0100  /* 100TX Full Duplex Capable */
    #define ADVERTISE_100T4_CAPS                0x0200  /* 100T4 Capable */
    #define ADVERTISE_PAUSE                     0x0400  /* Pause operation desired */
    #define ADVERTISE_ASM_DIR                   0x0800  /* Asymmetric Pause Direction bit */
    #define ADVERTISE_REMOTE_FAULT              0x2000  /* Remote Fault detected */
    #define ADVERTISE_NEXT_PAGE                 0x8000  /* Next Page ability supported */
    #define ADVERTISE_SPEED_MASK                0x01E0
    #define ADVERTISE_DEFAULT_CAP               0x0DE0

/* Link partner ability register */   
#define MII_LPA                                 0x05 
    #define LPA_SLCT                            0x001   /* Same as advertise selector  */
    #define LPA_10HALF                          0x002   /* Can do 10mbps half-duplex   */
    #define LPA_10FULL                          0x0040  /* Can do 10mbps full-duplex   */
    #define LPA_100HALF                         0x0080  /* Can do 100mbps half-duplex  */
    #define LPA_100FULL                         0x0100  /* Can do 100mbps full-duplex  */
    #define LPA_100BASE4                        0x0200  /* 100BASE-T4  */
    #define LPA_PAUSE                           0x0400  /* PAUSE */
    #define LPA_ASYPAUSE                        0x0800  /* Asymmetrical PAUSE */
    #define LPA_RFAULT                          0x2000  /* Link partner faulted        */
    #define LPA_LPACK                           0x4000  /* Link partner acked us       */
    #define LPA_NPAGE                           0x8000  /* Next page bit               */

/* 1000BASE-T Control Register */      
#define MII_GIGA_CR                             0x09   
    #define GIGA_CR_1000T_HD_CAPS               0x0100  /* Advertise 1000T HD capability */
    #define GIGA_CR_1000T_FD_CAPS               0x0200  /* Advertise 1000T FD capability  */
    #define GIGA_CR_1000T_REPEATER_DTE          0x0400  /* 1=Repeater/switch device port */
                            /* 0=DTE device */
    #define GIGA_CR_1000T_MS_VALUE              0x0800  /* 1=Configure PHY as Master */
                            /* 0=Configure PHY as Slave */
    #define GIGA_CR_1000T_MS_ENABLE             0x1000  /* 1=Master/Slave manual config value */
                            /* 0=Automatic Master/Slave config */
    #define GIGA_CR_1000T_TEST_MODE_NORMAL      0x0000  /* Normal Operation */
    #define GIGA_CR_1000T_TEST_MODE_1           0x2000  /* Transmit Waveform test */
    #define GIGA_CR_1000T_TEST_MODE_2           0x4000  /* Master Transmit Jitter test */
    #define GIGA_CR_1000T_TEST_MODE_3           0x6000  /* Slave Transmit Jitter test */
    #define GIGA_CR_1000T_TEST_MODE_4           0x8000  /* Transmitter Distortion test */
    #define GIGA_CR_1000T_SPEED_MASK            0x0300
    #define GIGA_CR_1000T_DEFAULT_CAP           0x0300

/* 1000BASE-T Status Register */      
#define MII_GIGA_SR                             0x0A   

/* PHY Specific Status Register */
#define MII_GIGA_PSSR                           0x11    
    #define GIGA_PSSR_SPD_DPLX_RESOLVED         0x0800  /* 1=Speed & Duplex resolved */
    #define GIGA_PSSR_DPLX                      0x2000  /* 1=Duplex 0=Half Duplex */
    #define GIGA_PSSR_SPEED                     0xC000  /* Speed, bits 14:15 */
    #define GIGA_PSSR_10MBS                     0x0000  /* 00=10Mbs */
    #define GIGA_PSSR_100MBS                    0x4000  /* 01=100Mbs */
    #define GIGA_PSSR_1000MBS                   0x8000  /* 10=1000Mbs */
    
/* PHY Interrupt Enable Register */
#define MII_IER                                 0x12
    #define IER_LINK_UP                         0x0400    
    #define IER_LINK_DOWN                       0x0800
    
/* PHY Interrupt Status Register */
#define MII_ISR                                 0x13
    #define ISR_LINK_UP                         0x0400
    #define ISR_LINK_DOWN                       0x0800  

/* Cable-Detect-Test Control Register */
#define MII_CDTC                                0x16
    #define CDTC_EN                             1   /* sc */
    #define CDTC_PAIR_OFFSET                    8
    #define CDTC_PAIR_MASK                      3


/* Cable-Detect-Test Status Register */    
#define MII_CDTS                                0x1C 
    #define CDTS_STATUS_OFFSET                  8
    #define CDTS_STATUS_MASK                    3
    #define CDTS_STATUS_NORMAL                  0 
    #define CDTS_STATUS_SHORT                   1
    #define CDTS_STATUS_OPEN                    2
    #define CDTS_STATUS_INVALID                 3
    
#define MII_DBG_ADDR                            0x1D
#define MII_DBG_DATA                            0x1E
#endif