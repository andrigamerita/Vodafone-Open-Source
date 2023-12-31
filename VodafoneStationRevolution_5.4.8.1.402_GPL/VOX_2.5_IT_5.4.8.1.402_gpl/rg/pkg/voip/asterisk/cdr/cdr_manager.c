/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2004 - 2005
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
 * \brief Asterisk Call Manager CDR records.
 * 
 * See also
 * \arg \ref AstCDR
 * \arg \ref AstAMI
 * \arg \ref Config_ami
 * \ingroup cdr_drivers
 */

#include <sys/types.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include "asterisk/channel.h"
#include "asterisk/cdr.h"
#include "asterisk/module.h"
#include "asterisk/logger.h"
#include "asterisk/utils.h"
#include "asterisk/manager.h"
#include "asterisk/config.h"
#include "asterisk/rtp.h"

#define DATE_FORMAT 	"%Y-%m-%d %T"
#define CONF_FILE	"cdr_manager.conf"

static char *desc = "Asterisk Call Manager CDR Backend";
static char *name = "cdr_manager";

static int enablecdr = 0;

static void loadconfigurationfile(void)
{
	char *cat;
	struct ast_config *cfg;
	struct ast_variable *v;
	
	cfg = ast_config_load(CONF_FILE);
	if (!cfg) {
		/* Standard configuration */
		enablecdr = 0;
		return;
	}
	
	cat = ast_category_browse(cfg, NULL);
	while (cat) {
		if (!strcasecmp(cat, "general")) {
			v = ast_variable_browse(cfg, cat);
			while (v) {
				if (!strcasecmp(v->name, "enabled")) {
					enablecdr = ast_true(v->value);
				}
				
				v = v->next;
			}
		}
	
		/* Next category */
		cat = ast_category_browse(cfg, cat);
	}
	
	ast_config_destroy(cfg);
}

static int manager_log(struct ast_cdr *cdr)
{
	time_t t;
	struct tm timeresult;
	char strStartTime[80] = "";
	char strAnswerTime[80] = "";
	char strEndTime[80] = "";
	char workspace[256] = "";
	char *trunkid;
	char us_buf[INET6_ADDRSTRLEN] = "";
	char them_buf[INET6_ADDRSTRLEN] = "";

	if (!enablecdr)
		return 0;

	t = cdr->start.tv_sec;
	localtime_r(&t, &timeresult);
	strftime(strStartTime, sizeof(strStartTime), DATE_FORMAT, &timeresult);
	ast_cdr_getvar(cdr, "trunkid", &trunkid, workspace, sizeof(workspace),
	    0);
	if (cdr->answer.tv_sec)	{
    		t = cdr->answer.tv_sec;
    		localtime_r(&t, &timeresult);
		strftime(strAnswerTime, sizeof(strAnswerTime), DATE_FORMAT, &timeresult);
	}

	t = cdr->end.tv_sec;
	localtime_r(&t, &timeresult);
	strftime(strEndTime, sizeof(strEndTime), DATE_FORMAT, &timeresult);

        ast_sockaddr_to_str_nowrap(us_buf, sizeof(us_buf), &cdr->us);
        ast_sockaddr_to_str_nowrap(them_buf, sizeof(them_buf), &cdr->them);

	manager_event(EVENT_FLAG_CALL, "Cdr",
	    "AccountCode: %s\r\n"
	    "Source: %s\r\n"
	    "Destination: %s\r\n"
	    "DestinationContext: %s\r\n"
	    "CallerID: %s\r\n"
	    "Channel: %s\r\n"
	    "DestinationChannel: %s\r\n"
	    "LastApplication: %s\r\n"
	    "LastData: %s\r\n"
	    "StartTime: %s\r\n"
	    "StartTimeUn: %ld\r\n"
	    "AnswerTime: %s\r\n"
	    "EndTime: %s\r\n"
	    "Duration: %d\r\n"
	    "BillableSeconds: %d\r\n"
	    "Disposition: %s\r\n"
	    "AMAFlags: %s\r\n"
	    "UniqueID: %s\r\n"
	    "UserField: %s\r\n"
	    "TrunkId: %s\r\n"
            "HangupCause: %d\r\n"
	    "PacketsSent: %d\r\n"
	    "PacketsReceived: %d\r\n"
	    "PacketsLost: %d\r\n"
	    "OurIP: %s\r\n"
	    "PeerIP: %s\r\n",
	    cdr->accountcode, cdr->src, cdr->dst, cdr->dcontext, cdr->clid, cdr->channel,
	    cdr->dstchannel, cdr->lastapp, cdr->lastdata, strStartTime,
	    cdr->start.tv_sec, strAnswerTime, strEndTime,
	    cdr->duration, cdr->billsec, ast_cdr_disp2str(cdr->disposition), 
	    ast_cdr_flags2str(cdr->amaflags), cdr->uniqueid, cdr->userfield,
	    trunkid, cdr->hangupcause, cdr->tx_packets, cdr->rx_packets,
            cdr->lost_packets, us_buf, them_buf);
	    	
	return 0;
}

char *description(void)
{
	return desc;
}

int unload_module(void)
{
	ast_cdr_unregister(name);
	return 0;
}

int load_module(void)
{
	int res;

	/* Configuration file */
	loadconfigurationfile();
	
	res = ast_cdr_register(name, desc, manager_log);
	if (res) {
		ast_log(LOG_ERROR, "Unable to register Asterisk Call Manager CDR handling\n");
	}
	
	return res;
}

int reload(void)
{
	loadconfigurationfile();
	return 0;
}

int usecount(void)
{
	return 0;
}

char *key()
{
	return ASTERISK_GPL_KEY;
}
