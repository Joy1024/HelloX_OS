//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb 11, 2023
//    Module Name               : ipcomm.c
//    Module Funciton           : 
//    Description               : 
//                                Common functions of ip protocol suite,
//                                it's includes the checking sum, change
//                                tcp mss, and other functions.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>

#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "lwip/inet.h"
#include "lwip/inet_chksum.h"

#include "hx_inet.h"
#include "ipcomm.h"

/*  Show out an ip pkt. */
#ifdef NAT_DEBUG
static void show_ip_pkt(struct pbuf *p)
{
	struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
	u8_t *payload;

	payload = (u8_t *)iphdr + IP_HLEN;

	_hx_printf("[%s]IP header:\r\n", __func__);
	_hx_printf("+-------------------------------+\r\n");
	_hx_printf("|%2"S16_F" |%2"S16_F" |  0x%02"X16_F" |     %5"U16_F"     | (v, hl, tos, len)\r\n",
		IPH_V(iphdr),
		IPH_HL(iphdr),
		IPH_TOS(iphdr),
		ntohs(IPH_LEN(iphdr)));
	_hx_printf("+-------------------------------+\r\n");
	_hx_printf("|    %5"U16_F"      |%"U16_F"%"U16_F"%"U16_F"|    %4"U16_F"   | (id, flags, offset)\r\n",
		ntohs(IPH_ID(iphdr)),
		ntohs(IPH_OFFSET(iphdr)) >> 15 & 1,
		ntohs(IPH_OFFSET(iphdr)) >> 14 & 1,
		ntohs(IPH_OFFSET(iphdr)) >> 13 & 1,
		ntohs(IPH_OFFSET(iphdr)) & IP_OFFMASK);
	_hx_printf("+-------------------------------+\r\n");
	_hx_printf("|  %3"U16_F"  |  %3"U16_F"  |    0x%04"X16_F"     | (ttl, proto, chksum)\r\n",
		IPH_TTL(iphdr),
		IPH_PROTO(iphdr),
		ntohs(IPH_CHKSUM(iphdr)));
	_hx_printf("+-------------------------------+\r\n");
	_hx_printf("|  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  | (src)\r\n",
		ip4_addr1_16(&iphdr->src),
		ip4_addr2_16(&iphdr->src),
		ip4_addr3_16(&iphdr->src),
		ip4_addr4_16(&iphdr->src));
	_hx_printf("+-------------------------------+\r\n");
	_hx_printf("|  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  | (dest)\r\n",
		ip4_addr1_16(&iphdr->dest),
		ip4_addr2_16(&iphdr->dest),
		ip4_addr3_16(&iphdr->dest),
		ip4_addr4_16(&iphdr->dest));
	_hx_printf("+-------------------------------+\r\n");
}
#else
#define show_ip_pkt(x)
#endif //NAT_DEBUG

/* Update UDP datagram's check sum accordingly. */
static void _udp_check_sum(struct ip_hdr* p, struct pbuf* pb)
{
	struct udp_hdr* pUdpHdr = NULL;
	int iph_len = IPH_HL(p);

	LWIP_UNUSED_ARG(pb);

	iph_len *= 4;
	pUdpHdr = (struct udp_hdr*)((char*)p + iph_len);
	__NATDEBUG("%s: reset UDP chksum from [%x] to 0.\r\n",
		__func__, pUdpHdr->chksum);
	pUdpHdr->chksum = 0; /* Reset check sum. */
}

/* Update TCP segment's check sum. */
static void _tcp_check_sum(struct ip_hdr* p, struct pbuf* pb)
{
	struct tcp_hdr* pTcpHdr = NULL;
	int iph_len = IPH_HL(p);
	int ip_len = ntohs(IPH_LEN(p)); /* Total IP packet's length. */
	int chksum = 0;
	ip_addr_t src, dest;

	iph_len *= 4;
	/* Locate TCP header. */
	pTcpHdr = (struct tcp_hdr*)((char*)p + iph_len);

	/* Validate pbuf object. */
	if (pb->tot_len < (iph_len + sizeof(struct tcp_hdr)))
	{
		return;
	}

	/* Validate IP packet's total length. */
	if (ip_len < (iph_len + (int)sizeof(struct tcp_hdr)))
	{
		return;
	}

	/*
	* Reset TCP header's check sum since it's source address
	* or source port is changed after NATing.
	*/
	chksum = pTcpHdr->chksum;
	ip_addr_copy(src, p->src);
	ip_addr_copy(dest, p->dest);
	pbuf_header(pb, -iph_len); /* move to TCP header. */
	pTcpHdr->chksum = 0; /* Reset the original check sum. */
	pTcpHdr->chksum = inet_chksum_pseudo(pb, &src, &dest, IP_PROTO_TCP,
		ip_len - iph_len);
	pbuf_header(pb, iph_len); /* move back. */
	__NATDEBUG("%s: TCP check sum updated[%X] -> [%X]\r\n",
		__func__,
		chksum,
		pTcpHdr->chksum);
}

/* Local helper to recalculate the ICMP checksum value. */
static void _icmp_check_sum(struct ip_hdr* pHdr, struct pbuf* pb)
{
	struct icmp_echo_hdr* pIcmpHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pHdr);
	BUG_ON(NULL == pb);

	iph_len = IPH_HL(pHdr);
	iph_len *= 4;
	/* Validate the pbuf. */
	if (pb->tot_len < (iph_len + sizeof(u16_t) * 2))
	{
		/* pbuf length less than the sum of IP header and minimal ICMP hdr. */
		_hx_printf("%s:too short ICMP packet(len = %d)\r\n", pb->tot_len);
		return;
	}
	/* Locate ICMP header. */
	pIcmpHdr = (struct icmp_echo_hdr*)((char*)pHdr + iph_len);
	/* Reset the original ICMP checksum value. */
	pIcmpHdr->chksum = 0;
	/* Move pb to ICMP header. */
	pbuf_header(pb, -iph_len);
	/* Calculate checksum. */
	pIcmpHdr->chksum = inet_chksum_pbuf(pb);
	/* Move back. */
	pbuf_header(pb, iph_len);
}

/* Local helper to re-calculate IP header's check sum. */
void _iphdr_check_sum(struct ip_hdr* p, struct pbuf* pb)
{
	u16_t chksum = 0;

	LWIP_UNUSED_ARG(pb);
	BUG_ON(NULL == p);
	p->_chksum = 0; /* Reset check sum value. */
	chksum = _hx_checksum((__u16*)p, sizeof(struct ip_hdr));
	p->_chksum = chksum;
	switch (p->_proto)
	{
	case IP_PROTO_UDP:
		_udp_check_sum(p, pb);
		break;
	case IP_PROTO_ICMP:
		_icmp_check_sum(p, pb);
		break;
	case IP_PROTO_TCP:
		_tcp_check_sum(p, pb);
		break;
	case IP_PROTO_IGMP:
		break;
	case IP_PROTO_UDPLITE:
		break;
	default:
		break;
	}
}

/* 
 * Adjust tcp's mss according genif's mtu. 
 * TRUE will be returned only the mss is
 * adjusted in this routine. Any other case,
 * include the mss is found but not adjusted,
 * will return FALSE.
 */
BOOL tcp_mss_adjust(__GENERIC_NETIF* pGenif, struct tcp_hdr* pTcpHdr)
{
	unsigned char* tcp_opt = NULL;
	int data_off = 0, total_opt_len = 0;
	unsigned char opt_len = 0;
	u16_t mss = 0, old_mss = 0, mtu = 0;

	BUG_ON((NULL == pGenif) || (NULL == pTcpHdr));

	/* Get the interface's mtu. */
	mtu = pGenif->genif_mtu;
	if (0 == mtu)
	{
		__LOG("[%s]invalid genif mtu[%s]\r\n",
			__func__,
			pGenif->genif_name);
		return FALSE;
	}

	/* Get data offset from TCP header. */
	data_off = TCPH_OFFSET(pTcpHdr);
	if (data_off <= TCP_FIXED_HEADER_LEN) /* No option or invalid TCP header. */
	{
		return FALSE;
	}
	total_opt_len = data_off - TCP_FIXED_HEADER_LEN;
	tcp_opt = ((unsigned char*)pTcpHdr + TCP_FIXED_HEADER_LEN);
	while (total_opt_len)
	{
		switch (*tcp_opt) /* Option kind value. */
		{
		case TCP_OPTION_KIND_END:
			return FALSE;
		case TCP_OPTION_KIND_NOP:
			tcp_opt++;
			total_opt_len--;
			break;
		case TCP_OPTION_KIND_MSS:
			/* Get the old MSS value. */
			tcp_opt += 2;
			old_mss = ntohs(*(u16_t*)tcp_opt);
			/* Get the new MSS value. */
			mss = mtu;
			mss -= TCP_FIXED_HEADER_LEN;
			mss -= IP_FIXED_HEADER_LEN;
			/* Change old MSS value if necessary. */
			if (old_mss > mss)
			{
				*(u16_t*)tcp_opt = htons(mss);
				//_hx_printf("[%s]Adjust TCP MSS: [%d]-->[%d]\r\n", 
				//	__func__,
				//	old_mss, mss);
			}
			return TRUE;
		default:
			opt_len = *(tcp_opt + 1);
			/* Option's length can not be 0. */
			if (0 == opt_len)
			{
				return FALSE;
			}
			tcp_opt += opt_len;
			if (total_opt_len < opt_len) /* Ilegal case. */
			{
				return FALSE;
			}
			total_opt_len -= opt_len;
			break;
		}
	}
	return FALSE;
}
