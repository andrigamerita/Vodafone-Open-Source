#include "dnsmasq.h"

#include <linux/rtnetlink.h>

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000000L
#endif

#define NDISC_TIMEOUT 5

static int ndisc_timeout(struct timespec *diff,
    struct timespec *now, struct timespec *end)
{
    clock_gettime(CLOCK_MONOTONIC, now);

    diff->tv_sec = end->tv_sec - now->tv_sec;
    diff->tv_nsec = end->tv_nsec - now->tv_nsec;

    if (diff->tv_nsec < 0)
    {
	diff->tv_sec--;
	diff->tv_nsec += NSEC_PER_SEC;
    }
    return (diff->tv_sec < 0);
}

int ndisc_event_open(void)
{
    int fd;
    socklen_t addr_len = sizeof(struct sockaddr_nl);
    struct sockaddr_nl addr = {
	.nl_family = AF_NETLINK,
	.nl_groups = RTMGRP_NEIGH
    };

    if ((fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
	goto Error;
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	goto Error;
    if (getsockname(fd, (struct sockaddr*)&addr, &addr_len) < 0)
	goto Error;
    return fd;
Error:
    if (fd != -1)
	close(fd);
    return -1;
}

int ndisc_event_close(int nl_fd)
{
    if (nl_fd != -1)
	close(nl_fd);
    return 0;
}

/* OpenRG patches are requred for linux kernel */
#ifndef RTM_RESOLVNEIGH
#define RTM_RESOLVNEIGH (RTM_GETNEIGH + 1)
#endif

static int ndisc_resolve_request(int nl_fd, int ifindex, int family,
    void *ipdata, int iplen)
{
    struct {
	struct {
	    struct nlmsghdr nlh;
	    struct ndmsg ndm;
	} h;
	char attr[32];
    } req;

    struct sockaddr_nl nladdr = {.nl_family = AF_NETLINK};

    memset(&req, 0, sizeof(req));
    req.h.nlh.nlmsg_len = NLMSG_ALIGN(sizeof(req.h));
    req.h.nlh.nlmsg_type = RTM_RESOLVNEIGH;
    req.h.nlh.nlmsg_flags = NLM_F_REQUEST;
    req.h.nlh.nlmsg_pid = 0;
    req.h.nlh.nlmsg_seq = 0; /*query.dump = ++query.seq;*/
    req.h.ndm.ndm_family = family;
    req.h.ndm.ndm_state = 0;
    req.h.ndm.ndm_ifindex = ifindex;

    struct rtattr *rta = (struct rtattr*)(req.attr);
    rta->rta_type = NDA_DST;
    rta->rta_len = RTA_LENGTH(iplen);
    memcpy(RTA_DATA(rta), ipdata, iplen);
    req.h.nlh.nlmsg_len += RTA_LENGTH(iplen);

    sendto(nl_fd, (void*)&req, req.h.nlh.nlmsg_len, 0,
	(struct sockaddr*)&nladdr, sizeof(nladdr));

    return 0;
}

#ifndef NDA_RTA
#define NDA_RTA(r)  ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#endif

static int ndisc_resolve_wait(int nl_fd, union mysockaddr *addr, unsigned char *mac)
{
    static char buf[2048];
    struct iovec iov = {.iov_base = buf};
    struct sockaddr_nl nladdr;
    struct msghdr msg = {
	.msg_name = &nladdr,
	.msg_namelen = sizeof(nladdr),
	.msg_iov = &iov,
	.msg_iovlen = 1,
    };
    struct timespec now, end, wait;

    clock_gettime(CLOCK_MONOTONIC, &now);
    end.tv_sec = now.tv_sec + NDISC_TIMEOUT;
    end.tv_nsec = now.tv_nsec;

    while (!ndisc_timeout(&wait, &now, &end))
    {
	int msglen;
	struct nlmsghdr *h;
	struct pollfd fds[1] = {{ .fd = nl_fd, .events = POLLIN, }};

	if (ppoll(fds, 1, &wait, NULL) <= 0)
	    break;

	if (!(fds[0].revents & POLLIN))
	    continue;

	iov.iov_len = sizeof(buf);

	if ((msglen = recvmsg(nl_fd, &msg, 0)) <= 0)
	{
	    my_syslog(LOG_ERR, _("netlink receives error: %s"), strerror(errno));

	    if ((msglen < 0) && (errno == EINTR || errno == EAGAIN))
		continue;
	    break;
	}

	h = (struct nlmsghdr *)buf;
	while (NLMSG_OK(h, msglen))
	{
	    struct rtattr *r;
	    struct ndmsg *n;
	    int len, mac_len = 0;
	    unsigned char *addr_ptr, *mac_ptr;

	    if (nladdr.nl_pid != 0)
		goto skip_it;
	    if (h->nlmsg_type == NLMSG_ERROR)
	    {
		if (h->nlmsg_len < NLMSG_LENGTH(sizeof(struct nlmsgerr)))
		{
		    my_syslog(LOG_ERR, _("netlink returns truncated error"));
		    return 0;
		}
		goto skip_it;
	    }
	    if (h->nlmsg_type != RTM_NEWNEIGH)
		goto skip_it;

	    n = NLMSG_DATA(h);
	    if ((n->ndm_type != RTN_UNICAST) ||
		(n->ndm_family != addr->sa.sa_family) ||
		(n->ndm_state & (NUD_NOARP | NUD_INCOMPLETE | NUD_FAILED)))
		goto skip_it;

	    len = h->nlmsg_len - NLMSG_LENGTH(sizeof(*n));
	    addr_ptr = mac_ptr = NULL;
	    r = NDA_RTA(n);
	    while (RTA_OK(r, len))
	    {
		switch (r->rta_type)
		{
		case NDA_DST: addr_ptr = RTA_DATA(r); break;
		case NDA_LLADDR: mac_ptr = RTA_DATA(r);
				 mac_len = RTA_PAYLOAD(r);
				 break;
		default: break;
		}
		r = RTA_NEXT(r, len);
	    }
	    if (!addr_ptr || !mac_ptr || mac_len > DHCP_CHADDR_MAX)
		goto skip_it;
	    if (addr->sa.sa_family == AF_INET)
	    {
		if(!memcmp(addr_ptr, &addr->in.sin_addr,
		    sizeof(struct in_addr)))
		{
		    memcpy(mac, mac_ptr, mac_len);
		    return mac_len;
		}
	    }
	    else if (addr->sa.sa_family == AF_INET6)
	    {
		if(!memcmp(addr_ptr, &addr->in6.sin6_addr,
		    sizeof(struct in6_addr)))
		{
		    memcpy(mac, mac_ptr, mac_len);
		    return mac_len;
		}
	    }
skip_it:
	    h = NLMSG_NEXT(h, msglen);
	}
    }
    return 0;
}

int ndisc_resolve(int nl_fd, int ifindex, union mysockaddr *addr, unsigned char *mac)
{
    if (addr->sa.sa_family == AF_INET)
    {
	ndisc_resolve_request(nl_fd, ifindex, AF_INET, &addr->in.sin_addr,
	    sizeof(struct in_addr));
    }
    else if (addr->sa.sa_family == AF_INET6)
    {
	ndisc_resolve_request(nl_fd, ifindex, AF_INET6, &addr->in6.sin6_addr,
	    sizeof(struct in6_addr));
    }
    return ndisc_resolve_wait(nl_fd, addr, mac);
}

