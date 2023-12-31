/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2005, Digium, Inc.
 *
 * Mark Spencer <markster@digium.com>
 * Oliver Daudey <traveler@xs4all.nl>
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
 *
 * \brief App to set rdnis
 *
 * \ingroup applications
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include "asterisk/lock.h"
#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/module.h"
#include "asterisk/translate.h"
#include "asterisk/image.h"
#include "asterisk/callerid.h"
#include "asterisk/utils.h"

static char *tdesc = "Set RDNIS Number";

static char *app = "SetRDNIS";

static char *synopsis = "Set RDNIS Number";

static char *descrip = 
"  SetRDNIS(cnum): Set RDNIS Number on a call to a new\n"
"value.\n"
"SetRDNIS has been deprecated in favor of the function\n"
"CALLERID(rdnis)\n";

STANDARD_LOCAL_USER;

LOCAL_USER_DECL;

static int setrdnis_exec(struct ast_channel *chan, void *data)
{
	struct localuser *u;
	char *opt, *n, *l;
	char *tmp = NULL;
	static int deprecation_warning = 0;

	LOCAL_USER_ADD(u);
	
	if (!deprecation_warning) {
		ast_log(LOG_WARNING, "SetRDNIS is deprecated, please use Set(CALLERID(rdnis)=value) instead.\n");
		deprecation_warning = 1;
	}

	if (data)
		tmp = ast_strdupa(data);
	else
		tmp = "";	

	opt = strchr(tmp, '|');
	if (opt)
		*opt = '\0';
	
	n = l = NULL;
	ast_callerid_parse(tmp, &n, &l);
	if (l) {
		ast_shrink_phone_number(l);
		ast_mutex_lock(&chan->lock);
		if (chan->cid.cid_rdnis)
			free(chan->cid.cid_rdnis);
		chan->cid.cid_rdnis = (l[0]) ? strdup(l) : NULL;
		ast_mutex_unlock(&chan->lock);
	}

	LOCAL_USER_REMOVE(u);
	
	return 0;
}

int unload_module(void)
{
	int res;
	
	res = ast_unregister_application(app);
	
	STANDARD_HANGUP_LOCALUSERS;

	return res;	
}

int load_module(void)
{
	return ast_register_application(app, setrdnis_exec, synopsis, descrip);
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
