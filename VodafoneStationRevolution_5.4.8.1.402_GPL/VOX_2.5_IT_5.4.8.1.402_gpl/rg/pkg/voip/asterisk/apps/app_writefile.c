/****************************************************************************
 *
 * rg/pkg/voip/asterisk/apps/app_writefile.c
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "asterisk.h"

#include "asterisk/lock.h"
#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/module.h"
#include "asterisk/manager.h"

static char *tdesc = "Stores data into file";

static char *app = "WriteFile";

static char *synopsis = "WriteFile(file, data)";

static char *descrip = 
"Write File(file, data)\n"
" File - The name of the file to overwrite.\n"
" Data - Content to be written.\n";

STANDARD_LOCAL_USER;

LOCAL_USER_DECL;

static int writefile_exec(struct ast_channel *chan, void *data)
{
	struct localuser *u;
	char *s, *filename = NULL, *content = NULL;
	int fd = 0, len, ret;

	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING,
		    "WriteFile requires an argument\n");
		return -1;
	}

	LOCAL_USER_ADD(u);

	if (!(s = ast_strdupa(data))) {
	    ast_log(LOG_ERROR, "Out of memory\n");
	    goto Error;
	}

	filename = strsep(&s, "|");
	content = s;

	if (!filename) {
	    ast_log(LOG_ERROR, "No filename\n");
	    goto Error;
	}

	if (!content) {
	    ast_log(LOG_ERROR, "No data\n");
	    goto Error;
	}

	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )) < 0) {
	    ast_log(LOG_ERROR, "Can't open file %s for writting. error: (%s)\n", filename, strerror(errno));
	    goto Error;
	}

	len = strlen(s);

	if ((ret = write(fd, s, len)) < 0) {
	    ast_log(LOG_ERROR, "Can't write file %s. error: (%s)\n", filename, strerror(errno));
	    goto Error;
	}

	if (len != ret) {
	    ast_log(LOG_ERROR, "Can't write file %s. Numbers of bytes written is less than expected. %d from %d\n", filename, ret, len);
	    goto Error;
	}

	close(fd);

	LOCAL_USER_REMOVE(u);
	return 0;
Error:
	if (fd > 0)
	    close(fd);
	LOCAL_USER_REMOVE(u);
	return -1;
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
	return ast_register_application(app, writefile_exec, synopsis, descrip);
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
