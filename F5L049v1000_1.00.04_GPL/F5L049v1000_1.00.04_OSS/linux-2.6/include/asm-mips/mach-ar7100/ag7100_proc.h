/**
 * @file
 *
 * ag7100 ethernet link check proc fs
 * 
 * Copyright (C) 2008 silex technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef _AG7100_BOOT_H
#define _AG7100_BOOT_H

/*
 * 0, 1, 2: based on hardware values for mii ctrl bits [5,4]
 */
typedef enum {
    AG7100_PHY_SPEED_10T,
    AG7100_PHY_SPEED_100TX,
    AG7100_PHY_SPEED_1000T,
}ag7100_phy_speed_t;

#define ag7100_mac_base(_no)    (_no) ? AR7100_GE1_BASE    : AR7100_GE0_BASE

#define ag7100_mac_irq(_no)     (_no) ? AR7100_CPU_IRQ_GE1 : AR7100_CPU_IRQ_GE0

#define ag7100_reset_mask(_no)  (_no) ? (AR7100_RESET_GE1_MAC |  \
                                         AR7100_RESET_GE1_PHY)   \
                                      : (AR7100_RESET_GE0_MAC |  \
                                         AR7100_RESET_GE0_PHY)

#define ag7100_unit2mac(_unit)     ag7100_macs[(_unit)]

#define assert(_cond)   do {                                     \
    if(!(_cond)) {                                               \
        ag7100_trc_dump();                                       \
        printk("%s:%d: assertion failed\n", __func__, __LINE__); \
        BUG();                                                   \
    }                                                            \
}while(0);

/*
 * Config Mac specs
 */
/* Max Transmit packet size */
#define AG7100_LEN_PER_TX_DS       1536
#define AG7100_NUMBER_TX_PKTS      40
#define AG7100_NUMBER_RX_PKTS      252



/*
 * Config/Mac Register definitions
 */
#define AG7100_MAC_CFG1            0x00
#define AG7100_MAC_CFG2            0x04
#define AG7100_MAC_IFCTL           0x38

/*
 * fifo control registers
 */
#define AG7100_MAC_FIFO_CFG_0      0x48
#define AG7100_MAC_FIFO_CFG_1      0x4c
#define AG7100_MAC_FIFO_CFG_2      0x50
#define AG7100_MAC_FIFO_CFG_3      0x54
#define AG7100_MAC_FIFO_CFG_4      0x58

#define AG7100_MAC_FIFO_CFG_5      0x5c
#define AG7100_BYTE_PER_CLK_EN     (1 << 19)

#define AG7100_MAC_FIFO_RAM_0      0x60
#define AG7100_MAC_FIFO_RAM_1      0x64
#define AG7100_MAC_FIFO_RAM_2      0x68
#define AG7100_MAC_FIFO_RAM_3      0x6c
#define AG7100_MAC_FIFO_RAM_4      0x70
#define AG7100_MAC_FIFO_RAM_5      0x74
#define AG7100_MAC_FIFO_RAM_6      0x78
#define AG7100_MAC_FIFO_RAM_7      0x7c

/*
 * fields
 */
#define AG7100_MAC_CFG1_SOFT_RST       (1 << 31)
#define AG7100_MAC_CFG1_RX_RST         (1 << 19)
#define AG7100_MAC_CFG1_TX_RST         (1 << 18)
#define AG7100_MAC_CFG1_LOOPBACK       (1 << 8)
#define AG7100_MAC_CFG1_RX_EN          (1 << 2)
#define AG7100_MAC_CFG1_TX_EN          (1 << 0)
#define AG7100_MAC_CFG1_RX_FCTL        (1 << 5)
#define AG7100_MAC_CFG1_TX_FCTL        (1 << 4)


#define AG7100_MAC_CFG2_FDX            (1 << 0)
#define AG7100_MAC_CFG2_CRC_EN         (1 << 1)
#define AG7100_MAC_CFG2_PAD_CRC_EN     (1 << 2)
#define AG7100_MAC_CFG2_LEN_CHECK      (1 << 4)
#define AG7100_MAC_CFG2_HUGE_FRAME_EN  (1 << 5)
#define AG7100_MAC_CFG2_IF_1000        (1 << 9)
#define AG7100_MAC_CFG2_IF_10_100      (1 << 8)

#define AG7100_MAC_IFCTL_SPEED         (1 << 16)

/*
 * DMA (tx/rx) register defines
 */
#define AG7100_DMA_TX_CTRL              0x180
#define AG7100_DMA_TX_DESC              0x184
#define AG7100_DMA_TX_STATUS            0x188
#define AG7100_DMA_RX_CTRL              0x18c
#define AG7100_DMA_RX_DESC              0x190
#define AG7100_DMA_RX_STATUS            0x194
#define AG7100_DMA_INTR_MASK            0x198
#define AG7100_DMA_INTR                 0x19c

/*
 * tx/rx ctrl and status bits
 */
#define AG7100_TXE                      (1 << 0)
#define AG7100_TX_STATUS_PKTCNT_SHIFT   16
#define AG7100_TX_STATUS_PKT_SENT       0x1
#define AG7100_TX_STATUS_URN            0x2
#define AG7100_TX_STATUS_BUS_ERROR      0x8

#define AG7100_RXE                      (1 << 0)

#define AG7100_RX_STATUS_PKTCNT_MASK    0xff0000
#define AG7100_RX_STATUS_PKT_RCVD       (1 << 0)
#define AG7100_RX_STATUS_OVF            (1 << 2)
#define AG7100_RX_STATUS_BUS_ERROR      (1 << 3)

/*
 * Int and int mask
 */
#define AG7100_INTR_TX                  (1 << 0)
#define AG7100_INTR_TX_URN              (1 << 1)
#define AG7100_INTR_TX_BUS_ERROR        (1 << 3)
#define AG7100_INTR_RX                  (1 << 4)
#define AG7100_INTR_RX_OVF              (1 << 6)
#define AG7100_INTR_RX_BUS_ERROR        (1 << 7)

/*
 * MII registers
 */
#define AG7100_MAC_MII_MGMT_CFG         0x20
#define AG7100_MGMT_CFG_CLK_DIV_20      0x06
#define AG7100_MII_MGMT_CMD             0x24
#define AG7100_MGMT_CMD_READ            0x1
#define AG7100_MII_MGMT_ADDRESS         0x28
#define AG7100_ADDR_SHIFT               8
#define AG7100_MII_MGMT_CTRL            0x2c
#define AG7100_MII_MGMT_STATUS          0x30
#define AG7100_MII_MGMT_IND             0x34
#define AG7100_MGMT_IND_BUSY            (1 << 0)
#define AG7100_MGMT_IND_INVALID         (1 << 2)
#define AG7100_GE_MAC_ADDR1             0x40
#define AG7100_GE_MAC_ADDR2             0x44
#define AG7100_MII0_CONTROL             0x18070000


/*****************/
/* PHY Registers */
/*****************/
#define ATHR_PHY_CONTROL                 0
#define ATHR_PHY_STATUS                  1
#define ATHR_PHY_ID1                     2
#define ATHR_PHY_ID2                     3
#define ATHR_AUTONEG_ADVERT              4
#define ATHR_LINK_PARTNER_ABILITY        5
#define ATHR_AUTONEG_EXPANSION           6
#define ATHR_PHY_FUNC_CONTROL            16
#define ATHR_PHY_SPEC_STATUS             17
#define ATHR_INT_ENABLE                  18
#define ATHR_INT_STATUS                  19
#define ATHR_EXT_PHY_SPEC_COONTROL       20
#define ATHR_RECV_ERR_COUNTER            21
#define ATHR_VIRL_CABLE_TEST_CTRL        22
#define ATHR_LED_CONTROL                 24
#define ATHR_MAN_LED_OVERRIDE            25
#define ATHR_VIRL_CABLE_TEST_STAT        28
#define ATHR_DEBUG_PORT1                 29
#define ATHR_DEBUG_PORT2                 30

/* ATHR_PHY_CONTROL fields */
#define ATHR_CTRL_SOFTWARE_RESET                    0x8000
#define ATHR_CTRL_SPEED_LSB                         0x2000
#define ATHR_CTRL_AUTONEGOTIATION_ENABLE            0x1000
#define ATHR_CTRL_RESTART_AUTONEGOTIATION           0x0200
#define ATHR_CTRL_SPEED_FULL_DUPLEX                 0x0100
#define ATHR_CTRL_SPEED_MSB                         0x0040

#define ATHR_RESET_DONE(phy_control)                   \
    (((phy_control) & (ATHR_CTRL_SOFTWARE_RESET)) == 0)
    
/* Phy status fields */
#define ATHR_STATUS_AUTO_NEG_DONE                   0x0020

#define ATHR_AUTONEG_DONE(ip_phy_status)                   \
        (((ip_phy_status) &                                \
        (ATHR_STATUS_AUTO_NEG_DONE)) ==                    \
        (ATHR_STATUS_AUTO_NEG_DONE))

/* Link Partner ability */
#define ATHR_LINK_100BASETX_FULL_DUPLEX       0x0100
#define ATHR_LINK_100BASETX                   0x0080
#define ATHR_LINK_10BASETX_FULL_DUPLEX        0x0040
#define ATHR_LINK_10BASETX                    0x0020

/* Advertisement register. */
#define ATHR_ADVERTISE_NEXT_PAGE              0x8000
#define ATHR_ADVERTISE_ASYM_PAUSE             0x0800
#define ATHR_ADVERTISE_PAUSE                  0x0400
#define ATHR_ADVERTISE_100FULL                0x0100
#define ATHR_ADVERTISE_100HALF                0x0080
#define ATHR_ADVERTISE_10FULL                 0x0040
#define ATHR_ADVERTISE_10HALF                 0x0020

#define ATHR_ADVERTISE_ALL (ATHR_ADVERTISE_10HALF | ATHR_ADVERTISE_10FULL | \
                            ATHR_ADVERTISE_100HALF | ATHR_ADVERTISE_100FULL)

/* Phy Specific status fields */
#define ATHER_STATUS_LINK_MASK                0xC000
#define ATHER_STATUS_LINK_SHIFT               14
#define ATHER_STATUS_FULL_DEPLEX              0x2000
#define ATHR_STATUS_LINK_PASS                 0x0400 
#define ATHR_STATUS_RESOVLED                  0x0800


/* PHY Link mode */
#define AG7100_LINKMODE_AUTO                  0
#define AG7100_LINKMODE_10HALF                1
#define AG7100_LINKMODE_10FULL                2
#define AG7100_LINKMODE_100HALF               3
#define AG7100_LINKMODE_100FULL               4
#define AG7100_LINKMODE_MAX                   AG7100_LINKMODE_100FULL
#endif /* _AG7100_BOOT_H */
