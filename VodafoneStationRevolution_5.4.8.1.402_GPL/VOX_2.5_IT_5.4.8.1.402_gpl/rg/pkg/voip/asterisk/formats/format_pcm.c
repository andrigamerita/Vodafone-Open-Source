/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2006, Digium, Inc.
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
 * \brief Flat, binary, ulaw PCM file format.
 * \arg File name extension: pcm, ulaw, ul, mu
 * 
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

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 1.6 $")

#include "asterisk/lock.h"
#include "asterisk/channel.h"
#include "asterisk/file.h"
#include "asterisk/logger.h"
#include "asterisk/sched.h"
#include "asterisk/module.h"
#include "asterisk/endian.h"
#include "asterisk/ulaw.h"

#define BUF_SIZE 160		/* 160 samples */

struct ast_filestream {
	void *reserved[AST_RESERVED_POINTERS];
	/* This is what a filestream means to us */
	FILE *f; /* Descriptor */
	struct ast_channel *owner;
	struct ast_frame fr;				/* Frame information */
	char waste[AST_FRIENDLY_OFFSET];	/* Buffer for sending frames, etc */
	char empty;							/* Empty character */
	unsigned char buf[BUF_SIZE];				/* Output Buffer */
	struct timeval last;
};


AST_MUTEX_DEFINE_STATIC(pcm_lock);
static int glistcnt = 0;

static char *name = "pcm";
static char *desc = "Raw uLaw 8khz Audio support (PCM)";
static char *exts = "pcm|ulaw|ul|mu";

static char ulaw_silence[BUF_SIZE];

static struct ast_filestream *_open(FILE *f, int subclass)
{
	/* We don't have any header to read or anything really, but
	   if we did, it would go here.  We also might want to check
	   and be sure it's a valid file.  */
	struct ast_filestream *tmp;
	if ((tmp = malloc(sizeof(struct ast_filestream)))) {
		memset(tmp, 0, sizeof(struct ast_filestream));
		if (ast_mutex_lock(&pcm_lock)) {
			ast_log(LOG_WARNING, "Unable to lock pcm list\n");
			free(tmp);
			return NULL;
		}
		tmp->f = f;
		tmp->fr.data = tmp->buf;
		tmp->fr.frametype = AST_FRAME_VOICE;
		tmp->fr.subclass = subclass;
		/* datalen will vary for each frame */
		tmp->fr.src = name;
		tmp->fr.mallocd = 0;
		glistcnt++;
		ast_mutex_unlock(&pcm_lock);
		ast_update_use_count();
	}
	return tmp;
}

static struct ast_filestream *pcm_open(FILE *f)
{
	return _open(f, AST_FORMAT_ULAW);
}

static struct ast_filestream *g722_open(FILE *f)
{
	return _open(f, AST_FORMAT_G722);
}

static struct ast_filestream *pcm_rewrite(FILE *f, const char *comment)
{
	/* We don't have any header to read or anything really, but
	   if we did, it would go here.  We also might want to check
	   and be sure it's a valid file.  */
	struct ast_filestream *tmp;
	if ((tmp = malloc(sizeof(struct ast_filestream)))) {
		memset(tmp, 0, sizeof(struct ast_filestream));
		if (ast_mutex_lock(&pcm_lock)) {
			ast_log(LOG_WARNING, "Unable to lock pcm list\n");
			free(tmp);
			return NULL;
		}
		tmp->f = f;
		glistcnt++;
		ast_mutex_unlock(&pcm_lock);
		ast_update_use_count();
	} else
		ast_log(LOG_WARNING, "Out of memory\n");
	return tmp;
}

static void pcm_close(struct ast_filestream *s)
{
	if (ast_mutex_lock(&pcm_lock)) {
		ast_log(LOG_WARNING, "Unable to lock pcm list\n");
		return;
	}
	glistcnt--;
	ast_mutex_unlock(&pcm_lock);
	ast_update_use_count();
	fclose(s->f);
	free(s);
	s = NULL;
}

static struct ast_frame *_read(struct ast_filestream *s, int *whennext, int subclass)
{
	int res;
	int delay;
	/* Send a frame from the file to the appropriate channel */

	s->fr.frametype = AST_FRAME_VOICE;
	s->fr.subclass = subclass;
	s->fr.offset = AST_FRIENDLY_OFFSET;
	s->fr.mallocd = 0;
	s->fr.data = s->buf;
	if ((res = fread(s->buf, 1, BUF_SIZE, s->f)) < 1) {
		if (res)
			ast_log(LOG_WARNING, "Short read (%d) (%s)!\n", res, strerror(errno));
		return NULL;
	}
	s->fr.samples = res;
	s->fr.datalen = res;
	delay = s->fr.samples;
	*whennext = delay;
	return &s->fr;
}

static struct ast_frame *pcm_read(struct ast_filestream *s, int *whennext)
{
	return _read(s, whennext, AST_FORMAT_ULAW);
}

static struct ast_frame *g722_read(struct ast_filestream *s, int *whennext)
{
	return _read(s, whennext, AST_FORMAT_G722);
}

static int _write(struct ast_filestream *fs, struct ast_frame *f, int subclass)
{
	int res;
	if (f->frametype != AST_FRAME_VOICE) {
		ast_log(LOG_WARNING, "Asked to write non-voice frame!\n");
		return -1;
	}
	if ((res = fwrite(f->data, 1, f->datalen, fs->f)) != f->datalen) {
			ast_log(LOG_WARNING, "Bad write (%d/%d): %s\n", res, f->datalen, strerror(errno));
			return -1;
	}
	return 0;
}

static int pcm_write(struct ast_filestream *fs, struct ast_frame *f)
{
	if (f->subclass != AST_FORMAT_ULAW) {
		ast_log(LOG_WARNING, "Asked to write non-ulaw frame (%d)!\n", f->subclass);
		return -1;
	}

	return _write(fs, f, AST_FORMAT_ULAW);
}

static int g722_write(struct ast_filestream *fs, struct ast_frame *f)
{
	if (f->subclass != AST_FORMAT_G722) {
		ast_log(LOG_WARNING, "Asked to write non-g722 frame (%d)!\n", f->subclass);
		return -1;
	}

	return _write(fs, f, AST_FORMAT_G722);
}

static int pcm_seek(struct ast_filestream *fs, long sample_offset, int whence)
{
	long cur, max, offset = 0;

	cur = ftell(fs->f);
	fseek(fs->f, 0, SEEK_END);
	max = ftell(fs->f);

	switch (whence) {
	case SEEK_SET:
		offset = sample_offset;
		break;
	case SEEK_END:
		offset = max - sample_offset;
		break;
	case SEEK_CUR:
	case SEEK_FORCECUR:
		offset = cur + sample_offset;
		break;
	}

	switch (whence) {
	case SEEK_FORCECUR:
		if (offset > max) {
			size_t left = offset - max;
			size_t res;

			while (left) {
				res = fwrite(ulaw_silence, sizeof(ulaw_silence[0]),
					     (left > BUF_SIZE) ? BUF_SIZE : left, fs->f);
				if (res == -1)
					return res;
				left -= res * sizeof(ulaw_silence[0]);
			}
			return offset;
		}
		/* fall through */
	default:
		offset = (offset > max) ? max : offset;
		offset = (offset < 0) ? 0 : offset;
		return fseek(fs->f, offset, SEEK_SET);
	}
}

static int pcm_trunc(struct ast_filestream *fs)
{
	return ftruncate(fileno(fs->f), ftell(fs->f));
}

static long pcm_tell(struct ast_filestream *fs)
{
	off_t offset;
	offset = ftell(fs->f);
	return offset;
}

static char *pcm_getcomment(struct ast_filestream *s)
{
	return NULL;
}

int load_module()
{
	int index, res;

	for (index = 0; index < (sizeof(ulaw_silence) / sizeof(ulaw_silence[0])); index++)
		ulaw_silence[index] = AST_LIN2MU(0);

	res = ast_format_register("g722", "g722", AST_FORMAT_G722,
				  g722_open,
				  pcm_rewrite,
				  g722_write,
				  pcm_seek,
				  pcm_trunc,
				  pcm_tell,
				  g722_read,
				  pcm_close,
				  pcm_getcomment);
	if (res) {
		ast_log(LOG_WARNING, "Failed to register format G.722.\n");
		return -1;
	}
	return ast_format_register(name, exts, AST_FORMAT_ULAW,
				   pcm_open,
				   pcm_rewrite,
				   pcm_write,
				   pcm_seek,
				   pcm_trunc,
				   pcm_tell,
				   pcm_read,
				   pcm_close,
				   pcm_getcomment);
}

int unload_module()
{
	int res;

	res = ast_format_unregister("g722");
	if (res) {
		ast_log(LOG_WARNING, "Failed to unregister format G.722\n");
		return -1;
	}
	return ast_format_unregister(name);
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
