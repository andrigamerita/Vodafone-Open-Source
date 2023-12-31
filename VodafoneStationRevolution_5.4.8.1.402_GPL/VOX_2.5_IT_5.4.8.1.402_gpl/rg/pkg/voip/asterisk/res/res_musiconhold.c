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
 * \brief Routines implementing music on hold
 *
 * \arg See also \ref Config_moh
 * 
 */

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <dirent.h>
#ifdef ZAPATA_MOH
#ifdef __linux__
#include <linux/zaptel.h>
#else
#include <zaptel.h>
#endif /* __linux__ */
#endif
#include <unistd.h>
#include <sys/ioctl.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include "asterisk/lock.h"
#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/pbx.h"
#include "asterisk/options.h"
#include "asterisk/module.h"
#include "asterisk/translate.h"
#include "asterisk/say.h"
#include "asterisk/musiconhold.h"
#include "asterisk/config.h"
#include "asterisk/utils.h"
#include "asterisk/cli.h"

#define MAX_MOHFILES 512
#define MAX_MOHFILE_LEN 128

static char *app0 = "MusicOnHold";
static char *app1 = "WaitMusicOnHold";
static char *app2 = "SetMusicOnHold";
static char *app3 = "StartMusicOnHold";
static char *app4 = "StopMusicOnHold";

static char *synopsis0 = "Play Music On Hold indefinitely";
static char *synopsis1 = "Wait, playing Music On Hold";
static char *synopsis2 = "Set default Music On Hold class";
static char *synopsis3 = "Play Music On Hold";
static char *synopsis4 = "Stop Playing Music On Hold";

static char *descrip0 = "MusicOnHold(class): "
"Plays hold music specified by class.  If omitted, the default\n"
"music source for the channel will be used. Set the default \n"
"class with the SetMusicOnHold() application.\n"
"Returns -1 on hangup.\n"
"Never returns otherwise.\n";

static char *descrip1 = "WaitMusicOnHold(delay): "
"Plays hold music specified number of seconds.  Returns 0 when\n"
"done, or -1 on hangup.  If no hold music is available, the delay will\n"
"still occur with no sound.\n";

static char *descrip2 = "SetMusicOnHold(class): "
"Sets the default class for music on hold for a given channel.  When\n"
"music on hold is activated, this class will be used to select which\n"
"music is played.\n";

static char *descrip3 = "StartMusicOnHold(class): "
"Starts playing music on hold, uses default music class for channel.\n"
"Starts playing music specified by class.  If omitted, the default\n"
"music source for the channel will be used.  Always returns 0.\n";

static char *descrip4 = "StopMusicOnHold: "
"Stops playing music on hold.\n";

static int respawn_time = 20;

struct moh_files_state {
	struct mohclass *class;
	int origwfmt;
	int samples;
	unsigned char pos;
	unsigned char save_pos;
	struct ast_frame *pending_frame;
};

#define MOH_QUIET		(1 << 0)
#define MOH_SINGLE		(1 << 1)
#define MOH_CUSTOM		(1 << 2)
#define MOH_RANDOMIZE		(1 << 3)

struct mohclass {
	char name[MAX_MUSICCLASS];
	char dir[256];
	char args[256];
	char mode[80];
	char filearray[MAX_MOHFILES][MAX_MOHFILE_LEN];
	unsigned int flags;
	int total_files;
	int format;
	int pid;		/* PID of mpg123 */
	time_t start;
	pthread_t thread;
	struct mohdata *members;
	/* Source of audio */
	int srcfd;
	/* FD for timing source */
	int pseudofd;
	struct mohclass *next;
};

struct mohdata {
	int pipe[2];
	int origwfmt;
	struct mohclass *parent;
	struct mohdata *next;
};

static struct mohclass *mohclasses;

AST_MUTEX_DEFINE_STATIC(moh_lock);

#define LOCAL_MPG_123 "/usr/local/bin/mpg123"
#define MPG_123 "/usr/bin/mpg123"
#define MAX_MP3S 256


static void ast_moh_free_class(struct mohclass **class) 
{
	struct mohdata *members, *mtmp;
	
	members = (*class)->members;
	while(members) {
		mtmp = members;
		members = members->next;
		free(mtmp);
	}
	free(*class);
	*class = NULL;
}


static void moh_files_release(struct ast_channel *chan, void *data)
{
	struct moh_files_state *state = chan->music_state;

	if (chan && state) {
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "Stopped music on hold on %s\n", chan->name);

		if (state->origwfmt && ast_set_write_format(chan, state->origwfmt)) {
			ast_log(LOG_WARNING, "Unable to restore channel '%s' to format '%d'\n", chan->name, state->origwfmt);
		}
		state->save_pos = state->pos + 1;
	}
}


static int ast_moh_files_next(struct ast_channel *chan) 
{
	struct moh_files_state *state = chan->music_state;
	int tries;

	if (state->save_pos) {
		state->pos = state->save_pos - 1;
		state->save_pos = 0;
	} else {
		/* Try 20 times to find something good */
		for (tries=0;tries < 20;tries++) {
			state->samples = 0;
			if (chan->stream) {
				ast_closestream(chan->stream);
				chan->stream = NULL;
				state->pos++;
			}

			if (ast_test_flag(state->class, MOH_RANDOMIZE))
				state->pos = rand();

			/* check to see if this file's format can be opened */
			if (ast_fileexists(state->class->filearray[state->pos], NULL, NULL) != -1)
				break;

		}
	}

	state->pos = state->pos % state->class->total_files;
	
	if (ast_set_write_format(chan, AST_FORMAT_SLINEAR)) {
		ast_log(LOG_WARNING, "Unable to set '%s' to linear format (write)\n", chan->name);
		return -1;
	}
	if (!ast_openstream_full(chan, state->class->filearray[state->pos], chan->language, 1)) {
		ast_log(LOG_WARNING, "Unable to open file '%s': %s\n", state->class->filearray[state->pos], strerror(errno));
		state->pos++;
		return -1;
	}

	if (option_debug)
		ast_log(LOG_DEBUG, "%s Opened file %d '%s'\n", chan->name, state->pos, state->class->filearray[state->pos]);

	if (state->samples)
		ast_seekstream(chan->stream, state->samples, SEEK_SET);

	return 0;
}


static struct ast_frame *moh_files_readframe(struct ast_channel *chan) 
{
	struct ast_frame *f = NULL;
	
	if (!(chan->stream && (f = ast_readframe(chan->stream)))) {
		if (!ast_moh_files_next(chan))
			f = ast_readframe(chan->stream);
	}

	return f;
}

static int moh_files_generator(struct ast_channel *chan, void *data, int len, int samples)
{
	struct moh_files_state *state = chan->music_state;
	struct ast_frame *f = NULL;
	int res = 0;

	while (1) {
		if (state->pending_frame)
			f = state->pending_frame;
		else
		    	f = moh_files_readframe(chan);
		if (f) {
			errno = 0;
			res = ast_write(chan, f);
			if (res < 0) {
				ast_log(LOG_WARNING, "Failed to write frame to '%s': %s\n", chan->name, strerror(errno));
				return -1;
			}
			if (errno == EAGAIN)
			{
			    	state->pending_frame = f;
				break;
			}
			else {
			    	state->samples += f->samples;
				state->pending_frame = NULL;
				ast_frfree(f);
			}
		} else
			return -1;	
	}
	return res;
}


static void *moh_files_alloc(struct ast_channel *chan, void *params)
{
	struct moh_files_state *state;
	struct mohclass *class = params;
	int allocated = 0;

	if (!chan->music_state && (state = malloc(sizeof(struct moh_files_state)))) {
		chan->music_state = state;
		allocated = 1;
	} else 
		state = chan->music_state;

	if (state) {
		if (allocated || state->class != class) {
			/* initialize */
			memset(state, 0, sizeof(struct moh_files_state));
			state->class = class;
		}

		state->origwfmt = chan->writeformat;

		if (ast_set_write_format(chan, AST_FORMAT_SLINEAR)) {
			ast_log(LOG_WARNING, "Unable to set '%s' to linear format (write)\n", chan->name);
			free(chan->music_state);
			chan->music_state = NULL;
		} else {
			if (option_verbose > 2)
				ast_verbose(VERBOSE_PREFIX_3 "Started music on hold, class '%s', on %s\n", class->name, chan->name);
		}
	}
	
	return chan->music_state;
}

static struct ast_generator moh_file_stream = 
{
	alloc: moh_files_alloc,
	release: moh_files_release,
	generate: moh_files_generator,
};

static int spawn_mp3(struct mohclass *class)
{
	int fds[2];
	int files = 0;
	char fns[MAX_MP3S][80];
	char *argv[MAX_MP3S + 50];
	char xargs[256];
	char *argptr;
	int argc = 0;
	DIR *dir = NULL;
	struct dirent *de;

	
	if (!strcasecmp(class->dir, "nodir")) {
		files = 1;
	} else {
		dir = opendir(class->dir);
		if (!dir && !strstr(class->dir,"http://") && !strstr(class->dir,"HTTP://")) {
			ast_log(LOG_WARNING, "%s is not a valid directory\n", class->dir);
			return -1;
		}
	}

	if (!ast_test_flag(class, MOH_CUSTOM)) {
		argv[argc++] = "mpg123";
		argv[argc++] = "-q";
		argv[argc++] = "-s";
		argv[argc++] = "--mono";
		argv[argc++] = "-r";
		argv[argc++] = "8000";
		
		if (!ast_test_flag(class, MOH_SINGLE)) {
			argv[argc++] = "-b";
			argv[argc++] = "2048";
		}
		
		argv[argc++] = "-f";
		
		if (ast_test_flag(class, MOH_QUIET))
			argv[argc++] = "4096";
		else
			argv[argc++] = "8192";
		
		/* Look for extra arguments and add them to the list */
		strncpy(xargs, class->args, sizeof(xargs) - 1);
		argptr = xargs;
		while (!ast_strlen_zero(argptr)) {
			argv[argc++] = argptr;
			argptr = strchr(argptr, ',');
			if (argptr) {
				*argptr = '\0';
				argptr++;
			}
		}
	} else  {
		/* Format arguments for argv vector */
		strncpy(xargs, class->args, sizeof(xargs) - 1);
		argptr = xargs;
		while (!ast_strlen_zero(argptr)) {
			argv[argc++] = argptr;
			argptr = strchr(argptr, ' ');
			if (argptr) {
				*argptr = '\0';
				argptr++;
			}
		}
	}


	if (strstr(class->dir,"http://") || strstr(class->dir,"HTTP://")) {
		strncpy(fns[files], class->dir, sizeof(fns[files]) - 1);
		argv[argc++] = fns[files];
		files++;
	} else if (dir) {
		while ((de = readdir(dir)) && (files < MAX_MP3S)) {
			if ((strlen(de->d_name) > 3) && 
			    ((ast_test_flag(class, MOH_CUSTOM) && 
			      (!strcasecmp(de->d_name + strlen(de->d_name) - 4, ".raw") || 
			       !strcasecmp(de->d_name + strlen(de->d_name) - 4, ".sln"))) ||
			     !strcasecmp(de->d_name + strlen(de->d_name) - 4, ".mp3"))) {
				strncpy(fns[files], de->d_name, sizeof(fns[files]) - 1);
				argv[argc++] = fns[files];
				files++;
			}
		}
	}
	argv[argc] = NULL;
	if (dir) {
		closedir(dir);
	}
	if (pipe(fds)) {	
		ast_log(LOG_WARNING, "Pipe failed\n");
		return -1;
	}
#if 0
	printf("%d files total, %d args total\n", files, argc);
	{
		int x;
		for (x=0;argv[x];x++)
			printf("arg%d: %s\n", x, argv[x]);
	}
#endif	
	if (!files) {
		ast_log(LOG_WARNING, "Found no files in '%s'\n", class->dir);
		close(fds[0]);
		close(fds[1]);
		return -1;
	}
	if (time(NULL) - class->start < respawn_time) {
		sleep(respawn_time - (time(NULL) - class->start));
	}
	time(&class->start);
	class->pid = fork();
	if (class->pid < 0) {
		close(fds[0]);
		close(fds[1]);
		ast_log(LOG_WARNING, "Fork failed: %s\n", strerror(errno));
		return -1;
	}
	if (!class->pid) {
		int x;
		close(fds[0]);
		/* Stdout goes to pipe */
		dup2(fds[1], STDOUT_FILENO);
		/* Close unused file descriptors */
		for (x=3;x<8192;x++) {
			if (-1 != fcntl(x, F_GETFL)) {
				close(x);
			}
		}
		/* Child */
		chdir(class->dir);
		if (ast_test_flag(class, MOH_CUSTOM)) {
			execv(argv[0], argv);
		} else {
			/* Default install is /usr/local/bin */
			execv(LOCAL_MPG_123, argv);
			/* Many places have it in /usr/bin */
			execv(MPG_123, argv);
			/* Check PATH as a last-ditch effort */
			execvp("mpg123", argv);
		}
		ast_log(LOG_WARNING, "Exec failed: %s\n", strerror(errno));
		close(fds[1]);
		exit(1);
	} else {
		/* Parent */
		close(fds[1]);
	}
	return fds[0];
}

static void *monmp3thread(void *data)
{
#define	MOH_MS_INTERVAL		100

	struct mohclass *class = data;
	struct mohdata *moh;
	char buf[8192];
	short sbuf[8192];
	int res, res2;
	int len;
	struct timeval tv, tv_tmp;

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	for(;/* ever */;) {
		/* Spawn mp3 player if it's not there */
		if (class->srcfd < 0) {
			if ((class->srcfd = spawn_mp3(class)) < 0) {
				ast_log(LOG_WARNING, "Unable to spawn mp3player\n");
				/* Try again later */
				sleep(500);
			}
		}
		if (class->pseudofd > -1) {
			/* Pause some amount of time */
			res = read(class->pseudofd, buf, sizeof(buf));
		} else {
			long delta;
			/* Reliable sleep */
			tv_tmp = ast_tvfromboot();
			if (ast_tvzero(tv))
				tv = tv_tmp;
			delta = ast_tvdiff_ms(tv_tmp, tv);
			if (delta < MOH_MS_INTERVAL) {	/* too early */
				tv = ast_tvadd(tv, ast_samp2tv(MOH_MS_INTERVAL, 1000));	/* next deadline */
				usleep(1000 * (MOH_MS_INTERVAL - delta));
			} else {
				ast_log(LOG_NOTICE, "Request to schedule in the past?!?!\n");
				tv = tv_tmp;
			}
			res = 8 * MOH_MS_INTERVAL;	/* 8 samples per millisecond */
		}
		if (!class->members)
			continue;
		/* Read mp3 audio */
		len = ast_codec_get_len(class->format, res);
		
		if ((res2 = read(class->srcfd, sbuf, len)) != len) {
			if (!res2) {
				close(class->srcfd);
				class->srcfd = -1;
				if (class->pid) {
					kill(class->pid, SIGKILL);
					class->pid = 0;
				}
			} else
				ast_log(LOG_DEBUG, "Read %d bytes of audio while expecting %d\n", res2, len);
			continue;
		}
		ast_mutex_lock(&moh_lock);
		moh = class->members;
		while (moh) {
			/* Write data */
			if ((res = write(moh->pipe[1], sbuf, res2)) != res2) 
				if (option_debug)
					ast_log(LOG_DEBUG, "Only wrote %d of %d bytes to pipe\n", res, res2);
			moh = moh->next;
		}
		ast_mutex_unlock(&moh_lock);
	}
	return NULL;
}

static int moh0_exec(struct ast_channel *chan, void *data)
{
	if (ast_moh_start(chan, data)) {
		ast_log(LOG_WARNING, "Unable to start music on hold (class '%s') on channel %s\n", (char *)data, chan->name);
		return -1;
	}
	while (!ast_safe_sleep(chan, 10000));
	ast_moh_stop(chan);
	return -1;
}

static int moh1_exec(struct ast_channel *chan, void *data)
{
	int res;
	if (!data || !atoi(data)) {
		ast_log(LOG_WARNING, "WaitMusicOnHold requires an argument (number of seconds to wait)\n");
		return -1;
	}
	if (ast_moh_start(chan, NULL)) {
		ast_log(LOG_WARNING, "Unable to start music on hold for %d seconds on channel %s\n", atoi(data), chan->name);
		return -1;
	}
	res = ast_safe_sleep(chan, atoi(data) * 1000);
	ast_moh_stop(chan);
	return res;
}

static int moh2_exec(struct ast_channel *chan, void *data)
{
	if (ast_strlen_zero(data)) {
		ast_log(LOG_WARNING, "SetMusicOnHold requires an argument (class)\n");
		return -1;
	}
	strncpy(chan->musicclass, data, sizeof(chan->musicclass) - 1);
	return 0;
}

static int moh3_exec(struct ast_channel *chan, void *data)
{
	char *class = NULL;
	if (data && strlen(data))
		class = data;
	if (ast_moh_start(chan, class)) 
		ast_log(LOG_NOTICE, "Unable to start music on hold class '%s' on channel %s\n", class ? class : "default", chan->name);

	return 0;
}

static int moh4_exec(struct ast_channel *chan, void *data)
{
	ast_moh_stop(chan);

	return 0;
}

static struct mohclass *get_mohbyname(char *name)
{
	struct mohclass *moh;
	moh = mohclasses;
	while (moh) {
		if (!strcasecmp(name, moh->name))
			return moh;
		moh = moh->next;
	}
	return NULL;
}

static struct mohdata *mohalloc(struct mohclass *cl)
{
	struct mohdata *moh;
	long flags;
	moh = malloc(sizeof(struct mohdata));
	if (!moh)
		return NULL;
	memset(moh, 0, sizeof(struct mohdata));
	if (pipe(moh->pipe)) {
		ast_log(LOG_WARNING, "Failed to create pipe: %s\n", strerror(errno));
		free(moh);
		return NULL;
	}
	/* Make entirely non-blocking */
	flags = fcntl(moh->pipe[0], F_GETFL);
	fcntl(moh->pipe[0], F_SETFL, flags | O_NONBLOCK);
	flags = fcntl(moh->pipe[1], F_GETFL);
	fcntl(moh->pipe[1], F_SETFL, flags | O_NONBLOCK);
	moh->parent = cl;
	moh->next = cl->members;
	cl->members = moh;
	return moh;
}

static void moh_release(struct ast_channel *chan, void *data)
{
	struct mohdata *moh = data, *prev, *cur;
	int oldwfmt;
	ast_mutex_lock(&moh_lock);
	/* Unlink */
	prev = NULL;
	cur = moh->parent->members;
	while (cur) {
		if (cur == moh) {
			if (prev)
				prev->next = cur->next;
			else
				moh->parent->members = cur->next;
			break;
		}
		prev = cur;
		cur = cur->next;
	}
	ast_mutex_unlock(&moh_lock);
	close(moh->pipe[0]);
	close(moh->pipe[1]);
	oldwfmt = moh->origwfmt;
	free(moh);
	if (chan) {
		if (oldwfmt && ast_set_write_format(chan, oldwfmt)) 
			ast_log(LOG_WARNING, "Unable to restore channel '%s' to format %s\n", chan->name, ast_getformatname(oldwfmt));
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "Stopped music on hold on %s\n", chan->name);
	}
}

static void *moh_alloc(struct ast_channel *chan, void *params)
{
	struct mohdata *res;
	struct mohclass *class = params;

	res = mohalloc(class);
	if (res) {
		res->origwfmt = chan->writeformat;
		if (ast_set_write_format(chan, class->format)) {
			ast_log(LOG_WARNING, "Unable to set channel '%s' to format '%s'\n", chan->name, ast_codec2str(class->format));
			moh_release(NULL, res);
			res = NULL;
		}
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "Started music on hold, class '%s', on channel '%s'\n", class->name, chan->name);
	}
	return res;
}

static int moh_generate(struct ast_channel *chan, void *data, int len, int samples)
{
	struct ast_frame f;
	struct mohdata *moh = data;
	short buf[1280 + AST_FRIENDLY_OFFSET / 2];
	int res;

	if (!moh->parent->pid)
		return -1;

	len = ast_codec_get_len(moh->parent->format, samples);

	if (len > sizeof(buf) - AST_FRIENDLY_OFFSET) {
		ast_log(LOG_WARNING, "Only doing %d of %d requested bytes on %s\n", (int)sizeof(buf), len, chan->name);
		len = sizeof(buf) - AST_FRIENDLY_OFFSET;
	}
	res = read(moh->pipe[0], buf + AST_FRIENDLY_OFFSET/2, len);
#if 0
	if (res != len) {
		ast_log(LOG_WARNING, "Read only %d of %d bytes: %s\n", res, len, strerror(errno));
	}
#endif
	if (res <= 0)
		return 0;

	memset(&f, 0, sizeof(f));
	
	f.frametype = AST_FRAME_VOICE;
	f.subclass = moh->parent->format;
	f.mallocd = 0;
	f.datalen = res;
	f.data = buf + AST_FRIENDLY_OFFSET / 2;
	f.offset = AST_FRIENDLY_OFFSET;
	f.samples = ast_codec_get_samples(&f);

	if (ast_write(chan, &f) < 0) {
		ast_log(LOG_WARNING, "Failed to write frame to '%s': %s\n", chan->name, strerror(errno));
		return -1;
	}

	return 0;
}

static struct ast_generator mohgen = 
{
	alloc: moh_alloc,
	release: moh_release,
	generate: moh_generate,
};

static int moh_scan_files(struct mohclass *class) {

	DIR *files_DIR;
	struct dirent *files_dirent;
	char path[512];
	char filepath[MAX_MOHFILE_LEN];
	char *ext;
	struct stat statbuf;
	int dirnamelen;
	int i;
	
	files_DIR = opendir(class->dir);
	if (!files_DIR) {
		ast_log(LOG_WARNING, "Cannot open dir %s or dir does not exist", class->dir);
		return -1;
	}

	class->total_files = 0;
	dirnamelen = strlen(class->dir) + 2;
	getcwd(path, 512);
	chdir(class->dir);
	memset(class->filearray, 0, MAX_MOHFILES*MAX_MOHFILE_LEN);
	while ((files_dirent = readdir(files_DIR))) {
		if ((strlen(files_dirent->d_name) < 4) || ((strlen(files_dirent->d_name) + dirnamelen) >= MAX_MOHFILE_LEN))
			continue;

		snprintf(filepath, MAX_MOHFILE_LEN, "%s/%s", class->dir, files_dirent->d_name);

		if (stat(filepath, &statbuf))
			continue;

		if (!S_ISREG(statbuf.st_mode))
			continue;

		if ((ext = strrchr(filepath, '.'))) {
			*ext = '\0';
			ext++;
		}

		/* if the file is present in multiple formats, ensure we only put it into the list once */
		for (i = 0; i < class->total_files; i++)
			if (!strcmp(filepath, class->filearray[i]))
				break;

		if (i == class->total_files)
			strcpy(class->filearray[class->total_files++], filepath);
	}

	closedir(files_DIR);
	chdir(path);
	return class->total_files;
}

static int moh_register(struct mohclass *moh, int reload)
{
#ifdef ZAPATA_MOH
	int x;
#endif
	ast_mutex_lock(&moh_lock);
	if (get_mohbyname(moh->name)) {
		if (reload) {
			ast_log(LOG_DEBUG, "Music on Hold class '%s' left alone from initial load.\n", moh->name);
		} else {
			ast_log(LOG_WARNING, "Music on Hold class '%s' already exists\n", moh->name);
		}
		free(moh);	
		ast_mutex_unlock(&moh_lock);
		return -1;
	}
	ast_mutex_unlock(&moh_lock);

	time(&moh->start);
	moh->start -= respawn_time;
	
	if (!strcasecmp(moh->mode, "files")) {
		if (!moh_scan_files(moh)) {
			ast_moh_free_class(&moh);
			return -1;
		}
		if (strchr(moh->args, 'r'))
			ast_set_flag(moh, MOH_RANDOMIZE);
	} else if (!strcasecmp(moh->mode, "mp3") || !strcasecmp(moh->mode, "mp3nb") || !strcasecmp(moh->mode, "quietmp3") || !strcasecmp(moh->mode, "quietmp3nb") || !strcasecmp(moh->mode, "httpmp3") || !strcasecmp(moh->mode, "custom")) {

		if (!strcasecmp(moh->mode, "custom"))
			ast_set_flag(moh, MOH_CUSTOM);
		else if (!strcasecmp(moh->mode, "mp3nb"))
			ast_set_flag(moh, MOH_SINGLE);
		else if (!strcasecmp(moh->mode, "quietmp3nb"))
			ast_set_flag(moh, MOH_SINGLE | MOH_QUIET);
		else if (!strcasecmp(moh->mode, "quietmp3"))
			ast_set_flag(moh, MOH_QUIET);
		
		moh->srcfd = -1;
#ifdef ZAPATA_MOH
		/* Open /dev/zap/pseudo for timing...  Is
		   there a better, yet reliable way to do this? */
		moh->pseudofd = open("/dev/zap/pseudo", O_RDONLY);
		if (moh->pseudofd < 0) {
			ast_log(LOG_WARNING, "Unable to open pseudo channel for timing...  Sound may be choppy.\n");
		} else {
			x = 320;
			ioctl(moh->pseudofd, ZT_SET_BLOCKSIZE, &x);
		}
#else
		moh->pseudofd = -1;
#endif
		if (ast_pthread_create(&moh->thread, NULL, monmp3thread, moh)) {
			ast_log(LOG_WARNING, "Unable to create moh...\n");
			if (moh->pseudofd > -1)
				close(moh->pseudofd);
			ast_moh_free_class(&moh);
			return -1;
		}
	} else {
		ast_log(LOG_WARNING, "Don't know how to do a mode '%s' music on hold\n", moh->mode);
		ast_moh_free_class(&moh);
		return -1;
	}
	ast_mutex_lock(&moh_lock);
	moh->next = mohclasses;
	mohclasses = moh;
	ast_mutex_unlock(&moh_lock);
	return 0;
}

static void local_ast_moh_cleanup(struct ast_channel *chan)
{
	if (chan->music_state) {
		free(chan->music_state);
		chan->music_state = NULL;
	}
}

static int local_ast_moh_start(struct ast_channel *chan, char *class)
{
	struct mohclass *mohclass;

	if (ast_strlen_zero(class))
		class = chan->musicclass;
	if (ast_strlen_zero(class))
		class = "default";
	ast_mutex_lock(&moh_lock);
	mohclass = get_mohbyname(class);
	ast_mutex_unlock(&moh_lock);

	if (!mohclass) {
		ast_log(LOG_WARNING, "No class: %s\n", (char *)class);
		return -1;
	}

	ast_set_flag(chan, AST_FLAG_MOH);
	if (mohclass->total_files) {
		return ast_activate_generator(chan, &moh_file_stream, mohclass);
	} else
		return ast_activate_generator(chan, &mohgen, mohclass);
}

static void local_ast_moh_stop(struct ast_channel *chan)
{
	ast_clear_flag(chan, AST_FLAG_MOH);
	ast_deactivate_generator(chan);

	if (chan->music_state) {
	    	struct moh_files_state *state = chan->music_state;

		if (state->pending_frame) {
			ast_frfree(state->pending_frame);
			state->pending_frame = NULL;
		}

		if (chan->stream) {
			ast_closestream(chan->stream);
			chan->stream = NULL;
		}
	}
}

static struct mohclass *moh_class_malloc(void)
{
	struct mohclass *class;

	class = malloc(sizeof(struct mohclass));

	if (!class)
		return NULL;

	memset(class, 0, sizeof(struct mohclass));

	class->format = AST_FORMAT_SLINEAR;

	return class;
}

static int load_moh_classes(int reload)
{
	struct ast_config *cfg;
	struct ast_variable *var;
	struct mohclass *class;	
	char *data;
	char *args;
	char *cat;
	int numclasses = 0;
	static int dep_warning = 0;

	cfg = ast_config_load("musiconhold.conf");

	if (!cfg)
		return 0;

	cat = ast_category_browse(cfg, NULL);
	for (; cat; cat = ast_category_browse(cfg, cat)) {
		if (strcasecmp(cat, "classes") && strcasecmp(cat, "moh_files")) {
			class = moh_class_malloc();
			if (!class) {
				ast_log(LOG_WARNING, "Out of memory!\n");
				break;
			}				
			ast_copy_string(class->name, cat, sizeof(class->name));	
			var = ast_variable_browse(cfg, cat);
			while (var) {
				if (!strcasecmp(var->name, "mode"))
					ast_copy_string(class->mode, var->value, sizeof(class->mode)); 
				else if (!strcasecmp(var->name, "directory"))
					ast_copy_string(class->dir, var->value, sizeof(class->dir));
				else if (!strcasecmp(var->name, "application"))
					ast_copy_string(class->args, var->value, sizeof(class->args));
				else if (!strcasecmp(var->name, "random"))
					ast_set2_flag(class, ast_true(var->value), MOH_RANDOMIZE);
				else if (!strcasecmp(var->name, "format")) {
					class->format = ast_getformatbyname(var->value);
					if (!class->format) {
						ast_log(LOG_WARNING, "Unknown format '%s' -- defaulting to SLIN\n", var->value);
						class->format = AST_FORMAT_SLINEAR;
					}
				}
					var = var->next;
			}

			if (ast_strlen_zero(class->dir)) {
				if (!strcasecmp(class->mode, "custom")) {
					strcpy(class->dir, "nodir");
				} else {
					ast_log(LOG_WARNING, "A directory must be specified for class '%s'!\n", class->name);
					free(class);
					continue;
				}
			}
			if (ast_strlen_zero(class->mode)) {
				ast_log(LOG_WARNING, "A mode must be specified for class '%s'!\n", class->name);
				free(class);
				continue;
			}
			if (ast_strlen_zero(class->args) && !strcasecmp(class->mode, "custom")) {
				ast_log(LOG_WARNING, "An application must be specified for class '%s'!\n", class->name);
				free(class);
				continue;
			}

			/* Don't leak a class when it's already registered */
			moh_register(class, reload);

			numclasses++;
		}
	}
	

	/* Deprecated Old-School Configuration */
	var = ast_variable_browse(cfg, "classes");
	while (var) {
		if (!dep_warning) {
			ast_log(LOG_WARNING, "The old musiconhold.conf syntax has been deprecated!  Please refer to the sample configuration for information on the new syntax.\n");
			dep_warning = 1;
		}
		data = strchr(var->value, ':');
		if (data) {
			*data++ = '\0';
			args = strchr(data, ',');
			if (args)
				*args++ = '\0';
			if (!(get_mohbyname(var->name))) {
				class = moh_class_malloc();
				if (!class) {
					ast_log(LOG_WARNING, "Out of memory!\n");
					return numclasses;
				}
				
				ast_copy_string(class->name, var->name, sizeof(class->name));
				ast_copy_string(class->dir, data, sizeof(class->dir));
				ast_copy_string(class->mode, var->value, sizeof(class->mode));
				if (args)
					ast_copy_string(class->args, args, sizeof(class->args));
				
				moh_register(class, reload);
				numclasses++;
			}
		}
		var = var->next;
	}
	var = ast_variable_browse(cfg, "moh_files");
	while (var) {
		if (!dep_warning) {
			ast_log(LOG_WARNING, "The old musiconhold.conf syntax has been deprecated!  Please refer to the sample configuration for information on the new syntax.\n");
			dep_warning = 1;
		}
		if (!(get_mohbyname(var->name))) {
			args = strchr(var->value, ',');
			if (args)
				*args++ = '\0';
			class = moh_class_malloc();
			if (!class) {
				ast_log(LOG_WARNING, "Out of memory!\n");
				return numclasses;
			}
			
			ast_copy_string(class->name, var->name, sizeof(class->name));
			ast_copy_string(class->dir, var->value, sizeof(class->dir));
			strcpy(class->mode, "files");
			if (args)	
				ast_copy_string(class->args, args, sizeof(class->args));
			
			moh_register(class, reload);
			numclasses++;
		}
		var = var->next;
	}

	ast_config_destroy(cfg);

	return numclasses;
}

static void ast_moh_destroy(void)
{
	struct mohclass *moh, *tmp;
	char buff[8192];
	int bytes, tbytes=0, stime = 0, pid = 0;

	if (option_verbose > 1)
		ast_verbose(VERBOSE_PREFIX_2 "Destroying musiconhold processes\n");
	ast_mutex_lock(&moh_lock);
	moh = mohclasses;

	while (moh) {
		if (moh->pid) {
			ast_log(LOG_DEBUG, "killing %d!\n", moh->pid);
			stime = time(NULL) + 2;
			pid = moh->pid;
			moh->pid = 0;
			kill(pid, SIGKILL);
			while ((ast_wait_for_input(moh->srcfd, 100) > 0) && (bytes = read(moh->srcfd, buff, 8192)) && time(NULL) < stime) {
				tbytes = tbytes + bytes;
			}
			ast_log(LOG_DEBUG, "mpg123 pid %d and child died after %d bytes read\n", pid, tbytes);
			close(moh->srcfd);
		}
		tmp = moh;
		moh = moh->next;
		ast_moh_free_class(&tmp);
	}
	mohclasses = NULL;
	ast_mutex_unlock(&moh_lock);
}

static void moh_on_off(int on)
{
	struct ast_channel *chan = NULL;

	while ( (chan = ast_channel_walk_locked(chan)) != NULL) {
		if (ast_test_flag(chan, AST_FLAG_MOH)) {
			if (on)
				local_ast_moh_start(chan, NULL);
			else
				ast_deactivate_generator(chan);
		}
		ast_mutex_unlock(&chan->lock);
	}
}

static int moh_cli(int fd, int argc, char *argv[]) 
{
	int x;

	moh_on_off(0);
	ast_moh_destroy();
	x = load_moh_classes(1);
	moh_on_off(1);
	ast_cli(fd, "\n%d class%s reloaded.\n", x, x == 1 ? "" : "es");
	return 0;
}

static int cli_files_show(int fd, int argc, char *argv[])
{
	int i;
	struct mohclass *class;

	ast_mutex_lock(&moh_lock);
	for (class = mohclasses; class; class = class->next) {
		if (!class->total_files)
			continue;

		ast_cli(fd, "Class: %s\n", class->name);
		for (i = 0; i < class->total_files; i++)
			ast_cli(fd, "\tFile: %s\n", class->filearray[i]);
	}
	ast_mutex_unlock(&moh_lock);

	return 0;
}

static int moh_classes_show(int fd, int argc, char *argv[])
{
	struct mohclass *class;

	ast_mutex_lock(&moh_lock);
	for (class = mohclasses; class; class = class->next) {
		ast_cli(fd, "Class: %s\n", class->name);
		ast_cli(fd, "\tMode: %s\n", ast_strlen_zero(class->mode) ? "<none>" : class->mode);
		ast_cli(fd, "\tDirectory: %s\n", ast_strlen_zero(class->dir) ? "<none>" : class->dir);
		if (ast_test_flag(class, MOH_CUSTOM))
			ast_cli(fd, "\tApplication: %s\n", ast_strlen_zero(class->args) ? "<none>" : class->args);
		ast_cli(fd, "\tFormat: %s\n", ast_getformatname(class->format));
	}
	ast_mutex_unlock(&moh_lock);

	return 0;
}

static struct ast_cli_entry  cli_moh = { { "moh", "reload"}, moh_cli, "Music On Hold", "Music On Hold", NULL};

static struct ast_cli_entry  cli_moh_classes_show = { { "moh", "classes", "show"}, moh_classes_show, "List MOH classes", "Lists all MOH classes", NULL};

static struct ast_cli_entry  cli_moh_files_show = { { "moh", "files", "show"}, cli_files_show, "List MOH file-based classes", "Lists all loaded file-based MOH classes and their files", NULL};

static int init_classes(int reload) 
{
	struct mohclass *moh;
    
	if (!load_moh_classes(reload)) 		/* Load classes from config */
		return 0;			/* Return if nothing is found */
	moh = mohclasses;
	while (moh) {
		if (moh->total_files)
			moh_scan_files(moh);
		moh = moh->next;
	}
	return 1;
}

int load_module(void)
{
	int res;

	res = ast_register_application(app0, moh0_exec, synopsis0, descrip0);
	ast_register_atexit(ast_moh_destroy);
	ast_cli_register(&cli_moh);
	ast_cli_register(&cli_moh_files_show);
	ast_cli_register(&cli_moh_classes_show);
	if (!res)
		res = ast_register_application(app1, moh1_exec, synopsis1, descrip1);
	if (!res)
		res = ast_register_application(app2, moh2_exec, synopsis2, descrip2);
	if (!res)
		res = ast_register_application(app3, moh3_exec, synopsis3, descrip3);
	if (!res)
		res = ast_register_application(app4, moh4_exec, synopsis4, descrip4);

	if (!init_classes(0)) { 	/* No music classes configured, so skip it */
		ast_log(LOG_WARNING, "No music on hold classes configured, disabling music on hold.");
	} else {
		ast_install_music_functions(local_ast_moh_start, local_ast_moh_stop, local_ast_moh_cleanup);
	}

	return 0;
}

int reload(void)
{
	if (init_classes(1))
		ast_install_music_functions(local_ast_moh_start, local_ast_moh_stop, local_ast_moh_cleanup);

	return 0;
}

int unload_module(void)
{
	return -1;
}

char *description(void)
{
	return "Music On Hold Resource";
}

int usecount(void)
{
	/* Never allow Music On Hold to be unloaded
	   unresolve needed symbols in the dialer */
#if 0
	int res;
	STANDARD_USECOUNT(res);
	return res;
#else
	return 1;
#endif
}

char *key()
{
	return ASTERISK_GPL_KEY;
}
