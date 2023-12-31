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
 * \brief Asterisk internal frame definitions.
 * \arg For an explanation of frames, see \ref Def_Frame
 * \arg Frames are send of Asterisk channels, see \ref Def_Channel
 */

#ifndef _ASTERISK_FRAME_H
#define _ASTERISK_FRAME_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <sys/types.h>
#include <sys/time.h>
#include "asterisk/endian.h"

struct ast_codec_pref_item {
	char slot; /* codec index */
	int ptime_list[8]; /* List of supported packetization times */
	int num_of_ptimes; /* Number of used packetization times */
};

struct ast_codec_pref {
	struct ast_codec_pref_item audio_order[32];
	struct ast_codec_pref_item video_order[32];
	int audio_bits;
	int video_bits;
};

#define MAX_QUALITY 1
typedef int ast_codec_quality;

/*! \page Def_Frame AST Multimedia and signalling frames
	\section Def_AstFrame What is an ast_frame ?
 	A frame of data read used to communicate between 
 	between channels and applications.
	Frames are divided into frame types and subclasses.

	\par Frame types 
	\arg \b VOICE:	Voice data, subclass is codec (AST_FORMAT_*)
	\arg \b VIDEO:	Video data, subclass is codec (AST_FORMAT_*)
	\arg \b DTMF:	A DTMF digit, subclass is the digit
	\arg \b IMAGE:	Image transport, mostly used in IAX
	\arg \b TEXT:	Text messages
	\arg \b HTML:	URL's and web pages
	\arg \b T38:	T38 Fax transport frames
	\arg \b IAX:	Private frame type for the IAX protocol
	\arg \b CNG:	Comfort noice frames
	\arg \b CONTROL:	A control frame, subclass defined as AST_CONTROL_
	\arg \b NULL:	Empty, useless frame

	\par Files
	\arg frame.h	Definitions
	\arg frame.c	Function library
	\arg \ref Def_Channel Asterisk channels
	\section Def_ControlFrame Control Frames
	Control frames send signalling information between channels
	and devices. They are prefixed with AST_CONTROL_, like AST_CONTROL_FRAME_HANGUP
	\arg \b HANGUP	The other end has hungup
	\arg \b RING	Local ring
	\arg \b RINGING	The other end is ringing
	\arg \b ANSWER	The other end has answered
	\arg \b BUSY	Remote end is busy
	\arg \b TAKEOFFHOOK	Make it go off hook (what's "it" ? )
	\arg \b OFFHOOK	Line is off hook
	\arg \b CONGESTION	Congestion (circuit is busy, not available)
	\arg \b FLASH	Other end sends flash hook
	\arg \b WINK	Other end sends wink
	\arg \b OPTION	Send low-level option
	\arg \b RADIO_KEY	Key radio (see app_rpt.c)
	\arg \b RADIO_UNKEY	Un-key radio (see app_rpt.c)
	\arg \b PROGRESS	Other end indicates call progress
	\arg \b PROCEEDING	Indicates proceeding
	\arg \b HOLD	Call is placed on hold
	\arg \b UNHOLD	Call is back from hold
	\arg \b VIDUPDATE	Video update requested

*/

/*! \brief Data structure associated with a single frame of data
 */
struct ast_frame {
	/*! Kind of frame */
	int frametype;				
	/*! Subclass, frame dependent */
	int subclass;				
	/*! Length of data */
	int datalen;				
	/*! Number of 8khz samples in this frame */
	int samples;				
	/*! Was the data malloc'd?  i.e. should we free it when we discard the frame? */
	int mallocd;				
	/*! How many bytes exist _before_ "data" that can be used if needed */
	int offset;				
	/*! Optional source of frame for debugging */
	const char *src;				
	/*! Pointer to actual data */
	void *data;		
	/*! Global delivery time */		
	struct timeval delivery;
	/*! Next/Prev for linking stand alone frames */
	struct ast_frame *prev;			
	/*! Next/Prev for linking stand alone frames */
	struct ast_frame *next;			
};

typedef enum {
	AST_TRANSPARENT_UNKNOWN = 0,
	AST_TRANSPARENT_ISDN_FACILITY = 1,
        AST_TRANSPARENT_SIP_RESPONSE = 2,
} ast_transparent_frame_code_t;

typedef struct ast_transparent_frame_payload {
	/*! Payload code enum */
	ast_transparent_frame_code_t code;
	/*! Payload data */
	void *data;
	/*! Size of payload data */
	int data_size;
} ast_transparent_frame_payload;

typedef enum {
  AST_CALLBACK_IN_PROGRESS = 0,
  AST_CALLBACK_ANSWERED = 1,
} ast_callback_frame_state_t;

typedef struct ast_callback_frame_payload {
	/* current callback state */
	ast_callback_frame_state_t state; 
} ast_callback_frame_payload_t;

#define AST_FRIENDLY_OFFSET 	64	/*! It's polite for a a new frame to
					  have this number of bytes for additional
					  headers.  */
#define AST_MIN_OFFSET 		32	/*! Make sure we keep at least this much handy */

/* special result for callback - should be returned from ...XXX */
#define AST_RESULT_CALLBACK -2

/*! Need the header be free'd? */
#define AST_MALLOCD_HDR		(1 << 0)
/*! Need the data be free'd? */
#define AST_MALLOCD_DATA	(1 << 1)
/*! Need the source be free'd? (haha!) */
#define AST_MALLOCD_SRC		(1 << 2)

/* Frame types */
/*! DTMF end event, subclass is the digit */
#define AST_FRAME_DTMF_END	1
/*! Voice data, subclass is AST_FORMAT_* */
#define AST_FRAME_VOICE		2
/*! Video frame, maybe?? :) */
#define AST_FRAME_VIDEO		3
/*! A control frame, subclass is AST_CONTROL_* */
#define AST_FRAME_CONTROL	4
/*! An empty, useless frame */
#define AST_FRAME_NULL		5
/*! Inter Asterisk Exchange private frame type */
#define AST_FRAME_IAX		6
/*! Text messages */
#define AST_FRAME_TEXT		7
/*! Image Frames */
#define AST_FRAME_IMAGE		8
/*! HTML Frame */
#define AST_FRAME_HTML		9
/*! Comfort Noise frame (subclass is level of CNG in -dBov), 
    body may include zero or more 8-bit quantization coefficients */
#define AST_FRAME_CNG		10
#if defined(T38_SUPPORT)
/*! Modem-over-IP data streams */
#define AST_FRAME_MODEM		11

/* MODEM subclasses */
/*! T.38 Fax-over-IP */
#define AST_MODEM_T38		1
/*! V.150 Modem-over-IP */
#define AST_MODEM_V150		2
#endif

/*! Call waiting frame, subclass is AST_CALLWAITING_*, data is caller ID */
#define AST_FRAME_CALLWAITING	12
/*! Request for blind call transfer */
#define AST_FRAME_BLINDTRANSFER	13
/*! Request for attended call transfer */
#define AST_FRAME_ATTENDEDTRANSFER 14
/*! DTMF begin event, subclass is the digit */
#define AST_FRAME_DTMF_BEGIN	15
/*! A DTMF digit, subclass is the digit */
#define AST_FRAME_DTMF AST_FRAME_DTMF_END

/* HTML subclasses */
/*! Sending a URL */
#define AST_HTML_URL		1
/*! Data frame */
#define AST_HTML_DATA		2
/*! Beginning frame */
#define AST_HTML_BEGIN		4
/*! End frame */
#define AST_HTML_END		8
/*! Load is complete */
#define AST_HTML_LDCOMPLETE	16
/*! Peer is unable to support HTML */
#define AST_HTML_NOSUPPORT	17
/*! Send URL, and track */
#define AST_HTML_LINKURL	18
/*! No more HTML linkage */
#define AST_HTML_UNLINK		19
/*! Reject link request */
#define AST_HTML_LINKREJECT	20

/*! Indicate that there is a waiting call */
#define AST_CALLWAITING_START	1
/*! Indicate that there is no waiting call */
#define AST_CALLWAITING_STOP	2

/* Data formats for capabilities and frames alike */
/*! G.723.1 compression */
#define AST_FORMAT_G723_1	(1 << 0)
/*! GSM compression */
#define AST_FORMAT_GSM		(1 << 1)
/*! Raw mu-law data (G.711) */
#define AST_FORMAT_ULAW		(1 << 2)
/*! Raw A-law data (G.711) */
#define AST_FORMAT_ALAW		(1 << 3)
/*! ADPCM (G.726, 32kbps) */
#define AST_FORMAT_G726		(1 << 4)
/*! ADPCM (IMA) */
#define AST_FORMAT_ADPCM	(1 << 5)
/*! Raw 16-bit Signed Linear (8000 Hz) PCM */
#define AST_FORMAT_SLINEAR	(1 << 6)
/*! LPC10, 180 samples/frame */
#define AST_FORMAT_LPC10	(1 << 7)
/*! G.729A audio */
#define AST_FORMAT_G729A	(1 << 8)
/*! SpeeX Free Compression */
#define AST_FORMAT_SPEEX	(1 << 9)
/*! iLBC Free Compression */
#define AST_FORMAT_ILBC		(1 << 10)
/*! G.722 compression */
#define AST_FORMAT_G722		(1 << 11)
/*! Raw 16-bit Signed Linear (16000 Hz) PCM */
#define AST_FORMAT_SLINEAR16  (1ULL << 12)
/*! 'Comfort Noise' (RFC3389) */
#define AST_FORMAT_CN              (1 << 13)
/*! Maximum audio format */
#define AST_FORMAT_MAX_AUDIO	(1 << 15)
/*! JPEG Images */
#define AST_FORMAT_JPEG		(1 << 16)
/*! PNG Images */
#define AST_FORMAT_PNG		(1 << 17)
/*! H.261 Video */
#define AST_FORMAT_H261		(1 << 18)
/*! H.263 Video */
#define AST_FORMAT_H263		(1 << 19)
/*! H.263+ Video */
#define AST_FORMAT_H263_PLUS	(1 << 20)
/*! Maximum video format */
#define AST_FORMAT_MAX_VIDEO	(1 << 24)

/* Control frame types */
/*! Other end has hungup */
#define AST_CONTROL_HANGUP		1
/*! Local ring */
#define AST_CONTROL_RING		2
/*! Remote end is ringing */
#define AST_CONTROL_RINGING 		3
/*! Remote end has answered */
#define AST_CONTROL_ANSWER		4
/*! Remote end is busy */
#define AST_CONTROL_BUSY		5
/*! Make it go off hook */
#define AST_CONTROL_TAKEOFFHOOK		6
/*! Line is off hook */
#define AST_CONTROL_OFFHOOK		7
/*! Congestion (circuits busy) */
#define AST_CONTROL_CONGESTION		8
/*! Flash hook */
#define AST_CONTROL_FLASH		9
/*! Wink */
#define AST_CONTROL_WINK		10
/*! Set a low-level option */
#define AST_CONTROL_OPTION		11
/*! Key Radio */
#define	AST_CONTROL_RADIO_KEY		12
/*! Un-Key Radio */
#define	AST_CONTROL_RADIO_UNKEY		13
/*! Indicate PROGRESS */
#define AST_CONTROL_PROGRESS            14
/*! Indicate CALL PROCEEDING */
#define AST_CONTROL_PROCEEDING		15
/*! Indicate call is placed on hold */
#define AST_CONTROL_HOLD			16
/*! Indicate call is left from hold */
#define AST_CONTROL_UNHOLD			17
/*! Indicate video frame update */
#define AST_CONTROL_VIDUPDATE		18
/*! Indicate fax being sent */
#define AST_CONTROL_FAX			19
/*! Indicate fax being sent using T38 */
#define AST_CONTROL_T38			20
/*! Indicate callback being sent to off-hooked handset */
#define AST_CONTROL_CALLBACK		21
/*! Indicate callback request cancel */
#define AST_CONTROL_CALLBACK_CANCEL	22
/*! Indicate modem being sent */
#define AST_CONTROL_MODEM		23
/*! No answer timeout occured */
#define AST_CONTROL_NOANSWER 24
/*! Indicate unallocated number */
#define AST_CONTROL_UNALLOCATED		25
/*! Indicate call declined */
#define AST_CONTROL_DECLINE 26
/*! Indicate call limit */
#define AST_CONTROL_CALL_LIMIT		27
/*! Indicate diagnostics is to be played */
#define AST_CONTROL_PLAY_DIAGNOSTICS    28
/*! Indicate copy frame data between channels */
#define AST_CONTROL_TRANSPARENT_DATA	29
/* Indicate Register changed */
#define AST_CONTROL_REG_CHANGED		30
/*! Indicate generating ringback tone */
#define AST_CONTROL_RINGBACK_TONE	31

#define AST_SMOOTHER_FLAG_G729		(1 << 0)

/* Option identifiers and flags */
#define AST_OPTION_FLAG_REQUEST		0
#define AST_OPTION_FLAG_ACCEPT		1
#define AST_OPTION_FLAG_REJECT		2
#define AST_OPTION_FLAG_QUERY		4
#define AST_OPTION_FLAG_ANSWER		5
#define AST_OPTION_FLAG_WTF		6


/*! Verify touchtones by muting audio transmission 
	(and reception) and verify the tone is still present */
#define AST_OPTION_TONE_VERIFY		1		

/*! Put a compatible channel into TDD (TTY for the hearing-impared) mode */
#define	AST_OPTION_TDD			2

/*! Relax the parameters for DTMF reception (mainly for radio use) */
#define	AST_OPTION_RELAXDTMF		3

/*! Set (or clear) Audio (Not-Clear) Mode */
#define	AST_OPTION_AUDIO_MODE		4

/*! Set channel transmit gain 
 * Option data is a single signed char
   representing number of decibels (dB)
   to set gain to (on top of any gain
   specified in channel driver)
*/
#define AST_OPTION_TXGAIN		5

/*! Set channel receive gain
 * Option data is a single signed char
   representing number of decibels (dB)
   to set gain to (on top of any gain
   specified in channel driver)
*/
#define AST_OPTION_RXGAIN		6

struct ast_option_header {
	/* Always keep in network byte order */
#if __BYTE_ORDER == __BIG_ENDIAN
        u_int16_t flag:3;
        u_int16_t option:13;
#else
#if __BYTE_ORDER == __LITTLE_ENDIAN
        u_int16_t option:13;
        u_int16_t flag:3;
#else
#error Byte order not defined
#endif
#endif
		u_int8_t data[0];
};

/*! \brief  Requests a frame to be allocated 
 * 
 * \param source 
 * Request a frame be allocated.  source is an optional source of the frame, 
 * len is the requested length, or "0" if the caller will supply the buffer 
 */
#if 0 /* Unimplemented */
struct ast_frame *ast_fralloc(char *source, int len);
#endif

/*!  \brief Frees a frame 
 * \param fr Frame to free
 * Free a frame, and the memory it used if applicable
 * \return no return.
 */
void ast_frfree(struct ast_frame *fr);

/*! \brief Copies a frame 
 * \param fr frame to act upon
 * Take a frame, and if it's not been malloc'd, make a malloc'd copy
 * and if the data hasn't been malloced then make the
 * data malloc'd.  If you need to store frames, say for queueing, then
 * you should call this function.
 * \return Returns a frame on success, NULL on error
 */
struct ast_frame *ast_frisolate(struct ast_frame *fr);

/*! \brief Copies a frame 
 * \param fr frame to copy
 * Dupliates a frame -- should only rarely be used, typically frisolate is good enough
 * \return Returns a frame on success, NULL on error
 */
struct ast_frame *ast_frdup(struct ast_frame *fr);

/*! \brief Reads a frame from an fd
 * Read a frame from a stream or packet fd, as written by fd_write
 * \param fd an opened fd to read from
 * \return returns a frame on success, NULL on error
 */
struct ast_frame *ast_fr_fdread(int fd);

/*! Writes a frame to an fd
 * Write a frame to an fd
 * \param fd Which fd to write to
 * \param frame frame to write to the fd
 * \return Returns 0 on success, -1 on failure
 */
int ast_fr_fdwrite(int fd, struct ast_frame *frame);

/*! \brief Sends a hangup to an fd 
 * Send a hangup (NULL equivalent) on an fd
 * \param fd fd to write to
 * \return Returns 0 on success, -1 on failure
 */
int ast_fr_fdhangup(int fd);

void ast_swapcopy_samples(void *dst, const void *src, int samples);

/* Helpers for byteswapping native samples to/from 
   little-endian and big-endian. */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ast_frame_byteswap_le(fr) do { ; } while(0)
#define ast_frame_byteswap_be(fr) do { struct ast_frame *__f = (fr); ast_swapcopy_samples(__f->data, __f->data, __f->samples); } while(0)
#else
#define ast_frame_byteswap_le(fr) do { struct ast_frame *__f = (fr); ast_swapcopy_samples(__f->data, __f->data, __f->samples); } while(0)
#define ast_frame_byteswap_be(fr) do { ; } while(0)
#endif


/*! \brief Get the name of a format
 * \param format id of format
 * \return A static string containing the name of the format or "UNKN" if unknown.
 */
extern char* ast_getformatname(int format);

/*! \brief Get the names of a set of formats
 * \param buf a buffer for the output string
 * \param size size of buf (bytes)
 * \param format the format (combined IDs of codecs)
 * Prints a list of readable codec names corresponding to "format".
 * ex: for format=AST_FORMAT_GSM|AST_FORMAT_SPEEX|AST_FORMAT_ILBC it will return "0x602 (GSM|SPEEX|ILBC)"
 * \return The return value is buf.
 */
extern char* ast_getformatname_multiple(char *buf, size_t size, int format);

/*!
 * \brief Gets a format from a name.
 * \param name string of format
 * \return This returns the form of the format in binary on success, 0 on error.
 */
extern int ast_getformatbyname(char *name);

/*! \brief Get a name from a format 
 * Gets a name from a format
 * \param codec codec number (1,2,4,8,16,etc.)
 * \return This returns a static string identifying the format on success, 0 on error.
 */
extern char *ast_codec2str(int codec);

struct ast_smoother;

extern struct ast_format_list *ast_get_format_list_index(int index);
extern struct ast_format_list *ast_get_format_list(size_t *size);
extern struct ast_smoother *ast_smoother_new(int bytes);
extern void ast_smoother_set_flags(struct ast_smoother *smoother, int flags);
extern int ast_smoother_get_flags(struct ast_smoother *smoother);
extern void ast_smoother_free(struct ast_smoother *s);
extern void ast_smoother_reset(struct ast_smoother *s, int bytes);
extern int __ast_smoother_feed(struct ast_smoother *s, struct ast_frame *f, int swap);
extern struct ast_frame *ast_smoother_read(struct ast_smoother *s);
#define ast_smoother_feed(s,f) __ast_smoother_feed(s, f, 0)
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ast_smoother_feed_be(s,f) __ast_smoother_feed(s, f, 1)
#define ast_smoother_feed_le(s,f) __ast_smoother_feed(s, f, 0)
#else
#define ast_smoother_feed_be(s,f) __ast_smoother_feed(s, f, 0)
#define ast_smoother_feed_le(s,f) __ast_smoother_feed(s, f, 1)
#endif

extern void ast_frame_dump(char *name, struct ast_frame *f, char *prefix);

/*! \brief Initialize a codec preference to "no preference" */
extern void ast_codec_pref_init(struct ast_codec_pref *pref);

/* Returns the packetization time of the codec matching the given format */
int ast_get_ptime_by_format(struct ast_codec_pref *pref, int format);

/*! \brief Return bitmap of common codecs of the two preferences */
extern int ast_codec_pref_intersect(const struct ast_codec_pref *pref1, const struct ast_codec_pref *pref2);

/*! \brief Returns true if both prefs consists of the same set of codecs disregarding their order */
extern int ast_codec_pref_eq_noorder(const struct ast_codec_pref *pref1, const struct ast_codec_pref *pref2);

/*! \brief Audio codec located at  a particular place in the preference index */
extern int ast_codec_pref_index_audio(const struct ast_codec_pref *pref, int index);

/*! \brief Video codec located at  a particular place in the preference index */
extern int ast_codec_pref_index_video(const struct ast_codec_pref *pref, int index);

/*! \brief Remove a codec from a preference list */
extern void ast_codec_pref_remove(struct ast_codec_pref *pref, int format);

/*! \brief Returns the full bitmap of all codecs */
extern int ast_codec_pref_bits(const struct ast_codec_pref *pref);

/*! \brief Append a codec to a preference list, removing it first if it was already there */
extern int ast_codec_pref_append_ex(struct ast_codec_pref *pref, int format,
	int* ptime_arr, int ptime_arr_size);

/*! \brief Append a codec to a preference list, removing it first if it was already there */
extern int ast_codec_pref_append(struct ast_codec_pref *pref, int format);

/*! \brief Select the best format according to preference list from supplied options. 
   If "find_best" is non-zero then if nothing is found, the "Best" format of 
   the format list is selected, otherwise 0 is returned. */
extern int ast_codec_choose(struct ast_codec_pref *pref, int formats, int find_best);

/*! \brief Parse an "allow" or "deny" line and update the mask and pref if provided */
extern void ast_parse_allow_disallow(struct ast_codec_pref *pref, int *mask, const char *list, int allowing);

/*! \brief Dump codec preference list into a string */
extern int ast_codec_pref_string(const struct ast_codec_pref *pref, char *buf, size_t size);

/*! \brief Dump codec preference list into a string */
extern char *ast_codec_pref_dump(char *buf, size_t size, const struct ast_codec_pref *pref);

/*! \brief Shift a codec preference list up 65 bytes so that it becomes an ASCII string */
extern void ast_codec_pref_convert_to_buf(const struct ast_codec_pref *pref, char *buf, size_t size);
/*! \brief Shift a codec preference list down 65 bytes so that it becomes an codec set */
extern void ast_codec_pref_convert_from_buf(struct ast_codec_pref *pref, char *buf, size_t size);

/*! \brief Returns the number of samples contained in the frame */
extern int ast_codec_get_samples(struct ast_frame *f);

/*! \brief Returns the number of bytes for the number of samples of the given format */
extern int ast_codec_get_len(int format, int samples);

/*! \brief Returns the codec's format as bits according to index*/
extern int ast_get_bits(int index);

/*! \brief Parse a codec list and returns found codecs as bitmask */
extern int ast_parse_codec_list(const char *list);

/*! \brief Gets duration in ms of interpolation frame for a format */
static inline int ast_codec_interp_len(int format) 
{ 
	return (format == AST_FORMAT_ILBC) ? 30 : 20;
}

/*!
 * \brief Append codecs.
 * \param pref Codec set to be modified.
 * \param newformats Bitmap of codecs to be appended.
 *
 * Append codecs from newformats to pref but only those that are not
 * already in the pref.
 */
void ast_codec_pref_append_missing2(struct ast_codec_pref *pref, int newformats);

/*!
 * \brief Append codecs.
 * \param pref Codec set to be modified.
 * \param newformats Codecs to be appended.
 *
 * Append codecs from newformats to pref but only those that are not 
 * already in the pref. Codecs appended after last codec in pref using 
 * order from newformats.
 */
void ast_codec_pref_append_missing(struct ast_codec_pref *pref, const struct ast_codec_pref *newformats);

/*!
 * \brief Clear the pref then append formats given.
 * \param pref Codec set to be set.
 * \param formats Bitmap of codecs to be put to pref.
 *
 * Clears pref and puts given formats into it. Codec order is not well
 * predictable so avoid using this function if codec order is important.
 */
void ast_codec_pref_set2(struct ast_codec_pref *pref, int formats);

/*!
 * \brief Combine codecs from main and mask and put them to dest.
 * \param dest Codec set to be modified.
 * \param main Source codec set.
 * \param mask Bitmap of formats.
 *
 * Set dest to all codecs from main (in that order) but only 
 * those that exist in mask. Previous content of dest is destroyed.
 */
void ast_codec_pref_combine(struct ast_codec_pref *dest, const struct ast_codec_pref *main, const int mask);

/*!
 * \brief Remove high quality codecs.
 * \param pref Codec set to be modified.
 * \param q Remove all codecs above this quality
 *
 * Removes all codecs from codec set pref that are higher than q quality.
 */
void ast_codec_pref_set_top_quality(struct ast_codec_pref *pref, ast_codec_quality q);

/*!
 * \brief Return codec quality
 * \param format Codec format
 * \return The codec quality
 */
ast_codec_quality ast_codec_get_quality(int format);

/*!
 * \brief Remove codecs.
 * \param pref Codec set to be modified.
 * \param formats Bitmap of codecs to be removed.
 *
 * Removes codecs that passed in bitmap formats from codec set pref.
 */
void ast_codec_pref_remove2(struct ast_codec_pref *pref, int formats);

/*!
  \brief Adjusts the volume of the audio samples contained in a frame.
  \param f The frame containing the samples (must be AST_FRAME_VOICE and AST_FORMAT_SLINEAR)
  \param adjustment The number of dB to adjust up or down.
  \return 0 for success, non-zero for an error
 */
int ast_frame_adjust_volume(struct ast_frame *f, int adjustment);

/*!
  \brief Sums two frames of audio samples.
  \param f1 The first frame (which will contain the result)
  \param f2 The second frame
  \return 0 for success, non-zero for an error

  The frames must be AST_FRAME_VOICE and must contain AST_FORMAT_SLINEAR samples,
  and must contain the same number of samples.
 */
int ast_frame_slinear_sum(struct ast_frame *f1, struct ast_frame *f2);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _ASTERISK_FRAME_H */
