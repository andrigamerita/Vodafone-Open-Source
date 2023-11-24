/****************************************************************************
 *
 * rg/pkg/include/klog_enums.h
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
/*
 * To be used in klog.h and mt_crash_log.c to create necessary structs.
 * KLOG_ENUM_MARCO is defined in klog_defs.h and in mgt_klog.c
 */

KLOG_ENUM_MACRO(KLOG_NO_CRASH, "KLOG_NO_CRASH", "No crash was detected in the last boot (hardware reset?).")
KLOG_ENUM_MACRO(KLOG_CRASH, "KLOG_CRASH", "A new crash was detected and logged in the last boot.")
KLOG_ENUM_MACRO(KLOG_ERROR, "KLOG_ERROR", "KLog internal error. Crash status unknown.")
KLOG_ENUM_MACRO(KLOG_REBOOT, "KLOG_REBOOT", "Reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_HW_BTN_REBOOT, "KLOG_HW_BTN_REBOOT", "HW button reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_WATCHDOG_REBOOT, "KLOG_WATCHDOG_REBOOT", "Watchdog reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_WBM_REBOOT, "KLOG_WBM_REBOOT", "WBM-initiated reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_SCR_REBOOT, "KLOG_SCR_REBOOT", "SCR-initiated reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_JNET_REBOOT, "KLOG_JNET_REBOOT", "JNET-initiated reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_RMT_UPGRADE_REBOOT, "KLOG_RMT_UPGRADE_REBOOT", "Post upgrade reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_DOCSIS_REBOOT, "KLOG_DOCSIS_REBOOT", "DOCSIS-initiated reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_SNMP_REBOOT, "KLOG_SNMP_REBOOT", "SNMP-initiated reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_SOFTWARE_UNKNOWN, "KLOG_SOFTWARE_UNKNOWN", "Unknown software crash.")
KLOG_ENUM_MACRO(KLOG_CLI_REBOOT, "KLOG_CLI_REBOOT", "CLI-initiated reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_IPC_REBOOT, "KLOG_IPC_REBOOT", "IPC-initiated reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_RESTORE_DEFAULT_DUE_TO_BAD_REBOOT,
"KLOG_RESTORE_DEFAULT_DUE_TO_BAD_REBOOT", "Reboot due to bad reboot count was detected and logged.")
KLOG_ENUM_MACRO(KLOG_DOCSIS_SNMP_REBOOT, "KLOG_DOCSIS_SNMP_REBOOT", "DOCSIS SNMP initiated reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_TR_REBOOT, "KLOG_TR_REBOOT", "TR-initiated reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_EASY_MNG_REBOOT, "KLOG_EASY_MNG_REBOOT", "Easy management reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_BR_MODE_REBOOT,
"KLOG_BR_MODE_REBOOT", "Returning from bridge mode, restoring defaults, reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_DSL_MONITOR_REBOOT, "KLOG_DSL_MONITOR_REBOOT", "DSL monitor hack reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_OTHER_REBOOT, "KLOG_OTHER_REBOOT", "Uncategorized reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_RESTORE_DEFAULTS, "KLOG_RESTORE_DEFAULTS", "Reboot due to restore to default settings was detected and logged.")
KLOG_ENUM_MACRO(KLOG_CWMP_REBOOT, "KLOG_CWMP_REBOOT", "CWMP-initiated reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_CWMP_RESTORE_DEFAULTS_REBOOT,
"KLOG_CWMP_RESTORE_DEFAULTS_REBOOT", "Reboot due to CWMP initiated  restore to default settings was detected and logged.")
KLOG_ENUM_MACRO(KLOG_VSAF_INSTALL_FAILURE_REBOOT,
"KLOG_VSAF_INSTALL_FAILURE_REBOOT", "VSAF installation failure reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_AUTOMATIC_REBOOT, "KLOG_AUTOMATIC_REBOOT",
    "Automatic reboot was detected and logged.")
KLOG_ENUM_MACRO(KLOG_EASY_MNG_RESTORE_DEFAULTS_REBOOT, 
"KLOG_EASY_MNG_RESTORE_DEFAULTS_REBOOT", "Reboot due to Easy management ootb restore to default settings was detected and logged.")
