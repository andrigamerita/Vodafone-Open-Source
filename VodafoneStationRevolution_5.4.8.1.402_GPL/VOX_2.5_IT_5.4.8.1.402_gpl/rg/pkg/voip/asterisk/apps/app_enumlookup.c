/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2005, Digium, Inc.
 *
 * Mark Spencer <markster@digium.com>
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
 * \brief Enumlookup - lookup entry in ENUM
 *
 * \ingroup applications
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include "asterisk/lock.h"
#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/options.h"
#include "asterisk/config.h"
#include "asterisk/module.h"
#include "asterisk/enum.h"
#include "asterisk/utils.h"
#include "asterisk/app.h"
#include "asterisk/options.h"

static char *tdesc = "ENUM Lookup";

static char *app = "EnumLookup";

static char *synopsis = "Lookup number in ENUM";

static char *descrip =
"  EnumLookup(exten[|option]):  Looks up an extension via ENUM and sets\n"
"the variable 'ENUM'. For VoIP URIs this variable will \n"
"look like 'TECHNOLOGY/URI' with the appropriate technology.\n"
"Currently, the enumservices SIP, H323, IAX, IAX2 and TEL are recognized. \n"
"\nReturns status in the ENUMSTATUS channel variable:\n"
"    ERROR	Failed to do a lookup\n"
"    <tech>	Technology of the successful lookup: SIP, H323, IAX, IAX2 or TEL\n"
"    BADURI	Got URI Asterisk does not understand.\n"
"  The option string may contain zero or the following character:\n"
"       'j' -- jump to +101 priority if the lookup isn't successful.\n"
"	       and jump to +51 priority on a TEL entry.\n";

#define ENUM_CONFIG "enum.conf"

static char h323driver[80] = "";
#define H323DRIVERDEFAULT "H323"

STANDARD_LOCAL_USER;

LOCAL_USER_DECL;

/*--- enumlookup_exec: Look up number in ENUM and return result */
static int enumlookup_exec(struct ast_channel *chan, void *data)
{
	int res=0,priority_jump=0;
	char tech[80];
	char dest[80];
	char tmp[256];
	char *c,*t = NULL;
	static int dep_warning=0;
	struct localuser *u;
	char *parse;
	AST_DECLARE_APP_ARGS(args,
		AST_APP_ARG(d);
		AST_APP_ARG(o);
	);

	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "EnumLookup requires an argument (extension)\n");
		return -1;
	}
		
	if (!dep_warning) {
		ast_log(LOG_WARNING, "The application EnumLookup is deprecated.  Please use the ENUMLOOKUP() function instead.\n");
		dep_warning = 1;
	}

	LOCAL_USER_ADD(u);

	parse = ast_strdupa(data);
	if (!parse) {
		ast_log(LOG_ERROR, "Out of memory!\n");
		LOCAL_USER_REMOVE(u);
		return -1;
	}

	AST_STANDARD_APP_ARGS(args, parse);

	tech[0] = '\0';
	dest[0] = '\0';

	if (args.o) {
		if (strchr(args.o, 'j'))
			priority_jump = 1;
	}

	res = ast_get_enum(chan, args.d, dest, sizeof(dest), tech, sizeof(tech), NULL, NULL);
	
	if (!res) {	/* Failed to do a lookup */
		if (priority_jump || option_priority_jumping) {
			/* Look for a "busy" place */
			ast_goto_if_exists(chan, chan->context, chan->exten, chan->priority + 101);
		}
		pbx_builtin_setvar_helper(chan, "ENUMSTATUS", "ERROR");
		LOCAL_USER_REMOVE(u);
		return 0;
	}
	pbx_builtin_setvar_helper(chan, "ENUMSTATUS", tech);
	/* Parse it out */
	if (res > 0) {
		if (!strcasecmp(tech, "SIP")) {
			c = dest;
			if (!strncmp(c, "sip:", 4))
				c += 4;
			snprintf(tmp, sizeof(tmp), "SIP/%s", c);
			pbx_builtin_setvar_helper(chan, "ENUM", tmp);
		} else if (!strcasecmp(tech, "h323")) {
			c = dest;
			if (!strncmp(c, "h323:", 5))
				c += 5;
			snprintf(tmp, sizeof(tmp), "%s/%s", h323driver, c);
/* do a s!;.*!! on the H323 URI */
			t = strchr(c,';');
                       if (t)
				*t = 0;
			pbx_builtin_setvar_helper(chan, "ENUM", tmp);
		} else if (!strcasecmp(tech, "iax")) {
			c = dest;
			if (!strncmp(c, "iax:", 4))
				c += 4;
			snprintf(tmp, sizeof(tmp), "IAX/%s", c);
			pbx_builtin_setvar_helper(chan, "ENUM", tmp);
		} else if (!strcasecmp(tech, "iax2")) {
			c = dest;
			if (!strncmp(c, "iax2:", 5))
				c += 5;
			snprintf(tmp, sizeof(tmp), "IAX2/%s", c);
			pbx_builtin_setvar_helper(chan, "ENUM", tmp);
		} else if (!strcasecmp(tech, "tel")) {
			c = dest;
			if (!strncmp(c, "tel:", 4))
				c += 4;

			if (c[0] != '+') {
				ast_log(LOG_NOTICE, "tel: uri must start with a \"+\" (got '%s')\n", c);
				res = 0;
			} else {
/* now copy over the number, skipping all non-digits and stop at ; or NULL */
                               t = tmp;
				while( *c && (*c != ';') && (t - tmp < (sizeof(tmp) - 1))) {
					if (isdigit(*c))
						*t++ = *c;
					c++;
				}
				*t = 0;
				pbx_builtin_setvar_helper(chan, "ENUM", tmp);
				ast_log(LOG_NOTICE, "tel: ENUM set to \"%s\"\n", tmp);
				if (priority_jump || option_priority_jumping) {
					if (ast_goto_if_exists(chan, chan->context, chan->exten, chan->priority + 51))
						res = 0;
				}
			}
		} else if (!ast_strlen_zero(tech)) {
			ast_log(LOG_NOTICE, "Don't know how to handle technology '%s'\n", tech);
			pbx_builtin_setvar_helper(chan, "ENUMSTATUS", "BADURI");
			res = 0;
		}
	}

	LOCAL_USER_REMOVE(u);

	return 0;
}

/*--- load_config: Load enum.conf and find out how to handle H.323 */
static int load_config(void)
{
	struct ast_config *cfg;
	char *s;

	cfg = ast_config_load(ENUM_CONFIG);
	if (cfg) {
		if (!(s=ast_variable_retrieve(cfg, "general", "h323driver"))) {
			strncpy(h323driver, H323DRIVERDEFAULT, sizeof(h323driver) - 1);
		} else {
			strncpy(h323driver, s, sizeof(h323driver) - 1);
		}
		ast_config_destroy(cfg);
		return 0;
	}
	ast_log(LOG_NOTICE, "No ENUM Config file, using defaults\n");
	return 0;
}


/*--- unload_module: Unload this application from PBX */
int unload_module(void)
{
	int res;

	res = ast_unregister_application(app);

	STANDARD_HANGUP_LOCALUSERS;

	return res;
}

/*--- load_module: Load this application into PBX */
int load_module(void)
{
	int res;
	
	res = ast_register_application(app, enumlookup_exec, synopsis, descrip);
	
	if (!res)
		res = load_config();
	
	return res;
}

/*--- reload: Reload configuration file */
int reload(void)
{
	return load_config();
}


/*--- description: Describe module */
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

