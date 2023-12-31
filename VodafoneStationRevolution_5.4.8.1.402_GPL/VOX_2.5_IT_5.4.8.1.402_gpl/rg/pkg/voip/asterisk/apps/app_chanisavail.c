/*
* Asterisk -- An open source telephony toolkit.
*
* Copyright (C) 1999 - 2005, Digium, Inc.
*
* Mark Spencer <markster@digium.com>
* James Golovich <james@gnuinter.net>
*
* See http://www.asterisk.org for more information about
* the Asterisk project. Please do not directly contact
* any of the maintainers of this project for assistance;
* the project provides a web site, mailing lists and IRC
* channels for your use.
*
* This program is free software, distributed under the terms of
* the GNU General Public License Version 2. See the LICENSE file
* at the top of the source tree.
*/

/*! \file
* \brief Check if Channel is Available
* 
 * \ingroup applications
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include "asterisk/lock.h"
#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/module.h"
#include "asterisk/app.h"
#include "asterisk/devicestate.h"
#include "asterisk/options.h"

static char *tdesc = "Check channel availability";

static char *app = "ChanIsAvail";

static char *synopsis = "Check channel availability";

static char *descrip = 
"  ChanIsAvail(Technology/resource[&Technology2/resource2...][|options]): \n"
"This application will check to see if any of the specified channels are\n"
"available. The following variables will be set by this application:\n"
"  ${AVAILCHAN}     - the name of the available channel, if one exists\n"
"  ${AVAILORIGCHAN} - the canonical channel name that was used to create the channel\n"
"  ${AVAILSTATUS}   - the status code for the available channel\n"
"  Options:\n"
"    s - Consider the channel unavailable if the channel is in use at all\n"
"    j - Support jumping to priority n+101 if no channel is available\n";

STANDARD_LOCAL_USER;

LOCAL_USER_DECL;

static int chanavail_exec(struct ast_channel *chan, void *data)
{
	int res=-1, inuse=-1, option_state=0, priority_jump=0;
	int status;
	struct localuser *u;
	char *info, tmp[512], trychan[512], *peers, *tech, *number, *rest, *cur, *options, *stringp;
	struct ast_channel *tempchan;

	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "ChanIsAvail requires an argument (Zap/1&Zap/2)\n");
		return -1;
	}

	LOCAL_USER_ADD(u);

	info = ast_strdupa(data); 
	stringp = info;
	strsep(&stringp, "|");
	options = strsep(&stringp, "|");
	if (options) {
		if (strchr(options, 's'))
			option_state = 1;
		if (strchr(options, 'j'))
			priority_jump = 1;
	}
	peers = info;
	if (peers) {
		cur = peers;
		do {
			/* remember where to start next time */
			rest = strchr(cur, '&');
			if (rest) {
				*rest = 0;
				rest++;
			}
			tech = cur;
			number = strchr(tech, '/');
			if (!number) {
				ast_log(LOG_WARNING, "ChanIsAvail argument takes format ([technology]/[device])\n");
				LOCAL_USER_REMOVE(u);
				return -1;
			}
			*number = '\0';
			number++;
			
			if (option_state) {
				/* If the pbx says in use then don't bother trying further.
				   This is to permit testing if someone's on a call, even if the 
	 			   channel can permit more calls (ie callwaiting, sip calls, etc).  */
                               
				snprintf(trychan, sizeof(trychan), "%s/%s",cur,number);
				status = inuse = ast_device_state(trychan);
			}
			if ((inuse <= 1) && (tempchan = ast_request(tech, chan->nativeformats, number, &status))) {
					pbx_builtin_setvar_helper(chan, "AVAILCHAN", tempchan->name);
					/* Store the originally used channel too */
					snprintf(tmp, sizeof(tmp), "%s/%s", tech, number);
					pbx_builtin_setvar_helper(chan, "AVAILORIGCHAN", tmp);
					snprintf(tmp, sizeof(tmp), "%d", status);
					pbx_builtin_setvar_helper(chan, "AVAILSTATUS", tmp);
					ast_hangup(tempchan);
					tempchan = NULL;
					res = 1;
					break;
			} else {
				snprintf(tmp, sizeof(tmp), "%d", status);
				pbx_builtin_setvar_helper(chan, "AVAILSTATUS", tmp);
			}
			cur = rest;
		} while (cur);
	}
	if (res < 1) {
		pbx_builtin_setvar_helper(chan, "AVAILCHAN", "");
		pbx_builtin_setvar_helper(chan, "AVAILORIGCHAN", "");
		if (priority_jump || option_priority_jumping) {
			if (ast_goto_if_exists(chan, chan->context, chan->exten, chan->priority + 101)) {
				LOCAL_USER_REMOVE(u);
				return -1;
			}
		}
	}

	LOCAL_USER_REMOVE(u);
	return 0;
}

int unload_module(void)
{
	int res = 0;

	res = ast_unregister_application(app);

	STANDARD_HANGUP_LOCALUSERS;
	
	return res;
}

int load_module(void)
{
	return ast_register_application(app, chanavail_exec, synopsis, descrip);
}

char *description(void)
{
	return tdesc;
}

int usecount(void)
{
	int res;
	STANDARD_USECOUNT(res);
	return res;
}

char *key()
{
	return ASTERISK_GPL_KEY;
}
