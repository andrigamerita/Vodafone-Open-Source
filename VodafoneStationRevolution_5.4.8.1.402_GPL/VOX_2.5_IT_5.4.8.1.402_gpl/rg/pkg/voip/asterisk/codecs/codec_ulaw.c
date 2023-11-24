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
 * \brief codec_ulaw.c - translate between signed linear and ulaw
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
#include "asterisk/config.h"
#include "asterisk/options.h"
#include "asterisk/translate.h"
#include "asterisk/channel.h"
#include "asterisk/ulaw.h"

#define BUFFER_DECODE 16000
#define BUFFER_SIZE   8000	/* size for the translation buffers */

AST_MUTEX_DEFINE_STATIC(localuser_lock);
static int localusecnt = 0;

static char *tdesc = "Mu-law Coder/Decoder";

static int useplc = 0;

/* Sample frame data */

#include "slin_ulaw_ex.h"
#include "ulaw_slin_ex.h"

/*
 * Private workspace for translating signed linear signals to ulaw.
 */

struct ulaw_encoder_pvt
{
  struct ast_frame f;
  char offset[AST_FRIENDLY_OFFSET];   /* Space to build offset */
  unsigned char outbuf[BUFFER_SIZE];  /* Encoded ulaw, two nibbles to a word */
  int tail;
  int codec;
};

/*
 * Private workspace for translating ulaw signals to signed linear.
 */

struct ulaw_decoder_pvt
{
  struct ast_frame f;
  char offset[AST_FRIENDLY_OFFSET];	/* Space to build offset */
  short outbuf[BUFFER_DECODE];	/* Decoded signed linear values */
  int tail;
#if 0
  plc_state_t plc;
#endif
  int codec;
};

/*
 * ulawToLin_New
 *  Create a new instance of ulaw_decoder_pvt.
 *
 * Results:
 *  Returns a pointer to the new instance.
 *
 * Side effects:
 *  None.
 */

static struct ast_translator_pvt *
ulawtolin_new (void)
{
  struct ulaw_decoder_pvt *tmp;
  tmp = malloc (sizeof (struct ulaw_decoder_pvt));
  if (tmp)
    {
	  memset(tmp, 0, sizeof(*tmp));
      tmp->tail = 0;
      tmp->codec = AST_FORMAT_SLINEAR;
#if 0
      plc_init(&tmp->plc);
#endif
      localusecnt++;
      ast_update_use_count ();
    }
  return (struct ast_translator_pvt *) tmp;
}


static struct ast_translator_pvt *
ulawtolin16_new (void)
{
  struct ulaw_decoder_pvt *tmp;
  tmp = malloc (sizeof (struct ulaw_decoder_pvt));
  if (tmp)
    {
	  memset(tmp, 0, sizeof(*tmp));
      tmp->tail = 0;
      tmp->codec = AST_FORMAT_SLINEAR16;
#if 0
      plc_init(&tmp->plc);
#endif
      localusecnt++;
      ast_update_use_count ();
    }
  return (struct ast_translator_pvt *) tmp;
}

/*
 * LinToulaw_New
 *  Create a new instance of ulaw_encoder_pvt.
 *
 * Results:
 *  Returns a pointer to the new instance.
 *
 * Side effects:
 *  None.
 */

static struct ast_translator_pvt *
lintoulaw_new (void)
{
  struct ulaw_encoder_pvt *tmp;
  tmp = malloc (sizeof (struct ulaw_encoder_pvt));
  if (tmp)
    {
	  memset(tmp, 0, sizeof(*tmp));
      tmp->codec = AST_FORMAT_SLINEAR;
      localusecnt++;
      ast_update_use_count ();
      tmp->tail = 0;
    }
  return (struct ast_translator_pvt *) tmp;
}

static struct ast_translator_pvt *
lin16toulaw_new (void)
{
  struct ulaw_encoder_pvt *tmp;
  tmp = malloc (sizeof (struct ulaw_encoder_pvt));
  if (tmp)
    {
	  memset(tmp, 0, sizeof(*tmp));
      tmp->codec = AST_FORMAT_SLINEAR16;
      localusecnt++;
      ast_update_use_count ();
      tmp->tail = 0;
    }
  return (struct ast_translator_pvt *) tmp;
}

/*
 * ulawToLin_FrameIn
 *  Fill an input buffer with packed 4-bit ulaw values if there is room
 *  left.
 *
 * Results:
 *  Foo
 *
 * Side effects:
 *  tmp->tail is the number of packed values in the buffer.
 */

static int
ulawtolin_framein (struct ast_translator_pvt *pvt, struct ast_frame *f)
{
  struct ulaw_decoder_pvt *tmp = (struct ulaw_decoder_pvt *) pvt;
  int x, fsamples, samples;
  unsigned char *b;
  short *s;
  short saved;

  if(f->datalen == 0) { /* perform PLC with nominal framesize of 20ms/160 samples */
#if 0
	if((tmp->tail + 160)  * 2 > sizeof(tmp->outbuf)) {
	    ast_log(LOG_WARNING, "Out of buffer space\n");
	    return -1;
	}
	if(useplc) {
	    plc_fillin(&tmp->plc, tmp->outbuf+tmp->tail, 160);
	    tmp->tail += 160;
	}
#endif
	return 0;
  }

  fsamples = samples = ast_codec_get_samples(f);
  if (tmp->codec == AST_FORMAT_SLINEAR16)
      samples *= 2;

  if (ast_codec_get_len(tmp->codec, tmp->tail) +
      ast_codec_get_len(tmp->codec, samples) >= sizeof(tmp->outbuf))
  {
  	ast_log(LOG_WARNING, "Out of buffer space\n");
	return -1;
  }

  /* Reset ssindex and signal to frame's specified values */
  b = f->data;
  s = tmp->outbuf + tmp->tail;

  if (tmp->codec == AST_FORMAT_SLINEAR) {
      for (x = 0; x<fsamples; x++) {
	  *s = AST_MULAW(*b);
	  s++; b++;
      }
      tmp->tail += fsamples;
  } else if (tmp->codec == AST_FORMAT_SLINEAR16) {
      for (x = 0; x<fsamples; x++) {
	  *s = saved = AST_MULAW(*b);
	  s++; b++;
	  *s = saved;
	  s++;
      }
      tmp->tail += samples;
  } else
      return -1;
#if 0
  if(useplc) plc_rx(&tmp->plc, tmp->outbuf+tmp->tail, f->datalen);
#endif

  return 0;
}

/*
 * ulawToLin_FrameOut
 *  Convert 4-bit ulaw encoded signals to 16-bit signed linear.
 *
 * Results:
 *  Converted signals are placed in tmp->f.data, tmp->f.datalen
 *  and tmp->f.samples are calculated.
 *
 * Side effects:
 *  None.
 */

static struct ast_frame *
ulawtolin_frameout (struct ast_translator_pvt *pvt)
{
  struct ulaw_decoder_pvt *tmp = (struct ulaw_decoder_pvt *) pvt;

  if (!tmp->tail)
    return NULL;

  tmp->f.frametype = AST_FRAME_VOICE;
  tmp->f.subclass = tmp->codec;
  tmp->f.datalen = ast_codec_get_len(tmp->codec, tmp->tail);
  tmp->f.samples = tmp->tail;
  tmp->f.mallocd = 0;
  tmp->f.offset = AST_FRIENDLY_OFFSET;
  tmp->f.src = __PRETTY_FUNCTION__;
  tmp->f.data = tmp->outbuf;
  tmp->tail = 0;
  return &tmp->f;
}

/*
 * LinToulaw_FrameIn
 *  Fill an input buffer with 16-bit signed linear PCM values.
 *
 * Results:
 *  None.
 *
 * Side effects:
 *  tmp->tail is number of signal values in the input buffer.
 */

static int
lintoulaw_framein (struct ast_translator_pvt *pvt, struct ast_frame *f)
{
  struct ulaw_encoder_pvt *tmp = (struct ulaw_encoder_pvt *) pvt;
  int x, samples;
  short *s;
  unsigned char *b;

  samples = ast_codec_get_samples(f);

  if (f->subclass == AST_FORMAT_SLINEAR16)
      samples /= 2;
  if (ast_codec_get_len(tmp->codec, tmp->tail) +
      ast_codec_get_len(tmp->codec, samples) >= sizeof(tmp->outbuf))
    {
      ast_log (LOG_WARNING, "Out of buffer space\n");
      return -1;
    }
  s = f->data;
  b = tmp->outbuf + tmp->tail;

  if (tmp->codec == AST_FORMAT_SLINEAR) {
      for (x = 0; x < samples; x++) {
	  *b = AST_LIN2MU(*s);
	  b++; s++;
      }
      tmp->tail += samples;
  }
  else if (tmp->codec == AST_FORMAT_SLINEAR16) {
      for (x = 0; x < samples; x++) {
	  *b = AST_LIN2MU(*s);
	  b++; s+=2;
      }
      tmp->tail += samples;
  }else
      return -1;
  return 0;
}

/*
 * LinToulaw_FrameOut
 *  Convert a buffer of raw 16-bit signed linear PCM to a buffer
 *  of 4-bit ulaw packed two to a byte (Big Endian).
 *
 * Results:
 *  Foo
 *
 * Side effects:
 *  Leftover inbuf data gets packed, tail gets updated.
 */

static struct ast_frame *
lintoulaw_frameout (struct ast_translator_pvt *pvt)
{
  struct ulaw_encoder_pvt *tmp = (struct ulaw_encoder_pvt *) pvt;
  
  if (tmp->tail) {
	  tmp->f.frametype = AST_FRAME_VOICE;
	  tmp->f.subclass = AST_FORMAT_ULAW;
	  tmp->f.samples = tmp->tail;
	  tmp->f.mallocd = 0;
	  tmp->f.offset = AST_FRIENDLY_OFFSET;
	  tmp->f.src = __PRETTY_FUNCTION__;
	  tmp->f.data = tmp->outbuf;
	  tmp->f.datalen = tmp->tail;
	  tmp->tail = 0;
	  return &tmp->f;
   } else return NULL;
}


/*
 * ulawToLin_Sample
 */

static struct ast_frame *
ulawtolin_sample (void)
{
  static struct ast_frame f;
  f.frametype = AST_FRAME_VOICE;
  f.subclass = AST_FORMAT_ULAW;
  f.datalen = sizeof (ulaw_slin_ex);
  f.samples = sizeof(ulaw_slin_ex);
  f.mallocd = 0;
  f.offset = 0;
  f.src = __PRETTY_FUNCTION__;
  f.data = ulaw_slin_ex;
  return &f;
}

/*
 * LinToulaw_Sample
 */

static struct ast_frame *
lintoulaw_sample (void)
{
  static struct ast_frame f;
  f.frametype = AST_FRAME_VOICE;
  f.subclass = AST_FORMAT_SLINEAR;
  f.datalen = sizeof (slin_ulaw_ex);
  /* Assume 8000 Hz */
  f.samples = sizeof (slin_ulaw_ex) / 2;
  f.mallocd = 0;
  f.offset = 0;
  f.src = __PRETTY_FUNCTION__;
  f.data = slin_ulaw_ex;
  return &f;
}

static struct ast_frame *
lin16toulaw_sample (void)
{
  static struct ast_frame f;
  f.frametype = AST_FRAME_VOICE;
  f.subclass = AST_FORMAT_SLINEAR16;
  f.datalen = sizeof (slin16_ulaw_ex);
  f.samples = sizeof (slin16_ulaw_ex) / 2;
  f.mallocd = 0;
  f.offset = 0;
  f.src = __PRETTY_FUNCTION__;
  f.data = slin16_ulaw_ex;
  return &f;
}

/*
 * ulaw_Destroy
 *  Destroys a private workspace.
 *
 * Results:
 *  It's gone!
 *
 * Side effects:
 *  None.
 */

static void
ulaw_destroy (struct ast_translator_pvt *pvt)
{
  free (pvt);
  localusecnt--;
  ast_update_use_count ();
}

/*
 * The complete translator for ulawToLin.
 */

static struct ast_translator ulawtolin = {
  "ulawtolin",
  AST_FORMAT_ULAW,
  AST_FORMAT_SLINEAR,
  ulawtolin_new,
  ulawtolin_framein,
  ulawtolin_frameout,
  ulaw_destroy,
  /* NULL */
  ulawtolin_sample
};

/*
 * The complete translator for LinToulaw.
 */

static struct ast_translator lintoulaw = {
  "lintoulaw",
  AST_FORMAT_SLINEAR,
  AST_FORMAT_ULAW,
  lintoulaw_new,
  lintoulaw_framein,
  lintoulaw_frameout,
  ulaw_destroy,
  /* NULL */
  lintoulaw_sample
};

static struct ast_translator ulawtolin16 = {
  "ulawtolin16",
  AST_FORMAT_ULAW,
  AST_FORMAT_SLINEAR16,
  ulawtolin16_new,
  ulawtolin_framein,
  ulawtolin_frameout,
  ulaw_destroy,
  ulawtolin_sample
};

static struct ast_translator lin16toulaw = {
  "lin16toulaw",
  AST_FORMAT_SLINEAR16,
  AST_FORMAT_ULAW,
  lin16toulaw_new,
  lintoulaw_framein,
  lintoulaw_frameout,
  ulaw_destroy,
  lin16toulaw_sample
};

static void 
parse_config(void)
{
  struct ast_config *cfg;
  struct ast_variable *var;
  if ((cfg = ast_config_load("codecs.conf"))) {
    if ((var = ast_variable_browse(cfg, "plc"))) {
      while (var) {
	if (!strcasecmp(var->name, "genericplc")) {
	  useplc = ast_true(var->value) ? 1 : 0;
#if 1
	  if (useplc)
	  {
	      ast_log (LOG_WARNING, "codec_ulaw: Generic PLC disabled\n");
	      useplc = 0;
	  }
#endif
	  if (option_verbose > 2)
	    ast_verbose(VERBOSE_PREFIX_3 "codec_ulaw: %susing generic PLC\n", useplc ? "" : "not ");
	}
	var = var->next;
      }
    }
    ast_config_destroy(cfg);
  }
}

int 
reload(void)
{
  parse_config();
  return 0;
}

int
unload_module (void)
{
  int res = 0;
  ast_mutex_lock (&localuser_lock);
  res |= ast_unregister_translator (&lin16toulaw);
  res |= ast_unregister_translator (&ulawtolin16);
  res |= ast_unregister_translator (&lintoulaw);
  res |= ast_unregister_translator (&ulawtolin);
  if (localusecnt)
    res = -1;
  ast_mutex_unlock (&localuser_lock);
  return res;
}

int
load_module (void)
{
  int res = 0;
  parse_config();
  res |= ast_register_translator (&ulawtolin);
  res |= ast_register_translator (&lintoulaw);
  res |= ast_register_translator (&ulawtolin16);
  res |= ast_register_translator (&lin16toulaw);

  return res;
}

/*
 * Return a description of this module.
 */

char *
description (void)
{
  return tdesc;
}

int
usecount (void)
{
  int res;
  STANDARD_USECOUNT (res);
  return res;
}

char *
key ()
{
  return ASTERISK_GPL_KEY;
}
