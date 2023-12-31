/****************************************************************************
 *
 * rg/pkg/include/klog_defs.h
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

#ifndef _MGT_KLOG_H_
#define _MGT_KLOG_H_

#define REBOOT_REASON_SIZE 128

typedef enum {
#define KLOG_ENUM_MACRO(enumkey, str, prettystr) enumkey,
#include <klog_enums.h>
#undef KLOG_ENUM_MACRO
    KLOG_STATUS_MAX
} klog_status_t;

extern void (*mgt_set_reboot_reason)(unsigned int reason, char *reason_file,
    const char *reason_func, int reason_line);
extern void (*mgt_set_reboot_reason_text)(unsigned int reason,
    char *reason_text);
#endif
