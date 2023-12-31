/****************************************************************************
 *
 * rg/pkg/include/kos_chardev_id.h
 * 
 * Copyright (C) Jungo LTD 2004
 * 
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General 
 * Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02111-1307, USA.
 *
 * Developed by Jungo LTD.
 * Residential Gateway Software Division
 * www.jungo.com
 * info@jungo.com
 */

#ifndef _KOS_CHARDEV_ID_H_
#define _KOS_CHARDEV_ID_H_

typedef enum {
    KOS_CDT_DHCP = 1,
    KOS_CDT_BRIDGE = 2,
    KOS_CDT_IPF = 3,
    KOS_CDT_IPF_NAT = 4,
    KOS_CDT_IPF_STATE = 5,
    KOS_CDT_IPV4 = 7,
    KOS_CDT_CHWAN = 10,
    KOS_CDT_LOG = 11,
    KOS_CDT_KNET_IF_EXT = 12,
    KOS_CDT_KLEDS = 15,
    KOS_CDT_HW_BUTTONS = 16,
    KOS_CDT_AUTH1X = 17,
    KOS_CDT_KCALL = 18,
    KOS_CDT_PKTCBL = 19,
    KOS_CDT_NETBIOS_RT = 20,
    KOS_CDT_JFW = 21,
    KOS_CDT_WATCHDOG = 22,
    KOS_CDT_PPPOE_RELAY = 23,
    KOS_CDT_KNET_MIB2_COUNTERS = 24,
    KOS_CDT_PPP = 25,
    KOS_CDT_PPPOE_BACKEND = 26,
    KOS_CDT_PPPCHARDEV_BACKEND = 27,
    KOS_CDT_LIC = 29,
    KOS_CDT_ROUTE = 30,
    KOS_CDT_RTP = 31,
    KOS_CDT_VOIP_SLIC = 32,
    KOS_CDT_WDS_CONN_NOTIFIER = 35,
    KOS_CDT_PPTP_BACKEND = 36,
    KOS_CDT_PPPOH_BACKEND = 37,
    KOS_CDT_HSS = 38,
    KOS_CDT_MSS = 39,
    KOS_CDT_VOIP_HWEMU = 40,
    KOS_CDT_VOIP_DSP = 41,
    KOS_CDT_COMP_REG = 42,
    KOS_CDT_M88E6021_SWITCH = 43,
    KOS_CDT_QOS_INGRESS = 45,
    KOS_CDT_BCM53XX_SWITCH = 46,
    KOS_CDT_FW_STATS = 47,
    KOS_CDT_STP = 48,
    KOS_CDT_FASTPATH = 49,
    KOS_CDT_HW_QOS = 50,
    KOS_CDT_SKB_CACHE = 51,
    KOS_CDT_SAMPLE_CHARDEV = 52,
    KOS_CDT_SWITCH = 53,
    KOS_CDT_KLOG = 54,
    KOS_CDT_ADM6996_SWITCH = 55,
    KOS_CDT_AR8316_SWITCH = 56,
    KOS_CDT_RT2880_MONITOR = 57,
    KOS_CDT_L2TP_BACKEND = 58,
    KOS_CDT_VIRTUAL_WAN = 59,
    KOS_CDT_PPPOS_BACKEND = 60,
    KOS_CDT_PSB6973_SWITCH = 61,
    KOS_CDT_BCM9636X_HW_SWITCH = 62,
    KOS_CDT_IGMP_PROXY = 63,
    KOS_CDT_SIM_DETECT = 64,
    KOS_CDT_RTL836X_HW_SWITCH = 65,
    KOS_CDT_BCM53125_HW_SWITCH = 66,
    KOS_CDT_PROXIMITY = 67,
    KOS_CDT_QOS_DSCP2PRIO = 68,
    KOS_CDT_IPSEC = 69,
    KOS_CDT_802_3AH = 70,
    KOS_CDT_PEAK_THROUGHPUT = 71,
    KOS_CDT_ADSL_THROUGHPUT_CHANGE = 72,
    KOS_CDT_DUS_RAW_DATA_ACTIVE = 73,
    KOS_CDT_DUS_RAW_DATA_CLOSED = 74,
} kos_chardev_type_t;

#endif

