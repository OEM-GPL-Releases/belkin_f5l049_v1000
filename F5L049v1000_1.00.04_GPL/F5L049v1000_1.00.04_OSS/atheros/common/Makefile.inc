#
# (C) Copyright 2007 Atheros Communications, Inc.
# All rights reserved.
#

#
# Path to the fusion top
#
TOPDIR := $(obj)/$(DEPTH)
#
# Path to the common source base
#
COMMON := $(TOPDIR)/common

#
# Path to common include files
#
COMINC := $(TOPDIR)/include

#
# Path to the HAL source code.
#
HAL := $(COMMON)/hal

#
# Path to the Atheros Device module.
#
ATH := $(COMMON)/ath_dev

#
# Path to the rate control algorithm.
#
ATH_RATE := $(COMMON)/ratectrl

#
# Path to the DFS module.
#
ATH_DFS := $(COMMON)/dfs

# Packet logging
ATH_PKTLOG := $(COMMON)/ath_pktlog

#
# Path to the 802.11 include files.
#
WLAN := $(TOPDIR)/$(OS)/net80211

# Protocol interface
IF_WLAN := $(TOPDIR)/if_ath_net80211
