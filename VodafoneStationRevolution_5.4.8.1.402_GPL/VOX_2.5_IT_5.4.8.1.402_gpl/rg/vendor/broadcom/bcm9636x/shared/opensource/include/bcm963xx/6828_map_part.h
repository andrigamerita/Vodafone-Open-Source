/*
 Copyright 2000-2010 Broadcom Corporation

 <:label-BRCM:2011:DUAL/GPL:standard    
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
 :>
*/

#ifndef __BCM6828_MAP_PART_H
#define __BCM6828_MAP_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#define CHIP_FAMILY_ID_HEX	0x6828

#ifndef __BCM6828_MAP_H
#define __BCM6828_MAP_H

#include "bcmtypes.h"

#define PERF_BASE                   0xb0000000  /* chip control */
#define TIMR_BASE                   0xb0000080  /* timer registers */
#define GPIO_BASE                   0xb00000c0  /* gpio registers */
#define UART_BASE                   0xb0000180  /* uart registers */
#define UART1_BASE                  0xb00001a0  /* uart registers */
#define NAND_REG_BASE               0xb0000200  /* nand interrupt control */
#define OTP_REG_BASE                0xb0000400  /* OTP control/status registers */
#define NAND_CACHE_BASE             0xb0000600
#define SPI_BASE                    0xb0000800  /* SPI master controller */
#define HSSPIM_BASE                 0xb0001000  /* High-Speed SPI registers */
#define MISC_BASE                   0xb0001800  /* Miscellaneous Registers */
#define LED_BASE                    0xb0001900  /* LED control registers */
#define USB_CTL_BASE                0xb0002400  /* USB 2.0 device control */
#define USB_EHCI_BASE               0x10002500  /* USB host registers */
#define USB_OHCI_BASE               0x10002600  /* USB host registers */
#define USBH_CFG_BASE               0xb0002700
#define HSI_BASE                    0xb0002800  /* Hornet SPI Interface */
#define DDR_BASE                    0xb0003000  /* DDR IO Buf Control */
#define SWITCH_DMA_BASE             0xb0004000
#define BMU_BASE                    0xb0201000
#define PCIE_BASE                   0xb06e0000
#define APM_BASE                    0xb0200000
#define SWITCH_BASE                 0xb0700000

#define FAP0_BASE                   0xb0800000
#define FAP0_QSM_UBUS_BASE          0xb0804000
#define FAP0_QSM_SMI_BASE           0xb0c04000
#define FAP0_PSM_BASE               0xb0820000

#define FAP1_BASE                   0xb0a00000
#define FAP1_QSM_UBUS_BASE          0xb0a04000
#define FAP1_QSM_SMI_BASE           0xb0e04000
#define FAP1_PSM_BASE               0xb0a20000

#endif

typedef struct DDRPhyControl {
    uint32 REVISION;               /* 0x00 */
    uint32 CLK_PM_CTRL;            /* 0x04 */
    uint32 unused0[2];             /* 0x08-0x10 */
    uint32 PLL_STATUS;             /* 0x10 */
    uint32 PLL_CONFIG;             /* 0x14 */
    uint32 PLL_PRE_DIVIDER;        /* 0x18 */
    uint32 PLL_DIVIDER;            /* 0x1c */
    uint32 PLL_CONTROL1;           /* 0x20 */
    uint32 PLL_CONTROL2;           /* 0x24 */
    uint32 PLL_SS_EN;              /* 0x28 */
    uint32 PLL_SS_CFG;             /* 0x2c */
    uint32 STATIC_VDL_OVERRIDE;    /* 0x30 */
    uint32 DYNAMIC_VDL_OVERRIDE;   /* 0x34 */
    uint32 IDLE_PAD_CONTROL;       /* 0x38 */
    uint32 ZQ_PVT_COMP_CTL;        /* 0x3c */
    uint32 DRIVE_PAD_CTL;          /* 0x40 */
    uint32 CLOCK_REG_CONTROL;      /* 0x44 */
    uint32 unused1[46];
} DDRPhyControl;

typedef struct DDRPhyByteLaneControl {
    uint32 REVISION;                /* 0x00 */
    uint32 VDL_CALIBRATE;           /* 0x04 */
    uint32 VDL_STATUS;              /* 0x08 */
    uint32 unused;                  /* 0x0c */
    uint32 VDL_OVERRIDE_0;          /* 0x10 */
    uint32 VDL_OVERRIDE_1;          /* 0x14 */
    uint32 VDL_OVERRIDE_2;          /* 0x18 */
    uint32 VDL_OVERRIDE_3;          /* 0x1c */
    uint32 VDL_OVERRIDE_4;          /* 0x20 */
    uint32 VDL_OVERRIDE_5;          /* 0x24 */
    uint32 VDL_OVERRIDE_6;          /* 0x28 */
    uint32 VDL_OVERRIDE_7;          /* 0x2c */
    uint32 READ_CONTROL;            /* 0x30 */
    uint32 READ_FIFO_STATUS;        /* 0x34 */
    uint32 READ_FIFO_CLEAR;         /* 0x38 */
    uint32 IDLE_PAD_CONTROL;        /* 0x3c */
    uint32 DRIVE_PAD_CTL;           /* 0x40 */
    uint32 CLOCK_PAD_DISABLE;       /* 0x44 */
    uint32 WR_PREAMBLE_MODE;        /* 0x48 */
    uint32 CLOCK_REG_CONTROL;       /* 0x4C */
    uint32 unused0[44];
} DDRPhyByteLaneControl;

typedef struct MEMCControl {
    uint32 CNFG;                            /* 0x000 */
    uint32 CSST;                            /* 0x004 */
    uint32 CSEND;                           /* 0x008 */
    uint32 unused;                          /* 0x00c */
    uint32 ROW00_0;                         /* 0x010 */
    uint32 ROW00_1;                         /* 0x014 */
    uint32 ROW01_0;                         /* 0x018 */
    uint32 ROW01_1;                         /* 0x01c */
    uint32 unused0[4];
    uint32 ROW20_0;                         /* 0x030 */
    uint32 ROW20_1;                         /* 0x034 */
    uint32 ROW21_0;                         /* 0x038 */
    uint32 ROW21_1;                         /* 0x03c */
    uint32 unused1[4];
    uint32 COL00_0;                         /* 0x050 */
    uint32 COL00_1;                         /* 0x054 */
    uint32 COL01_0;                         /* 0x058 */
    uint32 COL01_1;                         /* 0x05c */
    uint32 unused2[4];
    uint32 COL20_0;                         /* 0x070 */
    uint32 COL20_1;                         /* 0x074 */
    uint32 COL21_0;                         /* 0x078 */
    uint32 COL21_1;                         /* 0x07c */
    uint32 unused3[4];
    uint32 BNK10;                           /* 0x090 */
    uint32 BNK32;                           /* 0x094 */
    uint32 unused4[26];
    uint32 DCMD;                            /* 0x100 */
#define DCMD_CS1          (1 << 5)
#define DCMD_CS0          (1 << 4)
#define DCMD_SET_SREF     4
    uint32 DMODE_0;                         /* 0x104 */
    uint32 DMODE_1;                         /* 0x108 */
#define DMODE_1_DRAMSLEEP (1 << 11)	
    uint32 CLKS;                            /* 0x10c */
    uint32 ODT;                             /* 0x110 */
    uint32 TIM1_0;                          /* 0x114 */
    uint32 TIM1_1;                          /* 0x118 */
    uint32 TIM2;                            /* 0x11c */
    uint32 CTL_CRC;                         /* 0x120 */
    uint32 DOUT_CRC;                        /* 0x124 */
    uint32 DIN_CRC;                         /* 0x128 */
    uint32 unused5[53];

    DDRPhyControl           PhyControl;             /* 0x200 */
    DDRPhyByteLaneControl   PhyByteLane0Control;    /* 0x300 */
    DDRPhyByteLaneControl   PhyByteLane1Control;    /* 0x400 */
    uint32 unused6[192];

    uint32 GCFG;                            /* 0x800 */
    uint32 unused8;                         /* 0x804 */
    uint32 unused9;                         /* 0x808 */
    uint32 ARB;                             /* 0x80c */
    uint32 PI_GCF;                          /* 0x810 */
    uint32 PI_UBUS_CTL;                     /* 0x814 */
    uint32 PI_MIPS_CTL;                     /* 0x818 */
    uint32 PI_DSL_MIPS_CTL;                 /* 0x81c */
    uint32 PI_DSL_PHY_CTL;                  /* 0x820 */
    uint32 PI_UBUS_ST;                      /* 0x824 */
    uint32 PI_MIPS_ST;                      /* 0x828 */
    uint32 PI_DSL_MIPS_ST;                  /* 0x82c */
    uint32 PI_DSL_PHY_ST;                   /* 0x830 */
    uint32 PI_UBUS_SMPL;                    /* 0x834 */
    uint32 TESTMODE;                        /* 0x838 */
    uint32 TEST_CFG1;                       /* 0x83c */
    uint32 TEST_PAT;                        /* 0x840 */
    uint32 TEST_COUNT;                      /* 0x844 */
    uint32 TEST_CURR_COUNT;                 /* 0x848 */
    uint32 TEST_ADDR_UPDT;                  /* 0x84c */
    uint32 TEST_ADDR;                       /* 0x850 */
    uint32 TEST_DATA0_0;                    /* 0x854 */
    uint32 TEST_DATA0_1;                    /* 0x858 */
    uint32 TEST_DATA0_2;                    /* 0x85c */
    uint32 TEST_DATA0_3;                    /* 0x860 */
    uint32 TEST_DATA1_0;                    /* 0x864 */
    uint32 TEST_DATA1_1;                    /* 0x868 */
    uint32 TEST_DATA1_2;                    /* 0x86c */
    uint32 TEST_DATA1_3;                    /* 0x870 */
    uint32 REPLY_DATA0;                     /* 0x874 */
    uint32 REPLY_DATA1;                     /* 0x878 */
    uint32 REPLY_DATA2;                     /* 0x87c */
    uint32 REPLY_DATA3;                     /* 0x880 */
    uint32 REPLY_STAT;                      /* 0x884 */	
} MEMCControl;

#define DDR ((volatile MEMCControl * const) DDR_BASE)


/*
** Peripheral Controller
*/

#define IRQ_BITS 64
typedef struct  {
    uint64         ExtIrqMask;
    uint64         IrqMask;
    uint64         ExtIrqStatus;
    uint64         IrqStatus;
} IrqControl_t;

typedef struct  {
    uint32         ExtIrqMaskHi;
    uint32         ExtIrqMaskLo;
    uint32         IrqMaskHi;
    uint32         IrqMaskLo;
    uint32         ExtIrqStatusHi;
    uint32         ExtIrqStatusLo;
    uint32         IrqStatusHi;
    uint32         IrqStatusLo;
} IrqControl_32_t;

typedef struct PerfControl {
     uint32        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   16
#define CHIP_ID_MASK    (0xffff << CHIP_ID_SHIFT)
#define CHIP_VAR_SHIFT   8
#define CHIP_VAR_MASK    (0xff << CHIP_VAR_SHIFT)
#define REV_ID_MASK     0xff

     uint32        blkEnables;        /* (04) word 1 */
#define TBUS_CLK_EN              (1 << 27)
#define USBH_CLK_EN              (1 << 16) /* Only one bit for USB CLK */
#define USBD_CLK_EN              (1 << 16)
#define ROBOSW_CLK250_CLK_EN     (1 << 15)
#define APM_AUDIO_UBUS_CLKEN     (1 << 14)
#define APM_AUDIO_COMMON_CLKEN   (1 << 13)
#define APM_AUDIO_B_CLKEN        (1 << 12)
#define APM_AUDIO_A_CLKEN        (1 << 11)
#define APM_BMU_CLKEN            (1 << 10)
#define BMU_CLK_EN               (1 << 10)
#define APM_PCM_CLKEN            (1 << 9)
#define NAND_CLK_EN              (1 << 8)
#define PCIE_CLK_EN              (1 << 7)
#define HS_SPI_CLK_EN            (1 << 6)
#define SPI_CLK_EN               (1 << 5)
#define ROBOSW_CLK_EN    (1 << 4)
#define FAP1_CLK_EN      (1 << 3)
#define FAP0_CLK_EN      (1 << 2)
#define MIPS_CLK_EN      (1 << 1)
#define DISABLE_GLESS    (1 << 0)

#define APM_AUIDIO_UBUS_CLKEN 		APM_AUDIO_UBUS_CLKEN
#define APM_AUIDIO_COMMON_CLKEN 	APM_AUDIO_COMMON_CLKEN
#define APM_AUIDIO_B_CLKEN 				APM_AUDIO_B_CLKEN
#define APM_AUIDIO_A_CLKEN				APM_AUDIO_A_CLKEN

     uint32        pll_control;       /* (08) word 2. Misnomer, this is actually a watchdog timer. */
#define SOFT_RESET              0x00000001      // 0

     uint32        deviceTimeoutEn;   /* (0c) word 3 */
     uint32        softResetB;        /* (10) word 4 */
#define SOFT_RST_NAND_FLASH     (1 << 22)	 
#define SOFT_RST_EPON           (1 << 21)
#define SOFT_RST_GPHY_CAL       (1 << 20)
#define SOFT_RST_GPHY1          (1 << 19)
#define SOFT_RST_GPHY0          (1 << 18)
#define SOFT_RST2_PCIE          (1 << 17)
#define SOFT_RST_PCIE_HARD      SOFT_RST2_PCIE
#define SOFT_RST_FAP1           (1 << 16)
#define SOFT_RST_FAP0           (1 << 15)
#define SOFT_RST_DDR_PHY        (1 << 14)
#define SOFT_RST_USBH           (1 << 13)
#define SOFT_RST_USBD           (1 << 12)
#define SOFT_RST_SWITCH         (1 << 11)
#define SOFT_RST_PCIE_EXT       (1 << 10)
#define SOFT_RST_PCIE           (1 << 9)
#define SOFT_RST_PCIE_CORE      (1 << 8)
#define SOFT_RST_EPHY           (1 << 7)
#define SOFT_RST_APM_HARD_SOFT  (1 << 6)
#define SOFT_RST_APM_AUDIO_SOFT (1 << 5)
#define SOFT_RST_PCM            (1 << 4)
#define SOFT_RST_HVG_A          (1 << 3)
#define SOFT_RST_HVG_B          (1 << 2)
#define SOFT_RST_BMU            (1 << 1)
#define SOFT_RST_SPI            (1 << 0)

    uint32        diagControl;        /* (14) word 5 */
    uint32        ExtIrqCfg;          /* (18) word 6*/
    uint32        unused1;            /* (1c) word 7 */
#define EI_SENSE_SHFT   0
#define EI_STATUS_SHFT  4
#define EI_CLEAR_SHFT   8
#define EI_MASK_SHFT    12
#define EI_INSENS_SHFT  16
#define EI_LEVEL_SHFT   20

    union {
         IrqControl_t     IrqControl[3];    /* (20) (40) (60) */
         IrqControl_32_t  IrqControl32[3];  /* (20) (40) (60) */
    };
} PerfControl;

#define PERF ((volatile PerfControl * const) PERF_BASE)

/*
** Timer
*/
typedef struct Timer {
    uint16        unused0;
    byte          TimerMask;
#define TIMER0EN        0x01
#define TIMER1EN        0x02
#define TIMER2EN        0x04
    byte          TimerInts;
#define TIMER0          0x01
#define TIMER1          0x02
#define TIMER2          0x04
#define WATCHDOG        0x08
    uint32        TimerCtl0;
    uint32        TimerCtl1;
    uint32        TimerCtl2;
#define TIMERENABLE     0x80000000
#define RSTCNTCLR       0x40000000
    uint32        TimerCnt0;
    uint32        TimerCnt1;
    uint32        TimerCnt2;
    uint32        WatchDogDefCount;

    /* Write 0xff00 0x00ff to Start timer
     * Write 0xee00 0x00ee to Stop and re-load default count
     * Read from this register returns current watch dog count
     */
    uint32        WatchDogCtl;

    /* Number of 50-MHz ticks for WD Reset pulse to last */
    uint32        WDResetCount;

    uint32        EnSwPLL;
    uint32        ClkRstCtl;
#define POR_RESET_STATUS            (1 << 31)
#define HW_RESET_STATUS             (1 << 30)
#define SW_RESET_STATUS             (1 << 29)
#define USB_REF_CLKEN               (1 << 18)
#define FAP2_PLL_CLKEN              (1 << 15)
#define FAP2_PLL_FREQ_SHIFT         12
#define FAP1_PLL_CLKEN              (1 << 11)
#define FAP1_PLL_FREQ_SHIFT         8
} Timer;

#define TIMER ((volatile Timer * const) TIMR_BASE)

/*
** UART
*/
typedef struct UartChannel {
    byte          unused0;
    byte          control;
#define BRGEN           0x80    /* Control register bit defs */
#define TXEN            0x40
#define RXEN            0x20
#define LOOPBK          0x10
#define TXPARITYEN      0x08
#define TXPARITYEVEN    0x04
#define RXPARITYEN      0x02
#define RXPARITYEVEN    0x01

    byte          config;
#define XMITBREAK       0x40
#define BITS5SYM        0x00
#define BITS6SYM        0x10
#define BITS7SYM        0x20
#define BITS8SYM        0x30
#define ONESTOP         0x07
#define TWOSTOP         0x0f
    /* 4-LSBS represent STOP bits/char
     * in 1/8 bit-time intervals.  Zero
     * represents 1/8 stop bit interval.
     * Fifteen represents 2 stop bits.
     */
    byte          fifoctl;
#define RSTTXFIFOS      0x80
#define RSTRXFIFOS      0x40
    /* 5-bit TimeoutCnt is in low bits of this register.
     *  This count represents the number of characters
     *  idle times before setting receive Irq when below threshold
     */
    uint32        baudword;
    /* When divide SysClk/2/(1+baudword) we should get 32*bit-rate
     */

    byte          txf_levl;       /* Read-only fifo depth */
    byte          rxf_levl;       /* Read-only fifo depth */
    byte          fifocfg;        /* Upper 4-bits are TxThresh, Lower are
                                   *      RxThreshold.  Irq can be asserted
                                   *      when rx fifo> thresh, txfifo<thresh
                                   */
    byte          prog_out;       /* Set value of DTR (Bit0), RTS (Bit1)
                                   *  if these bits are also enabled to GPIO_o
                                   */
#define DTREN   0x01
#define RTSEN   0x02

    byte          unused1;
    byte          DeltaIPEdgeNoSense;     /* Low 4-bits, set corr bit to 1 to
                                           * detect irq on rising AND falling
                                           * edges for corresponding GPIO_i
                                           * if enabled (edge insensitive)
                                           */
    byte          DeltaIPConfig_Mask;     /* Upper 4 bits: 1 for posedge sense
                                           *      0 for negedge sense if
                                           *      not configured for edge
                                           *      insensitive (see above)
                                           * Lower 4 bits: Mask to enable change
                                           *  detection IRQ for corresponding
                                           *  GPIO_i
                                           */
    byte          DeltaIP_SyncIP;         /* Upper 4 bits show which bits
                                           *  have changed (may set IRQ).
                                           *  read automatically clears bit
                                           * Lower 4 bits are actual status
                                           */

    uint16        intMask;                /* Same Bit defs for Mask and status */
    uint16        intStatus;
#define DELTAIP         0x0001
#define TXUNDERR        0x0002
#define TXOVFERR        0x0004
#define TXFIFOTHOLD     0x0008
#define TXREADLATCH     0x0010
#define TXFIFOEMT       0x0020
#define RXUNDERR        0x0040
#define RXOVFERR        0x0080
#define RXTIMEOUT       0x0100
#define RXFIFOFULL      0x0200
#define RXFIFOTHOLD     0x0400
#define RXFIFONE        0x0800
#define RXFRAMERR       0x1000
#define RXPARERR        0x2000
#define RXBRK           0x4000

    uint16        unused2;
    uint16        Data;                   /* Write to TX, Read from RX */
                                          /* bits 11:8 are BRK,PAR,FRM errors */

    uint32        unused3;
    uint32        unused4;
} Uart;

#define UART ((volatile Uart * const) UART_BASE)

/*
** Gpio Controller
*/

typedef struct GpioControl {
    uint64      GPIODir;                    /* 0 */
    uint64      GPIOio;                     /* 8 */
    uint32      LEDCtrl;
    uint32      SpiSlaveCfg;                /* 14 */
    uint32      GPIOMode;                   /* 18 */
#define GPIO_MODE_SWITCH_LED_DATA    (1<<31)
#define GPIO_MODE_SWITCH_LED_CLK     (1<<30)
#define GPIO_MODE_DSL_NTR_PULSE_OUT2 (1<<29)
#define GPIO_MODE_NTR_PULSE_IN2     (1<<28)
#define GPIO_MODE_UART2_SDOUT2      (1<<27)
#define GPIO_MODE_UART2_SDIN2       (1<<26)
#define GPIO_MODE_UART2_SRTS2       (1<<25)
#define GPIO_MODE_UART2_SCTS2       (1<<24)
#define GPIO_MODE_PCIE_CLKREQ_B     (1<<23)
#define GPIO_MODE_APM_HVG_MAX_PWM   (1<<22)
#define GPIO_MODE_APM_PCM_CLK       (1<<21)
#define GPIO_MODE_APM_PCM_FS        (1<<20)
#define GPIO_MODE_APM_PCM_SDOUT     (1<<19)
#define GPIO_MODE_APM_PCM_SDIN      (1<<18)
#define GPIO_MODE_HS_SPI_SS_3       (1<<17)
#define GPIO_MODE_HS_SPI_SS_2       (1<<16)
#define GPIO_MODE_UART2_SDOUT       (1<<15)
#define GPIO_MODE_UART2_SDIN        (1<<14)
#define GPIO_MODE_UART2_SRTS        (1<<13)
#define GPIO_MODE_UART2_SCTS        (1<<12)
#define GPIO_MODE_NTR_PULSE_OUT     (1<<11)
#define GPIO_MODE_NTR_PULSE_IN      (1<<10)
#define GPIO_MODE_HS_SPI_SS_4       (1<<9)
#define GPIO_MODE_VREG_CLK          (1<<8)
#define GPIO_MODE_SERIAL_LED_DATA   (1<< 1)
#define GPIO_MODE_SERIAL_LED_CLK    (1<< 0)
    uint32      GPIOCtrl;                   /* 1C */
#define GPIO_CTRL_APM_CTRL_OVERRIDE_SHIFT  37            /* Shift for 64bit gpio mask so that target bits fit into APM gpio override field */
#define GPIO_CTRL_EPON_CTRL_OVERRIDE_SHIFT 33            /* Shift for 64bit gpio mask so that target bits fit into EPON gpio override field */
#define GPIO_EPON_PERIPH_CTRL_S     8
#define GPIO_EPON_PERIPH_CTRL_M     0xFFF
#define GPIO_APM_PERIPH_CTRL_S      0
#define GPIO_APM_PERIPH_CTRL_M      0xF
    uint32      unused3;                 /* 20 */
    uint32      PadControl;                /* 24 */
    uint32      TestControl;                /* 28 */
    uint32      unused4;                 /* 2c */
    uint32      RoboSWLEDControl;           /* 30 */
#define LED_BICOLOR_SPD             (1 << 30)
    uint32      RoboSWLEDLSR;               /* 34 */
    uint32      GPIOBaseMode;               /* 38 */
#define GPIO_BASE_EPON_SPI_GPIO_OVERRIDE (1<<6)
#define GPIO_BASE_EPON_MII_GPIO_OVERRIDE (1<<5)
#define GPIO_BASE_GPIO35_OVERRIDE       (1<<4)
#define GPIO_BASE_ZAR_OVERRIDE          (1<<3)
#define NAND_GPIO_OVERRIDE              (1<<2)
    uint32      RoboswEphyCtrl;             /* 3C */
#define EPHY_PLL_LOCK               (1<<27)
#define EPHY_ATEST_25MHZ_EN         (1<<26)
#define EPHY_PWR_DOWN_DLL           (1<<25)
#define EPHY_PWR_DOWN_BIAS          (1<<24)
#define EPHY_PWR_DOWN_TX_2          (1<<13)
#define EPHY_PWR_DOWN_RX_2          (1<<12)
#define EPHY_PWR_DOWN_SD_2          (1<<11)
#define EPHY_PWR_DOWN_RD_2          (1<<10)
#define EPHY_PWR_DOWN_2             (1<<9)
#define EPHY_PWR_DOWN_TX_1          (1<<8)
#define EPHY_PWR_DOWN_RX_1          (1<<7)
#define EPHY_PWR_DOWN_SD_1          (1<<6)
#define EPHY_PWR_DOWN_RD_1          (1<<5)
#define EPHY_PWR_DOWN_1             (1<<4)
#define EPHY_RST_2                  (1<<1)
#define EPHY_RST_1                  (1<<0)
#define EPHY_RST_SHIFT		    0x0
#define EPHY_RST_MASK		    (0x3<<EPHY_RST_SHIFT)
    uint32      RoboswSwitchCtrl;           /* 40 */
#define RSW_MII_2_AMP_EN            (1<<26)
#define RSW_MII_2_SEL_SHIFT         24
#define RSW_MII_1_AMP_EN            (1<<18)
#define RSW_MII_1_SEL_SHIFT         16
#define RSW_MII_SEL_3P3V            0
#define RSW_MII_SEL_2P5V            1
#define RSW_MII_SEL_1P5V            2
#define RSW_SPI_MODE                (1<<11)
#define RSW_BC_SUPP_EN              (1<<10)
#define RSW_CLK_FREQ_MASK           (3<<8)
#define RSW_ENF_DFX_FLOW            (1<<7)
#define RSW_ENH_DFX_FLOW            (1<<6)
#define RSW_GRX_0_SETUP             (1<<5)
#define RSW_GTX_0_SETUP             (1<<4)
#define RSW_HW_FWDG_EN              (1<<3)
#define RSW_QOS_EN                  (1<<2)
#define RSW_WD_CLR_EN               (1<<1)
#define RSW_MII_DUMB_FWDG_EN        (1<<0)
    uint32      RegFileTmCtl;               /* 44 */
    uint32      RingOscCtrl0;               /* 48 */
#define RING_OSC_256_CYCLES         8
#define RING_OSC_512_CYCLES         9
#define RING_OSC_1024_CYCLES        10

    uint32      RingOscCtrl1;               /* 4C */
#define RING_OSC_ENABLE_MASK        (0xff<<24)
#define RING_OSC_ENABLE_SHIFT       24
#define RING_OSC_MAX                8
#define RING_OSC_COUNT_RESET        (0x1<<23)
#define RING_OSC_SELECT_MASK        (0x7<<20)
#define RING_OSC_SELECT_SHIFT       20
#define RING_OSC_IRQ                (0x1<<18)
#define RING_OSC_COUNTER_OVERFLOW   (0x1<<17)
#define RING_OSC_COUNTER_BUSY       (0x1<<16)
#define RING_OSC_COUNT_MASK         0x0000ffff

    uint32      DisTpOut;                   /* 50 */
    uint32      DisTpOutUpper;               /* 54 */
    uint32      RoboswGphyCtrl;             /* 58 */
#define GPHY_LPI_FEATURE_EN_DEF_MASK (3<<16)
#define GPHY_EEE_1000BASE_T_DEF      (1<<13)
#define GPHY_EEE_100BASE_TX_DEF      (1<<12)
#define GPHY_EEE_PCS_1000BASE_T_DEF  (1<<10)
#define GPHY_EEE_PCS_100BASE_TX_DEF  (1<<9)
#define GPHY2_LOW_PWR_SD             (1<<6)	
#define GPHY2_LOW_PWR                (1<<5)	
#define GPHY1_LOW_PWR_SD             (1<<4)	
#define GPHY1_LOW_PWR                (1<<3)
#define GPHY_FORCE_DLL_EN            (1<<2)
#define GPHY_IDDQ_BIAS               (1<<0)
    uint32      unused5[4];                 /* 5C - 68 */
    uint32      DieRevID;                   /* 6C */
    uint32      unused6;                    /* 60 */
    uint32      DiagSelControl;             /* 74*/
    uint32      DiagReadBack;               /* 78*/
    uint32      DiagReadBackHi;             /* 7C */
    uint32      unused7;            /* 80 */
} GpioControl;

#define GPIO ((volatile GpioControl * const) GPIO_BASE)

/* Number to mask conversion macro used for GPIODir and GPIOio */
#define GPIO_NUM_MAX                    52
#define GPIO_NUM_TO_MASK(X)             ( (((X) & BP_GPIO_NUM_MASK) <= GPIO_NUM_MAX) ? ((uint64)1 << ((X) & BP_GPIO_NUM_MASK)) : (0) )
 
/*
** Spi Controller
*/

typedef struct SpiControl {
  uint16        spiMsgCtl;              /* (0x0) control byte */
#define FULL_DUPLEX_RW                  0
#define HALF_DUPLEX_W                   1
#define HALF_DUPLEX_R                   2
#define SPI_MSG_TYPE_SHIFT              14
#define SPI_BYTE_CNT_SHIFT              0
  byte          spiMsgData[0x21e];      /* (0x02 - 0x21f) msg data */
  byte          unused0[0x1e0];
  byte          spiRxDataFifo[0x220];   /* (0x400 - 0x61f) rx data */
  byte          unused1[0xe0];

  uint16        spiCmd;                 /* (0x700): SPI command */
#define SPI_CMD_NOOP                    0
#define SPI_CMD_SOFT_RESET              1
#define SPI_CMD_HARD_RESET              2
#define SPI_CMD_START_IMMEDIATE         3

#define SPI_CMD_COMMAND_SHIFT           0
#define SPI_CMD_COMMAND_MASK            0x000f

#define SPI_CMD_DEVICE_ID_SHIFT         4
#define SPI_CMD_PREPEND_BYTE_CNT_SHIFT  8
#define SPI_CMD_ONE_BYTE_SHIFT          11
#define SPI_CMD_ONE_WIRE_SHIFT          12
#define SPI_DEV_ID_0                    0
#define SPI_DEV_ID_1                    1
#define SPI_DEV_ID_2                    2
#define SPI_DEV_ID_3                    3
#define ZSI_SPI_DEV_ID                  3

  byte          spiIntStatus;           /* (0x702): SPI interrupt status */
  byte          spiMaskIntStatus;       /* (0x703): SPI masked interrupt status */

  byte          spiIntMask;             /* (0x704): SPI interrupt mask */
#define SPI_INTR_CMD_DONE               0x01
#define SPI_INTR_RX_OVERFLOW            0x02
#define SPI_INTR_INTR_TX_UNDERFLOW      0x04
#define SPI_INTR_TX_OVERFLOW            0x08
#define SPI_INTR_RX_UNDERFLOW           0x10
#define SPI_INTR_CLEAR_ALL              0x1f

  byte          spiStatus;              /* (0x705): SPI status */
#define SPI_RX_EMPTY                    0x02
#define SPI_CMD_BUSY                    0x04
#define SPI_SERIAL_BUSY                 0x08

  byte          spiClkCfg;              /* (0x706): SPI clock configuration */
#define SPI_CLK_0_391MHZ                1
#define SPI_CLK_0_781MHZ                2 /* default */
#define SPI_CLK_1_563MHZ                3
#define SPI_CLK_3_125MHZ                4
#define SPI_CLK_6_250MHZ                5
#define SPI_CLK_12_50MHZ                6
#define SPI_CLK_MASK                    0x07
#define SPI_SSOFFTIME_MASK              0x38
#define SPI_SSOFFTIME_SHIFT             3
#define SPI_BYTE_SWAP                   0x80

  byte          spiFillByte;            /* (0x707): SPI fill byte */
  byte          unused2;
  byte          spiMsgTail;             /* (0x709): msgtail */
  byte          unused3;
  byte          spiRxTail;              /* (0x70B): rxtail */
} SpiControl;

#define SPI ((volatile SpiControl * const) SPI_BASE)


/*
** High-Speed SPI Controller
*/

#define __mask(end, start)      (((1 << ((end - start) + 1)) - 1) << start)
typedef struct HsSpiControl {

  uint32    hs_spiGlobalCtrl;   // 0x0000
#define HS_SPI_MOSI_IDLE        (1 << 18)
#define HS_SPI_CLK_POLARITY      (1 << 17)
#define HS_SPI_CLK_GATE_SSOFF       (1 << 16)
#define HS_SPI_PLL_CLK_CTRL     (8)
#define HS_SPI_PLL_CLK_CTRL_MASK    __mask(15, HS_SPI_PLL_CLK_CTRL)
#define HS_SPI_SS_POLARITY      (0)
#define HS_SPI_SS_POLARITY_MASK     __mask(7, HS_SPI_SS_POLARITY)

  uint32    hs_spiExtTrigCtrl;  // 0x0004
#define HS_SPI_TRIG_RAW_STATE   (24)
#define HS_SPI_TRIG_RAW_STATE_MASK  __mask(31, HS_SPI_TRIG_RAW_STATE)
#define HS_SPI_TRIG_LATCHED     (16)
#define HS_SPI_TRIG_LATCHED_MASK    __mask(23, HS_SPI_TRIG_LATCHED)
#define HS_SPI_TRIG_SENSE       (8)
#define HS_SPI_TRIG_SENSE_MASK      __mask(15, HS_SPI_TRIG_SENSE)
#define HS_SPI_TRIG_TYPE        (0)
#define HS_SPI_TRIG_TYPE_MASK       __mask(7, HS_SPI_TRIG_TYPE)
#define HS_SPI_TRIG_TYPE_EDGE       (0)
#define HS_SPI_TRIG_TYPE_LEVEL      (1)

  uint32    hs_spiIntStatus;    // 0x0008
#define HS_SPI_IRQ_PING1_USER       (28)
#define HS_SPI_IRQ_PING1_USER_MASK  __mask(31, HS_SPI_IRQ_PING1_USER)
#define HS_SPI_IRQ_PING0_USER       (24)
#define HS_SPI_IRQ_PING0_USER_MASK  __mask(27, HS_SPI_IRQ_PING0_USER)

#define HS_SPI_IRQ_PING1_CTRL_INV   (1 << 12)
#define HS_SPI_IRQ_PING1_POLL_TOUT  (1 << 11)
#define HS_SPI_IRQ_PING1_TX_UNDER   (1 << 10)
#define HS_SPI_IRQ_PING1_RX_OVER    (1 << 9)
#define HS_SPI_IRQ_PING1_CMD_DONE   (1 << 8)

#define HS_SPI_IRQ_PING0_CTRL_INV   (1 << 4)
#define HS_SPI_IRQ_PING0_POLL_TOUT  (1 << 3)
#define HS_SPI_IRQ_PING0_TX_UNDER   (1 << 2)
#define HS_SPI_IRQ_PING0_RX_OVER    (1 << 1)
#define HS_SPI_IRQ_PING0_CMD_DONE   (1 << 0)

  uint32    hs_spiIntStatusMasked;  // 0x000C
#define HS_SPI_IRQSM__PING1_USER    (28)
#define HS_SPI_IRQSM__PING1_USER_MASK   __mask(31, HS_SPI_IRQSM__PING1_USER)
#define HS_SPI_IRQSM__PING0_USER    (24)
#define HS_SPI_IRQSM__PING0_USER_MASK   __mask(27, HS_SPI_IRQSM__PING0_USER)

#define HS_SPI_IRQSM__PING1_CTRL_INV    (1 << 12)
#define HS_SPI_IRQSM__PING1_POLL_TOUT   (1 << 11)
#define HS_SPI_IRQSM__PING1_TX_UNDER    (1 << 10)
#define HS_SPI_IRQSM__PING1_RX_OVER (1 << 9)
#define HS_SPI_IRQSM__PING1_CMD_DONE    (1 << 8)

#define HS_SPI_IRQSM__PING0_CTRL_INV    (1 << 4)
#define HS_SPI_IRQSM__PING0_POLL_TOUT   (1 << 3)
#define HS_SPI_IRQSM__PING0_TX_UNDER    (1 << 2)
#define HS_SPI_IRQSM__PING0_RX_OVER     (1 << 1)
#define HS_SPI_IRQSM__PING0_CMD_DONE    (1 << 0)

  uint32    hs_spiIntMask;      // 0x0010
#define HS_SPI_IRQM_PING1_USER      (28)
#define HS_SPI_IRQM_PING1_USER_MASK __mask(31, HS_SPI_IRQM_PING1_USER)
#define HS_SPI_IRQM_PING0_USER      (24)
#define HS_SPI_IRQM_PING0_USER_MASK __mask(27, HS_SPI_IRQM_PING0_USER)

#define HS_SPI_IRQM_PING1_CTRL_INV  (1 << 12)
#define HS_SPI_IRQM_PING1_POLL_TOUT (1 << 11)
#define HS_SPI_IRQM_PING1_TX_UNDER  (1 << 10)
#define HS_SPI_IRQM_PING1_RX_OVER   (1 << 9)
#define HS_SPI_IRQM_PING1_CMD_DONE  (1 << 8)

#define HS_SPI_IRQM_PING0_CTRL_INV  (1 << 4)
#define HS_SPI_IRQM_PING0_POLL_TOUT (1 << 3)
#define HS_SPI_IRQM_PING0_TX_UNDER  (1 << 2)
#define HS_SPI_IRQM_PING0_RX_OVER   (1 << 1)
#define HS_SPI_IRQM_PING0_CMD_DONE  (1 << 0)

#define HS_SPI_INTR_CLEAR_ALL       (0xFF001F1F)

  uint32    hs_spiFlashCtrl;    // 0x0014
#define HS_SPI_FCTRL_MB_ENABLE      (23)
#define HS_SPI_FCTRL_SS_NUM         (20)
#define HS_SPI_FCTRL_SS_NUM_MASK    __mask(22, HS_SPI_FCTRL_SS_NUM)
#define HS_SPI_FCTRL_PROFILE_NUM    (16)
#define HS_SPI_FCTRL_PROFILE_NUM_MASK   __mask(18, HS_SPI_FCTRL_PROFILE_NUM)
#define HS_SPI_FCTRL_DUMMY_BYTES    (10)
#define HS_SPI_FCTRL_DUMMY_BYTES_MASK   __mask(11, HS_SPI_FCTRL_DUMMY_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES     (8)
#define HS_SPI_FCTRL_ADDR_BYTES_MASK    __mask(9, HS_SPI_FCTRL_ADDR_BYTES)
#define HS_SPI_FCTRL_ADDR_BYTES_2   (0)
#define HS_SPI_FCTRL_ADDR_BYTES_3   (1)
#define HS_SPI_FCTRL_ADDR_BYTES_4   (2)
#define HS_SPI_FCTRL_READ_OPCODE    (0)
#define HS_SPI_FCTRL_READ_OPCODE_MASK   __mask(7, HS_SPI_FCTRL_READ_OPCODE)

  uint32    hs_spiFlashAddrBase;    // 0x0018

  char      fill0[0x80 - 0x18];

  uint32    hs_spiPP_0_Cmd;     // 0x0080
#define HS_SPI_PP_SS_NUM        (12)
#define HS_SPI_PP_SS_NUM_MASK       __mask(14, HS_SPI_PP_SS_NUM)
#define HS_SPI_PP_PROFILE_NUM       (8)
#define HS_SPI_PP_PROFILE_NUM_MASK  __mask(10, HS_SPI_PP_PROFILE_NUM)

} HsSpiControl;

typedef struct HsSpiPingPong {

    uint32 command;
#define HS_SPI_SS_NUM (12)
#define HS_SPI_PROFILE_NUM (8)
#define HS_SPI_TRIGGER_NUM (4)
#define HS_SPI_COMMAND_VALUE (0)
    #define HS_SPI_COMMAND_NOOP (0)
    #define HS_SPI_COMMAND_START_NOW (1)
    #define HS_SPI_COMMAND_START_TRIGGER (2)
    #define HS_SPI_COMMAND_HALT (3)
    #define HS_SPI_COMMAND_FLUSH (4)

    uint32 status;
#define HS_SPI_ERROR_BYTE_OFFSET (16)
#define HS_SPI_WAIT_FOR_TRIGGER (2)
#define HS_SPI_SOURCE_BUSY (1)
#define HS_SPI_SOURCE_GNT (0)

    uint32 fifo_status;
    uint32 control;

} HsSpiPingPong;

typedef struct HsSpiProfile {

    uint32 clk_ctrl;
#define HS_SPI_ACCUM_RST_ON_LOOP (15)
#define HS_SPI_SPI_CLK_2X_SEL (14)
#define HS_SPI_FREQ_CTRL_WORD (0)

    uint32 signal_ctrl;
#define	HS_SPI_ASYNC_INPUT_PATH (1 << 16)
#define	HS_SPI_LAUNCH_RISING    (1 << 13)
#define	HS_SPI_LATCH_RISING     (1 << 12)

    uint32 mode_ctrl;
#define HS_SPI_PREPENDBYTE_CNT (24)
#define HS_SPI_MODE_ONE_WIRE (20)
#define HS_SPI_MULTIDATA_WR_SIZE (18)
#define HS_SPI_MULTIDATA_RD_SIZE (16)
#define HS_SPI_MULTIDATA_WR_STRT (12)
#define HS_SPI_MULTIDATA_RD_STRT (8)
#define HS_SPI_FILLBYTE (0)

    uint32 polling_config;
    uint32 polling_and_mask;
    uint32 polling_compare;
    uint32 polling_timeout;
    uint32 reserved;

} HsSpiProfile;

#define HS_SPI_OP_CODE 13
    #define HS_SPI_OP_SLEEP (0)
    #define HS_SPI_OP_READ_WRITE (1)
    #define HS_SPI_OP_WRITE (2)
    #define HS_SPI_OP_READ (3)
    #define HS_SPI_OP_SETIRQ (4)

#define HS_SPI ((volatile HsSpiControl * const) HSSPIM_BASE)
#define HS_SPI_PINGPONG0 ((volatile HsSpiPingPong * const) (HSSPIM_BASE+0x80))
#define HS_SPI_PINGPONG1 ((volatile HsSpiPingPong * const) (HSSPIM_BASE+0xc0))
#define HS_SPI_PROFILES ((volatile HsSpiProfile * const) (HSSPIM_BASE+0x100))
#define HS_SPI_FIFO0 ((volatile uint8 * const) (HSSPIM_BASE+0x200))
#define HS_SPI_FIFO1 ((volatile uint8 * const) (HSSPIM_BASE+0x400))

/*
** Misc Register Set Definitions.
*/

typedef struct Misc {
    uint32  unused1;                            /* 0x00 */
    uint32  miscSerdesCtrl;                     /* 0x04 */
#define SERDES_PCIE_ENABLE                      0x00000001
#define SERDES_PCIE_EXD_ENABLE                  (1<<15)

    uint32  miscSerdesSts;                      /* 0x08 */
    uint32  miscIrqOutMask;                     /* 0x0C */
#define MISC_PCIE_EP_IRQ_MASK0                  (1<<0)
#define MISC_PCIE_EP_IRQ_MASK1                  (1<<1)

    uint32  miscMemcControl;                    /* 0x10 */
#define MISC_MEMC_CONTROL_MC_UBUS_ASYNC_MODE    (1<<3)
#define MISC_MEMC_CONTROL_MC_LMB_ASYNC_MODE     (1<<2)
#define MISC_MEMC_CONTROL_DDR_TEST_DONE         (1<<1)
#define MISC_MEMC_CONTROL_DDR_TEST_DISABLE      (1<<0)

    uint32  miscStrapBus;                       /* 0x14 */
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT      21
#define MISC_STRAP_BUS_MIPS_PLL_FVCO_MASK       (0xF<<MISC_STRAP_BUS_MIPS_PLL_FVCO_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT    18
#define MISC_STRAP_BUS_RESET_OUT_DELAY_MASK     (0x7<<MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_100MS    (0x5<<MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_150MS    (0x2<<MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_50MS     (0x1<<MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_RESET_OUT_DELAY_251MS    (0x4<<MISC_STRAP_BUS_RESET_OUT_DELAY_SHIFT)
#define MISC_STRAP_BUS_PLL_USE_LOCK             17
#define MISC_STRAP_BUS_BOOT_SEL_SHIFT           13
#define MISC_STRAP_BUS_BOOT_SEL_MASK            (0x7<<MISC_STRAP_BUS_BOOT_SEL_SHIFT)
#define MISC_STRAP_BUS_BOOT_SERIAL              0x7
#define MISC_STRAP_BUS_NAND_ECC_SELECT_SHIFT    13
#define MISC_STRAP_BUS_NAND_ECC_SELECT_MASK     (0x7<<MISC_STRAP_BUS_NAND_ECC_SELECT_SHIFT)
#define MISC_STRAP_BUS_DISABLE_NAND_ECC         (1<<12)
#define MISC_STRAP_BUS_HS_SPIM_24B_N_32B_ADDR   (1<<10)
#define MISC_STRAP_BUS_HS_SPIM_CLK_SLOW_N_FAST  (1<<9)
#define MISC_STRAP_BUS_LS_SPI_SLAVE_DISABLE     (1<<8)
#define MISC_STRAP_BUS_APM_PICO_ROM_BOOT        (1<<7)
#define MISC_STRAP_BUS_XTAL_ADJ                 (1<<5)
#define MISC_STRAP_BUS_TBUS_DISABLE             (1<<4)

    uint32  miscStrapOverride;                  /* 0x18 */
    uint32  miscVregCtrl0;                      /* 0x1C */

    uint32  miscVregCtrl1;                      /* 0x20 */
#define VREG_VSEL1P2_SHIFT        0
#define VREG_VSEL1P2_MASK         0x1f
#define VREG_VSEL1P2_MIDDLE       0x0f

    uint32  miscVregCtrl2;                      /* 0x24 */
    uint32  miscFap2IrqMaskLo;                  /* 0x28 */
#define FAP_IRQ_LO_GPHY_IRQ            (1<<31)
#define FAP_IRQ_LO_WAKEUP_IRQ          (1<<30)
#define FAP_IRQ_LO_SAR_DMA_IRQ_3       (1<<29)
#define FAP_IRQ_LO_SAR_DMA_IRQ_2       (1<<28)
#define FAP_IRQ_LO_SAR_DMA_IRQ_1       (1<<27)
#define FAP_IRQ_LO_SAR_DMA_IRQ_0       (1<<26)
#define FAP_IRQ_LO_FAP1_IRQ            (1<<25)
#define FAP_IRQ_LO_FAP0_IRQ            (1<<24)
#define FAP_IRQ_LO_DSL_IRQ             (1<<23)
#define FAP_IRQ_LO_IPSEC_DMA_IRQ_0     (1<<22)
#define FAP_IRQ_LO_USBD_DMA_IRQ_4      (1<<21)
#define FAP_IRQ_LO_USBD_DMA_IRQ_2      (1<<20)
#define FAP_IRQ_LO_USBD_DMA_IRQ_0      (1<<19)
#define FAP_IRQ_LO_GPHY_ENERGY_IRQ_3   (1<<18)
#define FAP_IRQ_LO_EPHY_ENERGY_IRQ_2   (1<<17)
#define FAP_IRQ_LO_EPHY_ENERGY_IRQ_1   (1<<16)
#define FAP_IRQ_LO_EPHY_ENERGY_IRQ_0   (1<<15)
#define FAP_IRQ_LO_DYING_GASP_IRQ      (1<<14)
#define FAP_IRQ_LO_EPHY_IRQ            (1<<13)
#define FAP_IRQ_LO_PCM_IRQ             (1<<12)
#define FAP_IRQ_LO_USBD_IRQ            (1<<11)
#define FAP_IRQ_LO_USBH_EHCI_IRQ       (1<<10)
#define FAP_IRQ_LO_USBH_OHCI_IRQ       (1<<9)
#define FAP_IRQ_LO_IPSEC_IRQ           (1<<8)
#define FAP_IRQ_LO_WLAN_IRQ            (1<<7)
#define FAP_IRQ_LO_HS_SPIM_IRQ         (1<<6)
#define FAP_IRQ_LO_UART0_IRQ           (1<<5)
#define FAP_IRQ_LO_SWITCH_RX_DMA_IRQ_3 (1<<4)
#define FAP_IRQ_LO_SWITCH_RX_DMA_IRQ_2 (1<<3)
#define FAP_IRQ_LO_SWITCH_RX_DMA_IRQ_1 (1<<2)
#define FAP_IRQ_LO_SWITCH_RX_DMA_IRQ_0 (1<<1)
#define FAP_IRQ_LO_TIMER_IRQ           (1<<0)
    uint32  miscFap2IrqMaskHi;                  /* 0x2c */
#define FAP_IRQ_HI_SAR_DMA_IRQ_7       (1<<30)
#define FAP_IRQ_HI_SAR_DMA_IRQ_6       (1<<29)
#define FAP_IRQ_HI_SAR_DMA_IRQ_5       (1<<28)
#define FAP_IRQ_HI_SAR_DMA_IRQ_4       (1<<27)
#define FAP_IRQ_HI_PER_MBOX3_IRQ       (1<<26)
#define FAP_IRQ_HI_PER_MBOX2_IRQ       (1<<25)
#define FAP_IRQ_HI_PER_MBOX1_IRQ       (1<<24)
#define FAP_IRQ_HI_PER_MBOX0_IRQ       (1<<23)
#define FAP_IRQ_HI_USB_DISCON_IRQ      (1<<22)
#define FAP_IRQ_HI_USB_CCS_IRQ         (1<<21)
#define FAP_IRQ_HI_RINGOSC_IRQ         (1<<20)
#define FAP_IRQ_HI_NAND_IRQ            (1<<18)
#define FAP_IRQ_HI_SAR_IRQ             (1<<17)
#define FAP_IRQ_HI_ROBOSW_IRQ          (1<<16)
#define FAP_IRQ_HI_EXTERNAL_IRQ_3      (1<<15)
#define FAP_IRQ_HI_EXTERNAL_IRQ_2      (1<<14)
#define FAP_IRQ_HI_EXTERNAL_IRQ_1      (1<<13)
#define FAP_IRQ_HI_EXTERNAL_IRQ_0      (1<<12)
#define FAP_IRQ_HI_PCM_DMA_IRQ_1       (1<<11)
#define FAP_IRQ_HI_PCM_DMA_IRQ_0       (1<<10)
#define FAP_IRQ_HI_PCIE_EP_IRQ         (1<<9)
#define FAP_IRQ_HI_PCIE_RC_IRQ         (1<<8)
#define FAP_IRQ_HI_IPSEC_DMA_IRQ_1     (1<<7)
#define FAP_IRQ_HI_USBD_DMA_IRQ_5      (1<<6)
#define FAP_IRQ_HI_USBD_DMA_IRQ_3      (1<<5)
#define FAP_IRQ_HI_USBD_DMA_IRQ_1      (1<<4)
#define FAP_IRQ_HI_WLAN_GPIO_IRQ       (1<<3)
#define FAP_IRQ_HI_UART1_IRQ           (1<<2)
#define FAP_IRQ_HI_DECT_IRQ_1          (1<<1)
#define FAP_IRQ_HI_DECT_IRQ_0          (1<<0)
    uint32  miscFap2ExtIrqMaskLo;               /* 0x30 */
#define FAP_EXT_IRQ_LO_LS_SPIM_IRQ     (1<<16)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_19  (1<<15)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_18  (1<<14)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_17  (1<<13)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_16  (1<<12)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_15  (1<<11)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_14  (1<<10)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_13  (1<<9)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_12  (1<<8)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_11  (1<<7)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_10  (1<<6)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_9   (1<<5)
#define FAP_EXT_IRQ_LO_SAR_DMA_IRQ_8   (1<<4)
#define FAP_EXT_IRQ_LO_SWITCH_TX_DMA_IRQ_3    (1<<3)
#define FAP_EXT_IRQ_LO_SWITCH_TX_DMA_IRQ_2    (1<<2)
#define FAP_EXT_IRQ_LO_SWITCH_TX_DMA_IRQ_1    (1<<1)
#define FAP_EXT_IRQ_LO_SWITCH_TX_DMA_IRQ_0    (1<<0)
    uint32  miscFap2ExtIrqMaskHi;               /* 0x34 */
    uint32  miscFapIrqMaskLo;                   /* 0x38 */
    uint32  miscFapIrqMaskHi;                   /* 0x3c */
    uint32  miscFapExtIrqMaskLo;                /* 0x40 */
    uint32  miscFapExtIrqMaskHi;                /* 0x44 */
    uint32  miscApm_ntr_ctrl;                   /* 0x48 */
    uint32  miscIddqCtrl;                       /* 0x4c */
#define MISC_IDDQ_CTRL_PCIE_SERDES_DISABLE  (1<<24)
#define MISC_IDDQ_CTRL_EPON_SERDES          (1<<10)
#define MISC_IDDQ_CTRL_EPON                 (1<<9)
#define MISC_IDDQ_CTRL_GPHY                 (1<<8)
#define MISC_IDDQ_CTRL_FAP                  (1<<7)
#define MISC_IDDQ_CTRL_PCIE                 (1<<6)
#define MISC_IDDQ_CTRL_ROBOSW               (1<<5)
#define MISC_IDDQ_CTRL_MEMC                 (1<<4)
#define MISC_IDDQ_CTRL_PERIPH               (1<<3)
#define MISC_IDDQ_CTRL_MIPS                 (1<<2)
#define MISC_IDDQ_CTRL_APM                  (1<<1)
#define MISC_IDDQ_CTRL_USBH                 (1<<0)
    uint32  miscSleep;                          /* 0x50 */
    uint32  miscRtc_enable;                     /* 0x54 */
    uint32  miscRtc_count_L;                    /* 0x58 */
    uint32  miscRtc_count_H;                    /* 0x5c */
    uint32  miscRtc_event;                      /* 0x60 */
    uint32  miscWakeup_mask;                    /* 0x64 */
    uint32  miscWakeup_status;                  /* 0x68 */
    uint32  miscExtIRQ_Debounce_Control;        /* 0x6c */
    uint32  miscSleep_CPU_Scratch;              /* 0x70 */
    uint32  miscLed_inv;                        /* 0x74 */
    uint32  miscReserve[10];                    /* 0x78 - 0x9c */
    uint32  miscPll_ctrl;                       /* 0xa0 */
    uint32  miscMbox0_data;                     /* 0xa4 */
    uint32  miscMbox1_data;                     /* 0xa8 */
    uint32  miscMbox2_data;                     /* 0xac */
    uint32  miscMbox3_data;                     /* 0xb0 */
    uint32  miscMbox_ctrl;                      /* 0xb4 */
    uint32  miscLcpll_ctrl;                     /* 0xb8 */
    uint32  miscLcpll_cfg0;                      /* 0xbc */
    uint32  miscLcpll_cfg1;                      /* 0xc0 */
    uint32  miscLcpll_cfg2;                      /* 0xc4 */
    uint32  miscLcpll_cfg3;                      /* 0xc8 */
    uint32  miscPllLock_stat;                    /* 0xcc */
#define MISC_LCPLL_LOCK         (1<<6)
} Misc;

#define MISC ((volatile Misc * const) MISC_BASE)

/*
** LedControl Register Set Definitions.
*/

#pragma pack(push, 4)
typedef struct LedControl {
    uint32  ledInit;
#define LED_LED_TEST                (1 << 31)
#define LED_SHIFT_TEST              (1 << 30)
#define LED_SERIAL_LED_SHIFT_DIR    (1 << 16)
#define LED_SERIAL_LED_DATA_PPOL    (1 << 15)
#define LEDSERIAL_LED_CLK_NPOL      (1 << 14)
#define LED_SERIAL_LED_MUX_SEL      (1 << 13)
#define LED_SERIAL_LED_EN           (1 << 12)
#define LED_FAST_INTV_SHIFT         6
#define LED_FAST_INTV_MASK          (0x3F<<LED_FAST_INTV_SHIFT)
#define LED_SLOW_INTV_SHIFT         0
#define LED_SLOW_INTV_MASK          (0x3F<<LED_SLOW_INTV_SHIFT)
#define LED_INTERVAL_20MS           1

    uint64  ledMode;
#define LED_MODE_MASK               (uint64)0x3
#define LED_MODE_OFF                (uint64)0x0
#define LED_MODE_FLASH              (uint64)0x1
#define LED_MODE_BLINK              (uint64)0x2
#define LED_MODE_ON                 (uint64)0x3

    uint32  ledHWDis;
#define LED_GPHY0_SPD0              0
#define LED_GPHY0_SPD1              1
#define LED_EPHY0_ACT               8
#define LED_EPHY1_ACT               9
#define LED_GPHY0_ACT               10
#define LED_GPHY1_ACT               11
#define LED_EPHY0_SPD               12
#define LED_EPHY1_SPD               13
#define LED_GPHY1_SPD0              14
#define LED_GPHY1_SPD1              15
#define LED_USB_ACT                 23

    uint32  ledStrobe;
    uint32  ledLinkActSelHigh;
#define LED_4_ACT_SHIFT             0
#define LED_5_ACT_SHIFT             4
#define LED_6_ACT_SHIFT             8
#define LED_7_ACT_SHIFT             12
#define LED_4_LINK_SHIFT            16
#define LED_5_LINK_SHIFT            20
#define LED_6_LINK_SHIFT            24
#define LED_7_LINK_SHIFT            28
    uint32  ledLinkActSelLow;
#define LED_0_ACT_SHIFT             0
#define LED_1_ACT_SHIFT             4
#define LED_2_ACT_SHIFT             8
#define LED_3_ACT_SHIFT             12
#define LED_0_LINK_SHIFT            16
#define LED_1_LINK_SHIFT            20
#define LED_2_LINK_SHIFT            24
#define LED_3_LINK_SHIFT            28

    uint32  ledReadback;
    uint32  ledSerialMuxSelect;
} LedControl;
#pragma pack(pop)

#define LED ((volatile LedControl * const) LED_BASE)

#define GPIO_NUM_TO_LED_MODE_SHIFT(X) \
    ((((X) & BP_GPIO_NUM_MASK) < 8) ? (32 + (((X) & BP_GPIO_NUM_MASK) << 1)) : \
    ((((X) & BP_GPIO_NUM_MASK) - 8) << 1))

#define IUDMA_MAX_CHANNELS          32

/*
** DMA Channel Configuration (1 .. 32)
*/
typedef struct DmaChannelCfg {
  uint32        cfg;                    /* (00) assorted configuration */
#define         DMA_ENABLE      0x00000001  /* set to enable channel */
#define         DMA_PKT_HALT    0x00000002  /* idle after an EOP flag is detected */
#define         DMA_BURST_HALT  0x00000004  /* idle after finish current memory burst */
  uint32        intStat;                /* (04) interrupts control and status */
  uint32        intMask;                /* (08) interrupts mask */
#define         DMA_BUFF_DONE   0x00000001  /* buffer done */
#define         DMA_DONE        0x00000002  /* packet xfer complete */
#define         DMA_NO_DESC     0x00000004  /* no valid descriptors */
#define         DMA_RX_ERROR    0x00000008  /* rxdma detect client protocol error */
  uint32        maxBurst;               /* (0C) max burst length permitted */
#define         DMA_DESCSIZE_SEL 0x00040000  /* DMA Descriptor Size Selection */
} DmaChannelCfg;

/*
** DMA State RAM (1 .. 16)
*/
typedef struct DmaStateRam {
  uint32        baseDescPtr;            /* (00) descriptor ring start address */
  uint32        state_data;             /* (04) state/bytes done/ring offset */
  uint32        desc_len_status;        /* (08) buffer descriptor status and len */
  uint32        desc_base_bufptr;       /* (0C) buffer descrpitor current processing */
} DmaStateRam;


/*
** DMA Registers
*/
typedef struct DmaRegs {
    uint32 controller_cfg;              /* (00) controller configuration */
#define DMA_MASTER_EN           0x00000001
#define DMA_FLOWC_CH1_EN        0x00000002
#define DMA_FLOWC_CH3_EN        0x00000004

    // Flow control Ch1
    uint32 flowctl_ch1_thresh_lo;           /* 004 */
    uint32 flowctl_ch1_thresh_hi;           /* 008 */
    uint32 flowctl_ch1_alloc;               /* 00c */
#define DMA_BUF_ALLOC_FORCE     0x80000000

    // Flow control Ch3
    uint32 flowctl_ch3_thresh_lo;           /* 010 */
    uint32 flowctl_ch3_thresh_hi;           /* 014 */
    uint32 flowctl_ch3_alloc;               /* 018 */

    // Flow control Ch5
    uint32 flowctl_ch5_thresh_lo;           /* 01C */
    uint32 flowctl_ch5_thresh_hi;           /* 020 */
    uint32 flowctl_ch5_alloc;               /* 024 */

    // Flow control Ch7
    uint32 flowctl_ch7_thresh_lo;           /* 028 */
    uint32 flowctl_ch7_thresh_hi;           /* 02C */
    uint32 flowctl_ch7_alloc;               /* 030 */

    uint32 ctrl_channel_reset;              /* 034 */
    uint32 ctrl_channel_debug;              /* 038 */
    uint32 reserved1;                       /* 03C */
    uint32 ctrl_global_interrupt_status;    /* 040 */
    uint32 ctrl_global_interrupt_mask;      /* 044 */

    // Unused words
    uint8 reserved2[0x200-0x48];

    // Per channel registers/state ram
    DmaChannelCfg chcfg[IUDMA_MAX_CHANNELS];/* (200-3FF) Channel configuration */
    union {
        DmaStateRam     s[IUDMA_MAX_CHANNELS];
        uint32          u32[4 * IUDMA_MAX_CHANNELS];
    } stram;                                /* (400-5FF) state ram */
} DmaRegs;

#define SW_DMA ((volatile DmaRegs * const) SWITCH_DMA_BASE)

/*
** DMA Buffer
*/
typedef struct DmaDesc {
  union {
    struct {
        uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM    0x8000
#define          DMA_DESC_MULTICAST 0x4000
#define          DMA_DESC_BUFLENGTH 0x0fff
        uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#define          DMA_PRIO               0x0C00  /* Prio for Tx */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet
    };
    uint32      word0;
  };

  uint32        address;                /* address of data */
} DmaDesc;

/*
** 16 Byte DMA Buffer
*/
typedef struct {
  union {
    struct {
        uint16        length;                   /* in bytes of data in buffer */
#define          DMA_DESC_USEFPM        0x8000
#define          DMA_DESC_MULTICAST     0x4000
#define          DMA_DESC_BUFLENGTH     0x0fff
        uint16        status;                   /* buffer status */
#define          DMA_OWN                0x8000  /* cleared by DMA, set by SW */
#define          DMA_EOP                0x4000  /* last buffer in packet */
#define          DMA_SOP                0x2000  /* first buffer in packet */
#define          DMA_WRAP               0x1000  /* */
#define          DMA_PRIO               0x0C00  /* Prio for Tx */
#define          DMA_APPEND_BRCM_TAG    0x0200
#define          DMA_APPEND_CRC         0x0100
#define          USB_ZERO_PKT           (1<< 0) // Set to send zero length packet
    };
    uint32      word0;
  };

  uint32        address;                 /* address of data */
  uint32        control;
#define         GEM_ID_MASK             0x001F
  uint32        reserved;
} DmaDesc16;


typedef struct USBControl {
    uint32 BrtControl1;
    uint32 BrtControl2;
    uint32 BrtStatus1;
    uint32 BrtStatus2;
    uint32 UTMIControl1;
    uint32 TestPortControl;
    uint32 PllControl1;
#define PLLC_REFCLKSEL_MASK     0x00000003
#define PLLC_REFCLKSEL_SHIFT    0
#define PLLC_CLKSEL_MASK        0x0000000c
#define PLLC_CLKSEL_SHIFT       2
#define PLLC_XTAL_PWRDWNB       0x00000010
#define PLLC_PLL_PWRDWNB        0x00000020
#define PLLC_PLL_CALEN          0x00000040
#define PLLC_PHYPLL_BYP         0x00000080
#define PLLC_PLL_RESET          0x00000100
#define PLLC_PLL_IDDQ_PWRDN     0x00000200
#define PLLC_PLL_PWRDN_DELAY    0x00000400
    uint32 SwapControl;
#define USB_DEVICE_SEL          (1<<6)
#define EHCI_LOGICAL_ADDRESS_EN (1<<5)
#define EHCI_ENDIAN_SWAP        (1<<4)
#define EHCI_DATA_SWAP          (1<<3)
#define OHCI_LOGICAL_ADDRESS_EN (1<<2)
#define OHCI_ENDIAN_SWAP        (1<<1)
#define OHCI_DATA_SWAP          (1<<0)
    uint32 GenericControl;
#define GC_PLL_SUSPEND_EN       (1<<1)
    uint32 FrameAdjustValue;
    uint32 Setup;
#define USBH_IOC                (1<<4)
    uint32 MDIO;
    uint32 MDIO32;
    uint32 USBSimControl;
#define USBH_OHCI_MEM_REQ_DIS   (1<<1)
} USBControl;

#define USBH ((volatile USBControl * const) USBH_CFG_BASE)

typedef struct ApmBaseRegs {
    uint32 ApmBaseApmDevIrqStatus;                  // APM Interrupt Status & APM Diag Select Register 
    uint32 ApmBaseApmDevIrqPend;                    // APM DMA Interrupt Pending Register 
    uint32 ApmBaseApmDevIrqMask;                    // APM DMA Interrupt Mask Register 
    uint32 ApmBaseApmDevControl;                    // APM Device Control Register 
    uint32 ApmBaseRegApmCoeffWrData;                // APM Coefficient Write Data Register 
    uint32 ApmBaseRegApmCoeffConfig;                // APM Coefficient Config Register 
    uint32 ApmBaseRegChaTestConfig;                 // APM Ch_A Test Config Register 
    uint32 ApmBaseRegChbTestConfig;                 // APM Ch_B Test Config Register 
    uint32 ApmBaseRegChaEqConfig;                   // APM Ch_A Equalizer Config Register 
    uint32 ApmBaseRegChaHybalConfig;                // APM Ch_A Hybrid Balance Config Register 
    uint32 ApmBaseRegChaIntConfig1;                 // APM Ch_A Interrupt Config Register 1 
    uint32 ApmBaseRegChaIntConfig2;                 // APM Ch_A Interrupt Config Register 2 
    uint32 ApmBaseRegChaPmConfig1;                  // APM Ch_A Pulse Metering Config Register 1 
    uint32 ApmBaseRegChaPmConfig2;                  // APM Ch_A Pulse Metering Config Register 2 
    uint32 ApmBaseRegChaPmConfig3;                  // APM Ch_A Pulse Metering Config Register 3 
    uint32 ApmBaseRegChaCicConfig;                  // APM Ch_A CIC Config Register 
    uint32 ApmBaseRegChaAsrcDecConfig;              // APM Ch_A ASRC Config Register 
    uint32 ApmBaseRegChaFcwConfig1;                 // APM Ch_A FCW Config Register 1 
    uint32 ApmBaseRegChaFcwConfig2;                 // APM Ch_A FCW Config Register 2 
    uint32 ApmBaseRegChaDigmodConfig;               // APM Ch_A Digmod Config Register 
    uint32 ApmBaseRegChbEqConfig;                   // APM Ch_B Equalizer Config Register 
    uint32 ApmBaseRegChbHybalConfig;                // APM Ch_B Hybrid Balance Config Register 
    uint32 ApmBaseRegChbIntConfig1;                 // APM Ch_B Interrupt Config Register 1 
    uint32 ApmBaseRegChbIntConfig2;                 // APM Ch_B Interrupt Config Register 2 
    uint32 ApmBaseRegChbPmConfig1;                  // APM Ch_B Pulse Metering Config Register 1 
    uint32 ApmBaseRegChbPmConfig2;                  // APM Ch_B Pulse Metering Config Register 2 
    uint32 ApmBaseRegChbPmConfig3;                  // APM Ch_B Pulse Metering Config Register 4 
    uint32 ApmBaseRegChbCicConfig;                  // APM Ch_B CIC Config Register 
    uint32 ApmBaseRegChbAsrcDecConfig;              // APM Ch_B ASRC Config Register 
    uint32 ApmBaseRegChbFcwConfig1;                 // APM Ch_B FCW Config Register 1 
    uint32 ApmBaseRegChbFcwConfig2;                 // APM Ch_B FCW Config Register 2 
    uint32 ApmBaseRegChbDigmodConfig;               // APM Ch_B Digmod Config Register 
    uint32 ApmBaseRegFcwConfig1;                    // APM FCW Config Register 1 
    uint32 ApmBaseRegFcwConfig2;                    // APM FCW Config Register 2 
    uint32 ApmBaseRegNtpConfig1;                    // APM NTP Config Register 1 
    uint32 ApmBaseRegNtpConfig2;                    // APM NTP Config Register 2 
    uint32 ApmBaseRegRingConfig1;                   // APM Ring Config Register 1 
    uint32 ApmBaseRegRingConfig2;                   // APM Ring Config Register 2 
    uint32 ApmBaseRegRingConfig3;                   // APM Ring Config Register 3 
    uint32 ApmBaseRegRingConfig4;                   // APM Ring Config Register 4 
    uint32 ApmBaseRegSerTstConfig;                  // APM Serial Test Interface Config Register 
    uint32 ApmBaseRegAudioSpare;                    // APM Spare audio configuration register 
    uint32 ApmBaseRegStatsConfig;                   // APM Statistics Config Register 
    uint32 ApmBaseRegApmStatus1;                    // APM Status Register 1 
    uint32 ApmBaseRegApmStatus2;                    // APM Status Register 2 
    uint32 ApmBaseRegApmStatus3;                    // APM Status Register 3 
    uint32 ApmBaseRegApmStatus4;                    // APM Status Register 4 
    uint32 ApmBaseRegApmStatus5;                    // APM Status Register 5 
    uint32 ApmBaseRegApmStatus6;                    // APM Status Register 6 
    uint32 ApmBaseRegApmStatus7;                    // APM Status Register 7 
    uint32 ApmBaseRegApmStatus8;                    // APM Status Register 8 
    uint32 ApmBaseRegApmStatus9;                    // APM Status Register 9 
    uint32 ApmBaseRegApmStatus10;                   // APM Status Register 10 
    uint32 ApmBaseRegApmStatus11;                   // APM Status Register 11 
    uint32 ApmBaseRegApmStatus12;                   // APM Status Register 12 
    uint32 ApmBaseRegApmStatus13;                   // APM Status Register 13 
    uint32 ApmBaseRegApmStatus14;                   // APM Status Register 14 
    uint32 ApmBaseRegApmStats1;                     // APM Statistics Register 1 
    uint32 ApmBaseRegApmStats2;                     // APM Statistics Register 2 
    uint32 ApmBaseRegApmStats3;                     // APM Statistics Register 3 
    uint32 ApmBaseRegApmStats4;                     // APM Statistics Register 4 
    uint32 ApmBaseRegApmStats5;                     // APM Statistics Register 5 
    uint32 ApmBaseRegApmStats6;                     // APM Statistics Register 6 
    uint32 ApmBaseRegApmAudioFcwA;                  // APM CH_A Audio FCW Register 
    uint32 ApmBaseRegApmAudioFcwB;                  // APM CH_B Audio FCW Register 
    uint32 ApmBaseRegApmIrqPend1;                   // APM Interrupt Request Pending Register 1 for Ch A 
    uint32 ApmBaseRegApmIrqMask1;                   // APM Interrupt Mask Register 1 for Ch A 
    uint32 ApmBaseRegApmIrqPend2;                   // APM Interrupt Request Pending Register 2 for Ch B 
    uint32 ApmBaseRegApmIrqMask2;                   // APM Interrupt Mask Register 2 for Ch B 
    uint32 ApmBaseRegApmIrqPend3;                   // APM Interrupt Request Pending Register 3 
    uint32 ApmBaseRegApmIrqMask3;                   // APM Interrupt Mask Register 3 
    uint32 ApmBaseRegSpare2;                        // APM SPARE 2 
    uint32 ApmBaseRegCodecConfig0;                  // APM Codec Config Register 0 
    uint32 ApmBaseRegCodecConfig1;                  // APM Codec Config Register 1 
    uint32 ApmBaseRegCodecConfig2;                  // APM Codec Config Register 2 
    uint32 ApmBaseRegCodecConfig3;                  // APM Codec Config Register 3 
    uint32 ApmBaseRegCodecConfig4;                  // APM Codec Config Register 4 
    uint32 ApmBaseRegCodecConfig5;                  // APM Codec Config Register 5 
    uint32 ApmBaseRegCodecConfig6;                  // APM Codec Config Register 6 
    uint32 ApmBaseRegCodecConfig7;                  // APM Codec Config Register 7 
    uint32 ApmBaseRegCodecConfig8;                  // APM Codec Config Register 8 
    uint32 ApmBaseRegApmCodecFb0;                   // APM Codec Feedback Register 
    uint32 ApmBaseRegMbist0;                        // APM MBIST Register 0 
    uint32 ApmBaseRegMbist1;                        // APM MBIST Register 1 
    uint32 ApmBaseRegOtp0;                          // APM OTP Register 0 
    uint32 ApmBaseRegOtp1;                          // APM OTP Register 1 
    uint32 ApmBaseRegDiagReadback;                  // APM Diagnostic Readback Register 
    uint32 ApmBaseRegAudioSpare2;                   // APM Audio Spare Register 2 
    uint32 ApmBaseRegDpllCntrl;                     // Digital PLL control register 
    uint32 ApmBaseRegDpllNomFreq;                   // Digital PLL Nominal Frequency (control) 
    uint32 ApmBaseRegDpllDiv;                       // Digital PLL Divide Register 
    uint32 ApmBaseRegDpllAcqFreq;                   // Digital PLL Acquired Frequency 
    uint32 ApmBaseRegDpllStatus;                    // Digital PLL status register 	
} ApmBaseRegs;

#define APMBASEREG ((volatile ApmBaseRegs * const) (SWITCH_BASE + 0x200000))


// HSI Regsters

typedef struct HsiBaseRegs {
    uint32 	HsiDmaCommand;                       // Hornet SPI DMA Command Register
#define HSI_DMACMD_SPROTECT	    0x00800000       // Indicates DMA sector protect/unprotect operation	
#define HSI_DMACMD_BOOT		    0x00400000       // Indicates DMA boot operation	
#define HSI_DMACMD_MBOX		   	0x00200000       // Indicates DMA mailbox operation	
#define HSI_DMACMD_RD	    	0x00100000       // Indicates DMA read operation	
#define HSI_DMACMD_WR	    	0x00080000       // Indicates DMA write operation	
#define HSI_DMACMD_ERASE    	0x00040000       // Indicates DMA sector erase operation

#define HSI_DMACMD_DMASZ_SHIFT  0				 // Shift to get size of DMA transfer in bytes
#define HSI_DMACMD_DMASZ_MASK   0x0003FFFF		 // Mask to get size of DMA transfer in bytes
	
    uint32 	HsiDmaAddress;                       // Hornet SPI DMA Address Register
    uint32 	HsiDmaStatus;                        // Hornet SPI DMA Status Register (Do not read - write only register)
#define HSI_DMASTAT_HRESETN     0x80000000       // Hornet Core Hard Reset
#define HSI_DMASTAT_PFCHDONE    0x00000004       // Indicates MIPS is done fetching the bootcode from the external flash.
#define HSI_DMASTAT_MBOXDONE    0x00000002       // Indicates Mailbox DMA done.
#define HSI_DMASTAT_MAILBOX     0x00000001       // Indicates the DMA done for the current operation. (Poorly named - has nothing to do with mailbox operations)
	
    uint32 	HsiHornetSpiAddrOffset;              // Hornet SPI Address Offset Register
    uint32 	HsiMboxWrAddrOffset;                 // Hornet SPI Mailbox Write Address Offset Register
    uint32 	HsiMboxRdAddrOffset;                 // Hornet SPI Mailbox Read Address Offset Register
    uint32 	HsiMailboxRdDmaSize;                 // Hornet SPI Mailbox Read Size Register
    uint32 	HsiBootcodeSize;                     // Hornet SPI Flash Bootcode Size Register
    uint32 	HsiSpiStatus;                        // Hornet SPI Status Register   //0x20
#define HSI_SPISTAT_IRQRDMASK   0x80000000       // SPI DDR read interrupt mask	
#define HSI_SPISTAT_IRQWRMASK   0x40000000       // SPI DDR write interrupt mask	
#define HSI_SPISTAT_IRQSEMASK   0x20000000       // SPI DDR sector erase interrupt mask	
#define HSI_SPISTAT_IRQSPMASK   0x10000000       // SPI DDR sector protect interrupt mask	
#define HSI_SPISTAT_RDIRQ	    0x08000000       // SPI DDR read interrupt signal	
#define HSI_SPISTAT_WRIRQ	    0x04000000       // SPI DDR write interrupt signal	
#define HSI_SPISTAT_SEIRQ   	0x02000000       // SPI DDR sector erase interrupt signal	
#define HSI_SPISTAT_SPIRQ    	0x01000000       // SPI DDR sector protect interrupt signal	
	
	uint32  HsiReserved1[0x1F];                  // Locations 0x2824-0x289F reserved
    uint32  HsiDluCommand;                       // Hornet DLU/GLU Command Register //0xa0
#define HSI_DLLUCMD_BUSY        0x80000000       // LUE access is in progress
#define HSI_DLLUCMD_DLU         0x40000000       // LUE access is for DLU
#define HSI_DLLUCMD_OP_WR       0x20000000       // Write Operation
#define HSI_DLLUCMD_ENTRY       0x000000FF       // LUE access is for DLU
    uint32 	HsiDluData31_0;                      // Hornet DLU/GLU Data[31:0] Register //0xa4
    uint32 	HsiDluData63_32;                     // Hornet DLU/GLU Data[63:32] Register //0xa8
    uint32 	HsiDluData92_64;                     // Hornet DLU/GLU Data[92:64] Register //0xac
	uint32  HsiReserved2[0x12];                  // Locations 0x28B0-0x28F7 reserved
    uint32 	HsiDiagSel;                          // HSI Diag Select Register //0xf8
    uint32 	HsiDiagRead;                         // HSI Diag Read Register
} HsiBaseRegs;

#define HSIBASEREG ((volatile HsiBaseRegs * const) HSI_BASE)

typedef struct EthSwAvTableRegs {
    uint8  arlTableControl;
#define ARL_CTRL_START_DONE 0x80
#define ARL_CTRL_READ       0x01
#define ARL_CTRL_WRITE      0x00
    uint16 macAddrIndexLow;
    uint32 macAddrIndexHigh;
    uint16 vlanIdIndex;
#define ARL_VID_INDEX_MASK 0x0FFF
    uint32 reserved0;
    uint32 arlMacVidEntryLow;
    uint32 arlMacVidEntryHigh;
#define ARL_MAC_VID_ENTRY_HIGH_VID_MASK 0x0FFF0000
#define ARL_MAC_VID_ENTRY_HIGH_VID_SHFT 16
#define ARL_MAC_VID_ENTRY_HIGH_MAC_MASK 0x0000FFFF
    uint16 arlDataEntry;
#define ARL_DATA_ENTRY_VALID     0x8000
#define ARL_DATA_ENTRY_STATIC    0x4000
#define ARL_DATA_ENTRY_AGE       0x2000
#define ARL_DATA_ENTRY_PRIO_MASK 0x1C00
#define ARL_DATA_ENTRY_PRIO_SHFT 10
#define ARL_DATA_ENTRY_PORT_MASK 0x01FF
    uint32 reserved1[5];
    uint16 arlSearchControl;
#define ARL_SEARCH_CTRL_START_DONE 0x0080
#define ARL_SEARCH_CTRL_VALID      0x0001
    uint16 arlSearchAddr;
    uint32 arlSearchMacVidResultLow;
    uint32 arlSearchMacVidResultHigh;
#define ARL_SEARCH_MAC_VID_RESULT_VID_MASK 0x0FFF0000
#define ARL_SEARCH_MAC_VID_RESULT_VID_SHFT 16
#define ARL_SEARCH_MAC_VID_RESULT_MAC_MASK 0x0000FFFF
    uint16 arlSearchDataResult;
#define ARL_SEARCH_DATA_RESULT_STATIC    0x8000
#define ARL_SEARCH_DATA_RESULT_AGE       0x4000
#define ARL_SEARCH_DATA_RESULT_PRIO_MASK 0x3800
#define ARL_SEARCH_DATA_RESULT_PRIO_SHFT 11
#define ARL_SEARCH_DATA_RESULT_PORT_MASK 0x03FE
#define ARL_SEARCH_DATA_RESULT_PORT_SHFT 1
    uint32 reserved2[8];
    uint8  vlanTableControl;
    uint16 vlanTableAddrIndex;
    uint32 vlanTableEntry;
} EthSwAvTableRegs;

#define ETHSW_AVTABLE_REG ((volatile EthSwAvTableRegs * const) (SWITCH_BASE + 0x500))

typedef struct EthSwMIBRegs {
    unsigned int TxOctetsLo;
    unsigned int TxOctetsHi;
    unsigned int TxDropPkts;
    unsigned int TxQoSPkts;
    unsigned int TxBroadcastPkts;
    unsigned int TxMulticastPkts;
    unsigned int TxUnicastPkts;
    unsigned int TxCol;
    unsigned int TxSingleCol;
    unsigned int TxMultipleCol;
    unsigned int TxDeferredTx;
    unsigned int TxLateCol;
    unsigned int TxExcessiveCol;
    unsigned int TxFrameInDisc;
    unsigned int TxPausePkts;
    unsigned int TxQoSOctetsLo;
    unsigned int TxQoSOctetsHi;
    unsigned int RxOctetsLo;
    unsigned int RxOctetsHi;
    unsigned int RxUndersizePkts;
    unsigned int RxPausePkts;
    unsigned int Pkts64Octets;
    unsigned int Pkts65to127Octets;
    unsigned int Pkts128to255Octets;
    unsigned int Pkts256to511Octets;
    unsigned int Pkts512to1023Octets;
    unsigned int Pkts1024to1522Octets;
    unsigned int RxOversizePkts;
    unsigned int RxJabbers;
    unsigned int RxAlignErrs;
    unsigned int RxFCSErrs;
    unsigned int RxGoodOctetsLo;
    unsigned int RxGoodOctetsHi;
    unsigned int RxDropPkts;
    unsigned int RxUnicastPkts;
    unsigned int RxMulticastPkts;
    unsigned int RxBroadcastPkts;
    unsigned int RxSAChanges;
    unsigned int RxFragments;
    unsigned int RxExcessSizeDisc;
    unsigned int RxSymbolError;
    unsigned int RxQoSPkts;
    unsigned int RxQoSOctetsLo;
    unsigned int RxQoSOctetsHi;
    unsigned int Pkts1523to2047;
    unsigned int Pkts2048to4095;
    unsigned int Pkts4096to8191;
    unsigned int Pkts8192to9728;
} EthSwMIBRegs;

#define ETHSWMIBREG ((volatile EthSwMIBRegs * const) (SWITCH_BASE + 0x2000))

/* Enet registers controlling rx iuDMA channel */
typedef struct EthSwQosIngressPortPriRegs{
    unsigned short pri_id_map[9];
} EthSwQosIngressPortPriRegs;

#define ETHSWQOSREG ((volatile EthSwQosIngressPortPriRegs * const) (SWITCH_BASE + 0x3050))

/*
** NAND Controller Registers
*/
typedef struct NandCtrlRegs {
    uint32 NandRevision;            /* NAND Revision */
    uint32 NandCmdStart;            /* Nand Flash Command Start */
#define NCMD_MASK           0x1f000000
#define NCMD_LOW_LEVEL_OP   0x10000000
#define NCMD_PARAM_CHG_COL  0x0f000000
#define NCMD_PARAM_READ     0x0e000000
#define NCMD_BLK_LOCK_STS   0x0d000000
#define NCMD_BLK_UNLOCK     0x0c000000
#define NCMD_BLK_LOCK_DOWN  0x0b000000
#define NCMD_BLK_LOCK       0x0a000000
#define NCMD_FLASH_RESET    0x09000000
#define NCMD_BLOCK_ERASE    0x08000000
#define NCMD_DEV_ID_READ    0x07000000
#define NCMD_COPY_BACK      0x06000000
#define NCMD_PROGRAM_SPARE  0x05000000
#define NCMD_PROGRAM_PAGE   0x04000000
#define NCMD_STS_READ       0x03000000
#define NCMD_SPARE_READ     0x02000000
#define NCMD_PAGE_READ      0x01000000

    uint32 NandCmdExtAddr;          /* Nand Flash Command Extended Address */
    uint32 NandCmdAddr;             /* Nand Flash Command Address */
    uint32 NandCmdEndAddr;          /* Nand Flash Command End Address */
    uint32 NandNandBootConfig;      /* Nand Flash Boot Config */
#define NBC_CS_LOCK         0x80000000
#define NBC_AUTO_DEV_ID_CFG 0x40000000
#define NBC_WR_PROT_BLK0    0x10000000
#define NBC_EBI_CS7_USES_NAND (1<<15)
#define NBC_EBI_CS6_USES_NAND (1<<14)
#define NBC_EBI_CS5_USES_NAND (1<<13)
#define NBC_EBI_CS4_USES_NAND (1<<12)
#define NBC_EBI_CS3_USES_NAND (1<<11)
#define NBC_EBI_CS2_USES_NAND (1<<10)
#define NBC_EBI_CS1_USES_NAND (1<< 9)
#define NBC_EBI_CS0_USES_NAND (1<< 8)
#define NBC_EBC_CS7_SEL       (1<< 7)
#define NBC_EBC_CS6_SEL       (1<< 6)
#define NBC_EBC_CS5_SEL       (1<< 5)
#define NBC_EBC_CS4_SEL       (1<< 4)
#define NBC_EBC_CS3_SEL       (1<< 3)
#define NBC_EBC_CS2_SEL       (1<< 2)
#define NBC_EBC_CS1_SEL       (1<< 1)
#define NBC_EBC_CS0_SEL       (1<< 0)

    uint32 NandCsNandXor;           /* Nand Flash EBI CS Address XOR with */
                                    /*   1FC0 Control */
    uint32 NandReserved1;
    uint32 NandSpareAreaReadOfs0;   /* Nand Flash Spare Area Read Bytes 0-3 */
    uint32 NandSpareAreaReadOfs4;   /* Nand Flash Spare Area Read Bytes 4-7 */
    uint32 NandSpareAreaReadOfs8;   /* Nand Flash Spare Area Read Bytes 8-11 */
    uint32 NandSpareAreaReadOfsC;   /* Nand Flash Spare Area Read Bytes 12-15*/
    uint32 NandSpareAreaWriteOfs0;  /* Nand Flash Spare Area Write Bytes 0-3 */
    uint32 NandSpareAreaWriteOfs4;  /* Nand Flash Spare Area Write Bytes 4-7 */
    uint32 NandSpareAreaWriteOfs8;  /* Nand Flash Spare Area Write Bytes 8-11*/
    uint32 NandSpareAreaWriteOfsC;  /* Nand Flash Spare Area Write Bytes12-15*/
    uint32 NandAccControl;          /* Nand Flash Access Control */
#define NAC_RD_ECC_EN       0x80000000
#define NAC_WR_ECC_EN       0x40000000
#define NAC_RD_ECC_BLK0_EN  0x20000000
#define NAC_FAST_PGM_RDIN   0x10000000
#define NAC_RD_ERASED_ECC_EN 0x08000000
#define NAC_PARTIAL_PAGE_EN 0x04000000
#define NAC_WR_PREEMPT_EN   0x02000000
#define NAC_PAGE_HIT_EN     0x01000000
#define NAC_ECC_LVL_0_SHIFT 20
#define NAC_ECC_LVL_0_MASK  0x00f00000
#define NAC_ECC_LVL_SHIFT   16
#define NAC_ECC_LVL_MASK    0x000f0000
#define NAC_ECC_LVL_DISABLE 0
#define NAC_ECC_LVL_BCH_1   1
#define NAC_ECC_LVL_BCH_2   2
#define NAC_ECC_LVL_BCH_3   3
#define NAC_ECC_LVL_BCH_4   4
#define NAC_ECC_LVL_BCH_5   5
#define NAC_ECC_LVL_BCH_6   6
#define NAC_ECC_LVL_BCH_7   7
#define NAC_ECC_LVL_BCH_8   8
#define NAC_ECC_LVL_BCH_9   9
#define NAC_ECC_LVL_BCH_10  10
#define NAC_ECC_LVL_BCH_11  11
#define NAC_ECC_LVL_BCH_12  12
#define NAC_ECC_LVL_RESVD_1 13
#define NAC_ECC_LVL_RESVD_2 14
#define NAC_ECC_LVL_HAMMING 15
#define NAC_SPARE_SZ_0_SHIFT 8
#define NAC_SPARE_SZ_0_MASK 0x00003f00
#define NAC_SPARE_SZ_SHIFT  0
#define NAC_SPARE_SZ_MASK   0x0000003f
    uint32 NandReserved2;
    uint32 NandConfig;              /* Nand Flash Config */
#define NC_CONFIG_LOCK      0x80000000
#define NC_BLK_SIZE_MASK    0x70000000
#define NC_BLK_SIZE_2048K   0x60000000
#define NC_BLK_SIZE_1024K   0x50000000
#define NC_BLK_SIZE_512K    0x30000000
#define NC_BLK_SIZE_128K    0x10000000
#define NC_BLK_SIZE_16K     0x00000000
#define NC_BLK_SIZE_8K      0x20000000
#define NC_BLK_SIZE_256K    0x40000000
#define NC_DEV_SIZE_MASK    0x0f000000
#define NC_DEV_SIZE_SHIFT   24
#define NC_DEV_WIDTH_MASK   0x00800000
#define NC_DEV_WIDTH_16     0x00800000
#define NC_DEV_WIDTH_8      0x00000000
#define NC_PG_SIZE_MASK     0x00300000
#define NC_PG_SIZE_8K       0x00300000
#define NC_PG_SIZE_4K       0x00200000
#define NC_PG_SIZE_2K       0x00100000
#define NC_PG_SIZE_512B     0x00000000
#define NC_FUL_ADDR_MASK    0x00070000
#define NC_FUL_ADDR_SHIFT   16
#define NC_BLK_ADDR_MASK    0x00000700
#define NC_BLK_ADDR_SHIFT   8

    uint32 NandReserved3;
    uint32 NandTiming1;             /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2;             /* Nand Flash Timing Parameters 2 */
    uint32 NandSemaphore;           /* Semaphore */
    uint32 NandReserved4;
    uint32 NandFlashDeviceId;       /* Nand Flash Device ID */
    uint32 NandFlashDeviceIdExt;    /* Nand Flash Extended Device ID */
    uint32 NandBlockLockStatus;     /* Nand Flash Block Lock Status */
    uint32 NandIntfcStatus;         /* Nand Flash Interface Status */
#define NIS_CTLR_READY      0x80000000
#define NIS_FLASH_READY     0x40000000
#define NIS_CACHE_VALID     0x20000000
#define NIS_SPARE_VALID     0x10000000
#define NIS_FLASH_STS_MASK  0x000000ff
#define NIS_WRITE_PROTECT   0x00000080
#define NIS_DEV_READY       0x00000040
#define NIS_PGM_ERASE_ERROR 0x00000001

    uint32 NandEccCorrExtAddr;      /* ECC Correctable Error Extended Address*/
    uint32 NandEccCorrAddr;         /* ECC Correctable Error Address */
    uint32 NandEccUncExtAddr;       /* ECC Uncorrectable Error Extended Addr */
    uint32 NandEccUncAddr;          /* ECC Uncorrectable Error Address */
    uint32 NandReadErrorCount;      /* Read Error Count */
    uint32 NandCorrStatThreshold;   /* Correctable Error Reporting Threshold */
    uint32 NandOnfiStatus;          /* ONFI Status */
    uint32 NandOnfiDebugData;       /* ONFI Debug Data */
    uint32 NandFlashReadExtAddr;    /* Flash Read Data Extended Address */
    uint32 NandFlashReadAddr;       /* Flash Read Data Address */
    uint32 NandProgramPageExtAddr;  /* Page Program Extended Address */
    uint32 NandProgramPageAddr;     /* Page Program Address */
    uint32 NandCopyBackExtAddr;     /* Copy Back Extended Address */
    uint32 NandCopyBackAddr;        /* Copy Back Address */
    uint32 NandBlockEraseExtAddr;   /* Block Erase Extended Address */
    uint32 NandBlockEraseAddr;      /* Block Erase Address */
    uint32 NandInvReadExtAddr;      /* Flash Invalid Data Extended Address */
    uint32 NandInvReadAddr;         /* Flash Invalid Data Address */
    uint32 NandReserved5[2];
    uint32 NandBlkWrProtect;        /* Block Write Protect Enable and Size */
                                    /*   for EBI_CS0b */
    uint32 NandReserved6[3];    
    uint32 NandAccControlCs1;       /* Nand Flash Access Control */
    uint32 NandConfigCs1;           /* Nand Flash Config */
    uint32 NandTiming1Cs1;          /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs1;          /* Nand Flash Timing Parameters 2 */
    uint32 NandAccControlCs2;       /* Nand Flash Access Control */
    uint32 NandConfigCs2;           /* Nand Flash Config */
    uint32 NandTiming1Cs2;          /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs2;          /* Nand Flash Timing Parameters 2 */
    uint32 NandAccControlCs3;       /* Nand Flash Access Control */
    uint32 NandConfigCs3;           /* Nand Flash Config */
    uint32 NandTiming1Cs3;          /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs3;          /* Nand Flash Timing Parameters 2 */
    uint32 NandAccControlCs4;       /* Nand Flash Access Control */
    uint32 NandConfigCs4;           /* Nand Flash Config */
    uint32 NandTiming1Cs4;          /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs4;          /* Nand Flash Timing Parameters 2 */
    uint32 NandAccControlCs5;       /* Nand Flash Access Control */
    uint32 NandConfigCs5;           /* Nand Flash Config */
    uint32 NandTiming1Cs5;          /* Nand Flash Timing Parameters 1 */
    uint32 NandTiming2Cs5;          /* Nand Flash Timing Parameters 2 */
    uint32 NandReserved7[4];
    uint32 NandSpareAreaReadOfs10;  /* Nand Flash Spare Area Read Bytes 16-19 */
    uint32 NandSpareAreaReadOfs14;  /* Nand Flash Spare Area Read Bytes 20-23 */
    uint32 NandSpareAreaReadOfs18;  /* Nand Flash Spare Area Read Bytes 24-27 */
    uint32 NandSpareAreaReadOfs1C;  /* Nand Flash Spare Area Read Bytes 28-31 */
    uint32 NandSpareAreaWriteOfs10; /* Nand Flash Spare Area Write Bytes 16-19 */
    uint32 NandSpareAreaWriteOfs14; /* Nand Flash Spare Area Write Bytes 20-23 */
    uint32 NandSpareAreaWriteOfs18; /* Nand Flash Spare Area Write Bytes 24-27 */
    uint32 NandSpareAreaWriteOfs1C; /* Nand Flash Spare Area Write Bytes 28-31 */
    uint32 NandReserved8[10];
    uint32 NandLlOpNand;            /* Flash Low Level Operation */
    uint32 NandLlRdData;            /* Nand Flash Low Level Read Data */
} NandCtrlRegs;

#define NAND ((volatile NandCtrlRegs * const) NAND_REG_BASE)


/*
** PCI-E
*/
typedef struct PcieRegs{
  uint32 devVenID;
  uint16 command;
  uint16 status;
  uint32 revIdClassCode;
  uint32 headerTypeLatCacheLineSize;
  uint32 bar1;
  uint32 bar2;
  uint32 priSecBusNo;
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SUB_BUS_NO_MASK              0x00ff0000
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SUB_BUS_NO_SHIFT             16
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SEC_BUS_NO_MASK              0x0000ff00
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_SEC_BUS_NO_SHIFT             8
#define PCIE_CFG_TYPE1_PRI_SEC_BUS_NO_PRI_BUS_NO_MASK              0x000000ff

  uint32 ioBaseLimit;
  uint32 secStatus;
  uint32 rcMemBaseLimit;
  uint32 rcPrefBaseLimit;
  uint32 rcPrefBaseHi;
  uint32 rcPrefLimitHi;
  uint32 rcIoBaseLimit;
  uint32 capPointer;
  uint32 expRomBase;
  uint32 brdigeCtrlIntPinIntLine;
  uint32 bridgeCtrl;
  uint32 unused1[27];

  /* PcieExpressCtrlRegs */
  uint16 pciExpressCap;
  uint16 pcieCapabilitiy;
  uint32 deviceCapability;
  uint16 deviceControl;
  uint16 deviceStatus;
  uint32 linkCapability;
  uint16 linkControl;
  uint16 linkStatus;
  uint32 slotCapability;
  uint16 slotControl;
  uint16 slotStatus;
  uint16 rootControl;
  uint16 rootCap;
  uint32 rootStatus;
  uint32 deviceCapability2;
  uint16 deviceControl2;
  uint16 deviceStatus2;
  uint32 linkCapability2;
  uint16 linkControl2;
  uint16 linkStatus2;
  uint32 slotCapability2;
  uint16 slotControl2;
  uint16 slotStatus2;
  uint32 unused2[6];

  /* PcieErrorRegs */
  uint16 advErrCapId;
  uint16 advErrCapOff;
  uint32 ucErrStatus;
  uint32 ucorrErrMask;
  uint32 ucorrErrSevr;
  uint32 corrErrStatus;
  uint32 corrErrMask;
  uint32 advErrCapControl;
  uint32 headerLog1;
  uint32 headerLog2;
  uint32 headerLog3;
  uint32 headerLog4;
  uint32 rootErrorCommand;
  uint32 rootErrorStatus;
  uint32 rcCorrId;
  uint32 rcFatalNonfatalId;
  uint32 unused3[10];

  /* PcieVcRegs */
  uint16 vcCapId;
  uint16 vcCapOffset;
  uint32 prtVcCapability;
  uint32 portVcCapability2;
  uint16 portVcControl;
  uint16 portVcCtatus;
  uint32 portArbStatus;
  uint32 vcRsrcControl;
  uint32 vcRsrcStatus;
  uint32 unused4[1];

  /* PcieVendor */
  uint32 vendorCapability;
  uint32 vendorSpecificHdr;
} PcieRegs;

typedef struct PcieBlk404Regs{
  uint32 unused;          /* 0x404 */
  uint32 config2;         /* 0x408 */
#define PCIE_IP_BLK404_CONFIG_2_BAR1_SIZE_MASK         0x0000000f
#define PCIE_IP_BLK404_CONFIG_2_BAR1_DISABLE           0
  uint32 config3;         /* 0x40c */
  uint32 pmDataA;         /* 0x410 */
  uint32 pmDataB;         /* 0x414 */
} PcieBlk404Regs;

typedef struct PcieBlk428Regs{
  uint32 vpdIntf;        /* 0x428 */
  uint16 unused_g;       /* 0x42c */
  uint16 vpdAddrFlag;    /* 0x42e */
  uint32 vpdData;        /* 0x430 */
  uint32 idVal1;         /* 0x434 */
  uint32 idVal2;         /* 0x438 */
  uint32 idVal3;         /* 0x43c */
#define PCIE_IP_BLK428_ID_VAL3_REVISION_ID_MASK                    0xff000000
#define PCIE_IP_BLK428_ID_VAL3_REVISION_ID_SHIFT                   24
#define PCIE_IP_BLK428_ID_VAL3_CLASS_CODE_MASK                     0x00ffffff
#define PCIE_IP_BLK428_ID_VAL3_CLASS_CODE_SHIFT                    16
#define PCIE_IP_BLK428_ID_VAL3_SUB_CLASS_CODE_SHIFT                 8

  uint32 idVal4;
  uint32 idVal5;
  uint32 unused_h;
  uint32 idVal6;
  uint32 msiData;
  uint32 msiAddr_h;
  uint32 msiAddr_l;
  uint32 msiMask;
  uint32 msiPend;
  uint32 pmData_c;
  uint32 msixControl;
  uint32 msixTblOffBir;
  uint32 msixPbaOffBit;
  uint32 unused_k;
  uint32 pcieCapability;
  uint32 deviceCapability;
  uint32 unused_l;
  uint32 linkCapability;
  uint32 bar2Config;
  uint32 pcieDeviceCapability2;
  uint32 pcieLinkCapability2;
  uint32 pcieLinkControl;
  uint32 pcieLinkCapabilityRc;
  uint32 bar3Config;
  uint32 rootCap;
  uint32 devSerNumCapId;
  uint32 lowerSerNum;
  uint32 upperSerNum;
  uint32 advErrCap;
  uint32 pwrBdgtData0;
  uint32 pwrBdgtData1;
  uint32 pwrBdgtData2;
  uint32 pwdBdgtData3;
  uint32 pwrBdgtData4;
  uint32 pwrBdgtData5;
  uint32 pwrBdgtData6;
  uint32 pwrBdgtData7;
  uint32 pwrBdgtCapability;
  uint32 vsecHdr;
  uint32 rcUserMemLo1;
  uint32 rcUserMemHi1;
  uint32 rcUserMemLo2;
  uint32 rcUserMemHi2;
} PcieBlk428Regs;

typedef struct PcieBlk800Regs{
#define NUM_PCIE_BLK_800_CTRL_REGS  6
  uint32 tlControl[NUM_PCIE_BLK_800_CTRL_REGS];
  uint32 tlCtlStat0;
  uint32 pmStatus0;
  uint32 pmStatus1;

#define NUM_PCIE_BLK_800_TAGS       32
  uint32 tlStatus[NUM_PCIE_BLK_800_TAGS];
  uint32 tlHdrFcStatus;
  uint32 tlDataFcStatus;
  uint32 tlHdrFcconStatus;
  uint32 tlDataFcconStatus;
  uint32 tlTargetCreditStatus;
  uint32 tlCreditAllocStatus;
  uint32 tlSmlogicStatus;
} PcieBlk800Regs;


typedef struct PcieBlk1000Regs{
#define NUM_PCIE_BLK_1000_PDL_CTRL_REGS  16
  uint32 pdlControl[NUM_PCIE_BLK_1000_PDL_CTRL_REGS];
  uint32 dlattnVec;
  uint32 dlAttnMask;
  uint32 dlStatus;        /* 0x1048 */
#define PCIE_IP_BLK1000_DL_STATUS_PHYLINKUP_MASK                   0x00002000
#define PCIE_IP_BLK1000_DL_STATUS_PHYLINKUP_SHIFT                  13
  uint32 dlTxChecksum;
  uint32 dlForcedUpdateGen1;
  uint32 mdioAddr;
  uint32 mdioWrData;
  uint32 mdioRdData;
  uint32 dlRxPFcCl;
  uint32 dlRxCFcCl;
  uint32 dlRxAckNack;
  uint32 dlTxRxSeqnb;
  uint32 dlTxPFcAl;
  uint32 dlTxNpFcAl;
  uint32 regDlSpare;
  uint32 dlRegSpare;
  uint32 dlTxRxSeq;
  uint32 dlRxNpFcCl;
} PcieBlk1000Regs;

typedef struct PcieBlk1800Regs{
#define NUM_PCIE_BLK_1800_PHY_CTRL_REGS         5
  uint32 phyCtrl[NUM_PCIE_BLK_1800_PHY_CTRL_REGS];
#define REG_POWERDOWN_P1PLL_ENA                      (1<<12)
  uint32 phyErrorAttnVec;
  uint32 phyErrorAttnMask;
  uint32 phyReceivedMcpErrors;
  uint32 phyTransmittedMcpErrors;
  uint32 phyGenDebug;
  uint32 phyRecoveryHist;
#define NUM_PCIE_BLK_1800_PHY_LTSSM_HIST_REGS   3
  uint32 phyltssmHist[NUM_PCIE_BLK_1800_PHY_LTSSM_HIST_REGS];
#define NUM_PCIE_BLK_1800_PHY_DBG_REGS          11
  uint32 phyDbg[NUM_PCIE_BLK_1800_PHY_DBG_REGS];
} PcieBlk1800Regs;

typedef struct PcieBridgeRegs{
   uint32 bar1Remap;       /* 0x0818*/
#define PCIE_BRIDGE_BAR1_REMAP_addr_MASK                    0xffff0000
#define PCIE_BRIDGE_BAR1_REMAP_addr_MASK_SHIFT              16
#define PCIE_BRIDGE_BAR1_REMAP_remap_enable                 (1<<1)
#define PCIE_BRIDGE_BAR1_REMAP_swap_enable                  1

   uint32 bar2Remap;       /* 0x081c*/
#define PCIE_BRIDGE_BAR2_REMAP_addr_MASK                    0xffff0000
#define PCIE_BRIDGE_BAR2_REMAP_addr_MASK_SHIFT              16
#define PCIE_BRIDGE_BAR2_REMAP_remap_enable                 (1<<1)
#define PCIE_BRIDGE_BAR2_REMAP_swap_enable                  1

   uint32 bridgeOptReg1;   /* 0x0820*/
#define PCIE_BRIDGE_OPT_REG1_en_l1_int_status_mask_polarity  (1<<12)
#define PCIE_BRIDGE_OPT_REG1_en_pcie_bridge_hole_detection   (1<<11)
#define PCIE_BRIDGE_OPT_REG1_en_rd_reply_be_fix              (1<<9)
#define PCIE_BRIDGE_OPT_REG1_enable_rd_be_opt                (1<<7)

   uint32 bridgeOptReg2;    /* 0x0824*/
#define PCIE_BRIDGE_OPT_REG2_cfg_type1_func_no_MASK    0xe0000000
#define PCIE_BRIDGE_OPT_REG2_cfg_type1_func_no_SHIFT   29
#define PCIE_BRIDGE_OPT_REG2_cfg_type1_dev_no_MASK     0x1f000000
#define PCIE_BRIDGE_OPT_REG2_cfg_type1_dev_no_SHIFT    24
#define PCIE_BRIDGE_OPT_REG2_cfg_type1_bus_no_MASK     0x00ff0000
#define PCIE_BRIDGE_OPT_REG2_cfg_type1_bus_no_SHIFT    16
#define PCIE_BRIDGE_OPT_REG2_cfg_type1_bd_sel_MASK     0x00000080
#define PCIE_BRIDGE_OPT_REG2_cfg_type1_bd_sel_SHIFT    7
#define PCIE_BRIDGE_OPT_REG2_dis_pcie_timeout_MASK     0x00000040
#define PCIE_BRIDGE_OPT_REG2_dis_pcie_timeout_SHIFT    6
#define PCIE_BRIDGE_OPT_REG2_dis_pcie_abort_MASK       0x00000020
#define PCIE_BRIDGE_OPT_REG2_dis_pcie_abort_SHIFT      5
#define PCIE_BRIDGE_OPT_REG2_enable_tx_crd_chk_MASK    0x00000010
#define PCIE_BRIDGE_OPT_REG2_enable_tx_crd_chk_SHIFT   4
#define PCIE_BRIDGE_OPT_REG2_dis_ubus_ur_decode_MASK   0x00000004
#define PCIE_BRIDGE_OPT_REG2_dis_ubus_ur_decode_SHIFT  2
#define PCIE_BRIDGE_OPT_REG2_cfg_reserved_MASK         0x0000ff0b

   uint32 Ubus2PcieBar0BaseMask; /* 0x0828 */
#define PCIE_BRIDGE_BAR0_BASE_base_MASK                     0xfff00000
#define PCIE_BRIDGE_BAR0_BASE_base_MASK_SHIFT               20
#define PCIE_BRIDGE_BAR0_BASE_mask_MASK                     0x0000fff0
#define PCIE_BRIDGE_BAR0_BASE_mask_MASK_SHIFT               4
#define PCIE_BRIDGE_BAR0_BASE_swap_enable                   (1<<1)
#define PCIE_BRIDGE_BAR0_BASE_remap_enable                  1

   uint32 Ubus2PcieBar0RemapAdd; /* 0x082c */
#define PCIE_BRIDGE_BAR0_REMAP_ADDR_addr_MASK               0xfff00000
#define PCIE_BRIDGE_BAR0_REMAP_ADDR_addr_SHIFT              20

   uint32 Ubus2PcieBar1BaseMask; /* 0x0830 */
#define PCIE_BRIDGE_BAR1_BASE_base_MASK                     0xfff00000
#define PCIE_BRIDGE_BAR1_BASE_base_MASK_SHIFT               20
#define PCIE_BRIDGE_BAR1_BASE_mask_MASK                     0x0000fff0
#define PCIE_BRIDGE_BAR1_BASE_mask_MASK_SHIFT               4
#define PCIE_BRIDGE_BAR1_BASE_swap_enable                   (1<<1)
#define PCIE_BRIDGE_BAR1_BASE_remap_enable                  1

   uint32 Ubus2PcieBar1RemapAdd; /* 0x0834 */
#define PCIE_BRIDGE_BAR1_REMAP_ADDR_addr_MASK               0xfff00000
#define PCIE_BRIDGE_BAR1_REMAP_ADDR_addr_SHIFT              20

   uint32 bridgeErrStatus;       /* 0x0838 */
   uint32 bridgeErrMask;         /* 0x083c */
   uint32 coreErrStatus2;        /* 0x0840 */
   uint32 coreErrMask2;          /* 0x0844 */
   uint32 coreErrStatus1;        /* 0x0848 */
   uint32 coreErrMask1;          /* 0x084c */
   uint32 rcInterruptStatus;     /* 0x0850 */
   uint32 rcInterruptMask;       /* 0x0854 */
#define PCIE_BRIDGE_INTERRUPT_MASK_int_a_MASK   (1<<0)
#define PCIE_BRIDGE_INTERRUPT_MASK_int_b_MASK   (1<<1)
#define PCIE_BRIDGE_INTERRUPT_MASK_int_c_MASK   (1<<2)
#define PCIE_BRIDGE_INTERRUPT_MASK_int_d_MASK   (1<<3)

   uint32 ubusToParam;           /* 0x0858 */
   uint32 pcieToParam;           /* 0x085c */
   uint32 toTickParam;           /* 0x0860 */
   uint32 epL1IntMask;           /* 0x0864 */
   uint32 epL1IntStatus;         /* 0x0868 */
   uint32 unused[4];
   uint32 epL2IntMask;           /* 0x087c */
   uint32 epL2IntSetStatus;      /* 0x0880 */
   uint32 epL2IntStatus;         /* 0x0884 */
   uint32 epCoreLinkRstStatus;   /* 0x0888 */
   uint32 epCoreLinkRstMask;     /* 0x088c */
   uint32 pcieControl;           /* 0x0890 */
#define PCIE_BRIDGE_CLKREQ_ENABLE               (1<<2)

   uint32 pcieStatus;            /* 0x0894 */

} PcieBridgeRegs;

#define PCIEH_DEV_OFFSET              0x8000
#define PCIEH                         ((volatile uint32 * const) PCIE_BASE)
#define PCIEH_REGS                    ((volatile PcieRegs * const) PCIE_BASE)

#define PCIEH_BLK_404_REGS            ((volatile PcieBlk404Regs * const) \
                                        (PCIE_BASE+0x404))
#define PCIEH_BLK_428_REGS            ((volatile PcieBlk428Regs * const) \
                                        (PCIE_BASE+0x428))
#define PCIEH_BLK_800_REGS            ((volatile PcieBlk800Regs * const) \
                                        (PCIE_BASE+0x800))
#define PCIEH_BLK_1000_REGS           ((volatile PcieBlk1000Regs * const) \
                                        (PCIE_BASE+0x1000))
#define PCIEH_BLK_1800_REGS           ((volatile PcieBlk1800Regs * const) \
                                        (PCIE_BASE+0x1800))
#define PCIEH_BRIDGE_REGS             ((volatile PcieBridgeRegs * const)  \
                                        (PCIE_BASE+0x2818))
										
#define PCIEH_MEM1_BASE               0x11000000
#define PCIEH_MEM1_SIZE               0x00f00000

#define PCIEH_MEM2_BASE               0xa0000000
#define PCIEH_MEM2_SIZE               0x01000000
#define PCIEH_PCIE_IS_DEFAULT_TARGET            /* define this only if pcie is the default ubus target and size can be extended*/

typedef struct WlanShimRegs {
    uint32 ShimMisc;                            /* SHIM control registers */
#define WLAN_SHIM_FORCE_CLOCKS_ON   (1 << 2)
#define WLAN_SHIM_MACRO_DISABLE     (1 << 1)
#define WLAN_SHIM_MACRO_SOFT_RESET  (1 << 0)

    uint32 ShimStatus;                          /* SHIM status */

    uint32 CcControl;                           /* CC control */
#define SICF_WOC_CORE_RESET         0x10000
#define SICF_BIST_EN                0x8000
#define SICF_PME_EN                 0x4000
#define SICF_CORE_BITS              0x3ffc
#define SICF_FGC                    0x0002
#define SICF_CLOCK_EN               0x0001

    uint32 CcStatus;                            /* CC status */
#define SISF_BIST_DONE              0x8000
#define SISF_BIST_ERROR             0x4000
#define SISF_GATED_CLK              0x2000
#define SISF_DMA64                  0x1000
#define SISF_CORE_BITS              0x0fff

    uint32 MacControl;                          /* MAC control */
    uint32 MacStatus;                           /* MAC status */

    uint32 CcIdA;                               /* CC desc A */
    uint32 CcIdB;                               /* CC desc B */
    uint32 CcAddr;                              /* CC base addr */
    uint32 MacIdA;                              /* MAC desc A */
    uint32 MacIdB;                              /* MAC desc B */
    uint32 MacAddr;                             /* MAC base addr */
    uint32 ShimIdA;                             /* SHIM desc A */
    uint32 ShimIdB;                             /* SHIM desc B */
    uint32 ShimAddr;                            /* SHIM addr */
    uint32 ShimEot;                             /* EOT */
}WlanShimRegs;

#define WLAN_SHIM ((volatile WlanShimRegs * const)WLAN_SHIM_BASE)

/*
** FAP Control Registers
*/
typedef struct CoprocCtlRegs_S
{
  uint32    irq_4ke_mask;    /* 00 */
  uint32    irq_4ke_status;  /* 04 */
            #define IRQ_FAP_4KE_TIMER                           (1 << 0)
            #define IRQ_FAP_4KE_OUT_FIFO                        (1 << 1)
            #define IRQ_FAP_4KE_IN_FIFO                         (1 << 2)
            #define IRQ_FAP_4KE_DQM                             (1 << 3)
            #define IRQ_FAP_4KE_MBOX_IN                         (1 << 4)
            #define IRQ_FAP_4KE_MBOX_OUT                        (1 << 5)
            #define IRQ_FAP_4KE_GENERAL_PURPOSE_INPUT           (1 << 6)
            #define IRQ_FAP_4KE_ERROR_EB2UBUS_TIMEOUT           (1 << 7)
            #define IRQ_FAP_4KE_ERROR_UB_SLAVE_TIMEOUT          (1 << 8)
            #define IRQ_FAP_4KE_ERROR_UB_SLAVE                  (1 << 9)
            #define IRQ_FAP_4KE_ERROR_UB_MASTER                 (1 << 10)
            #define IRQ_FAP_4KE_ERROR_EB_DQM_OVERFLOW           (1 << 11)
            #define IRQ_FAP_4KE_ERROR_UB_DQM_OVERFLOW           (1 << 12)
            #define IRQ_FAP_4KE_ERROR_DMA0_RX_FIFO_NOT_EMPTY    (1 << 13)
            #define IRQ_FAP_4KE_ERROR_DMA0_TX_FIFO_NOT_EMPTY    (1 << 14)
            #define IRQ_FAP_4KE_ERROR_DMA1_RX_FIFO_NOT_EMPTY    (1 << 15)
            #define IRQ_FAP_4KE_ERROR_DMA1_TX_FIFO_NOT_EMPTY    (1 << 16)
            #define IRQ_FAP_4KE_TIMER_0                         (1 << 17)
            #define IRQ_FAP_4KE_TIMER_1                         (1 << 18)
            #define IRQ_FAP_4KE_TIMER_2                         (1 << 19)
            #define IRQ_FAP_4KE_SMISB_ERROR                     (1 << 20)
            #define IRQ_FAP_4KE_DMA0_RESULT_FIFO_NOT_EMPTY      (1 << 23)
            #define IRQ_FAP_4KE_DMA1_RESULT_FIFO_NOT_EMPTY      (1 << 24)

  uint32    irq_mips_mask;   /* 08 */
  uint32    irq_mips_status; /* 0C */
            #define IRQ_FAP_HOST_TIMER                           (1 << 0)
            #define IRQ_FAP_HOST_DQM                             (1 << 3)
            #define IRQ_FAP_HOST_MBOX_IN                         (1 << 4)
            #define IRQ_FAP_HOST_MBOX_OUT                        (1 << 5)
            #define IRQ_FAP_HOST_GENERAL_PURPOSE_INPUT           (1 << 6)
            #define IRQ_FAP_HOST_ERROR_EB2UBUS_TIMEOUT           (1 << 7)
            #define IRQ_FAP_HOST_ERROR_UB_SLAVE_TIMEOUT          (1 << 8)
            #define IRQ_FAP_HOST_ERROR_UB_SLAVE                  (1 << 9)
            #define IRQ_FAP_HOST_ERROR_UB_MASTER                 (1 << 10)
            #define IRQ_FAP_HOST_ERROR_EB_DQM_OVERFLOW           (1 << 11)
            #define IRQ_FAP_HOST_ERROR_UB_DQM_OVERFLOW           (1 << 12)
            #define IRQ_FAP_HOST_ERROR_DMA0_RX_FIFO_NOT_EMPTY    (1 << 13)
            #define IRQ_FAP_HOST_ERROR_DMA0_TX_FIFO_NOT_EMPTY    (1 << 14)
            #define IRQ_FAP_HOST_ERROR_DMA1_RX_FIFO_NOT_EMPTY    (1 << 15)
            #define IRQ_FAP_HOST_ERROR_DMA1_TX_FIFO_NOT_EMPTY    (1 << 16)
            #define IRQ_FAP_HOST_TIMER_0                         (1 << 17)
            #define IRQ_FAP_HOST_TIMER_1                         (1 << 18)
            #define IRQ_FAP_HOST_TIMER_2                         (1 << 19)
            #define IRQ_FAP_HOST_SMISB_ERROR                     (1 << 20)
            #define IRQ_FAP_HOST_DMA0_RESULT_FIFO_NOT_EMPTY      (1 << 23)
            #define IRQ_FAP_HOST_DMA1_RESULT_FIFO_NOT_EMPTY      (1 << 24)


  uint32    gp_mask;         /* 10 */
  uint32    gp_status;       /* 14 */
            #define IRQ_FAP_GP_TIMER_0                         (1 << 0)
            #define IRQ_FAP_GP_TIMER_1                         (1 << 1)
            #define IRQ_FAP_GP_MBOX_IN                         (1 << 2)
            #define IRQ_FAP_GP_MBOX_OUT                        (1 << 3)
            #define IRQ_FAP_GP_ERROR_EB2UBUS_TIMEOUT           (1 << 4)
            #define IRQ_FAP_GP_ERROR_UB_SLAVE_TIMEOUT          (1 << 5)
            #define IRQ_FAP_GP_ERROR_UB_SLAVE                  (1 << 6)
            #define IRQ_FAP_GP_ERROR_UB_MASTER                 (1 << 7)
            #define IRQ_FAP_GP_ERROR_EB_DQM_OVERFLOW           (1 << 8)
            #define IRQ_FAP_GP_ERROR_UB_DQM_OVERFLOW           (1 << 9)
            #define IRQ_FAP_GP_ERROR_DMA0_RX_FIFO_NOT_EMPTY    (1 << 10)
            #define IRQ_FAP_GP_ERROR_DMA0_TX_FIFO_NOT_EMPTY    (1 << 11)
            #define IRQ_FAP_GP_ERROR_DMA1_RX_FIFO_NOT_EMPTY    (1 << 12)
            #define IRQ_FAP_GP_ERROR_DMA1_TX_FIFO_NOT_EMPTY    (1 << 13)
            #define IRQ_FAP_GP_TIMER_2                         (1 << 14)
            #define IRQ_FAP_GP_SMISB_ERROR                     (1 << 15)
            #define IRQ_FAP_GP_DMA0_RESULT_FIFO_NOT_EMPTY      (1 << 18)
            #define IRQ_FAP_GP_DMA1_RESULT_FIFO_NOT_EMPTY      (1 << 19)

  uint32    gp_tmr0_ctl;     /* 18 */
            #define   TIMER_ENABLE                     0x80000000
            #define   TIMER_MODE_REPEAT                0x40000000
            #define   TIMER_COUNT_MASK                 0x3fffffff
  uint32    gp_tmr0_cnt;     /* 1C */
  uint32    gp_tmr1_ctl;     /* 20 */
  uint32    gp_tmr1_cnt;     /* 24 */
  uint32    host_mbox_in;    /* 28 */
  uint32    host_mbox_out;   /* 2C */
  uint32    gp_out;          /* 30 */
  uint32    gp_in;           /* 34 */
            #define GP_IN_TAM_IRQ_MASK                 0x80000000
            #define GP_IN_SEGDMA_IRQ_MASK              0x00000002
            #define GP_IN_USPP_BUSY_FLAG               0x00000001
  uint32    gp_in_irq_mask;  /* 38 */
            #define GP_IN_BASE4_IRQ_MASK               0x80000000
            #define GP_IN_BASE4_IRQ_SHIFT              31
  uint32    gp_in_irq_status;/* 3C */
            #define GP_IN_IRQ_STATUS_MASK              0x0000FFFF
            #define GP_IN_IRQ_STATUS_SHIFT             0
  uint32    dma_control;     /* 40 */
            #define DMA_NO_WRITE_WITH_ACK              (1<<31)
  uint32    dma_status;      /* 44 */
            #define DMA_STS_DMA1_RSLT_FULL_BIT         (1<<7)
            #define DMA_STS_DMA1_RSLT_EMPTY_BIT        (1<<6)
            #define DMA_STS_DMA1_CMD_FULL_BIT          (1<<5)
            #define DMA_STS_DMA1_BUSY                  (1<<4)
            #define DMA_STS_DMA1_SHIFT                 4
            #define DMA_STS_DMA0_RSLT_FULL_BIT         (1<<3)
            #define DMA_STS_DMA0_RSLT_EMPTY_BIT        (1<<2)
            #define DMA_STS_DMA0_CMD_FULL_BIT          (1<<1)
            #define DMA_STS_DMA0_BUSY                  (1<<0)
  uint32    dma0_3_fifo_status; /* 48 */
            #define DMA_FIFO_STS_DMA1_RSLT_DEPTH_MSK        0x0000F000
            #define DMA_FIFO_STS_DMA1_RSLT_DEPTH_SHIFT      12
            #define DMA_FIFO_STS_DMA1_CMD_ROOM_MSK          0x00000F00
            #define DMA_FIFO_STS_DMA1_CMD_ROOM_SHIFT        8
            #define DMA_FIFO_STS_DMA0_RSLT_FIFO_DEPTH_MSK   0x000000F0
            #define DMA_FIFO_STS_DMA0_RSLT_DEPTH_SHIFT      4
            #define DMA_FIFO_STS_DMA0_CMD_ROOM_MSK          0x0000000F
            #define DMA_FIFO_STS_DMA0_CMD_ROOM_SHIFT        0
  uint32    unused1; /* 4c   //uint32    dma4_7_fifo_status; // 4C */
  uint32    unused2; /* 50   //uint32    dma_irq_sts;        // 50 */
  uint32    unused3; /* 54   //uint32    dma_4ke_irq_mask;   // 54 */
  uint32    unused4; /* 58   //uint32    dma_host_irq_mask;  // 58 */
  uint32    diag_cntrl;         /* 5C */
            #define DIAG_SEL_HI_HI_MSK          0xFF000000
            #define DIAG_SEL_HI_LO_MSK          0x00FF0000
            #define DIAG_SEL_LO_HI_MSK          0x0000FF00
            #define DIAG_SEL_LO_LO_MSK          0x000000FF
  uint32    diag_hi;            /* 60 */
            #define DIAG_HI_HI_MSK              0xFFFF0000
            #define DIAG_HI_LO_MSK              0x0000FFFF
  uint32    diag_lo;            /* 64 */
            #define DIAG_LO_HI_MSK              0xFFFF0000
            #define DIAG_LO_LO_MSK              0x0000FFFF
  uint32    bad_address;        /* 68 */
  uint32    addr1_mask;         /* 6C */
  uint32    addr1_base_in;      /* 70 */
  uint32    addr1_base_out;     /* 74 */
  uint32    addr2_mask;         /* 78 */
  uint32    addr2_base_in;      /* 7C */
  uint32    addr2_base_out;     /* 80 */
  uint32    scratch;            /* 84 */
  uint32    tm_ctl;             /* 88 */
            #define TM_CTL_BASE_4_MASK          0xFFFF0000
            #define TM_CTL_ICDATA_MASK          0x0000F000
            #define TM_CTL_ICTAG_MASK           0x00000F00
            #define TM_CTL_ICWS_MASK            0x000000F0
            #define TM_CTL_DSPRAM_MASK          0x0000000F
  uint32    soft_resets;        /* 8C active high */
            #define SOFT_RESET_DMA                    0x00000004
            #define SOFT_RESET_BASE4                  0x00000002
            #define SOFT_RESET_4KE                    0x00000001
  uint32    eb2ubus_timeout;    /* 90 */
            #define EB2UBUS_TIMEOUT_EN                0x80000000
            #define EB2UBUS_TIMEOUT_MASK              0x0000FFFF
            #define EB2UBUS_TIMEOUT_SHIFT             0
  uint32    m4ke_core_status;   /* 94 */
            #define M4KE_CORE_REV_ID_EXISTS           (1<<31)
            #define M4KE_CORE_EJ_DEBUGM               (1<<16)
            #define M4KE_CORE_EJ_SRSTE                (1<<15)
            #define M4KE_CORE_EJ_PRRST                (1<<14)
            #define M4KE_CORE_EJ_PERRST               (1<<13)
            #define M4KE_CORE_EJ_SLEEP                (1<<12)
            #define M4KE_CORE_EJ_SI_SWINT_MASK        0x00000C00
            #define M4KE_CORE_EJ_SI_SWINT_SHIFT       10
            #define M4KE_CORE_EJ_SI_IPL_MASK          0x000003F0
            #define M4KE_CORE_EJ_SI_IPL_SHIFT         4
            #define M4KE_CORE_EJ_SI_IACK              (1<<3)
            #define M4KE_CORE_EJ_SI_RP                (1<<2)
            #define M4KE_CORE_EJ_SI_EXL               (1<<1)
            #define M4KE_CORE_EJ_SI_ERL               (1<<0)
  uint32    gp_in_irq_sense;    /* 98 */
            #define GP_IN_SENSE_BASE4                 (1<<31)
            #define GP_IN_SENSE_MASK                  0x7FFFFFFF
            #define GP_IN_SENSE_SHIFT                 0
  uint32    ub_slave_timeout;   /* 9C */
            #define UB_SLAVE_TIMEOUT_EN               0x80000000
            #define UB_SLAVE_TIMEOUT_MASK             0x0000FFFF
            #define UB_SLAVE_TIMEOUT_SHIFT            0
  uint32    diag_en;            /* A0 */
            #define DIAG_EN_DIAG_SEL_OVERRIDE         (1<<31)
            #define DIAG_EN_EJTAG_DINT_MASK           (1<<30)
            #define DIAG_EN_SEL_HI_HI_MASK            0x000000C0
            #define DIAG_EN_SEL_HI_HI_SHIFT           6
            #define DIAG_EN_SEL_HI_LO_MASK            0x00000030
            #define DIAG_EN_SEL_HI_LO_SHIFT           4
            #define DIAG_EN_SEL_LO_HI_MASK            0x0000000C
            #define DIAG_EN_SEL_LO_HI_SHIFT           2
            #define DIAG_EN_SEL_LO_LO_MASK            0x00000003
            #define DIAG_EN_SEL_LO_LO_SHIFT           0
  uint32    dev_timeout;        /* A4 */
  uint32    ubus_error_out_mask;/* A8 */
            #define UBUS_ERROR_BASE4                  (1<<15)
            #define UBUS_ERROR_IN_MSG_FIFO            (1<<3)
            #define UBUS_ERROR_UBSLAVE_TIMEOUT        (1<<2)
            #define UBUS_ERROR_EB2UB_TIMEOUT          (1<<1)
            #define UBUS_ERROR_UB_DQM_OVERFLOW        (1<<0)
  uint32    diag_capt_stop_mask;/* AC */
            #define DIAG_ERROR_BASE4                  (1<<31)
            #define DIAG_ERROR_IN_MSG_FIFO            (1<<3)
            #define DIAG_ERROR_UBSLAVE_TIMEOUT        (1<<2)
            #define DIAG_ERROR_EB2UB_TIMEOUT          (1<<1)
            #define DIAG_ERROR_UB_DQM_OVERFLOW        (1<<0)
  uint32    rev_id;             /* B0 */
  uint32    gp_tmr2_ctl;        /* B4 */
            #define GP_TMR2_ENABLE                      (1<<31)
            #define GP_TMR2_MODE                        (1<<30)
            #define GP_TMR2_COUNT_MASK                  (0x3FFFFFFF)
            #define GP_TMR2_COUNT_SHIFT                 0
  uint32    gp_tmr2_cnt;        /* B8 */
            #define GP_TMR2_CNT_MASK                    (0x3FFFFFFF)
            #define GP_TMR2_CNT_SHIFT                   0
  uint32    legacy_mode;        /* BC */
  uint32    smisb_mon;          /* C0 */
            #define SMISB_MON_MAX_CLEAR                 (1<<31)
            #define SMISB_MON_MAX_TIME_MASK             0x007FFF00
            #define SMISB_MON_MAX_TIME_SHIFT            8
            #define SMISB_MON_TIME_OUT_MASK             0x007FFF00
            #define SMISB_MON_TIME_OUT_SHIFT            8
  uint32    diag_ctl;           /* C4 */
            #define DIAG_CTL_USE_INT_DIAG_HIGH          (1<<31)
            #define DIAG_CTL_HIGH_MODE_MASK             0x70000000
            #define DIAG_CTL_HIGH_MODE_SHIFT            28
            #define DIAG_CTL_HIGH_SEL_MASK              0x00FF0000
            #define DIAG_CTL_HIGH_SEL_SHIFT             16
            #define DIAG_CTL_USE_INT_LOW                (1<<15)
            #define DIAG_CTL_LOW_MODE_MASK              0x00007000
            #define DIAG_CTL_LOW_MODE_SHIFT             12
            #define DIAG_CTL_LOW_SEL_MASK               0x000000FF
            #define DIAG_CTL_LOW_SEL_SHIFT              0
  uint32    diag_stat;         /* C8 */
            #define DIAG_STAT_HI_VALID                  (1<<16)
            #define DIAG_STAT_LO_VALID                  (1<<0)
  uint32    diag_mask;         /* CC */
            #define DIAG_HI_MASK                        0xFFFF0000
            #define DIAG_HI_SHIFT                       16
            #define DIAG_LO_MASK                        0x0000FFFF
            #define DIAG_LO_SHIFT                       0
  uint32    diag_result;        /* D0 */
  uint32    diag_compare;       /* D4 */
  uint32    diag_capture;       /* D8 */
  uint32    daig_count;         /* DC */
  uint32    diag_edge_cnt;      /* E0 */
  uint32    addr3_mask;         /* E4 */
  uint32    addr3_base_in;      /* E8 */
  uint32    addr3_base_out;     /* EC */  
  uint32    unused5;            /* F0 */
  uint32    unused6;            /* F4 */
  uint32    lfsr;               /* F8 */
            #define LFSR_MASK                         0x0000FFFF
            #define LFSR_SHIFT                        0

} CoprocCtlRegs_S;

/*
** FAP Hardware counter registers
*/
typedef struct HwCntrs_S {
    uint32    hw_counter_0;       /* 00 */
    uint32    hw_counter_1;       /* 04 */
    uint32    hw_counter_2;       /* 08 */
    uint32    hw_counter_3;       /* 0C */
} HwCntrs_S;

/*
** FAP Outgoing Msg Fifo
*/
typedef struct OGMsgFifoRegs_S
{
  uint32    og_msg_ctl;    /* 00 */
            #define OG_MSG_LOW_WM_MSK                       (1<<13)
            #define OG_MSG_LOW_WM_WORDS_MASK                0x0000000F
            #define OG_MSG_LOW_WM_WORDS_SHIFT               0
  uint32    og_msg_sts;    /* 04 */
            #define OG_MSG_STS_AVAIL_FIFO_SPC_MASK          0x0000000F
            #define OG_MSG_STS_AVAIL_FIFO_SPC_SHIFT         0
  uint32    resv[14];
  uint32    og_msg_data;   /* 40 */
  uint32    resv2[14];
  uint32    og_msg_data_cont;   /* 7C */
} OGMsgFifoRegs_S;


/*
** FAP Incoming Msg Fifo
*/
typedef struct INMsgFifoRegs_S     /* 200 */
{
  uint32    in_msg_ctl;
            #define   NOT_EMPTY_IRQ_STS_MASK          0x00008000
            #define   NOT_EMPTY_IRQ_STS_OFFSET        15
            #define   ERR_IRQ_STS_MASK                0x00004000
            #define   ERR_IRQ_STS_OFFSET              14
            #define   LOW_WTRMRK_IRQ_STS_MASK         0x00002000
            #define   LOW_WTRMRK_IRQ_MSK_OFFSET       13
            #define   MSG_RCVD_IRQ_STS_MASK           0x00001000
            #define   MSG_RCVD_IRQ_MSK_OFFSET         12
            #define   LOW_WATER_MARK_MASK             0x0000003F
            #define   LOW_WATER_MARK_SHIFT            0
            #define   AVAIL_FIFO_SPACE_MASK           0x0000003F
            #define   AVAIL_FIFO_SPACE_OFFSET         0
  uint32    in_msg_sts;
            #define INMSG_NOT_EMPTY_STS_BIT           0x80000000
            #define INMSG_NOT_EMPTY_STS_SHIFT         31
            #define INMSG_ERR_STS_BIT                 0x40000000
            #define INMSG_ERR_STS_SHIFT               30
            #define INMSG_LOW_WATER_STS_BIT           0x20000000
            #define INMSG_LOW_WATER_STS_SHIFT         29
            #define INMSG_MSG_RX_STS_BIT              0x10000000
            #define INMSG_MSG_RX_STS_SHIFT            28
            #define INMSG_RESERVED1_MASK              0x0fc00000
            #define INMSG_RESERVED1_SHIFT             22
            #define INMSG_NUM_MSGS_MASK               0x003F0000
            #define INMSG_NUM_MSGS_SHIFT              16
            #define INMSG_NOT_EMPTY_IRQ_STS_BIT       0x00008000
            #define INMSG_NOT_EMPTY_IRQ_STS_SHIFT     15
            #define INMSG_ERR_IRQ_STS_BIT             0x00004000
            #define INMSG_ERR_IRQ_STS_SHIFT           14
            #define INMSG_LOW_WATER_IRQ_STS_BIT       0x00002000
            #define INMSG_LOW_WATER_IRQ_STS_SHIFT     13
            #define INMSG_MSG_RX_IRQ_STS_BIT          0x00001000
            #define INMSG_MSG_RX_IRQ_STS_SHIFT        12
            #define INMSG_RESERVED2_MASK              0x00000fc0
            #define INMSG_RESERVED2_SHIFT             6
            #define INMSG_AVAIL_FIFO_SPACE_MASK       0x0000003f
            #define INMSG_AVAIL_FIFO_SPACE_SHIFT      0
  uint32    resv[13];
  uint32    in_msg_last;
  uint32    in_msg_data;
} INMsgFifoRegs_S;


/*
** FAP DMA Registers
*/
typedef struct mDma_regs_S
{
  uint32    dma_source;         /* 00 */
  uint32    dma_dest;           /* 04 */
  uint32    dma_cmd_list;       /* 08 */
            #define DMA_CMD_MEMSET                   0x08000000
            #define DMA_CMD_REPLACE_LENGTH           0x07000000
            #define DMA_CMD_INSERT_LENGTH            0x06000000
            #define DMA_CMD_CHECKSUM2                0x05000000
            #define DMA_CMD_CHECKSUM1                0x04000000
            #define DMA_CMD_DELETE                   0x03000000
            #define DMA_CMD_REPLACE                  0x02000000
            #define DMA_CMD_INSERT                   0x01000000
            #define DMA_CMD_OPCODE_MASK              0xFF000000
            #define DMA_CMD_OPCODE_SHIFT             24
            #define DMA_CMD_OFFSET_MASK              0x00FFFF00
            #define DMA_CMD_OFFSET_SHIFT             8
            #define DMA_CMD_LENGTH_MASK              0x000000FF
            #define DMA_CMD_LENGTH_SHIFT             0
  uint32    dma_len_ctl;        /* 0c */
            #define DMA_CTL_LEN_LENGTH_N_VALUE_MASK  0xFFFC0000
            #define DMA_CTL_LEN_LENGTH_N_VALUE_SHIFT 18
            #define DMA_CTL_LEN_WAIT_BIT             0x00020000
            #define DMA_CTL_LEN_EXEC_CMD_LIST_BIT    0x00010000
            #define DMA_CTL_LEN_DEST_ADDR_MASK       0x0000C000
            #define DMA_CTL_LEN_DEST_IS_TOKEN_MASK   0x0000C000
            #define DMA_CTL_LEN_DEST_IS_TOKEN_SHIFT  14
            #define DMA_CTL_LEN_SRC_IS_TOKEN_BIT     0x00002000
            #define DMA_CTL_LEN_CONTINUE_BIT         0x00001000
            #define DMA_CTL_LEN_LEN_MASK             0x00000FFF
  uint32    dma_rslt_source;    /* 10 */
  uint32    dma_rslt_dest;      /* 14 */
  uint32    dma_rslt_hcs;       /* 18 */
            #define DMA_RSLT_HCS_HCS0_MASK           0x0000FFFF
            #define DMA_RSLT_HCS_HCS0_SHIFT          0
            #define DMA_RSLT_HCS_HCS1_MASK           0xFFFF0000
            #define DMA_RSLT_HCS_HCS1_SHIFT          16
  uint32    dma_rslt_len_stat;  /* 1C */
            #define DMA_RSLT_ERROR_MASK              0x003FE000
            #define DMA_RSLT_NOT_END_CMDS            0x00200000
            #define DMA_RSLT_FLUSHED                 0x00100000
            #define DMA_RSLT_ABORTED                 0x00080000
            #define DMA_RSLT_ERR_CMD_FMT             0x00040000
            #define DMA_RSLT_ERR_DEST                0x00020000
            #define DMA_RSLT_ERR_SRC                 0x00010000
            #define DMA_RSLT_ERR_CMD_LIST            0x00008000
            #define DMA_RSLT_ERR_DEST_LEN            0x00004000
            #define DMA_RSLT_ERR_SRC_LEN             0x00002000
            #define DMA_RSLT_CONTINUE                0x00001000
            #define DMA_RSLT_DMA_LEN                 0x00000FFF
} mDma_regs_S;


typedef struct DMARegs_S
{
  mDma_regs_S    dma_ch[2];
} DMARegs_S;


/* Token Registers */
typedef struct TknIntfRegs_S
{
  uint32    tok_buf_size;    /* 00 */
  uint32    tok_buf_base;    /* 04 */
  uint32    tok_idx2ptr_idx; /* 08 */
  uint32    tok_idx2ptr_ptr; /* 0c */
} TknIntfRegs_S;

/* Performance Measurement Registers on 4ke */
typedef struct PMRegs_S
{
  uint32        DCacheHit;      /* n/a  */
  uint32        DCacheMiss;     /* n/a  */
  uint32        ICacheHit;      /* 08 */
  uint32        ICacheMiss;     /* 0c */
  uint32        InstnComplete;  /* 10 */
  uint32        WTBMerge;       /* n/a  */
  uint32        WTBNoMerge;     /* n/a  */
} PMRegs_S;

/* MessageID Registers */
typedef struct MsgIdRegs_S
{
  uint32    msg_id[64];
} MsgIdRegs_S;



/*
** FAP DQM Registers
*/

typedef struct DQMCtlRegs_S /* 1800 */
{
  uint32        cfg;                        /* 00 */
                #define DQM_CFG_TOT_MEM_SZ_MASK      0xFFFF0000
                #define DQM_CFG_TOT_MEM_SZ_SHIFT     16
                #define DQM_CFG_START_ADDR_MASK      0x0000FFFF
                #define DQM_CFG_START_ADDR_SHIFT     0
  uint32        _4ke_low_wtmk_irq_msk;      /* 04 */
  uint32        mips_low_wtmk_irq_msk;      /* 08 */
  uint32        low_wtmk_irq_sts;           /* 0c */
  uint32        _4ke_not_empty_irq_msk;     /* 10 */
  uint32        mips_not_empty_irq_msk;     /* 14 */
  uint32        not_empty_irq_sts;          /* 18 */
  uint32        queue_rst;                  /* 1c */
  uint32        not_empty_sts;              /* 20 */
  uint32        next_avail_mask;            /* 24 */
  uint32        next_avail_queue;           /* 28 */
} DQMCtlRegs_S;


/* DQM Queue Control */
typedef struct DQMQRegs_S
{
  uint32        size;   /* 00 */
                #define Q_HEAD_PTR_MASK                     0xFFFC0000
                #define Q_HEAD_PTR_SHIFT                    18
                #define Q_TAIL_PTR_MASK                     0x0003FFF0
                #define Q_TAIL_PTR_SHIFT                    4
                #define Q_TOKEN_SIZE_MASK                   0x00000003
                #define Q_TOKEN_SIZE_SHIFT                  0
  uint32        cfgA;   /* 04 */
                #define Q_SIZE_MASK                         0xffff0000
                #define Q_SIZE_SHIFT                        16
                #define Q_START_ADDR_MASK                   0x0000ffff
                #define Q_START_ADDR_SHIFT                  0
  uint32        cfgB;   /* 08 */
                #define Q_NUM_TKNS_MASK                     0x3fff0000
                #define Q_NUM_TKNS_SHIFT                    16
                #define Q_LOW_WATERMARK_MASK                0x00003fff
                #define Q_LOW_WATERMARK_SHIFT               0
  uint32        sts;    /* 0c */
                #define AVAIL_TOKEN_SPACE_MASK              0x00003FFF
} DQMQRegs_S;

typedef struct DQMQCntrlRegs_S
{
  DQMQRegs_S q[32];
} DQMQCntrlRegs_S;

/* DQM Queue Data */
typedef struct DQMQueueDataReg_S
{
  uint32        word0;   /* 00 */
  uint32        word1;   /* 04 */
  uint32        word2;   /* 08 */
  uint32        word3;   /* 0c */
} DQMQueueDataReg_S;

typedef struct DQMQDataRegs_S
{
  DQMQueueDataReg_S q[32];
} DQMQDataRegs_S;


/* DQM Queue MIB */
typedef struct DQMQMibRegs_S
{
  uint32          MIB_NumFull[32];
  uint32          MIB_NumEmpty[32];
  uint32          MIB_TokensPushed[32];

} DQMQMibRegs_S;

#ifdef __cplusplus
}
#endif

#endif

