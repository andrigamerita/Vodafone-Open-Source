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
 * \brief Various sorts of access control
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>

#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)
#include <fcntl.h>
#include <net/route.h>
#endif

#if defined(SOLARIS)
#include <sys/sockio.h>
#endif

/* netinet/ip.h may not define the following (See RFCs 791 and 1349) */
#if !defined(IPTOS_LOWCOST)
#define       IPTOS_LOWCOST           0x02
#endif

#if !defined(IPTOS_MINCOST)
#define       IPTOS_MINCOST           IPTOS_LOWCOST
#endif

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include "asterisk/acl.h"
#include "asterisk/logger.h"
#include "asterisk/channel.h"
#include "asterisk/options.h"
#include "asterisk/utils.h"
#include "asterisk/lock.h"
#include "asterisk/srv.h"
#include "asterisk/compat.h"
#include <linux/version.h>

#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__FreeBSD__)
AST_MUTEX_DEFINE_STATIC(routeseq_lock);
#endif

struct ast_ha {
	/* Host access rule */
	struct ast_sockaddr addr;
	struct ast_sockaddr netmask;
	int sense;
	struct ast_ha *next;
};

/* Default IP - if not otherwise set, don't breathe garbage */
static struct in_addr __ourip = { 0x00000000 };

struct my_ifreq {
	char ifrn_name[IFNAMSIZ];	/* Interface name, e.g. "eth0", "ppp0", etc.  */
	struct sockaddr_in ifru_addr;
};

/* Free HA structure */
void ast_free_ha(struct ast_ha *ha)
{
	struct ast_ha *hal;
	while(ha) {
		hal = ha;
		ha = ha->next;
		free(hal);
	}
}

/* Copy HA structure */
static void ast_copy_ha(struct ast_ha *from, struct ast_ha *to)
{
	ast_sockaddr_copy(&to->addr, &from->addr);
	ast_sockaddr_copy(&to->netmask, &from->netmask);
	to->sense = from->sense;
}

/* Create duplicate of ha structure */
static struct ast_ha *ast_duplicate_ha(struct ast_ha *original)
{
	struct ast_ha *new_ha = malloc(sizeof(struct ast_ha));
	/* Copy from original to new object */
	ast_copy_ha(original, new_ha); 

	return new_ha;
}

/* Create duplicate HA link list */
/*  Used in chan_sip2 templates */
struct ast_ha *ast_duplicate_ha_list(struct ast_ha *original)
{
	struct ast_ha *start=original;
	struct ast_ha *ret = NULL;
	struct ast_ha *link,*prev=NULL;

	while (start) {
		link = ast_duplicate_ha(start);  /* Create copy of this object */
		if (prev)
			prev->next = link;		/* Link previous to this object */

		if (!ret) 
			ret = link;		/* Save starting point */

		start = start->next;		/* Go to next object */
		prev = link;			/* Save pointer to this object */
	}
	return ret;    			/* Return start of list */
}

/*!
 * \brief
 * Isolate a 32-bit section of an IPv6 address
 *
 * An IPv6 address can be divided into 4 32-bit chunks. This gives
 * easy access to one of these chunks.
 *
 * \param sin6 A pointer to a struct sockaddr_in6
 * \param index Which 32-bit chunk to operate on. Must be in the range 0-3.
 */
#define V6_WORD(sin6, index) ((uint32_t *)&((sin6)->sin6_addr))[(index)]

/*!
 * \brief
 * Apply a netmask to an address and store the result in a separate structure.
 *
 * When dealing with IPv6 addresses, one cannot apply a netmask with a simple
 * logical and operation. Furthermore, the incoming address may be an IPv4 address
 * and need to be mapped properly before attempting to apply a rule.
 *
 * \param addr The IP address to apply the mask to.
 * \param netmask The netmask configured in the host access rule.
 * \param result The resultant address after applying the netmask to the given address
 * \retval 0 Successfully applied netmask
 * \reval -1 Failed to apply netmask
 */
static int apply_netmask(const struct ast_sockaddr *addr, const struct ast_sockaddr *netmask,
		struct ast_sockaddr *result)
{
	int res = 0;

	if (ast_sockaddr_is_ipv4(addr)) {
		struct sockaddr_in result4 = { 0, };
		struct sockaddr_in *addr4 = (struct sockaddr_in *) &addr->ss;
		struct sockaddr_in *mask4 = (struct sockaddr_in *) &netmask->ss;
		result4.sin_family = AF_INET;
		result4.sin_addr.s_addr = addr4->sin_addr.s_addr & mask4->sin_addr.s_addr;
		ast_sockaddr_from_sin(result, &result4);
	} else if (ast_sockaddr_is_ipv6(addr)) {
		struct sockaddr_in6 result6 = { 0, };
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) &addr->ss;
		struct sockaddr_in6 *mask6 = (struct sockaddr_in6 *) &netmask->ss;
		int i;
		result6.sin6_family = AF_INET6;
		for (i = 0; i < 4; ++i) {
			V6_WORD(&result6, i) = V6_WORD(addr6, i) & V6_WORD(mask6, i);
		}
		memcpy(&result->ss, &result6, sizeof(result6));
		result->len = sizeof(result6);
	} else {
		/* Unsupported address scheme */
		res = -1;
	}

	return res;
}

/*!
 * \brief
 * Parse a netmask in CIDR notation
 *
 * \details
 * For a mask of an IPv4 address, this should be a number between 0 and 32. For
 * a mask of an IPv6 address, this should be a number between 0 and 128. This
 * function creates an IPv6 ast_sockaddr from the given netmask. For masks of
 * IPv4 addresses, this is accomplished by adding 96 to the original netmask.
 *
 * \param[out] addr The ast_sockaddr produced from the CIDR netmask
 * \param is_v4 Tells if the address we are masking is IPv4.
 * \param mask_str The CIDR mask to convert
 * \retval -1 Failure
 * \retval 0 Success
 */
static int parse_cidr_mask(struct ast_sockaddr *addr, int is_v4, const char *mask_str)
{
	int mask;

	if (sscanf(mask_str, "%30d", &mask) != 1) {
		return -1;
	}

	if (is_v4) {
		struct sockaddr_in sin;
		if (mask < 0 || mask > 32) {
			return -1;
		}
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		/* If mask is 0, then we already have the
		 * appropriate all 0s address in sin from
		 * the above memset.
		 */
		if (mask != 0) {
			sin.sin_addr.s_addr = htonl(0xFFFFFFFF << (32 - mask));
		}
		ast_sockaddr_from_sin(addr, &sin);
	} else {
		struct sockaddr_in6 sin6;
		int i;
		if (mask < 0 || mask > 128) {
			return -1;
		}
		memset(&sin6, 0, sizeof(sin6));
		sin6.sin6_family = AF_INET6;
		for (i = 0; i < 2; ++i) {
			/* Once mask reaches 0, we don't have
			 * to explicitly set anything anymore
			 * since sin6 was zeroed out already
			 */
			if (mask > 0) {
				V6_WORD(&sin6, i) = htonl(0xFFFFFFFF << (mask < 32 ? (32 - mask) : 0));
				mask -= mask < 32 ? mask : 32;
			}
		}
		memcpy(&addr->ss, &sin6, sizeof(sin6));
		addr->len = sizeof(sin6);
	}

	return 0;
}

void ast_append_ha_from_list(char *name, char *sense, char *list, struct ast_ha **ha)
{
	char *addr;
	char *parse = ast_strdupa(list);

	while ((addr = strsep(&parse, ",")))
	{
		if (!(*ha = ast_append_ha(sense, addr, *ha)))
			ast_log(LOG_WARNING, "Invalid value in %s: %s\n", name, addr);
		else
			ast_log(LOG_WARNING, "Added to %s: %s\n", name, addr);
	}
}

struct ast_ha *ast_append_ha(char *sense, char *stuff, struct ast_ha *path)
{
    	struct ast_ha *ha;
	struct ast_ha *prev = NULL;
	struct ast_ha *ret;
	char *tmp = ast_strdupa(stuff);
	char *address = NULL, *mask = NULL;
	int addr_is_v4;
	char addr_buf[INET6_ADDRSTRLEN], mask_buf[INET6_ADDRSTRLEN];

	ret = path;
	while (path) {
		prev = path;
		path = path->next;
	}

	if (!(ha = calloc(1, sizeof(*ha)))) {
		return ret;
	}

	address = strsep(&tmp, "/");
	if (!address) {
		address = tmp;
	} else {
		mask = tmp;
	}

	if (!ast_sockaddr_parse(&ha->addr, address, PARSE_PORT_FORBID)) {
		ast_log(LOG_WARNING, "Invalid IP address: %s\n", address);
		ast_free_ha(ha);
		return ret;
	}

	/* If someone specifies an IPv4-mapped IPv6 address,
	 * we just convert this to an IPv4 ACL
	 */
	if (ast_sockaddr_ipv4_mapped(&ha->addr, &ha->addr)) {
		ast_log(LOG_NOTICE, "IPv4-mapped ACL network address specified. "
				"Converting to an IPv4 ACL network address.\n");
	}

	addr_is_v4 = ast_sockaddr_is_ipv4(&ha->addr);

	if (!mask) {
		parse_cidr_mask(&ha->netmask, addr_is_v4, addr_is_v4 ? "32" : "128");
	} else if (strchr(mask, ':') || strchr(mask, '.')) {
		int mask_is_v4;
		/* Mask is of x.x.x.x or x:x:x:x:x:x:x:x variety */
		if (!ast_sockaddr_parse(&ha->netmask, mask, PARSE_PORT_FORBID)) {
			ast_log(LOG_WARNING, "Invalid netmask: %s\n", mask);
			ast_free_ha(ha);
			return ret;
		}
		/* If someone specifies an IPv4-mapped IPv6 netmask,
		 * we just convert this to an IPv4 ACL
		 */
		if (ast_sockaddr_ipv4_mapped(&ha->netmask, &ha->netmask)) {
			ast_log(LOG_NOTICE, "IPv4-mapped ACL netmask specified. "
					"Converting to an IPv4 ACL netmask.\n");
		}
		mask_is_v4 = ast_sockaddr_is_ipv4(&ha->netmask);
		if (addr_is_v4 ^ mask_is_v4) {
			ast_log(LOG_WARNING, "Address and mask are not using same address scheme.\n");
			ast_free_ha(ha);
			return ret;
		}
	} else if (parse_cidr_mask(&ha->netmask, addr_is_v4, mask)) {
		ast_log(LOG_WARNING, "Invalid CIDR netmask: %s\n", mask);
		ast_free_ha(ha);
		return ret;
	}

	if (apply_netmask(&ha->addr, &ha->netmask, &ha->addr)) {
		/* This shouldn't happen because ast_sockaddr_parse would
		 * have failed much earlier on an unsupported address scheme
		 */
		ast_log(LOG_WARNING, "Unable to apply netmask %s to address %s\n",
			ast_sockaddr_to_str(mask_buf, sizeof(mask_buf), &ha->netmask),
			ast_sockaddr_to_str(addr_buf, sizeof(addr_buf), &ha->addr));
		ast_free_ha(ha);
		return ret;
	}

	ha->sense = strncasecmp(sense, "p", 1) ? AST_SENSE_DENY : AST_SENSE_ALLOW;

	ha->next = NULL;
	if (prev) {
		prev->next = ha;
	} else {
		ret = ha;
	}

	return ret;
}

int ast_apply_ha_default(struct ast_ha *ha, struct ast_sockaddr *addr,
    int def_sense)
{
	int res = def_sense;
	const struct ast_ha *current_ha;

	for (current_ha = ha; current_ha; current_ha = current_ha->next) {
		struct ast_sockaddr result;
		struct ast_sockaddr mapped_addr;
		const struct ast_sockaddr *addr_to_use;
		if (ast_sockaddr_is_ipv4(&current_ha->addr)) {
			if (ast_sockaddr_is_ipv6(addr)) {
				if (ast_sockaddr_is_ipv4_mapped(addr)) {
					/* IPv4 ACLs apply to IPv4-mapped addresses */
					ast_sockaddr_ipv4_mapped(addr, &mapped_addr);
					addr_to_use = &mapped_addr;
				} else {
					/* An IPv4 ACL does not apply to an IPv6 address */
					continue;
				}
			} else {
				/* Address is IPv4 and ACL is IPv4. No biggie */
				addr_to_use = addr;
			}
		} else {
			if (ast_sockaddr_is_ipv6(addr) && !ast_sockaddr_is_ipv4_mapped(addr)) {
				addr_to_use = addr;
			} else {
				/* Address is IPv4 or IPv4 mapped but ACL is IPv6. Skip */
				continue;
			}
		}

		/* For each rule, if this address and the netmask = the net address
		   apply the current rule */
		if (apply_netmask(addr_to_use, &current_ha->netmask, &result)) {
			/* Unlikely to happen since we know the address to be IPv4 or IPv6 */
			continue;
		}
		if (!ast_sockaddr_cmp_addr(&result, &current_ha->addr)) {
			res = current_ha->sense;
		}
	}
	return res;
}

int ast_apply_ha(struct ast_ha *ha, struct ast_sockaddr *addr)
{
	/* Start optimistic */
	return ast_apply_ha_default(ha, addr, AST_SENSE_ALLOW);
}

int ast_get_next_srv_ip(struct srv_context **context, 
	struct ast_sockaddr *addr, const char *obproxy, int min_srv_ttl, int def_port)
{
	const char *thost;
	char service[MAXHOSTNAMELEN];
	int ret, ttl = 0;
	unsigned short tport;

	snprintf(service, sizeof(service), "_sip._udp.%s", obproxy);

	if ((ret = ast_srv_lookup(context, service, min_srv_ttl, &thost, &tport,
		&ttl)))
	{
		if (h_errno != HOST_NOT_FOUND && h_errno != NO_DATA)
			return ret;
		ast_log(LOG_DEBUG, "Can't get valid SRV entry, resolving as target\n");
		thost = obproxy;
		tport = def_port;
	}

	/* Resolve IP address from found host */
	if (ast_sockaddr_resolve_first_af(addr, thost, PARSE_PORT_FORBID,
		get_address_family_filter(addr))) {
			ast_log(LOG_WARNING, "Can't resolve host %s\n", thost);
			return -1;
	}

	if (addr->ttl && addr->ttl < ttl)
		ast_srv_set_ttl(context, addr->ttl);
	else if (ttl)
		addr->ttl = ttl; 

	if (!ast_sockaddr_port(addr))
		ast_sockaddr_set_port(addr, tport);

	return 0;
}

int ast_get_ip_or_srv(struct ast_sockaddr *addr, const char *value, const char *service)
{
	char srv[256];
	char host[256];
	int srv_ret = 0;
	int tportno;

	if (ast_sockaddr_parse(addr, value, 0))
		return 0;

	if (service) {
		snprintf(srv, sizeof(srv), "%s.%s", service, value);
		if ((srv_ret = ast_get_srv(NULL, host, sizeof(host), &tportno, srv)) > 0) {
			value = host;
		}
	}

	if (ast_sockaddr_resolve_first_af(addr, value, PARSE_PORT_FORBID,
		get_address_family_filter(addr))) {
			return -1;
	}

	if (srv_ret > 0)
		ast_sockaddr_set_port(addr, tportno);

	return 0;
}

int ast_str2tos(const char *value, int *tos)
{
	int fval;
	if (sscanf(value, "%i", &fval) == 1)
		*tos = fval & 0xff;
	else if (!strcasecmp(value, "lowdelay"))
		*tos = IPTOS_LOWDELAY;
	else if (!strcasecmp(value, "throughput"))
		*tos = IPTOS_THROUGHPUT;
	else if (!strcasecmp(value, "reliability"))
		*tos = IPTOS_RELIABILITY;
	else if (!strcasecmp(value, "mincost"))
		*tos = IPTOS_MINCOST;
	else if (!strcasecmp(value, "none"))
		*tos = 0;
	else
		return -1;
	return 0;
}

int ast_get_ip(struct ast_sockaddr *addr, const char *value)
{
	return ast_get_ip_or_srv(addr, value, NULL);
}

/* iface is the interface (e.g. eth0); address is the return value */
int ast_lookup_iface(char *iface, struct in_addr *address) 
{
	int mysock, res = 0;
	struct my_ifreq ifreq;

	memset(&ifreq, 0, sizeof(ifreq));
	ast_copy_string(ifreq.ifrn_name, iface, sizeof(ifreq.ifrn_name));

	mysock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	res = ioctl(mysock, SIOCGIFADDR, &ifreq);

	close(mysock);
	if (res < 0) {
		ast_log(LOG_WARNING, "Unable to get IP of %s: %s\n", iface, strerror(errno));
		memcpy((char *)address, (char *)&__ourip, sizeof(__ourip));
		return -1;
	} else {
		memcpy((char *)address, (char *)&ifreq.ifru_addr.sin_addr, sizeof(ifreq.ifru_addr.sin_addr));
		return 0;
	}
}

int ast_ouraddrfor(struct ast_sockaddr *them, struct ast_sockaddr *us)
{
	int port;
	int s;

	port = ast_sockaddr_port(us);

	if ((s = socket(ast_sockaddr_is_ipv6(them) ? AF_INET6 : AF_INET,
			SOCK_DGRAM, 0)) < 0) {
		ast_log(LOG_ERROR, "Cannot create socket\n");
		return -1;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	{
	    int so_mark = ast_get_so_mark();
   	    setsockopt(s, SOL_SOCKET, SO_MARK, &so_mark, sizeof(so_mark));
	}
#endif	
	if (ast_connect(s, them)) {
		ast_log(LOG_WARNING, "Cannot connect\n");
		close(s);
		return -1;
	}
	if (ast_getsockname(s, us)) {
		ast_log(LOG_WARNING, "Cannot get socket name\n");
		close(s);
		return -1;
	}
	close(s);

	ast_sockaddr_set_port(us, port);

	return 0;
}

int ast_find_ourip(struct ast_sockaddr *ourip, const struct ast_sockaddr *bindaddr,
    struct ast_sockaddr *localip)
{
	char ourhost[MAXHOSTNAMELEN] = "";
	struct ast_sockaddr root;

	/* just use the bind address if it is nonzero */
	if (!ast_sockaddr_is_any(bindaddr)) {
		ast_sockaddr_copy(ourip, bindaddr);
		return 0;
	}

	if (!ast_sockaddr_is_any(localip))
	{
		ast_sockaddr_copy(ourip, localip);
		return 0;
	}

	/* try to use our hostname */
	if (gethostname(ourhost, sizeof(ourhost) - 1)) {
		ast_log(LOG_WARNING, "Unable to get hostname\n");
	} else {
		if (!ast_sockaddr_resolve_first_af(ourip, ourhost, PARSE_PORT_FORBID,
			get_address_family_filter(bindaddr))) {
				return 0;
		}
	}

	/* A.ROOT-SERVERS.NET. */
	if (!ast_sockaddr_resolve_first_af(&root, "A.ROOT-SERVERS.NET", PARSE_PORT_FORBID,
		get_address_family_filter(bindaddr)) && !ast_ouraddrfor(&root, ourip)) {
			return 0;
	}

	return -1;
}

