/****************************************************************************
 *
 * rg/pkg/util/mgt_client.h
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

#ifndef _MGT_CLIENT_LCL_H_
#define _MGT_CLIENT_LCL_H_

#include <klog_defs.h>

/* Open a connection to the local host, execute a command, receive the 
 * results and close the connection */
int mgt_command(char *cmd, char **result);
int mgt_command_fmt(char **result, char *fmt, ...);
#define mgt_reboot_command(linux_fallback, reason) \
    do { \
    _mgt_reboot_command(linux_fallback, reason, __FILE__, __FUNCTION__, \
        __LINE__); \
    } while (0)

void mgt_set_reboot_reason_command(klog_status_t reason,
    char *reason_file, const char *reason_func, int reason_line);
void _mgt_reboot_command(int linux_fallback, klog_status_t reason,
    char *reason_file, const char *reason_func, int reason_line);

#endif
