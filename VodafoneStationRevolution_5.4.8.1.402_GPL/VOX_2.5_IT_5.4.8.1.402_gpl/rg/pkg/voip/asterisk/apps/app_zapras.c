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
 * \brief Execute an ISDN RAS
 * 
 * \ingroup applications
 */

#include <sys/ioctl.h>
#include <sys/wait.h>
#ifdef __linux__
#include <sys/signal.h>
#else
#include <signal.h>
#endif /* __linux__ */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

/* Need some zaptel help here */
#ifdef __linux__
#include <linux/zaptel.h>
#else
#include <zaptel.h>
#endif /* __linux__ */

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include "asterisk/lock.h"
#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/module.h"
#include "asterisk/options.h"

static char *tdesc = "Zap RAS Application";

static char *app = "ZapRAS";

static char *synopsis = "Executes Zaptel ISDN RAS application";

static char *descrip =
"  ZapRAS(args): Executes a RAS server using pppd on the given channel.\n"
"The channel must be a clear channel (i.e. PRI source) and a Zaptel\n"
"channel to be able to use this function (No modem emulation is included).\n"
"Your pppd must be patched to be zaptel aware. Arguments should be\n"
"separated by | characters.\n";

STANDARD_LOCAL_USER;

LOCAL_USER_DECL;

#define PPP_MAX_ARGS	32
#define PPP_EXEC	"/usr/sbin/pppd"

static pid_t spawn_ras(struct ast_channel *chan, char *args)
{
	pid_t pid;
	int x;	
	char *c;

	char *argv[PPP_MAX_ARGS];
	int argc = 0;
	char *stringp=NULL;

	/* Start by forking */
	pid = fork();
	if (pid)
		return pid;

	/* Execute RAS on File handles */
	dup2(chan->fds[0], STDIN_FILENO);

	/* Close other file descriptors */
	for (x=STDERR_FILENO + 1;x<1024;x++) 
		close(x);

	/* Restore original signal handlers */
	for (x=0;x<NSIG;x++)
		signal(x, SIG_DFL);

	/* Reset all arguments */
	memset(argv, 0, sizeof(argv));

	/* First argument is executable, followed by standard
	   arguments for zaptel PPP */
	argv[argc++] = PPP_EXEC;
	argv[argc++] = "nodetach";

	/* And all the other arguments */
	stringp=args;
	c = strsep(&stringp, "|");
	while(c && strlen(c) && (argc < (PPP_MAX_ARGS - 4))) {
		argv[argc++] = c;
		c = strsep(&stringp, "|");
	}

	argv[argc++] = "plugin";
	argv[argc++] = "zaptel.so";
	argv[argc++] = "stdin";

#if 0
	for (x=0;x<argc;x++) {
		fprintf(stderr, "Arg %d: %s\n", x, argv[x]);
	}
#endif

	/* Finally launch PPP */
	execv(PPP_EXEC, argv);
	fprintf(stderr, "Failed to exec PPPD!\n");
	exit(1);
}

static void run_ras(struct ast_channel *chan, char *args)
{
	pid_t pid;
	int status;
	int res;
	int signalled = 0;
	struct zt_bufferinfo savebi;
	int x;
	
	res = ioctl(chan->fds[0], ZT_GET_BUFINFO, &savebi);
	if(res) {
		ast_log(LOG_WARNING, "Unable to check buffer policy on channel %s\n", chan->name);
		return;
	}

	pid = spawn_ras(chan, args);
	if (pid < 0) {
		ast_log(LOG_WARNING, "Failed to spawn RAS\n");
	} else {
		for (;;) {
			res = wait4(pid, &status, WNOHANG, NULL);
			if (!res) {
				/* Check for hangup */
				if (chan->_softhangup && !signalled) {
					ast_log(LOG_DEBUG, "Channel '%s' hungup.  Signalling RAS at %d to die...\n", chan->name, pid);
					kill(pid, SIGTERM);
					signalled=1;
				}
				/* Try again */
				sleep(1);
				continue;
			}
			if (res < 0) {
				ast_log(LOG_WARNING, "wait4 returned %d: %s\n", res, strerror(errno));
			}
			if (option_verbose > 2) {
				if (WIFEXITED(status)) {
					ast_verbose(VERBOSE_PREFIX_3 "RAS on %s terminated with status %d\n", chan->name, WEXITSTATUS(status));
				} else if (WIFSIGNALED(status)) {
					ast_verbose(VERBOSE_PREFIX_3 "RAS on %s terminated with signal %d\n", 
						 chan->name, WTERMSIG(status));
				} else {
					ast_verbose(VERBOSE_PREFIX_3 "RAS on %s terminated weirdly.\n", chan->name);
				}
			}
			/* Throw back into audio mode */
			x = 1;
			ioctl(chan->fds[0], ZT_AUDIOMODE, &x);

			/* Restore saved values */
			res = ioctl(chan->fds[0], ZT_SET_BUFINFO, &savebi);
			if (res < 0) {
				ast_log(LOG_WARNING, "Unable to set buffer policy on channel %s\n", chan->name);
			}
			break;
		}
	}
}

static int zapras_exec(struct ast_channel *chan, void *data)
{
	int res=-1;
	char *args;
	struct localuser *u;
	ZT_PARAMS ztp;

	if (!data) 
		data = "";

	LOCAL_USER_ADD(u);

	args = ast_strdupa(data);
	if (!args) {
		ast_log(LOG_ERROR, "Out of memory\n");
		LOCAL_USER_REMOVE(u);
		return -1;
	}
	
	/* Answer the channel if it's not up */
	if (chan->_state != AST_STATE_UP)
		ast_answer(chan);
	if (strcasecmp(chan->type, "Zap")) {
		/* If it's not a zap channel, we're done.  Wait a couple of
		   seconds and then hangup... */
		if (option_verbose > 1)
			ast_verbose(VERBOSE_PREFIX_2 "Channel %s is not a Zap channel\n", chan->name);
		sleep(2);
	} else {
		memset(&ztp, 0, sizeof(ztp));
		if (ioctl(chan->fds[0], ZT_GET_PARAMS, &ztp)) {
			ast_log(LOG_WARNING, "Unable to get zaptel parameters\n");
		} else if (ztp.sigtype != ZT_SIG_CLEAR) {
			if (option_verbose > 1)
				ast_verbose(VERBOSE_PREFIX_2 "Channel %s is not a clear channel\n", chan->name);
		} else {
			/* Everything should be okay.  Run PPP. */
			if (option_verbose > 2)
				ast_verbose(VERBOSE_PREFIX_3 "Starting RAS on %s\n", chan->name);
			/* Execute RAS */
			run_ras(chan, args);
		}
	}
	LOCAL_USER_REMOVE(u);
	return res;
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
	return ast_register_application(app, zapras_exec, synopsis, descrip);
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
