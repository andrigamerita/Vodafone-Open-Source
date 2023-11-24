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

/*! 
 * \file 
 * \brief Supports RTP and RTCP with Symmetric RTP support for NAT traversal.
 * 
 * RTP is deffined in RFC 3550.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <util/openrg_gpl.h>
#include <kos_chardev_id.h>
#include <rg_ioctl.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include "asterisk/rtp.h"
#include "asterisk/frame.h"
#include "asterisk/logger.h"
#include "asterisk/options.h"
#include "asterisk/channel.h"
#include "asterisk/acl.h"
#include "asterisk/channel.h"
#include "asterisk/config.h"
#include "asterisk/lock.h"
#include "asterisk/utils.h"
#include "asterisk/cli.h"
#include "asterisk/unaligned.h"
#include "asterisk/utils.h"
#include "asterisk/translate.h"
#include "asterisk/manager.h"

#define MAX_TIMESTAMP_SKEW	640

#define RTP_MTU		1200

#define DEFAULT_DTMF_TIMEOUT 3000 /* samples */
#define DEFAULT_DTMF_PAYLOAD 101

#define RTP_STATS_EVENT "RtpStatistics"

static int dtmftimeout = DEFAULT_DTMF_TIMEOUT;
static int dtmfpayload = DEFAULT_DTMF_PAYLOAD;

static unsigned char conf_file_md5[MD5_DIGEST_LEN];

static int rtpstart = 0;
static int rtpend = 0;
static int rtpdebug = 0;		/* Are we debugging? */
static struct ast_sockaddr rtpdebugaddr; /* Debug packets to/from this host */
#ifdef SO_NO_CHECK
static int nochecksums = 0;
static int mssclamping = 0;
#endif
static int rtpstatistics = 0;
static int rtpstatistics_period = 5000;

#define MAX_RTP_PT 256
/*
 * RFC3551, Section 6:
 *  "...payload type values in the range 96-127 MAY be defined dynamically..."
 */
#define MAX_RTP_STATIC_PT               95


#define FLAG_3389_WARNING		(1 << 0)
#define FLAG_NAT_ACTIVE			(3 << 1)
#define FLAG_NAT_INACTIVE		(0 << 1)
#define FLAG_NAT_INACTIVE_NOWARN	(1 << 1)

#define MAX_ID_LEN 9

struct ast_rtp {
	int fd;
	char resp;
	struct ast_frame f;
	unsigned char rawdata[8192 + AST_FRIENDLY_OFFSET];
	/*! Synchronization source, RFC 3550, page 10. */
	unsigned int ssrc;
	unsigned int lastts;
	unsigned int lastdigitts;
	unsigned int lastrxts;
	unsigned int lastividtimestamp;
	unsigned int lastovidtimestamp;
	unsigned int lasteventseqn;
	unsigned int lasteventendseqn;
	int lasttxformat;
	int lastrxformat;
	int dtmfcount;
	unsigned int dtmfduration;
	/* DTMF Transmission Variables */
	char send_digit;
	int send_payload;
	int send_duration;
	int nat;
	unsigned int flags;
	/*! Socket representation of the local endpoint. */
	struct ast_sockaddr us;
	/*! Socket representation of the remote endpoint. */
	struct ast_sockaddr them;
	struct timeval rxcore;
	struct timeval txcore;
	struct timeval dtmfmute;
	struct ast_smoother *smoother;
	int *ioid;
	/*! Sequence number, RFC 3550, page 13. */
	unsigned short seqno;
	unsigned short rxseqno;
	struct sched_context *sched;
	struct io_context *io;
	void *data;
	ast_rtp_callback callback;
	struct rtpPayloadType current_RTP_PT[MAX_RTP_PT];
	/*! a cache for the result of rtp_lookup_code(): */
	int rtp_lookup_code_cache_isAstFormat;
	int rtp_lookup_code_cache_code;
	int rtp_lookup_code_cache_result;
	int rtp_offered_from_local;
	int inband_dtmf;
	struct ast_rtcp *rtcp;
	unsigned int dtmf_timestamp;
	int stats_timer;
	char stats_id[MAX_ID_LEN];
};

/*!
 * \brief Structure defining an RTCP session.
 * 
 * The concept "RTCP session" is not defined in RFC 3550, but since 
 * this structure is analogous to ast_rtp, which tracks a RTP session, 
 * it is logical to think of this as a RTCP session.
 *
 * RTCP packet is defined on page 9 of RFC 3550.
 * 
 */
struct ast_rtcp {
	char quality[AST_MAX_USER_FIELD];
};

static struct ast_rtp_protocol *protos = NULL;

int ast_rtp_fd(struct ast_rtp *rtp)
{
	return rtp->fd;
}

int ast_rtcp_fd(struct ast_rtp *rtp)
{
	return -1;
}

void ast_rtp_set_data(struct ast_rtp *rtp, void *data)
{
	rtp->data = data;
}

void ast_rtp_set_callback(struct ast_rtp *rtp, ast_rtp_callback callback)
{
	rtp->callback = callback;
}

void ast_rtp_setnat(struct ast_rtp *rtp, int nat)
{
	rtp->nat = nat;
}

static struct ast_frame *send_dtmf(struct ast_rtp *rtp, int type)
{
	static struct ast_frame null_frame = { AST_FRAME_NULL, };
	char iabuf[INET6_ADDRSTRLEN];

	if ((type == AST_FRAME_DTMF_BEGIN) && ast_tvcmp(ast_tvfromboot(), rtp->dtmfmute) < 0) {
		if (option_debug)
			ast_log(LOG_DEBUG, "Ignore potential DTMF echo from '%s'\n", ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them));
		rtp->resp = 0;
		rtp->dtmfduration = 0;
		return &null_frame;
	}
	if (option_debug)
		ast_log(LOG_DEBUG, "Sending dtmf: %d (%c), at %s\n", rtp->resp, rtp->resp, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them));
	if (rtp->resp == 'X') {
		rtp->f.frametype = AST_FRAME_CONTROL;
		rtp->f.subclass = AST_CONTROL_FLASH;
	} else {
		rtp->f.frametype = type;
		rtp->f.subclass = rtp->resp;
	}
	rtp->f.datalen = 0;
	rtp->f.samples = 0;
	rtp->f.mallocd = 0;
	rtp->f.src = "RTP";
	return &rtp->f;
	
}

static inline int rtp_debug_test_addr(struct ast_sockaddr *addr)
{
	if (rtpdebug == 0)
		return 0;

	return !ast_sockaddr_cmp(&rtpdebugaddr, addr);
}

static struct ast_frame *process_cisco_dtmf(struct ast_rtp *rtp, unsigned char *data, int len)
{
	unsigned int event;
	char resp = 0;
	struct ast_frame *f = NULL;
	unsigned char seq;
	unsigned int flags;
	unsigned int power;

	/* We should have at least 4 bytes in RTP data */
	if (len < 4)
		return f;

	seq = data[0];
	flags = data[1];
	power = data[2];
	event = data[3] & 0x1f;

#if 0
	printf("Cisco Digit: %08x (len = %d)\n", event, len);
#endif	
	if (event < 10) {
		resp = '0' + event;
	} else if (event < 11) {
		resp = '*';
	} else if (event < 12) {
		resp = '#';
	} else if (event < 16) {
		resp = 'A' + (event - 12);
	} else if (event < 17) {
		resp = 'X';
	}
	if ((!rtp->resp && power) || (rtp->resp && (rtp->resp != resp))) {
		rtp->resp = resp;
		f = send_dtmf(rtp, AST_FRAME_DTMF_BEGIN);
		rtp->dtmfduration = 0;
	} else if ((rtp->resp == resp) && !power) {
		f = send_dtmf(rtp, AST_FRAME_DTMF_END);
		f->samples = rtp->dtmfduration * 8;
		rtp->resp = 0;
	} else if (rtp->resp == resp)
		rtp->dtmfduration += 20 * 8;
	rtp->dtmfcount = dtmftimeout;
	return f;
}

/*! 
 * \brief Process RTP DTMF and events according to RFC 2833.
 * 
 * RFC 2833 is "RTP Payload for DTMF Digits, Telephony Tones and Telephony Signals".
 * 
 * \param rtp
 * \param data
 * \param len
 * \param seqno
 * \returns
 */
static struct ast_frame *process_rfc2833(struct ast_rtp *rtp, unsigned char *data, int len, unsigned int seqno, unsigned int timestamp)
{
	unsigned int event;
	unsigned int event_end;
	unsigned int duration;
	char resp = 0;
	struct ast_frame *f = NULL;
	event = ntohl(*((unsigned int *)(data)));
	event >>= 24;
	event_end = ntohl(*((unsigned int *)(data)));
	event_end <<= 8;
	event_end >>= 24;
	duration = ntohl(*((unsigned int *)(data)));
	duration &= 0xFFFF;
	if (rtpdebug)
		ast_log(LOG_DEBUG, "- RTP 2833 Event: %08x (len = %d)\n", event, len);
	if (event < 10) {
		resp = '0' + event;
	} else if (event < 11) {
		resp = '*';
	} else if (event < 12) {
		resp = '#';
	} else if (event < 16) {
		resp = 'A' + (event - 12);
	} else if (event < 17) {	/* Event 16: Hook flash */
		resp = 'X';	
	}
	
	if ((!(rtp->resp) && (!(event_end & 0x80))) || (rtp->resp && rtp->resp != resp)) {
		rtp->resp = resp;
		f = send_dtmf(rtp, AST_FRAME_DTMF_BEGIN);
	} else if (event_end & 0x80 && rtp->lasteventendseqn != seqno && rtp->resp) {
		f = send_dtmf(rtp, AST_FRAME_DTMF_END);
		f->samples = duration;
		rtp->resp = 0;
		rtp->lasteventendseqn = seqno;
/* The following code is part of dtmf compensation implementation */
#if 0
	} else if (event_end & 0x80 && rtp->lasteventendseqn != seqno) {
		rtp->resp = resp;
		f = send_dtmf(rtp, AST_FRAME_DTMF_END);
		f->samples = duration;
		rtp->resp = 0;
		rtp->lasteventendseqn = seqno;
#endif
	}

	rtp->dtmfcount = dtmftimeout;
	rtp->dtmfduration = duration;
	return f;
}

/*!
 * \brief Process Comfort Noise RTP.
 * 
 * This is incomplete at the moment.
 * 
*/
static struct ast_frame *process_rfc3389(struct ast_rtp *rtp, unsigned char *data, int len)
{
	struct ast_frame *f = NULL;
	/* Convert comfort noise into audio with various codecs.  Unfortunately this doesn't
	   totally help us out becuase we don't have an engine to keep it going and we are not
	   guaranteed to have it every 20ms or anything */
	if (rtpdebug)
		ast_log(LOG_DEBUG, "- RTP 3389 Comfort noise event: Level %d (len = %d)\n", rtp->lastrxformat, len);

	if (!(ast_test_flag(rtp, FLAG_3389_WARNING))) {
		char iabuf[INET6_ADDRSTRLEN];

		ast_log(LOG_NOTICE, "Comfort noise support incomplete in Asterisk (RFC 3389). Please turn off on client if possible. Client IP: %s\n",
			ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them));
		ast_set_flag(rtp, FLAG_3389_WARNING);
	}

	/* Must have at least one byte */
	if (!len)
		return NULL;
	if (len < 24) {
		rtp->f.data = rtp->rawdata + AST_FRIENDLY_OFFSET;
		rtp->f.datalen = len - 1;
		rtp->f.offset = AST_FRIENDLY_OFFSET;
		memcpy(rtp->f.data, data + 1, len - 1);
	} else {
		rtp->f.data = NULL;
		rtp->f.offset = 0;
		rtp->f.datalen = 0;
	}
	rtp->f.frametype = AST_FRAME_CNG;
	rtp->f.subclass = data[0] & 0x7f;
	rtp->f.samples = 0;
	rtp->f.delivery.tv_usec = rtp->f.delivery.tv_sec = 0;
	f = &rtp->f;
	return f;
}

static int rtpread(int *id, int fd, short events, void *cbdata)
{
	struct ast_rtp *rtp = cbdata;
	struct ast_frame *f;
	f = ast_rtp_read(rtp);
	if (f) {
		if (rtp->callback)
			rtp->callback(rtp, f, rtp->data);
	}
	return 1;
}

struct ast_frame *ast_rtcp_read(struct ast_rtp *rtp)
{
	static struct ast_frame null_frame = { AST_FRAME_NULL, };

	return &null_frame;
}

static void calc_rxstamp(struct timeval *tv, struct ast_rtp *rtp, unsigned int timestamp, int mark)
{
	struct timeval ts = ast_samp2tv( timestamp, 8000);
	if (ast_tvzero(rtp->rxcore) || mark) {
		rtp->rxcore = ast_tvsub(ast_tvfromboot(), ts);
		/* Round to 20ms for nice, pretty timestamps */
		rtp->rxcore.tv_usec -= rtp->rxcore.tv_usec % 20000;
	}
	*tv = ast_tvadd(rtp->rxcore, ts);
}

struct ast_frame *ast_rtp_read(struct ast_rtp *rtp)
{
	int res;
	struct ast_sockaddr addr;
	unsigned int seqno;
	int version;
	int payloadtype;
	int hdrlen = 12;
	int padding;
	int mark;
	int ext;
	int x;
	char iabuf[INET6_ADDRSTRLEN];
	unsigned int timestamp;
	unsigned int *rtpheader;
	static struct ast_frame *f, null_frame = { AST_FRAME_NULL, };
	struct rtpPayloadType rtpPT;
	
	/* Cache where the header will go */
	res = read(rtp->fd, rtp->rawdata + AST_FRIENDLY_OFFSET, sizeof(rtp->rawdata) - AST_FRIENDLY_OFFSET);

	rtpheader = (unsigned int *)(rtp->rawdata + AST_FRIENDLY_OFFSET);
	if (res < 0) {
		if (errno == EBADF)
			CRASH;
		if (errno != EAGAIN) {
			ast_log(LOG_WARNING, "RTP Read error: %s. Inserting NULL frame!\n", strerror(errno));
		}
		return &null_frame;
	}
	if (res < hdrlen) {
		ast_log(LOG_WARNING, "RTP Read too short\n");
		return &null_frame;
	}

	/* Ignore if the other side hasn't been given an address
	   yet.  */
	if (ast_sockaddr_isnull(&rtp->them))
		return &null_frame;

	if (rtp->nat) {
		/* Send to whoever sent to us */
		//XXX addr is not initialized
		if (ast_sockaddr_cmp(&rtp->them, &addr)) {
			ast_sockaddr_copy(&rtp->them, &addr);
			rtp->rxseqno = 0;
			ast_set_flag(rtp, FLAG_NAT_ACTIVE);
			if (option_debug || rtpdebug)
				ast_log(LOG_DEBUG, "RTP NAT: Got audio from other end. Now sending to address %s:%d\n", ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them), ast_sockaddr_port(&rtp->them));
		}
	}

	/* Get fields */
	seqno = ntohl(rtpheader[0]);

	/* Check RTP version */
	version = (seqno & 0xC0000000) >> 30;
	if (version != 2)
		return &null_frame;
	
	payloadtype = (seqno & 0x7f0000) >> 16;
	padding = seqno & (1 << 29);
	mark = seqno & (1 << 23);
	ext = seqno & (1 << 28);
	seqno &= 0xffff;
	timestamp = ntohl(rtpheader[1]);
	
	if (padding) {
		/* Remove padding bytes */
		res -= rtp->rawdata[AST_FRIENDLY_OFFSET + res - 1];
	}
	
	if (ext) {
		/* RTP Extension present */
		hdrlen += 4;
		hdrlen += (ntohl(rtpheader[3]) & 0xffff) << 2;
	}

	if (res < hdrlen) {
		ast_log(LOG_WARNING, "RTP Read too short (%d, expecting %d)\n", res, hdrlen);
		return &null_frame;
	}

	if(rtp_debug_test_addr(&addr))
		ast_verbose("Got RTP packet from %s:%d (type %d, seq %d, ts %d, len %d)\n"
			, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &addr), ast_sockaddr_port(&addr), payloadtype, seqno, timestamp,res - hdrlen);

   rtpPT = ast_rtp_lookup_pt(rtp, payloadtype);
	if (!rtpPT.isAstFormat) {
		/* This is special in-band data that's not one of our codecs */
		if (rtpPT.code == AST_RTP_DTMF) {
			/* It's special -- rfc2833 process it */
			if(rtp_debug_test_addr(&addr)) {
				unsigned char *data;
				unsigned int event;
				unsigned int event_end;
				unsigned int duration;
				data = rtp->rawdata + AST_FRIENDLY_OFFSET + hdrlen;
				event = ntohl(*((unsigned int *)(data)));
				event >>= 24;
				event_end = ntohl(*((unsigned int *)(data)));
				event_end <<= 8;
				event_end >>= 24;
				duration = ntohl(*((unsigned int *)(data)));
				duration &= 0xFFFF;
				ast_verbose("Got rfc2833 RTP packet from %s:%d (type %d, seq %d, ts %d, len %d, mark %d, event %08x, end %d, duration %d) \n", ast_sockaddr_to_str(iabuf, sizeof(iabuf), &addr), ast_sockaddr_port(&addr), payloadtype, seqno, timestamp, res - hdrlen, (mark?1:0), event, ((event_end & 0x80)?1:0), duration);
			}
			if (rtp->lasteventseqn <= seqno || rtp->resp == 0 || (rtp->lasteventseqn >= 65530 && seqno <= 6)) {
				f = process_rfc2833(rtp, rtp->rawdata + AST_FRIENDLY_OFFSET + hdrlen, res - hdrlen, seqno, timestamp);
				rtp->lasteventseqn = seqno;
				rtp->lastrxts = timestamp;
			} else
				f = NULL;
			if (f)
				return f;
			else
				return &null_frame;
		} else if (rtpPT.code == AST_RTP_CISCO_DTMF) {
			/* It's really special -- process it the Cisco way */
			if (rtp->lasteventseqn <= seqno || rtp->resp == 0 || (rtp->lasteventseqn >= 65530 && seqno <= 6)) {
				f = process_cisco_dtmf(rtp, rtp->rawdata + AST_FRIENDLY_OFFSET + hdrlen, res - hdrlen);
				rtp->lasteventseqn = seqno;
			} else 
				f = NULL;
				if (f) 
				return f; 
			else 
				return &null_frame;
		} else if (rtpPT.code == AST_RTP_CN) {
			/* Comfort Noise */
			f = process_rfc3389(rtp, rtp->rawdata + AST_FRIENDLY_OFFSET + hdrlen, res - hdrlen);
			if (f) 
				return f; 
			else 
				return &null_frame;
		} else {
			ast_log(LOG_NOTICE, "Unknown RTP codec %d received\n", payloadtype);
			return &null_frame;
		}
	}
	rtp->f.subclass = rtpPT.code;
	if (rtp->f.subclass < AST_FORMAT_MAX_AUDIO)
		rtp->f.frametype = AST_FRAME_VOICE;
	else
		rtp->f.frametype = AST_FRAME_VIDEO;
	rtp->lastrxformat = rtp->f.subclass;

	if (!rtp->lastrxts)
		rtp->lastrxts = timestamp;

	if (rtp->rxseqno) {
		for (x=rtp->rxseqno + 1; x < seqno; x++) {
			/* Queue empty frames */
			rtp->f.mallocd = 0;
			rtp->f.datalen = 0;
			rtp->f.data = NULL;
			rtp->f.offset = 0;
			rtp->f.samples = 0;
			rtp->f.src = "RTPMissedFrame";
		}
	}
	rtp->rxseqno = seqno;

	/* Record received timestamp as last received now */
	rtp->lastrxts = timestamp;

	rtp->f.mallocd = 0;
	rtp->f.datalen = res - hdrlen;
	rtp->f.data = rtp->rawdata + hdrlen + AST_FRIENDLY_OFFSET;
	rtp->f.offset = hdrlen + AST_FRIENDLY_OFFSET;
	if (rtp->f.subclass < AST_FORMAT_MAX_AUDIO) {
		rtp->f.samples = ast_codec_get_samples(&rtp->f);
		if (rtp->f.subclass == AST_FORMAT_SLINEAR || rtp->f.subclass == AST_FORMAT_SLINEAR16)
			ast_frame_byteswap_be(&rtp->f);
		calc_rxstamp(&rtp->f.delivery, rtp, timestamp, mark);
	} else {
		/* Video -- samples is # of samples vs. 90000 */
		if (!rtp->lastividtimestamp)
			rtp->lastividtimestamp = timestamp;
		rtp->f.samples = timestamp - rtp->lastividtimestamp;
		rtp->lastividtimestamp = timestamp;
		rtp->f.delivery.tv_sec = 0;
		rtp->f.delivery.tv_usec = 0;
		if (mark)
			rtp->f.subclass |= 0x1;
		
	}
	rtp->f.src = "RTP";
	return &rtp->f;
}

/* The following array defines the MIME Media type (and subtype) for each
   of our codecs, or RTP-specific data type. */
static struct {
  struct rtpPayloadType payloadType;
  char* type;
  char* subtype;
  unsigned int sample_rate;
} mimeTypes[] = {
  {{1, AST_FORMAT_G723_1}, "audio", "G723", 8000},
  {{1, AST_FORMAT_GSM}, "audio", "GSM", 8000},
  {{1, AST_FORMAT_ULAW}, "audio", "PCMU", 8000},
  {{1, AST_FORMAT_ALAW}, "audio", "PCMA", 8000},
  {{1, AST_FORMAT_G726}, "audio", "G726-32", 8000},
  {{1, AST_FORMAT_ADPCM}, "audio", "DVI4", 8000},
  {{1, AST_FORMAT_SLINEAR}, "audio", "L8", 8000},
  {{1, AST_FORMAT_LPC10}, "audio", "LPC", 8000},
  {{1, AST_FORMAT_G729A}, "audio", "G729", 8000},
  {{1, AST_FORMAT_SPEEX}, "audio", "speex", 8000},
  {{1, AST_FORMAT_ILBC}, "audio", "iLBC", 8000},
  {{1, AST_FORMAT_G722}, "audio", "G722", 8000},
  {{1, AST_FORMAT_SLINEAR}, "audio", "L16", 8000},
  {{1, AST_FORMAT_SLINEAR16}, "audio", "L16", 16000},
  {{0, AST_RTP_DTMF}, "audio", "telephone-event", 8000},
  {{0, AST_RTP_CISCO_DTMF}, "audio", "cisco-telephone-event", 8000},
  {{1, AST_FORMAT_CN}, "audio", "CN", 8000},
  {{1, AST_FORMAT_JPEG}, "video", "JPEG", 90000},
  {{1, AST_FORMAT_PNG}, "video", "PNG", 90000},
  {{1, AST_FORMAT_H261}, "video", "H261", 90000},
  {{1, AST_FORMAT_H263}, "video", "H263", 90000},
  {{1, AST_FORMAT_H263_PLUS}, "video", "h263-1998", 90000},
};

/* Static (i.e., well-known) RTP payload types for our "AST_FORMAT..."s:
   also, our own choices for dynamic payload types.  This is our master
   table for transmission */
static struct rtpPayloadType static_RTP_PT[MAX_RTP_PT] = {
  [0] = {1, AST_FORMAT_ULAW},
#ifdef USE_DEPRECATED_G726
  [2] = {1, AST_FORMAT_G726}, /* Technically this is G.721, but if Cisco can do it, so can we... */
#endif
  [3] = {1, AST_FORMAT_GSM},
  [4] = {1, AST_FORMAT_G723_1},
  [5] = {1, AST_FORMAT_ADPCM}, /* 8 kHz */
  [6] = {1, AST_FORMAT_ADPCM}, /* 16 kHz */
  [7] = {1, AST_FORMAT_LPC10},
  [8] = {1, AST_FORMAT_ALAW},
  [9] = {1, AST_FORMAT_G722},
  [10] = {1, AST_FORMAT_SLINEAR}, /* 2 channels */
  [11] = {1, AST_FORMAT_SLINEAR}, /* 1 channel */
  [13] = {1, AST_FORMAT_CN},
  [16] = {1, AST_FORMAT_ADPCM}, /* 11.025 kHz */
  [17] = {1, AST_FORMAT_ADPCM}, /* 22.050 kHz */
  [18] = {1, AST_FORMAT_G729A},
  [19] = {0, AST_RTP_CN},		/* Also used for CN */
  [26] = {1, AST_FORMAT_JPEG},
  [31] = {1, AST_FORMAT_H261},
  [34] = {1, AST_FORMAT_H263},
  [103] = {1, AST_FORMAT_H263_PLUS},
  [97] = {1, AST_FORMAT_ILBC},
  [DEFAULT_DTMF_PAYLOAD] = {0, AST_RTP_DTMF},
  [110] = {1, AST_FORMAT_SPEEX},
  [111] = {1, AST_FORMAT_G726},
  [118] = {1, AST_FORMAT_SLINEAR16}, /* 16 Khz signed linear */
  [121] = {0, AST_RTP_CISCO_DTMF}, /* Must be type 121 */
};

void ast_rtp_pt_clear(struct ast_rtp* rtp) 
{
	int i;
	if (!rtp)
		return;

	for (i = 0; i < MAX_RTP_PT; ++i) {
		rtp->current_RTP_PT[i].isAstFormat = 0;
		rtp->current_RTP_PT[i].code = 0;
	}

	rtp->rtp_lookup_code_cache_isAstFormat = 0;
	rtp->rtp_lookup_code_cache_code = 0;
	rtp->rtp_lookup_code_cache_result = 0;
}

void ast_rtp_pt_default(struct ast_rtp* rtp) 
{
	int i;

	/* Initialize to default payload types */
	for (i = 0; i < MAX_RTP_PT; ++i) {
		rtp->current_RTP_PT[i].isAstFormat = static_RTP_PT[i].isAstFormat;
		rtp->current_RTP_PT[i].code = static_RTP_PT[i].code;
	}

	rtp->current_RTP_PT[DEFAULT_DTMF_PAYLOAD].code = 0;
	rtp->current_RTP_PT[dtmfpayload].code = AST_RTP_DTMF;
	rtp->current_RTP_PT[dtmfpayload].isAstFormat = 0;

	rtp->rtp_lookup_code_cache_isAstFormat = 0;
	rtp->rtp_lookup_code_cache_code = 0;
	rtp->rtp_lookup_code_cache_result = 0;
}

/* Make a note of a RTP paymoad type that was seen in a SDP "m=" line. */
/* By default, use the well-known value for this type (although it may */
/* still be set to a different value by a subsequent "a=rtpmap:" line): */
void ast_rtp_set_m_type(struct ast_rtp* rtp, int pt) {
	/*
	 * Get only static payload types here
	 */
	if (pt < 0 || pt > MAX_RTP_STATIC_PT) 
		return; /* non-static payload type */

	if (static_RTP_PT[pt].code != 0) {
		rtp->current_RTP_PT[pt] = static_RTP_PT[pt];
	}
} 

/* Make a note of a RTP payload type (with MIME type) that was seen in */
/* a SDP "a=rtpmap:" line. */
void ast_rtp_set_rtpmap_type(struct ast_rtp* rtp, int pt,
			 char* mimeType, char* mimeSubtype, unsigned int sample_rate) {
	int i;

	if (pt < 0 || pt > MAX_RTP_PT) 
			return; /* bogus payload type */

	for (i = 0; i < sizeof mimeTypes/sizeof mimeTypes[0]; ++i) {
		if (strcasecmp(mimeSubtype, mimeTypes[i].subtype))
			continue;
		if (strcasecmp(mimeType, mimeTypes[i].type))
			continue;
		if (sample_rate && mimeTypes[i].sample_rate &&
		    (sample_rate != mimeTypes[i].sample_rate)) {
			continue;
		}
		rtp->current_RTP_PT[pt] = mimeTypes[i].payloadType;
		return;
	}
} 

/* Return the union of all of the codecs that were set by rtp_set...() calls */
/* They're returned as two distinct sets: AST_FORMATs, and AST_RTPs */
void ast_rtp_get_current_formats(struct ast_rtp* rtp,
			     int* astFormats, int* nonAstFormats) {
	int pt;

	*astFormats = *nonAstFormats = 0;
	for (pt = 0; pt < MAX_RTP_PT; ++pt) {
		if (rtp->current_RTP_PT[pt].isAstFormat) {
			*astFormats |= rtp->current_RTP_PT[pt].code;
		} else {
			*nonAstFormats |= rtp->current_RTP_PT[pt].code;
		}
	}
}

void ast_rtp_offered_from_local(struct ast_rtp* rtp, int local) {
	if (rtp)
		rtp->rtp_offered_from_local = local;
	else
		ast_log(LOG_WARNING, "rtp structure is null\n");
}

struct rtpPayloadType ast_rtp_lookup_pt(struct ast_rtp* rtp, int pt) 
{
	struct rtpPayloadType result;

	result.isAstFormat = result.code = 0;
	if (pt < 0 || pt > MAX_RTP_PT) 
		return result; /* bogus payload type */

	/* Start with the negotiated codecs */
	if (!rtp->rtp_offered_from_local)
		result = rtp->current_RTP_PT[pt];

	/* If it doesn't exist, check our static RTP type list, just in case
	   (and only for static payload types). */
	if (!result.code && pt <= (rtp->rtp_offered_from_local ? MAX_RTP_PT : MAX_RTP_STATIC_PT)) 
		result = static_RTP_PT[pt];
	return result;
}

/* Looks up an RTP code out of our *static* outbound list */
int ast_rtp_lookup_code(struct ast_rtp* rtp, const int isAstFormat, const int code) {

	int pt;

	if (!rtp) {
	    for (pt = 0; pt < MAX_RTP_PT; ++pt) {
		if (static_RTP_PT[pt].code == code && static_RTP_PT[pt].isAstFormat == isAstFormat)
			return pt;
	    }

	    return -1;
	}

	if (isAstFormat == rtp->rtp_lookup_code_cache_isAstFormat &&
		code == rtp->rtp_lookup_code_cache_code) {

		/* Use our cached mapping, to avoid the overhead of the loop below */
		return rtp->rtp_lookup_code_cache_result;
	}

	/* Check the dynamic list first */
	for (pt = 0; pt < MAX_RTP_PT; ++pt) {
  		if (rtp->current_RTP_PT[pt].code == code && rtp->current_RTP_PT[pt].isAstFormat == isAstFormat) {
			rtp->rtp_lookup_code_cache_isAstFormat = isAstFormat;
			rtp->rtp_lookup_code_cache_code = code;
			rtp->rtp_lookup_code_cache_result = pt;
			return pt;
		}
	}

	/* Then the static list */
	for (pt = 0; pt < MAX_RTP_PT; ++pt) {
		if (static_RTP_PT[pt].code == code && static_RTP_PT[pt].isAstFormat == isAstFormat) {
			rtp->rtp_lookup_code_cache_isAstFormat = isAstFormat;
  			rtp->rtp_lookup_code_cache_code = code;
			rtp->rtp_lookup_code_cache_result = pt;
			return pt;
		}
	}
	return -1;
}

unsigned int ast_rtp_lookup_sample_rate(int isAstFormat, const int code)
{
	unsigned int i;

	for (i = 0; i < sizeof mimeTypes/sizeof mimeTypes[0]; ++i) {
		if ((mimeTypes[i].payloadType.code == code) && (mimeTypes[i].payloadType.isAstFormat == isAstFormat)) {
			return mimeTypes[i].sample_rate;
		}
	}

	return 0;
}

char* ast_rtp_lookup_mime_subtype(const int isAstFormat, const int code) {

	int i;

	for (i = 0; i < sizeof mimeTypes/sizeof mimeTypes[0]; ++i) {
	if (mimeTypes[i].payloadType.code == code && mimeTypes[i].payloadType.isAstFormat == isAstFormat) {
      		return mimeTypes[i].subtype;
		}
	}
	return "";
}

char *ast_rtp_lookup_mime_multiple(char *buf, int size, const int capability, const int isAstFormat)
{
	int format;
	unsigned len;
	char *end = buf;
	char *start = buf;

	if (!buf || !size)
		return NULL;

	snprintf(end, size, "0x%x (", capability);

	len = strlen(end);
	end += len;
	size -= len;
	start = end;

	for (format = 1; format < AST_RTP_MAX; format <<= 1) {
		if (capability & format) {
			const char *name = ast_rtp_lookup_mime_subtype(isAstFormat, format);
			snprintf(end, size, "%s|", name);
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

static struct ast_rtcp *ast_rtcp_new(void)
{
	struct ast_rtcp *rtcp;
	rtcp = malloc(sizeof(struct ast_rtcp));
	if (!rtcp)
		return NULL;
	memset(rtcp, 0, sizeof(struct ast_rtcp));
	return rtcp;
}

struct ast_rtp *ast_rtp_new_with_bindaddr(struct sched_context *sched, struct io_context *io, int rtcpenable, int callbackmode, struct ast_sockaddr *addr)
{
	struct ast_rtp *rtp;
	int x;
	int startplace;
	rtp_ioctl_new_t params;
	rtp = malloc(sizeof(struct ast_rtp));
	if (!rtp)
		return NULL;
	memset(rtp, 0, sizeof(struct ast_rtp));
	rtp->fd = gpl_sys_rg_chrdev_open(KOS_CDT_RTP, O_RDWR);
	rtp->ssrc = rand();
	rtp->seqno = rand() & 0xffff;
	if (rtp->fd < 0)
    	{
		free(rtp);
		ast_log(LOG_ERROR, "Unable to open jrtp char device\n");
		return NULL;
	}
	if (rtcpenable) {
		rtp->rtcp = ast_rtcp_new();
	}

	/* Prepare params for RTP_NEW ioctl */
	params.addr = addr->ss;
	/* TOS will be set in rtp_settos() */
    	params.tos = 0;
	/* so_mark will be set in rtp_set_so_mark() */
	params.so_mark = 0;
	params.mss_clamping_limit = mssclamping;
	params.rtcp_enable = rtcpenable;

	/* Select a random port number in the range of possible RTP */
	x = (rand() % (rtpend-rtpstart)) + rtpstart;
	x = x & ~1;
	/* Save it for future references. */
	startplace = x;
	/* Iterate tring to bind that port and incrementing it otherwise untill a port was found or no ports are available. */
	for (;;) {
		/* Must be an even port number by RTP spec */
		ast_sockaddr_copy(&rtp->us, addr);
		ast_sockaddr_set_port(&rtp->us, x);
		((struct sockaddr_in *)&params.addr)->sin_port = htons(x);
		if (ioctl(rtp->fd, RTP_NEW, &params) != -1)
			break;
		if (errno != EADDRINUSE) {
			/* We got an error that wasn't expected, abort! */
			ast_log(LOG_ERROR, "Unexpected bind error: %s\n", strerror(errno));
			close(rtp->fd);
			free(rtp);
			return NULL;
		}
		/* The port was used, increment it (by two). */
		x += 2;
		/* Did we go over the limit ? */
		if (x > rtpend)
			/* then, start from the begingig. */
			x = (rtpstart + 1) & ~1;
		/* Check if we reached the place were we started. */
		if (x == startplace) {
			/* If so, there's no ports available. */
			ast_log(LOG_ERROR, "No RTP ports remaining. Can't setup media stream for this call.\n");
			close(rtp->fd);
			free(rtp);
			return NULL;
		}
	}
	
	rtp->sched = sched;
	rtp->stats_timer = -1;
	rtp->io = io;
	if (callbackmode) {
		/* Operate this one in a callback mode */
		rtp->ioid = ast_io_add(rtp->io, rtp->fd, rtpread, AST_IO_IN, rtp);
	}
	ast_rtp_pt_default(rtp);
	return rtp;
}

struct ast_rtp *ast_rtp_new(struct sched_context *sched, struct io_context *io, int rtcpenable, int callbackmode)
{
	struct sockaddr_in sin;
	struct ast_sockaddr temp_addr;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	ast_sockaddr_from_sin(&temp_addr, &sin);
	return ast_rtp_new_with_bindaddr(sched, io, rtcpenable, callbackmode, &temp_addr);
}

int ast_rtp_settos(struct ast_rtp *rtp, int tos)
{
	int res;

	if ((res = ioctl(rtp->fd, RTP_TOS_SET, tos)))
		ast_log(LOG_WARNING, "Unable to set TOS to %d\n", tos);
	return res;
}

int ast_rtp_set_so_mark(struct ast_rtp *rtp, int so_mark)
{
	int res;

	if ((res = ioctl(rtp->fd, RTP_SO_MARK_SET, so_mark)))
		ast_log(LOG_WARNING, "Unable to set SO_MARK to %d\n", so_mark);
	return res;
}

void ast_rtp_clear_payload_types(struct ast_rtp *rtp)
{
	ioctl(rtp->fd, RTP_PAYLOAD, NULL); /* Clear payloads */
}

int ast_rtp_set_payload_type(struct ast_rtp *rtp, int media_payload_type,
	int rtp_code, int is_ast_format)
{
	int rtp_payload_type = ast_rtp_lookup_code(rtp, is_ast_format, rtp_code);
	rtp_ioctl_payload_t params;
	
	/* If no media payload type provided (-1), use rtp for both media
	 * and rtp */
	params.media_payload_type = (media_payload_type >= 0) ? media_payload_type :
		rtp_payload_type;
	params.rtp_payload_type = rtp_payload_type;

	return ioctl(rtp->fd, RTP_PAYLOAD, &params);
}

unsigned int ast_rtp_get_context(struct ast_rtp *rtp)
{
	unsigned int context = 0;

	if (ioctl(rtp->fd, RTP_ID_GET, &context))
		ast_log(LOG_ERROR, "Unable to get jrtp context\n");

	return context;
}

void ast_rtp_set_peer(struct ast_rtp *rtp, struct ast_sockaddr *them, int mode)
{
	rtp_ioctl_connect_t params;

	params.addr = them->ss;

	if (ioctl(rtp->fd, RTP_CONNECT, &params))
	{
		ast_log(LOG_ERROR, "Unable to set jrtp peer\n");
		return;
	}

	if (ioctl(rtp->fd, RTP_MODE, mode))
	{
		ast_log(LOG_ERROR, "Unable to set jrtp mode\n");
		return;
	}

	ast_sockaddr_copy(&rtp->them, them);
	rtp->rxseqno = 0;
}

int ast_rtp_get_peer(struct ast_rtp *rtp, struct ast_sockaddr *them)
{
	if (ast_sockaddr_cmp(them, &rtp->them)) {
		ast_sockaddr_copy(them, &rtp->them);
		return 1;
	}
	return 0;
}

void ast_rtp_get_us(struct ast_rtp *rtp, struct ast_sockaddr *us)
{
	ast_sockaddr_copy(us, &rtp->us);
}

void ast_rtp_stop(struct ast_rtp *rtp)
{
	ast_sockaddr_setnull(&rtp->them);
}

void ast_rtp_reset(struct ast_rtp *rtp)
{
	memset(&rtp->rxcore, 0, sizeof(rtp->rxcore));
	memset(&rtp->txcore, 0, sizeof(rtp->txcore));
	memset(&rtp->dtmfmute, 0, sizeof(rtp->dtmfmute));
	rtp->lastts = 0;
	rtp->lastdigitts = 0;
	rtp->lastrxts = 0;
	rtp->lastividtimestamp = 0;
	rtp->lastovidtimestamp = 0;
	rtp->lasteventseqn = 0;
	rtp->lasteventendseqn = 0;
	rtp->lasttxformat = 0;
	rtp->lastrxformat = 0;
	rtp->dtmfcount = 0;
	rtp->dtmfduration = 0;
	rtp->seqno = 0;
	rtp->rxseqno = 0;
}

struct rtp_stats_t;

char *ast_rtp_get_quality(struct ast_rtp *rtp)
{
	rtp_stats_t stats;
    
	if (ioctl(rtp->fd, RTP_STATS_GET, &stats))
	{
		ast_log(LOG_ERROR, "Unable to get RTP stats from jrtp\n");
		return NULL;
	}
    
	/*
	*ssrc          our ssrc
	*themssrc      their ssrc
	*lp            lost packets
	*rxjitter      our calculated jitter(rx)
	*arxjitter     our average calculated jitter(rx)
	*rxpackets     no. received packets
	*rxoctets      no. received octets
	*txjitter      reported jitter of the other end
	*atxjitter     average reported jitter of the other end
	*txpackets     transmitted packets
	*txoctets      transmitted octets
	*rlp           remote lost packets
	*/
	
	snprintf(rtp->rtcp->quality, sizeof(rtp->rtcp->quality), "ssrc=%u;"
	    "themssrc=%u;lp=%u;lpr=%u;rxjitter=%u;arxjitter=%u;rxcount=%u;"
	    "rxoctets=%u;txjitter=%u;atxjitter=%u;txcount=%u;txoctets=%u;"
	    "rlp=%u;rlpr=%u;rtt=%u;artt=%u;rxfs=%u;txfs=%u;ntx=%u;nrx=%u", 0, 0, stats.rx_packets_lost,
	    stats.rx_loss_rate, stats.rx_jitter, stats.avg_rx_jitter, 
	    stats.rx_packets, stats.rx_octets, stats.reported_jitter, 
	    stats.avg_reported_jitter, stats.tx_packets, stats.tx_octets,
	    stats.reported_packets_lost, stats.reported_loss_rate, 
	    stats.round_trip_delay, stats.avg_round_trip_delay,
	    stats.rx_fraction_sum, stats.tx_fraction_sum,
	    stats.num_of_rtcp_sent, stats.num_of_rtcp_received);
	return rtp->rtcp->quality;
}

void ast_rtp_destroy(struct ast_rtp *rtp)
{
	if (rtp->smoother)
		ast_smoother_free(rtp->smoother);
	if (rtp->ioid)
		ast_io_remove(rtp->io, rtp->ioid);
	if (rtp->fd > -1)
		close(rtp->fd);
	if (rtp->rtcp) {
		free(rtp->rtcp);
	}
	if (rtp->stats_timer > -1)
		ast_sched_del(rtp->sched, rtp->stats_timer);
	free(rtp);
}

static unsigned int calc_txstamp(struct ast_rtp *rtp, struct timeval *delivery)
{
	struct timeval t;
	long ms;
	if (ast_tvzero(rtp->txcore) || !rtp->lastts) {
		rtp->txcore = ast_samp2tv(rtp->lastts, 8000);
	}
	/* Use previous txcore if available */
	t = (delivery && !ast_tvzero(*delivery)) ? *delivery : ast_tvfromboot();
	ms = ast_tvdiff_ms(t, rtp->txcore);
	if (ms < 0)
		ms = 0;
	/* Use what we just got for next time */
	rtp->txcore = t;
	return (unsigned int) ms;
}

#if 0
/*! \brief Send continuation frame for DTMF */
static int ast_rtp_senddigit_continuation(void *dtmf_data)
{
    	struct ast_rtp *rtp = (struct ast_rtp *)dtmf_data;
	unsigned int *rtpheader;
	int hdrlen = 12, res = 0;
	char data[256];
	char iabuf[INET_ADDRSTRLEN];
	unsigned int timestamp;

	if (!rtp->them.sin_addr.s_addr || !rtp->them.sin_port)
		return 0;

	/* Setup packet to send */
	rtpheader = (unsigned int *)data;
        rtpheader[0] = htonl((2 << 30) | (1 << 23) | (rtp->send_payload << 16) | (rtp->seqno));
	rtpheader[1] = htonl(timestamp);
        rtpheader[2] = htonl(rtp->ssrc);
        rtpheader[3] = htonl((rtp->send_digit << 24) | (0xa << 16) | (rtp->send_duration));
	rtpheader[0] = htonl((2 << 30) | (rtp->send_payload << 16) | (rtp->seqno));
	
	/* Transmit */
	res = write(rtp->fd, (void *) rtpheader, hdrlen + 4);
	if (res < 0)
		ast_log(LOG_ERROR, "RTP Transmission error to %s:%d: %s\n",
			ast_inet_ntoa(iabuf, sizeof(iabuf), rtp->them.sin_addr),
			ntohs(rtp->them.sin_port), strerror(errno));
	if (rtp_debug_test_addr(&rtp->them))
		ast_verbose("Sent RTP DTMF packet to %s:%d (type %-2.2d, seq %-6.6u, ts %-6.6u, len %-6.6u)\n",
			    ast_inet_ntoa(iabuf, sizeof(iabuf), rtp->them.sin_addr),
			    ntohs(rtp->them.sin_port), rtp->send_payload, rtp->seqno, rtp->lastdigitts, res - hdrlen);

	/* Increment sequence number */
	rtp->seqno++;
	/* Increment duration */
	rtp->send_duration += 160;

	return 0;
}
#endif

int ast_rtp_senddigit_begin(struct ast_rtp *rtp, char digit)
{
	unsigned int *rtpheader;
	int hdrlen = 12;
	int res;
	int x;
	int payload;
	char data[256];
	char iabuf[INET6_ADDRSTRLEN];

	if ((digit <= '9') && (digit >= '0'))
		digit -= '0';
	else if (digit == '*')
		digit = 10;
	else if (digit == '#')
		digit = 11;
	else if ((digit >= 'A') && (digit <= 'D')) 
		digit = digit - 'A' + 12;
	else if ((digit >= 'a') && (digit <= 'd')) 
		digit = digit - 'a' + 12;
	else if (digit == 'f')
		digit = 16;
	else {
		ast_log(LOG_WARNING, "Don't know how to represent '%c'\n", digit);
		return -1;
	}
	payload = ast_rtp_lookup_code(rtp, 0, AST_RTP_DTMF);

	/* If we have no peer, return immediately */	
	if (ast_sockaddr_isnull(&rtp->them))
		return 0;

	rtp->dtmfmute = ast_tvadd(ast_tvfromboot(), ast_tv(0, 500000));
	rtp->send_duration = 160;

	if (ioctl(rtp->fd, RTP_TIMESTAMP_GET, &rtp->dtmf_timestamp))
	{
		ast_log(LOG_ERROR, "Unable to get timestamp from jrtp\n");
		return -1;
	}
	ast_log(LOG_DEBUG, "Got timestamp %d from jrtp for digit %d\n", rtp->dtmf_timestamp, digit);
	
	/* Get a pointer to the header */
	rtpheader = (unsigned int *)data;
	rtpheader[0] = htonl((2 << 30) | (1 << 23) | (payload << 16) | (rtp->seqno));
	rtpheader[1] = htonl(rtp->dtmf_timestamp);
	rtpheader[2] = htonl(rtp->ssrc); 
	for (x = 0; x < 2; x++) {
		rtpheader[3] = htonl((digit << 24) | (0xa << 16) | (rtp->send_duration));
		{
			res = write(rtp->fd, (void *) rtpheader, hdrlen + 4);
			if (res < 0) 
				ast_log(LOG_ERROR, "RTP Transmission error to %s:%d: %s\n",
					ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them),
					ast_sockaddr_port(&rtp->them), strerror(errno));
			if (rtp_debug_test_addr(&rtp->them))
				ast_verbose("Sent RTP packet to %s:%d (type %d, seq %u, ts %u, len %u)\n",
					    ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them),
					    ast_sockaddr_port(&rtp->them), payload, rtp->seqno, rtp->lastdigitts, res - hdrlen);
		}
		/* Increment sequence number */
		rtp->seqno++;
		/* Increment duration */
		rtp->send_duration += 160;
		/* Clear marker bit and set seqno */
		rtpheader[0] = htonl((2 << 30) | (payload << 16) | (rtp->seqno));
	}
	rtp->send_digit = digit;
	rtp->send_payload = payload;
	return 0;
}

/*! \brief Send end packets for DTMF */
int ast_rtp_senddigit_end(struct ast_rtp *rtp, char digit)
{
	unsigned int *rtpheader;
	int hdrlen = 12, res = 0, i = 0;
	char data[256];
	char iabuf[INET6_ADDRSTRLEN];

	/* If no address, then bail out */
	if (ast_sockaddr_isnull(&rtp->them))
		return 0;
	
	if ((digit <= '9') && (digit >= '0'))
		digit -= '0';
	else if (digit == '*')
		digit = 10;
	else if (digit == '#')
		digit = 11;
	else if ((digit >= 'A') && (digit <= 'D'))
		digit = digit - 'A' + 12;
	else if ((digit >= 'a') && (digit <= 'd'))
		digit = digit - 'a' + 12;
	else if (digit == 'f')
		digit = 16;
	else {
		ast_log(LOG_WARNING, "Don't know how to represent '%c'\n", digit);
		return 0;
	}

	rtp->dtmfmute = ast_tvadd(ast_tvfromboot(), ast_tv(0, 500000));

	rtpheader = (unsigned int *)data;
	rtpheader[0] = htonl((2 << 30) | (1 << 23) | (rtp->send_payload << 16) | (rtp->seqno));
	rtpheader[1] = htonl(rtp->dtmf_timestamp);
	rtpheader[2] = htonl(rtp->ssrc);
	rtpheader[3] = htonl((digit << 24) | (0xa << 16) | (rtp->send_duration));
	/* Set end bit */
	rtpheader[3] |= htonl((1 << 23));
	rtpheader[0] = htonl((2 << 30) | (rtp->send_payload << 16) | (rtp->seqno));
	/* Send 3 termination packets */
	for (i = 0; i < 3; i++) {
		res = write(rtp->fd, (void *) rtpheader, hdrlen + 4);
		if (res < 0)
			ast_log(LOG_ERROR, "RTP Transmission error to %s:%d: %s\n",
			    ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them),
				ast_sockaddr_port(&rtp->them), strerror(errno));
		if (rtp_debug_test_addr(&rtp->them))
			ast_verbose("Sent RTP DTMF packet to %s:%d (type %-2.2d, seq %-6.6u, ts %-6.6u, len %-6.6u)\n",
			    ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them),
				    ast_sockaddr_port(&rtp->them), rtp->send_payload, rtp->seqno, rtp->lastdigitts, res - hdrlen);
	}
	rtp->send_digit = 0;
	/* Increment lastdigitts */
	rtp->lastdigitts += 960;
	rtp->seqno++;

	return res;
}

int ast_rtp_sendcng(struct ast_rtp *rtp, int level)
{
	unsigned int *rtpheader;
	int hdrlen = 12;
	int res;
	int payload;
	char data[256];
	char iabuf[INET6_ADDRSTRLEN];
	level = 127 - (level & 0x7f);
	payload = ast_rtp_lookup_code(rtp, 0, AST_RTP_CN);

	/* If we have no peer, return immediately */	
	if (ast_sockaddr_isnull(&rtp->them))
		return 0;

	rtp->dtmfmute = ast_tvadd(ast_tvfromboot(), ast_tv(0, 500000));

	/* Get a pointer to the header */
	rtpheader = (unsigned int *)data;
	rtpheader[0] = htonl((2 << 30) | (1 << 23) | (payload << 16) | (rtp->seqno++));
	rtpheader[1] = htonl(rtp->lastts);
	rtpheader[2] = htonl(rtp->ssrc); 
	data[12] = level;
	{
		res = write(rtp->fd, (void *)rtpheader, hdrlen + 1);
		if (res <0) 
			ast_log(LOG_ERROR, "RTP Comfort Noise Transmission error to %s:%d: %s\n", ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them), ast_sockaddr_port(&rtp->them), strerror(errno));
		if(rtp_debug_test_addr(&rtp->them))
			ast_verbose("Sent Comfort Noise RTP packet to %s:%d (type %d, seq %d, ts %d, len %d)\n"
					, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them), ast_sockaddr_port(&rtp->them), payload, rtp->seqno, rtp->lastts,res - hdrlen);		   
		   
	}
	return 0;
}

static int ast_rtp_raw_write(struct ast_rtp *rtp, struct ast_frame *f, int codec)
{
	unsigned char *rtpheader;
	char iabuf[INET6_ADDRSTRLEN];
	int hdrlen = 12;
	int res;
	unsigned int ms;
	int pred;
	int mark = 0;

	ms = calc_txstamp(rtp, &f->delivery);
	/* Default prediction */
	if (f->subclass < AST_FORMAT_MAX_AUDIO) {
		pred = rtp->lastts + f->samples;

		/* Re-calculate last TS */
		rtp->lastts = rtp->lastts + ms * 8;
		if (ast_tvzero(f->delivery)) {
			/* If this isn't an absolute delivery time, Check if it is close to our prediction, 
			   and if so, go with our prediction */
			if (abs(rtp->lastts - pred) < MAX_TIMESTAMP_SKEW)
				rtp->lastts = pred;
			else {
				if (option_debug > 2)
					ast_log(LOG_DEBUG, "Difference is %d, ms is %d\n", abs(rtp->lastts - pred), ms);
				mark = 1;
			}
		}
	} else {
		mark = f->subclass & 0x1;
		pred = rtp->lastovidtimestamp + f->samples;
		/* Re-calculate last TS */
		rtp->lastts = rtp->lastts + ms * 90;
		/* If it's close to our prediction, go for it */
		if (ast_tvzero(f->delivery)) {
			if (abs(rtp->lastts - pred) < 7200) {
				rtp->lastts = pred;
				rtp->lastovidtimestamp += f->samples;
			} else {
				if (option_debug > 2)
					ast_log(LOG_DEBUG, "Difference is %d, ms is %d (%d), pred/ts/samples %d/%d/%d\n", abs(rtp->lastts - pred), ms, ms * 90, rtp->lastts, pred, f->samples);
				rtp->lastovidtimestamp = rtp->lastts;
			}
		}
	}
	/* If the timestamp for non-digit packets has moved beyond the timestamp
	   for digits, update the digit timestamp.
	*/
	if (rtp->lastts > rtp->lastdigitts)
		rtp->lastdigitts = rtp->lastts;

	/* Get a pointer to the header */
	rtpheader = (unsigned char *)(f->data - hdrlen);

	put_unaligned_uint32(rtpheader, htonl((2 << 30) | (codec << 16) | (rtp->seqno) | (mark << 23)));
	put_unaligned_uint32(rtpheader + 4, htonl(rtp->lastts));
	put_unaligned_uint32(rtpheader + 8, htonl(rtp->ssrc)); 

	if (!ast_sockaddr_isnull(&rtp->them)) {
		res = write(rtp->fd, (void *)rtpheader, f->datalen + hdrlen);
		if (res <0) {
			if (!rtp->nat || (rtp->nat && (ast_test_flag(rtp, FLAG_NAT_ACTIVE) == FLAG_NAT_ACTIVE))) {
				ast_log(LOG_DEBUG, "RTP Transmission error of packet %d to %s:%d: %s\n", rtp->seqno, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them), ast_sockaddr_port(&rtp->them), strerror(errno));
			} else if ((ast_test_flag(rtp, FLAG_NAT_ACTIVE) == FLAG_NAT_INACTIVE) || rtpdebug) {
				/* Only give this error message once if we are not RTP debugging */
				if (option_debug || rtpdebug)
					ast_log(LOG_DEBUG, "RTP NAT: Can't write RTP to private address %s:%d, waiting for other end to send audio...\n", ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them), ast_sockaddr_port(&rtp->them));
				ast_set_flag(rtp, FLAG_NAT_INACTIVE_NOWARN);
			}
		}
				
		if(rtp_debug_test_addr(&rtp->them))
			ast_verbose("Sent RTP packet to %s:%d (type %d, seq %u, ts %u, len %u)\n"
					, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtp->them), ast_sockaddr_port(&rtp->them), codec, rtp->seqno, rtp->lastts,res - hdrlen);
	}

	rtp->seqno++;

	return 0;
}

int ast_rtp_write(struct ast_rtp *rtp, struct ast_frame *_f)
{
	struct ast_frame *f;
	int codec;
	int hdrlen = 12;
	int subclass;
	

	/* If we have no peer, return immediately */	
	if (ast_sockaddr_isnull(&rtp->them))
		return 0;

	/* If there is no data length, return immediately */
	if (!_f->datalen) 
		return 0;
	
	/* Make sure we have enough space for RTP header */
	if ((_f->frametype != AST_FRAME_VOICE) && (_f->frametype != AST_FRAME_VIDEO)) {
		ast_log(LOG_WARNING, "RTP can only send voice\n");
		return -1;
	}

	subclass = _f->subclass;
	if (_f->frametype == AST_FRAME_VIDEO)
		subclass &= ~0x1;

	codec = ast_rtp_lookup_code(rtp, 1, subclass);
	if (codec < 0) {
		ast_log(LOG_WARNING, "Don't know how to send format %s packets with RTP\n", ast_getformatname(_f->subclass));
		return -1;
	}

	if (rtp->lasttxformat != subclass) {
		/* New format, reset the smoother */
		if (option_debug)
			ast_log(LOG_DEBUG, "Ooh, format changed from %s to %s\n", ast_getformatname(rtp->lasttxformat), ast_getformatname(subclass));
		rtp->lasttxformat = subclass;
		if (rtp->smoother)
			ast_smoother_free(rtp->smoother);
		rtp->smoother = NULL;
	}


	switch(subclass) {
	case AST_FORMAT_SLINEAR:
	case AST_FORMAT_SLINEAR16:
		if (!rtp->smoother) {
		    if (subclass == AST_FORMAT_SLINEAR)
			rtp->smoother = ast_smoother_new(320);
		    else if (subclass == AST_FORMAT_SLINEAR16)
			rtp->smoother = ast_smoother_new(640);
		}
		if (!rtp->smoother) {
			ast_log(LOG_WARNING, "Unable to create smoother :(\n");
			return -1;
		}
		ast_smoother_feed_be(rtp->smoother, _f);
		
		while((f = ast_smoother_read(rtp->smoother)))
			ast_rtp_raw_write(rtp, f, codec);
		break;
	case AST_FORMAT_ULAW:
	case AST_FORMAT_ALAW:
		if (!rtp->smoother) {
			rtp->smoother = ast_smoother_new(160);
		}
		if (!rtp->smoother) {
			ast_log(LOG_WARNING, "Unable to create smoother :(\n");
			return -1;
		}
		ast_smoother_feed(rtp->smoother, _f);
		
		while((f = ast_smoother_read(rtp->smoother)))
			ast_rtp_raw_write(rtp, f, codec);
		break;
	case AST_FORMAT_ADPCM:
	case AST_FORMAT_G726:
		if (!rtp->smoother) {
			rtp->smoother = ast_smoother_new(80);
		}
		if (!rtp->smoother) {
			ast_log(LOG_WARNING, "Unable to create smoother :(\n");
			return -1;
		}
		ast_smoother_feed(rtp->smoother, _f);
		
		while((f = ast_smoother_read(rtp->smoother)))
			ast_rtp_raw_write(rtp, f, codec);
		break;
	case AST_FORMAT_G729A:
		if (!rtp->smoother) {
			rtp->smoother = ast_smoother_new(20);
			if (rtp->smoother)
				ast_smoother_set_flags(rtp->smoother, AST_SMOOTHER_FLAG_G729);
		}
		if (!rtp->smoother) {
			ast_log(LOG_WARNING, "Unable to create g729 smoother :(\n");
			return -1;
		}
		ast_smoother_feed(rtp->smoother, _f);
		
		while((f = ast_smoother_read(rtp->smoother)))
			ast_rtp_raw_write(rtp, f, codec);
		break;
	case AST_FORMAT_GSM:
		if (!rtp->smoother) {
			rtp->smoother = ast_smoother_new(33);
		}
		if (!rtp->smoother) {
			ast_log(LOG_WARNING, "Unable to create GSM smoother :(\n");
			return -1;
		}
		ast_smoother_feed(rtp->smoother, _f);
		while((f = ast_smoother_read(rtp->smoother)))
			ast_rtp_raw_write(rtp, f, codec);
		break;
	case AST_FORMAT_ILBC:
		if (!rtp->smoother) {
			rtp->smoother = ast_smoother_new(50);
		}
		if (!rtp->smoother) {
			ast_log(LOG_WARNING, "Unable to create ILBC smoother :(\n");
			return -1;
		}
		ast_smoother_feed(rtp->smoother, _f);
		while((f = ast_smoother_read(rtp->smoother)))
			ast_rtp_raw_write(rtp, f, codec);
		break;
	default:	
		ast_log(LOG_WARNING, "Not sure about sending format %s packets\n", ast_getformatname(subclass));
		/* fall through to... */
	case AST_FORMAT_H261:
	case AST_FORMAT_H263:
	case AST_FORMAT_H263_PLUS:
	case AST_FORMAT_G722:
	case AST_FORMAT_G723_1:
	case AST_FORMAT_LPC10:
	case AST_FORMAT_SPEEX:
	        /* Don't buffer outgoing frames; send them one-per-packet: */
		if (_f->offset < hdrlen) {
			f = ast_frdup(_f);
		} else {
			f = _f;
		}
		ast_rtp_raw_write(rtp, f, codec);
	}
		
	return 0;
}

/*--- ast_rtp_proto_unregister: Unregister interface to channel driver */
void ast_rtp_proto_unregister(struct ast_rtp_protocol *proto)
{
	struct ast_rtp_protocol *cur, *prev;

	cur = protos;
	prev = NULL;
	while(cur) {
		if (cur == proto) {
			if (prev)
				prev->next = proto->next;
			else
				protos = proto->next;
			return;
		}
		prev = cur;
		cur = cur->next;
	}
}

/*--- ast_rtp_proto_register: Register interface to channel driver */
int ast_rtp_proto_register(struct ast_rtp_protocol *proto)
{
	struct ast_rtp_protocol *cur;
	cur = protos;
	while(cur) {
		if (cur->type == proto->type) {
			ast_log(LOG_WARNING, "Tried to register same protocol '%s' twice\n", cur->type);
			return -1;
		}
		cur = cur->next;
	}
	proto->next = protos;
	protos = proto;
	return 0;
}

/*--- get_proto: Get channel driver interface structure */
static struct ast_rtp_protocol *get_proto(struct ast_channel *chan)
{
	struct ast_rtp_protocol *cur;

	cur = protos;
	while(cur) {
		if (cur->type == chan->type) {
			return cur;
		}
		cur = cur->next;
	}
	return NULL;
}

/* ast_rtp_bridge: Bridge calls. If possible and allowed, initiate
	re-invite so the peers exchange media directly outside 
	of Asterisk. */
enum ast_bridge_result ast_rtp_bridge(struct ast_channel *c0, struct ast_channel *c1, int flags, struct ast_frame **fo, struct ast_channel **rc, int timeoutms)
{
	struct ast_frame *f;
	struct ast_channel *who, *cs[3];
	struct ast_rtp *p0, *p1;		/* Audio RTP Channels */
	struct ast_rtp *vp0, *vp1;		/* Video RTP channels */
	struct ast_rtp_protocol *pr0, *pr1;
	struct ast_sockaddr ac0, ac1;
	struct ast_sockaddr vac0, vac1;
	struct ast_sockaddr t0, t1;
	struct ast_sockaddr vt0, vt1;
	char iabuf[INET6_ADDRSTRLEN];
	
	void *pvt0, *pvt1;
	int codec0,codec1, oldcodec0, oldcodec1;
        struct ast_codec_pref codecs0;
        struct ast_codec_pref codecs1;
	int dummy, noncodec0, noncodec1;

	
	memset(&vt0, 0, sizeof(vt0));
	memset(&vt1, 0, sizeof(vt1));
	memset(&vac0, 0, sizeof(vac0));
	memset(&vac1, 0, sizeof(vac1));
        ast_codec_pref_init(&codecs0);
        ast_codec_pref_init(&codecs1);

	/* if need DTMF, cant native bridge */
	if (flags & (AST_BRIDGE_DTMF_CHANNEL_0 | AST_BRIDGE_DTMF_CHANNEL_1))
		return AST_BRIDGE_FAILED_NOWARN;

	/* Lock channels */
	ast_mutex_lock(&c0->lock);
	while(ast_mutex_trylock(&c1->lock)) {
		ast_mutex_unlock(&c0->lock);
		usleep(1);
		ast_mutex_lock(&c0->lock);
	}

	/* Find channel driver interfaces */
	pr0 = get_proto(c0);
	pr1 = get_proto(c1);
	if (!pr0) {
		ast_log(LOG_WARNING, "Can't find native functions for channel '%s'\n", c0->name);
		ast_mutex_unlock(&c0->lock);
		ast_mutex_unlock(&c1->lock);
		return AST_BRIDGE_FAILED;
	}
	if (!pr1) {
		ast_log(LOG_WARNING, "Can't find native functions for channel '%s'\n", c1->name);
		ast_mutex_unlock(&c0->lock);
		ast_mutex_unlock(&c1->lock);
		return AST_BRIDGE_FAILED;
	}

	/* Get channel specific interface structures */
	pvt0 = c0->tech_pvt;
	pvt1 = c1->tech_pvt;

	/* Get audio and video interface (if native bridge is possible) */
	p0 = pr0->get_rtp_info(c0);
	if (pr0->get_vrtp_info)
		vp0 = pr0->get_vrtp_info(c0);
	else
		vp0 = NULL;
	p1 = pr1->get_rtp_info(c1);
	if (pr1->get_vrtp_info)
		vp1 = pr1->get_vrtp_info(c1);
	else
		vp1 = NULL;

	/* Check if bridge is still possible (In SIP canreinvite=no stops this, like NAT) */
	if (!p0 || !p1) {
		/* Somebody doesn't want to play... */
		ast_mutex_unlock(&c0->lock);
		ast_mutex_unlock(&c1->lock);
		return AST_BRIDGE_FAILED_NOWARN;
	}
	codec0 = ast_codec_pref_bits(&c0->nativeformats);
	codec1 = ast_codec_pref_bits(&c1->nativeformats);
	memcpy(&codecs0, &c0->nativeformats, sizeof(codecs0));
	memcpy(&codecs1, &c1->nativeformats, sizeof(codecs1));
	/* Hey, we can't do reinvite if both parties speak different codecs */
	if (!(codec0 & codec1)) {
	    if (option_debug)
		ast_log(LOG_DEBUG, "Channel codec0 = %d is not codec1 = %d, cannot native bridge in RTP.\n", codec0, codec1);
	    ast_mutex_unlock(&c0->lock);
	    ast_mutex_unlock(&c1->lock);
	    return AST_BRIDGE_FAILED_NOWARN;
	}

	/* XXX Workaround for non-codec-capability mismatch:
	 * We can't do reinvite if both parties support different DTMF modes */
	ast_rtp_get_current_formats(p0, &dummy, &noncodec0);
	ast_rtp_get_current_formats(p1, &dummy, &noncodec1);
	if (noncodec0 != noncodec1) {
	    if (option_debug)
		ast_log(LOG_DEBUG, "Channel noncodec0 = %d is not noncodec1 = %d, cannot native bridge in RTP.\n", noncodec0, noncodec1);
	    ast_mutex_unlock(&c0->lock);
	    ast_mutex_unlock(&c1->lock);
	    return AST_BRIDGE_FAILED_NOWARN;
	}

	/* Ok, we should be able to redirect the media. Start with one channel */
	if (pr0->set_rtp_peer(c0, p1, vp1, &codecs1, ast_test_flag(p1, FLAG_NAT_ACTIVE))) 
		ast_log(LOG_WARNING, "Channel '%s' failed to talk to '%s'\n", c0->name, c1->name);
	else {
		/* Store RTP peer */
		ast_rtp_get_peer(p1, &ac1);
		if (vp1)
			ast_rtp_get_peer(vp1, &vac1);
	}
	/* Then test the other channel */
	if (pr1->set_rtp_peer(c1, p0, vp0, &codecs0, ast_test_flag(p0, FLAG_NAT_ACTIVE)))
		ast_log(LOG_WARNING, "Channel '%s' failed to talk back to '%s'\n", c1->name, c0->name);
	else {
		/* Store RTP peer */
		ast_rtp_get_peer(p0, &ac0);
		if (vp0)
			ast_rtp_get_peer(vp0, &vac0);
	}
	ast_mutex_unlock(&c0->lock);
	ast_mutex_unlock(&c1->lock);
	/* External RTP Bridge up, now loop and see if something happes that force us to take the
		media back to Asterisk */
	cs[0] = c0;
	cs[1] = c1;
	cs[2] = NULL;
	oldcodec0 = codec0;
	oldcodec1 = codec1;
	for (;;) {
		/* Check if something changed... */
		if ((c0->tech_pvt != pvt0)  ||
			(c1->tech_pvt != pvt1) ||
			(c0->masq || c0->masqr || c1->masq || c1->masqr)) {
				ast_log(LOG_DEBUG, "Oooh, something is weird, backing out\n");
				if (c0->tech_pvt == pvt0) {
					if (pr0->set_rtp_peer(c0, NULL, NULL, NULL, 0)) 
						ast_log(LOG_WARNING, "Channel '%s' failed to break RTP bridge\n", c0->name);
				}
				if (c1->tech_pvt == pvt1) {
					if (pr1->set_rtp_peer(c1, NULL, NULL, NULL, 0)) 
						ast_log(LOG_WARNING, "Channel '%s' failed to break RTP bridge\n", c1->name);
				}
				return AST_BRIDGE_RETRY;
		}
		/* Now check if they have changed address */
		ast_rtp_get_peer(p1, &t1);
		ast_rtp_get_peer(p0, &t0);
		codec0 = ast_codec_pref_bits(&c0->nativeformats);
		codec1 = ast_codec_pref_bits(&c1->nativeformats);
		if (vp1)
			ast_rtp_get_peer(vp1, &vt1);
		if (vp0)
			ast_rtp_get_peer(vp0, &vt0);
		if (ast_sockaddr_cmp(&t1, &ac1) || (vp1 && ast_sockaddr_cmp(&vt1, &vac1)) || (codec1 != oldcodec1)) {
			if (option_debug > 1) {
				ast_log(LOG_DEBUG, "Oooh, '%s' changed end address to %s:%d (format %d)\n", 
					c1->name, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &t1), ast_sockaddr_port(&t1), codec1);
				ast_log(LOG_DEBUG, "Oooh, '%s' changed end vaddress to %s:%d (format %d)\n", 
					c1->name, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &vt1), ast_sockaddr_port(&vt1), codec1);
				ast_log(LOG_DEBUG, "Oooh, '%s' was %s:%d/(format %d)\n", 
					c1->name, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &ac1), ast_sockaddr_port(&ac1), oldcodec1);
				ast_log(LOG_DEBUG, "Oooh, '%s' was %s:%d/(format %d)\n", 
					c1->name, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &vac1), ast_sockaddr_port(&vac1), oldcodec1);
			}
                        memcpy(&codecs1, &c1->nativeformats, sizeof(codecs1));
                        ast_codec_pref_append_missing2(&codecs1, ast_compatible_audio_formats(codecs1.audio_bits));
			if (pr0->set_rtp_peer(c0, !ast_sockaddr_isnull(&t1) ? p1 : NULL, !ast_sockaddr_isnull(&vt1) ? vp1 : NULL, &codecs1, ast_test_flag(p1, FLAG_NAT_ACTIVE))) 
				ast_log(LOG_WARNING, "Channel '%s' failed to update to '%s'\n", c0->name, c1->name);
			ast_sockaddr_copy(&ac1, &t1);
			ast_sockaddr_copy(&vac1, &vt1);
			oldcodec1 = codec1;
		}
		if (ast_sockaddr_cmp(&t0, &ac0) || (vp0 && ast_sockaddr_cmp(&vt0, &vac0))) {
			if (option_debug) {
				ast_log(LOG_DEBUG, "Oooh, '%s' changed end address to %s:%d (format %d)\n", 
					c0->name, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &t0), ast_sockaddr_port(&t0), codec0);
				ast_log(LOG_DEBUG, "Oooh, '%s' was %s:%d/(format %d)\n", 
					c0->name, ast_sockaddr_to_str(iabuf, sizeof(iabuf), &ac0), ast_sockaddr_port(&ac0), oldcodec0);
			}
                        memcpy(&codecs0, &c0->nativeformats, sizeof(codecs0));
                        ast_codec_pref_append_missing2(&codecs0, ast_compatible_audio_formats(codecs0.audio_bits));
			if (pr1->set_rtp_peer(c1, !ast_sockaddr_isnull(&t0) ? p0 : NULL, !ast_sockaddr_isnull(&vt0) ? vp0 : NULL, &codecs0, ast_test_flag(p0, FLAG_NAT_ACTIVE)))
				ast_log(LOG_WARNING, "Channel '%s' failed to update to '%s'\n", c1->name, c0->name);
			ast_sockaddr_copy(&ac0, &t0);
			ast_sockaddr_copy(&vac0, &vt0);
			oldcodec0 = codec0;
		}
		who = ast_waitfor_n(cs, 2, &timeoutms);
		if (!who) {
			if (!timeoutms) 
				return AST_BRIDGE_RETRY;
			if (option_debug)
				ast_log(LOG_DEBUG, "Ooh, empty read...\n");
			/* check for hangup / whentohangup */
			if (ast_check_hangup(c0) || ast_check_hangup(c1))
				break;
			continue;
		}
		f = ast_read(who);
		if (!f || ((f->frametype == AST_FRAME_DTMF) &&
				   (((who == c0) && (flags & AST_BRIDGE_DTMF_CHANNEL_0)) || 
			       ((who == c1) && (flags & AST_BRIDGE_DTMF_CHANNEL_1))))) {
			*fo = f;
			*rc = who;
			if (option_debug)
				ast_log(LOG_DEBUG, "Oooh, got a %s\n", f ? "digit" : "hangup");
			if ((c0->tech_pvt == pvt0) && (!c0->_softhangup)) {
				if (pr0->set_rtp_peer(c0, NULL, NULL, 0, 0)) 
					ast_log(LOG_WARNING, "Channel '%s' failed to break RTP bridge\n", c0->name);
			}
			if ((c1->tech_pvt == pvt1) && (!c1->_softhangup)) {
				if (pr1->set_rtp_peer(c1, NULL, NULL, 0, 0)) 
					ast_log(LOG_WARNING, "Channel '%s' failed to break RTP bridge\n", c1->name);
			}
			return AST_BRIDGE_COMPLETE;
		} else if ((f->frametype == AST_FRAME_CONTROL) && !(flags & AST_BRIDGE_IGNORE_SIGS)) {
			if ((f->subclass == AST_CONTROL_HOLD) || (f->subclass == AST_CONTROL_UNHOLD) ||
			    (f->subclass == AST_CONTROL_VIDUPDATE)) {
				ast_indicate(who == c0 ? c1 : c0, f->subclass);
				ast_frfree(f);
			} else {
				*fo = f;
				*rc = who;
				ast_log(LOG_DEBUG, "Got a FRAME_CONTROL (%d) frame on channel %s\n", f->subclass, who->name);
				return AST_BRIDGE_COMPLETE;
			}
		} else {
			if ((f->frametype == AST_FRAME_DTMF) || 
				(f->frametype == AST_FRAME_VOICE) || 
				(f->frametype == AST_FRAME_VIDEO)) {
				/* Forward voice or DTMF frames if they happen upon us */
				if (who == c0) {
					ast_write(c1, f);
				} else if (who == c1) {
					ast_write(c0, f);
				}
			}
			ast_frfree(f);
		}
		/* Swap priority not that it's a big deal at this point */
		cs[2] = cs[0];
		cs[0] = cs[1];
		cs[1] = cs[2];
		
	}
	return AST_BRIDGE_FAILED;
}

static int rtp_do_debug_ip(int fd, int argc, char *argv[])
{
	char iabuf[INET6_ADDRSTRLEN];
	char *arg;

	if (argc != 4)
		return RESULT_SHOWUSAGE;

	arg = ast_strdupa(argv[3]);

	if (!ast_sockaddr_parse(&rtpdebugaddr, arg, 0))
		return RESULT_SHOWUSAGE;

	ast_cli(fd, "RTCP Debugging Enabled for address: %s:%d\n",
		ast_sockaddr_to_str(iabuf, sizeof(iabuf), &rtpdebugaddr), ast_sockaddr_port(&rtpdebugaddr));
	rtpdebug = 1;
	return RESULT_SUCCESS;
}

static int rtp_do_debug(int fd, int argc, char *argv[])
{
	if(argc != 2) {
		if(argc != 4)
			return RESULT_SHOWUSAGE;
		return rtp_do_debug_ip(fd, argc, argv);
	}
	rtpdebug = 1;
	memset(&rtpdebugaddr,0,sizeof(rtpdebugaddr));
	ast_cli(fd, "RTP Debugging Enabled\n");
	return RESULT_SUCCESS;
}
   
static int rtp_no_debug(int fd, int argc, char *argv[])
{
	if(argc !=3)
		return RESULT_SHOWUSAGE;
	rtpdebug = 0;
	ast_cli(fd,"RTP Debugging Disabled\n");
	return RESULT_SUCCESS;
}

static char debug_usage[] =
  "Usage: rtp debug [ip host[:port]]\n"
  "       Enable dumping of all RTP packets to and from host.\n";

static char no_debug_usage[] =
  "Usage: rtp no debug\n"
  "       Disable all RTP debugging\n";

static struct ast_cli_entry  cli_debug_ip =
{{ "rtp", "debug", "ip", NULL } , rtp_do_debug, "Enable RTP debugging on IP", debug_usage };

static struct ast_cli_entry  cli_debug =
{{ "rtp", "debug", NULL } , rtp_do_debug, "Enable RTP debugging", debug_usage };

static struct ast_cli_entry  cli_no_debug =
{{ "rtp", "no", "debug", NULL } , rtp_no_debug, "Disable RTP debugging", no_debug_usage };

void ast_rtp_reload(int check_conf_file)
{
	struct ast_config *cfg;
	char *s;

	if (!ast_config_file_md5_update("rtp.conf", conf_file_md5) && check_conf_file)
	{
		ast_log(LOG_DEBUG, "Skipping reload since rtp.conf was not changed\n");
		return;
	}

	rtpstart = 5000;
	rtpend = 31000;
	dtmftimeout = DEFAULT_DTMF_TIMEOUT;
	dtmfpayload = DEFAULT_DTMF_PAYLOAD;
	mssclamping = 0; /* Disabled by default */
	rtpstatistics = 0;
	rtpstatistics_period = 5000;

	cfg = ast_config_load("rtp.conf");
	if (cfg) {
		if ((s = ast_variable_retrieve(cfg, "general", "dtmfpayload"))) {
			dtmfpayload = atoi(s);
			if (dtmfpayload >= MAX_RTP_PT || dtmfpayload <= MAX_RTP_STATIC_PT)
				dtmfpayload = DEFAULT_DTMF_PAYLOAD;
		}
		if ((s = ast_variable_retrieve(cfg, "general", "rtpstart"))) {
			rtpstart = atoi(s);
			if (rtpstart < 1024)
				rtpstart = 1024;
			if (rtpstart > 65535)
				rtpstart = 65535;
		}
		if ((s = ast_variable_retrieve(cfg, "general", "rtpend"))) {
			rtpend = atoi(s);
			if (rtpend < 1024)
				rtpend = 1024;
			if (rtpend > 65535)
				rtpend = 65535;
		}
		if ((s = ast_variable_retrieve(cfg, "general", "rtpchecksums"))) {
#ifdef SO_NO_CHECK
			if (ast_false(s))
				nochecksums = 1;
			else
				nochecksums = 0;
#else
			if (ast_false(s))
				ast_log(LOG_WARNING, "Disabling RTP checksums is not supported on this operating system!\n");
#endif
		}
		if ((s = ast_variable_retrieve(cfg, "general", "mssclamping"))) 
			mssclamping = atoi(s);
		if ((s = ast_variable_retrieve(cfg, "general", "dtmftimeout"))) {
			dtmftimeout = atoi(s);
			if ((dtmftimeout < 0) || (dtmftimeout > 20000)) {
				ast_log(LOG_WARNING, "DTMF timeout of '%d' outside range, using default of '%d' instead\n",
					dtmftimeout, DEFAULT_DTMF_TIMEOUT);
				dtmftimeout = DEFAULT_DTMF_TIMEOUT;
			};
		}
		if ((s = ast_variable_retrieve(cfg, "general", "rtpstatistics"))) 
			rtpstatistics = ast_true(s);
		if (rtpstatistics && 
			(s = ast_variable_retrieve(cfg, "general", "rtpstatistics_period")))
		{
			rtpstatistics_period = atoi(s);
		}

		ast_config_destroy(cfg);
	}
	if (rtpstart >= rtpend) {
		ast_log(LOG_WARNING, "Unreasonable values for RTP start/end port in rtp.conf\n");
		rtpstart = 5000;
		rtpend = 31000;
	}
	if (option_verbose > 1)
		ast_verbose(VERBOSE_PREFIX_2 "RTP Allocating from port range %d -> %d\n", rtpstart, rtpend);
	
}

/*--- ast_rtp_init: Initialize the RTP system in Asterisk */
void ast_rtp_init(void)
{
	ast_cli_register(&cli_debug);
	ast_cli_register(&cli_debug_ip);
	ast_cli_register(&cli_no_debug);
	ast_rtp_reload(0);
}

static int rtp_stats_get(struct ast_rtp *rtp, rtp_stats_t *stats)
{
	if (ioctl(rtp->fd, RTP_STATS_GET, stats))
	{
		ast_log(LOG_ERROR, "RTP_STATS_GET failed\n");
		return 1;
	}

	return 0;
}

void rtp_stats_accumulate(rtp_stats_t *res_stats, struct ast_rtp *rtp)
{
	rtp_stats_t stats = {};
	
	if (rtp_stats_get(rtp, &stats))
		return;

    /* Peer statistics */
	res_stats->rx_packets += stats.rx_packets;
	res_stats->tx_packets += stats.tx_packets;
	res_stats->rx_octets += stats.rx_octets;
	res_stats->tx_octets += stats.tx_octets;
	res_stats->rx_packets_lost += stats.rx_packets_lost;
	res_stats->reported_packets_lost += stats.reported_packets_lost;
	res_stats->rx_jitter += stats.rx_jitter;
	res_stats->reported_jitter += stats.reported_jitter;
	res_stats->round_trip_delay += stats.round_trip_delay;
	
	/* RTCP interval statisctics */
	res_stats->rx_loss_rate = stats.rx_loss_rate;
	res_stats->reported_loss_rate = stats.reported_loss_rate;

    /* Call statistics */
	res_stats->avg_round_trip_delay = stats.avg_round_trip_delay;
	res_stats->rx_fraction_sum = stats.rx_fraction_sum;
	res_stats->tx_fraction_sum = stats.tx_fraction_sum;
	res_stats->num_of_rtcp_sent = stats.num_of_rtcp_sent;
	res_stats->num_of_rtcp_received = stats.num_of_rtcp_received;
}

void ast_rtp_set_inband_dtmf(struct ast_rtp *rtp, int inband_dtmf)
{
    	rtp->inband_dtmf = inband_dtmf;
}

int ast_rtp_get_inband_dtmf(struct ast_rtp *rtp)
{
    	return rtp->inband_dtmf;
}

/* convert 1/256 units to percents => (f/256)*100 */
#define RTCP_FRACTION_TO_PERCENT(f) (((f)*100)/256)

/* convert 1/65536 sec units to milliseconds => (rtt/65536)*1000 */
#define RTCP_RTT_TO_MS(rtt) (((rtt)*1000)/65536)

static int rtp_stats_cb(void *data)
{
	rtp_stats_t stats;
	struct ast_rtp *rtp = data;
	struct ast_channel *chan = NULL;
	struct ast_cdr *cdr = NULL;

	if (ioctl(rtp->fd, RTP_STATS_GET, &stats))
	{
		ast_log(LOG_ERROR, "RTP_STATS_GET failed\n");
		return 1; /* run again */
	}

        chan = ast_get_cdr_channel_owner_of_rtp(rtp);
        if (chan && (cdr = chan->cdr))
	{
	    rtp_addrs_t addrs;

	    cdr->tx_packets = stats.tx_packets;
	    cdr->rx_packets = stats.rx_packets;
	    cdr->lost_packets = stats.rx_packets_lost;

	    if (ioctl(rtp->fd, RTP_ADDRS_GET, &addrs))
	    {
		ast_log(LOG_ERROR, "RTP_ADDRS_GET failed\n");
	    }
	    else
	    {
	        struct ast_channel *sip_chan;

		ast_sockaddr_from_storage(&cdr->them, &addrs.dest_addr);

                sip_chan = chan;
		if (!sip_chan->tech || !sip_chan->tech->get_ourip)
                    sip_chan = ast_bridged_channel(chan);
		    
                if (sip_chan && sip_chan->tech && sip_chan->tech->get_ourip)
		    cdr->us = sip_chan->tech->get_ourip(sip_chan);
	    }
	}

	if (chan)
	    ast_mutex_unlock(&chan->lock);

	manager_event(EVENT_FLAG_SYSTEM, RTP_STATS_EVENT, "id:%s:rx_j:%u:"
		"arx_j:%u:rx_lp:%u:tx_j:%u:atx_j:%u:tx_lp:%u:rtt:%u:artt:%u\n",
		rtp->stats_id, stats.rx_jitter, stats.avg_rx_jitter,
		RTCP_FRACTION_TO_PERCENT(stats.rx_loss_rate),
		stats.reported_jitter, stats.avg_reported_jitter,
		RTCP_FRACTION_TO_PERCENT(stats.reported_loss_rate),
		RTCP_RTT_TO_MS(stats.round_trip_delay),
		RTCP_RTT_TO_MS(stats.avg_round_trip_delay));

	return 1; /* run again */
}

AST_MUTEX_DEFINE_STATIC(rand_lock);
static force_inline int thread_safe_rand(void)
{
	int val;

	ast_mutex_lock(&rand_lock);
	val = rand();
	ast_mutex_unlock(&rand_lock);
	
	return val;
}

void ast_rtp_stats_logging_start(struct ast_rtp *rtp, int is_outgoing,
	char *username)
{
	if (!rtpstatistics)
		return;

	snprintf(rtp->stats_id, MAX_ID_LEN, "%08x", thread_safe_rand());
	rtp->stats_timer = ast_sched_add(rtp->sched, rtpstatistics_period,
		rtp_stats_cb, rtp);

	manager_event(EVENT_FLAG_SYSTEM, RTP_STATS_EVENT,
	    "id:%s:call_start:%s:%s\n", 
		rtp->stats_id, is_outgoing ? "Outgoing" : "Incoming",
		username);
}

void ast_rtp_stats_logging_stop(struct ast_rtp *rtp, int is_abnormal_end)
{
	if (!rtpstatistics)
		return;

	manager_event(EVENT_FLAG_SYSTEM, RTP_STATS_EVENT, "id:%s:call_end:%s\n",
		rtp->stats_id, is_abnormal_end ? "abnormal" : "normal");
	if (rtp->stats_timer > -1)
		ast_sched_del(rtp->sched, rtp->stats_timer);
	rtp->stats_timer = -1;
}

void ast_rtp_setrtcpinterval(struct ast_rtp *rtp, int interval)
{
    ioctl(rtp->fd, RTP_RTCP_SET_INTERVAL, interval);
}

u32 ast_rtp_get_time_since_last(void)
{
    u32 params = -1;
    int fd = gpl_sys_rg_chrdev_open(KOS_CDT_RTP, O_RDWR);

    if (fd < 0)
    {
	ast_log(LOG_ERROR, "Unable to open jrtp char device\n");
	return 0;
    }

    if (ioctl(fd, RTP_SECS_SINCE_LAST_RTP_GET, &params))
	ast_log(LOG_ERROR, "Unable to get last rx rtp timestamp from jrtp\n");

    close(fd);
    return params;
}

/* eth+ppp+IP+UDP overhead is calculated for 10ms (worst case) ptime */
#define UDP_RTP_HEADER_BANDWIDTH 48000
long ast_rtp_get_codec_bw(int codec)
{
	switch (codec)
	{
	case AST_FORMAT_ULAW:
	case AST_FORMAT_ALAW:
	case AST_FORMAT_G722:
		return 64000 + UDP_RTP_HEADER_BANDWIDTH;
	case AST_FORMAT_G726:
		return 32000 + UDP_RTP_HEADER_BANDWIDTH;
	case AST_FORMAT_G723_1:
		return 6300 + UDP_RTP_HEADER_BANDWIDTH;
	case AST_FORMAT_G729A:
		return 8000 + UDP_RTP_HEADER_BANDWIDTH;
	default:
		return 64000 + UDP_RTP_HEADER_BANDWIDTH;
	}
}
