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
 * \brief Implementation of Agents (proxy channel)
 *
 * This file is the implementation of Agents modules.
 * It is a dynamic module that is loaded by Asterisk. 
 * \par See also
 * \arg \ref Config_agent
 *
 * \ingroup channel_drivers
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 1.7 $")

#include "asterisk/lock.h"
#include "asterisk/channel.h"
#include "asterisk/config.h"
#include "asterisk/logger.h"
#include "asterisk/module.h"
#include "asterisk/pbx.h"
#include "asterisk/options.h"
#include "asterisk/lock.h"
#include "asterisk/sched.h"
#include "asterisk/io.h"
#include "asterisk/rtp.h"
#include "asterisk/acl.h"
#include "asterisk/callerid.h"
#include "asterisk/file.h"
#include "asterisk/cli.h"
#include "asterisk/app.h"
#include "asterisk/musiconhold.h"
#include "asterisk/manager.h"
#include "asterisk/features.h"
#include "asterisk/utils.h"
#include "asterisk/causes.h"
#include "asterisk/astdb.h"
#include "asterisk/devicestate.h"
#include "asterisk/monitor.h"

static const char desc[] = "Agent Proxy Channel";
static const char channeltype[] = "Agent";
static const char tdesc[] = "Call Agent Proxy Channel";
static const char config[] = "agents.conf";

static const char app[] = "AgentLogin";
static const char app2[] = "AgentCallbackLogin";
static const char app3[] = "AgentMonitorOutgoing";

static const char synopsis[] = "Call agent login";
static const char synopsis2[] = "Call agent callback login";
static const char synopsis3[] = "Record agent's outgoing call";

static const char descrip[] =
"  AgentLogin([AgentNo][|options]):\n"
"Asks the agent to login to the system.  Always returns -1.  While\n"
"logged in, the agent can receive calls and will hear a 'beep'\n"
"when a new call comes in. The agent can dump the call by pressing\n"
"the star key.\n"
"The option string may contain zero or more of the following characters:\n"
"      's' -- silent login - do not announce the login ok segment after agent logged in/off\n";

static const char descrip2[] =
"  AgentCallbackLogin([AgentNo][|[options][|[exten]@context]]):\n"
"Asks the agent to login to the system with callback.\n"
"The agent's callback extension is called (optionally with the specified\n"
"context).\n"
"The option string may contain zero or more of the following characters:\n"
"      's' -- silent login - do not announce the login ok segment agent logged in/off\n";

static const char descrip3[] =
"  AgentMonitorOutgoing([options]):\n"
"Tries to figure out the id of the agent who is placing outgoing call based on\n"
"comparison of the callerid of the current interface and the global variable \n"
"placed by the AgentCallbackLogin application. That's why it should be used only\n"
"with the AgentCallbackLogin app. Uses the monitoring functions in chan_agent \n"
"instead of Monitor application. That have to be configured in the agents.conf file.\n"
"\nReturn value:\n"
"Normally the app returns 0 unless the options are passed. Also if the callerid or\n"
"the agentid are not specified it'll look for n+101 priority.\n"
"\nOptions:\n"
"	'd' - make the app return -1 if there is an error condition and there is\n"
"	      no extension n+101\n"
"	'c' - change the CDR so that the source of the call is 'Agent/agent_id'\n"
"	'n' - don't generate the warnings when there is no callerid or the\n"
"	      agentid is not known.\n"
"             It's handy if you want to have one context for agent and non-agent calls.\n";

static const char mandescr_agents[] =
"Description: Will list info about all possible agents.\n"
"Variables: NONE\n";

static const char mandescr_agent_logoff[] =
"Description: Sets an agent as no longer logged in.\n"
"Variables: (Names marked with * are required)\n"
"	*Agent: Agent ID of the agent to log off\n"
"	Soft: Set to 'true' to not hangup existing calls\n";

static const char mandescr_agent_callback_login[] =
"Description: Sets an agent as logged in with callback.\n"
"Variables: (Names marked with * are required)\n"
"	*Agent: Agent ID of the agent to login\n"
"	*Exten: Extension to use for callback\n"
"	Context: Context to use for callback\n"
"	AckCall: Set to 'true' to require an acknowledgement by '#' when agent is called back\n"
"	WrapupTime: the minimum amount of time after disconnecting before the caller can receive a new call\n";

static char moh[80] = "default";

#define AST_MAX_AGENT	80		/**< Agent ID or Password max length */
#define AST_MAX_BUF	256
#define AST_MAX_FILENAME_LEN	256

/** Persistent Agents astdb family */
static const char pa_family[] = "/Agents";
/** The maximum length of each persistent member agent database entry */
#define PA_MAX_LEN 2048
/** queues.conf [general] option */
static int persistent_agents = 0;
static void dump_agents(void);

static ast_group_t group;
static int autologoff;
static int wrapuptime;
static int ackcall;

static int maxlogintries = 3;
static char agentgoodbye[AST_MAX_FILENAME_LEN] = "vm-goodbye";

static int usecnt =0;
AST_MUTEX_DEFINE_STATIC(usecnt_lock);

/* Protect the interface list (of pvt's) */
AST_MUTEX_DEFINE_STATIC(agentlock);

static int recordagentcalls = 0;
static char recordformat[AST_MAX_BUF] = "";
static char recordformatext[AST_MAX_BUF] = "";
static int createlink = 0;
static char urlprefix[AST_MAX_BUF] = "";
static char savecallsin[AST_MAX_BUF] = "";
static int updatecdr = 0;
static char beep[AST_MAX_BUF] = "beep";

#define GETAGENTBYCALLERID	"AGENTBYCALLERID"

/**
 * Structure representing an agent.
 */
struct agent_pvt {
	ast_mutex_t lock;              /**< Channel private lock */
	int dead;                      /**< Poised for destruction? */
	int pending;                   /**< Not a real agent -- just pending a match */
	int abouttograb;               /**< About to grab */
	int autologoff;                /**< Auto timeout time */
	int ackcall;                   /**< ackcall */
	time_t loginstart;             /**< When agent first logged in (0 when logged off) */
	time_t start;                  /**< When call started */
	struct timeval lastdisc;       /**< When last disconnected */
	int wrapuptime;                /**< Wrapup time in ms */
	ast_group_t group;             /**< Group memberships */
	int acknowledged;              /**< Acknowledged */
	char moh[80];                  /**< Which music on hold */
	char agent[AST_MAX_AGENT];     /**< Agent ID */
	char password[AST_MAX_AGENT];  /**< Password for Agent login */
	char name[AST_MAX_AGENT];
	ast_mutex_t app_lock;          /**< Synchronization between owning applications */
	volatile pthread_t owning_app; /**< Owning application thread id */
	volatile int app_sleep_cond;   /**< Sleep condition for the login app */
	struct ast_channel *owner;     /**< Agent */
	char loginchan[80];            /**< channel they logged in from */
	char logincallerid[80];        /**< Caller ID they had when they logged in */
	struct ast_channel *chan;      /**< Channel we use */
	struct agent_pvt *next;        /**< Next Agent in the linked list. */
};

static struct agent_pvt *agents = NULL;  /**< Holds the list of agents (loaded form agents.conf). */

#define CHECK_FORMATS(ast, p) do { \
	if (p->chan) {\
		if (ast->nativeformats != p->chan->nativeformats) { \
			ast_log(LOG_DEBUG, "Native formats changing from %d to %d\n", ast->nativeformats, p->chan->nativeformats); \
			/* Native formats changed, reset things */ \
			ast->nativeformats = p->chan->nativeformats; \
			ast_log(LOG_DEBUG, "Resetting read to %d and write to %d\n", ast->readformat, ast->writeformat);\
			ast_set_read_format(ast, ast->readformat); \
			ast_set_write_format(ast, ast->writeformat); \
		} \
		if (p->chan->readformat != ast->rawreadformat)  \
			ast_set_read_format(p->chan, ast->rawreadformat); \
		if (p->chan->writeformat != ast->rawwriteformat) \
			ast_set_write_format(p->chan, ast->rawwriteformat); \
	} \
} while(0)

/* Cleanup moves all the relevant FD's from the 2nd to the first, but retains things
   properly for a timingfd XXX This might need more work if agents were logged in as agents or other
   totally impractical combinations XXX */

#define CLEANUP(ast, p) do { \
	int x; \
	if (p->chan) { \
		for (x=0;x<AST_MAX_FDS;x++) {\
			if (x != AST_MAX_FDS - 2) \
				ast->fds[x] = p->chan->fds[x]; \
		} \
		ast->fds[AST_MAX_FDS - 3] = p->chan->fds[AST_MAX_FDS - 2]; \
	} \
} while(0)

static struct ast_channel *agent_request(const char *type, int format, void *data, int *cause);
static int agent_devicestate(void *data);
static int agent_digit(struct ast_channel *ast, char digit);
static int agent_call(struct ast_channel *ast, char *dest, int timeout);
static int agent_hangup(struct ast_channel *ast);
static int agent_answer(struct ast_channel *ast);
static struct ast_frame *agent_read(struct ast_channel *ast);
static int agent_write(struct ast_channel *ast, struct ast_frame *f);
static int agent_sendhtml(struct ast_channel *ast, int subclass, const char *data, int datalen);
static int agent_sendtext(struct ast_channel *ast, const char *text);
static int agent_indicate(struct ast_channel *ast, int condition);
static int agent_fixup(struct ast_channel *oldchan, struct ast_channel *newchan);
static struct ast_channel *agent_bridgedchannel(struct ast_channel *chan, struct ast_channel *bridge);

static const struct ast_channel_tech agent_tech = {
	.type = channeltype,
	.description = tdesc,
	.capabilities = -1,
	.requester = agent_request,
	.devicestate = agent_devicestate,
	.send_digit = agent_digit,
	.call = agent_call,
	.hangup = agent_hangup,
	.answer = agent_answer,
	.read = agent_read,
	.write = agent_write,
	.send_html = agent_sendhtml,
	.send_text = agent_sendtext,
	.exception = agent_read,
	.indicate = agent_indicate,
	.fixup = agent_fixup,
	.bridged_channel = agent_bridgedchannel,
};

/**
 * Unlink (that is, take outside of the linked list) an agent.
 *
 * @param agent Agent to be unlinked.
 */
static void agent_unlink(struct agent_pvt *agent)
{
	struct agent_pvt *p, *prev;
	prev = NULL;
	p = agents;
	// Iterate over all agents looking for the one.
	while(p) {
		if (p == agent) {
			// Once it wal found, check if it is the first one.
			if (prev)
				// If it is not, tell the previous agent that the next one is the next one of the current (jumping the current).
				prev->next = agent->next;
			else
				// If it is the first one, just change the general pointer to point to the second one.
				agents = agent->next;
			// We are done.
			break;
		}
		prev = p;
		p = p->next;
	}
}

/**
 * Adds an agent to the global list of agents.
 *
 * @param agent A string with the username, password and real name of an agent. As defined in agents.conf. Example: "13,169,John Smith"
 * @param pending If it is pending or not.
 * @return The just created agent.
 * @sa agent_pvt, agents.
 */
static struct agent_pvt *add_agent(char *agent, int pending)
{
	int argc;
	char *argv[3];
	char *args;
	char *password = NULL;
	char *name = NULL;
	char *agt = NULL;
	struct agent_pvt *p, *prev;

	args = ast_strdupa(agent);

	// Extract username (agt), password and name from agent (args).
	if ((argc = ast_app_separate_args(args, ',', argv, sizeof(argv) / sizeof(argv[0])))) {
		agt = argv[0];
		if (argc > 1) {
			password = argv[1];
			while (*password && *password < 33) password++;
		} 
		if (argc > 2) {
			name = argv[2];
			while (*name && *name < 33) name++;
		}
	} else {
		ast_log(LOG_WARNING, "A blank agent line!\n");
	}
	
	// Are we searching for the agent here ? to see if it exists already ?
	prev=NULL;
	p = agents;
	while(p) {
		if (!pending && !strcmp(p->agent, agt))
			break;
		prev = p;
		p = p->next;
	}
	if (!p) {
		// Build the agent.
		p = malloc(sizeof(struct agent_pvt));
		if (p) {
			memset(p, 0, sizeof(struct agent_pvt));
			ast_copy_string(p->agent, agt, sizeof(p->agent));
			ast_mutex_init(&p->lock);
			ast_mutex_init(&p->app_lock);
			p->owning_app = (pthread_t) -1;
			p->app_sleep_cond = 1;
			p->group = group;
			p->pending = pending;
			p->next = NULL;
			if (prev)
				prev->next = p;
			else
				agents = p;
			
		} else {
			return NULL;
		}
	}
	
	ast_copy_string(p->password, password ? password : "", sizeof(p->password));
	ast_copy_string(p->name, name ? name : "", sizeof(p->name));
	ast_copy_string(p->moh, moh, sizeof(p->moh));
	p->ackcall = ackcall;
	p->autologoff = autologoff;

	/* If someone reduces the wrapuptime and reloads, we want it
	 * to change the wrapuptime immediately on all calls */
	if (p->wrapuptime > wrapuptime) {
		struct timeval now = ast_tvfromboot();
		/* XXX check what is this exactly */

		/* We won't be pedantic and check the tv_usec val */
		if (p->lastdisc.tv_sec > (now.tv_sec + wrapuptime/1000)) {
			p->lastdisc.tv_sec = now.tv_sec + wrapuptime/1000;
			p->lastdisc.tv_usec = now.tv_usec;
		}
	}
	p->wrapuptime = wrapuptime;

	if (pending)
		p->dead = 1;
	else
		p->dead = 0;
	return p;
}

/**
 * Deletes an agent after doing some clean up.
 * Further documentation: How safe is this function ? What state should the agent be to be cleaned.
 * @param p Agent to be deleted.
 * @returns Always 0.
 */
static int agent_cleanup(struct agent_pvt *p)
{
	struct ast_channel *chan = p->owner;
	p->owner = NULL;
	chan->tech_pvt = NULL;
	p->app_sleep_cond = 1;
	/* Release ownership of the agent to other threads (presumably running the login app). */
	ast_mutex_unlock(&p->app_lock);
	if (chan)
		ast_channel_free(chan);
	if (p->dead) {
		ast_mutex_destroy(&p->lock);
		ast_mutex_destroy(&p->app_lock);
		free(p);
        }
	return 0;
}

static int check_availability(struct agent_pvt *newlyavailable, int needlock);

static int agent_answer(struct ast_channel *ast)
{
	ast_log(LOG_WARNING, "Huh?  Agent is being asked to answer?\n");
	return -1;
}

static int __agent_start_monitoring(struct ast_channel *ast, struct agent_pvt *p, int needlock)
{
	char tmp[AST_MAX_BUF],tmp2[AST_MAX_BUF], *pointer;
	char filename[AST_MAX_BUF];
	int res = -1;
	if (!p)
		return -1;
	if (!ast->monitor) {
		snprintf(filename, sizeof(filename), "agent-%s-%s",p->agent, ast->uniqueid);
		/* substitute . for - */
		if ((pointer = strchr(filename, '.')))
			*pointer = '-';
		snprintf(tmp, sizeof(tmp), "%s%s",savecallsin ? savecallsin : "", filename);
		ast_monitor_start(ast, recordformat, tmp, needlock);
		ast_monitor_setjoinfiles(ast, 1);
		snprintf(tmp2, sizeof(tmp2), "%s%s.%s", urlprefix ? urlprefix : "", filename, recordformatext);
#if 0
		ast_verbose("name is %s, link is %s\n",tmp, tmp2);
#endif
		if (!ast->cdr)
			ast->cdr = ast_cdr_alloc();
		ast_cdr_setuserfield(ast, tmp2);
		res = 0;
	} else
		ast_log(LOG_ERROR, "Recording already started on that call.\n");
	return res;
}

static int agent_start_monitoring(struct ast_channel *ast, int needlock)
{
	return __agent_start_monitoring(ast, ast->tech_pvt, needlock);
}

static struct ast_frame *agent_read(struct ast_channel *ast)
{
	struct agent_pvt *p = ast->tech_pvt;
	struct ast_frame *f = NULL;
	static struct ast_frame null_frame = { AST_FRAME_NULL, };
	static struct ast_frame answer_frame = { AST_FRAME_CONTROL, AST_CONTROL_ANSWER };
	ast_mutex_lock(&p->lock); 
	CHECK_FORMATS(ast, p);
	if (p->chan) {
		ast_copy_flags(p->chan, ast, AST_FLAG_EXCEPTION);
		if (ast->fdno == AST_MAX_FDS - 3)
			p->chan->fdno = AST_MAX_FDS - 2;
		else
			p->chan->fdno = ast->fdno;
		f = ast_read(p->chan);
	} else
		f = &null_frame;
	if (!f) {
		/* If there's a channel, hang it up (if it's on a callback) make it NULL */
		if (p->chan) {
			p->chan->_bridge = NULL;
			/* Note that we don't hangup if it's not a callback because Asterisk will do it
			   for us when the PBX instance that called login finishes */
			if (!ast_strlen_zero(p->loginchan)) {
				if (p->chan)
					ast_log(LOG_DEBUG, "Bridge on '%s' being cleared (2)\n", p->chan->name);
				ast_hangup(p->chan);
				if (p->wrapuptime && p->acknowledged)
					p->lastdisc = ast_tvadd(ast_tvfromboot(), ast_samp2tv(p->wrapuptime, 1000));
			}
			p->chan = NULL;
			p->acknowledged = 0;
		}
 	} else {
 		/* if acknowledgement is not required, and the channel is up, we may have missed
 		   an AST_CONTROL_ANSWER (if there was one), so mark the call acknowledged anyway */
 		if (!p->ackcall && !p->acknowledged && p->chan && (p->chan->_state == AST_STATE_UP))
  			p->acknowledged = 1;
 		switch (f->frametype) {
 		case AST_FRAME_CONTROL:
 			if (f->subclass == AST_CONTROL_ANSWER) {
 				if (p->ackcall) {
 					if (option_verbose > 2)
 						ast_verbose(VERBOSE_PREFIX_3 "%s answered, waiting for '#' to acknowledge\n", p->chan->name);
 					/* Don't pass answer along */
 					ast_frfree(f);
 					f = &null_frame;
 				} else {
 					p->acknowledged = 1;
 					/* Use the builtin answer frame for the 
					   recording start check below. */
 					ast_frfree(f);
 					f = &answer_frame;
 				}
 			}
 			break;
 		case AST_FRAME_DTMF:
 			if (!p->acknowledged && (f->subclass == '#')) {
 				if (option_verbose > 2)
 					ast_verbose(VERBOSE_PREFIX_3 "%s acknowledged\n", p->chan->name);
 				p->acknowledged = 1;
 				ast_frfree(f);
 				f = &answer_frame;
 			} else if (f->subclass == '*') {
 				/* terminates call */
 				ast_frfree(f);
 				f = NULL;
 			}
 			break;
 		case AST_FRAME_VOICE:
 			/* don't pass voice until the call is acknowledged */
 			if (!p->acknowledged) {
 				ast_frfree(f);
 				f = &null_frame;
 			}
 			break;
  		}
  	}

	CLEANUP(ast,p);
	if (p->chan && !p->chan->_bridge) {
		if (strcasecmp(p->chan->type, "Local")) {
			p->chan->_bridge = ast;
			if (p->chan)
				ast_log(LOG_DEBUG, "Bridge on '%s' being set to '%s' (3)\n", p->chan->name, p->chan->_bridge->name);
		}
	}
	ast_mutex_unlock(&p->lock);
	if (recordagentcalls && f == &answer_frame)
		agent_start_monitoring(ast,0);
	return f;
}

static int agent_sendhtml(struct ast_channel *ast, int subclass, const char *data, int datalen)
{
	struct agent_pvt *p = ast->tech_pvt;
	int res = -1;
	ast_mutex_lock(&p->lock);
	if (p->chan) 
		res = ast_channel_sendhtml(p->chan, subclass, data, datalen);
	ast_mutex_unlock(&p->lock);
	return res;
}

static int agent_sendtext(struct ast_channel *ast, const char *text)
{
	struct agent_pvt *p = ast->tech_pvt;
	int res = -1;
	ast_mutex_lock(&p->lock);
	if (p->chan) 
		res = ast_sendtext(p->chan, text);
	ast_mutex_unlock(&p->lock);
	return res;
}

static int agent_write(struct ast_channel *ast, struct ast_frame *f)
{
	struct agent_pvt *p = ast->tech_pvt;
	int res = -1;
	CHECK_FORMATS(ast, p);
	ast_mutex_lock(&p->lock);
	if (p->chan) {
		if ((f->frametype != AST_FRAME_VOICE) ||
		    (f->subclass == p->chan->writeformat)) {
			res = ast_write(p->chan, f);
		} else {
			ast_log(LOG_DEBUG, "Dropping one incompatible voice frame on '%s' to '%s'\n", ast->name, p->chan->name);
			res = 0;
		}
	} else
		res = 0;
	CLEANUP(ast, p);
	ast_mutex_unlock(&p->lock);
	return res;
}

static int agent_fixup(struct ast_channel *oldchan, struct ast_channel *newchan)
{
	struct agent_pvt *p = newchan->tech_pvt;
	ast_mutex_lock(&p->lock);
	if (p->owner != oldchan) {
		ast_log(LOG_WARNING, "old channel wasn't %p but was %p\n", oldchan, p->owner);
		ast_mutex_unlock(&p->lock);
		return -1;
	}
	p->owner = newchan;
	ast_mutex_unlock(&p->lock);
	return 0;
}

static int agent_indicate(struct ast_channel *ast, int condition)
{
	struct agent_pvt *p = ast->tech_pvt;
	int res = -1;
	ast_mutex_lock(&p->lock);
	if (p->chan)
		res = ast_indicate(p->chan, condition);
	else
		res = 0;
	ast_mutex_unlock(&p->lock);
	return res;
}

static int agent_digit(struct ast_channel *ast, char digit)
{
	struct agent_pvt *p = ast->tech_pvt;
	int res = -1;
	ast_mutex_lock(&p->lock);
	if (p->chan)
		res = p->chan->tech->send_digit(p->chan, digit);
	else
		res = 0;
	ast_mutex_unlock(&p->lock);
	return res;
}

static int agent_call(struct ast_channel *ast, char *dest, int timeout)
{
	struct agent_pvt *p = ast->tech_pvt;
	int res = -1;
	int newstate=0;
	ast_mutex_lock(&p->lock);
	p->acknowledged = 0;
	if (!p->chan) {
		if (p->pending) {
			ast_log(LOG_DEBUG, "Pretending to dial on pending agent\n");
			newstate = AST_STATE_DIALING;
			res = 0;
		} else {
			ast_log(LOG_NOTICE, "Whoa, they hung up between alloc and call...  what are the odds of that?\n");
			res = -1;
		}
		ast_mutex_unlock(&p->lock);
		if (newstate)
			ast_setstate(ast, newstate);
		return res;
	} else if (!ast_strlen_zero(p->loginchan)) {
		time(&p->start);
		/* Call on this agent */
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "outgoing agentcall, to agent '%s', on '%s'\n", p->agent, p->chan->name);
		if (p->chan->cid.cid_num)
			free(p->chan->cid.cid_num);
		if (ast->cid.cid_num)
			p->chan->cid.cid_num = strdup(ast->cid.cid_num);
		else
			p->chan->cid.cid_num = NULL;
		if (p->chan->cid.cid_name)
			free(p->chan->cid.cid_name);
		if (ast->cid.cid_name)
			p->chan->cid.cid_name = strdup(ast->cid.cid_name);
		else
			p->chan->cid.cid_name = NULL;
		ast_channel_inherit_variables(ast, p->chan);
		res = ast_call(p->chan, p->loginchan, 0);
		CLEANUP(ast,p);
		ast_mutex_unlock(&p->lock);
		return res;
	}
	ast_verbose( VERBOSE_PREFIX_3 "agent_call, call to agent '%s' call on '%s'\n", p->agent, p->chan->name);
	ast_log( LOG_DEBUG, "Playing beep, lang '%s'\n", p->chan->language);
	res = ast_streamfile(p->chan, beep, p->chan->language);
	ast_log( LOG_DEBUG, "Played beep, result '%d'\n", res);
	if (!res) {
		res = ast_waitstream(p->chan, "");
		ast_log( LOG_DEBUG, "Waited for stream, result '%d'\n", res);
	}
	if (!res) {
		res = ast_set_read_format(p->chan, ast_best_codec(p->chan->nativeformats));
		ast_log( LOG_DEBUG, "Set read format, result '%d'\n", res);
		if (res)
			ast_log(LOG_WARNING, "Unable to set read format to %s\n", ast_getformatname(ast_best_codec(p->chan->nativeformats)));
	} else {
		/* Agent hung-up */
		p->chan = NULL;
	}

	if (!res) {
		ast_set_write_format(p->chan, ast_best_codec(p->chan->nativeformats));
		ast_log( LOG_DEBUG, "Set write format, result '%d'\n", res);
		if (res)
			ast_log(LOG_WARNING, "Unable to set write format to %s\n", ast_getformatname(ast_best_codec(p->chan->nativeformats)));
	}
	if( !res )
	{
		/* Call is immediately up, or might need ack */
		if (p->ackcall > 1)
			newstate = AST_STATE_RINGING;
		else {
			newstate = AST_STATE_UP;
			if (recordagentcalls)
				agent_start_monitoring(ast,0);
			p->acknowledged = 1;
		}
		res = 0;
	}
	CLEANUP(ast,p);
	ast_mutex_unlock(&p->lock);
	if (newstate)
		ast_setstate(ast, newstate);
	return res;
}

/* store/clear the global variable that stores agentid based on the callerid */
static void set_agentbycallerid(const char *callerid, const char *agent)
{
	char buf[AST_MAX_BUF];

	/* if there is no Caller ID, nothing to do */
	if (ast_strlen_zero(callerid))
		return;

	snprintf(buf, sizeof(buf), "%s_%s",GETAGENTBYCALLERID, callerid);
	pbx_builtin_setvar_helper(NULL, buf, agent);
}

static int agent_hangup(struct ast_channel *ast)
{
	struct agent_pvt *p = ast->tech_pvt;
	int howlong = 0;
	ast_mutex_lock(&p->lock);
	p->owner = NULL;
	ast->tech_pvt = NULL;
	p->app_sleep_cond = 1;
	p->acknowledged = 0;

	/* if they really are hung up then set start to 0 so the test
	 * later if we're called on an already downed channel
	 * doesn't cause an agent to be logged out like when
	 * agent_request() is followed immediately by agent_hangup()
	 * as in apps/app_chanisavail.c:chanavail_exec()
	 */

	ast_mutex_lock(&usecnt_lock);
	usecnt--;
	ast_mutex_unlock(&usecnt_lock);

	ast_log(LOG_DEBUG, "Hangup called for state %s\n", ast_state2str(ast->_state, ast_test_flag(ast, AST_FLAG_CALL_ONHOLD)));
	if (p->start && (ast->_state != AST_STATE_UP)) {
		howlong = time(NULL) - p->start;
		p->start = 0;
	} else if (ast->_state == AST_STATE_RESERVED) {
		howlong = 0;
	} else
		p->start = 0; 
	if (p->chan) {
		p->chan->_bridge = NULL;
		/* If they're dead, go ahead and hang up on the agent now */
		if (!ast_strlen_zero(p->loginchan)) {
			/* Store last disconnect time */
			if (p->wrapuptime)
				p->lastdisc = ast_tvadd(ast_tvfromboot(), ast_samp2tv(p->wrapuptime, 1000));
			else
				p->lastdisc = ast_tv(0,0);
			if (p->chan) {
				/* Recognize the hangup and pass it along immediately */
				ast_hangup(p->chan);
				p->chan = NULL;
			}
			ast_log(LOG_DEBUG, "Hungup, howlong is %d, autologoff is %d\n", howlong, p->autologoff);
			if (howlong  && p->autologoff && (howlong > p->autologoff)) {
				char agent[AST_MAX_AGENT] = "";
				long logintime = time(NULL) - p->loginstart;
				p->loginstart = 0;
				ast_log(LOG_NOTICE, "Agent '%s' didn't answer/confirm within %d seconds (waited %d)\n", p->name, p->autologoff, howlong);
				manager_event(EVENT_FLAG_AGENT, "Agentcallbacklogoff",
					      "Agent: %s\r\n"
					      "Loginchan: %s\r\n"
					      "Logintime: %ld\r\n"
					      "Reason: Autologoff\r\n"
					      "Uniqueid: %s\r\n",
					      p->agent, p->loginchan, logintime, ast->uniqueid);
				snprintf(agent, sizeof(agent), "Agent/%s", p->agent);
				ast_queue_log("NONE", ast->uniqueid, agent, "AGENTCALLBACKLOGOFF", "%s|%ld|%s", p->loginchan, logintime, "Autologoff");
				set_agentbycallerid(p->logincallerid, NULL);
				ast_device_state_changed("Agent/%s", p->agent);
				p->loginchan[0] = '\0';
				p->logincallerid[0] = '\0';
				if (persistent_agents)
					dump_agents();
			}
		} else if (p->dead) {
			ast_mutex_lock(&p->chan->lock);
			ast_softhangup(p->chan, AST_SOFTHANGUP_EXPLICIT);
			ast_mutex_unlock(&p->chan->lock);
		} else {
			ast_mutex_lock(&p->chan->lock);
			ast_moh_start(p->chan, p->moh);
			ast_mutex_unlock(&p->chan->lock);
		}
	}
	ast_mutex_unlock(&p->lock);
	ast_device_state_changed("Agent/%s", p->agent);

	if (p->pending) {
		ast_mutex_lock(&agentlock);
		agent_unlink(p);
		ast_mutex_unlock(&agentlock);
	}
	if (p->abouttograb) {
		/* Let the "about to grab" thread know this isn't valid anymore, and let it
		   kill it later */
		p->abouttograb = 0;
	} else if (p->dead) {
		ast_mutex_destroy(&p->lock);
		ast_mutex_destroy(&p->app_lock);
		free(p);
	} else {
		if (p->chan) {
			/* Not dead -- check availability now */
			ast_mutex_lock(&p->lock);
			/* Store last disconnect time */
			p->lastdisc = ast_tvfromboot();
			ast_mutex_unlock(&p->lock);
		}
		/* Release ownership of the agent to other threads (presumably running the login app). */
		ast_mutex_unlock(&p->app_lock);
	}
	return 0;
}

static int agent_cont_sleep( void *data )
{
	struct agent_pvt *p;
	int res;

	p = (struct agent_pvt *)data;

	ast_mutex_lock(&p->lock);
	res = p->app_sleep_cond;
	if (p->lastdisc.tv_sec) {
		if (ast_tvdiff_ms(ast_tvfromboot(), p->lastdisc) > p->wrapuptime) 
			res = 1;
	}
	ast_mutex_unlock(&p->lock);
#if 0
	if( !res )
		ast_log( LOG_DEBUG, "agent_cont_sleep() returning %d\n", res );
#endif		
	return res;
}

static int agent_ack_sleep( void *data )
{
	struct agent_pvt *p;
	int res=0;
	int to = 1000;
	struct ast_frame *f;

	/* Wait a second and look for something */

	p = (struct agent_pvt *)data;
	if (p->chan) {
		for(;;) {
			to = ast_waitfor(p->chan, to);
			if (to < 0) {
				res = -1;
				break;
			}
			if (!to) {
				res = 0;
				break;
			}
			f = ast_read(p->chan);
			if (!f) {
				res = -1;
				break;
			}
			if (f->frametype == AST_FRAME_DTMF)
				res = f->subclass;
			else
				res = 0;
			ast_frfree(f);
			ast_mutex_lock(&p->lock);
			if (!p->app_sleep_cond) {
				ast_mutex_unlock(&p->lock);
				res = 0;
				break;
			} else if (res == '#') {
				ast_mutex_unlock(&p->lock);
				res = 1;
				break;
			}
			ast_mutex_unlock(&p->lock);
			res = 0;
		}
	} else
		res = -1;
	return res;
}

static struct ast_channel *agent_bridgedchannel(struct ast_channel *chan, struct ast_channel *bridge)
{
	struct agent_pvt *p = bridge->tech_pvt;
	struct ast_channel *ret=NULL;

	if (p) {
		if (chan == p->chan)
			ret = bridge->_bridge;
		else if (chan == bridge->_bridge)
			ret = p->chan;
	}

	if (option_debug)
		ast_log(LOG_DEBUG, "Asked for bridged channel on '%s'/'%s', returning '%s'\n", chan->name, bridge->name, ret ? ret->name : "<none>");
	return ret;
}

/*--- agent_new: Create new agent channel ---*/
static struct ast_channel *agent_new(struct agent_pvt *p, int state)
{
	struct ast_channel *tmp;
	struct ast_frame null_frame = { AST_FRAME_NULL };
#if 0
	if (!p->chan) {
		ast_log(LOG_WARNING, "No channel? :(\n");
		return NULL;
	}
#endif	
	tmp = ast_channel_alloc(0);
	if (tmp) {
		tmp->tech = &agent_tech;
		if (p->chan) {
			tmp->nativeformats = p->chan->nativeformats;
			tmp->writeformat = p->chan->writeformat;
			tmp->rawwriteformat = p->chan->writeformat;
			tmp->readformat = p->chan->readformat;
			tmp->rawreadformat = p->chan->readformat;
			ast_copy_string(tmp->language, p->chan->language, sizeof(tmp->language));
			ast_copy_string(tmp->context, p->chan->context, sizeof(tmp->context));
			ast_copy_string(tmp->exten, p->chan->exten, sizeof(tmp->exten));
		} else {
			tmp->nativeformats = AST_FORMAT_SLINEAR;
			tmp->writeformat = AST_FORMAT_SLINEAR;
			tmp->rawwriteformat = AST_FORMAT_SLINEAR;
			tmp->readformat = AST_FORMAT_SLINEAR;
			tmp->rawreadformat = AST_FORMAT_SLINEAR;
		}
		if (p->pending)
			snprintf(tmp->name, sizeof(tmp->name), "Agent/P%s-%d", p->agent, rand() & 0xffff);
		else
			snprintf(tmp->name, sizeof(tmp->name), "Agent/%s", p->agent);
		tmp->type = channeltype;
		/* Safe, agentlock already held */
		ast_setstate(tmp, state);
		tmp->tech_pvt = p;
		p->owner = tmp;
		ast_mutex_lock(&usecnt_lock);
		usecnt++;
		ast_mutex_unlock(&usecnt_lock);
		ast_update_use_count();
		tmp->priority = 1;
		/* Wake up and wait for other applications (by definition the login app)
		 * to release this channel). Takes ownership of the agent channel
		 * to this thread only.
		 * For signalling the other thread, ast_queue_frame is used until we
		 * can safely use signals for this purpose. The pselect() needs to be
		 * implemented in the kernel for this.
		 */
		p->app_sleep_cond = 0;
		if( ast_mutex_trylock(&p->app_lock) )
		{
			if (p->chan) {
				ast_queue_frame(p->chan, &null_frame);
				ast_mutex_unlock(&p->lock);	/* For other thread to read the condition. */
				ast_mutex_lock(&p->app_lock);
				ast_mutex_lock(&p->lock);
			}
			if( !p->chan )
			{
				ast_log(LOG_WARNING, "Agent disconnected while we were connecting the call\n");
				p->owner = NULL;
				tmp->tech_pvt = NULL;
				p->app_sleep_cond = 1;
				ast_channel_free( tmp );
				ast_mutex_unlock(&p->lock);	/* For other thread to read the condition. */
				ast_mutex_unlock(&p->app_lock);
				return NULL;
			}
		}
		p->owning_app = pthread_self();
		/* After the above step, there should not be any blockers. */
		if (p->chan) {
			if (ast_test_flag(p->chan, AST_FLAG_BLOCKING)) {
				ast_log( LOG_ERROR, "A blocker exists after agent channel ownership acquired\n" );
				CRASH;
			}
			ast_moh_stop(p->chan);
		}
	} else
		ast_log(LOG_WARNING, "Unable to allocate agent channel structure\n");
	return tmp;
}


/**
 * Read configuration data. The file named agents.conf.
 *
 * @returns Always 0, or so it seems.
 */
static int read_agent_config(void)
{
	struct ast_config *cfg;
	struct ast_variable *v;
	struct agent_pvt *p, *pl, *pn;
	char *general_val;

	group = 0;
	autologoff = 0;
	wrapuptime = 0;
	ackcall = 0;
	cfg = ast_config_load(config);
	if (!cfg) {
		ast_log(LOG_NOTICE, "No agent configuration found -- agent support disabled\n");
		return 0;
	}
	ast_mutex_lock(&agentlock);
	p = agents;
	while(p) {
		p->dead = 1;
		p = p->next;
	}
	strcpy(moh, "default");
	/* set the default recording values */
	recordagentcalls = 0;
	createlink = 0;
	strcpy(recordformat, "wav");
	strcpy(recordformatext, "wav");
	urlprefix[0] = '\0';
	savecallsin[0] = '\0';

	/* Read in [general] section for persistence */
	if ((general_val = ast_variable_retrieve(cfg, "general", "persistentagents")))
		persistent_agents = ast_true(general_val);

	/* Read in the [agents] section */
	v = ast_variable_browse(cfg, "agents");
	while(v) {
		/* Create the interface list */
		if (!strcasecmp(v->name, "agent")) {
			add_agent(v->value, 0);
		} else if (!strcasecmp(v->name, "group")) {
			group = ast_get_group(v->value);
		} else if (!strcasecmp(v->name, "autologoff")) {
			autologoff = atoi(v->value);
			if (autologoff < 0)
				autologoff = 0;
		} else if (!strcasecmp(v->name, "ackcall")) {
			if (!strcasecmp(v->value, "always"))
				ackcall = 2;
			else if (ast_true(v->value))
				ackcall = 1;
			else
				ackcall = 0;
		} else if (!strcasecmp(v->name, "wrapuptime")) {
			wrapuptime = atoi(v->value);
			if (wrapuptime < 0)
				wrapuptime = 0;
		} else if (!strcasecmp(v->name, "maxlogintries") && !ast_strlen_zero(v->value)) {
			maxlogintries = atoi(v->value);
			if (maxlogintries < 0)
				maxlogintries = 0;
		} else if (!strcasecmp(v->name, "goodbye") && !ast_strlen_zero(v->value)) {
			strcpy(agentgoodbye,v->value);
		} else if (!strcasecmp(v->name, "musiconhold")) {
			ast_copy_string(moh, v->value, sizeof(moh));
		} else if (!strcasecmp(v->name, "updatecdr")) {
			if (ast_true(v->value))
				updatecdr = 1;
			else
				updatecdr = 0;
		} else if (!strcasecmp(v->name, "recordagentcalls")) {
			recordagentcalls = ast_true(v->value);
		} else if (!strcasecmp(v->name, "createlink")) {
			createlink = ast_true(v->value);
		} else if (!strcasecmp(v->name, "recordformat")) {
			ast_copy_string(recordformat, v->value, sizeof(recordformat));
			if (!strcasecmp(v->value, "wav49"))
				strcpy(recordformatext, "WAV");
			else
				ast_copy_string(recordformatext, v->value, sizeof(recordformatext));
		} else if (!strcasecmp(v->name, "urlprefix")) {
			ast_copy_string(urlprefix, v->value, sizeof(urlprefix));
			if (urlprefix[strlen(urlprefix) - 1] != '/')
				strncat(urlprefix, "/", sizeof(urlprefix) - strlen(urlprefix) - 1);
		} else if (!strcasecmp(v->name, "savecallsin")) {
			if (v->value[0] == '/')
				ast_copy_string(savecallsin, v->value, sizeof(savecallsin));
			else
				snprintf(savecallsin, sizeof(savecallsin) - 2, "/%s", v->value);
			if (savecallsin[strlen(savecallsin) - 1] != '/')
				strncat(savecallsin, "/", sizeof(savecallsin) - strlen(savecallsin) - 1);
		} else if (!strcasecmp(v->name, "custom_beep")) {
			ast_copy_string(beep, v->value, sizeof(beep));
		}
		v = v->next;
	}
	p = agents;
	pl = NULL;
	while(p) {
		pn = p->next;
		if (p->dead) {
			/* Unlink */
			if (pl)
				pl->next = p->next;
			else
				agents = p->next;
			/* Destroy if  appropriate */
			if (!p->owner) {
				if (!p->chan) {
					ast_mutex_destroy(&p->lock);
					ast_mutex_destroy(&p->app_lock);
					free(p);
				} else {
					/* Cause them to hang up */
					ast_softhangup(p->chan, AST_SOFTHANGUP_EXPLICIT);
				}
			}
		} else
			pl = p;
		p = pn;
	}
	ast_mutex_unlock(&agentlock);
	ast_config_destroy(cfg);
	return 0;
}

static int check_availability(struct agent_pvt *newlyavailable, int needlock)
{
	struct ast_channel *chan=NULL, *parent=NULL;
	struct agent_pvt *p;
	int res;

	if (option_debug)
		ast_log(LOG_DEBUG, "Checking availability of '%s'\n", newlyavailable->agent);
	if (needlock)
		ast_mutex_lock(&agentlock);
	p = agents;
	while(p) {
		if (p == newlyavailable) {
			p = p->next;
			continue;
		}
		ast_mutex_lock(&p->lock);
		if (!p->abouttograb && p->pending && ((p->group && (newlyavailable->group & p->group)) || !strcmp(p->agent, newlyavailable->agent))) {
			if (option_debug)
				ast_log(LOG_DEBUG, "Call '%s' looks like a winner for agent '%s'\n", p->owner->name, newlyavailable->agent);
			/* We found a pending call, time to merge */
			chan = agent_new(newlyavailable, AST_STATE_DOWN);
			parent = p->owner;
			p->abouttograb = 1;
			ast_mutex_unlock(&p->lock);
			break;
		}
		ast_mutex_unlock(&p->lock);
		p = p->next;
	}
	if (needlock)
		ast_mutex_unlock(&agentlock);
	if (parent && chan)  {
		if (newlyavailable->ackcall > 1) {
			/* Don't do beep here */
			res = 0;
		} else {
			if (option_debug > 2)
				ast_log( LOG_DEBUG, "Playing beep, lang '%s'\n", newlyavailable->chan->language);
			res = ast_streamfile(newlyavailable->chan, beep, newlyavailable->chan->language);
			if (option_debug > 2)
				ast_log( LOG_DEBUG, "Played beep, result '%d'\n", res);
			if (!res) {
				res = ast_waitstream(newlyavailable->chan, "");
				ast_log( LOG_DEBUG, "Waited for stream, result '%d'\n", res);
			}
		}
		if (!res) {
			/* Note -- parent may have disappeared */
			if (p->abouttograb) {
				newlyavailable->acknowledged = 1;
				/* Safe -- agent lock already held */
				ast_setstate(parent, AST_STATE_UP);
				ast_setstate(chan, AST_STATE_UP);
				ast_copy_string(parent->context, chan->context, sizeof(parent->context));
				/* Go ahead and mark the channel as a zombie so that masquerade will
				   destroy it for us, and we need not call ast_hangup */
				ast_mutex_lock(&parent->lock);
				ast_set_flag(chan, AST_FLAG_ZOMBIE);
				ast_channel_masquerade(parent, chan);
				ast_mutex_unlock(&parent->lock);
				p->abouttograb = 0;
			} else {
				if (option_debug)
					ast_log(LOG_DEBUG, "Sneaky, parent disappeared in the mean time...\n");
				agent_cleanup(newlyavailable);
			}
		} else {
			if (option_debug)
				ast_log(LOG_DEBUG, "Ugh...  Agent hung up at exactly the wrong time\n");
			agent_cleanup(newlyavailable);
		}
	}
	return 0;
}

static int check_beep(struct agent_pvt *newlyavailable, int needlock)
{
	struct agent_pvt *p;
	int res=0;

	ast_log(LOG_DEBUG, "Checking beep availability of '%s'\n", newlyavailable->agent);
	if (needlock)
		ast_mutex_lock(&agentlock);
	p = agents;
	while(p) {
		if (p == newlyavailable) {
			p = p->next;
			continue;
		}
		ast_mutex_lock(&p->lock);
		if (!p->abouttograb && p->pending && ((p->group && (newlyavailable->group & p->group)) || !strcmp(p->agent, newlyavailable->agent))) {
			if (option_debug)
				ast_log(LOG_DEBUG, "Call '%s' looks like a would-be winner for agent '%s'\n", p->owner->name, newlyavailable->agent);
			ast_mutex_unlock(&p->lock);
			break;
		}
		ast_mutex_unlock(&p->lock);
		p = p->next;
	}
	if (needlock)
		ast_mutex_unlock(&agentlock);
	if (p) {
		ast_mutex_unlock(&newlyavailable->lock);
		if (option_debug > 2)
			ast_log( LOG_DEBUG, "Playing beep, lang '%s'\n", newlyavailable->chan->language);
		res = ast_streamfile(newlyavailable->chan, beep, newlyavailable->chan->language);
		if (option_debug > 2)
			ast_log( LOG_DEBUG, "Played beep, result '%d'\n", res);
		if (!res) {
			res = ast_waitstream(newlyavailable->chan, "");
			if (option_debug)
				ast_log( LOG_DEBUG, "Waited for stream, result '%d'\n", res);
		}
		ast_mutex_lock(&newlyavailable->lock);
	}
	return res;
}

/*--- agent_request: Part of the Asterisk PBX interface ---*/
static struct ast_channel *agent_request(const char *type, int format, void *data, int *cause)
{
	struct agent_pvt *p;
	struct ast_channel *chan = NULL;
	char *s;
	ast_group_t groupmatch;
	int groupoff;
	int waitforagent=0;
	int hasagent = 0;
	struct timeval tv;

	s = data;
	if ((s[0] == '@') && (sscanf(s + 1, "%d", &groupoff) == 1)) {
		groupmatch = (1 << groupoff);
	} else if ((s[0] == ':') && (sscanf(s + 1, "%d", &groupoff) == 1)) {
		groupmatch = (1 << groupoff);
		waitforagent = 1;
	} else {
		groupmatch = 0;
	}

	/* Check actual logged in agents first */
	ast_mutex_lock(&agentlock);
	p = agents;
	while(p) {
		ast_mutex_lock(&p->lock);
		if (!p->pending && ((groupmatch && (p->group & groupmatch)) || !strcmp(data, p->agent)) &&
		    ast_strlen_zero(p->loginchan)) {
			if (p->chan)
				hasagent++;
			if (!p->lastdisc.tv_sec) {
				/* Agent must be registered, but not have any active call, and not be in a waiting state */
				if (!p->owner && p->chan) {
					/* Fixed agent */
					chan = agent_new(p, AST_STATE_DOWN);
				}
				if (chan) {
					ast_mutex_unlock(&p->lock);
					break;
				}
			}
		}
		ast_mutex_unlock(&p->lock);
		p = p->next;
	}
	if (!p) {
		p = agents;
		while(p) {
			ast_mutex_lock(&p->lock);
			if (!p->pending && ((groupmatch && (p->group & groupmatch)) || !strcmp(data, p->agent))) {
				if (p->chan || !ast_strlen_zero(p->loginchan))
					hasagent++;
				tv = ast_tvfromboot();
#if 0
				ast_log(LOG_NOTICE, "Time now: %ld, Time of lastdisc: %ld\n", tv.tv_sec, p->lastdisc.tv_sec);
#endif
				if (!p->lastdisc.tv_sec || (tv.tv_sec > p->lastdisc.tv_sec)) {
					p->lastdisc = ast_tv(0, 0);
					/* Agent must be registered, but not have any active call, and not be in a waiting state */
					if (!p->owner && p->chan) {
						/* Could still get a fixed agent */
						chan = agent_new(p, AST_STATE_DOWN);
					} else if (!p->owner && !ast_strlen_zero(p->loginchan)) {
						/* Adjustable agent */
						p->chan = ast_request("Local", format, p->loginchan, cause);
						if (p->chan)
							chan = agent_new(p, AST_STATE_DOWN);
					}
					if (chan) {
						ast_mutex_unlock(&p->lock);
						break;
					}
				}
			}
			ast_mutex_unlock(&p->lock);
			p = p->next;
		}
	}

	if (!chan && waitforagent) {
		/* No agent available -- but we're requesting to wait for one.
		   Allocate a place holder */
		if (hasagent) {
			if (option_debug)
				ast_log(LOG_DEBUG, "Creating place holder for '%s'\n", s);
			p = add_agent(data, 1);
			p->group = groupmatch;
			chan = agent_new(p, AST_STATE_DOWN);
			if (!chan) {
				ast_log(LOG_WARNING, "Weird...  Fix this to drop the unused pending agent\n");
			}
		} else
			ast_log(LOG_DEBUG, "Not creating place holder for '%s' since nobody logged in\n", s);
	}
	if (hasagent)
		*cause = AST_CAUSE_BUSY;
	else
		*cause = AST_CAUSE_UNREGISTERED;
	ast_mutex_unlock(&agentlock);
	return chan;
}

static int powerof(unsigned int v)
{
	int x;
	for (x=0;x<32;x++) {
		if (v & (1 << x)) return x;
	}
	return 0;
}

/**
 * Lists agents and their status to the Manager API.
 * It is registered on load_module() and it gets called by the manager backend.
 * @param s
 * @param m
 * @returns 
 * @sa action_agent_logoff(), action_agent_callback_login(), load_module().
 */
static int action_agents(struct mansession *s, struct message *m)
{
	char *id = astman_get_header(m,"ActionID");
	char idText[256] = "";
	char chanbuf[256];
	struct agent_pvt *p;
	char *username = NULL;
	char *loginChan = NULL;
	char *talkingtoChan = NULL;
	char *status = NULL;

	if (!ast_strlen_zero(id))
		snprintf(idText, sizeof(idText) ,"ActionID: %s\r\n", id);
	astman_send_ack(s, m, "Agents will follow");
	ast_mutex_lock(&agentlock);
	p = agents;
	while(p) {
        	ast_mutex_lock(&p->lock);

		/* Status Values:
		   AGENT_LOGGEDOFF - Agent isn't logged in
		   AGENT_IDLE      - Agent is logged in, and waiting for call
		   AGENT_ONCALL    - Agent is logged in, and on a call
		   AGENT_UNKNOWN   - Don't know anything about agent. Shouldn't ever get this. */

		if(!ast_strlen_zero(p->name)) {
			username = p->name;
		} else {
			username = "None";
		}

		/* Set a default status. It 'should' get changed. */
		status = "AGENT_UNKNOWN";

		if (!ast_strlen_zero(p->loginchan) && !p->chan) {
			loginChan = p->loginchan;
			talkingtoChan = "n/a";
			status = "AGENT_IDLE";
			if (p->acknowledged) {
				snprintf(chanbuf, sizeof(chanbuf), " %s (Confirmed)", p->loginchan);
				loginChan = chanbuf;
			}
		} else if (p->chan) {
			loginChan = ast_strdupa(p->chan->name);
			if (p->owner && p->owner->_bridge) {
        			talkingtoChan = p->chan->cid.cid_num;
        			status = "AGENT_ONCALL";
			} else {
        			talkingtoChan = "n/a";
        			status = "AGENT_IDLE";
			}
		} else {
			loginChan = "n/a";
			talkingtoChan = "n/a";
			status = "AGENT_LOGGEDOFF";
		}

		ast_cli(s->fd, "Event: Agents\r\n"
			"Agent: %s\r\n"
			"Name: %s\r\n"
			"Status: %s\r\n"
			"LoggedInChan: %s\r\n"
			"LoggedInTime: %d\r\n"
			"TalkingTo: %s\r\n"
			"%s"
			"\r\n",
			p->agent, username, status, loginChan, (int)p->loginstart, talkingtoChan, idText);
		ast_mutex_unlock(&p->lock);
		p = p->next;
	}
	ast_mutex_unlock(&agentlock);
	ast_cli(s->fd, "Event: AgentsComplete\r\n"
		"%s"
		"\r\n",idText);
	return 0;
}

static int agent_logoff(char *agent, int soft)
{
	struct agent_pvt *p;
	long logintime;
	int ret = -1; /* Return -1 if no agent if found */

	for (p=agents; p; p=p->next) {
		if (!strcasecmp(p->agent, agent)) {
			if (!soft) {
				if (p->owner) {
					ast_softhangup(p->owner, AST_SOFTHANGUP_EXPLICIT);
				}
				if (p->chan) {
					ast_softhangup(p->chan, AST_SOFTHANGUP_EXPLICIT);
				}
			}
			ret = 0; /* found an agent => return 0 */
			logintime = time(NULL) - p->loginstart;
			p->loginstart = 0;
			
			manager_event(EVENT_FLAG_AGENT, "Agentcallbacklogoff",
				      "Agent: %s\r\n"
				      "Loginchan: %s\r\n"
				      "Logintime: %ld\r\n",
				      p->agent, p->loginchan, logintime);
			ast_queue_log("NONE", "NONE", agent, "AGENTCALLBACKLOGOFF", "%s|%ld|%s", p->loginchan, logintime, "CommandLogoff");
			set_agentbycallerid(p->logincallerid, NULL);
			p->loginchan[0] = '\0';
			p->logincallerid[0] = '\0';
			ast_device_state_changed("Agent/%s", p->agent);
			if (persistent_agents)
				dump_agents();
			break;
		}
	}

	return ret;
}

static int agent_logoff_cmd(int fd, int argc, char **argv)
{
	int ret;
	char *agent;

	if (argc < 3 || argc > 4)
		return RESULT_SHOWUSAGE;
	if (argc == 4 && strcasecmp(argv[3], "soft"))
		return RESULT_SHOWUSAGE;

	agent = argv[2] + 6;
	ret = agent_logoff(agent, argc == 4);
	if (ret == 0)
		ast_cli(fd, "Logging out %s\n", agent);

	return RESULT_SUCCESS;
}

/**
 * Sets an agent as no longer logged in in the Manager API.
 * It is registered on load_module() and it gets called by the manager backend.
 * @param s
 * @param m
 * @returns 
 * @sa action_agents(), action_agent_callback_login(), load_module().
 */
static int action_agent_logoff(struct mansession *s, struct message *m)
{
	char *agent = astman_get_header(m, "Agent");
	char *soft_s = astman_get_header(m, "Soft"); /* "true" is don't hangup */
	int soft;
	int ret; /* return value of agent_logoff */

	if (ast_strlen_zero(agent)) {
		astman_send_error(s, m, "No agent specified");
		return 0;
	}

	if (ast_true(soft_s))
		soft = 1;
	else
		soft = 0;

	ret = agent_logoff(agent, soft);
	if (ret == 0)
		astman_send_ack(s, m, "Agent logged out");
	else
		astman_send_error(s, m, "No such agent");

	return 0;
}

static char *complete_agent_logoff_cmd(char *line, char *word, int pos, int state)
{
	struct agent_pvt *p;
	char name[AST_MAX_AGENT];
	int which = 0;

	if (pos == 2) {
		for (p=agents; p; p=p->next) {
			snprintf(name, sizeof(name), "Agent/%s", p->agent);
			if (!strncasecmp(word, name, strlen(word))) {
				if (++which > state) {
					return strdup(name);
				}
			}
		}
	} else if (pos == 3 && state == 0) {
		return strdup("soft");
	}
	return NULL;
}

/**
 * Show agents in cli.
 */
static int agents_show(int fd, int argc, char **argv)
{
	struct agent_pvt *p;
	char username[AST_MAX_BUF];
	char location[AST_MAX_BUF] = "";
	char talkingto[AST_MAX_BUF] = "";
	char moh[AST_MAX_BUF];
	int count_agents = 0;		/* Number of agents configured */
	int online_agents = 0;		/* Number of online agents */
	int offline_agents = 0;		/* Number of offline agents */
	if (argc != 2)
		return RESULT_SHOWUSAGE;
	ast_mutex_lock(&agentlock);
	p = agents;
	while(p) {
		ast_mutex_lock(&p->lock);
		if (p->pending) {
			if (p->group)
				ast_cli(fd, "-- Pending call to group %d\n", powerof(p->group));
			else
				ast_cli(fd, "-- Pending call to agent %s\n", p->agent);
		} else {
			if (!ast_strlen_zero(p->name))
				snprintf(username, sizeof(username), "(%s) ", p->name);
			else
				username[0] = '\0';
			if (p->chan) {
				snprintf(location, sizeof(location), "logged in on %s", p->chan->name);
				if (p->owner && ast_bridged_channel(p->owner)) {
					snprintf(talkingto, sizeof(talkingto), " talking to %s", ast_bridged_channel(p->owner)->name);
				} else {
					strcpy(talkingto, " is idle");
				}
				online_agents++;
			} else if (!ast_strlen_zero(p->loginchan)) {
				snprintf(location, sizeof(location) - 20, "available at '%s'", p->loginchan);
				talkingto[0] = '\0';
				online_agents++;
				if (p->acknowledged)
					strncat(location, " (Confirmed)", sizeof(location) - strlen(location) - 1);
			} else {
				strcpy(location, "not logged in");
				talkingto[0] = '\0';
				offline_agents++;
			}
			if (!ast_strlen_zero(p->moh))
				snprintf(moh, sizeof(moh), " (musiconhold is '%s')", p->moh);
			ast_cli(fd, "%-12.12s %s%s%s%s\n", p->agent, 
				username, location, talkingto, moh);
			count_agents++;
		}
		ast_mutex_unlock(&p->lock);
		p = p->next;
	}
	ast_mutex_unlock(&agentlock);
	if ( !count_agents ) {
		ast_cli(fd, "No Agents are configured in %s\n",config);
	} else {
		ast_cli(fd, "%d agents configured [%d online , %d offline]\n",count_agents, online_agents, offline_agents);
	}
	ast_cli(fd, "\n");
	                
	return RESULT_SUCCESS;
}

static char show_agents_usage[] = 
"Usage: show agents\n"
"       Provides summary information on agents.\n";

static char agent_logoff_usage[] =
"Usage: agent logoff <channel> [soft]\n"
"       Sets an agent as no longer logged in.\n"
"       If 'soft' is specified, do not hangup existing calls.\n";

static struct ast_cli_entry cli_show_agents = {
	{ "show", "agents", NULL }, agents_show, 
	"Show status of agents", show_agents_usage, NULL };

static struct ast_cli_entry cli_agent_logoff = {
	{ "agent", "logoff", NULL }, agent_logoff_cmd, 
	"Sets an agent offline", agent_logoff_usage, complete_agent_logoff_cmd };

STANDARD_LOCAL_USER;
LOCAL_USER_DECL;

/*!
 * \brief Log in agent application.
 *
 * \param chan
 * \param data
 * \param callbackmode non-zero for AgentCallbackLogin
 */
static int __login_exec(struct ast_channel *chan, void *data, int callbackmode)
{
	int res=0;
	int tries = 0;
	int max_login_tries = maxlogintries;
	struct agent_pvt *p;
	struct localuser *u;
	int login_state = 0;
	char user[AST_MAX_AGENT] = "";
	char pass[AST_MAX_AGENT];
	char agent[AST_MAX_AGENT] = "";
	char xpass[AST_MAX_AGENT] = "";
	char *errmsg;
	char *parse;
	AST_DECLARE_APP_ARGS(args,
			     AST_APP_ARG(agent_id);
			     AST_APP_ARG(options);
			     AST_APP_ARG(extension);
		);
	char *tmpoptions = NULL;
	char *context = NULL;
	int play_announcement = 1;
	char agent_goodbye[AST_MAX_FILENAME_LEN];
	int update_cdr = updatecdr;
	char *filename = "agent-loginok";
	char tmpchan[AST_MAX_BUF] = "";

	LOCAL_USER_ADD(u);

	if (!(parse = ast_strdupa(data))) {
		ast_log(LOG_ERROR, "Out of memory!\n");
		LOCAL_USER_REMOVE(u);
		return -1;
	}

	AST_STANDARD_APP_ARGS(args, parse);

	ast_copy_string(agent_goodbye, agentgoodbye, sizeof(agent_goodbye));

	/* Set Channel Specific Login Overrides */
	if (pbx_builtin_getvar_helper(chan, "AGENTLMAXLOGINTRIES") && strlen(pbx_builtin_getvar_helper(chan, "AGENTLMAXLOGINTRIES"))) {
		max_login_tries = atoi(pbx_builtin_getvar_helper(chan, "AGENTMAXLOGINTRIES"));
		if (max_login_tries < 0)
			max_login_tries = 0;
		tmpoptions=pbx_builtin_getvar_helper(chan, "AGENTMAXLOGINTRIES");
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "Saw variable AGENTMAXLOGINTRIES=%s, setting max_login_tries to: %d on Channel '%s'.\n",tmpoptions,max_login_tries,chan->name);
	}
	if (pbx_builtin_getvar_helper(chan, "AGENTUPDATECDR") && !ast_strlen_zero(pbx_builtin_getvar_helper(chan, "AGENTUPDATECDR"))) {
		if (ast_true(pbx_builtin_getvar_helper(chan, "AGENTUPDATECDR")))
			update_cdr = 1;
		else
			update_cdr = 0;
		tmpoptions=pbx_builtin_getvar_helper(chan, "AGENTUPDATECDR");
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "Saw variable AGENTUPDATECDR=%s, setting update_cdr to: %d on Channel '%s'.\n",tmpoptions,update_cdr,chan->name);
	}
	if (pbx_builtin_getvar_helper(chan, "AGENTGOODBYE") && !ast_strlen_zero(pbx_builtin_getvar_helper(chan, "AGENTGOODBYE"))) {
		strcpy(agent_goodbye, pbx_builtin_getvar_helper(chan, "AGENTGOODBYE"));
		tmpoptions=pbx_builtin_getvar_helper(chan, "AGENTGOODBYE");
		if (option_verbose > 2)
			ast_verbose(VERBOSE_PREFIX_3 "Saw variable AGENTGOODBYE=%s, setting agent_goodbye to: %s on Channel '%s'.\n",tmpoptions,agent_goodbye,chan->name);
	}
	/* End Channel Specific Login Overrides */
	
	if (callbackmode && args.extension) {
		parse = args.extension;
		args.extension = strsep(&parse, "@");
		context = parse;
	}

	if (!ast_strlen_zero(args.options)) {
		if (strchr(args.options, 's')) {
			play_announcement = 0;
		}
	}

	if (chan->_state != AST_STATE_UP)
		res = ast_answer(chan);
	if (!res) {
		if (!ast_strlen_zero(args.agent_id))
			ast_copy_string(user, args.agent_id, AST_MAX_AGENT);
		else
			res = ast_app_getdata(chan, "agent-user", user, sizeof(user) - 1, 0);
	}
	while (!res && (max_login_tries==0 || tries < max_login_tries)) {
		tries++;
		/* Check for password */
		ast_mutex_lock(&agentlock);
		p = agents;
		while(p) {
			if (!strcmp(p->agent, user) && !p->pending)
				ast_copy_string(xpass, p->password, sizeof(xpass));
			p = p->next;
		}
		ast_mutex_unlock(&agentlock);
		if (!res) {
			if (!ast_strlen_zero(xpass))
				res = ast_app_getdata(chan, "agent-pass", pass, sizeof(pass) - 1, 0);
			else
				pass[0] = '\0';
		}
		errmsg = "agent-incorrect";

#if 0
		ast_log(LOG_NOTICE, "user: %s, pass: %s\n", user, pass);
#endif		

		/* Check again for accuracy */
		ast_mutex_lock(&agentlock);
		p = agents;
		while(p) {
			ast_mutex_lock(&p->lock);
			if (!strcmp(p->agent, user) &&
			    !strcmp(p->password, pass) && !p->pending) {
				login_state = 1; /* Successful Login */

				/* Ensure we can't be gotten until we're done */
				p->lastdisc = ast_tvfromboot();
				p->lastdisc.tv_sec++;

				/* Set Channel Specific Agent Overrides */
				if (pbx_builtin_getvar_helper(chan, "AGENTACKCALL") && strlen(pbx_builtin_getvar_helper(chan, "AGENTACKCALL"))) {
					if (!strcasecmp(pbx_builtin_getvar_helper(chan, "AGENTACKCALL"), "always"))
						p->ackcall = 2;
					else if (ast_true(pbx_builtin_getvar_helper(chan, "AGENTACKCALL")))
						p->ackcall = 1;
					else
						p->ackcall = 0;
					tmpoptions=pbx_builtin_getvar_helper(chan, "AGENTACKCALL");
					if (option_verbose > 2)
						ast_verbose(VERBOSE_PREFIX_3 "Saw variable AGENTACKCALL=%s, setting ackcall to: %d for Agent '%s'.\n",tmpoptions,p->ackcall,p->agent);
				}
				if (pbx_builtin_getvar_helper(chan, "AGENTAUTOLOGOFF") && strlen(pbx_builtin_getvar_helper(chan, "AGENTAUTOLOGOFF"))) {
					p->autologoff = atoi(pbx_builtin_getvar_helper(chan, "AGENTAUTOLOGOFF"));
					if (p->autologoff < 0)
						p->autologoff = 0;
					tmpoptions=pbx_builtin_getvar_helper(chan, "AGENTAUTOLOGOFF");
					if (option_verbose > 2)
						ast_verbose(VERBOSE_PREFIX_3 "Saw variable AGENTAUTOLOGOFF=%s, setting autologff to: %d for Agent '%s'.\n",tmpoptions,p->autologoff,p->agent);
				}
				if (pbx_builtin_getvar_helper(chan, "AGENTWRAPUPTIME") && strlen(pbx_builtin_getvar_helper(chan, "AGENTWRAPUPTIME"))) {
					p->wrapuptime = atoi(pbx_builtin_getvar_helper(chan, "AGENTWRAPUPTIME"));
					if (p->wrapuptime < 0)
						p->wrapuptime = 0;
					tmpoptions=pbx_builtin_getvar_helper(chan, "AGENTWRAPUPTIME");
					if (option_verbose > 2)
						ast_verbose(VERBOSE_PREFIX_3 "Saw variable AGENTWRAPUPTIME=%s, setting wrapuptime to: %d for Agent '%s'.\n",tmpoptions,p->wrapuptime,p->agent);
				}
				/* End Channel Specific Agent Overrides */
				if (!p->chan) {
					char last_loginchan[80] = "";
					long logintime;
					snprintf(agent, sizeof(agent), "Agent/%s", p->agent);

					if (callbackmode) {
						int pos = 0;
						/* Retrieve login chan */
						for (;;) {
							if (!ast_strlen_zero(args.extension)) {
								ast_copy_string(tmpchan, args.extension, sizeof(tmpchan));
								res = 0;
							} else
								res = ast_app_getdata(chan, "agent-newlocation", tmpchan+pos, sizeof(tmpchan) - 2, 0);
							if (ast_strlen_zero(tmpchan) || ast_exists_extension(chan, !ast_strlen_zero(context) ? context : "default", tmpchan,
													     1, NULL))
								break;
							if (args.extension) {
								ast_log(LOG_WARNING, "Extension '%s' is not valid for automatic login of agent '%s'\n", args.extension, p->agent);
								args.extension = NULL;
								pos = 0;
							} else {
								ast_log(LOG_WARNING, "Extension '%s@%s' is not valid for automatic login of agent '%s'\n", tmpchan, !ast_strlen_zero(context) ? context : "default", p->agent);
								res = ast_streamfile(chan, "invalid", chan->language);
								if (!res)
									res = ast_waitstream(chan, AST_DIGIT_ANY);
								if (res > 0) {
									tmpchan[0] = res;
									tmpchan[1] = '\0';
									pos = 1;
								} else {
									tmpchan[0] = '\0';
									pos = 0;
								}
							}
						}
						args.extension = tmpchan;
						if (!res) {
							set_agentbycallerid(p->logincallerid, NULL);
							if (!ast_strlen_zero(context) && !ast_strlen_zero(tmpchan))
								snprintf(p->loginchan, sizeof(p->loginchan), "%s@%s", tmpchan, context);
							else {
								ast_copy_string(last_loginchan, p->loginchan, sizeof(last_loginchan));
								ast_copy_string(p->loginchan, tmpchan, sizeof(p->loginchan));
							}
							p->acknowledged = 0;
							if (ast_strlen_zero(p->loginchan)) {
								login_state = 2;
								filename = "agent-loggedoff";
							} else {
								if (chan->cid.cid_num) {
									ast_copy_string(p->logincallerid, chan->cid.cid_num, sizeof(p->logincallerid));
									set_agentbycallerid(p->logincallerid, p->agent);
								} else
									p->logincallerid[0] = '\0';
							}

							if(update_cdr && chan->cdr)
								snprintf(chan->cdr->channel, sizeof(chan->cdr->channel), "Agent/%s", p->agent);

						}
					} else {
						p->loginchan[0] = '\0';
						p->logincallerid[0] = '\0';
						p->acknowledged = 0;
					}
					ast_mutex_unlock(&p->lock);
					ast_mutex_unlock(&agentlock);
					if( !res && play_announcement==1 )
						res = ast_streamfile(chan, filename, chan->language);
					if (!res)
						ast_waitstream(chan, "");
					ast_mutex_lock(&agentlock);
					ast_mutex_lock(&p->lock);
					if (!res) {
						res = ast_set_read_format(chan, ast_best_codec(chan->nativeformats));
						if (res)
							ast_log(LOG_WARNING, "Unable to set read format to %d\n", ast_best_codec(chan->nativeformats));
					}
					if (!res) {
						res = ast_set_write_format(chan, ast_best_codec(chan->nativeformats));
						if (res)
							ast_log(LOG_WARNING, "Unable to set write format to %d\n", ast_best_codec(chan->nativeformats));
					}
					/* Check once more just in case */
					if (p->chan)
						res = -1;
					if (callbackmode && !res) {
						/* Just say goodbye and be done with it */
						if (!ast_strlen_zero(p->loginchan)) {
							if (p->loginstart == 0)
								time(&p->loginstart);
							manager_event(EVENT_FLAG_AGENT, "Agentcallbacklogin",
								      "Agent: %s\r\n"
								      "Loginchan: %s\r\n"
								      "Uniqueid: %s\r\n",
								      p->agent, p->loginchan, chan->uniqueid);
							ast_queue_log("NONE", chan->uniqueid, agent, "AGENTCALLBACKLOGIN", "%s", p->loginchan);
							if (option_verbose > 1)
								ast_verbose(VERBOSE_PREFIX_2 "Callback Agent '%s' logged in on %s\n", p->agent, p->loginchan);
							ast_device_state_changed("Agent/%s", p->agent);
						} else {
							logintime = time(NULL) - p->loginstart;
							p->loginstart = 0;
							manager_event(EVENT_FLAG_AGENT, "Agentcallbacklogoff",
								      "Agent: %s\r\n"
								      "Loginchan: %s\r\n"
								      "Logintime: %ld\r\n"
								      "Uniqueid: %s\r\n",
								      p->agent, last_loginchan, logintime, chan->uniqueid);
							ast_queue_log("NONE", chan->uniqueid, agent, "AGENTCALLBACKLOGOFF", "%s|%ld|", last_loginchan, logintime);
							if (option_verbose > 1)
								ast_verbose(VERBOSE_PREFIX_2 "Callback Agent '%s' logged out\n", p->agent);
							ast_device_state_changed("Agent/%s", p->agent);
						}
						ast_mutex_unlock(&agentlock);
						if (!res)
							res = ast_safe_sleep(chan, 500);
						ast_mutex_unlock(&p->lock);
						if (persistent_agents)
							dump_agents();
					} else if (!res) {
#ifdef HONOR_MUSIC_CLASS
						/* check if the moh class was changed with setmusiconhold */
						if (*(chan->musicclass))
							ast_copy_string(p->moh, chan->musicclass, sizeof(p->moh));
#endif								
						ast_moh_start(chan, p->moh);
						if (p->loginstart == 0)
							time(&p->loginstart);
						manager_event(EVENT_FLAG_AGENT, "Agentlogin",
							      "Agent: %s\r\n"
							      "Channel: %s\r\n"
							      "Uniqueid: %s\r\n",
							      p->agent, chan->name, chan->uniqueid);
						if (update_cdr && chan->cdr)
							snprintf(chan->cdr->channel, sizeof(chan->cdr->channel), "Agent/%s", p->agent);
						ast_queue_log("NONE", chan->uniqueid, agent, "AGENTLOGIN", "%s", chan->name);
						if (option_verbose > 1)
							ast_verbose(VERBOSE_PREFIX_2 "Agent '%s' logged in (format %s/%s)\n", p->agent,
								    ast_getformatname(chan->readformat), ast_getformatname(chan->writeformat));
						/* Login this channel and wait for it to
						   go away */
						p->chan = chan;
						if (p->ackcall > 1)
							check_beep(p, 0);
						else
							check_availability(p, 0);
						ast_mutex_unlock(&p->lock);
						ast_mutex_unlock(&agentlock);
						ast_device_state_changed("Agent/%s", p->agent);
						while (res >= 0) {
							ast_mutex_lock(&p->lock);
							if (p->chan != chan)
								res = -1;
							ast_mutex_unlock(&p->lock);
							/* Yield here so other interested threads can kick in. */
							sched_yield();
							if (res)
								break;

							ast_mutex_lock(&agentlock);
							ast_mutex_lock(&p->lock);
							if (p->lastdisc.tv_sec) {
								if (ast_tvdiff_ms(ast_tvfromboot(), p->lastdisc) > p->wrapuptime) {
									if (option_debug)
										ast_log(LOG_DEBUG, "Wrapup time for %s expired!\n", p->agent);
									p->lastdisc = ast_tv(0, 0);
									if (p->ackcall > 1)
										check_beep(p, 0);
									else
										check_availability(p, 0);
								}
							}
							ast_mutex_unlock(&p->lock);
							ast_mutex_unlock(&agentlock);
							/*	Synchronize channel ownership between call to agent and itself. */
							ast_mutex_lock( &p->app_lock );
							ast_mutex_lock(&p->lock);
							p->owning_app = pthread_self();
							ast_mutex_unlock(&p->lock);
							if (p->ackcall > 1) 
								res = agent_ack_sleep(p);
							else
								res = ast_safe_sleep_conditional( chan, 1000,
												  agent_cont_sleep, p );
							ast_mutex_unlock( &p->app_lock );
							if ((p->ackcall > 1)  && (res == 1)) {
								ast_mutex_lock(&agentlock);
								ast_mutex_lock(&p->lock);
								check_availability(p, 0);
								ast_mutex_unlock(&p->lock);
								ast_mutex_unlock(&agentlock);
								res = 0;
							}
							sched_yield();
						}
						ast_mutex_lock(&p->lock);
						if (res && p->owner) 
							ast_log(LOG_WARNING, "Huh?  We broke out when there was still an owner?\n");
						/* Log us off if appropriate */
						if (p->chan == chan)
							p->chan = NULL;
						p->acknowledged = 0;
						logintime = time(NULL) - p->loginstart;
						p->loginstart = 0;
						ast_mutex_unlock(&p->lock);
						manager_event(EVENT_FLAG_AGENT, "Agentlogoff",
							      "Agent: %s\r\n"
							      "Logintime: %ld\r\n"
							      "Uniqueid: %s\r\n",
							      p->agent, logintime, chan->uniqueid);
						ast_queue_log("NONE", chan->uniqueid, agent, "AGENTLOGOFF", "%s|%ld", chan->name, logintime);
						if (option_verbose > 1)
							ast_verbose(VERBOSE_PREFIX_2 "Agent '%s' logged out\n", p->agent);
						/* If there is no owner, go ahead and kill it now */
						ast_device_state_changed("Agent/%s", p->agent);
						if (p->dead && !p->owner) {
							ast_mutex_destroy(&p->lock);
							ast_mutex_destroy(&p->app_lock);
							free(p);
						}
					}
					else {
						ast_mutex_unlock(&p->lock);
						p = NULL;
					}
					res = -1;
				} else {
					ast_mutex_unlock(&p->lock);
					errmsg = "agent-alreadyon";
					p = NULL;
				}
				break;
			}
			ast_mutex_unlock(&p->lock);
			p = p->next;
		}
		if (!p)
			ast_mutex_unlock(&agentlock);

		if (!res && (max_login_tries==0 || tries < max_login_tries))
			res = ast_app_getdata(chan, errmsg, user, sizeof(user) - 1, 0);
	}
		
	if (!res)
		res = ast_safe_sleep(chan, 500);

	/* AgentLogin() exit */
	if (!callbackmode) {
		LOCAL_USER_REMOVE(u);
		return -1;
	}
	/* AgentCallbackLogin() exit*/
	else {
		/* Set variables */
		if (login_state > 0) {
			pbx_builtin_setvar_helper(chan, "AGENTNUMBER", user);
			if (login_state==1) {
				pbx_builtin_setvar_helper(chan, "AGENTSTATUS", "on");
				pbx_builtin_setvar_helper(chan, "AGENTEXTEN", args.extension);
			}
			else {
				pbx_builtin_setvar_helper(chan, "AGENTSTATUS", "off");
			}
		}
		else {
			pbx_builtin_setvar_helper(chan, "AGENTSTATUS", "fail");
		}
		if (ast_exists_extension(chan, chan->context, chan->exten, chan->priority + 1, chan->cid.cid_num)) {
			LOCAL_USER_REMOVE(u);
			return 0;
		}
		/* Do we need to play agent-goodbye now that we will be hanging up? */
		if (play_announcement) {
			if (!res)
				res = ast_safe_sleep(chan, 1000);
			res = ast_streamfile(chan, agent_goodbye, chan->language);
			if (!res)
				res = ast_waitstream(chan, "");
			if (!res)
				res = ast_safe_sleep(chan, 1000);
		}
	}

	LOCAL_USER_REMOVE(u);
	
	/* We should never get here if next priority exists when in callbackmode */
 	return -1;
}

/**
 * Called by the AgentLogin application (from the dial plan).
 * 
 * @param chan
 * @param data
 * @returns
 * @sa callback_login_exec(), agentmonitoroutgoing_exec(), load_module().
 */
static int login_exec(struct ast_channel *chan, void *data)
{
	return __login_exec(chan, data, 0);
}

/**
 *  Called by the AgentCallbackLogin application (from the dial plan).
 * 
 * @param chan
 * @param data
 * @returns
 * @sa login_exec(), agentmonitoroutgoing_exec(), load_module().
 */
static int callback_exec(struct ast_channel *chan, void *data)
{
	return __login_exec(chan, data, 1);
}

/**
 * Sets an agent as logged in by callback in the Manager API.
 * It is registered on load_module() and it gets called by the manager backend.
 * @param s
 * @param m
 * @returns 
 * @sa action_agents(), action_agent_logoff(), load_module().
 */
static int action_agent_callback_login(struct mansession *s, struct message *m)
{
	char *agent = astman_get_header(m, "Agent");
	char *exten = astman_get_header(m, "Exten");
	char *context = astman_get_header(m, "Context");
	char *wrapuptime_s = astman_get_header(m, "WrapupTime");
	char *ackcall_s = astman_get_header(m, "AckCall");
	struct agent_pvt *p;
	int login_state = 0;

	if (ast_strlen_zero(agent)) {
		astman_send_error(s, m, "No agent specified");
		return 0;
	}

	if (ast_strlen_zero(exten)) {
		astman_send_error(s, m, "No extension specified");
		return 0;
	}

	ast_mutex_lock(&agentlock);
	p = agents;
	while(p) {
		if (strcmp(p->agent, agent) || p->pending) {
			p = p->next;
			continue;
		}
		if (p->chan) {
			login_state = 2; /* already logged in (and on the phone)*/
			break;
		}
		ast_mutex_lock(&p->lock);
		login_state = 1; /* Successful Login */
		
		if (ast_strlen_zero(context))
			ast_copy_string(p->loginchan, exten, sizeof(p->loginchan));
		else
			snprintf(p->loginchan, sizeof(p->loginchan), "%s@%s", exten, context);

		if (!ast_strlen_zero(wrapuptime_s)) {
			p->wrapuptime = atoi(wrapuptime_s);
			if (p->wrapuptime < 0)
				p->wrapuptime = 0;
		}

		if (ast_true(ackcall_s))
			p->ackcall = 1;
		else
			p->ackcall = 0;

		if (p->loginstart == 0)
			time(&p->loginstart);
		manager_event(EVENT_FLAG_AGENT, "Agentcallbacklogin",
			      "Agent: %s\r\n"
			      "Loginchan: %s\r\n",
			      p->agent, p->loginchan);
		ast_queue_log("NONE", "NONE", agent, "AGENTCALLBACKLOGIN", "%s", p->loginchan);
		if (option_verbose > 1)
			ast_verbose(VERBOSE_PREFIX_2 "Callback Agent '%s' logged in on %s\n", p->agent, p->loginchan);
		ast_device_state_changed("Agent/%s", p->agent);
		ast_mutex_unlock(&p->lock);
		p = p->next;
	}
	ast_mutex_unlock(&agentlock);

	if (login_state == 1)
		astman_send_ack(s, m, "Agent logged in");
	else if (login_state == 0)
		astman_send_error(s, m, "No such agent");
	else if (login_state == 2)
		astman_send_error(s, m, "Agent already logged in");

	return 0;
}

/**
 *  Called by the AgentMonitorOutgoing application (from the dial plan).
 *
 * @param chan
 * @param data
 * @returns
 * @sa login_exec(), callback_login_exec(), load_module().
 */
static int agentmonitoroutgoing_exec(struct ast_channel *chan, void *data)
{
	int exitifnoagentid = 0;
	int nowarnings = 0;
	int changeoutgoing = 0;
	int res = 0;
	char agent[AST_MAX_AGENT], *tmp;

	if (data) {
		if (strchr(data, 'd'))
			exitifnoagentid = 1;
		if (strchr(data, 'n'))
			nowarnings = 1;
		if (strchr(data, 'c'))
			changeoutgoing = 1;
	}
	if (chan->cid.cid_num) {
		char agentvar[AST_MAX_BUF];
		snprintf(agentvar, sizeof(agentvar), "%s_%s", GETAGENTBYCALLERID, chan->cid.cid_num);
		if ((tmp = pbx_builtin_getvar_helper(NULL, agentvar))) {
			struct agent_pvt *p = agents;
			ast_copy_string(agent, tmp, sizeof(agent));
			ast_mutex_lock(&agentlock);
			while (p) {
				if (!strcasecmp(p->agent, tmp)) {
					if (changeoutgoing) snprintf(chan->cdr->channel, sizeof(chan->cdr->channel), "Agent/%s", p->agent);
					__agent_start_monitoring(chan, p, 1);
					break;
				}
				p = p->next;
			}
			ast_mutex_unlock(&agentlock);
			
		} else {
			res = -1;
			if (!nowarnings)
				ast_log(LOG_WARNING, "Couldn't find the global variable %s, so I can't figure out which agent (if it's an agent) is placing outgoing call.\n", agentvar);
		}
	} else {
		res = -1;
		if (!nowarnings)
			ast_log(LOG_WARNING, "There is no callerid on that call, so I can't figure out which agent (if it's an agent) is placing outgoing call.\n");
	}
	/* check if there is n + 101 priority */
	if (res) {
		if (ast_exists_extension(chan, chan->context, chan->exten, chan->priority + 101, chan->cid.cid_num)) {
			chan->priority+=100;
			if (option_verbose > 2)
				ast_verbose(VERBOSE_PREFIX_3 "Going to %d priority because there is no callerid or the agentid cannot be found.\n",chan->priority);
		}
		else if (exitifnoagentid)
			return res;
	}
	return 0;
}

/**
 * Dump AgentCallbackLogin agents to the database for persistence
 */
static void dump_agents(void)
{
	struct agent_pvt *cur_agent = NULL;
	char buf[256];

	for (cur_agent = agents; cur_agent; cur_agent = cur_agent->next) {
		if (cur_agent->chan)
			continue;

		if (!ast_strlen_zero(cur_agent->loginchan)) {
			snprintf(buf, sizeof(buf), "%s;%s", cur_agent->loginchan, cur_agent->logincallerid);
			if (ast_db_put(pa_family, cur_agent->agent, buf))
				ast_log(LOG_WARNING, "failed to create persistent entry!\n");
			else if (option_debug)
				ast_log(LOG_DEBUG, "Saved Agent: %s on %s\n", cur_agent->agent, cur_agent->loginchan);
		} else {
			/* Delete -  no agent or there is an error */
			ast_db_del(pa_family, cur_agent->agent);
		}
	}
}

/**
 * Reload the persistent agents from astdb.
 */
static void reload_agents(void)
{
	char *agent_num;
	struct ast_db_entry *db_tree;
	struct ast_db_entry *entry;
	struct agent_pvt *cur_agent;
	char agent_data[256];
	char *parse;
	char *agent_chan;
	char *agent_callerid;

	db_tree = ast_db_gettree(pa_family, NULL);

	ast_mutex_lock(&agentlock);
	for (entry = db_tree; entry; entry = entry->next) {
		agent_num = entry->key + strlen(pa_family) + 2;
		cur_agent = agents;
		while (cur_agent) {
			ast_mutex_lock(&cur_agent->lock);
			if (strcmp(agent_num, cur_agent->agent) == 0)
				break;
			ast_mutex_unlock(&cur_agent->lock);
			cur_agent = cur_agent->next;
		}
		if (!cur_agent) {
			ast_db_del(pa_family, agent_num);
			continue;
		} else
			ast_mutex_unlock(&cur_agent->lock);
		if (!ast_db_get(pa_family, agent_num, agent_data, sizeof(agent_data)-1)) {
			if (option_debug)
				ast_log(LOG_DEBUG, "Reload Agent: %s on %s\n", cur_agent->agent, agent_data);
			parse = agent_data;
			agent_chan = strsep(&parse, ";");
			agent_callerid = strsep(&parse, ";");
			ast_copy_string(cur_agent->loginchan, agent_chan, sizeof(cur_agent->loginchan));
			if (agent_callerid) {
				ast_copy_string(cur_agent->logincallerid, agent_callerid, sizeof(cur_agent->logincallerid));
				set_agentbycallerid(cur_agent->logincallerid, cur_agent->agent);
			} else
				cur_agent->logincallerid[0] = '\0';
			if (cur_agent->loginstart == 0)
				time(&cur_agent->loginstart);
			ast_device_state_changed("Agent/%s", cur_agent->agent);	
		}
	}
	ast_mutex_unlock(&agentlock);
	if (db_tree) {
		ast_log(LOG_NOTICE, "Agents successfully reloaded from database.\n");
		ast_db_freetree(db_tree);
	}
}

/*--- agent_devicestate: Part of PBX channel interface ---*/
static int agent_devicestate(void *data)
{
	struct agent_pvt *p;
	char *s;
	ast_group_t groupmatch;
	int groupoff;
	int waitforagent=0;
	int res = AST_DEVICE_INVALID;
	
	s = data;
	if ((s[0] == '@') && (sscanf(s + 1, "%d", &groupoff) == 1)) {
		groupmatch = (1 << groupoff);
	} else if ((s[0] == ':') && (sscanf(s + 1, "%d", &groupoff) == 1)) {
		groupmatch = (1 << groupoff);
		waitforagent = 1;
	} else {
		groupmatch = 0;
	}

	/* Check actual logged in agents first */
	ast_mutex_lock(&agentlock);
	p = agents;
	while(p) {
		ast_mutex_lock(&p->lock);
		if (!p->pending && ((groupmatch && (p->group & groupmatch)) || !strcmp(data, p->agent))) {
			if (p->owner) {
				if (res != AST_DEVICE_INUSE)
					res = AST_DEVICE_BUSY;
			} else {
				if (res == AST_DEVICE_BUSY)
					res = AST_DEVICE_INUSE;
				if (p->chan || !ast_strlen_zero(p->loginchan)) {
					if (res == AST_DEVICE_INVALID)
						res = AST_DEVICE_UNKNOWN;
				} else if (res == AST_DEVICE_INVALID)	
					res = AST_DEVICE_UNAVAILABLE;
			}
			if (!strcmp(data, p->agent)) {
				ast_mutex_unlock(&p->lock);
				break;
			}
		}
		ast_mutex_unlock(&p->lock);
		p = p->next;
	}
	ast_mutex_unlock(&agentlock);
	return res;
}

/**
 * Initialize the Agents module.
 * This function is being called by Asterisk when loading the module. Among other thing it registers applications, cli commands and reads the cofiguration file.
 *
 * @returns int Always 0.
 */
int load_module()
{
	/* Make sure we can register our agent channel type */
	if (ast_channel_register(&agent_tech)) {
		ast_log(LOG_ERROR, "Unable to register channel class %s\n", channeltype);
		return -1;
	}
	/* Dialplan applications */
	ast_register_application(app, login_exec, synopsis, descrip);
	ast_register_application(app2, callback_exec, synopsis2, descrip2);
	ast_register_application(app3, agentmonitoroutgoing_exec, synopsis3, descrip3);
	/* Manager commands */
	ast_manager_register2("Agents", EVENT_FLAG_AGENT, action_agents, "Lists agents and their status", mandescr_agents);
	ast_manager_register2("AgentLogoff", EVENT_FLAG_AGENT, action_agent_logoff, "Sets an agent as no longer logged in", mandescr_agent_logoff);
	ast_manager_register2("AgentCallbackLogin", EVENT_FLAG_AGENT, action_agent_callback_login, "Sets an agent as logged in by callback", mandescr_agent_callback_login);
	/* CLI Application */
	ast_cli_register(&cli_show_agents);
	ast_cli_register(&cli_agent_logoff);
	/* Read in the config */
	read_agent_config();
	if (persistent_agents)
		reload_agents();
	return 0;
}

int reload()
{
	read_agent_config();
	if (persistent_agents)
		reload_agents();
	return 0;
}

int unload_module()
{
	struct agent_pvt *p;
	/* First, take us out of the channel loop */
	/* Unregister CLI application */
	ast_cli_unregister(&cli_show_agents);
	ast_cli_unregister(&cli_agent_logoff);
	/* Unregister dialplan applications */
	ast_unregister_application(app);
	ast_unregister_application(app2);
	ast_unregister_application(app3);
	/* Unregister manager command */
	ast_manager_unregister("Agents");
	ast_manager_unregister("AgentLogoff");
	ast_manager_unregister("AgentCallbackLogin");
	/* Unregister channel */
	ast_channel_unregister(&agent_tech);
	if (!ast_mutex_lock(&agentlock)) {
		/* Hangup all interfaces if they have an owner */
		p = agents;
		while(p) {
			if (p->owner)
				ast_softhangup(p->owner, AST_SOFTHANGUP_APPUNLOAD);
			p = p->next;
		}
		agents = NULL;
		ast_mutex_unlock(&agentlock);
	} else {
		ast_log(LOG_WARNING, "Unable to lock the monitor\n");
		return -1;
	}		
	return 0;
}

int usecount()
{
	return usecnt;
}

char *key()
{
	return ASTERISK_GPL_KEY;
}

char *description()
{
	return (char *) desc;
}

