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
 * \brief Frame manipulation routines
 * 
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 1.10.2.6 $")

#include "asterisk/lock.h"
#include "asterisk/frame.h"
#include "asterisk/logger.h"
#include "asterisk/options.h"
#include "asterisk/channel.h"
#include "asterisk/cli.h"
#include "asterisk/term.h"
#include "asterisk/utils.h"

#ifdef TRACE_FRAMES
static int headers = 0;
static struct ast_frame *headerlist = NULL;
AST_MUTEX_DEFINE_STATIC(framelock);
#endif

#define SMOOTHER_SIZE 16000

#define TYPE_HIGH	 0x0
#define TYPE_LOW	 0x1
#define TYPE_SILENCE	 0x2
#define TYPE_DONTSEND	 0x3
#define TYPE_MASK	 0x3

struct ast_format_list {
	int visible;	/*!< Can we see this entry */
	int bits;	/*!< bitmask value */
	char *name;	/*!< short name */
	char *desc;	/*!< Description */
	ast_codec_quality quality; /*!< What's the codec quality 0 - low, 1 - high */
};

struct ast_smoother {
	int size;
	int format;
	int readdata;
	int optimizablestream;
	int flags;
	float samplesperbyte;
	struct ast_frame f;
	struct timeval delivery;
	char data[SMOOTHER_SIZE];
	char framedata[SMOOTHER_SIZE + AST_FRIENDLY_OFFSET];
	struct ast_frame *opt;
	int len;
};

/*! \brief Definition of supported media formats (codecs) */
static struct ast_format_list AST_FORMAT_LIST[] = {
	{ 1, AST_FORMAT_G723_1 , "g723" , "G.723.1", 1 },       /*!< codec_g723_1.c */
	{ 1, AST_FORMAT_GSM, "gsm" , "GSM"},		/*!< codec_gsm.c */
	{ 1, AST_FORMAT_ULAW, "ulaw", "G.711 u-law", 1 },       /*!< codec_ulaw.c */
	{ 1, AST_FORMAT_ALAW, "alaw", "G.711 A-law", 1 },       /*!< codec_alaw.c */
	{ 1, AST_FORMAT_G726, "g726", "G.726" },     /*!< codec_g726.c */
	{ 1, AST_FORMAT_ADPCM, "adpcm" , "ADPCM", 1 },  /*!< codec_adpcm.c */
	{ 1, AST_FORMAT_SLINEAR, "slin",  "16 bit Signed Linear PCM", 1 },      /*!<  */
	{ 1, AST_FORMAT_LPC10, "lpc10", "LPC10", 1 },   /*!< codec_lpc10.c */
	{ 1, AST_FORMAT_G729A, "g729", "G.729A", 1 },	/*!< Binary commercial distribution */
	{ 1, AST_FORMAT_SPEEX, "speex", "SpeeX" },	/*!< codec_speex.c */
	{ 1, AST_FORMAT_ILBC, "ilbc", "iLBC"},	/*!< codec_ilbc.c */
	{ 1, AST_FORMAT_G722 , "g722" , "G.722", 1 },
	{ 1, AST_FORMAT_SLINEAR16, "slin16",  "16 bit Signed Linear PCM (16kHz)", 1 },
	{ 1, AST_FORMAT_CN, "cn", "cn"},
	{ 0, 0, "nothing", "undefined" },
	{ 0, AST_FORMAT_MAX_AUDIO, "maxaudio", "Maximum audio format" },
	{ 1, AST_FORMAT_JPEG, "jpeg", "JPEG image"},	/*!< See format_jpeg.c */
	{ 1, AST_FORMAT_PNG, "png", "PNG image"},	/*!< Image format */
	{ 1, AST_FORMAT_H261, "h261", "H.261 Video" },	/*!< Passthrough */
	{ 1, AST_FORMAT_H263, "h263", "H.263 Video" },	/*!< Passthrough support, see format_h263.c */
	{ 1, AST_FORMAT_H263_PLUS, "h263p", "H.263+ Video" },	/*!< See format_h263.c */
	{ 0, 0, "nothing", "undefined" },
	{ 0, 0, "nothing", "undefined" },
	{ 0, 0, "nothing", "undefined" },
	{ 0, 0, "nothing", "undefined" },
	{ 0, AST_FORMAT_MAX_VIDEO, "maxvideo", "Maximum video format" },
};

void ast_smoother_reset(struct ast_smoother *s, int size)
{
	memset(s, 0, sizeof(struct ast_smoother));
	s->size = size;
}

struct ast_smoother *ast_smoother_new(int size)
{
	struct ast_smoother *s;
	if (size < 1)
		return NULL;
	s = malloc(sizeof(struct ast_smoother));
	if (s)
		ast_smoother_reset(s, size);
	return s;
}

int ast_smoother_get_flags(struct ast_smoother *s)
{
	return s->flags;
}

void ast_smoother_set_flags(struct ast_smoother *s, int flags)
{
	s->flags = flags;
}

int __ast_smoother_feed(struct ast_smoother *s, struct ast_frame *f, int swap)
{
	if (f->frametype != AST_FRAME_VOICE) {
		ast_log(LOG_WARNING, "Huh?  Can't smooth a non-voice frame!\n");
		return -1;
	}
	if (!s->format) {
		s->format = f->subclass;
		s->samplesperbyte = (float)f->samples / (float)f->datalen;
	} else if (s->format != f->subclass) {
		ast_log(LOG_WARNING, "Smoother was working on %d format frames, now trying to feed %d?\n", s->format, f->subclass);
		return -1;
	}
	if (s->len + f->datalen > SMOOTHER_SIZE) {
		ast_log(LOG_WARNING, "Out of smoother space\n");
		return -1;
	}
	if (((f->datalen == s->size) || ((f->datalen < 10) && (s->flags & AST_SMOOTHER_FLAG_G729)))
				 && !s->opt && (f->offset >= AST_MIN_OFFSET)) {
		if (!s->len) {
			/* Optimize by sending the frame we just got
			   on the next read, thus eliminating the douple
			   copy */
			s->opt = f;
			return 0;
		} else {
			s->optimizablestream++;
			if (s->optimizablestream > 10) {
				/* For the past 10 rounds, we have input and output
				   frames of the correct size for this smoother, yet
				   we were unable to optimize because there was still
				   some cruft left over.  Lets just drop the cruft so
				   we can move to a fully optimized path */
				s->len = 0;
				s->opt = f;
				return 0;
			}
		}
	} else 
		s->optimizablestream = 0;
	if (s->flags & AST_SMOOTHER_FLAG_G729) {
		if (s->len % 10) {
			ast_log(LOG_NOTICE, "Dropping extra frame of G.729 since we already have a VAD frame at the end\n");
			return 0;
		}
	}
	if (swap)
		ast_swapcopy_samples(s->data+s->len, f->data, f->samples);
	else
		memcpy(s->data + s->len, f->data, f->datalen);
	/* If either side is empty, reset the delivery time */
	if (!s->len || ast_tvzero(f->delivery) || ast_tvzero(s->delivery))	/* XXX really ? */
		s->delivery = f->delivery;
	s->len += f->datalen;
	return 0;
}

struct ast_frame *ast_smoother_read(struct ast_smoother *s)
{
	struct ast_frame *opt;
	int len;
	/* IF we have an optimization frame, send it */
	if (s->opt) {
		if (s->opt->offset < AST_FRIENDLY_OFFSET)
			ast_log(LOG_WARNING, "Returning a frame of inappropriate offset (%d).",
							s->opt->offset);
		opt = s->opt;
		s->opt = NULL;
		return opt;
	}

	/* Make sure we have enough data */
	if (s->len < s->size) {
		/* Or, if this is a G.729 frame with VAD on it, send it immediately anyway */
		if (!((s->flags & AST_SMOOTHER_FLAG_G729) && (s->size % 10)))
			return NULL;
	}
	len = s->size;
	if (len > s->len)
		len = s->len;
	/* Make frame */
	s->f.frametype = AST_FRAME_VOICE;
	s->f.subclass = s->format;
	s->f.data = s->framedata + AST_FRIENDLY_OFFSET;
	s->f.offset = AST_FRIENDLY_OFFSET;
	s->f.datalen = len;
	/* Samples will be improper given VAD, but with VAD the concept really doesn't even exist */
	s->f.samples = len * s->samplesperbyte;	/* XXX rounding */
	s->f.delivery = s->delivery;
	/* Fill Data */
	memcpy(s->f.data, s->data, len);
	s->len -= len;
	/* Move remaining data to the front if applicable */
	if (s->len) {
		/* In principle this should all be fine because if we are sending
		   G.729 VAD, the next timestamp will take over anyawy */
		memmove(s->data, s->data + len, s->len);
		if (!ast_tvzero(s->delivery)) {
			/* If we have delivery time, increment it, otherwise, leave it at 0 */
			s->delivery = ast_tvadd(s->delivery, ast_samp2tv(s->f.samples,
			    s->f.subclass == AST_FORMAT_SLINEAR16 ? 16000 : 8000));
		}
	}
	/* Return frame */
	return &s->f;
}

void ast_smoother_free(struct ast_smoother *s)
{
	free(s);
}

static struct ast_frame *ast_frame_header_new(void)
{
	struct ast_frame *f;
	f = malloc(sizeof(struct ast_frame));
	if (f)
		memset(f, 0, sizeof(struct ast_frame));
#ifdef TRACE_FRAMES
	if (f) {
		headers++;
		f->prev = NULL;
		ast_mutex_lock(&framelock);
		f->next = headerlist;
		if (headerlist)
			headerlist->prev = f;
		headerlist = f;
		ast_mutex_unlock(&framelock);
	}
#endif	
	return f;
}

/*!
 * \todo Important: I should be made more efficient.  Frame headers should
 * most definitely be cached
 */
void ast_frfree(struct ast_frame *fr)
{
	if (fr->mallocd & AST_MALLOCD_DATA) {
		if (fr->data) 
			free(fr->data - fr->offset);
	}
	if (fr->mallocd & AST_MALLOCD_SRC) {
		if (fr->src)
			free((char *)fr->src);
	}
	if (fr->mallocd & AST_MALLOCD_HDR) {
#ifdef TRACE_FRAMES
		headers--;
		ast_mutex_lock(&framelock);
		if (fr->next)
			fr->next->prev = fr->prev;
		if (fr->prev)
			fr->prev->next = fr->next;
		else
			headerlist = fr->next;
		ast_mutex_unlock(&framelock);
#endif			
		free(fr);
	}
}

/*!
 * \brief 'isolates' a frame by duplicating non-malloc'ed components
 * (header, src, data).
 * On return all components are malloc'ed
 */
struct ast_frame *ast_frisolate(struct ast_frame *fr)
{
	struct ast_frame *out;
	if (!(fr->mallocd & AST_MALLOCD_HDR)) {
		/* Allocate a new header if needed */
		out = ast_frame_header_new();
		if (!out) {
			ast_log(LOG_WARNING, "Out of memory\n");
			return NULL;
		}
		out->frametype = fr->frametype;
		out->subclass = fr->subclass;
		out->datalen = fr->datalen;
		out->samples = fr->samples;
		out->offset = fr->offset;
		out->src = NULL;
		out->data = fr->data;
	} else {
		out = fr;
	}
	if (!(fr->mallocd & AST_MALLOCD_SRC)) {
		if (fr->src)
			out->src = strdup(fr->src);
	} else
		out->src = fr->src;
	if (!(fr->mallocd & AST_MALLOCD_DATA))  {
		out->data = malloc(fr->datalen + AST_FRIENDLY_OFFSET);
		if (!out->data) {
			free(out);
			ast_log(LOG_WARNING, "Out of memory\n");
			return NULL;
		}
		out->data += AST_FRIENDLY_OFFSET;
		out->offset = AST_FRIENDLY_OFFSET;
		out->datalen = fr->datalen;
		memcpy(out->data, fr->data, fr->datalen);
	}
	out->mallocd = AST_MALLOCD_HDR | AST_MALLOCD_SRC | AST_MALLOCD_DATA;
	return out;
}

struct ast_frame *ast_frdup(struct ast_frame *f)
{
	struct ast_frame *out;
	int len, srclen = 0;
	void *buf;
	/* Start with standard stuff */
	len = sizeof(struct ast_frame) + AST_FRIENDLY_OFFSET + f->datalen;
	/* If we have a source, add space for it */
	/*
	 * XXX Watch out here - if we receive a src which is not terminated
	 * properly, we can be easily attacked. Should limit the size we deal with.
	 */
	if (f->src)
		srclen = strlen(f->src);
	if (srclen > 0)
		len += srclen + 1;
	buf = malloc(len);
	if (!buf)
		return NULL;
	out = buf;
	/* Set us as having malloc'd header only, so it will eventually
	   get freed. */
	out->frametype = f->frametype;
	out->subclass = f->subclass;
	out->datalen = f->datalen;
	out->samples = f->samples;
	out->delivery = f->delivery;
	out->mallocd = AST_MALLOCD_HDR;
	out->offset = AST_FRIENDLY_OFFSET;
	out->data = buf + sizeof(struct ast_frame) + AST_FRIENDLY_OFFSET;
	if (srclen > 0) {
		out->src = out->data + f->datalen;
		/* Must have space since we allocated for it */
		strcpy((char *)out->src, f->src);
	} else
		out->src = NULL;
	out->prev = NULL;
	out->next = NULL;
	memcpy(out->data, f->data, out->datalen);	
	return out;
}

#if 0
/*
 * XXX
 * This function is badly broken - it does not handle correctly
 * partial reads on either header or body.
 * However is it never used anywhere so we leave it commented out
 */
struct ast_frame *ast_fr_fdread(int fd)
{
	char buf[65536];
	int res;
	int ttl = sizeof(struct ast_frame);
	struct ast_frame *f = (struct ast_frame *)buf;
	/* Read a frame directly from there.  They're always in the
	   right format. */
	
	while(ttl) {
		res = read(fd, buf, ttl);
		if (res < 0) {
			ast_log(LOG_WARNING, "Bad read on %d: %s\n", fd, strerror(errno));
			return NULL;
		}
		ttl -= res;
	}
	
	/* read the frame header */
	f->mallocd = 0;
	/* Re-write data position */
	f->data = buf + sizeof(struct ast_frame);
	f->offset = 0;
	/* Forget about being mallocd */
	f->mallocd = 0;
	/* Re-write the source */
	f->src = (char *)__FUNCTION__;
	if (f->datalen > sizeof(buf) - sizeof(struct ast_frame)) {
		/* Really bad read */
		ast_log(LOG_WARNING, "Strange read (%d bytes)\n", f->datalen);
		return NULL;
	}
	if (f->datalen) {
		if ((res = read(fd, f->data, f->datalen)) != f->datalen) {
			/* Bad read */
			ast_log(LOG_WARNING, "How very strange, expected %d, got %d\n", f->datalen, res);
			return NULL;
		}
	}
	if ((f->frametype == AST_FRAME_CONTROL) && (f->subclass == AST_CONTROL_HANGUP)) {
		return NULL;
	}
	return ast_frisolate(f);
}

/* Some convenient routines for sending frames to/from stream or datagram
   sockets, pipes, etc (maybe even files) */

/*
 * XXX this function is also partly broken because it does not handle
 * partial writes. We comment it out too, and also the unique
 * client it has, ast_fr_fdhangup()
 */
int ast_fr_fdwrite(int fd, struct ast_frame *frame)
{
	/* Write the frame exactly */
	if (write(fd, frame, sizeof(struct ast_frame)) != sizeof(struct ast_frame)) {
		ast_log(LOG_WARNING, "Write error: %s\n", strerror(errno));
		return -1;
	}
	if (write(fd, frame->data, frame->datalen) != frame->datalen) {
		ast_log(LOG_WARNING, "Write error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int ast_fr_fdhangup(int fd)
{
	struct ast_frame hangup = {
		AST_FRAME_CONTROL,
		AST_CONTROL_HANGUP
	};
	return ast_fr_fdwrite(fd, &hangup);
}

#endif /* unused functions */
void ast_swapcopy_samples(void *dst, const void *src, int samples)
{
	int i;
	unsigned short *dst_s = dst;
	const unsigned short *src_s = src;

	for (i=0; i<samples; i++)
		dst_s[i] = (src_s[i]<<8) | (src_s[i]>>8);
}

int ast_get_bits(int index)
{
	return AST_FORMAT_LIST[index - 1].bits;
}

struct ast_format_list *ast_get_format_list_index(int index) 
{
	return &AST_FORMAT_LIST[index];
}

struct ast_format_list *ast_get_format_list(size_t *size) 
{
	*size = (sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list));
	return AST_FORMAT_LIST;
}

char* ast_getformatname(int format)
{
	int x = 0;
	char *ret = "unknown";
	for (x = 0 ; x < sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list) ; x++) {
		if(AST_FORMAT_LIST[x].visible && AST_FORMAT_LIST[x].bits == format) {
			ret = AST_FORMAT_LIST[x].name;
			break;
		}
	}
	return ret;
}

char *ast_getformatname_multiple(char *buf, size_t size, int format) {

	int x = 0;
	unsigned len;
	char *end = buf;
	char *start = buf;
	if (!size) return buf;
	snprintf(end, size, "0x%x (", format);
	len = strlen(end);
	end += len;
	size -= len;
	start = end;
	for (x = 0 ; x < sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list) ; x++) {
		if (AST_FORMAT_LIST[x].visible && (AST_FORMAT_LIST[x].bits & format)) {
			snprintf(end, size,"%s|",AST_FORMAT_LIST[x].name);
			len = strlen(end);
			end += len;
			size -= len;
		}
	}
	if (start == end)
		snprintf(start, size, "nothing)");
	else if (size > 1)
		*(end -1) = ')';
	return buf;
}

static struct ast_codec_alias_table {
	char *alias;
	char *realname;

} ast_codec_alias_table[] = {
	{"slinear","slin"},
	{"g723.1","g723"},
};

static char *ast_expand_codec_alias(char *in) {
	int x = 0;

	for (x = 0; x < sizeof(ast_codec_alias_table) / sizeof(struct ast_codec_alias_table) ; x++) {
		if(!strcmp(in,ast_codec_alias_table[x].alias))
			return ast_codec_alias_table[x].realname;
	}
	return in;
}

int ast_getformatbyname(char *name)
{
	int x = 0, all = 0, format = 0;

	all = strcasecmp(name, "all") ? 0 : 1;
	for (x = 0 ; x < sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list) ; x++) {
		if(AST_FORMAT_LIST[x].visible && (all || 
										  !strcasecmp(AST_FORMAT_LIST[x].name,name) ||
										  !strcasecmp(AST_FORMAT_LIST[x].name,ast_expand_codec_alias(name)))) {
			format |= AST_FORMAT_LIST[x].bits;
			if(!all)
				break;
		}
	}

	return format;
}

char *ast_codec2str(int codec) {
	int x = 0;
	char *ret = "unknown";
	for (x = 0 ; x < sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list) ; x++) {
		if(AST_FORMAT_LIST[x].visible && AST_FORMAT_LIST[x].bits == codec) {
			ret = AST_FORMAT_LIST[x].desc;
			break;
		}
	}
	return ret;
}

static int show_codecs(int fd, int argc, char *argv[])
{
	int i, found=0;
	char hex[25];
	
	if ((argc < 2) || (argc > 3))
		return RESULT_SHOWUSAGE;

	if (!option_dontwarn)
		ast_cli(fd, "Disclaimer: this command is for informational purposes only.\n"
				"\tIt does not indicate anything about your configuration.\n");

	ast_cli(fd, "%11s %9s %10s   TYPE   %5s   %s\n","INT","BINARY","HEX","NAME","DESC");
	ast_cli(fd, "--------------------------------------------------------------------------------\n");
	if ((argc == 2) || (!strcasecmp(argv[1],"audio"))) {
		found = 1;
		for (i=0;i<12;i++) {
			snprintf(hex,25,"(0x%x)",1<<i);
			ast_cli(fd, "%11u (1 << %2d) %10s  audio   %5s   (%s)\n",1 << i,i,hex,ast_getformatname(1<<i),ast_codec2str(1<<i));
		}
	}

	if ((argc == 2) || (!strcasecmp(argv[1],"image"))) {
		found = 1;
		for (i=16;i<18;i++) {
			snprintf(hex,25,"(0x%x)",1<<i);
			ast_cli(fd, "%11u (1 << %2d) %10s  image   %5s   (%s)\n",1 << i,i,hex,ast_getformatname(1<<i),ast_codec2str(1<<i));
		}
	}

	if ((argc == 2) || (!strcasecmp(argv[1],"video"))) {
		found = 1;
		for (i=18;i<21;i++) {
			snprintf(hex,25,"(0x%x)",1<<i);
			ast_cli(fd, "%11u (1 << %2d) %10s  video   %5s   (%s)\n",1 << i,i,hex,ast_getformatname(1<<i),ast_codec2str(1<<i));
		}
	}

	if (! found)
		return RESULT_SHOWUSAGE;
	else
		return RESULT_SUCCESS;
}

static char frame_show_codecs_usage[] =
"Usage: show [audio|video|image] codecs\n"
"       Displays codec mapping\n";

static int show_codec_n(int fd, int argc, char *argv[])
{
	int codec, i, found=0;

	if (argc != 3)
		return RESULT_SHOWUSAGE;

	if (sscanf(argv[2],"%d",&codec) != 1)
		return RESULT_SHOWUSAGE;

	for (i=0;i<32;i++)
		if (codec & (1 << i)) {
			found = 1;
			ast_cli(fd, "%11u (1 << %2d)  %s\n",1 << i,i,ast_codec2str(1<<i));
		}

	if (! found)
		ast_cli(fd, "Codec %d not found\n", codec);

	return RESULT_SUCCESS;
}

static char frame_show_codec_n_usage[] =
"Usage: show codec <number>\n"
"       Displays codec mapping\n";

/*! Dump a frame for debugging purposes */
void ast_frame_dump(char *name, struct ast_frame *f, char *prefix)
{
	char *n = "unknown";
	char ftype[40] = "Unknown Frametype";
	char cft[80];
	char subclass[40] = "Unknown Subclass";
	char csub[80];
	char moreinfo[40] = "";
	char cn[60];
	char cp[40];
	char cmn[40];
	if (name)
		n = name;
	if (!f) {
		ast_verbose("%s [ %s (NULL) ] [%s]\n", 
			term_color(cp, prefix, COLOR_BRMAGENTA, COLOR_BLACK, sizeof(cp)),
			term_color(cft, "HANGUP", COLOR_BRRED, COLOR_BLACK, sizeof(cft)), 
			term_color(cn, n, COLOR_YELLOW, COLOR_BLACK, sizeof(cn)));
		return;
	}
	/* XXX We should probably print one each of voice and video when the format changes XXX */
	if (f->frametype == AST_FRAME_VOICE)
		return;
	if (f->frametype == AST_FRAME_VIDEO)
		return;
	switch(f->frametype) {
	case AST_FRAME_DTMF_BEGIN:
		strcpy(ftype, "DTMF Begin");
		subclass[0] = f->subclass;
		subclass[1] = '\0';
		break;
	case AST_FRAME_DTMF_END:
		strcpy(ftype, "DTMF End");
		subclass[0] = f->subclass;
		subclass[1] = '\0';
		break;
	case AST_FRAME_CONTROL:
		strcpy(ftype, "Control");
		switch(f->subclass) {
		case AST_CONTROL_HANGUP:
			strcpy(subclass, "Hangup");
			break;
		case AST_CONTROL_RING:
			strcpy(subclass, "Ring");
			break;
		case AST_CONTROL_RINGING:
			strcpy(subclass, "Ringing");
			break;
		case AST_CONTROL_ANSWER:
			strcpy(subclass, "Answer");
			break;
		case AST_CONTROL_BUSY:
			strcpy(subclass, "Busy");
			break;
		case AST_CONTROL_TAKEOFFHOOK:
			strcpy(subclass, "Take Off Hook");
			break;
		case AST_CONTROL_OFFHOOK:
			strcpy(subclass, "Line Off Hook");
			break;
		case AST_CONTROL_CONGESTION:
			strcpy(subclass, "Congestion");
			break;
		case AST_CONTROL_CALL_LIMIT:
			strcpy(subclass, "Calllimit");
			break;
		case AST_CONTROL_FLASH:
			strcpy(subclass, "Flash");
			break;
		case AST_CONTROL_WINK:
			strcpy(subclass, "Wink");
			break;
		case AST_CONTROL_OPTION:
			strcpy(subclass, "Option");
			break;
		case AST_CONTROL_RADIO_KEY:
			strcpy(subclass, "Key Radio");
			break;
		case AST_CONTROL_RADIO_UNKEY:
			strcpy(subclass, "Unkey Radio");
			break;
		case -1:
			strcpy(subclass, "Stop generators");
			break;
		default:
			snprintf(subclass, sizeof(subclass), "Unknown control '%d'", f->subclass);
		}
		break;
	case AST_FRAME_NULL:
		strcpy(ftype, "Null Frame");
		strcpy(subclass, "N/A");
		break;
	case AST_FRAME_IAX:
		/* Should never happen */
		strcpy(ftype, "IAX Specific");
		snprintf(subclass, sizeof(subclass), "IAX Frametype %d", f->subclass);
		break;
	case AST_FRAME_TEXT:
		strcpy(ftype, "Text");
		strcpy(subclass, "N/A");
		ast_copy_string(moreinfo, f->data, sizeof(moreinfo));
		break;
	case AST_FRAME_IMAGE:
		strcpy(ftype, "Image");
		snprintf(subclass, sizeof(subclass), "Image format %s\n", ast_getformatname(f->subclass));
		break;
	case AST_FRAME_HTML:
		strcpy(ftype, "HTML");
		switch(f->subclass) {
		case AST_HTML_URL:
			strcpy(subclass, "URL");
			ast_copy_string(moreinfo, f->data, sizeof(moreinfo));
			break;
		case AST_HTML_DATA:
			strcpy(subclass, "Data");
			break;
		case AST_HTML_BEGIN:
			strcpy(subclass, "Begin");
			break;
		case AST_HTML_END:
			strcpy(subclass, "End");
			break;
		case AST_HTML_LDCOMPLETE:
			strcpy(subclass, "Load Complete");
			break;
		case AST_HTML_NOSUPPORT:
			strcpy(subclass, "No Support");
			break;
		case AST_HTML_LINKURL:
			strcpy(subclass, "Link URL");
			ast_copy_string(moreinfo, f->data, sizeof(moreinfo));
			break;
		case AST_HTML_UNLINK:
			strcpy(subclass, "Unlink");
			break;
		case AST_HTML_LINKREJECT:
			strcpy(subclass, "Link Reject");
			break;
		default:
			snprintf(subclass, sizeof(subclass), "Unknown HTML frame '%d'\n", f->subclass);
			break;
		}
		break;
	default:
		snprintf(ftype, sizeof(ftype), "Unknown Frametype '%d'", f->frametype);
	}
	if (!ast_strlen_zero(moreinfo))
		ast_verbose("%s [ TYPE: %s (%d) SUBCLASS: %s (%d) '%s' ] [%s]\n",  
			term_color(cp, prefix, COLOR_BRMAGENTA, COLOR_BLACK, sizeof(cp)),
			term_color(cft, ftype, COLOR_BRRED, COLOR_BLACK, sizeof(cft)),
			f->frametype, 
			term_color(csub, subclass, COLOR_BRCYAN, COLOR_BLACK, sizeof(csub)),
			f->subclass, 
			term_color(cmn, moreinfo, COLOR_BRGREEN, COLOR_BLACK, sizeof(cmn)),
			term_color(cn, n, COLOR_YELLOW, COLOR_BLACK, sizeof(cn)));
	else
		ast_verbose("%s [ TYPE: %s (%d) SUBCLASS: %s (%d) ] [%s]\n",  
			term_color(cp, prefix, COLOR_BRMAGENTA, COLOR_BLACK, sizeof(cp)),
			term_color(cft, ftype, COLOR_BRRED, COLOR_BLACK, sizeof(cft)),
			f->frametype, 
			term_color(csub, subclass, COLOR_BRCYAN, COLOR_BLACK, sizeof(csub)),
			f->subclass, 
			term_color(cn, n, COLOR_YELLOW, COLOR_BLACK, sizeof(cn)));

}


#ifdef TRACE_FRAMES
static int show_frame_stats(int fd, int argc, char *argv[])
{
	struct ast_frame *f;
	int x=1;
	if (argc != 3)
		return RESULT_SHOWUSAGE;
	ast_cli(fd, "     Framer Statistics     \n");
	ast_cli(fd, "---------------------------\n");
	ast_cli(fd, "Total allocated headers: %d\n", headers);
	ast_cli(fd, "Queue Dump:\n");
	ast_mutex_lock(&framelock);
	for (f=headerlist; f; f = f->next) {
		ast_cli(fd, "%d.  Type %d, subclass %d from %s\n", x++, f->frametype, f->subclass, f->src ? f->src : "<Unknown>");
	}
	ast_mutex_unlock(&framelock);
	return RESULT_SUCCESS;
}

static char frame_stats_usage[] =
"Usage: show frame stats\n"
"       Displays debugging statistics from framer\n";
#endif

/* Builtin Asterisk CLI-commands for debugging */
static struct ast_cli_entry my_clis[] = {
{ { "show", "codecs", NULL }, show_codecs, "Shows codecs", frame_show_codecs_usage },
{ { "show", "audio", "codecs", NULL }, show_codecs, "Shows audio codecs", frame_show_codecs_usage },
{ { "show", "video", "codecs", NULL }, show_codecs, "Shows video codecs", frame_show_codecs_usage },
{ { "show", "image", "codecs", NULL }, show_codecs, "Shows image codecs", frame_show_codecs_usage },
{ { "show", "codec", NULL }, show_codec_n, "Shows a specific codec", frame_show_codec_n_usage },
#ifdef TRACE_FRAMES
{ { "show", "frame", "stats", NULL }, show_frame_stats, "Shows frame statistics", frame_stats_usage },
#endif
};

int init_framer(void)
{
	ast_cli_register_multiple(my_clis, sizeof(my_clis)/sizeof(my_clis[0]) );
	return 0;	
}

void ast_codec_pref_convert_to_buf(const struct ast_codec_pref *pref, char *buf, size_t size)
{
	int limit = size < 32 ? size : 32;
	int i;
	int x = 0;

	memset(buf, 0, size);
	for (i = 0; i < limit; ++i)
	{
		if (!pref->audio_order[i].slot)
			break;
		buf[i] = pref->audio_order[i].slot + (int) 'A';
	}
	x = i;
	for (i = 0; (i + x) < limit; ++i)
       	{
		if (!pref->video_order[i].slot)
			break;
		buf[i + x] = pref->video_order[i].slot + (int) 'A';
	}
}

void ast_codec_pref_convert_from_buf(struct ast_codec_pref *pref, char *buf, size_t size)
{
	int i;
	int limit = size < 32 ? size : 32;
	int slot;
	static int max_slot = sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list);

	ast_codec_pref_init(pref);
	for (i = 0; i < limit; ++i)
	{
		if (!buf[i])
			break;
		slot = buf[i] - 1 - (int) 'A';
		if (slot >= 0 && slot < max_slot) {
			ast_codec_pref_append(pref, AST_FORMAT_LIST[slot].bits);
		}
	}
}

char *ast_codec_pref_dump(char *buf, size_t size, const struct ast_codec_pref *pref) 
{
	ast_codec_pref_string(pref, buf, size);
	return buf;
}

int ast_codec_pref_string(const struct ast_codec_pref *pref, char *buf, size_t size) 
{
	int x = 0, codec = 0; 
	size_t total_len = 0, slen = 0;
	char *formatname = 0;
	
	memset(buf,0,size);
	total_len = size;
	buf[0] = '(';
	total_len--;
	for(x = 0; x < 32 ; x++) {
		if(total_len <= 0)
			break;
		if(!(codec = ast_codec_pref_index_audio(pref,x)))
			break;
		if((formatname = ast_getformatname(codec))) {
			slen = strlen(formatname);
			if(slen > total_len)
				break;
			strncat(buf,formatname,total_len);
			total_len -= slen;
		}
		if(total_len && x < 31 && ast_codec_pref_index_audio(pref , x + 1)) {
			strncat(buf,"|",total_len);
			total_len--;
		}
	}
	if ((slen <= total_len) && pref->video_bits) {
		strncat(buf,"|",total_len);
		total_len--;
	}
	for(x = 0; x < 32 ; x++) {
		if(total_len <= 0)
			break;
		if(!(codec = ast_codec_pref_index_video(pref,x)))
			break;
		if((formatname = ast_getformatname(codec))) {
			slen = strlen(formatname);
			if(slen > total_len)
				break;
			strncat(buf,formatname,total_len);
			total_len -= slen;
		}
		if(total_len && x < 31 && ast_codec_pref_index_video(pref , x + 1)) {
			strncat(buf,"|",total_len);
			total_len--;
		}
	}
	if(total_len) {
		strncat(buf,")",total_len);
		total_len--;
	}

	return size - total_len;
}

int ast_codec_pref_index_audio(const struct ast_codec_pref *pref, int index) 
{
	int slot = 0;
	int size = sizeof(pref->audio_order) / sizeof(struct ast_codec_pref_item);

	
	if((index >= 0) && (index < size)) {
		slot = pref->audio_order[index].slot;
	}

	return slot ? AST_FORMAT_LIST[slot-1].bits : 0;
}

int ast_codec_pref_index_video(const struct ast_codec_pref *pref, int index) 
{
	int slot = 0;
	int size = sizeof(pref->video_order) / sizeof(struct ast_codec_pref_item);
	
	if((index >= 0) && (index < size)) {
		slot = pref->video_order[index].slot;
	}

	return slot ? AST_FORMAT_LIST[slot-1].bits : 0;
}

int ast_codec_pref_bits(const struct ast_codec_pref *pref) {
	return pref->audio_bits | pref->video_bits;
}

/*! \brief ast_codec_pref_remove: Remove codec from pref list ---*/
void ast_codec_pref_remove(struct ast_codec_pref *pref, int format)
{
	struct ast_codec_pref oldorder;
	int x=0, y=0;
	size_t size = 0;
	int slot = 0;

	if (!(pref->audio_order[0].slot + pref->video_order[0].slot) ||
	    !(ast_codec_pref_bits(pref) & format))
	{
		return;
	}

	size = sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list);

	memcpy(&oldorder,pref,sizeof(struct ast_codec_pref));
	memset(pref,0,sizeof(struct ast_codec_pref));

	for (x = 0; x < size; x++) {
		slot = oldorder.audio_order[x].slot;
		if(! slot)
			break;
		if(AST_FORMAT_LIST[slot-1].bits != format) {
			pref->audio_order[y++].slot = slot;
			pref->audio_bits |= AST_FORMAT_LIST[slot - 1].bits;
		}
	}
	for (y = 0, x = 0; x < size; x++) {
		slot = oldorder.video_order[x].slot;
		if(! slot)
			break;
		if(AST_FORMAT_LIST[slot-1].bits != format) {
			pref->video_order[y++].slot = slot;
			pref->video_bits |= AST_FORMAT_LIST[slot - 1].bits;
		}
	}
	
}

/*! \brief ast_codec_pref_append: Append codec to list ---*/
int ast_codec_pref_append_ex(struct ast_codec_pref *pref, int format,
	int *ptime_arr, int ptime_arr_size)
{
	size_t size = 0;
	int x = 0, newindex = 0;
	struct ast_codec_pref_item *order = format < AST_FORMAT_MAX_AUDIO ?
					pref->audio_order :
				       	pref->video_order;
	int *bits = format < AST_FORMAT_MAX_AUDIO ?
					&pref->audio_bits :
					&pref->video_bits;
	if (!format)
		return 0;

	size = sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list);

	for (x = 0; x < size; x++) {
		if(AST_FORMAT_LIST[x].bits == format) {
			newindex = x + 1;
			break;
		}
	}

	if(newindex) {
		for (x = 0; x < size; x++) {
		        /* if the format already exists, just exit */
		        if (order[x].slot == newindex)
				break;
			if(!order[x].slot) {
				order[x].slot = newindex;
				if (ptime_arr)
				{
					memcpy(&order[x].ptime_list, ptime_arr,
						sizeof(order[x].ptime_list));
				}
				order[x].num_of_ptimes = ptime_arr_size;
				*bits |= format;
				break;
			}
		}
	}

	return x;
}


int ast_codec_pref_append(struct ast_codec_pref *pref, int format)
{
    return ast_codec_pref_append_ex(pref, format, NULL, 0);
}

/*! \brief ast_codec_choose: Pick a codec ---*/
int ast_codec_choose(struct ast_codec_pref *pref, int formats, int find_best)
{
	size_t size = 0;
	int x = 0, ret = 0, slot = 0;

	size = sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list);
	for (x = 0; x < size; x++) {
		slot = pref->audio_order[x].slot;

		if(!slot)
			break;
		if ( formats & AST_FORMAT_LIST[slot-1].bits ) {
			ret = AST_FORMAT_LIST[slot-1].bits;
			break;
		}
	}
	if(ret)
		return ret;

	for (x = 0; x < size; x++) {
		slot = pref->video_order[x].slot;

		if(!slot)
			break;
		if ( formats & AST_FORMAT_LIST[slot-1].bits ) {
			ret = AST_FORMAT_LIST[slot-1].bits;
			break;
		}
	}
	if(ret)
		return ret;

   	return find_best ? ast_best_codec(formats) : 0;
}

int ast_get_ptime_by_format(struct ast_codec_pref *pref, int format)
{
	size_t size = 0;
	int x = 0, slot = 0;

	size = sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list);
	for (x = 0; x < size; x++) {
		slot = pref->audio_order[x].slot;

		if(!slot)
			break;
		if ( format == AST_FORMAT_LIST[slot-1].bits ) {
			return pref->audio_order[x].ptime_list[0];
		}
	}
	return 0;
}

int ast_parse_codec_list(const char *list)
{
	char *single_codec;
	int format, codec_list = 0;
	char *parse = ast_strdupa(list);

	while ((single_codec = strsep(&parse, ",")))
	{
		ast_log(LOG_DEBUG,"Current codec - %s\n", single_codec);
		if (!(format = ast_getformatbyname(single_codec)))
		{
			ast_log(LOG_WARNING, "%s is unknown\n", single_codec);
			continue;
		}

		codec_list |= format;
	}

	return codec_list;
}

void ast_parse_allow_disallow(struct ast_codec_pref *pref, int *mask, const char
	*list, int allowing)
{
	char *parse;
	char *ptime_str = NULL;
	char *ptime_single;
	int format;
	int ptime_arr[8];
	int used = 0;

	parse = ast_strdupa(list);

	if ((ptime_str = strchr(parse, ':'))) {
		*ptime_str = '\0';
		ptime_str++;
	}
	if (!(format = ast_getformatbyname(parse))) {
		ast_log(LOG_WARNING, "Cannot %s allow unknown format '%s'\n", allowing ?
			"allow" : "disallow", parse);
		return;
	}

	if (mask) {
		if (allowing)
			*mask |= format;
		else
			*mask &= ~format;
	}

	while ((ptime_single = strsep(&ptime_str, ",")))
		ptime_arr[used++] = ptime_single ? atoi(ptime_single) : 0;
	
	if (pref) {
		if (strcasecmp(parse, "all")) {
			if (allowing)
				ast_codec_pref_append_ex(pref, format, ptime_arr, used);
			else
				ast_codec_pref_remove(pref, format);
		} else if (!allowing) {
			memset (pref, 0, sizeof(*pref));
		}
	}
}

void ast_codec_pref_append_missing2(struct ast_codec_pref *pref, int newformats)
{
	int i;
	int format;

	for (i = 0; i < 32; ++i) {
		format = (1 << i);
		if (!(format & ast_codec_pref_bits(pref)) &&
		    (format & newformats))
		{
			ast_codec_pref_append(pref, format);
		}
	}
}

void ast_codec_pref_append_missing(struct ast_codec_pref *pref, const struct ast_codec_pref *newformats)
{
	int i;
	int format;
	const int size = sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list);
	int slot;

	for (i = 0; i < size; i++) {
		if (!(slot = newformats->audio_order[i].slot))
			break;
		format = AST_FORMAT_LIST[slot - 1].bits;
		if (!(format & pref->audio_bits))
			ast_codec_pref_append(pref, format);
	}
	for (i = 0; i < size; i++) {
		if (!(slot = newformats->video_order[i].slot))
			break;
		format = AST_FORMAT_LIST[slot - 1].bits;
		if (!(format & pref->video_bits))
			ast_codec_pref_append(pref, format);
	}
}

void ast_codec_pref_init(struct ast_codec_pref *pref)
{
	memset(pref, 0, sizeof(*pref));
}

void ast_codec_pref_set2(struct ast_codec_pref *pref, int formats)
{
	ast_codec_pref_init(pref);
	ast_codec_pref_append_missing2(pref, formats);
}

int ast_codec_pref_intersect(const struct ast_codec_pref *pref1, const struct ast_codec_pref *pref2)
{
	return (pref1->video_bits | pref1->audio_bits) &
		(pref2->video_bits | pref2->audio_bits);
}

int ast_codec_pref_eq_noorder(const struct ast_codec_pref *pref1, const struct ast_codec_pref *pref2)
{
	return (pref1->video_bits | pref1->audio_bits) ==
		(pref2->video_bits | pref2->audio_bits);
}

void ast_codec_pref_combine(struct ast_codec_pref *dest, 
			    const struct ast_codec_pref *main, 
			    const int mask)
{
	const int size = sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list);
	int i;
	int slot;
	int format;

	ast_codec_pref_init(dest);
	for (i = 0; i < size; ++i)
	{
		if (!(slot = main->audio_order[i].slot))
			break;
		format = AST_FORMAT_LIST[slot - 1].bits;
		if (mask & format)
			ast_codec_pref_append(dest, format);
	}
	for (i = 0; i < size; ++i)
	{
		if (!(slot = main->video_order[i].slot))
			break;
		format = AST_FORMAT_LIST[slot - 1].bits;
		if (mask & format)
			ast_codec_pref_append(dest, format);
	}
}

void ast_codec_pref_set_top_quality(struct ast_codec_pref *pref, ast_codec_quality q)
{
    	struct ast_codec_pref oldorder;
    	int x, y;
    	size_t size = sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list);
    	struct ast_codec_pref_item pref_item;

    	memcpy(oldorder.audio_order,pref->audio_order,sizeof(pref->audio_order));
    	memset(pref->audio_order,0,sizeof(pref->audio_order));
    	pref->audio_bits = 0;

    	for (x = 0, y = 0; x < size; x++) {
		pref_item = oldorder.audio_order[x];
		if(!pref_item.slot)
	    		break;
		if(AST_FORMAT_LIST[pref_item.slot-1].quality <= q) {
	    		pref->audio_order[y++] = pref_item;
	    	pref->audio_bits |= AST_FORMAT_LIST[pref_item.slot-1].bits;
		}
    	}
}

ast_codec_quality ast_codec_get_quality(int format)
{
    	int x;

    	for (x = 0; x < sizeof(AST_FORMAT_LIST) / sizeof(struct ast_format_list); x++) {
		if(AST_FORMAT_LIST[x].visible && AST_FORMAT_LIST[x].bits == format)
	    		return AST_FORMAT_LIST[x].quality;
    	}
    	return 0;
}

void ast_codec_pref_remove2(struct ast_codec_pref *pref, int formats)
{
	int i;
	int format;

	for (i = 0; i < 32; ++i)
	{
		format = (1 << i);
		if (format & formats & ast_codec_pref_bits(pref))
			ast_codec_pref_remove(pref, format);
	}
}

static int g723_len(unsigned char buf)
{
	switch(buf & TYPE_MASK) {
	case TYPE_DONTSEND:
		return 0;
		break;
	case TYPE_SILENCE:
		return 4;
		break;
	case TYPE_HIGH:
		return 24;
		break;
	case TYPE_LOW:
		return 20;
		break;
	default:
		ast_log(LOG_WARNING, "Badly encoded frame (%d)\n", buf & TYPE_MASK);
	}
	return -1;
}

static int g723_samples(unsigned char *buf, int maxlen)
{
	int pos = 0;
	int samples = 0;
	int res;
	while(pos < maxlen) {
		res = g723_len(buf[pos]);
		if (res <= 0)
			break;
		samples += 240;
		pos += res;
	}
	return samples;
}

static unsigned char get_n_bits_at(unsigned char *data, int n, int bit)
{
	int byte = bit / 8;       /* byte containing first bit */
	int rem = 8 - (bit % 8);  /* remaining bits in first byte */
	unsigned char ret = 0;
	
	if (n <= 0 || n > 8)
		return 0;

	if (rem < n) {
		ret = (data[byte] << (n - rem));
		ret |= (data[byte + 1] >> (8 - n + rem));
	} else {
		ret = (data[byte] >> (rem - n));
	}

	return (ret & (0xff >> (8 - n)));
}

static int speex_get_wb_sz_at(unsigned char *data, int len, int bit)
{
	static int SpeexWBSubModeSz[] = {
		0, 36, 112, 192,
		352, 0, 0, 0 };
	int off = bit;
	unsigned char c;

	/* skip up to two wideband frames */
	if (((len * 8 - off) >= 5) && 
		get_n_bits_at(data, 1, off)) {
		c = get_n_bits_at(data, 3, off + 1);
		off += SpeexWBSubModeSz[c];

		if (((len * 8 - off) >= 5) && 
			get_n_bits_at(data, 1, off)) {
			c = get_n_bits_at(data, 3, off + 1);
			off += SpeexWBSubModeSz[c];

			if (((len * 8 - off) >= 5) && 
				get_n_bits_at(data, 1, off)) {
				ast_log(LOG_WARNING, "Encountered corrupt speex frame; too many wideband frames in a row.\n");
				return -1;
			}
		}

	}
	return off - bit;
}

static int speex_samples(unsigned char *data, int len)
{
	static int SpeexSubModeSz[] = {
               5, 43, 119, 160,
		220, 300, 364, 492, 
		79, 0, 0, 0,
		0, 0, 0, 0 };
	static int SpeexInBandSz[] = { 
		1, 1, 4, 4,
		4, 4, 4, 4,
		8, 8, 16, 16,
		32, 32, 64, 64 };
	int bit = 0;
	int cnt = 0;
	int off = 0;
	unsigned char c;

	while ((len * 8 - bit) >= 5) {
		/* skip wideband frames */
		off = speex_get_wb_sz_at(data, len, bit);
		if (off < 0)  {
			ast_log(LOG_WARNING, "Had error while reading wideband frames for speex samples\n");
			break;
		}
		bit += off;

		if ((len * 8 - bit) < 5) {
			ast_log(LOG_WARNING, "Not enough bits remaining after wide band for speex samples.\n");
			break;
		}

		/* get control bits */
		c = get_n_bits_at(data, 5, bit);
		bit += 5;

		if (c == 15) { 
			/* terminator */
			break; 
		} else if (c == 14) {
			/* in-band signal; next 4 bits contain signal id */
			c = get_n_bits_at(data, 4, bit);
			bit += 4;
			bit += SpeexInBandSz[c];
		} else if (c == 13) {
			/* user in-band; next 5 bits contain msg len */
			c = get_n_bits_at(data, 5, bit);
			bit += 5;
			bit += c * 8;
		} else if (c > 8) {
			/* unknown */
			break;
		} else {
			/* skip number bits for submode (less the 5 control bits) */
			bit += SpeexSubModeSz[c] - 5;
			cnt += 160; /* new frame */
		}
	}
	return cnt;
}

int ast_codec_get_samples(struct ast_frame *f)
{
	int samples=0;
	switch(f->subclass) {
	case AST_FORMAT_SPEEX:
		samples = speex_samples(f->data, f->datalen);
		break;
	case AST_FORMAT_G723_1:
                samples = g723_samples(f->data, f->datalen);
		break;
	case AST_FORMAT_ILBC:
		samples = 240 * (f->datalen / 50);
		break;
	case AST_FORMAT_GSM:
		samples = 160 * (f->datalen / 33);
		break;
	case AST_FORMAT_G729A:
		samples = f->datalen * 8;
		break;
	case AST_FORMAT_SLINEAR:
	case AST_FORMAT_SLINEAR16:
		samples = f->datalen / 2;
		break;
	case AST_FORMAT_LPC10:
                /* assumes that the RTP packet contains one LPC10 frame */
		samples = 22 * 8;
		samples += (((char *)(f->data))[7] & 0x1) * 8;
		break;
	case AST_FORMAT_ULAW:
	case AST_FORMAT_ALAW:
	case AST_FORMAT_G722:
		samples = f->datalen;
		break;
	case AST_FORMAT_ADPCM:
	case AST_FORMAT_G726:
		samples = f->datalen * 2;
		break;
	default:
		ast_log(LOG_WARNING, "Unable to calculate samples for format %s\n", ast_getformatname(f->subclass));
	}
	return samples;
}

int ast_codec_get_len(int format, int samples)
{
	int len = 0;

	/* XXX Still need speex, g723, and lpc10 XXX */	
	switch(format) {
	case AST_FORMAT_ILBC:
		len = (samples / 240) * 50;
		break;
	case AST_FORMAT_GSM:
		len = (samples / 160) * 33;
		break;
	case AST_FORMAT_G729A:
		len = samples / 8;
		break;
	case AST_FORMAT_SLINEAR:
	case AST_FORMAT_SLINEAR16:
		len = samples * 2;
		break;
	case AST_FORMAT_ULAW:
	case AST_FORMAT_ALAW:
		len = samples;
		break;
	case AST_FORMAT_ADPCM:
	case AST_FORMAT_G726:
		len = samples / 2;
		break;
	default:
		ast_log(LOG_WARNING, "Unable to calculate sample length for format %s\n", ast_getformatname(format));
	}

	return len;
}

int ast_frame_adjust_volume(struct ast_frame *f, int adjustment)
{
	int count;
	short *fdata = f->data;
	short adjust_value = abs(adjustment);

	if ((f->frametype != AST_FRAME_VOICE) || (f->subclass != AST_FORMAT_SLINEAR))
		return -1;

	if (!adjustment)
		return 0;

	for (count = 0; count < f->samples; count++) {
		if (adjustment > 0) {
			ast_slinear_saturated_multiply(&fdata[count], &adjust_value);
		} else if (adjustment < 0) {
			ast_slinear_saturated_divide(&fdata[count], &adjust_value);
		}
	}

	return 0;
}

int ast_frame_slinear_sum(struct ast_frame *f1, struct ast_frame *f2)
{
	int count;
	short *data1, *data2;

	if ((f1->frametype != AST_FRAME_VOICE) || (f1->subclass != AST_FORMAT_SLINEAR))
		return -1;

	if ((f2->frametype != AST_FRAME_VOICE) || (f2->subclass != AST_FORMAT_SLINEAR))
		return -1;

	if (f1->samples != f2->samples)
		return -1;

	for (count = 0, data1 = f1->data, data2 = f2->data;
	     count < f1->samples;
	     count++, data1++, data2++)
		ast_slinear_saturated_add(data1, data2);

	return 0;
}
