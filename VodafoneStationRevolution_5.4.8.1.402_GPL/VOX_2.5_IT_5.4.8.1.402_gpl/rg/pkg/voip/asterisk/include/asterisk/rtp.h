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
 * \file rtp.h
 * \brief Supports RTP and RTCP with Symmetric RTP support for NAT traversal.
 * 
 * RTP is defined in RFC 3550.
 */

#ifndef _ASTERISK_RTP_H
#define _ASTERISK_RTP_H

#include "asterisk/frame.h"
#include "asterisk/io.h"
#include "asterisk/sched.h"
#include "asterisk/channel.h"
#include "asterisk/netsock2.h"

#include <netinet/in.h>
#include <rtp/rtp.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Codes for RTP-specific data - not defined by our AST_FORMAT codes */
/*! DTMF (RFC2833) */
#define AST_RTP_DTMF            (1 << 0)
/*! 'Comfort Noise' (RFC3389) */
#define AST_RTP_CN              (1 << 1)
/*! DTMF (Cisco Proprietary) */
#define AST_RTP_CISCO_DTMF      (1 << 2)
/*! Maximum RTP-specific code */
#define AST_RTP_MAX             AST_RTP_CISCO_DTMF

struct ast_rtp_protocol {
	/* Get RTP struct, or NULL if unwilling to transfer */
	struct ast_rtp *(* const get_rtp_info)(struct ast_channel *chan);
	/* Get RTP struct, or NULL if unwilling to transfer */
	struct ast_rtp *(* const get_vrtp_info)(struct ast_channel *chan);
	/* Set RTP peer */
	int (* const set_rtp_peer)(struct ast_channel *chan, struct ast_rtp *peer, struct ast_rtp *vpeer, const struct ast_codec_pref *codecs, int nat_active);
/*	int (* const get_codec)(struct ast_channel *chan);*/
	const char * const type;
	struct ast_rtp_protocol *next;
};

/* The value of each payload format mapping: */
struct rtpPayloadType {
	int isAstFormat;     /* whether the following code is an AST_FORMAT */
	int code;
};

/*!
 * \brief Structure representing a RTP session.
 * 
 * RTP session is defined on page 9 of RFC 3550: "An association among a set of participants communicating with RTP.  A participant may be involved in multiple RTP sessions at the same time [...]"
 * 
 */
struct ast_rtp;

typedef int (*ast_rtp_callback)(struct ast_rtp *rtp, struct ast_frame *f, void *data);

/*!
 * \brief Initializate a RTP session.
 * 
 * \param sched
 * \param io
 * \param rtcpenable
 * \param callbackmode
 * \returns A representation (structure) of an RTP session.
 */
struct ast_rtp *ast_rtp_new(struct sched_context *sched, struct io_context *io, int rtcpenable, int callbackmode);

/*!
 * \brief Initializate a RTP session using an in_addr structure.
 * 
 * This fuction gets called by ast_rtp_new().
 * 
 * \param sched
 * \param io
 * \param rtcpenable
 * \param callbackmode
 * \param in
 * \returns A representation (structure) of an RTP session.
 */
struct ast_rtp *ast_rtp_new_with_bindaddr(struct sched_context *sched, struct io_context *io, int rtcpenable, int callbackmode, struct ast_sockaddr *addr);

void ast_rtp_set_peer(struct ast_rtp *rtp, struct ast_sockaddr *them, int mode);

int ast_rtp_get_peer(struct ast_rtp *rtp, struct ast_sockaddr *them);

void ast_rtp_get_us(struct ast_rtp *rtp, struct ast_sockaddr *us);

void ast_rtp_destroy(struct ast_rtp *rtp);

void ast_rtp_reset(struct ast_rtp *rtp);

void ast_rtp_set_callback(struct ast_rtp *rtp, ast_rtp_callback callback);

void ast_rtp_set_data(struct ast_rtp *rtp, void *data);

int ast_rtp_write(struct ast_rtp *rtp, struct ast_frame *f);

struct ast_frame *ast_rtp_read(struct ast_rtp *rtp);

struct ast_frame *ast_rtcp_read(struct ast_rtp *rtp);

int ast_rtp_fd(struct ast_rtp *rtp);

void rtp_stats_accumulate(rtp_stats_t *res_stats, struct ast_rtp *rtp);

int ast_rtcp_fd(struct ast_rtp *rtp);

int ast_rtp_senddigit_begin(struct ast_rtp *rtp, char digit);

int ast_rtp_senddigit_end(struct ast_rtp *rtp, char digit);

int ast_rtp_sendcng(struct ast_rtp *rtp, int level);

int ast_rtp_settos(struct ast_rtp *rtp, int tos);

int ast_rtp_set_so_mark(struct ast_rtp *rtp, int so_mark);

/*  Setting RTP payload types from lines in a SDP description: */
void ast_rtp_pt_clear(struct ast_rtp* rtp);
/* Set payload types to defaults */
void ast_rtp_pt_default(struct ast_rtp* rtp);
void ast_rtp_set_m_type(struct ast_rtp* rtp, int pt);
void ast_rtp_set_rtpmap_type(struct ast_rtp* rtp, int pt,
			 char* mimeType, char* mimeSubtype, unsigned int sample_rate);

/*  Mapping between RTP payload format codes and Asterisk codes: */
struct rtpPayloadType ast_rtp_lookup_pt(struct ast_rtp* rtp, int pt);
int ast_rtp_lookup_code(struct ast_rtp* rtp, int isAstFormat, int code);
void ast_rtp_offered_from_local(struct ast_rtp* rtp, int local);

void ast_rtp_get_current_formats(struct ast_rtp* rtp,
			     int* astFormats, int* nonAstFormats);

unsigned int ast_rtp_lookup_sample_rate(int isAstFormat, const int code);

/*  Mapping an Asterisk code into a MIME subtype (string): */
char* ast_rtp_lookup_mime_subtype(int isAstFormat, int code);

/* Build a string of MIME subtype names from a capability list */
char *ast_rtp_lookup_mime_multiple(char *buf, int size, const int capability, const int isAstFormat);

void ast_rtp_setnat(struct ast_rtp *rtp, int nat);

void ast_rtp_setrtcpinterval(struct ast_rtp *rtp, int interval);

int ast_rtp_bridge(struct ast_channel *c0, struct ast_channel *c1, int flags, struct ast_frame **fo, struct ast_channel **rc, int timeoutms);

int ast_rtp_proto_register(struct ast_rtp_protocol *proto);

void ast_rtp_proto_unregister(struct ast_rtp_protocol *proto);

void ast_rtp_stop(struct ast_rtp *rtp);

void ast_rtp_init(void);

void ast_rtp_reload(int check_conf_file);

void ast_rtp_clear_payload_types(struct ast_rtp *rtp);

/* If media_payload_type is -1, rtp code will be used for media as well */
int ast_rtp_set_payload_type(struct ast_rtp *rtp, int media_payload_type,
	int rtp_code, int is_ast_format);

unsigned int ast_rtp_get_context(struct ast_rtp *rtp);

void ast_rtp_set_inband_dtmf(struct ast_rtp *rtp, int inband_dtmf);

int ast_rtp_get_inband_dtmf(struct ast_rtp *rtp);

char *ast_rtp_get_quality(struct ast_rtp *rtp);

void ast_rtp_stats_logging_start(struct ast_rtp *rtp, int is_outgoing,
	char *username);

void ast_rtp_stats_logging_stop(struct ast_rtp *rtp, int is_abnormal_end);

u32 ast_rtp_get_time_since_last(void);

/* get maximum bandwidth for the codec */
long ast_rtp_get_codec_bw(int codec);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _ASTERISK_RTP_H */
