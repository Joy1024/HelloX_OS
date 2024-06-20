//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 14, 2022
//    Module Name               : genip.c
//    Module Funciton           : 
//    Description               : 
//                                General IP input routine's implementation
//                                code.
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

#include "netcfg.h"
#include "proto.h"
#include "genif.h"
#include "netmgr.h"
#include "netglob.h"
#include "lwip/ip.h"
#include "lwip/tcpip.h"
#include "lwip/inet.h"

/* 
 * Local helper to check if a packet is 
 * desting for this interface. 
 * The packet is interpreted as ethernet, IP, or
 * others according to genif's type, and
 * pbuf p also is modified(pbuf_header) to point
 * to IP header.
 */
static BOOL __packet_is_for_me(struct pbuf* p, __GENERIC_NETIF* pGenif, int* adjusted_len)
{
	BOOL bResult = FALSE;
	struct ip_hdr* pIpHdr = NULL;
	int adjust_hdr = 0;

	/* Interpret p according interface link level type. */
	if (pGenif->link_type == lt_ethernet)
	{
		/* p is a ethernet frame, so skip ethernet header. */
		pIpHdr = (struct ip_hdr*)((char*)p->payload + sizeof(__ETHERNET_II_HEADER));
		adjust_hdr = sizeof(__ETHERNET_II_HEADER);
	}
	else if (pGenif->link_type == lt_tunnel)
	{
		pIpHdr = (struct ip_hdr*)p->payload;
	}
	else if (pGenif->link_type == lt_vlanif)
	{
		/* Skip the VLAN header. */
	}
	else {
		pIpHdr = (struct ip_hdr*)p->payload;
	}

	/* Check if the dest's addr is same as us. */
	if (pIpHdr->dest.addr == pGenif->ip_addr[0].addr)
	{
		/* Packet is for this interface, adjust pbuf accordingly. */
		pbuf_header(p, -adjust_hdr);
		*adjusted_len = adjust_hdr;
		bResult = TRUE;
		goto __TERMINAL;
	}

__TERMINAL:
	return bResult;
}

/*
 * General IP input routine.
 * All incoming IP packet will be delivered to this routine
 * to process. It's the replacement of tcpip_input for lwIP.
 */
err_t general_ip_input(struct pbuf *p, struct netif *inp)
{
	__GENERIC_NETIF* pGenif = NULL;
	__GENERIC_NETIF* pChild = NULL;
	int adjust_len = 0;

	BUG_ON((NULL == p) || (NULL == inp));
	pGenif = inp->pGenif;
	BUG_ON(NULL == pGenif);

	if (!__packet_is_for_me(p, pGenif, &adjust_len))
	{
		/* Packet is not for this interface. */
		return tcpip_input(p, inp);
	}

	/* 
	 * Check if our child interface can eat the packet. 
	 * Use network manager's spin lock to protect genif 
	 * hirarchy architecture, since it's maybe changed 
	 * else wehere.
	 */
	__ACQUIRE_SPIN_LOCK(NetworkManager.spin_lock);
	pChild = pGenif->pGenifChild;
	while (pChild)
	{
		if (pChild->packet_for_me(pGenif, pChild, p))
		{
			/* Child can accept it, give it. */
			__RELEASE_SPIN_LOCK(NetworkManager.spin_lock);
			pChild->genif_input(pChild, p);
			return 0;
		}
		/* Consult next slibing genif. */
		pChild = pChild->pGenifSibling;
	}
	__RELEASE_SPIN_LOCK(NetworkManager.spin_lock);

	/* 
	 * No child want it, delivery to lwIP stack. 
	 * Adjust back to original header since the pbuf
	 * is changed in packet_is_for_me routine.
	 */
	pbuf_header(p, adjust_len);
	return tcpip_input(p, inp);
}

/*
 * General IP output routine,it's used to initialize
 * the genif_l3_output, if the interface is a common
 * IP interface, no need special packet processing.
 * For VTI or other genif with special packet processing,
 * new genif_l3_output should be implemented to
 * carry out special action on packets.
 * It just invoke the output routine of concrete protocol's
 * output.
 */
int general_ip_output(__GENERIC_NETIF* pGenif, struct pbuf* pkt,
	__COMMON_NETWORK_ADDRESS* comm_addr)
{
	struct netif* netif = NULL;
	__u8 addr_type = NETWORK_ADDRESS_TYPE_NONE;
	struct ip_hdr* pIpHdr = NULL;
	ip_addr_t ip_addr;

	BUG_ON((NULL == pGenif) || (NULL == pkt) || (NULL == comm_addr));

	/* Invoke the corresponding protocol's output routine. */
	addr_type = comm_addr->AddressType;
	switch (addr_type)
	{
	case NETWORK_ADDRESS_TYPE_IPV4:
		netif = (struct netif*)pGenif->proto_binding[0].pIfState;
		BUG_ON(NULL == netif);
		ip_addr.addr = comm_addr->Address.ipv4_addr;
		/* For debugging. */
		//pIpHdr = (struct ip_hdr*)pkt->payload;
		//_hx_printf("[%s]packet is from[%s] ", __func__, inet_ntoa(pIpHdr->src));
		//_hx_printf("to [%s] through netif [%c%c]\r\n", 
		//	inet_ntoa(ip_addr), netif->name[0], netif->name[1]);

		return netif->output(netif, pkt, &ip_addr);
	default:
		/* Not implemented yet. */
		BUG();
	}

	return -1;
}

/*
 * Apply all call backs of a genif.
 * It invokes the corresponding call back routine
 * according genif flags.
 * Use genif's spin lock to protect genif's flags
 * and callback routine, since they maybe changed
 * anytime.
 */
int genif_apply_callback(__GENERIC_NETIF* pGenif, struct pbuf* pkt)
{
	BUG_ON(NULL == pGenif);
	unsigned long ulFlags = 0;

	/* Invoke call back routines according flags. */
	__ENTER_CRITICAL_SECTION_SMP(pGenif->spin_lock, ulFlags);
	if (pGenif->genif_flags & GENIF_FLAG_CAPTURE)
	{
		BUG_ON(NULL == pGenif->capture_callback);
		pGenif->capture_callback(pGenif, pkt, pGenif->capture_param);
	}
	__LEAVE_CRITICAL_SECTION_SMP(pGenif->spin_lock, ulFlags);

	return 1;
}
