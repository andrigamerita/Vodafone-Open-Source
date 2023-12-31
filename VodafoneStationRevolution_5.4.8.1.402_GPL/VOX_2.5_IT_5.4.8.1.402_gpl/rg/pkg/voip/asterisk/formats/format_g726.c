/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (c) 2004 - 2005, inAccess Networks
 *
 * Michael Manousos <manousos@inaccessnetworks.com>
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

/*!\file
 *
 * \brief Headerless G.726 (16/24/32/40kbps) data format for Asterisk.
 * 
 * File name extensions:
 * \arg 40 kbps: g726-40
 * \arg 32 kbps: g726-32
 * \arg 24 kbps: g726-24
 * \arg 16 kbps: g726-16
 * \ingroup formats
 */
 
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include "asterisk/lock.h"
#include "asterisk/options.h"
#include "asterisk/channel.h"
#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/sched.h"
#include "asterisk/module.h"
#include "asterisk/endian.h"

#define	RATE_40		0
#define	RATE_32		1
#define	RATE_24		2
#define	RATE_16		3

/* We can only read/write chunks of FRAME_TIME ms G.726 data */
#define	FRAME_TIME	10	/* 10 ms size */

/* Frame sizes in bytes */
static int frame_size[4] = { 
		FRAME_TIME * 5,
		FRAME_TIME * 4,
		FRAME_TIME * 3,
		FRAME_TIME * 2
};

struct ast_filestream {
	/* Do not place anything before "reserved" */
	void *reserved[AST_RESERVED_POINTERS];
	/* This is what a filestream means to us */
	FILE *f; 							/* Open file descriptor */
	int rate;							/* RATE_* defines */
	struct ast_frame fr;				/* Frame information */
	char waste[AST_FRIENDLY_OFFSET];	/* Buffer for sending frames, etc */
	char empty;							/* Empty character */
	unsigned char g726[FRAME_TIME * 5];	/* G.726 encoded voice */
};

AST_MUTEX_DEFINE_STATIC(g726_lock);
static int glistcnt = 0;

static char *desc = "Raw G.726 (16/24/32/40kbps) data";
static char *name40 = "g726-40";
static char *name32 = "g726-32";
static char *name24 = "g726-24";
static char *name16 = "g726-16";
static char *exts40 = "g726-40";
static char *exts32 = "g726-32";
static char *exts24 = "g726-24";
static char *exts16 = "g726-16";

/*
 * Rate dependant format functions (open, rewrite)
 */
static struct ast_filestream *g726_40_open(FILE *f)
{
	/* We don't have any header to read or anything really, but
	   if we did, it would go here.  We also might want to check
	   and be sure it's a valid file.  */
	struct ast_filestream *tmp;
	if ((tmp = malloc(sizeof(struct ast_filestream)))) {
		memset(tmp, 0, sizeof(struct ast_filestream));
		if (ast_mutex_lock(&g726_lock)) {
			ast_log(LOG_WARNING, "Unable to lock g726 list.\n");
			free(tmp);
			return NULL;
		}
		tmp->f = f;
		tmp->rate = RATE_40;
		tmp->fr.data = tmp->g726;
		tmp->fr.frametype = AST_FRAME_VOICE;
		tmp->fr.subclass = AST_FORMAT_G726;
		/* datalen will vary for each frame */
		tmp->fr.src = name40;
		tmp->fr.mallocd = 0;
		glistcnt++;
		if (option_debug)
			ast_log(LOG_DEBUG, "Created filestream G.726-%dk.\n", 
									40 - tmp->rate * 8);
		ast_mutex_unlock(&g726_lock);
		ast_update_use_count();
	}
	return tmp;
}

static struct ast_filestream *g726_32_open(FILE *f)
{
	/* We don't have any header to read or anything really, but
	   if we did, it would go here.  We also might want to check
	   and be sure it's a valid file.  */
	struct ast_filestream *tmp;
	if ((tmp = malloc(sizeof(struct ast_filestream)))) {
		memset(tmp, 0, sizeof(struct ast_filestream));
		if (ast_mutex_lock(&g726_lock)) {
			ast_log(LOG_WARNING, "Unable to lock g726 list.\n");
			free(tmp);
			return NULL;
		}
		tmp->f = f;
		tmp->rate = RATE_32;
		tmp->fr.data = tmp->g726;
		tmp->fr.frametype = AST_FRAME_VOICE;
		tmp->fr.subclass = AST_FORMAT_G726;
		/* datalen will vary for each frame */
		tmp->fr.src = name32;
		tmp->fr.mallocd = 0;
		glistcnt++;
		if (option_debug)
			ast_log(LOG_DEBUG, "Created filestream G.726-%dk.\n", 
									40 - tmp->rate * 8);
		ast_mutex_unlock(&g726_lock);
		ast_update_use_count();
	}
	return tmp;
}

static struct ast_filestream *g726_24_open(FILE *f)
{
	/* We don't have any header to read or anything really, but
	   if we did, it would go here.  We also might want to check
	   and be sure it's a valid file.  */
	struct ast_filestream *tmp;
	if ((tmp = malloc(sizeof(struct ast_filestream)))) {
		memset(tmp, 0, sizeof(struct ast_filestream));
		if (ast_mutex_lock(&g726_lock)) {
			ast_log(LOG_WARNING, "Unable to lock g726 list.\n");
			free(tmp);
			return NULL;
		}
		tmp->f = f;
		tmp->rate = RATE_24;
		tmp->fr.data = tmp->g726;
		tmp->fr.frametype = AST_FRAME_VOICE;
		tmp->fr.subclass = AST_FORMAT_G726;
		/* datalen will vary for each frame */
		tmp->fr.src = name24;
		tmp->fr.mallocd = 0;
		glistcnt++;
		if (option_debug)
			ast_log(LOG_DEBUG, "Created filestream G.726-%dk.\n", 
									40 - tmp->rate * 8);
		ast_mutex_unlock(&g726_lock);
		ast_update_use_count();
	}
	return tmp;
}

static struct ast_filestream *g726_16_open(FILE *f)
{
	/* We don't have any header to read or anything really, but
	   if we did, it would go here.  We also might want to check
	   and be sure it's a valid file.  */
	struct ast_filestream *tmp;
	if ((tmp = malloc(sizeof(struct ast_filestream)))) {
		memset(tmp, 0, sizeof(struct ast_filestream));
		if (ast_mutex_lock(&g726_lock)) {
			ast_log(LOG_WARNING, "Unable to lock g726 list.\n");
			free(tmp);
			return NULL;
		}
		tmp->f = f;
		tmp->rate = RATE_16;
		tmp->fr.data = tmp->g726;
		tmp->fr.frametype = AST_FRAME_VOICE;
		tmp->fr.subclass = AST_FORMAT_G726;
		/* datalen will vary for each frame */
		tmp->fr.src = name16;
		tmp->fr.mallocd = 0;
		glistcnt++;
		if (option_debug)
			ast_log(LOG_DEBUG, "Created filestream G.726-%dk.\n", 
									40 - tmp->rate * 8);
		ast_mutex_unlock(&g726_lock);
		ast_update_use_count();
	}
	return tmp;
}

static struct ast_filestream *g726_40_rewrite(FILE *f, const char *comment)
{
	/* We don't have any header to read or anything really, but
	   if we did, it would go here.  We also might want to check
	   and be sure it's a valid file.  */
	struct ast_filestream *tmp;
	if ((tmp = malloc(sizeof(struct ast_filestream)))) {
		memset(tmp, 0, sizeof(struct ast_filestream));
		if (ast_mutex_lock(&g726_lock)) {
			ast_log(LOG_WARNING, "Unable to lock g726 list.\n");
			free(tmp);
			return NULL;
		}
		tmp->f = f;
		tmp->rate = RATE_40;
		glistcnt++;
		if (option_debug)
			ast_log(LOG_DEBUG, "Created filestream G.726-%dk.\n", 
									40 - tmp->rate * 8);
		ast_mutex_unlock(&g726_lock);
		ast_update_use_count();
	} else
		ast_log(LOG_WARNING, "Out of memory\n");
	return tmp;
}

static struct ast_filestream *g726_32_rewrite(FILE *f, const char *comment)
{
	/* We don't have any header to read or anything really, but
	   if we did, it would go here.  We also might want to check
	   and be sure it's a valid file.  */
	struct ast_filestream *tmp;
	if ((tmp = malloc(sizeof(struct ast_filestream)))) {
		memset(tmp, 0, sizeof(struct ast_filestream));
		if (ast_mutex_lock(&g726_lock)) {
			ast_log(LOG_WARNING, "Unable to lock g726 list.\n");
			free(tmp);
			return NULL;
		}
		tmp->f = f;
		tmp->rate = RATE_32;
		glistcnt++;
		if (option_debug)
			ast_log(LOG_DEBUG, "Created filestream G.726-%dk.\n", 
									40 - tmp->rate * 8);
		ast_mutex_unlock(&g726_lock);
		ast_update_use_count();
	} else
		ast_log(LOG_WARNING, "Out of memory\n");
	return tmp;
}

static struct ast_filestream *g726_24_rewrite(FILE *f, const char *comment)
{
	/* We don't have any header to read or anything really, but
	   if we did, it would go here.  We also might want to check
	   and be sure it's a valid file.  */
	struct ast_filestream *tmp;
	if ((tmp = malloc(sizeof(struct ast_filestream)))) {
		memset(tmp, 0, sizeof(struct ast_filestream));
		if (ast_mutex_lock(&g726_lock)) {
			ast_log(LOG_WARNING, "Unable to lock g726 list.\n");
			free(tmp);
			return NULL;
		}
		tmp->f = f;
		tmp->rate = RATE_24;
		glistcnt++;
		if (option_debug)
			ast_log(LOG_DEBUG, "Created filestream G.726-%dk.\n", 
									40 - tmp->rate * 8);
		ast_mutex_unlock(&g726_lock);
		ast_update_use_count();
	} else
		ast_log(LOG_WARNING, "Out of memory\n");
	return tmp;
}

static struct ast_filestream *g726_16_rewrite(FILE *f, const char *comment)
{
	/* We don't have any header to read or anything really, but
	   if we did, it would go here.  We also might want to check
	   and be sure it's a valid file.  */
	struct ast_filestream *tmp;
	if ((tmp = malloc(sizeof(struct ast_filestream)))) {
		memset(tmp, 0, sizeof(struct ast_filestream));
		if (ast_mutex_lock(&g726_lock)) {
			ast_log(LOG_WARNING, "Unable to lock g726 list.\n");
			free(tmp);
			return NULL;
		}
		tmp->f = f;
		tmp->rate = RATE_16;
		glistcnt++;
		if (option_debug)
			ast_log(LOG_DEBUG, "Created filestream G.726-%dk.\n", 
									40 - tmp->rate * 8);
		ast_mutex_unlock(&g726_lock);
		ast_update_use_count();
	} else
		ast_log(LOG_WARNING, "Out of memory\n");
	return tmp;
}

/*
 * Rate independent format functions (close, read, write)
 */
static void g726_close(struct ast_filestream *s)
{
	if (ast_mutex_lock(&g726_lock)) {
		ast_log(LOG_WARNING, "Unable to lock g726 list.\n");
		return;
	}
	glistcnt--;
	if (option_debug)
		ast_log(LOG_DEBUG, "Closed filestream G.726-%dk.\n", 40 - s->rate * 8);
	ast_mutex_unlock(&g726_lock);
	ast_update_use_count();
	fclose(s->f);
	free(s);
	s = NULL;
}

static struct ast_frame *g726_read(struct ast_filestream *s, int *whennext)
{
	int res;
	/* Send a frame from the file to the appropriate channel */
	s->fr.frametype = AST_FRAME_VOICE;
	s->fr.subclass = AST_FORMAT_G726;
	s->fr.offset = AST_FRIENDLY_OFFSET;
	s->fr.samples = 8 * FRAME_TIME;
	s->fr.datalen = frame_size[s->rate];
	s->fr.mallocd = 0;
	s->fr.data = s->g726;
	if ((res = fread(s->g726, 1, s->fr.datalen, s->f)) != s->fr.datalen) {
		if (res)
			ast_log(LOG_WARNING, "Short read (%d) (%s)!\n", res, strerror(errno));
		return NULL;
	}
	*whennext = s->fr.samples;
	return &s->fr;
}

static int g726_write(struct ast_filestream *fs, struct ast_frame *f)
{
	int res;
	if (f->frametype != AST_FRAME_VOICE) {
		ast_log(LOG_WARNING, "Asked to write non-voice frame!\n");
		return -1;
	}
	if (f->subclass != AST_FORMAT_G726) {
		ast_log(LOG_WARNING, "Asked to write non-G726 frame (%d)!\n", 
						f->subclass);
		return -1;
	}
	if (f->datalen % frame_size[fs->rate]) {
		ast_log(LOG_WARNING, "Invalid data length %d, should be multiple of %d\n", 
						f->datalen, frame_size[fs->rate]);
		return -1;
	}
	if ((res = fwrite(f->data, 1, f->datalen, fs->f)) != f->datalen) {
			ast_log(LOG_WARNING, "Bad write (%d/%d): %s\n", 
							res, frame_size[fs->rate], strerror(errno));
			return -1;
	}
	return 0;
}

static char *g726_getcomment(struct ast_filestream *s)
{
	return NULL;
}

static int g726_seek(struct ast_filestream *fs, long sample_offset, int whence)
{
	return -1;
}

static int g726_trunc(struct ast_filestream *fs)
{
	return -1;
}

static long g726_tell(struct ast_filestream *fs)
{
	return -1;
}

/*
 * Module interface (load_module, unload_module, usecount, description, key)
 */
int load_module()
{
	int res;

	res = ast_format_register(name40, exts40, AST_FORMAT_G726,
								g726_40_open,
								g726_40_rewrite,
								g726_write,
								g726_seek,
								g726_trunc,
								g726_tell,
								g726_read,
								g726_close,
								g726_getcomment);
	if (res) {
		ast_log(LOG_WARNING, "Failed to register format %s.\n", name40);
		return(-1);
	}
	res = ast_format_register(name32, exts32, AST_FORMAT_G726,
								g726_32_open,
								g726_32_rewrite,
								g726_write,
								g726_seek,
								g726_trunc,
								g726_tell,
								g726_read,
								g726_close,
								g726_getcomment);
	if (res) {
		ast_log(LOG_WARNING, "Failed to register format %s.\n", name32);
		return(-1);
	}
	res = ast_format_register(name24, exts24, AST_FORMAT_G726,
								g726_24_open,
								g726_24_rewrite,
								g726_write,
								g726_seek,
								g726_trunc,
								g726_tell,
								g726_read,
								g726_close,
								g726_getcomment);
	if (res) {
		ast_log(LOG_WARNING, "Failed to register format %s.\n", name24);
		return(-1);
	}
	res = ast_format_register(name16, exts16, AST_FORMAT_G726,
								g726_16_open,
								g726_16_rewrite,
								g726_write,
								g726_seek,
								g726_trunc,
								g726_tell,
								g726_read,
								g726_close,
								g726_getcomment);
	if (res) {
		ast_log(LOG_WARNING, "Failed to register format %s.\n", name16);
		return(-1);
	}
	return(0);
}

int unload_module()
{
	int res;

	res = ast_format_unregister(name16);
	if (res) {
		ast_log(LOG_WARNING, "Failed to unregister format %s.\n", name16);
		return(-1);
	}
	res = ast_format_unregister(name24);
	if (res) {
		ast_log(LOG_WARNING, "Failed to unregister format %s.\n", name24);
		return(-1);
	}
	res = ast_format_unregister(name32);
	if (res) {
		ast_log(LOG_WARNING, "Failed to unregister format %s.\n", name32);
		return(-1);
	}
	res = ast_format_unregister(name40);
	if (res) {
		ast_log(LOG_WARNING, "Failed to unregister format %s.\n", name40);
		return(-1);
	}
	return(0);
}	

int usecount()
{
	return glistcnt;
}

char *description()
{
	return desc;
}

char *key()
{
	return ASTERISK_GPL_KEY;
}

