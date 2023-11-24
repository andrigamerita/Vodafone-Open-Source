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
 * \brief codec_slin.c - translate between slin and slin16 directly
 *
 * \ingroup codecs
 */

#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include "asterisk/lock.h"
#include "asterisk/logger.h"
#include "asterisk/module.h"
#include "asterisk/translate.h"
#include "asterisk/channel.h"
#include "slin_ulaw_ex.h"

#define BUFFER_SIZE8	8000	/* size for the translation buffers */
#define BUFFER_SIZE16	16000	/* size for the translation buffers */

AST_MUTEX_DEFINE_STATIC(localuser_lock);
static int localusecnt = 0;

static char *tdesc = "slin8 and slin16 direct Coder/Decoder";

struct lin8lin16_pvt {
    struct ast_frame f;
    char offset[AST_FRIENDLY_OFFSET];
    short outbuf[BUFFER_SIZE16];
    int tail;
};

struct lin16lin8_pvt {
    struct ast_frame f;
    char offset[AST_FRIENDLY_OFFSET];
    short outbuf[BUFFER_SIZE8];
    int tail;
};

static struct ast_translator_pvt *lin8to16_new(void)
{
    struct lin8lin16_pvt *tmp;
    tmp = malloc(sizeof(struct lin8lin16_pvt));
    if (tmp)
    {
	memset(tmp, 0, sizeof(*tmp));
	localusecnt++;
	ast_update_use_count();
	tmp->tail = 0;
    }
    return (struct ast_translator_pvt *)tmp;
}

static struct ast_translator_pvt *lin16to8_new(void)
{
    struct lin16lin8_pvt *tmp;
    tmp = malloc(sizeof(struct lin16lin8_pvt));
    if (tmp)
    {
	memset(tmp, 0, sizeof(*tmp));
	localusecnt++;
	ast_update_use_count();
	tmp->tail = 0;
    }
    return (struct ast_translator_pvt *)tmp;
}


static int lin8to16_framein(struct ast_translator_pvt *pvt, struct ast_frame *f)
{
    struct lin8lin16_pvt *tmp = (struct lin8lin16_pvt *)pvt;
    int x, samples;
    short *s, *d;

    samples = ast_codec_get_samples(f);

    if (ast_codec_get_len(AST_FORMAT_SLINEAR16, tmp->tail) +
	ast_codec_get_len(AST_FORMAT_SLINEAR16, samples) * 2 >= sizeof(tmp->outbuf))
    {
	ast_log (LOG_WARNING, "Out of buffer space\n");
	return -1;
    }

    s = f->data;
    d = tmp->outbuf + tmp->tail;

    for (x=0; x < samples; x++)
    {
	*d = *s;
	d++;
	*d = *s;
	d++; s++;
    }
    tmp->tail += samples * 2;
    return 0;
}

static int lin16to8_framein(struct ast_translator_pvt *pvt, struct ast_frame *f)
{
    struct lin16lin8_pvt *tmp = (struct lin16lin8_pvt *)pvt;
    int x, samples;
    short *s, *d;

    samples = ast_codec_get_samples(f) / 2;

    if (ast_codec_get_len(AST_FORMAT_SLINEAR, tmp->tail) +
	ast_codec_get_len(AST_FORMAT_SLINEAR, samples) >= sizeof(tmp->outbuf))
    {
	ast_log (LOG_WARNING, "Out of buffer space\n");
	return -1;
    }

    s = f->data;
    d = tmp->outbuf + tmp->tail;

    for (x=0; x < samples; x++)
    {
	*d = *s;
	d++; s+=2;
    }
    tmp->tail += samples;
    return 0;
}

static struct ast_frame *lin8to16_frameout(struct ast_translator_pvt *pvt)
{
    struct lin8lin16_pvt *tmp = (struct lin8lin16_pvt *)pvt;

    if (!tmp->tail)
	return NULL;
    tmp->f.frametype = AST_FRAME_VOICE;
    tmp->f.subclass = AST_FORMAT_SLINEAR16;
    tmp->f.datalen = ast_codec_get_len(AST_FORMAT_SLINEAR16, tmp->tail);
    tmp->f.samples = tmp->tail;
    tmp->f.mallocd = 0;
    tmp->f.offset = AST_FRIENDLY_OFFSET;
    tmp->f.src = __PRETTY_FUNCTION__;
    tmp->f.data = tmp->outbuf;
    tmp->tail = 0;
    return &tmp->f;
}

static struct ast_frame *lin16to8_frameout(struct ast_translator_pvt *pvt)
{
    struct lin16lin8_pvt *tmp = (struct lin16lin8_pvt *)pvt;

    if (!tmp->tail)
	return NULL;
    tmp->f.frametype = AST_FRAME_VOICE;
    tmp->f.subclass = AST_FORMAT_SLINEAR;
    tmp->f.datalen = ast_codec_get_len(AST_FORMAT_SLINEAR, tmp->tail);
    tmp->f.samples = tmp->tail;
    tmp->f.mallocd = 0;
    tmp->f.offset = AST_FRIENDLY_OFFSET;
    tmp->f.src = __PRETTY_FUNCTION__;
    tmp->f.data = tmp->outbuf;
    tmp->tail = 0;
    return &tmp->f;
}

static struct ast_frame *lin8to16_sample(void)
{
    static struct ast_frame f;
    f.frametype = AST_FRAME_VOICE;
    f.subclass = AST_FORMAT_SLINEAR;
    f.datalen = sizeof(slin_ulaw_ex);
    f.samples = sizeof(slin_ulaw_ex) / 2;
    f.mallocd = 0;
    f.offset = 0;
    f.src = __PRETTY_FUNCTION__;
    f.data = slin_ulaw_ex;
    return &f;
}

static struct ast_frame *lin16to8_sample(void)
{
    static struct ast_frame f;
    f.frametype = AST_FRAME_VOICE;
    f.subclass = AST_FORMAT_SLINEAR16;
    f.datalen = sizeof(slin16_ulaw_ex);
    f.samples = sizeof(slin16_ulaw_ex) / 2;
    f.mallocd = 0;
    f.offset = 0;
    f.src = __PRETTY_FUNCTION__;
    f.data = slin16_ulaw_ex;
    return &f;
}

static void linXtoX_destroy(struct ast_translator_pvt *pvt)
{
    free (pvt);
    localusecnt--;
    ast_update_use_count ();
}

static struct ast_translator lin8to16 = {
    "lintolin16",
    AST_FORMAT_SLINEAR,
    AST_FORMAT_SLINEAR16,
    lin8to16_new,
    lin8to16_framein,
    lin8to16_frameout,
    linXtoX_destroy,
    /* NULL */
    lin8to16_sample
};

static struct ast_translator lin16to8 = {
    "lin16tolin",
    AST_FORMAT_SLINEAR16,
    AST_FORMAT_SLINEAR,
    lin16to8_new,
    lin16to8_framein,
    lin16to8_frameout,
    linXtoX_destroy,
    /* NULL */
    lin16to8_sample
};

int unload_module (void)
{
    int res = 0;
    ast_mutex_lock (&localuser_lock);
    res = ast_unregister_translator(&lin16to8);
    res = ast_unregister_translator(&lin8to16);
    if (localusecnt)
	res = -1;
    ast_mutex_unlock (&localuser_lock);
    return res;
}

int load_module(void)
{
    int res = 0;
    res |= ast_register_translator(&lin8to16);
    res |= ast_register_translator(&lin16to8);
    return res;
}

/*
 * Return a description of this module.
 */

char *description(void)
{
    return tdesc;
}

int usecount (void)
{
    int res;
    STANDARD_USECOUNT(res);
    return res;
}

char *key()
{
    return ASTERISK_GPL_KEY;
}
