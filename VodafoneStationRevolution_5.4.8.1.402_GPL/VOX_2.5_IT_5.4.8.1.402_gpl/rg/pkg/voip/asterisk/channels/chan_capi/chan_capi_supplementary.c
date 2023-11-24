/*
 * An implementation of Common ISDN API 2.0 for Asterisk
 *
 * Copyright (C) 2005-2009 Cytronics & Melware
 *
 * Armin Schindler <armin@melware.de>
 * 
 * This program is free software and may be modified and 
 * distributed under the terms of the GNU Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "chan_capi_platform.h"
#include "chan_capi20.h"
#include "chan_capi.h"
#include "chan_capi_supplementary.h"
#include "chan_capi_utils.h"
#include "asterisk/jdsp_common.h"


#define CCBSNR_TYPE_CCBS 1
#define CCBSNR_TYPE_CCNR 2

#define CCBSNR_AVAILABLE  1
#define CCBSNR_REQUESTED  2
#define CCBSNR_ACTIVATED  3

#define FACILITY_CAUSE_ACCEPT 0x0000
#define FACILITY_CAUSE_NOT_SUPPORTED 0x300b
#define FACILITY_CAUSE_NOT_SUPPORTED_STATE 0x2001

struct ccbsnr_s {
	char type;
	_cword id;
	unsigned int plci;
	unsigned int state;
	unsigned int handle;
	_cword mode;
	_cword rbref;
	char partybusy;
	char context[AST_MAX_CONTEXT];
	char exten[AST_MAX_EXTENSION];
	int priority;
	time_t age;
	struct ccbsnr_s *next;
};

static struct ccbsnr_s *ccbsnr_list = NULL;
AST_MUTEX_DEFINE_STATIC(ccbsnr_lock);

/*
 * remove too old CCBS/CCNR entries
 * (must be called with ccbsnr_lock held)
 */
static void del_old_ccbsnr(void)
{
	struct ccbsnr_s *ccbsnr;
	struct ccbsnr_s *tmp = NULL;

	ccbsnr = ccbsnr_list;
	while (ccbsnr) {
		if ((ccbsnr->age + 86400) < time(NULL)) {
			cc_verbose(1, 1, VERBOSE_PREFIX_3 CC_MESSAGE_NAME
				": CCBS/CCNR handle=%d timeout.\n", ccbsnr->handle);
			if (!tmp) {
				ccbsnr_list = ccbsnr->next;
			} else {
				tmp->next = ccbsnr->next;
			}
			free(ccbsnr);
			break;
		}
		tmp = ccbsnr;
		ccbsnr = ccbsnr->next;
	}
}

/*
 * cleanup CCBS/CCNR ids
 */
void cleanup_ccbsnr(void)
{
	struct ccbsnr_s *ccbsnr;
	struct ccbsnr_s *tmp = NULL;

	cc_mutex_lock(&ccbsnr_lock);
	ccbsnr = ccbsnr_list;
	while (ccbsnr) {
		tmp = ccbsnr;
		ccbsnr = ccbsnr->next;
		free(tmp);
	}
	cc_mutex_unlock(&ccbsnr_lock);
}

/*
 * return the controller of ccbsnr handle
 */
unsigned int capi_get_ccbsnrcontroller(unsigned int handle)
{
	unsigned int contr = 0;
	struct ccbsnr_s *ccbsnr;
	
	cc_mutex_lock(&ccbsnr_lock);
	ccbsnr = ccbsnr_list;
	while (ccbsnr) {
		if (ccbsnr->handle == handle) {
			contr = (ccbsnr->plci & 0xff);
			break;
		}
		ccbsnr = ccbsnr->next;
	}
	cc_mutex_unlock(&ccbsnr_lock);

	return contr;
}

/*
 * a new CCBS/CCNR id was received
 */
static void new_ccbsnr_id(char type, unsigned int plci,
	_cword id, struct capi_pvt *i)
{
	char buffer[CAPI_MAX_STRING];
	struct ccbsnr_s *ccbsnr;

	ccbsnr = malloc(sizeof(struct ccbsnr_s));
	if (ccbsnr == NULL) {
		cc_log(LOG_ERROR, "Unable to allocate CCBS/CCNR struct.\n");
		return;
	}
	memset(ccbsnr, 0, sizeof(struct ccbsnr_s));

    ccbsnr->age = time(NULL);
    ccbsnr->type = type;
    ccbsnr->id = id;
    ccbsnr->rbref = 0xdead;
    ccbsnr->plci = plci;
    ccbsnr->state = CCBSNR_AVAILABLE;
    ccbsnr->handle = (id | ((plci & 0xff) << 16) | (type << 28));

	if (i->peer) {
		snprintf(buffer, CAPI_MAX_STRING-1, "%u", ccbsnr->handle);
		pbx_builtin_setvar_helper(i->peer, "CCLINKAGEID", buffer);
	} else {
		cc_log(LOG_NOTICE, "No peerlink found to set CCBS/CCNR linkage ID.\n");
	}

	cc_mutex_lock(&ccbsnr_lock);
	del_old_ccbsnr();
	ccbsnr->next = ccbsnr_list;
	ccbsnr_list = ccbsnr;
	cc_mutex_unlock(&ccbsnr_lock);

	cc_verbose(1, 1, VERBOSE_PREFIX_3
		"%s: PLCI=%#x CCBS/CCNR new id=0x%04x handle=%d\n",
		i->vname, plci, id, ccbsnr->handle);

	/* if the hangup frame was deferred, it can be done now and here */
	if (i->whentoqueuehangup) {
		i->whentoqueuehangup = 0;
		capi_queue_cause_control(i, 1);
	}
}

/*
 * return the pointer to ccbsnr structure by handle
 */
static struct ccbsnr_s *get_ccbsnr_link(char type, unsigned int plci,
	unsigned int handle, _cword ref, unsigned int *state, char *busy)
{
	struct ccbsnr_s *ret;
	
	cc_mutex_lock(&ccbsnr_lock);
	ret = ccbsnr_list;
	while (ret) {
		if (((handle) && (ret->handle == handle)) ||
		    ((ref != 0xffff) && (ret->rbref == ref) &&
			 (ret->type == type) && ((ret->plci & 0xff) == (plci & 0xff)))) {
			if (state) {
				*state = ret->state;
			}
			if (busy) {
				*busy = ret->partybusy;
			}
			break;
		}
		ret = ret->next;
	}
	cc_mutex_unlock(&ccbsnr_lock);

	return ret;
}

/*
 * function to tell if CCBSNR is activated
 */
static int ccbsnr_tell_activated(void *data)
{
	unsigned int handle = (unsigned int)(unsigned long)data;
	int ret = 0;
	unsigned int state;

	if (get_ccbsnr_link(0, 0, handle, 0xffff, &state, NULL) != NULL) {
		if (state == CCBSNR_REQUESTED) {
			ret = 1;
		}
	}

	return ret;
}

/*
 * select CCBS/CCNR id
 */
static unsigned int select_ccbsnr_id(unsigned int id, char type,
	char *context, char *exten, int priority)
{
	struct ccbsnr_s *ccbsnr;
	int ret = 0;
	
	cc_mutex_lock(&ccbsnr_lock);
	ccbsnr = ccbsnr_list;
	while (ccbsnr) {
		if (((ccbsnr->plci & 0xff) == ((id >> 16) & 0xff)) &&
		   (ccbsnr->id == (id & 0xffff)) &&
		   (ccbsnr->type == type) &&
		   (ccbsnr->state == CCBSNR_AVAILABLE)) {
			strncpy(ccbsnr->context, context, sizeof(ccbsnr->context) - 1);
			strncpy(ccbsnr->exten, exten, sizeof(ccbsnr->exten) - 1);
			ccbsnr->priority = priority;
			ccbsnr->state = CCBSNR_REQUESTED;
			ret = ccbsnr->handle;
			cc_verbose(1, 1, VERBOSE_PREFIX_3 CC_MESSAGE_NAME
				": request CCBS/NR id=0x%x handle=%d (%s,%s,%d)\n",
				id, ret, context, exten, priority);
			break;
		}
		ccbsnr = ccbsnr->next;
	}
	cc_mutex_unlock(&ccbsnr_lock);
	
	return ret;
}

/*
 * a CCBS/CCNR ref was removed 
 */
static void del_ccbsnr_ref(unsigned int plci, _cword ref)
{
	struct ccbsnr_s *ccbsnr;
	struct ccbsnr_s *tmp = NULL;

	cc_mutex_lock(&ccbsnr_lock);
	ccbsnr = ccbsnr_list;
	while (ccbsnr) {
		if (((ccbsnr->plci & 0xff) == (plci & 0xff)) &&
		   (ccbsnr->rbref == ref)) {
			if (!tmp) {
				ccbsnr_list = ccbsnr->next;
			} else {
				tmp->next = ccbsnr->next;
			}
			free(ccbsnr);
			cc_verbose(1, 1, VERBOSE_PREFIX_3 CC_MESSAGE_NAME
				": PLCI=%#x CCBS/CCNR removed ref=0x%04x\n", plci, ref);
			break;
		}
		tmp = ccbsnr;
		ccbsnr = ccbsnr->next;
	}
	cc_mutex_unlock(&ccbsnr_lock);
}

/*
 * return rbref of CCBS/CCNR and delete entry
 */
_cword capi_ccbsnr_take_ref(unsigned int handle)
{
	unsigned int plci = 0;
	_cword rbref = 0xdead;
	struct ccbsnr_s *ccbsnr;
	
	cc_mutex_lock(&ccbsnr_lock);
	ccbsnr = ccbsnr_list;
	while (ccbsnr) {
		if (ccbsnr->handle == handle) {
			plci = ccbsnr->plci;
			rbref = ccbsnr->rbref;
			break;
		}
		ccbsnr = ccbsnr->next;
	}
	cc_mutex_unlock(&ccbsnr_lock);

	if (rbref != 0xdead) {
		del_ccbsnr_ref(plci, rbref);
	}

	return rbref;
}

/*
 * a CCBS/CCNR id was removed 
 */
static void del_ccbsnr_id(unsigned int plci, _cword id)
{
	struct ccbsnr_s *ccbsnr;
	struct ccbsnr_s *tmp = NULL;
	unsigned int oldstate;

	cc_mutex_lock(&ccbsnr_lock);
	ccbsnr = ccbsnr_list;
	while (ccbsnr) {
		if (((ccbsnr->plci & 0xff) == (plci & 0xff)) &&
		    (ccbsnr->id == id)) {
			oldstate = ccbsnr->state;
			if (ccbsnr->state == CCBSNR_AVAILABLE) {
				if (!tmp) {
					ccbsnr_list = ccbsnr->next;
				} else {
					tmp->next = ccbsnr->next;
				}
				free(ccbsnr);
				cc_verbose(1, 1, VERBOSE_PREFIX_3 CC_MESSAGE_NAME ": PLCI=%#x CCBS/CCNR removed "
					"id=0x%04x state=%d\n",	plci, id, oldstate);
			} else {
				/* just deactivate the linkage id */
				ccbsnr->id = 0xdead;
				cc_verbose(1, 1, VERBOSE_PREFIX_3 CC_MESSAGE_NAME ": PLCI=%#x CCBS/CCNR erase-only "
					"id=0x%04x state=%d\n",	plci, id, ccbsnr->state);
			}
			break;
		}
		tmp = ccbsnr;
		ccbsnr = ccbsnr->next;
	}
	cc_mutex_unlock(&ccbsnr_lock);
}

/*
 * on an activated CCBS, the remote party is now free
 */
static void	ccbsnr_remote_user_free(_cmsg *CMSG, char type, unsigned int PLCI, _cword rbref)
{
	struct ast_channel *c;
	struct ccbsnr_s *ccbsnr;
	char handlename[CAPI_MAX_STRING];
	int state = AST_STATE_DOWN;

	/* XXX start alerting , when answered use CCBS call */
	cc_mutex_lock(&ccbsnr_lock);
	ccbsnr = ccbsnr_list;
	while (ccbsnr) {
		if ((ccbsnr->type == type) &&
		    ((ccbsnr->plci & 0xff) == (PLCI & 0xff)) &&
		    (ccbsnr->rbref == rbref)) {
			break;
		}
		ccbsnr = ccbsnr->next;
	}
	cc_mutex_unlock(&ccbsnr_lock);

	if (!(ccbsnr)) {
		cc_log(LOG_ERROR, CC_MESSAGE_NAME " CCBS/CCBR reference not found!\n");
		return;
	}

	snprintf(handlename, CAPI_MAX_STRING-1, "%u", ccbsnr->handle);

#ifdef CC_AST_HAS_EXT_CHAN_ALLOC
	c = ast_channel_alloc(0, state, handlename, NULL,
#ifdef CC_AST_HAS_EXT2_CHAN_ALLOC
		0, ccbsnr->exten, ccbsnr->context,
#ifdef CC_AST_HAS_LINKEDID_CHAN_ALLOC
		NULL,
#endif
		0,
#endif
		"CCBSNR/%x", ccbsnr->handle);
#else
	c = ast_channel_alloc(0);
#endif
	
	if (c == NULL) {
		cc_log(LOG_ERROR, "Unable to allocate channel!\n");
		return;
	}

#ifndef CC_AST_HAS_EXT_CHAN_ALLOC
#ifdef CC_AST_HAS_STRINGFIELD_IN_CHANNEL
	ast_string_field_build(c, name, "CCBSNR/%x", ccbsnr->handle);
#else
	snprintf(c->name, sizeof(c->name) - 1, "CCBSNR/%x",
		ccbsnr->handle);
#endif
#endif
#ifndef CC_AST_HAS_VERSION_1_4
	c->type = "CCBS/CCNR";
#endif

	c->priority = ccbsnr->priority;

	if (c->cid.cid_num) {
		free(c->cid.cid_num);
	}
	c->cid.cid_num = strdup(handlename);
	if (c->cid.cid_dnid) {
		free(c->cid.cid_dnid);
	}
	c->cid.cid_dnid = strdup(ccbsnr->exten);

#ifndef CC_AST_HAS_EXT2_CHAN_ALLOC
	cc_copy_string(c->context, ccbsnr->context, sizeof(c->context));
	cc_copy_string(c->exten, ccbsnr->exten, sizeof(c->exten));
#endif

#ifndef CC_AST_HAS_EXT_CHAN_ALLOC
	ast_setstate(c, state);
#endif

	if (ast_pbx_start(c)) {
		cc_log(LOG_ERROR, CC_MESSAGE_NAME " CCBS/CCNR: Unable to start pbx!\n");
	} else {
		cc_verbose(2, 1, VERBOSE_PREFIX_2 "contr%d: started PBX for CCBS/CCNR callback (%s/%s/%d)\n",
			PLCI & 0xff, ccbsnr->context, ccbsnr->exten, ccbsnr->priority);
	}
}

/*
 * send Listen for supplementary to specified controller
 */
void ListenOnSupplementary(unsigned controller)
{
	_cmsg	CMSG;
	MESSAGE_EXCHANGE_ERROR error;
	int waitcount = 50;

	error = capi_sendf(NULL, 0, CAPI_FACILITY_REQ, controller, get_capi_MessageNumber(),
		"w(w(d))",
		FACILITYSELECTOR_SUPPLEMENTARY,
		0x0001,  /* LISTEN */
		0x0000079f
	);

	while (waitcount) {
		error = capidev_check_wait_get_cmsg(&CMSG);

		if (IS_FACILITY_CONF(&CMSG)) {
			break;
		}
		usleep(30000);
		waitcount--;
	}
	if (!waitcount) {
		cc_log(LOG_ERROR,"Unable to supplementary-listen on contr%d (error=0x%x)\n",
			controller, error);
	}
}

static void transfer_cdr(struct ast_channel *cdr_src_chan,
	struct ast_channel *cdr_target_chan)
{
	struct ast_channel *cdr_src_bridged = ast_bridged_channel(cdr_src_chan);

	if (cdr_src_chan->cdr)
	{
		/* Move CDR from second channel to current one */
		cdr_target_chan->cdr = 
			ast_cdr_append(cdr_target_chan->cdr, cdr_src_chan->cdr);
		cdr_src_chan->cdr = NULL;
	}
	if (cdr_src_bridged->cdr)
	{
		/* Move CDR from second channel's bridge to current one */
		cdr_target_chan->cdr = 
			ast_cdr_append(cdr_target_chan->cdr, cdr_src_bridged->cdr);
		cdr_src_bridged->cdr = NULL;
	}	
}

static struct capi_pvt *get_leg_by_call_identity(char *call_identity,
	int *suspended_exist)
{
	struct capi_pvt *pvt = capi_iflist; /* current call pvt */
	
	for (pvt = capi_iflist; pvt; pvt = pvt->next)
	{
		if (pvt->state == CAPI_STATE_SUSPEND)
		{
			*suspended_exist = 1;

			if(!strncmp(pvt->call_identity, call_identity, 
				sizeof(pvt->call_identity)))
			{
				break;
			}
		}
	}

	return pvt;
}

static struct capi_pvt *get_second_leg(struct capi_pvt *i)
{
	struct capi_pvt *pvt = capi_iflist; /* current call pvt */

	/* Find the other leg pvt - same jdsp_line but different PLCI */
	for (pvt = capi_iflist; pvt; pvt = pvt->next)
	{
		if(i->jdsp_line == pvt->jdsp_line && i->PLCI != pvt->PLCI)
			break;
	}

	return pvt;
}

static int start_transfer_call(struct capi_pvt *i)
{
	struct ast_channel *transferer = NULL, *transfered_holder = i->owner;	
	struct capi_pvt *pvt = get_second_leg(i); /* active call pvt */
	struct ast_channel *transferee = NULL, *transfered = NULL; 

	if (!pvt)
	{
		ast_log(LOG_WARNING, "No active call found, transfer failed\n");
		return -1;
	}

	/* get transfer channels */
	transferer = pvt->owner;

	ast_log(LOG_DEBUG, "Transfering call: Active call: chan %s, PLCI =%#x, "
		"Held call: chan %s, PLCI= %#x\n",transferer->name, pvt->PLCI,
		transfered_holder->name, i->PLCI);

	transferee = ast_bridged_channel(transferer);
	transfered = ast_bridged_channel(transfered_holder);
	if (i->internal_transfer_only &&
	    !jdsp_is_internal_call_leg(transfered, transferer))
	{
		ast_log(LOG_DEBUG, "The call cannot be transfered because of the "
			"internal call constraint\n");
		return -1;
	}
	ast_indicate(transfered, AST_CONTROL_UNHOLD);

	if (transferee)
	{
		/* Attended transfer */
		transfer_cdr(transferee, transfered_holder);
		/* Last chance to stop audio on the channel before it becomes a
		 * zombie */
		/* i is the pivot of transfer_holder */
		cc_jdsp_stop_audio(i, pvt->chan);
		if (ast_channel_masquerade(transfered_holder, transferee)) 
		{
			ast_log(LOG_WARNING, "Unable to masquerade %s as %s\n",
				transferee->name, transfered_holder->name);
			return -1;
		}
	}
	else if (transfered)
	{	/* Unnatended transfer - source channel still has no bridged one,
		 * because it's still ringing */ 
		transfer_cdr(transfered_holder, transferer);
		/* Last chance to stop audio on the channel before it becomes a
		 * zombie */
		/* pvt is the pivot of transferer */
		cc_jdsp_stop_audio(pvt, pvt->chan);
		if (ast_channel_masquerade(transferer, transfered))
		{
			ast_log(LOG_WARNING, "Unable to masquerade %s as %s\n",
				transfered->name, transferer->name);
			return -1;
		}
	}
	else
	{
		ast_log(LOG_DEBUG, "Neither %s nor %s are in a bridge, nothing to "
			"transfer\n", transferer->name, transfered_holder->name);
		return -1;
	}
	return 0;	
}

static int capi_call_suspend_timeout(void *data)
{
	struct capi_pvt *i = data;

	i->call_suspend_sched_id = -1;
	pbx_capi_hangup(i->owner);
	return 0;
}

static void handle_facility_indication_supp_suspend(_cmsg *CMSG, 
	unsigned int PLCI, struct capi_pvt *i)
{
	struct ast_frame fr = { AST_FRAME_CONTROL, AST_CONTROL_HOLD, };
	char call_identity[CAPI_MAX_CALL_ID_SIZE + 1];
	int suspended = 0;
	int len = MIN(sizeof(call_identity) - 1, 
		FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[4]);

	strncpy(call_identity,
		&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[5], len);
	call_identity[len] = '\0';

	if (get_leg_by_call_identity(call_identity, &suspended))
	{
		capi_sendf(NULL, 0, CAPI_FACILITY_RESP, PLCI, HEADER_MSGNUM(CMSG), 
			"w(w())w", FACILITYSELECTOR_SUPPLEMENTARY, 0x0004,
			0x3400 | AST_CAUSE_CALL_ID_USED);
		return;
	}

	i->state = CAPI_STATE_SUSPEND;
	i->onholdPLCI = i->PLCI;

	cc_jdsp_stop_audio(i, i->chan);

	local_queue_frame(i, &fr);

	strncpy(i->call_identity, call_identity, sizeof(i->call_identity));

	i->call_suspend_sched_id = ast_sched_add(sched,
		CALL_SUSPEND_TIMEOUT * 1000, capi_call_suspend_timeout, i);

	capi_sendf(NULL, 0, CAPI_FACILITY_RESP, PLCI, HEADER_MSGNUM(CMSG), 
		"w(w())w", FACILITYSELECTOR_SUPPLEMENTARY, 0x0004,
		FACILITY_CAUSE_ACCEPT);
}

static void handle_facility_indication_supp_resume(_cmsg *CMSG, 
	unsigned int PLCI)
{
	char call_identity[CAPI_MAX_CALL_ID_SIZE + 1];
	char call_id_send[CAPI_MAX_CALL_ID_SIZE + 2];

	struct capi_pvt *i;
	struct ast_frame fr = { AST_FRAME_CONTROL, AST_CONTROL_UNHOLD, };
	int suspended = 0;
	int len = MIN(sizeof(call_identity) - 1, 
		FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[4]);

	strncpy(call_identity,
		&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[5], len);
	call_identity[len] = '\0';

	call_id_send[0] = strlen(call_identity);
	strncpy(&call_id_send[1], call_identity,
		sizeof(call_id_send) - 1);

	i = get_leg_by_call_identity(call_identity, &suspended);

	if (!i)
	{
		int reason = suspended ? 
			AST_CAUSE_SUSPENDED_EXISTS_CALL_ID_NOT :
			AST_CAUSE_SUSPENDED_DOESNT_EXIST;

		capi_sendf(NULL, 0, CAPI_FACILITY_RESP, PLCI, HEADER_MSGNUM(CMSG), 
			"w(w(ws))w", FACILITYSELECTOR_SUPPLEMENTARY, 0x0005,
			0x0000,
			call_id_send,
			0x3400 | reason);
		return;
	}

	i->state = CAPI_STATE_CONNECTED;
	i->PLCI = i->onholdPLCI;
	i->onholdPLCI = 0;

	cc_jdsp_start_audio(i->owner, i->chan);
	local_queue_frame(i, &fr);
	if(i->call_suspend_sched_id > -1)
		ast_sched_del(sched, i->call_suspend_sched_id);
	memset(i->call_identity, 0, sizeof(i->call_identity));

	capi_sendf(NULL, 0, CAPI_FACILITY_RESP, PLCI, HEADER_MSGNUM(CMSG), 
		"w(w(ws))w", FACILITYSELECTOR_SUPPLEMENTARY, 0x0005,
		i->chan,
		call_id_send,
		FACILITY_CAUSE_ACCEPT);
}

/*
 * CAPI FACILITY_IND supplementary services 
 */
int handle_facility_indication_supplementary(
	_cmsg *CMSG, unsigned int PLCI, unsigned int NCCI, struct capi_pvt *i)
{
	_cword function;
	_cword infoword = 0xffff;
	unsigned char length;
	_cdword handle;
	_cword mode;
	_cword rbref;
	struct ccbsnr_s *ccbsnrlink;
	char partybusy = 0;
	int ret = 0;
	struct ast_frame fr_unhold = { AST_FRAME_CONTROL, AST_CONTROL_UNHOLD, };
	struct ast_frame fr_hold = { AST_FRAME_CONTROL, AST_CONTROL_HOLD, };

	function = read_capi_word(&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[1]);
	length = FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[3];

	if (length >= 2) {
		infoword = read_capi_word(&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[4]);
	}

	if (!jdsp_is_supplementry_enabled(i->owner))
 	{
		capi_sendf(NULL, 0, CAPI_FACILITY_RESP, PLCI, HEADER_MSGNUM(CMSG), 
			"w(w())w",
			FACILITYSELECTOR_SUPPLEMENTARY,
			function,
			FACILITY_CAUSE_NOT_SUPPORTED_STATE);
		return 1;
 	}

	/* first check functions without interface needed */
	switch (function) {
	case 0x0005: /* Resume */
		{
			cc_verbose(1, 1, VERBOSE_PREFIX_3 "contr%d: PLCI=%#x Resume reason=0x%x\n",
				PLCI & 0xff, PLCI, infoword);
			handle_facility_indication_supp_resume(CMSG, PLCI);
			ret = 1;
			break;
		}
	case 0x000f: /* CCBS request */
		handle = read_capi_dword(&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[6]);
		mode = read_capi_word(&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[10]);
		rbref = read_capi_dword(&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[12]);
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "contr%d: PLCI=%#x CCBS request reason=0x%04x "
			"handle=%d mode=0x%x rbref=0x%x\n",
			PLCI & 0xff, PLCI, infoword, handle, mode, rbref);
		show_capi_info(NULL, infoword);
		if ((ccbsnrlink = get_ccbsnr_link(0, 0, handle, 0xffff, NULL, NULL)) == NULL) {
			cc_log(LOG_WARNING, CC_MESSAGE_NAME " ccbs request indication without request!\n");
			break;
		}
		if (infoword == 0) {
			/* success */
			ccbsnrlink->state = CCBSNR_ACTIVATED;
			ccbsnrlink->rbref = rbref;
			ccbsnrlink->mode = mode;
		} else {
			/* error */
			ccbsnrlink->state = CCBSNR_AVAILABLE;
		}
		break;
	case 0x0010: /* CCBS deactivate */
		handle = read_capi_dword(&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[6]);
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "contr%d: PLCI=%#x CCBS deactivate handle=0x%x reason=0x%x\n",
			PLCI & 0xff, PLCI, handle, infoword);
		show_capi_info(NULL, infoword);
		if ((ccbsnrlink = get_ccbsnr_link(0, 0, handle, 0xffff, NULL, NULL)) == NULL) {
			cc_log(LOG_WARNING, CC_MESSAGE_NAME " ccbs deactivate indication without request!\n");
			break;
		}
		if (infoword == 0) {
			/* success */
			ccbsnrlink->state = CCBSNR_AVAILABLE;
			ccbsnrlink->rbref = 0xdead;
			ccbsnrlink->id = 0xdead;
			ccbsnrlink->mode = 0;
		}
		break;
	case 0x800d: /* CCBS erase call linkage ID */
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "contr%d: PLCI=%#x CCBS/CCNR erase id=0x%04x\n",
			PLCI & 0xff, PLCI, infoword);
		del_ccbsnr_id(PLCI, infoword);
		break;
	case 0x800e: /* CCBS status */
		rbref = read_capi_word(&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[6]);
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "contr%d: PLCI=%#x CCBS status ref=0x%04x mode=0x%x\n",
			PLCI & 0xff, PLCI, rbref, infoword);
		if (get_ccbsnr_link(CCBSNR_TYPE_CCBS, PLCI, 0, rbref, NULL, &partybusy) == NULL) {
			cc_log(LOG_WARNING, CC_MESSAGE_NAME " CCBS status reference not found!\n");
		}
		capi_sendf(NULL, 0, CAPI_FACILITY_RESP, PLCI, HEADER_MSGNUM(CMSG),
			"w(w(w))w",
			FACILITYSELECTOR_SUPPLEMENTARY,
			0x800e,  /* CCBS status */
			(partybusy) ? 0x0000 : 0x0001,
			FACILITY_CAUSE_ACCEPT
		);
		ret = 1;
		break;
	case 0x800f: /* CCBS remote user free */
		rbref = read_capi_word(&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[6]);
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "contr%d: PLCI=%#x CCBS remote user free ref=0x%04x mode=0x%x\n",
			PLCI & 0xff, PLCI, rbref, infoword);
		ccbsnr_remote_user_free(CMSG, CCBSNR_TYPE_CCBS, PLCI, rbref);
		break;
	case 0x8010: /* CCBS B-free */
		rbref = read_capi_word(&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[6]);
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "contr%d: PLCI=%#x CCBS B-free ref=0x%04x mode=0x%x\n",
			PLCI & 0xff, PLCI, rbref, infoword);
		break;
	case 0x8011: /* CCBS erase (ref), deactivated by network */
		rbref = read_capi_word(&FACILITY_IND_FACILITYINDICATIONPARAMETER(CMSG)[6]);
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "contr%d: PLCI=%#x CCBS deactivate ref=0x%04x mode=0x%x\n",
			PLCI & 0xff, PLCI, rbref, infoword);
		del_ccbsnr_ref(PLCI, rbref);
		break;
	case 0x8012: /* CCBS stop alerting */
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "contr%d: PLCI=%#x CCBS B-free ref=0x%04x\n",
			PLCI & 0xff, PLCI, infoword);
		break;
	}

	if (!i) {
		cc_verbose(4, 1, "CAPI: FACILITY_IND SUPPLEMENTARY " 
			"no interface for PLCI=%#x\n", PLCI);
		return ret;
	}

	/* now functions bound to interface */
	switch (function) {
	case 0x0002: /* HOLD */
		if (infoword != 0) {
			/* reason != 0x0000 == problem */
			i->onholdPLCI = 0;
			i->isdnstate &= ~CAPI_ISDN_STATE_HOLD;
			cc_log(LOG_WARNING, "%s: unable to put PLCI=%#x onhold, REASON = 0x%04x, maybe you need to subscribe for this...\n",
				i->vname, PLCI, infoword);
			show_capi_info(i, infoword);
		} else {
			struct ast_frame fr = { AST_FRAME_CONTROL, AST_CONTROL_HOLD, };
			/* reason = 0x0000 == call on hold */
			i->state = CAPI_STATE_ONHOLD;
			i->onholdPLCI = i->PLCI;
			cc_verbose(1, 1, VERBOSE_PREFIX_3 "%s: PLCI=%#x put onhold\n",
				i->vname, PLCI);
			cc_disconnect_b3(i, 0);
			cc_jdsp_stop_audio(i, i->chan);
			if (i->dtmf.dtmf_sched)
			{
				ast_sched_del(sched, i->dtmf.dtmf_sched);
				capi_dtmf_stop(i);
			}
			local_queue_frame(i, &fr);
		}
		break;
	case 0x0003: /* RETRIEVE */
		if (infoword != 0) {
			cc_log(LOG_WARNING, "%s: unable to retrieve PLCI=%#x, REASON = 0x%04x\n",
				i->vname, PLCI, infoword);
			show_capi_info(i, infoword);
		} 
		else if (i->state == CAPI_STATE_ONHOLD) {
			struct ast_frame fr = { AST_FRAME_CONTROL, AST_CONTROL_UNHOLD, };
			i->state = CAPI_STATE_CONNECTED;
			i->PLCI = i->onholdPLCI;
			i->onholdPLCI = 0;
			cc_verbose(1, 1, VERBOSE_PREFIX_3 "%s: PLCI=%#x, chan - %d retrieved\n",
				i->vname, PLCI, i->chan);
			cc_start_b3(i);
			cc_jdsp_start_audio(i->owner, i->chan);
			local_queue_frame(i, &fr);
		}
		break;
	case 0x0004: /* Suspend */
		{
			cc_verbose(1, 1, VERBOSE_PREFIX_3 "%s: PLCI=%#x Suspend Reason=0x%04x, length %d\n",
				i->vname, PLCI, infoword, length);

			handle_facility_indication_supp_suspend(CMSG, PLCI, i);
			ret = 1;
			break;
		}
	case 0x0006:	/* ECT */
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "%s: PLCI=%#x ECT  Reason=0x%04x\n",
			i->vname, PLCI, infoword);
		if (infoword != 0) {
			i->isdnstate &= ~CAPI_ISDN_STATE_ECT;
		}
		else
		{		
		    /* send response to ECT request */
		    capi_sendf(NULL, 0, CAPI_FACILITY_RESP, PLCI, HEADER_MSGNUM(CMSG), 
				"w(w())w", 
				FACILITYSELECTOR_SUPPLEMENTARY,
				function,
				FACILITY_CAUSE_ACCEPT);
		    ret = 1;
		    i->isdnstate |= CAPI_ISDN_STATE_ECT;
		    start_transfer_call(i);
		}
		show_capi_info(i, infoword);
		break;
	case 0x0007: /* 3PTY begin */
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "%s: PLCI=%#x 3PTY begin Reason=0x%04x\n",
			i->vname, PLCI, infoword);
		show_capi_info(i, infoword);
		i->state = CAPI_STATE_CONNECTED;

		i->PLCI = i->onholdPLCI;
		i->onholdPLCI = 0;
		i->held_in_conference = 1;
		
		/* exclusively alloc a line, with a conference channel */
		i->jdsp_line->inthreeway = 1;
		i->chan = OTHER_DC_NUM(i->chan);
		capi_dc_exclusive_alloc(i, i->chan, 0);
		cc_jdsp_start_audio(i->owner, i->chan);

		local_queue_frame(i, &fr_unhold);
		break;
	case 0x0008: /* 3PTY end */
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "%s: PLCI=%#x 3PTY end Reason=0x%04x\n",
			i->vname, PLCI, infoword);

		/* Find held leg */
		if (!i->held_in_conference && !(i = get_second_leg(i)))
			break;

		show_capi_info(i, infoword);
		i->state = CAPI_STATE_ONHOLD;
		i->held_in_conference = 0;
		i->onholdPLCI = i->PLCI;

		cc_jdsp_stop_audio(i, i->chan);
		capi_dc_free(i, i->chan);

		i->chan = OTHER_DC_NUM(i->chan);
		local_queue_frame(i, &fr_hold);
		break;
	case 0x8013: /* CCBS info retain */
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "%s: PLCI=%#x CCBS unique id=0x%04x\n",
			i->vname, PLCI, infoword);
		new_ccbsnr_id(CCBSNR_TYPE_CCBS, PLCI, infoword, i);
		break;
	case 0x8015: /* CCNR info retain */
		cc_verbose(1, 1, VERBOSE_PREFIX_3 "%s: PLCI=%#x CCNR unique id=0x%04x\n",
			i->vname, PLCI, infoword);
		new_ccbsnr_id(CCBSNR_TYPE_CCNR, PLCI, infoword, i);
		break;
	case 0x000e: /* CCBS status */
	case 0x000f: /* CCBS request */
	case 0x800f: /* CCBS remote user free */
	case 0x800d: /* CCBS erase call linkage ID */
	case 0x8010: /* CCBS B-free */
	case 0x8011: /* CCBS erase (ref), deactivated by network */
	case 0x8012: /* CCBS stop alerting */
		/* handled above */
		break;
	default:
		cc_verbose(3, 1, VERBOSE_PREFIX_3 "%s: unhandled FACILITY_IND supplementary function %04x\n",
			i->vname, function);
	}

	if (!ret)
	{
		capi_sendf(NULL, 0, CAPI_FACILITY_RESP, PLCI, HEADER_MSGNUM(CMSG), 
			"w(w())w",
			FACILITYSELECTOR_SUPPLEMENTARY,
			function,
			FACILITY_CAUSE_ACCEPT);
		ret = 1;
	}

	return ret;
}


/*
 * CAPI FACILITY_CONF supplementary
 */
void handle_facility_confirmation_supplementary(
	_cmsg *CMSG, unsigned int PLCI, unsigned int NCCI, struct capi_pvt **i)
{
	_cword function;
	_cword serviceinfo;
	char name[64];

	if (*i) {
		strncpy(name, (*i)->vname, sizeof(name) - 1);
	} else {
		snprintf(name, sizeof(name) - 1, "contr%d", PLCI & 0xff);
	}

	function = read_capi_word(&FACILITY_CONF_FACILITYCONFIRMATIONPARAMETER(CMSG)[1]);
	serviceinfo = FACILITY_CONF_INFO(CMSG);

	switch(function) {
	case 0x0002: /* HOLD */
		if (serviceinfo == 0) {
			cc_verbose(2, 0, VERBOSE_PREFIX_3 "%s: Call on hold (PLCI=%#x)\n",
				name, PLCI);
		}
		break;
	case 0x0003: /* RETRIEVE */
		if (serviceinfo == 0) {
			cc_verbose(2, 0, VERBOSE_PREFIX_3 "%s: Call retreived (PLCI=%#x)\n",
				name, PLCI);
		}
		break;
	case 0x0006: /* ECT */
		if (serviceinfo == 0) {
			cc_verbose(2, 0, VERBOSE_PREFIX_3 "%s: ECT confirmed (PLCI=%#x)\n",
				name, PLCI);
		}
		break;
	case 0x000f: /* CCBS request */
		cc_verbose(2, 1, VERBOSE_PREFIX_3 "%s: CCBS request confirmation (0x%04x) (PLCI=%#x)\n",
			name, serviceinfo, PLCI);
		break;
	case 0x0012: /* CCBS call */
		cc_verbose(2, 1, VERBOSE_PREFIX_3 "%s: CCBS call confirmation (0x%04x) (PLCI=%#x)\n",
			name, serviceinfo, PLCI);
		capidev_handle_connection_conf(i, PLCI, FACILITY_CONF_INFO(CMSG), HEADER_MSGNUM(CMSG));
		break;
	default:
		cc_verbose(3, 1, VERBOSE_PREFIX_3 "%s: unhandled FACILITY_CONF supplementary function %04x\n",
			name, function);
	}
}

/*
 * capicommand 'ccpartybusy'
 */
int pbx_capi_ccpartybusy(struct ast_channel *c, char *data)
{
	char *slinkageid, *yesno;
	unsigned int linkid = 0;
	struct ccbsnr_s *ccbsnr;
	char partybusy = 0;

	slinkageid = strsep(&data, "|");
	yesno = data;
	
	if (slinkageid) {
		linkid = (unsigned int)strtoul(slinkageid, NULL, 0);
	}

	if ((yesno) && ast_true(yesno)) {
		partybusy = 1;
	}

	cc_mutex_lock(&ccbsnr_lock);
	ccbsnr = ccbsnr_list;
	while (ccbsnr) {
		if (((ccbsnr->plci & 0xff) == ((linkid >> 16) & 0xff)) &&
		   (ccbsnr->id == (linkid & 0xffff))) {
			ccbsnr->partybusy = partybusy;
			cc_verbose(1, 1, VERBOSE_PREFIX_3 CC_MESSAGE_NAME
				": CCBS/NR id=0x%x busy set to %d\n", linkid, partybusy);
			break;
		}
		ccbsnr = ccbsnr->next;
	}
	cc_mutex_unlock(&ccbsnr_lock);

	return 0;
}

/*
 * capicommand 'ccbsstop'
 */
int pbx_capi_ccbsstop(struct ast_channel *c, char *data)
{
	char *slinkageid;
	unsigned int linkid = 0;
	unsigned int handle = 0;
	MESSAGE_EXCHANGE_ERROR error;
	_cword ref = 0xdead;
	struct ccbsnr_s *ccbsnr;

	slinkageid = data;

	if (slinkageid) {
		linkid = (unsigned int)strtoul(slinkageid, NULL, 0);
	}

	cc_verbose(3, 1, VERBOSE_PREFIX_3 CC_MESSAGE_NAME " ccbsstop: '%d'\n",
		linkid);

	cc_mutex_lock(&ccbsnr_lock);
	ccbsnr = ccbsnr_list;
	while (ccbsnr) {
		if (((ccbsnr->plci & 0xff) == ((linkid >> 16) & 0xff)) &&
		   (ccbsnr->id == (linkid & 0xffff)) &&
		   (ccbsnr->type == CCBSNR_TYPE_CCBS) &&
		   (ccbsnr->state == CCBSNR_ACTIVATED)) {
			ref = ccbsnr->rbref;
			handle = ccbsnr->handle;
			break;
		}
		ccbsnr = ccbsnr->next;
	}
	cc_mutex_unlock(&ccbsnr_lock);
	
	if (ref != 0xdead) {
	 	error = capi_sendf(NULL, 0, CAPI_FACILITY_REQ, (linkid >> 16) & 0xff,
			get_capi_MessageNumber(),
			"w(w(dw))",
			FACILITYSELECTOR_SUPPLEMENTARY,
			0x0010,  /* CCBS deactivate */
			handle, /* handle */
			ref /* CCBS reference */
		);
	} else {
		cc_verbose(3, 1, VERBOSE_PREFIX_3, CC_MESSAGE_NAME
			" ccbsstop: linkid %d not found in table.\n", linkid);
	}

	return 0;
}

/*
 * capicommand 'ccbs'
 */
int pbx_capi_ccbs(struct ast_channel *c, char *data)
{
	char *slinkageid, *context, *exten, *priority;
	unsigned int linkid = 0;
	unsigned int handle, a;
	char *result = "ERROR";
	char *goodresult = "ACTIVATED";
	MESSAGE_EXCHANGE_ERROR error;
	unsigned int ccbsnrstate;

	slinkageid = strsep(&data, "|");
	context = strsep(&data, "|");
	exten = strsep(&data, "|");
	priority = data;

	if (slinkageid) {
		linkid = (unsigned int)strtoul(slinkageid, NULL, 0);
	}

	if ((!context) || (!exten) || (!priority)) {
		cc_log(LOG_WARNING, CC_MESSAGE_NAME
			" ccbs requires <context>|<exten>|<priority>\n");
		return -1;
	}

	cc_verbose(3, 1, VERBOSE_PREFIX_3 CC_MESSAGE_NAME
		" ccbs: '%d' '%s' '%s' '%s'\n",
		linkid, context, exten, priority);

	handle = select_ccbsnr_id(linkid, CCBSNR_TYPE_CCBS,
		context, exten, (int)strtol(priority, NULL, 0));

	if (handle > 0) {
	 	error = capi_sendf(NULL, 0, CAPI_FACILITY_REQ, (linkid >> 16) & 0xff,
			get_capi_MessageNumber(),
			"w(w(dw))",
			FACILITYSELECTOR_SUPPLEMENTARY,
			0x000f,  /* CCBS request */
			handle, /* handle */
			(linkid & 0xffff) /* CCBS linkage ID */
		);

		for (a = 0; a < 7; a++) {
		/* Wait for CCBS request indication */
			if (ast_safe_sleep_conditional(c, 500, ccbsnr_tell_activated,
			   (void *)(unsigned long)handle) != 0) {
				/* we got a hangup */
				cc_verbose(3, 1,
					VERBOSE_PREFIX_3 CC_MESSAGE_NAME " ccbs: hangup.\n");
				break;
			}
		}
		if (get_ccbsnr_link(0, 0, handle, 0xffff, &ccbsnrstate, NULL) != NULL) {
			if (ccbsnrstate == CCBSNR_ACTIVATED) {
				result = goodresult;
			}
		}
	} else {
		cc_verbose(3, 1, VERBOSE_PREFIX_3, CC_MESSAGE_NAME
			" ccbs: linkid %d not found in table.\n", linkid);
	}

	pbx_builtin_setvar_helper(c, "CCBSSTATUS", result);

	return 0;
}

