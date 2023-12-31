/*
 * receive code
 * Copyright (C) 1996, 1997  John Ioannidis.
 * Copyright (C) 1998, 1999, 2000, 2001  Richard Guy Briggs.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <linux/version.h>

#define __NO_VERSION__
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */

#define IPSEC_KLIPS1_COMPAT 1
#include "ipsec_param.h"

#ifdef MALLOC_SLAB
# include <linux/slab.h> /* kmalloc() */
#else /* MALLOC_SLAB */
# include <linux/malloc.h> /* kmalloc() */
#endif /* MALLOC_SLAB */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/interrupt.h> /* mark_bh */

#include <linux/netdevice.h>   /* struct net_device, and other headers */
#include <linux/etherdevice.h> /* eth_type_trans */
#include <linux/ip.h>          /* struct iphdr */
#include <linux/skbuff.h>
#include <net/protocol.h>
#include <freeswan.h>
#ifdef SPINLOCK
# ifdef SPINLOCK_23
#  include <linux/spinlock.h> /* *lock* */
# else /* SPINLOCK_23 */
#  include <asm/spinlock.h> /* *lock* */
# endif /* SPINLOCK_23 */
#endif /* SPINLOCK */
#ifdef NET_21
# include <asm/uaccess.h>
# include <linux/in6.h>
# define proto_priv cb
#endif /* NET21 */
#include <asm/checksum.h>
#include <net/ip.h>

#include "radij.h"
#include "ipsec_encap.h"
#include "ipsec_sa.h"

#include "ipsec_radij.h"
#include "ipsec_netlink.h"
#include "ipsec_xform.h"
#include "ipsec_tunnel.h"
#include "ipsec_rcv.h"
#if defined(CONFIG_IPSEC_ESP) || defined(CONFIG_IPSEC_AH)
# include "ipsec_ah.h"
#endif /* defined(CONFIG_IPSEC_ESP) || defined(CONFIG_IPSEC_AH) */
#ifdef CONFIG_IPSEC_ESP
# include "ipsec_esp.h"
#endif /* !CONFIG_IPSEC_ESP */
#ifdef CONFIG_IPSEC_IPCOMP
# include "ipcomp.h"
#endif /* CONFIG_IPSEC_COMP */

#include <pfkeyv2.h>
#include <pfkey.h>

#ifdef CONFIG_IPSEC_NAT_TRAVERSAL
#include <linux/udp.h>
#endif

#include "ipsec_proto.h"
#include "ipsec_alg.h"
#include "ipsec_log.h"
#include "ipsec_rcv_common.h"
#include "ipsec_tunnel_common.h"

#ifdef CONFIG_IPSEC_DEBUG
#include "ipsec_reject_debug.h"

int debug_ah = 0;
int debug_esp = 0;
int debug_rcv = 0;
#endif /* CONFIG_IPSEC_DEBUG */

static inline int os_skb_linearize(struct sk_buff *buf)
{
#if defined(CONFIG_RG_OS_LINUX_26) || defined(CONFIG_RG_OS_LINUX_3X)
    return skb_linearize(buf);
#else
    return skb_linearize(buf, GFP_ATOMIC);
#endif
}

int sysctl_ipsec_inbound_policy_check = 1;

#if defined(CONFIG_IPSEC_ESP) || defined(CONFIG_IPSEC_AH)
__u32 zeroes[AH_AMAX];
#endif /* defined(CONFIG_IPSEC_ESP) || defined(CONFIG_IPSEC_AH) */

/*
 * Check-replay-window routine, adapted from the original 
 * by J. Hughes, from draft-ietf-ipsec-esp-des-md5-03.txt
 *
 *  This is a routine that implements a 64 packet window. This is intend-
 *  ed on being an implementation sample.
 */

DEBUG_NO_STATIC int
ipsec_checkreplaywindow(struct ipsec_sa*tdbp, __u32 seq, char *ipaddr_txt,
    char *buf, int buf_size)
{
	__u32 diff;
	
	if (tdbp->tdb_replaywin == 0)	/* replay shut off */
		return 1;
	if (seq == 0) {
		snprintf(buf, buf_size,
			    "klips_debug:ipsec_checkreplaywindow: "
			    "first or wrapped frame from %s, packet dropped\n",
			    ipaddr_txt);
		return 0;		/* first == 0 or wrapped */
	}

	/* new larger sequence number */
	if (seq > tdbp->tdb_replaywin_lastseq) {
		return 1;		/* larger is good */
	}
	diff = tdbp->tdb_replaywin_lastseq - seq;

	/* too old or wrapped */ /* if wrapped, kill off SA? */
	if (diff >= tdbp->tdb_replaywin) {
		snprintf(buf, buf_size,
			    "klips_debug:ipsec_checkreplaywindow: "
			    "too old frame from %s, packet dropped\n",
			    ipaddr_txt);
		return 0;
	}
	/* this packet already seen */
	if (tdbp->tdb_replaywin_bitmap & (1 << diff)) {
		snprintf(buf, buf_size,
			    "klips_debug:ipsec_checkreplaywindow: "
			    "duplicate frame from %s (sequence %d), "
			    "packet dropped\n", ipaddr_txt, seq);
		/* This will be always printed (ICSA). */
		KLIPS_PRINT(1, "%s", buf);
		return 0;
	}
	return 1;			/* out of order but good */
}

DEBUG_NO_STATIC int
ipsec_updatereplaywindow(struct ipsec_sa*tdbp, __u32 seq)
{
	__u32 diff;
	
	if (tdbp->tdb_replaywin == 0)	/* replay shut off */
		return 1;
	if (seq == 0) 
		return 0;		/* first == 0 or wrapped */

	/* new larger sequence number */
	if (seq > tdbp->tdb_replaywin_lastseq) {
		diff = seq - tdbp->tdb_replaywin_lastseq;

		/* In win, set bit for this pkt */
		if (diff < tdbp->tdb_replaywin)
			tdbp->tdb_replaywin_bitmap =
				(tdbp->tdb_replaywin_bitmap << diff) | 1;
		else
			/* This packet has way larger seq num */
			tdbp->tdb_replaywin_bitmap = 1;

		if(seq - tdbp->tdb_replaywin_lastseq - 1 > tdbp->tdb_replaywin_maxdiff) {
			tdbp->tdb_replaywin_maxdiff = seq - tdbp->tdb_replaywin_lastseq - 1;
		}
		tdbp->tdb_replaywin_lastseq = seq;
		return 1;		/* larger is good */
	}
	diff = tdbp->tdb_replaywin_lastseq - seq;

	/* too old or wrapped */ /* if wrapped, kill off SA? */
	if (diff >= tdbp->tdb_replaywin) {
/*
		if(seq < 0.25*max && tdbp->tdb_replaywin_lastseq > 0.75*max) {
			deltdbchain(tdbp);
		}
*/	
		return 0;
	}
	/* this packet already seen */
	if (tdbp->tdb_replaywin_bitmap & (1 << diff))
		return 0;
	tdbp->tdb_replaywin_bitmap |= (1 << diff);	/* mark as seen */
	return 1;			/* out of order but good */
}

#ifdef CONFIG_IPSEC_ALG_JOINT_ASYNC
static void ipsec_rcv_end_cb(struct ipsec_rcv_state *irs, int status)
{
	struct sk_buff *skb = irs->skb;
	struct iphdr *ipp;
#ifdef CONFIG_IPSEC_DEBUG
	struct ipsec_pkt_info_t pkt_info;
#endif

	int iphlen;
	unsigned char *dat;
	struct ipsec_sa *tdbp = NULL, *stats_tdbp = NULL;
	char sa[SATOA_BUF];
	size_t sa_len;
	char ipaddr_txt[ADDRTOA_BUF];
	struct in_addr ipaddr;
	__u8 proto;

	int len;	/* packet length */
	struct ipsec_sa* tdbprev = NULL;	/* previous SA from outside of packet */
	struct ipsec_sa* tdbnext = NULL;	/* next SA towards inside of packet */

	ipp = (struct iphdr *)skb->data;
	proto = ipp->protocol;
	ipaddr.s_addr = ipp->saddr;
	addrtoa(ipaddr, 0, ipaddr_txt, sizeof(ipaddr_txt));

	iphlen = ipp->ihl << 2;

	/*
	 * Find tunnel control block and (indirectly) call the
	 * appropriate tranform routine. The resulting sk_buf
	 * is a valid IP packet ready to go through input processing.
	 */
        /*
	 * XXX: Not sure it is safe to that like that. it is probably safe to
	 * search for the SA again based on the SPI in case the sa has been
	 * removed by some user application between the ipsec_rcv_decap and
	 * ipsec_rcv_decap_cb.
         *
         */		
	tdbp = irs->ipsp;
	/*
	 * Adjust pointers
	 */
		
	len = skb->len;
	dat = skb->data;

	skb_set_network_header(skb, 0);
	skb_set_transport_header(skb, ip_hdr(skb)->ihl << 2);
		
	memset(&(IPCB(skb)->opt), 0, sizeof(struct ip_options));
		
	ipp = (struct iphdr *)dat;
	ipp->check = 0;
	ipp->check = ip_fast_csum((unsigned char *)dat, iphlen >> 2);

	sa_len = satoa(tdbp->tdb_said, 0, sa, SATOA_BUF);

	KLIPS_PRINT(debug_rcv & DB_RX_PKTRX,
		"klips_debug:ipsec_rcv_end_cb: after <%s%s%s>, SA:%s:\n",
		IPS_XFORM_NAME(tdbp),
		sa_len ? sa : " (error)");
	KLIPS_IP_PRINT(debug_rcv & DB_RX_PKTRX, ipp);
		
#ifdef CONFIG_IPSEC_NAT_TRAVERSAL
	if ((irs->natt_type) && (ipp->protocol != IPPROTO_IPIP)) {
	    ipsec_rcv_natt_correct_tcp_udp_csum(skb, ipp, tdbp);
	}
#endif
	tdbprev = tdbp;
	tdbnext = tdbp->tdb_inext;

	if(sysctl_ipsec_inbound_policy_check) {
		if(tdbnext) {
			if(tdbnext->tdb_onext != tdbp) {
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
					"klips_debug:ipsec_rcv_end_cb: "
					"SA:%s, backpolicy does not agree with fwdpolicy.\n",
					sa_len ? sa : " (error)");
				goto rx_dropped;
			}
			KLIPS_PRINT(debug_rcv,
				"klips_debug:ipsec_rcv_end_cb: "
				"SA:%s, backpolicy agrees with fwdpolicy.\n",
				sa_len ? sa : " (error)");
			if(
				ipp->protocol != IPPROTO_AH  
				&& ipp->protocol != IPPROTO_ESP 
#ifdef CONFIG_IPSEC_IPCOMP
				&& ipp->protocol != IPPROTO_COMP
				&& (tdbnext->tdb_said.proto != IPPROTO_COMP
				|| (tdbnext->tdb_said.proto == IPPROTO_COMP
				&& tdbnext->tdb_inext))
#endif /* CONFIG_IPSEC_IPCOMP */
				&& ipp->protocol != IPPROTO_IPIP
			) {
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
					"klips_debug:ipsec_rcv_end_cb: "
					"packet with incomplete policy dropped, last successful SA:%s.\n",
					sa_len ? sa : " (error)");
				goto rx_dropped;
			}
			KLIPS_PRINT(debug_rcv,
				"klips_debug:ipsec_rcv_end_cb: "
				"SA:%s, Another IPSEC header to process.\n",
				sa_len ? sa : " (error)");
		} else {
			KLIPS_PRINT(debug_rcv,
				"klips_debug:ipsec_rcv_end_cb: "
				"No tdb_inext from this SA:%s.\n",
				sa_len ? sa : " (error)");
		}
	}

	tdbp->ips_life.ipl_bytes.ipl_count += len;
	tdbp->ips_life.ipl_bytes.ipl_last   = len;

	if(!tdbp->ips_life.ipl_usetime.ipl_count) {
		tdbp->ips_life.ipl_usetime.ipl_count = jiffies / HZ;
	}
	tdbp->ips_life.ipl_usetime.ipl_last = jiffies / HZ;
	tdbp->ips_life.ipl_packets.ipl_count += 1;	
		
	if(skb->len < iphlen) {
		PRINTK_REJECT(&pkt_info, KERN_WARNING, "klips_debug: "
			"tried to skb_pull iphlen=%d, %d available.  This should never happen, please report.\n",
			iphlen,
			(int)(skb->len));

		goto rcvleave;
	}

	if(ipp->protocol == IPPROTO_IPIP)
		skb_pull(skb, iphlen);

	skb_set_network_header(skb, 0);
	ipp = ip_hdr(skb);
	skb_set_transport_header(skb, ip_hdr(skb)->ihl << 2);
		
	memset(&(IPCB(skb)->opt), 0, sizeof(struct ip_options));

	skb->protocol = htons(ETH_P_IP);
	skb->ip_summed = 0;
	KLIPS_PRINT(debug_rcv & DB_RX_PKTRX,
		"klips_debug:ipsec_rcv_end_cb: IPIP tunnel stripped.\n");
	KLIPS_IP_PRINT(debug_rcv & DB_RX_PKTRX, ipp);

	if(sysctl_ipsec_inbound_policy_check)
	{
		u32 s_addr, d_addr, tdb_s_ip, tdb_d_ip, tdb_s_to, tdb_d_to;
		#define IS_MATCH(ip, t_ip, t_mask, t_to) \
		    ((t_to) ? (ip) >= (t_ip) && (ip) <= (t_to) : \
		    !(((ip) & (t_mask)) ^ (t_ip)))

		s_addr = ntohl(ipp->saddr);
		tdb_s_ip = ntohl(tdbp->tdb_flow_s.u.v4.sin_addr.s_addr);
		tdb_s_to = ntohl(tdbp->tdb_to_s.u.v4.sin_addr.s_addr);
		d_addr = ntohl(ipp->daddr);
		tdb_d_ip = ntohl(tdbp->tdb_flow_d.u.v4.sin_addr.s_addr);
		tdb_d_to = ntohl(tdbp->tdb_to_d.u.v4.sin_addr.s_addr);
		if (!IS_MATCH(s_addr, tdb_s_ip,
			ntohl(tdbp->tdb_mask_s.u.v4.sin_addr.s_addr),
			tdb_s_to) ||
			!IS_MATCH(d_addr, tdb_d_ip,
			ntohl(tdbp->tdb_mask_d.u.v4.sin_addr.s_addr), tdb_d_to))
		{
			struct in_addr daddr, saddr;
			char saddr_txt[ADDRTOA_BUF], daddr_txt[ADDRTOA_BUF];
			char sflow_txt[SUBNETTOA_BUF], dflow_txt[SUBNETTOA_BUF];
			
			subnettoa(tdbp->tdb_flow_s.u.v4.sin_addr,
				tdbp->tdb_mask_s.u.v4.sin_addr,
				0, sflow_txt, sizeof(sflow_txt));
			subnettoa(tdbp->tdb_flow_d.u.v4.sin_addr,
				tdbp->tdb_mask_d.u.v4.sin_addr,
				0, dflow_txt, sizeof(dflow_txt));
			saddr.s_addr = ipp->saddr;
			daddr.s_addr = ipp->daddr;
			addrtoa(saddr, 0, saddr_txt, sizeof(saddr_txt));
			addrtoa(daddr, 0, daddr_txt, sizeof(daddr_txt));
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
				    "klips_debug:ipsec_rcv_end_cb: "
				    "SA:%s, inner tunnel policy [%s -> %s] does not agree with pkt contents [%s -> %s].\n",
				    sa_len ? sa : " (error)",
				    sflow_txt,
				    dflow_txt,
				    saddr_txt,
				    daddr_txt);
			goto rx_dropped;
		}
		
	}

#ifdef NET_21
	if(irs->stats) {
		irs->stats->rx_bytes += skb->len;
	}
	skb_dst_drop(skb);
	skb->pkt_type = PACKET_HOST;
	if(irs->hard_header_len &&
		(skb_mac_header(skb) != (skb->data - irs->hard_header_len)) &&
		(irs->hard_header_len <= skb_headroom(skb))) {
		/* copy back original MAC header */
		memmove(skb->data - irs->hard_header_len, skb_mac_header(skb), irs->hard_header_len);
		skb_set_mac_header(skb, -irs->hard_header_len);
	}
#endif /* NET_21 */

#ifdef SKB_RESET_NFCT
        nf_conntrack_put(skb->nfct);
        skb->nfct = NULL;
#ifdef CONFIG_NETFILTER_DEBUG
	skb->nf_debug = 0;
#endif /* CONFIG_NETFILTER_DEBUG */
#endif /* SKB_RESET_NFCT */
	KLIPS_PRINT(debug_rcv & DB_RX_PKTRX,
		    "klips_debug:ipsec_rcv_end_cb: netif_rx() called.\n");

	while (stats_tdbp)
	{
	    stats_tdbp->ips_life.orig_bytes += skb->len;
	    stats_tdbp->ips_life.orig_packets++;
	    stats_tdbp = stats_tdbp->ips_inext;
	}

	netif_rx(skb);
	skb = NULL;

	goto rcvleave;

 rx_dropped:
	if (irs->stats)
	    irs->stats->rx_dropped++;
	while (stats_tdbp)
	{
	    stats_tdbp->ips_life.orig_dropped++;
	    stats_tdbp = stats_tdbp->ips_inext;
	}
	goto rcvleave;

 rcvleave:
	kfree(irs);
	if(skb) {
		ipsec_kfree_skb(skb);
	}

	return;
}
#endif

/* Core decapsulation loop for all protocols. */
static void ipsec_rcv_decap(struct ipsec_rcv_state *irs, int status)
{
    	struct ipsec_rcv_state nirs;
	struct sk_buff *skb = irs->skb;
	int after_decrypt = irs->after_decrypt;
	struct iphdr *ipp;
	int authlen = 0;
#ifdef CONFIG_IPSEC_ESP
	struct esp *espp = NULL;
	int esphlen = 0;
#ifdef CONFIG_IPSEC_ENC_3DES
	__u32 iv[ESP_IV_MAXSZ_INT];
#endif /* !CONFIG_IPSEC_ENC_3DES */
#endif /* !CONFIG_IPSEC_ESP */
#ifdef CONFIG_IPSEC_AH
	struct ah *ahp = NULL;
	int ahhlen = 0;
#if defined (CONFIG_IPSEC_AUTH_HMAC_MD5) || defined(CONFIG_IPSEC_AUTH_HMAC_SHA1)
	struct iphdr ipo;
#endif
#endif /* CONFIG_IPSEC_AH */
	unsigned char *authenticator = NULL;
	union {
		MD5_CTX		md5;
		SHA1_CTX	sha1;
	} tctx;
	__u8 hash[AH_AMAX];
#ifdef CONFIG_IPSEC_IPCOMP
	struct ipcomphdr*compp = NULL;
#endif /* CONFIG_IPSEC_IPCOMP */
#ifdef CONFIG_IPSEC_DEBUG
	struct ipsec_pkt_info_t pkt_info;
#endif
	char err_msg[128];

	int iphlen;
	unsigned char *dat;
	struct ipsec_sa *tdbp = NULL, *stats_tdbp = NULL;
	struct sa_id said;
	char sa[SATOA_BUF];
	size_t sa_len;
	char ipaddr_txt[ADDRTOA_BUF];
	int i;
	struct in_addr ipaddr;
	__u8 next_header = 0;
	__u8 proto;
	int lifename_idx = 0;
	char *lifename[] = {
	    "bytes",
	    "addtime",
	    "usetime",
	    "packets"
	};

#ifdef CONFIG_IPSEC_ESP
	int pad = 0, padlen;
#endif /* CONFIG_IPSEC_ESP */
	int ilen;	/* content to be decrypted/authenticated */
	int len;	/* packet length */
	int replay = 0;	/* replay value in AH or ESP packet */
	__u8 *idat;	/* pointer to content to be decrypted/authenticated */
	struct ipsec_sa* tdbprev = NULL;	/* previous SA from outside of packet */
	struct ipsec_sa* tdbnext = NULL;	/* next SA towards inside of packet */
#ifdef CONFIG_IPSEC_ALG
	struct ipsec_alg_enc *ixt_e=NULL;
	struct ipsec_alg_auth *ixt_a=NULL;
	struct ipsec_alg_joint *ixt_j=NULL;
#endif /* CONFIG_IPSEC_ALG */
	int tdb_unlock = 0;

	nirs = *irs;
	if (after_decrypt)
	    kfree(irs);
	irs = &nirs;

	/* begin decapsulating loop here */
	do {
		authlen = 0;
#ifdef CONFIG_IPSEC_ESP
		espp = NULL;
		esphlen = 0;
#endif /* !CONFIG_IPSEC_ESP */
#ifdef CONFIG_IPSEC_AH
		ahp = NULL;
		ahhlen = 0;
#endif /* CONFIG_IPSEC_AH */
#ifdef CONFIG_IPSEC_IPCOMP
		compp = NULL;
#endif /* CONFIG_IPSEC_IPCOMP */

		len = skb->len;
		dat = skb->data;
		ipp = (struct iphdr *)skb->data;
		proto = ipp->protocol;
		ipaddr.s_addr = ipp->saddr;
		addrtoa(ipaddr, 0, ipaddr_txt, sizeof(ipaddr_txt));
		
		iphlen = ipp->ihl << 2;
		ipp->check = 0;			/* we know the sum is good */
		
#ifdef CONFIG_IPSEC_DEBUG
		pkt_info.has_spi_seq = 0;
		pkt_info.proto = proto;
		pkt_info.src.s_addr = ipp->saddr;
		pkt_info.dst.s_addr = ipp->daddr;
#endif

#ifdef CONFIG_IPSEC_ESP
		/* XXX this will need to be 8 for IPv6 */
		if ((proto == IPPROTO_ESP) && ((len - iphlen) % 4)) {
			PRINTK_REJECT(&pkt_info, "", "klips_error: "
			       "got packet with content length = %d from %s -- should be on 4 octet boundary, packet dropped\n",
			       len - iphlen,
			       ipaddr_txt);
			goto rx_errors;
		}
#endif /* !CONFIG_IPSEC_ESP */
		
		/*
		 * Find tunnel control block and (indirectly) call the
		 * appropriate tranform routine. The resulting sk_buf
		 * is a valid IP packet ready to go through input processing.
		 */
		
		said.dst.s_addr = ipp->daddr;
		switch(proto) {
#ifdef CONFIG_IPSEC_ESP
		case IPPROTO_ESP:
			if(skb->len < (irs->hard_header_len + sizeof(struct iphdr) + sizeof(struct esp))) {
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv & DB_RX_INAU,
					    "klips_debug: "
					    "runt esp packet of skb->len=%d received from %s, dropped.\n",
					    skb->len,
					    ipaddr_txt);
				goto rx_errors;
			}
		       
			espp = (struct esp *)(skb->data + iphlen);
#ifdef CONFIG_IPSEC_DEBUG
			pkt_info.has_spi_seq = 1;
			pkt_info.spi = espp->esp_spi;
			pkt_info.seq = espp->esp_rpl;
#endif
			said.spi = espp->esp_spi;
			break;
#endif /* !CONFIG_IPSEC_ESP */
#ifdef CONFIG_IPSEC_AH
		case IPPROTO_AH:
			if((skb->len 
			    < (irs->hard_header_len + sizeof(struct iphdr) + sizeof(struct ah)))
			   || (skb->len 
			       < (irs->hard_header_len + sizeof(struct iphdr) 
				  + ((ahp = (struct ah *) (skb->data + iphlen))->ah_hl << 2)))) {
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv & DB_RX_INAU,
					    "klips_debug: "
					    "runt ah packet of skb->len=%d received from %s, dropped.\n",
					    skb->len,
					    ipaddr_txt);
				goto rx_errors;
			}
#ifdef CONFIG_IPSEC_DEBUG
			pkt_info.has_spi_seq = 1;
			pkt_info.spi = ahp->ah_spi;
			pkt_info.seq = ahp->ah_rpl;
#endif
			said.spi = ahp->ah_spi;
			break;
#endif /* CONFIG_IPSEC_AH */
#ifdef CONFIG_IPSEC_IPCOMP
		case IPPROTO_COMP:
			if(skb->len < (irs->hard_header_len + sizeof(struct iphdr) + sizeof(struct ipcomphdr))) {
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv & DB_RX_INAU,
					    "klips_debug: "
					    "runt comp packet of skb->len=%d received from %s, dropped.\n",
					    skb->len,
					    ipaddr_txt);
				goto rx_errors;
			}
		       
			compp = (struct ipcomphdr *)(skb->data + iphlen);
			said.spi = htonl((__u32)ntohs(compp->ipcomp_cpi));
			break;
#endif /* CONFIG_IPSEC_IPCOMP */
		default:
			KLIPS_REJECT_INFO(&pkt_info, "unknown protocol");
			goto rx_errors;
		}
		said.proto = proto;
		sa_len = satoa(said, 0, sa, SATOA_BUF);
		if(sa_len == 0) {
		  strcpy(sa, "(error)");
		}
		
#ifdef CONFIG_IPSEC_AH
		if(proto == IPPROTO_AH) {
			ahhlen = (ahp->ah_hl << 2) +
				((caddr_t)&(ahp->ah_rpl) - (caddr_t)ahp);
			next_header = ahp->ah_nh;
			if (ahhlen != sizeof(struct ah)) {
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv & DB_RX_INAU,
					    "klips_debug: "
					    "bad authenticator length %d, expected %d from %s.\n",
					    ahhlen - ((caddr_t)(ahp->ah_data) - (caddr_t)ahp),
					    AHHMAC_HASHLEN,
					    ipaddr_txt);
				goto rx_errors;
			}
			
		}
#endif /* CONFIG_IPSEC_AH */
		
		/*
		  The spinlock is to prevent any other process from
		  accessing or deleting the TDB hash table or any of the
		  TDBs while we are using and updating them.
		  
		  This is not optimal, but was relatively straightforward
		  at the time.  A better way to do it has been planned for
		  more than a year, to lock the hash table and put reference
		  counts on each TDB instead.  This is not likely to happen
		  in KLIPS1 unless a volunteer contributes it, but will be
		  designed into KLIPS2.
		*/
		if(tdbprev == NULL) {
			spin_lock(&tdb_lock);
			tdb_unlock = 1;
		}
		
#ifdef CONFIG_IPSEC_IPCOMP
		if (proto == IPPROTO_COMP) {
			unsigned int flags = 0;
			if (tdbp == NULL) {
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
					    "klips_debug: "
					    "Incoming packet with outer IPCOMP header SA:%s: not yet supported by KLIPS, dropped\n",
					    sa_len ? sa : " (error)");
				goto rx_dropped;
			}

			tdbprev = tdbp;
			tdbp = tdbnext;

			if(sysctl_ipsec_inbound_policy_check
			   && ((tdbp == NULL)
			       || (((ntohl(tdbp->tdb_said.spi) & 0x0000ffff)
				    != ntohl(said.spi))
				/* next line is a workaround for peer
				   non-compliance with rfc2393 */
				   && (tdbp->tdb_encalg != ntohl(said.spi)) 
				       ))) {
				char sa2[SATOA_BUF];
				size_t sa_len2 = 0;

				if(tdbp) {
					sa_len2 = satoa(tdbp->tdb_said, 0, sa2, SATOA_BUF);
				}
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
					    "klips_debug: "
					    "Incoming packet with SA(IPCA):%s does not match policy SA(IPCA):%s cpi=%04x cpi->spi=%08x spi=%08x, spi->cpi=%04x for SA grouping, dropped.\n",
					    sa_len ? sa : " (error)",
					    tdbp ? (sa_len2 ? sa2 : " (error)") : "NULL",
					    ntohs(compp->ipcomp_cpi),
					    (__u32)ntohl(said.spi),
					    tdbp ? (__u32)ntohl((tdbp->tdb_said.spi)) : 0,
					    tdbp ? (__u16)(ntohl(tdbp->tdb_said.spi) & 0x0000ffff) : 0);
				goto rx_dropped;
			}

			if (tdbp) {
				tdbp->tdb_comp_ratio_cbytes += ntohs(ipp->tot_len);
				tdbnext = tdbp->tdb_inext;
			}
			next_header = compp->ipcomp_nh;

			skb = skb_decompress(skb, tdbp, &flags);
			if (!skb || flags) {
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
					    "klips_debug: "
					    "skb_decompress() returned error flags=%x, dropped.\n",
					    flags);
				if (flags)
				    goto rx_errors;
				else
				    goto rx_dropped;
			}
#ifdef NET_21
			ipp = ip_hdr(skb);
#else /* NET_21 */
			ipp = skb->ip_hdr;
#endif /* NET_21 */

			if (tdbp) {
				tdbp->tdb_comp_ratio_dbytes += ntohs(ipp->tot_len);
			}

			KLIPS_PRINT(debug_rcv,
				    "klips_debug:ipsec_rcv: "
				    "packet decompressed SA(IPCA):%s cpi->spi=%08x spi=%08x, spi->cpi=%04x, nh=%d.\n",
				    sa_len ? sa : " (error)",
				    (__u32)ntohl(said.spi),
				    tdbp ? (__u32)ntohl((tdbp->tdb_said.spi)) : 0,
				    tdbp ? (__u16)(ntohl(tdbp->tdb_said.spi) & 0x0000ffff) : 0,
				    next_header);
			KLIPS_IP_PRINT(debug_rcv & DB_RX_PKTRX, ipp);

			continue;
			/* Skip rest of stuff and decapsulate next inner
			   packet, if any */
		}
#endif /* CONFIG_IPSEC_IPCOMP */
		
		stats_tdbp = tdbp = ipsec_sa_getbyid(&said);
		if (tdbp == NULL) {
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
				    "klips_debug: "
				    "no Tunnel Descriptor Block for SA:%s: incoming packet with no SA dropped\n",
				    sa_len ? sa : " (error)");
			goto rx_dropped;
		}

		/* The tdbp may be freed until decap callback is called. */
		irs->ipsp = tdbp;

		if(!after_decrypt && sysctl_ipsec_inbound_policy_check) {
			if(ipp->saddr != ((struct sockaddr_in*)(tdbp->tdb_addr_s))->sin_addr.s_addr) {
				ipaddr.s_addr = ipp->saddr;
				addrtoa(ipaddr, 0, ipaddr_txt, sizeof(ipaddr_txt));
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
					    "klips_debug: "
					    "SA:%s, src=%s of pkt does not agree with expected SA source address policy.\n",
					    sa_len ? sa : " (error)",
					    ipaddr_txt);
				goto rx_dropped;
			}
			ipaddr.s_addr = ipp->saddr;
			addrtoa(ipaddr, 0, ipaddr_txt, sizeof(ipaddr_txt));
			KLIPS_PRINT(debug_rcv,
				    "klips_debug:ipsec_rcv: "
				    "SA:%s, src=%s of pkt agrees with expected SA source address policy.\n",
				    sa_len ? sa : " (error)",
				    ipaddr_txt);
			if(tdbnext) {
				if(tdbnext != tdbp) {
					KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
						    "klips_debug: "
						    "unexpected SA:%s: does not agree with tdb->inext policy, dropped\n",
						    sa_len ? sa : " (error)");
					goto rx_dropped;
				}
				KLIPS_PRINT(debug_rcv,
					    "klips_debug:ipsec_rcv: "
					    "SA:%s grouping from previous SA is OK.\n",
					    sa_len ? sa : " (error)");
			} else {
				KLIPS_PRINT(debug_rcv,
					    "klips_debug:ipsec_rcv: "
					    "SA:%s First SA in group.\n",
					    sa_len ? sa : " (error)");
			}
			
			if(tdbp->tdb_onext) {
				if(tdbprev != tdbp->tdb_onext) {
					KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
						    "klips_debug: "
						    "unexpected SA:%s: does not agree with tdb->onext policy, dropped.\n",
						    sa_len ? sa : " (error)");
					goto rx_dropped;
				} else {
					KLIPS_PRINT(debug_rcv,
						    "klips_debug:ipsec_rcv: "
						    "SA:%s grouping to previous SA is OK.\n",
						    sa_len ? sa : " (error)");
				}
			} else {
				KLIPS_PRINT(debug_rcv,
					    "klips_debug:ipsec_rcv: "
					    "SA:%s No previous backlink in group.\n",
					    sa_len ? sa : " (error)");
			}
#ifdef CONFIG_IPSEC_NAT_TRAVERSAL
			KLIPS_PRINT(debug_rcv,
				"klips_debug:ipsec_rcv: "
				"natt_type=%u tdbp->ips_natt_type=%u : %s\n",
				irs->natt_type, tdbp->ips_natt_type,
				(irs->natt_type==tdbp->ips_natt_type)?"ok":"bad");
			if (irs->natt_type != tdbp->ips_natt_type) {
				KLIPS_PRINT(debug_rcv,
					    "klips_debug:ipsec_rcv: "
					    "SA:%s does not agree with expected NAT-T policy.\n",
					    sa_len ? sa : " (error)");
				goto rx_dropped;
			}
#endif		 
		}
		
		/* If it is in larval state, drop the packet, we cannot process yet. */
		if(tdbp->tdb_state == SADB_SASTATE_LARVAL) {
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
				    "klips_debug: "
				    "TDB in larval state, cannot be used yet, dropping packet.\n");
			goto rx_dropped;
		}
		
		if(tdbp->tdb_state == SADB_SASTATE_DEAD) {
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
				    "klips_debug: "
				    "TDB in dead state, cannot be used any more, dropping packet.\n");
			goto rx_dropped;
		}
		
		if(!after_decrypt && (ipsec_lifetime_check(&tdbp->ips_life.ipl_bytes, lifename[lifename_idx], sa,
					ipsec_life_countbased, ipsec_incoming, tdbp) == ipsec_life_harddied ||
		   ipsec_lifetime_check(&tdbp->ips_life.ipl_addtime, lifename[++lifename_idx], sa,
					ipsec_life_timebased,  ipsec_incoming, tdbp) == ipsec_life_harddied ||
		   ipsec_lifetime_check(&tdbp->ips_life.ipl_addtime, lifename[++lifename_idx], sa,
					ipsec_life_timebased,  ipsec_incoming, tdbp) == ipsec_life_harddied ||
		   ipsec_lifetime_check(&tdbp->ips_life.ipl_packets, lifename[++lifename_idx], sa, 
					ipsec_life_countbased, ipsec_incoming, tdbp) == ipsec_life_harddied)) {
			ipsec_sa_delchain(tdbp);
			stats_tdbp = NULL;
			KLIPS_REJECT_INFO(&pkt_info,
			    "hard %s lifetime of SA:<%s%s%s> %s has been reached, SA expired",
			    lifename[lifename_idx], IPS_XFORM_NAME(tdbp), sa);
			goto rx_dropped;
		}

		/* authenticate, if required */
		idat = dat + iphlen;
#ifdef CONFIG_IPSEC_ALG
		ixt_j = IPSEC_ALG_SA_ESP_JOINT(tdbp);
		if ((ixt_a=IPSEC_ALG_SA_ESP_AUTH(tdbp))) {
			authlen = AHHMAC_HASHLEN;
			KLIPS_PRINT(debug_rcv,
					"klips_debug:ipsec_rcv: "
					"authalg=%d authlen=%d\n",
					tdbp->tdb_authalg, authlen);
		} else
#endif /* CONFIG_IPSEC_ALG */
		switch(tdbp->tdb_authalg) {
#ifdef CONFIG_IPSEC_AUTH_HMAC_MD5
		case AH_MD5:
			authlen = AHHMAC_HASHLEN;
			break;
#endif /* CONFIG_IPSEC_AUTH_HMAC_MD5 */
#ifdef CONFIG_IPSEC_AUTH_HMAC_SHA1
		case AH_SHA:
			authlen = AHHMAC_HASHLEN;
			break;
#endif /* CONFIG_IPSEC_AUTH_HMAC_SHA1 */
		case AH_NONE:
			authlen = 0;
			break;
		default:
			tdbp->tdb_alg_errs += 1;
			KLIPS_REJECT_INFO(&pkt_info,
			    "unknown authentication type");
			goto rx_errors;
		}
		ilen = len - iphlen - authlen;
		if(ilen <= 0) {
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
				    "klips_debug: "
				    "runt AH packet with no data, dropping.\n");
			goto rx_dropped;
		}
		
#ifdef CONFIG_IPSEC_ESP
		KLIPS_PRINT(proto == IPPROTO_ESP && debug_rcv, 
			    "klips_debug:ipsec_rcv: "
			    "packet from %s received with seq=%d (iv)=0x%08x%08x iplen=%d esplen=%d sa=%s\n",
			    ipaddr_txt,
			    (__u32)ntohl(espp->esp_rpl),
			    (__u32)ntohl(*((__u32 *)(espp->esp_iv)    )),
			    (__u32)ntohl(*((__u32 *)(espp->esp_iv) + 1)),
			    len,
			    ilen,
			    sa_len ? sa : " (error)");
#endif /* !CONFIG_IPSEC_ESP */
		
		switch(proto) {
#ifdef CONFIG_IPSEC_ESP
		case IPPROTO_ESP:
			replay = ntohl(espp->esp_rpl);
			authenticator = &(dat[len - authlen]);
			break;
#endif /* !CONFIG_IPSEC_ESP */
#ifdef CONFIG_IPSEC_AH
		case IPPROTO_AH:
			replay = ntohl(ahp->ah_rpl);
			authenticator = ahp->ah_data;
			break;
#endif /* CONFIG_IPSEC_AH */
		}

		if (!after_decrypt && !ipsec_checkreplaywindow(tdbp, replay,
		        ipaddr_txt, err_msg, sizeof(err_msg)))
		{
			tdbp->tdb_replaywin_errs += 1;
			KLIPS_REJECT_INFO(&pkt_info, "%s", err_msg);
			goto rx_dropped;
		}
		
		/*
		 * verify authenticator
		 */
		
		KLIPS_PRINT(debug_rcv,
			    "klips_debug:ipsec_rcv: "
			    "encalg = %d, authalg = %d.\n",
			    tdbp->tdb_encalg,
			    tdbp->tdb_authalg);
		
#ifdef CONFIG_IPSEC_ALG
		if (ixt_a) {
			KLIPS_PRINT(debug_rcv,
					"klips_debug:ipsec_rcv: "
					"ipsec_alg hashing ... ");
			if(proto == IPPROTO_ESP) {
			        ipsec_alg_sa_esp_hash(tdbp,
				        (caddr_t)espp, ilen,
				        hash, AHHMAC_HASHLEN);
#ifdef CONFIG_IPSEC_AH
#ifdef IPSEC_ALG_WHEN_AH_IS_READY
			} else {
				ipo = *ipp;
				ipo.tos = 0;
				ipo.frag_off = 0;
				ipo.ttl = 0;
				ipo.check = 0;

				ipsec_alg_hmac_update(tdbp->tdb_key_a,
						(caddr_t)&ipo, 
						sizeof(struct iphdr));
				ipsec_alg_hmac_update(tdbp->tdb_key_a,
						(caddr_t)ahp,
						ahhlen - AHHMAC_HASHLEN);
				ipsec_alg_hmac_update(tdbp->tdb_key_a,
						(caddr_t)zeroes,
						AHHMAC_HASHLEN);
				ipsec_alg_hmac_hash(tdbp->tdb_key_a,
						(caddr_t)dat + iphlen + ahhlen,
						len - iphlen - ahhlen,
						hash, AHHMAC_HASHLEN);
#endif
#endif /* CONFIG_IPSEC_AH */
			}
		} else if (!(ixt_j && ixt_j->ixt_j_is_support_ah_proto) && proto == IPPROTO_AH)
#endif /* CONFIG_IPSEC_ALG */
		if(tdbp->tdb_authalg) {
			switch(tdbp->tdb_authalg) {
#ifdef CONFIG_IPSEC_AUTH_HMAC_MD5
			case AH_MD5:
				tctx.md5 = ((struct md5_ctx*)(tdbp->tdb_key_a))->ictx;
				if(proto == IPPROTO_ESP) {
					MD5Update(&tctx.md5, (caddr_t)espp, ilen);
#ifdef CONFIG_IPSEC_AH
				} else {
					ipo = *ipp;
					ipo.tos = 0;	/* mutable RFC 2402 3.3.3.1.1.1 */
					ipo.frag_off = 0;
					ipo.ttl = 0;
					ipo.check = 0;
					
					MD5Update(&tctx.md5, (caddr_t)&ipo,
						  sizeof(struct iphdr));
					MD5Update(&tctx.md5, (caddr_t)ahp,
						  ahhlen - AHHMAC_HASHLEN);
					MD5Update(&tctx.md5, (caddr_t)zeroes,
						  AHHMAC_HASHLEN);
					MD5Update(&tctx.md5,
						  (caddr_t)dat + iphlen + ahhlen,
						  len - iphlen - ahhlen);
#endif /* CONFIG_IPSEC_AH */
				}
				MD5Final(hash, &tctx.md5);
				tctx.md5 = ((struct md5_ctx*)(tdbp->tdb_key_a))->octx;
				MD5Update(&tctx.md5, hash, AHMD596_ALEN);
				MD5Final(hash, &tctx.md5);
				break;
#endif /* CONFIG_IPSEC_AUTH_HMAC_MD5 */
#ifdef CONFIG_IPSEC_AUTH_HMAC_SHA1
			case AH_SHA:
				tctx.sha1 = ((struct sha1_ctx*)(tdbp->tdb_key_a))->ictx;
				if(proto == IPPROTO_ESP) {
					SHA1Update(&tctx.sha1, (caddr_t)espp, ilen);
#ifdef CONFIG_IPSEC_AH
				} else {
					ipo = *ipp;
					ipo.tos = 0;
					ipo.frag_off = 0;
					ipo.ttl = 0;
					ipo.check = 0;
					
					SHA1Update(&tctx.sha1, (caddr_t)&ipo,
						   sizeof(struct iphdr));
					SHA1Update(&tctx.sha1, (caddr_t)ahp,
						   ahhlen - AHHMAC_HASHLEN);
					SHA1Update(&tctx.sha1, (caddr_t)zeroes,
						   AHHMAC_HASHLEN);
					SHA1Update(&tctx.sha1,
						   (caddr_t)dat + iphlen + ahhlen,
						   len - iphlen - ahhlen);
#endif /* CONFIG_IPSEC_AH */
				}
				SHA1Final(hash, &tctx.sha1);
				tctx.sha1 = ((struct sha1_ctx*)(tdbp->tdb_key_a))->octx;
				SHA1Update(&tctx.sha1, hash, AHSHA196_ALEN);
				SHA1Final(hash, &tctx.sha1);
				break;
#endif /* CONFIG_IPSEC_AUTH_HMAC_SHA1 */
			case AH_NONE:
				break;
			}
		}

#ifdef CONFIG_IPSEC_ALG
		if (!ixt_j || (!ixt_j->ixt_j_is_support_ah_proto && proto == IPPROTO_AH))
#endif
		{
			if(!authenticator) {
				tdbp->tdb_auth_errs += 1;
				KLIPS_REJECT_INFO(&pkt_info,
				    "authentication failed");
				goto rx_dropped;
			}

			if (memcmp(hash, authenticator, authlen)) {
				tdbp->tdb_auth_errs += 1;
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv & DB_RX_INAU,
					    "klips_debug: "
					    "auth failed on incoming packet from %s: hash=%08x%08x%08x auth=%08x%08x%08x, dropped\n",
					    ipaddr_txt,
					    *(__u32*)&hash[0],
					    *(__u32*)&hash[4],
					    *(__u32*)&hash[8],
					    *(__u32*)authenticator,
					    *((__u32*)authenticator + 1),
					    *((__u32*)authenticator + 2));
				goto rx_dropped;
			} else {
				KLIPS_PRINT(debug_rcv,
					    "klips_debug:ipsec_rcv: "
					    "authentication successful.\n");
			}
			
			memset((caddr_t)&tctx, 0, sizeof(tctx));
			memset(hash, 0, sizeof(hash));
		}

		/* If the sequence number == 0, expire SA, it had rolled */
		if(!after_decrypt && tdbp->tdb_replaywin && !replay /* !tdbp->tdb_replaywin_lastseq */) {
			ipsec_sa_delchain(tdbp);
			stats_tdbp = NULL;
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
				    "klips_debug: "
				    "replay window counter rolled, expiring SA.\n");
			goto rx_dropped;
		}

		if (!after_decrypt && !ipsec_updatereplaywindow(tdbp, replay)) {
			tdbp->tdb_replaywin_errs += 1;
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv & DB_RX_REPLAY,
				    "klips_debug: "
				    "duplicate frame from %s, packet dropped\n",
				    ipaddr_txt);
			goto rx_dropped;
		}
		
		switch(proto) {
#ifdef CONFIG_IPSEC_ESP
		case IPPROTO_ESP:
#ifdef CONFIG_IPSEC_ALG
			if ((ixt_e=IPSEC_ALG_SA_ESP_ENC(tdbp))) {
				esphlen = ESP_HEADER_LEN + ixt_e->ixt_ivlen/8;
				KLIPS_PRINT(debug_rcv,
						"klips_debug:ipsec_rcv: "
						"encalg=%d esphlen=%d\n",
						tdbp->tdb_encalg, esphlen);
			} else
#endif /* CONFIG_IPSEC_ALG */
			switch(tdbp->tdb_encalg) {
#ifdef USE_SINGLE_DES
			case ESP_DES:
#endif
#ifdef CONFIG_IPSEC_ENC_3DES
			case ESP_3DES:
				iv[0] = *((__u32 *)(espp->esp_iv)    );
				iv[1] = *((__u32 *)(espp->esp_iv) + 1);
				esphlen = ESP_HEADER_LEN + EMT_ESPDES_IV_SZ;
				break;
#endif
#ifdef CONFIG_IPSEC_ENC_NULL
			case ESP_NULL:
			        esphlen = ESP_HEADER_LEN;
				break;
#endif
#ifdef CONFIG_IPSEC_ENC_AES
		        case ESP_AES:
			        esphlen = ESP_HEADER_LEN + EMT_ESPAES_IV_SZ;
			        break;
#endif
			default:
				tdbp->tdb_alg_errs += 1;
				KLIPS_REJECT_INFO(&pkt_info,
				    "unsupported encryption protocol");
				goto rx_errors;
			}
			idat += esphlen;
			ilen -= esphlen;
			
#ifdef CONFIG_IPSEC_ALG
			if (ixt_e)
			{
				if (ipsec_alg_esp_encrypt(tdbp, 
					idat, ilen, espp->esp_iv, 
					IPSEC_ALG_DECRYPT) <= 0)
				{
					ipsec_log("klips_error:ipsec_rcv: "
						"got packet with esplen = %d "
						"from %s -- should be on "
						"ENC(%d) octet boundary, "
						"packet dropped\n",
							ilen,
							ipaddr_txt,
							tdbp->tdb_encalg);
					goto rx_errors;
				}
			} else if (!ixt_j)
#endif /* CONFIG_IPSEC_ALG */
			switch(tdbp->tdb_encalg) {
#ifdef USE_SINGLE_DES
			case ESP_DES:
				if ((ilen) % 8) {
				    PRINTK_REJECT(&pkt_info, "", "klips_error: "
					"got packet with esplen = %d from %s "
					"-- should be on 8 octet boundary, packet dropped\n",
					ilen, ipaddr_txt);
				    tdbp->tdb_encsize_errs += 1;
				    goto rx_errors;
				}
				des_cbc_encrypt(idat, idat, ilen,
				    tdbp->tdb_key_e,
				    (caddr_t)iv, 0);
				break;
#endif
#ifdef CONFIG_IPSEC_ENC_3DES
			case ESP_3DES:
				if ((ilen) % 8) {
					tdbp->tdb_encsize_errs += 1;
					PRINTK_REJECT(&pkt_info, "", "klips_error: "
					       "got packet with esplen = %d from %s -- should be on 8 octet boundary, packet dropped\n",
					       ilen,
					       ipaddr_txt);
					goto rx_errors;
				}
				des_ede3_cbc_encrypt((des_cblock *)idat,
						     (des_cblock *)idat,
						     ilen,
						     ((struct des_eks *)(tdbp->tdb_key_e))[0].ks,
						     ((struct des_eks *)(tdbp->tdb_key_e))[1].ks,
						     ((struct des_eks *)(tdbp->tdb_key_e))[2].ks,
						     (des_cblock *)iv, 0);
				break;
#endif
#ifdef CONFIG_IPSEC_ENC_NULL
			case ESP_NULL:
				break;
#endif
#ifdef CONFIG_IPSEC_ENC_AES
			case ESP_AES:
			        KLIPS_PRINT(debug_tunnel & DB_TN_CROUT,
					    "AES isn't supported without CryptoAPI or HW encryption\n");
				goto rx_errors;
#endif
			}
			break;
#endif /* !CONFIG_IPSEC_ESP */
#ifdef CONFIG_IPSEC_AH
		case IPPROTO_AH:
			break;
#endif /* CONFIG_IPSEC_AH */
		}
#ifdef CONFIG_IPSEC_ALG
		if (ixt_j && (proto == IPPROTO_ESP || (proto == IPPROTO_AH && ixt_j->ixt_j_is_support_ah_proto)))
		{
		        ipsec_alg_complete_cb_t complete_cb = NULL;
#ifdef CONFIG_IPSEC_ALG_JOINT_ASYNC
			complete_cb = ixt_j->ixt_j_is_full_process_orig_pkt ? ipsec_rcv_end_cb : ipsec_rcv_decap;
			if (!after_decrypt)
			{
				irs = kmalloc(sizeof(*irs), GFP_ATOMIC);
				if (!irs)
				    status = ENOMEM;
				else
				{
				    *irs = nirs;
				    irs->after_decrypt = 1;
#endif
				    status = ipsec_alg_joint_auth_and_encrypt(
					irs->skb, tdbp, idat - esphlen,
					ilen + esphlen + authlen,
					esphlen, authlen, espp->esp_iv,
					IPSEC_ALG_DECRYPT, complete_cb, irs);
#ifdef CONFIG_IPSEC_ALG_JOINT_ASYNC
				    if (status)
				    {
					kfree(irs);
					irs = &nirs;
				    }
				}
			}
#endif

			if (status)
			{
				ipsec_log("klips_error:ipsec_rcv: "
				    "packet from %s failed auth/decrypt\n",
				    ipaddr_txt);
				goto rx_errors;
			}

#ifdef CONFIG_IPSEC_ALG_JOINT_ASYNC
			if (after_decrypt)
				after_decrypt = 0;
			else
			{
				skb = NULL;
				goto rcvleave;
			}
#endif
		}
#endif
		if (proto == IPPROTO_ESP)
		{
			next_header = idat[ilen - 1];
			padlen = idat[ilen - 2];
			pad = padlen + 2 + authlen;
			{
			        int badpad = 0;
				
				KLIPS_PRINT(debug_rcv & DB_RX_IPAD,
					    "klips_debug:ipsec_rcv: "
					    "padlen=%d, contents: 0x<offset>: 0x<value> 0x<value> ...\n",
					    padlen);
				
				for (i = 1; i <= padlen; i++) {
					if((i % 16) == 1) {
						KLIPS_PRINTMORE_START(debug_rcv & DB_RX_IPAD,
							    "klips_debug:           %02x:",
							    i - 1);
					}
					KLIPS_PRINTMORE(debug_rcv & DB_RX_IPAD,
						    " %02x",
						    idat[ilen - 2 - padlen + i - 1]);
					if(i != idat[ilen - 2 - padlen + i - 1]) {
					        badpad = 1;
					} 
					if((i % 16) == 0) {
						KLIPS_PRINTMORE_FINISH(debug_rcv & DB_RX_IPAD);
					}
				}
				if((i % 16) != 1) {
					KLIPS_PRINTMORE_FINISH(debug_rcv & DB_RX_IPAD);
				}
				if(badpad) {
					KLIPS_PRINT(debug_rcv & DB_RX_IPAD,
						    "klips_debug:ipsec_rcv: "
						    "warning, decrypted packet from %s has bad padding\n",
						    ipaddr_txt);
					KLIPS_PRINT(debug_rcv & DB_RX_IPAD,
						    "klips_debug:ipsec_rcv: "
						    "...may be bad decryption -- not dropped\n");
					tdbp->tdb_encpad_errs += 1;
				}
				
				KLIPS_PRINT(debug_rcv & DB_RX_IPAD,
					    "klips_debug:ipsec_rcv: "
					    "packet decrypted from %s: next_header = %d, padding = %d\n",
					    ipaddr_txt,
					    next_header,
					    pad - 2 - authlen);
			}
		}
	       
		/*
		 *	Discard the original ESP/AH header
		 */
		
		ipp->protocol = next_header;
		
		switch(proto) {
#ifdef CONFIG_IPSEC_ESP
		case IPPROTO_ESP:
			ipp->tot_len = htons(ntohs(ipp->tot_len) - (esphlen + pad));
			memmove((void *)(skb->data + esphlen),
				(void *)(skb->data), iphlen);
			if(skb->len < esphlen) {
				PRINTK_REJECT(&pkt_info, KERN_WARNING,
				       "klips_error: "
				       "tried to skb_pull esphlen=%d, %d available.  This should never happen, please report.\n",
				       esphlen, (int)(skb->len));
				goto rcvleave;
			}

			KLIPS_PRINT(debug_rcv & DB_RX_PKTRX,
				    "klips_debug:ipsec_rcv: "
				    "trimming to %d.\n",
				    len - esphlen - pad);
			if(pad + esphlen <= len) {
				skb_pull(skb, esphlen);
			        skb_trim(skb, len - esphlen - pad);
			} else {
			        KLIPS_PRINT_REJECT(&pkt_info, debug_rcv & DB_RX_PKTRX,
				            "klips_debug: "
					    "bogus packet, size is zero or negative "
					    "(may be invalid encryption key), dropping.\n");
				goto rcvleave;
			}
		break;
#endif /* !CONFIG_IPSEC_ESP */
#ifdef CONFIG_IPSEC_AH
		case IPPROTO_AH:
			ipp->tot_len = htons(ntohs(ipp->tot_len) - ahhlen);
			memmove((void *)(skb->data + ahhlen),
				(void *)(skb->data), iphlen);
			if(skb->len < ahhlen) {
				PRINTK_REJECT(&pkt_info, KERN_WARNING,
				       "klips_error: "
				       "tried to skb_pull ahhlen=%d, %d available.  This should never happen, please report.\n",
				       ahhlen,
				       (int)(skb->len));
				goto rcvleave;
			}
			skb_pull(skb, ahhlen);
			break;
#endif /* CONFIG_IPSEC_AH */
		}


		/*
		 *	Adjust pointers
		 */
		
		len = skb->len;
		dat = skb->data;
		
#ifdef NET_21
/*		skb->h.ipiph=(struct iphdr *)skb->data; */
		skb_set_network_header(skb, 0);
		skb_set_transport_header(skb, ip_hdr(skb)->ihl << 2);
		
		memset(&(IPCB(skb)->opt), 0, sizeof(struct ip_options));
#else /* NET_21 */
		skb->h.iph=(struct iphdr *)skb->data;
		skb->ip_hdr=(struct iphdr *)skb->data;
		memset(skb->proto_priv, 0, sizeof(struct options));
#endif /* NET_21 */
		
		ipp = (struct iphdr *)dat;
		ipp->check = 0;
		ipp->check = ip_fast_csum((unsigned char *)dat, iphlen >> 2);
		
		KLIPS_PRINT(debug_rcv & DB_RX_PKTRX,
			    "klips_debug:ipsec_rcv: "
			    "after <%s%s%s>, SA:%s:\n",
			    IPS_XFORM_NAME(tdbp),
			    sa_len ? sa : " (error)");
		KLIPS_IP_PRINT(debug_rcv & DB_RX_PKTRX, ipp);
		
		skb->protocol = htons(ETH_P_IP);
		skb->ip_summed = 0;

		tdbprev = tdbp;
		tdbnext = tdbp->tdb_inext;
		if(sysctl_ipsec_inbound_policy_check) {
			if(tdbnext) {
				if(tdbnext->tdb_onext != tdbp) {
					KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
						    "klips_debug: "
						    "SA:%s, backpolicy does not agree with fwdpolicy.\n",
						    sa_len ? sa : " (error)");
					goto rx_dropped;
				}
				KLIPS_PRINT(debug_rcv,
					    "klips_debug:ipsec_rcv: "
					    "SA:%s, backpolicy agrees with fwdpolicy.\n",
					    sa_len ? sa : " (error)");
				if(
					ipp->protocol != IPPROTO_AH  
					&& ipp->protocol != IPPROTO_ESP 
#ifdef CONFIG_IPSEC_IPCOMP
					&& ipp->protocol != IPPROTO_COMP
					&& (tdbnext->tdb_said.proto != IPPROTO_COMP
					    || (tdbnext->tdb_said.proto == IPPROTO_COMP
						&& tdbnext->tdb_inext))
#endif /* CONFIG_IPSEC_IPCOMP */
					&& ipp->protocol != IPPROTO_IPIP
					) {
					KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
						    "klips_debug: "
						    "packet with incomplete policy dropped, last successful SA:%s.\n",
						    sa_len ? sa : " (error)");
					goto rx_dropped;
				}
				KLIPS_PRINT(debug_rcv,
					    "klips_debug:ipsec_rcv: "
					    "SA:%s, Another IPSEC header to process.\n",
					    sa_len ? sa : " (error)");
			} else {
				KLIPS_PRINT(debug_rcv,
					    "klips_debug:ipsec_rcv: "
					    "No tdb_inext from this SA:%s.\n",
					    sa_len ? sa : " (error)");
			}
		}

#ifdef CONFIG_IPSEC_IPCOMP
		/* update ipcomp ratio counters, even if no ipcomp packet is present */
		if (tdbnext
		  && tdbnext->tdb_said.proto == IPPROTO_COMP
		  && ipp->protocol != IPPROTO_COMP) {
			tdbnext->tdb_comp_ratio_cbytes += ntohs(ipp->tot_len);
			tdbnext->tdb_comp_ratio_dbytes += ntohs(ipp->tot_len);
		}
#endif /* CONFIG_IPSEC_IPCOMP */

		tdbp->ips_life.ipl_bytes.ipl_count += len;
		tdbp->ips_life.ipl_bytes.ipl_last   = len;

		if(!tdbp->ips_life.ipl_usetime.ipl_count) {
			tdbp->ips_life.ipl_usetime.ipl_count = jiffies / HZ;
		}
		tdbp->ips_life.ipl_usetime.ipl_last = jiffies / HZ;
		tdbp->ips_life.ipl_packets.ipl_count += 1;
		
	/* end decapsulation loop here */
	} while(   (ipp->protocol == IPPROTO_ESP )
		|| (ipp->protocol == IPPROTO_AH  )
#ifdef CONFIG_IPSEC_IPCOMP
		|| (ipp->protocol == IPPROTO_COMP)
#endif /* CONFIG_IPSEC_IPCOMP */
		);
	
#ifdef CONFIG_IPSEC_IPCOMP
	if(tdbnext && tdbnext->tdb_said.proto == IPPROTO_COMP) {
		tdbprev = tdbp;
		tdbp = tdbnext;
		tdbnext = tdbp->tdb_inext;
	}
#endif /* CONFIG_IPSEC_IPCOMP */

#ifdef CONFIG_IPSEC_NAT_TRAVERSAL
	if ((irs->natt_type) && (ipp->protocol != IPPROTO_IPIP)) {
	    ipsec_rcv_natt_correct_tcp_udp_csum(skb, ipp, tdbp);
	}
#endif

	/*
	 * XXX this needs to be locked from when it was first looked
	 * up in the decapsulation loop.  Perhaps it is better to put
	 * the IPIP decap inside the loop.
	 */
	if(tdbnext) {
		tdbp = tdbnext;
		sa_len = satoa(tdbp->tdb_said, 0, sa, SATOA_BUF);
		if(ipp->protocol != IPPROTO_IPIP) {
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
				    "klips_debug: "
				    "SA:%s, Hey!  How did this get through?  Dropped.\n",
				    sa_len ? sa : " (error)");
			goto rx_dropped;
		}
		if(sysctl_ipsec_inbound_policy_check) {
			if((tdbnext = tdbp->tdb_inext)) {
				char sa2[SATOA_BUF];
				size_t sa_len2;
				sa_len2 = satoa(tdbnext->tdb_said, 0, sa2, SATOA_BUF);
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
					    "klips_debug: "
					    "unexpected SA:%s after IPIP SA:%s\n",
					    sa_len2 ? sa2 : " (error)",
					    sa_len ? sa : " (error)");
				goto rx_dropped;
			}
			if(ipp->saddr != ((struct sockaddr_in*)(tdbp->tdb_addr_s))->sin_addr.s_addr) {
				ipaddr.s_addr = ipp->saddr;
				addrtoa(ipaddr, 0, ipaddr_txt, sizeof(ipaddr_txt));
				KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
					    "klips_debug: "
					    "SA:%s, src=%s of pkt does not agree with expected SA source address policy.\n",
					    sa_len ? sa : " (error)",
					    ipaddr_txt);
				goto rx_dropped;
			}
		}

		/*
		 * XXX this needs to be locked from when it was first looked
		 * up in the decapsulation loop.  Perhaps it is better to put
		 * the IPIP decap inside the loop.
		 */
		tdbp->ips_life.ipl_bytes.ipl_count += len;
		tdbp->ips_life.ipl_bytes.ipl_last   = len;

		if(!tdbp->ips_life.ipl_usetime.ipl_count) {
			tdbp->ips_life.ipl_usetime.ipl_count = jiffies / HZ;
		}
		tdbp->ips_life.ipl_usetime.ipl_last = jiffies / HZ;
		tdbp->ips_life.ipl_packets.ipl_count += 1;
		
		if(skb->len < iphlen) {
			PRINTK_REJECT(&pkt_info, KERN_WARNING, "klips_debug: "
			       "tried to skb_pull iphlen=%d, %d available.  This should never happen, please report.\n",
			       iphlen,
			       (int)(skb->len));

			goto rcvleave;
		}
		skb_pull(skb, iphlen);

#ifdef NET_21
		skb_set_network_header(skb, 0);
		ipp = ip_hdr(skb);
		skb_set_transport_header(skb, ip_hdr(skb)->ihl << 2);

		memset(&(IPCB(skb)->opt), 0, sizeof(struct ip_options));
#else /* NET_21 */
		ipp = skb->ip_hdr = skb->h.iph = (struct iphdr *)skb->data;

		memset(skb->proto_priv, 0, sizeof(struct options));
#endif /* NET_21 */

		skb->protocol = htons(ETH_P_IP);
		skb->ip_summed = 0;
		KLIPS_PRINT(debug_rcv & DB_RX_PKTRX,
			    "klips_debug:ipsec_rcv: "
			    "IPIP tunnel stripped.\n");
		KLIPS_IP_PRINT(debug_rcv & DB_RX_PKTRX, ipp);

		if(sysctl_ipsec_inbound_policy_check)
		{
		    u32 s_addr, d_addr, tdb_s_ip, tdb_d_ip, tdb_s_to, tdb_d_to;
#define IS_MATCH(ip, t_ip, t_mask, t_to) \
		    ((t_to) ? (ip) >= (t_ip) && (ip) <= (t_to) : \
		    !(((ip) & (t_mask)) ^ (t_ip)))

		    s_addr = ntohl(ipp->saddr);
		    tdb_s_ip = ntohl(tdbp->tdb_flow_s.u.v4.sin_addr.s_addr);
		    tdb_s_to = ntohl(tdbp->tdb_to_s.u.v4.sin_addr.s_addr);
		    d_addr = ntohl(ipp->daddr);
		    tdb_d_ip = ntohl(tdbp->tdb_flow_d.u.v4.sin_addr.s_addr);
		    tdb_d_to = ntohl(tdbp->tdb_to_d.u.v4.sin_addr.s_addr);
		    if (!IS_MATCH(s_addr, tdb_s_ip,
			ntohl(tdbp->tdb_mask_s.u.v4.sin_addr.s_addr),
			tdb_s_to) ||
			!IS_MATCH(d_addr, tdb_d_ip,
			ntohl(tdbp->tdb_mask_d.u.v4.sin_addr.s_addr), tdb_d_to))
		    {
			struct in_addr daddr, saddr;
			char saddr_txt[ADDRTOA_BUF], daddr_txt[ADDRTOA_BUF];
			char sflow_txt[SUBNETTOA_BUF], dflow_txt[SUBNETTOA_BUF];
			
			subnettoa(tdbp->tdb_flow_s.u.v4.sin_addr,
				tdbp->tdb_mask_s.u.v4.sin_addr,
				0, sflow_txt, sizeof(sflow_txt));
			subnettoa(tdbp->tdb_flow_d.u.v4.sin_addr,
				tdbp->tdb_mask_d.u.v4.sin_addr,
				0, dflow_txt, sizeof(dflow_txt));
			saddr.s_addr = ipp->saddr;
			daddr.s_addr = ipp->daddr;
			addrtoa(saddr, 0, saddr_txt, sizeof(saddr_txt));
			addrtoa(daddr, 0, daddr_txt, sizeof(daddr_txt));
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv,
				    "klips_debug: "
				    "SA:%s, inner tunnel policy [%s -> %s] does not agree with pkt contents [%s -> %s].\n",
				    sa_len ? sa : " (error)",
				    sflow_txt,
				    dflow_txt,
				    saddr_txt,
				    daddr_txt);
			goto rx_dropped;
		    }
		}
	}

#ifdef NET_21
	if(irs->stats) {
		irs->stats->rx_bytes += skb->len;
	}
	skb_dst_drop(skb);
	skb->pkt_type = PACKET_HOST;
	if(irs->hard_header_len &&
	   (skb_mac_header(skb) != (skb->data - irs->hard_header_len)) &&
	   (irs->hard_header_len <= skb_headroom(skb))) {
		/* copy back original MAC header */
		memmove(skb->data - irs->hard_header_len, skb_mac_header(skb), irs->hard_header_len);
		skb_set_mac_header(skb, -irs->hard_header_len);
	}
#endif /* NET_21 */

#ifdef CONFIG_IPSEC_IPCOMP
	if(ipp->protocol == IPPROTO_COMP) {
		unsigned int flags = 0;

		if(sysctl_ipsec_inbound_policy_check) {
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv & DB_RX_PKTRX,
				"klips_debug: "
				"inbound policy checking enabled, IPCOMP follows IPIP, dropped.\n");
			goto rx_errors;
		}
		/*
		  XXX need a TDB for updating ratio counters but it is not
		  following policy anyways so it is not a priority
		*/
		skb = skb_decompress(skb, NULL, &flags);
		if (!skb || flags) {
			KLIPS_PRINT_REJECT(&pkt_info, debug_rcv & DB_RX_PKTRX,
				"klips_debug: "
				"skb_decompress() returned error flags: %d, dropped.\n",
			       flags);
			goto rx_errors;
		}
	}
#endif /* CONFIG_IPSEC_IPCOMP */

#ifdef SKB_RESET_NFCT
        nf_conntrack_put(skb->nfct);
        skb->nfct = NULL;
#ifdef CONFIG_NETFILTER_DEBUG
	skb->nf_debug = 0;
#endif /* CONFIG_NETFILTER_DEBUG */
#endif /* SKB_RESET_NFCT */
	KLIPS_PRINT(debug_rcv & DB_RX_PKTRX,
		    "klips_debug:ipsec_rcv: "
		    "netif_rx() called.\n");

	while (stats_tdbp)
	{
	    stats_tdbp->ips_life.orig_bytes += skb->len;
	    stats_tdbp->ips_life.orig_packets++;
	    stats_tdbp = stats_tdbp->ips_inext;
	}
	
	netif_rx(skb);
	skb = NULL;

	goto rcvleave;

 rx_errors:
	if (irs->stats)
	    irs->stats->rx_errors++;
	while (stats_tdbp)
	{
	    stats_tdbp->ips_life.orig_errors++;
	    stats_tdbp = stats_tdbp->ips_inext;
	}
	goto rcvleave;

 rx_dropped:
	if (irs->stats)
	    irs->stats->rx_dropped++;
	while (stats_tdbp)
	{
	    stats_tdbp->ips_life.orig_dropped++;
	    stats_tdbp = stats_tdbp->ips_inext;
	}
	goto rcvleave;

 rcvleave:
	if (tdb_unlock)
	    spin_unlock(&tdb_lock);
	if(skb) {
		ipsec_kfree_skb(skb);
	}

	return;
}

int _ipsec_rcv(struct sk_buff *skb, struct sock *sk)
{
#ifdef NET_21
#ifdef CONFIG_IPSEC_DEBUG
	struct net_device *dev = skb->dev;
#endif /* CONFIG_IPSEC_DEBUG */
#endif /* NET_21 */
	unsigned char protoc;
	struct iphdr *ipp;

	int iphlen;
	struct net_device_stats *stats = NULL;		/* This device's statistics */
	struct net_device *ipsecdev = NULL, *prvdev;
	struct ipsecpriv *prv;
	struct ipsec_rcv_state nirs, *irs = &nirs;
	
	/* Don't unlink in the middle of a turnaround */
	KLIPS_INC_USE;
	
	if (skb == NULL) {
		KLIPS_PRINT(debug_rcv, 
			    "klips_debug:ipsec_rcv: "
			    "NULL skb passed in.\n");
		goto rcvleave;
	}
		
	if (skb->data == NULL) {
		KLIPS_PRINT(debug_rcv,
			    "klips_debug:ipsec_rcv: "
			    "NULL skb->data passed in, packet is bogus, dropping.\n");
		goto rcvleave;
	}

	memset(irs, 0, sizeof(*irs));
		
#if defined(CONFIG_IPSEC_NAT_TRAVERSAL) && !defined(NET_26)
	{
		/* NET_26 NAT-T is handled by seperate function */
		struct sk_buff *nskb;
		int udp_decap_ret = 0;

		nskb = ipsec_rcv_natt_decap(skb, sk, irs, &udp_decap_ret);
		if (nskb == NULL) {
			/* return with non-zero, because UDP.c code
			 * need to send it upstream.
			 */
			if (skb && udp_decap_ret == 0) {
				ipsec_kfree_skb(skb);
			}
			KLIPS_DEC_USE;
			return(udp_decap_ret);
		}
		skb = nskb;
	}
#endif /* NAT_T */

#ifdef IPH_is_SKB_PULLED
	/* In Linux 2.4.4, the IP header has been skb_pull()ed before the
	   packet is passed to us. So we'll skb_push() to get back to it. */
	if (skb->data == skb_transport_header(skb)) {
		skb_push(skb, skb_transport_header(skb) -
		    skb_network_header(skb));
	}
#endif /* IPH_is_SKB_PULLED */

	/* irs->hard_header_len is unreliable and should not be used */
	irs->hard_header_len = skb_mac_header(skb) ? (skb->data - skb_mac_header(skb)) : 0;
	if((irs->hard_header_len < 0) || (irs->hard_header_len > skb_headroom(skb)))
		irs->hard_header_len = 0;

	skb = ipsec_rcv_unclone(skb, irs->hard_header_len);
	if (skb == NULL) {
		goto rcvleave;
	}

#if IP_FRAGMENT_LINEARIZE
	/* In Linux 2.4.4, we may have to reassemble fragments. They are
	   not assembled automatically to save TCP from having to copy
	   twice.
	*/
	if (skb_is_nonlinear(skb)) {
	    if (os_skb_linearize(skb) != 0) {
		goto rcvleave;
	    }
	}
#endif
	
#if defined(CONFIG_IPSEC_NAT_TRAVERSAL) && !defined(NET_26)
	if (irs->natt_len) {
		/**
		 * Now, we are sure packet is ESPinUDP, and we have a private
		 * copy that has been linearized, remove natt_len bytes
		 * from packet and modify protocol to ESP.
		 */
		if (((unsigned char *)skb->data > (unsigned char *)ip_hdr(skb))
		    && ((unsigned char *)ip_hdr(skb) > (unsigned char *)skb->head))
		{
			unsigned int _len = (unsigned char *)skb->data -
				(unsigned char *)ip_hdr(skb);
			KLIPS_PRINT(debug_rcv,
				"klips_debug:ipsec_rcv: adjusting skb: skb_push(%u)\n",
				_len);
			skb_push(skb, _len);
		}
		KLIPS_PRINT(debug_rcv,
		    "klips_debug:ipsec_rcv: "
			"removing %d bytes from ESPinUDP packet\n", irs->natt_len);
		ipp = ip_hdr(skb);
		iphlen = ipp->ihl << 2;
		ipp->tot_len = htons(ntohs(ipp->tot_len) - irs->natt_len);
		if (skb->len < iphlen + irs->natt_len) {
			printk(KERN_WARNING
		        "klips_error:ipsec_rcv: "
		        "ESPinUDP packet is too small (%d < %d+%d). "
			"This should never happen, please report.\n",
		        (int)(skb->len), iphlen, irs->natt_len);
			goto rcvleave;
		}

		memmove(skb->data + irs->natt_len, skb->data, iphlen);
		skb_pull(skb, irs->natt_len);
		/* update nh.iph */
		skb_set_network_header(skb, 0);
		ipp = ip_hdr(skb);

		/* modify protocol */
		ipp->protocol = IPPROTO_ESP;

		skb->sk = NULL;

		KLIPS_IP_PRINT(debug_rcv, ip_hdr(skb));
	}
#endif

	ipp = ip_hdr(skb);
	iphlen = ipp->ihl << 2;

	KLIPS_PRINTMORE_START(debug_rcv,
		    "klips_debug:ipsec_rcv: "
		    "<<< Info -- ");
	KLIPS_PRINTMORE(debug_rcv && skb->dev, "skb->dev=%s ",
		    skb->dev->name ? skb->dev->name : "NULL");
	KLIPS_PRINTMORE(debug_rcv && dev, "dev=%s ",
		    dev->name ? dev->name : "NULL");
	KLIPS_PRINTMORE_FINISH(debug_rcv);

	KLIPS_PRINT(debug_rcv && !(skb->dev && dev && (skb->dev == dev)),
		    "klips_debug:ipsec_rcv: "
		    "Informational -- **if this happens, find out why** skb->dev:%s is not equal to dev:%s\n",
		    skb->dev ? (skb->dev->name ? skb->dev->name : "NULL") : "NULL",
		    dev ? (dev->name ? dev->name : "NULL") : "NULL");

	protoc = ipp->protocol;
#ifndef NET_21
	if((!protocol) || (protocol->protocol != protoc)) {
		KLIPS_PRINT(debug_rcv & DB_RX_TDB,
			    "klips_debug:ipsec_rcv: "
			    "protocol arg is NULL or unequal to the packet contents, this is odd, using value in packet.\n");
	}
#endif /* !NET_21 */

	if( (protoc != IPPROTO_AH) &&
#ifdef CONFIG_IPSEC_IPCOMP_disabled_until_we_register_IPCOMP_HANDLER
	    (protoc != IPPROTO_COMP) &&
#endif /* CONFIG_IPSEC_IPCOMP */
	    (protoc != IPPROTO_ESP) ) {
		KLIPS_PRINT(debug_rcv & DB_RX_TDB,
			    "klips_debug:ipsec_rcv: Why the hell is someone "
			    "passing me a non-ipsec protocol = %d packet? -- dropped.\n",
			    protoc);
		goto rcvleave;
	}

	if(skb->dev) {
		ipsec_dev_list *cur;

		for(cur=ipsec_dev_head; cur; cur=cur->next) {
			if(!strcmp(cur->ipsec_dev->name, skb->dev->name)) {
				prv = (struct ipsecpriv *)netdev_priv(skb->dev);
				if(prv) {
					stats = (struct net_device_stats *) &(prv->mystats);
				}
				ipsecdev = skb->dev;
				KLIPS_PRINT(debug_rcv,
					    "klips_debug:ipsec_rcv: "
					    "Info -- pkt already proc'ed a group of ipsec headers, processing next group of ipsec headers.\n");
				break;
			}
			if((ipsecdev = ipsec_dev_get(cur->ipsec_dev->name)) == NULL) {
				KLIPS_PRINT(debug_rcv,
					    "klips_error:ipsec_rcv: "
					    "device %s does not exist\n",
					    cur->ipsec_dev->name);
			}
			prv = ipsecdev ?
			    (struct ipsecpriv *)netdev_priv(ipsecdev) : NULL;
			prvdev = prv ? (struct net_device *)(prv->dev) : NULL;
			
			if(prvdev && skb->dev &&
			   !strcmp(prvdev->name, skb->dev->name)) {
				stats = prv ? ((struct net_device_stats *) &(prv->mystats)) : NULL;
				skb->dev = ipsecdev;
				KLIPS_PRINT(debug_rcv && prvdev, 
					    "klips_debug:ipsec_rcv: "
					    "assigning packet ownership to virtual device %s from physical device %s.\n",
					    cur->ipsec_dev->name, prvdev->name);
				if(stats) {
					stats->rx_packets++;
				}
				break;
			}
		}
	} else {
		KLIPS_PRINT(debug_rcv, 
			    "klips_debug:ipsec_rcv: "
			    "device supplied with skb is NULL\n");
	}
			
	if(!stats) {
	        KLIPS_PRINT((debug_rcv),
			    "klips_error:ipsec_rcv: "
			    "packet received from physical I/F (%s) not connected to ipsec I/F.  Cannot record stats.  May not have SA for decoding.  Is IPSEC traffic expected on this I/F?  Check routing.\n",
			    skb->dev ? (skb->dev->name ? skb->dev->name : "NULL") : "NULL");
	}

	KLIPS_IP_PRINT(debug_rcv, ipp);

	/* set up for decap loop */
	irs->stats= stats;
	irs->skb = skb;

	ipsec_rcv_decap(irs, 0);
	KLIPS_DEC_USE;
	return(0);
	
 rcvleave:
 	if(skb) {
	        ipsec_kfree_skb(skb);
	}

	KLIPS_DEC_USE;
	return(0);
}

int
#ifdef PROTO_HANDLER_SINGLE_PARM
ipsec_rcv(struct sk_buff *skb)
#else /* PROTO_HANDLER_SINGLE_PARM */
#ifdef NET_21
ipsec_rcv(struct sk_buff *skb, unsigned short xlen)
#else /* NET_21 */
ipsec_rcv(struct sk_buff *skb, struct net_device *dev, struct options *opt, 
		__u32 daddr_unused, unsigned short xlen, __u32 saddr,
                                   int redo, struct inet_protocol *protocol)
#endif /* NET_21 */
#endif
{
    return _ipsec_rcv(skb, NULL);
}

#ifdef NET_26
/*
 * this entry point is not a protocol entry point, so the entry
 * is a bit different.
 *
 * skb->iph->tot_len has been byte-swapped, and reduced by the size of
 *              the IP header (and options).
 * 
 * skb->h.raw has been pulled up the ESP header.
 *
 * skb->iph->protocol = 50 IPPROTO_ESP;
 *
 */
int klips26_udp_encap_rcv(struct sock *sk, struct sk_buff *skb)
{
	struct udp_sock *up = udp_sk(sk);
	struct udphdr *uh;
	struct iphdr *iph;
	int iphlen, len;
	int ret;

	__u8 *udpdata;
	__be32 *udpdata32;
	__u16 encap_type = up->encap_type;

	/* if this is not encapsulated socket, then just return now */
	if (!encap_type)
		return 1;

	/* If this is a paged skb, make sure we pull up
	* whatever data we need to look at. */
	len = skb->len - sizeof(struct udphdr);
	if (!pskb_may_pull(skb, sizeof(struct udphdr) + min(len, 8)))
		return 1;

	/* Now we can get the pointers */
	uh = udp_hdr(skb);
	udpdata = (__u8 *)(uh + 1);
	udpdata32 = (__be32 *)udpdata;

	switch (encap_type) {
	default:
	case UDP_ENCAP_ESPINUDP:
		KLIPS_PRINT(debug_rcv, "UDP_ENCAP_ESPINUDP_NON_IKE: len=%d 0x%x\n",
				len, udpdata32[0]);
		/* Check if this is a keepalive packet.  If so, eat it. */
		if (len == 1 && udpdata[0] == 0xff) {
			KLIPS_PRINT(debug_rcv,
					"UDP_ENCAP_ESPINUDP: keepalive packet detected\n");
			goto drop;
		} else if (len > sizeof(struct ip_esp_hdr) && udpdata32[0] != 0) {
			KLIPS_PRINT(debug_rcv,
					"UDP_ENCAP_ESPINUDP: ESP IN UDP packet detected\n");
			/* ESP Packet without Non-ESP header */
			len = sizeof(struct udphdr);
		} else {
			/* Must be an IKE packet.. pass it through */
			KLIPS_PRINT(debug_rcv,
					"UDP_ENCAP_ESPINUDP: IKE packet detected\n");
			return 1;
		}
		break;
	case UDP_ENCAP_ESPINUDP_NON_IKE:
		KLIPS_PRINT(debug_rcv, "UDP_ENCAP_ESPINUDP_NON_IKE: len=%d 0x%x\n",
				len, udpdata32[0]);
		/* Check if this is a keepalive packet.  If so, eat it. */
		if (len == 1 && udpdata[0] == 0xff) {
			KLIPS_PRINT(debug_rcv,
					"UDP_ENCAP_ESPINUDP_NON_IKE: keepalive packet detected\n");
			goto drop;
		} else if (len > 2 * sizeof(u32) + sizeof(struct ip_esp_hdr) &&
				udpdata32[0] == 0 && udpdata32[1] == 0) {
			KLIPS_PRINT(debug_rcv,
					"UDP_ENCAP_ESPINUDP_NON_IKE: ESP IN UDP NON IKE packet detected\n");
			/* ESP Packet with Non-IKE marker */
			len = sizeof(struct udphdr) + 2 * sizeof(u32);
		} else {
			/* Must be an IKE packet.. pass it through */
			KLIPS_PRINT(debug_rcv,
					"UDP_ENCAP_ESPINUDP_NON_IKE: IKE packet detected\n");
			return 1;
		}
		break;
	}

	/* At this point we are sure that this is an ESPinUDP packet,
	 * so we need to remove 'len' bytes from the packet (the UDP
	 * header and optional ESP marker bytes) and then modify the
	 * protocol to ESP, and then call into the transform receiver.
	 */
	if (skb_cloned(skb) && pskb_expand_head(skb, 0, 0, GFP_ATOMIC)) {
		KLIPS_PRINT(debug_rcv, "clone or expand problem\n");
		goto drop;
	}

	/* Now we can update and verify the packet length... */
	iph = ip_hdr(skb);
	iphlen = iph->ihl << 2;
	iph->tot_len = htons(ntohs(iph->tot_len) - len);
	if (skb->len < iphlen + len) {
		/* packet is too small!?! */
		KLIPS_PRINT(debug_rcv, "packet too small\n");
		goto drop;
	}

	/* pull the data buffer up to the ESP header and set the
	 * transport header to point to ESP.  Keep UDP on the stack
	 * for later.
	 */
	__skb_pull(skb, len);
	skb_reset_transport_header(skb);

	/* modify the protocol (it's ESP!) */
	iph->protocol = IPPROTO_ESP;

	/* process ESP */
	KLIPS_PRINT(debug_rcv, "starting processing ESP packet\n");
	ret = klips26_rcv_encap(skb, encap_type);
	return ret;

drop:
	kfree_skb(skb);
	return 0;
}

static struct net_device *get_ipsec_dev_by_phy_dev(struct net_device *phydev)
{
	struct ipsecpriv *priv;
	ipsec_dev_list *cur;

	if (!phydev)
		return NULL;

	for (cur = ipsec_dev_head; cur; cur = cur->next) {
		if ((priv = (struct ipsecpriv *)netdev_priv(cur->ipsec_dev)) &&
		    priv->dev && phydev->ifindex == priv->dev->ifindex) {
			return cur->ipsec_dev;
		}
	}

	return NULL;
}

int klips26_rcv_encap(struct sk_buff *skb, __u16 encap_type)
{
	struct ipsec_rcv_state nirs, *irs = &nirs;
	struct iphdr *ipp;
	extern struct net_device *ipsec_get_first_device(void);

	/* Don't unlink in the middle of a turnaround */
	KLIPS_INC_USE;

	memset(irs, 0, sizeof(*irs));

	/* fudge it so that all nat-t stuff comes from ipsec0    */
	/* eventually, the SA itself will determine which device
	 * it comes from
	 */ 
	{
		skb->dev = get_ipsec_dev_by_phy_dev(skb->dev) ? :
		    ipsec_get_first_device();
	}

	/* set up for decap loop */
	irs->hard_header_len = skb->dev->hard_header_len;

	skb = ipsec_rcv_unclone(skb, irs->hard_header_len);

#if IP_FRAGMENT_LINEARIZE
	/* In Linux 2.4.4, we may have to reassemble fragments. They are
	   not assembled automatically to save TCP from having to copy
	   twice.
	*/
	if (skb_is_nonlinear(skb)) {
		if (os_skb_linearize(skb) != 0) {
			goto rcvleave;
		}
	}
#endif /* IP_FRAGMENT_LINEARIZE */

	ipp = ip_hdr(skb);

	irs->iphlen = ipp->ihl << 2;

	KLIPS_IP_PRINT(debug_rcv, ipp);

	irs->stats= NULL;
	irs->skb = skb;

#ifdef CONFIG_IPSEC_NAT_TRAVERSAL
	switch(encap_type) {
	case UDP_ENCAP_ESPINUDP:
	  irs->natt_type = ESPINUDP_WITH_NON_ESP;
	  irs->natt_len = sizeof(struct udphdr);
	  break;
	  
	case UDP_ENCAP_ESPINUDP_NON_IKE:
	  irs->natt_type = ESPINUDP_WITH_NON_IKE;
	  irs->natt_len = sizeof(struct udphdr) + 2*sizeof(__u32);
	  break;
	  
	default:
	  if(printk_ratelimit()) {
	    printk(KERN_INFO "KLIPS received unknown UDP-ESP encap type %u\n",
		   encap_type);
	  }
	  return -1;
	}

	memmove((void *)(skb_network_header(skb) + irs->natt_len),
	    skb_network_header(skb), irs->iphlen);
	skb_push(skb, skb->data - skb_network_header(skb) - irs->natt_len);
#endif
	skb_reset_network_header(skb);

	ipsec_rcv_decap(irs, 0);
	KLIPS_DEC_USE;
	return 0;

rcvleave:
	if(skb) {
		ipsec_kfree_skb(skb);
	}
	KLIPS_DEC_USE;
	return 0;
}
#endif

#ifdef NET_26
struct inet_protocol ah_protocol = {
  .handler = ipsec_rcv,
  .no_policy = 1,
};
#else
struct inet_protocol ah_protocol =
{
	ipsec_rcv,				/* AH handler */
	NULL,				/* TUNNEL error control */
#ifdef NETDEV_25
	1,				/* no policy */
#else
	0,				/* next */
	IPPROTO_AH,			/* protocol ID */
	0,				/* copy */
	NULL,				/* data */
	"AH"				/* name */
#endif
};
#endif

#ifdef NET_26
struct inet_protocol esp_protocol = {
  .handler = ipsec_rcv,
  .no_policy = 1,
};
#else
struct inet_protocol esp_protocol = 
{
	ipsec_rcv,			/* ESP handler          */
	NULL,				/* TUNNEL error control */
#ifdef NETDEV_25
	1,				/* no policy */
#else
	0,				/* next */
	IPPROTO_ESP,			/* protocol ID */
	0,				/* copy */
	NULL,				/* data */
	"ESP"				/* name */
#endif
};
#endif

#if 0
/* We probably don't want to install a pure IPCOMP protocol handler, but
   only want to handle IPCOMP if it is encapsulated inside an ESP payload
   (which is already handled) */
#ifdef CONFIG_IPSEC_IPCOMP
struct inet_protocol comp_protocol =
{
	ipsec_rcv,			/* COMP handler		*/
	NULL,				/* COMP error control	*/
	0,				/* next */
	IPPROTO_COMP,			/* protocol ID */
	0,				/* copy */
	NULL,				/* data */
	"COMP"				/* name */
};
#endif /* CONFIG_IPSEC_IPCOMP */
#endif

