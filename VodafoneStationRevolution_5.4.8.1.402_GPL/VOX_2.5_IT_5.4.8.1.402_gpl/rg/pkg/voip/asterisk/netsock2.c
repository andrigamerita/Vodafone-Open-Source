/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 2010, Digium, Inc.
 *
 * Viagénie <asteriskv6@viagenie.ca>
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
 * \brief Network socket handling
 *
 * \author Viagénie <asteriskv6@viagenie.ca>
 */

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision$")

#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <util/rg_resolv.h>

#include "asterisk/config.h"
#include "asterisk/netsock2.h"
#include "asterisk/utils.h"
#if 0
#include "asterisk/threadstorage.h"
#endif
#include "asterisk/channel.h"

int ast_sockaddr_ipv4_mapped(const struct ast_sockaddr *addr, struct ast_sockaddr *ast_mapped)
{
	const struct sockaddr_in6 *sin6;
	struct sockaddr_in sin4;

	if (!ast_sockaddr_is_ipv6(addr)) {
		return 0;
	}

	if (!ast_sockaddr_is_ipv4_mapped(addr)) {
		return 0;
	}

	sin6 = (const struct sockaddr_in6*)&addr->ss;

	memset(&sin4, 0, sizeof(sin4));
	sin4.sin_family = AF_INET;
	sin4.sin_port = sin6->sin6_port;
	sin4.sin_addr.s_addr = ((uint32_t *)&sin6->sin6_addr)[3];

	ast_sockaddr_from_sin(ast_mapped, &sin4);

	return 1;
}

#if 0
AST_THREADSTORAGE(ast_sockaddr_stringify_buf);

char *ast_sockaddr_stringify_fmt(const struct ast_sockaddr *sa, int format)
{
	struct ast_sockaddr sa_ipv4;
	const struct ast_sockaddr *sa_tmp;
	char host[NI_MAXHOST];
	char port[NI_MAXSERV];
	struct ast_str *str;
	int e;
	static const size_t size = sizeof(host) - 1 + sizeof(port) - 1 + 4;


	if (ast_sockaddr_isnull(sa)) {
		return "(null)";
	}

	if (!(str = ast_str_thread_get(&ast_sockaddr_stringify_buf, size))) {
		return "";
	}

	if (ast_sockaddr_ipv4_mapped(sa, &sa_ipv4)) {
		sa_tmp = &sa_ipv4;
	} else {
		sa_tmp = sa;
	}

	if ((e = getnameinfo((struct sockaddr *)&sa_tmp->ss, sa->len,
			     format & AST_SOCKADDR_STR_ADDR ? host : NULL,
			     format & AST_SOCKADDR_STR_ADDR ? sizeof(host) : 0,
			     format & AST_SOCKADDR_STR_PORT ? port : 0,
			     format & AST_SOCKADDR_STR_PORT ? sizeof(port): 0,
			     NI_NUMERICHOST | NI_NUMERICSERV))) {
		ast_log(LOG_ERROR, "getnameinfo(): %s\n", gai_strerror(e));
		return "";
	}

	switch (format)  {
	case AST_SOCKADDR_STR_DEFAULT:
		ast_str_set(&str, 0, sa_tmp->ss.ss_family == AF_INET6 ?
				"[%s]:%s" : "%s:%s", host, port);
		break;
	case AST_SOCKADDR_STR_ADDR:
		ast_str_set(&str, 0, "%s", host);
		break;
	case AST_SOCKADDR_STR_HOST:
		ast_str_set(&str, 0,
			    sa_tmp->ss.ss_family == AF_INET6 ? "[%s]" : "%s", host);
		break;
	case AST_SOCKADDR_STR_PORT:
		ast_str_set(&str, 0, "%s", port);
		break;
	default:
		ast_log(LOG_ERROR, "Invalid format\n");
		return "";
	}

	return ast_str_buffer(str);
}
#endif

int static _ast_sockaddr_parse(char *str, char **host, char **port, int flags)
{
	char *s = str;

#if 0
	ast_debug(5, "Splitting '%s' gives...\n", str);
#endif

	*host = NULL;
	*port = NULL;

	if (ast_strlen_zero(s)) {
		ast_log(LOG_WARNING, "cannot parse, string is empty.\n");
		return 0;
	}

	if (*s == '[') {
		*host = ++s;
		for (; *s && *s != ']'; ++s) {
		}
		if (*s == ']') {
			*s++ = '\0';
		}
		if (*s == ':') {
			*port = s + 1;
		}
	} else {
		*host = s;
		for (; *s; ++s) {
			if (*s == ':') {
				if (*port) {
					*port = NULL;
					break;
				} else {
					*port = s;
				}
			}
		}
		if (*port) {
			**port = '\0';
			++*port;
		}
	}
#if 0
	ast_debug(5, "...host '%s' and port '%s'.\n", *host, *port);
#endif

	switch (flags & PARSE_PORT_MASK) {
	case PARSE_PORT_IGNORE:
		*port = NULL;
		break;
	case PARSE_PORT_REQUIRE:
		if (*port == NULL) {
			ast_log(LOG_WARNING, "missing port\n");
			return 0;
		}
		break;
	case PARSE_PORT_FORBID:
		if (*port != NULL) {
			ast_log(LOG_WARNING, "port disallowed\n");
			return 0;
		}
		break;
	}

	return 1;
}

int ast_sockaddr_parse(struct ast_sockaddr *addr, const char *str, int flags)
{
	struct addrinfo hints;
	struct addrinfo	*res;
	char *s;
	char *host;
	char *port;
	int	e;

	s = ast_strdupa(str);
	if (!_ast_sockaddr_parse(s, &host, &port, flags)) {
		return 0;
	}

	memset(&hints, 0, sizeof(hints));
	/* Hint to get only one entry from getaddrinfo */
	hints.ai_socktype = SOCK_DGRAM;

	hints.ai_flags = AI_NUMERICHOST;
	if ((e = getaddrinfo(host, port, &hints, &res))) {
		ast_log(LOG_ERROR, "getaddrinfo(\"%s\", \"%s\", ...): %s\n",
			host, S_OR(port, "(null)"), gai_strerror(e));
		return 0;
	}

	/*
	 * I don't see how this could be possible since we're not resolving host
	 * names. But let's be careful...
	 */
	if (res->ai_next != NULL) {
		ast_log(LOG_WARNING, "getaddrinfo() returned multiple "
			"addresses. Ignoring all but the first.\n");
	}

	addr->len = res->ai_addrlen;
	addr->ttl = 0;
	addr->timestamp = time(NULL);
	memcpy(&addr->ss, res->ai_addr, addr->len);

	freeaddrinfo(res);

	return 1;
}

int ast_sockaddr_parse_addr(struct ast_sockaddr *addr, const char *str)
{
	int ret;
	uint16_t port = ast_sockaddr_port(addr);

	if ((ret = ast_sockaddr_parse(addr, str, 0))) {
		ast_sockaddr_set_port(addr, port);
	}

	return ret;
}

static int ast_sockaddr_resolve(struct ast_sockaddr **addrs, const char *str,
			 int flags, int family, int *ttl)
{
	struct addrinfo hints, *res, *ai;
	char *s, *host, *port;
	int	e, i, res_cnt;

	s = ast_strdupa(str);
	if (!_ast_sockaddr_parse(s, &host, &port, flags)) {
		return 0;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_DGRAM;

	if ((e = rg_getaddrinfo(host, port, &hints, &res, ast_get_so_mark(), ttl))) {
		ast_log(LOG_WARNING, "getaddrinfo(\"%s\", \"%s\", ...): %s\n",
			host, S_OR(port, "(null)"), gai_strerror(e));
		return 0;
	}

	res_cnt = 0;
	for (ai = res; ai; ai = ai->ai_next) {
		res_cnt++;
	}

	if ((*addrs = malloc(res_cnt * sizeof(struct ast_sockaddr))) == NULL) {
		res_cnt = 0;
		goto cleanup;
	}

	i = 0;
	for (ai = res; ai; ai = ai->ai_next) {
		(*addrs)[i].len = ai->ai_addrlen;
		memcpy(&(*addrs)[i].ss, ai->ai_addr, ai->ai_addrlen);
		++i;
	}

cleanup:
	rg_freeaddrinfo(res);
	return res_cnt;
}

int ast_sockaddr_resolve_first_af(struct ast_sockaddr *addr,
				  const char* name, int flag, int family)
{
	struct ast_sockaddr *addrs;
	int addrs_cnt, ttl = 0;

	addrs_cnt = ast_sockaddr_resolve(&addrs, name, flag, family, &ttl);
	if (addrs_cnt <= 0) {
		return 1;
	}

	/* Use the first address only */
	ast_sockaddr_copy(addr, &addrs[0]);
	addr->ttl = ttl;
	addr->timestamp = time(NULL);

	free(addrs);
	return 0;
}

int get_address_family_filter(const struct ast_sockaddr *addr)
{
	if (ast_sockaddr_is_ipv6(addr) && ast_sockaddr_is_any(addr)) {
		return AF_INET6;
	}

	return addr->ss.ss_family;
}

int ast_sockaddr_is_ttl_valid(struct ast_sockaddr *addr)
{
	int passed_time = time(NULL) - addr->timestamp;

	return addr->ttl ? passed_time < addr->ttl : 1;
}

int ast_sockaddr_cmp(const struct ast_sockaddr *a, const struct ast_sockaddr *b)
{
	const struct ast_sockaddr *a_tmp, *b_tmp;
	struct ast_sockaddr ipv4_mapped;

	a_tmp = a;
	b_tmp = b;

	if (a_tmp->len != b_tmp->len) {
		if (ast_sockaddr_ipv4_mapped(a, &ipv4_mapped)) {
			a_tmp = &ipv4_mapped;
		} else if (ast_sockaddr_ipv4_mapped(b, &ipv4_mapped)) {
			b_tmp = &ipv4_mapped;
		}
	}

	if (a_tmp->len < b_tmp->len) {
		return -1;
	} else if (a_tmp->len > b_tmp->len) {
		return 1;
	}

	return memcmp(&a_tmp->ss, &b_tmp->ss, a_tmp->len);
}

int ast_sockaddr_cmp_addr(const struct ast_sockaddr *a, const struct ast_sockaddr *b)
{
	const struct ast_sockaddr *a_tmp, *b_tmp;
	struct ast_sockaddr ipv4_mapped;
	const struct in_addr *ip4a, *ip4b;
	const struct in6_addr *ip6a, *ip6b;
	int ret = -1;

	a_tmp = a;
	b_tmp = b;

	if (a_tmp->len != b_tmp->len) {
		if (ast_sockaddr_ipv4_mapped(a, &ipv4_mapped)) {
			a_tmp = &ipv4_mapped;
		} else if (ast_sockaddr_ipv4_mapped(b, &ipv4_mapped)) {
			b_tmp = &ipv4_mapped;
		}
	}

	if (a->len < b->len) {
		ret = -1;
	} else if (a->len > b->len) {
		ret = 1;
	}

	switch (a_tmp->ss.ss_family) {
	case AF_INET:
		ip4a = &((const struct sockaddr_in*)&a_tmp->ss)->sin_addr;
		ip4b = &((const struct sockaddr_in*)&b_tmp->ss)->sin_addr;
		ret = memcmp(ip4a, ip4b, sizeof(*ip4a));
		break;
	case AF_INET6:
		ip6a = &((const struct sockaddr_in6*)&a_tmp->ss)->sin6_addr;
		ip6b = &((const struct sockaddr_in6*)&b_tmp->ss)->sin6_addr;
		ret = memcmp(ip6a, ip6b, sizeof(*ip6a));
		break;
	}
	return ret;
}

uint16_t ast_sockaddr_port(const struct ast_sockaddr *addr)
{
	if (addr->ss.ss_family == AF_INET) {
		return ntohs(((struct sockaddr_in *)&addr->ss)->sin_port);
	} else if (addr->ss.ss_family == AF_INET6) {
		return ntohs(((struct sockaddr_in6 *)&addr->ss)->sin6_port);
	}
	ast_log(LOG_DEBUG, "Not an IPv4 nor IPv6 address, cannot get port.\n");
	return 0;
}

void ast_sockaddr_set_port(struct ast_sockaddr *addr, uint16_t port)
{
	if (addr->ss.ss_family == AF_INET) {
		((struct sockaddr_in *)&addr->ss)->sin_port = htons(port);
	} else if (addr->ss.ss_family == AF_INET6) {
		((struct sockaddr_in6 *)&addr->ss)->sin6_port = htons(port);
	} else {
		ast_log(LOG_DEBUG,
			"Not an IPv4 nor IPv6 address, cannot set port.\n");
	}
}

uint32_t ast_sockaddr_ipv4(const struct ast_sockaddr *addr)
{
	const struct sockaddr_in *sin = (struct sockaddr_in *)&addr->ss;
	return ntohl(sin->sin_addr.s_addr);
}

int ast_sockaddr_is_ipv4(const struct ast_sockaddr *addr)
{
	return addr->ss.ss_family == AF_INET &&
	    addr->len == sizeof(struct sockaddr_in);
}

int ast_sockaddr_is_ipv4_mapped(const struct ast_sockaddr *addr)
{
	const struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&addr->ss;
	return addr->len && IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr);
}

int ast_sockaddr_is_ipv6(const struct ast_sockaddr *addr)
{
	return addr->ss.ss_family == AF_INET6 &&
	    addr->len == sizeof(struct sockaddr_in6);
}

int ast_sockaddr_is_any(const struct ast_sockaddr *addr)
{
	return (ast_sockaddr_is_ipv4(addr) &&
		((const struct sockaddr_in *)&addr->ss)->sin_addr.s_addr ==
		INADDR_ANY) ||
	    (ast_sockaddr_is_ipv6(addr) &&
	     IN6_IS_ADDR_UNSPECIFIED(&((const struct sockaddr_in6 *)&addr->ss)->sin6_addr));
}

unsigned long ast_sockaddr_hash(const struct ast_sockaddr *addr)
{
	/*
	 * For IPv4, return the IP address as-is. For IPv6, return the last 32
	 * bits.
	 */
	switch (addr->ss.ss_family) {
	case AF_INET:
		return ((const struct sockaddr_in *)&addr->ss)->sin_addr.s_addr;
	case AF_INET6:
		return ((uint32_t *)&((const struct sockaddr_in6 *)&addr->ss)->sin6_addr)[3];
	default:
		ast_log(LOG_ERROR, "Unknown address family '%d'.\n",
			addr->ss.ss_family);
		return 0;
	}
}

int ast_accept(int sockfd, struct ast_sockaddr *addr)
{
	addr->len = sizeof(addr->ss);
	return accept(sockfd, (struct sockaddr *)&addr->ss, &addr->len);
}

int ast_bind(int sockfd, const struct ast_sockaddr *addr)
{
	return bind(sockfd, (const struct sockaddr *)&addr->ss, addr->len);
}

int ast_connect(int sockfd, const struct ast_sockaddr *addr)
{
	return connect(sockfd, (const struct sockaddr *)&addr->ss, addr->len);
}

int ast_getsockname(int sockfd, struct ast_sockaddr *addr)
{
	addr->len = sizeof(addr->ss);
	return getsockname(sockfd, (struct sockaddr *)&addr->ss, &addr->len);
}

ssize_t ast_recvfrom(int sockfd, void *buf, size_t len, int flags,
		     struct ast_sockaddr *src_addr)
{
	src_addr->len = sizeof(src_addr->ss);
	return recvfrom(sockfd, buf, len, flags,
			(struct sockaddr *)&src_addr->ss, &src_addr->len);
}

ssize_t ast_sendto(int sockfd, const void *buf, size_t len, int flags,
		   const struct ast_sockaddr *dest_addr)
{
	return sendto(sockfd, buf, len, flags,
		      (const struct sockaddr *)&dest_addr->ss, dest_addr->len);
}

int ast_set_qos(int sockfd, int tos, int cos, const char *desc)
{
	int res;

	if ((res = setsockopt(sockfd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)))) {
		ast_log(LOG_WARNING, "Unable to set %s TOS to %d (may be you have no "
			"root privileges): %s\n", desc, tos, strerror(errno));
	} else if (tos) {
#if 0
		ast_verb(2, "Using %s TOS bits %d\n", desc, tos);
#endif
	}

#ifdef linux
	if (setsockopt(sockfd, SOL_SOCKET, SO_PRIORITY, &cos, sizeof(cos))) {
		ast_log(LOG_WARNING, "Unable to set %s CoS to %d: %s\n", desc, cos,
			strerror(errno));
	} else if (cos) {
#if 0
		ast_verb(2, "Using %s CoS mark %d\n", desc, cos);
#endif
	}
#endif

	return res;
}

int ast_sockaddr_to_sin(const struct ast_sockaddr *addr,
			struct sockaddr_in *sin)
{
	if (ast_sockaddr_isnull(addr)) {
		memset(sin, 0, sizeof(*sin));
		return 1;
	}

	if (addr->len != sizeof(*sin)) {
		ast_log(LOG_ERROR, "Bad address cast to IPv4\n");
		return 0;
	}

	if (addr->ss.ss_family != AF_INET) {
		ast_log(LOG_DEBUG, "Address family is not AF_INET\n");
	}

	*sin = *(struct sockaddr_in *)&addr->ss;
	return 1;
}

void ast_sockaddr_from_sin(struct ast_sockaddr *addr, const struct sockaddr_in *sin)
{
	memcpy(&addr->ss, sin, sizeof(*sin));

	if (addr->ss.ss_family != AF_INET) {
		ast_log(LOG_DEBUG, "Address family is not AF_INET\n");
	}

	addr->len = sizeof(*sin);
}

void ast_sockaddr_from_storage(struct ast_sockaddr *addr, const struct sockaddr_storage *ss)
{
	switch (ss->ss_family) {
	case AF_INET:
		addr->ss = *ss;
		addr->len = sizeof(struct sockaddr_in);
		break;
	case AF_INET6:
		addr->ss = *ss;
		addr->len = sizeof(struct sockaddr_in6);
		break;
	case AF_UNSPEC:
		addr->len = 0;
		break;
	default:
		ast_log(LOG_WARNING, "Wrong address family\n");
		break;
	}
}

const char *_ast_sockaddr_to_str(char *buf, int buf_size, struct ast_sockaddr *addr,
    int need_brackets)
{
	const char *ret;

	if (ast_sockaddr_isnull(addr))
		return "(null)";

	switch(addr->ss.ss_family) {
	case AF_INET:
		if (!(ret = inet_ntop(AF_INET,
			&((struct sockaddr_in *)&addr->ss)->sin_addr, buf,
			buf_size))) {
				ast_log(LOG_WARNING, "Failed to convert IPv4 address\n");
				return "";
		}
		break;
	case AF_INET6:
		if (!(ret = inet_ntop(AF_INET6,
			&((struct sockaddr_in6 *)&addr->ss)->sin6_addr, buf,
			buf_size))) {
				ast_log(LOG_WARNING, "Failed to convert IPv6 address\n");
				return "";
		}
		if (need_brackets) {
		 	char *s = ast_strdupa(buf);

			snprintf(buf, buf_size, "[%s]", s);
		}
		break;
	default:
		ast_log(LOG_WARNING, "Wrong address family\n");
		return "";
	}

	return ret;
}

const char *ast_sockaddr_to_str(char *buf, int buf_size, struct ast_sockaddr *addr)
{
	return _ast_sockaddr_to_str(buf, buf_size, addr, 1);
}

const char *ast_sockaddr_to_str_nowrap(char *buf, int buf_size, struct ast_sockaddr *addr)
{
	return _ast_sockaddr_to_str(buf, buf_size, addr, 0);
}
