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
 * \brief codec_alaw.c - translate between signed linear and alaw
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
#include "asterisk/alaw.h"

#define BUFFER_DECODE 16000
#define BUFFER_SIZE   8000	/* size for the translation buffers */

AST_MUTEX_DEFINE_STATIC(localuser_lock);
static int localusecnt = 0;

static char *tdesc = "A-law Coder/Decoder";

static int useplc = 0;

/* Sample frame data (Mu data is okay) */

#include "slin_ulaw_ex.h"
#include "ulaw_slin_ex.h"

/*
 * Private workspace for translating signed linear signals to alaw.
 */

struct alaw_encoder_pvt
{
  struct ast_frame f;
  char offset[AST_FRIENDLY_OFFSET];   /* Space to build offset */
  unsigned char outbuf[BUFFER_SIZE];  /* Encoded alaw, two nibbles to a word */
  int tail;
  int codec;
};

/*
 * Private workspace for translating alaw signals to signed linear.
 */

struct alaw_decoder_pvt
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
 * alawToLin_New
 *  Create a new instance of alaw_decoder_pvt.
 *
 * Results:
 *  Returns a pointer to the new instance.
 *
 * Side effects:
 *  None.
 */

static struct ast_translator_pvt *
alawtolin_new (void)
{
  struct alaw_decoder_pvt *tmp;
  tmp = malloc (sizeof (struct alaw_decoder_pvt));
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
alawtolin16_new (void)
{
  struct alaw_decoder_pvt *tmp;
  tmp = malloc (sizeof (struct alaw_decoder_pvt));
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
 * LinToalaw_New
 *  Create a new instance of alaw_encoder_pvt.
 *
 * Results:
 *  Returns a pointer to the new instance.
 *
 * Side effects:
 *  None.
 */

static struct ast_translator_pvt *
lintoalaw_new (void)
{
  struct alaw_encoder_pvt *tmp;
  tmp = malloc (sizeof (struct alaw_encoder_pvt));
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
lin16toalaw_new (void)
{
  struct alaw_encoder_pvt *tmp;
  tmp = malloc (sizeof (struct alaw_encoder_pvt));
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
 * alawToLin_FrameIn
 *  Fill an input buffer with packed 4-bit alaw values if there is room
 *  left.
 *
 * Results:
 *  Foo
 *
 * Side effects:
 *  tmp->tail is the number of packed values in the buffer.
 */

static int
alawtolin_framein (struct ast_translator_pvt *pvt, struct ast_frame *f)
{
  struct alaw_decoder_pvt *tmp = (struct alaw_decoder_pvt *) pvt;
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
	  *s = AST_ALAW(*b);
	  s++; b++;
      }
      tmp->tail += fsamples;
  } else if (tmp->codec == AST_FORMAT_SLINEAR16) {
      for (x = 0; x<fsamples; x++) {
	  *s = saved = AST_ALAW(*b);
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
 * alawToLin_FrameOut
 *  Convert 4-bit alaw encoded signals to 16-bit signed linear.
 *
 * Results:
 *  Converted signals are placed in tmp->f.data, tmp->f.datalen
 *  and tmp->f.samples are calculated.
 *
 * Side effects:
 *  None.
 */

static struct ast_frame *
alawtolin_frameout (struct ast_translator_pvt *pvt)
{
  struct alaw_decoder_pvt *tmp = (struct alaw_decoder_pvt *) pvt;

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
 * LinToalaw_FrameIn
 *  Fill an input buffer with 16-bit signed linear PCM values.
 *
 * Results:
 *  None.
 *
 * Side effects:
 *  tmp->tail is number of signal values in the input buffer.
 */

static int
lintoalaw_framein (struct ast_translator_pvt *pvt, struct ast_frame *f)
{
  struct alaw_encoder_pvt *tmp = (struct alaw_encoder_pvt *) pvt;
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
	  *b = AST_LIN2A(*s);
	  b++; s++;
      }
      tmp->tail += samples;
  }
  else if (tmp->codec == AST_FORMAT_SLINEAR16) {
      for (x = 0; x < samples; x++) {
	  *b = AST_LIN2A(*s);
	  b++; s+=2;
      }
      tmp->tail += samples;
  }else
      return -1;
  return 0;
}

/*
 * LinToalaw_FrameOut
 *  Convert a buffer of raw 16-bit signed linear PCM to a buffer
 *  of 4-bit alaw packed two to a byte (Big Endian).
 *
 * Results:
 *  Foo
 *
 * Side effects:
 *  Leftover inbuf data gets packed, tail gets updated.
 */

static struct ast_frame *
lintoalaw_frameout (struct ast_translator_pvt *pvt)
{
  struct alaw_encoder_pvt *tmp = (struct alaw_encoder_pvt *) pvt;
  
  if (tmp->tail) {
	  tmp->f.frametype = AST_FRAME_VOICE;
	  tmp->f.subclass = AST_FORMAT_ALAW;
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
 * alawToLin_Sample
 */

static struct ast_frame *
alawtolin_sample (void)
{
  static struct ast_frame f;
  f.frametype = AST_FRAME_VOICE;
  f.subclass = AST_FORMAT_ALAW;
  f.datalen = sizeof (ulaw_slin_ex);
  f.samples = sizeof(ulaw_slin_ex);
  f.mallocd = 0;
  f.offset = 0;
  f.src = __PRETTY_FUNCTION__;
  f.data = ulaw_slin_ex;
  return &f;
}

/*
 * LinToalaw_Sample
 */

static struct ast_frame *
lintoalaw_sample (void)
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
lin16toalaw_sample (void)
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
 * alaw_Destroy
 *  Destroys a private workspace.
 *
 * Results:
 *  It's gone!
 *
 * Side effects:
 *  None.
 */

static void
alaw_destroy (struct ast_translator_pvt *pvt)
{
  free (pvt);
  localusecnt--;
  ast_update_use_count ();
}

/*
 * The complete translator for alawToLin.
 */

static struct ast_translator alawtolin = {
  "alawtolin",
  AST_FORMAT_ALAW,
  AST_FORMAT_SLINEAR,
  alawtolin_new,
  alawtolin_framein,
  alawtolin_frameout,
  alaw_destroy,
  /* NULL */
  alawtolin_sample
};

/*
 * The complete translator for LinToalaw.
 */

static struct ast_translator lintoalaw = {
  "lintoalaw",
  AST_FORMAT_SLINEAR,
  AST_FORMAT_ALAW,
  lintoalaw_new,
  lintoalaw_framein,
  lintoalaw_frameout,
  alaw_destroy,
  /* NULL */
  lintoalaw_sample
};

static struct ast_translator alawtolin16 = {
  "alawtolin16",
  AST_FORMAT_ALAW,
  AST_FORMAT_SLINEAR16,
  alawtolin16_new,
  alawtolin_framein,
  alawtolin_frameout,
  alaw_destroy,
  alawtolin_sample
};

static struct ast_translator lin16toalaw = {
  "lin16toalaw",
  AST_FORMAT_SLINEAR16,
  AST_FORMAT_ALAW,
  lin16toalaw_new,
  lintoalaw_framein,
  lintoalaw_frameout,
  alaw_destroy,
  lin16toalaw_sample
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
	      ast_log (LOG_WARNING, "codec_alaw: Generic PLC disabled\n");
	      useplc = 0;
	  }
#endif
	  if (option_verbose > 2)
	    ast_verbose(VERBOSE_PREFIX_3 "codec_alaw: %susing generic PLC\n", useplc ? "" : "not ");
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
  res |= ast_unregister_translator (&lin16toalaw);
  res |= ast_unregister_translator (&alawtolin16);
  res |= ast_unregister_translator (&lintoalaw);
  res |= ast_unregister_translator (&alawtolin);
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
  res |= ast_register_translator (&alawtolin);
  res |= ast_register_translator (&lintoalaw);
  res |= ast_register_translator (&alawtolin16);
  res |= ast_register_translator (&lin16toalaw);

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
