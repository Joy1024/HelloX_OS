//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 18, 2022
//    Module Name               : gretun.c
//    Module Funciton           : 
//    Description               : 
//                                GRE VTI's implementation code.
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
#include <string.h>
#include <stdlib.h>

#include <genif.h>
#include <proto.h>
#include <netmgr.h>
#include <lwip/ip.h>
#include <lwip/inet.h>

#include "ipsec/ipsec.h"
#include "ipsec/util.h"
#include "gretun.h"

/* If we can accept the packet. */
static BOOL gre_packet_for_me(__GENERIC_NETIF* genif_parent,
	__GENERIC_NETIF* genif_this, struct pbuf* pkt)
{
	struct ip_hdr* pIpHdr = NULL;

	BUG_ON(NULL == genif_parent || NULL == genif_this || NULL == pkt);

	/* 
	 * Check the IP header.
	 * If the destination address is vti source, and the protocol is
	 * GRE, then we can accept it.
	 */
	pIpHdr = (struct ip_hdr*)pkt->payload;

	/* For debugging. */
	_hx_printf("[%s]packet is from[%s]\r\n", __func__,
		inet_ntoa(pIpHdr->src));

	if (pIpHdr->_proto == IP_PROTO_GRE)
	{
		if (pIpHdr->src.addr == genif_this->vti_source.addr)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/* Output routine for GRE VTI, for Layer 3 mode. */
static int gretun_output_l3(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt,
	__COMMON_NETWORK_ADDRESS* comm_addr)
{
	struct ip_hdr* pIpHdr = NULL;
	pIpHdr = (struct ip_hdr*)pkt->payload;
	__GENERIC_NETIF* pParent = NULL;

	/* For debugging. */
	_hx_printf("[%s]packet is to[%s]\r\n", __func__,
		inet_ntoa(pIpHdr->dest));

	/* Just send out directly through parent. */
	pParent = pGenif->pGenifParent;
	BUG_ON(NULL == pParent);
	return pParent->genif_l3_output(pParent, pkt, comm_addr);
}

/* Output routine for GRE VTI, for Layer 2 mode. */
static int gretun_output_l2(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt)
{
	return -1;
}

/* Input routine for GRE VTI. */
static int gretun_input(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt)
{
	struct ip_hdr* pIpHdr = NULL;
	__GRE_HEADER* pGreHdr = NULL;
	int iph_len = 0;

	BUG_ON(NULL == pGenif || NULL == pkt);
	pIpHdr = (struct ip_hdr*)pkt->payload;
	BUG_ON(IP_PROTO_GRE != pIpHdr->_proto);

	/* Get GRE's header. */
	iph_len = IPH_LEN(pIpHdr);
	iph_len *= 4;
	if (0 == iph_len)
	{
		__LOG("[%s]invalid IP header: len = 0\r\n", __func__);
		goto __TERMINAL;
	}
	pGreHdr = (__GRE_HEADER*)((char*)pkt->payload + iph_len);

	/* Show this GRE pkt. */
	_hx_printf("[%s]gre version: %d, proto: 0x%X\r\n",
		__func__, pGreHdr->_ver, pGreHdr->_proto);

__TERMINAL:
	return -1;
}

/* Specific show routine of GRE VTI. */
static void gre_specific_show(__GENERIC_NETIF* genif)
{
	return;
}

/* Add one GRE VTI into system. */
BOOL gretun_add(const char* if_name,
	const char* parent_name,
	const char* address)
{
	ip_addr_t ipaddr, mask, gw;
	__COMMON_NETWORK_ADDRESS commaddr[3];
	__GENERIC_NETIF* pParent = NULL;
	__GENERIC_NETIF* pGenif = NULL;
	BOOL bResult = FALSE;

	BUG_ON((NULL == if_name) || (NULL == parent_name));
	BUG_ON(NULL == address);

	if (strlen(if_name) >= GENIF_NAME_LENGTH)
	{
		/* Name is too long. */
		goto __TERMINAL;
	}
	if (strlen(parent_name) >= GENIF_NAME_LENGTH)
	{
		goto __TERMINAL;
	}
	/* Validate the source and destination address. */
	if (!ipaddr_aton(address, &ipaddr))
	{
		goto __TERMINAL;
	}

	/* Get the parent interface. */
	pParent = NetworkManager.GetGenifByName(parent_name);
	if (NULL == pParent)
	{
		_hx_printf("[%s]invalid parent name.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Create a new generic interface. */
	pGenif = NetworkManager.CreateGenif(pParent, NULL);
	if (NULL == pGenif)
	{
		_hx_printf("[%s]create new genif failed.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Initialize it. */
	strcpy(pGenif->genif_name, if_name);
	pGenif->link_type = lt_tunnel;
	pGenif->vti_type = vti_gre;
	pGenif->packet_for_me = gre_packet_for_me;
	pGenif->genif_input = gretun_input;
	pGenif->genif_l2_output = gretun_output_l2;
	pGenif->genif_l3_output = gretun_output_l3;
	pGenif->specific_show = gre_specific_show;
	pGenif->pGenifParent = pParent;
	pGenif->genif_mtu = GRE_DEFAULT_MTU;

	/* Register it to system. */
	if (!NetworkManager.RegisterGenif(pGenif))
	{
		_hx_printf("[%s]failed to register genif.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Set it's ip address. */
	ipaddr_aton("255.255.255.255", &mask);
	ipaddr_aton("0.0.0.0", &gw);
	commaddr[0].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	commaddr[0].Address.ipv4_addr = ipaddr.addr;
	commaddr[1].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	commaddr[1].Address.ipv4_addr = mask.addr;
	commaddr[2].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	commaddr[2].Address.ipv4_addr = gw.addr;
	NetworkManager.AddGenifAddress(pGenif->if_index, 
		NETWORK_PROTOCOL_TYPE_IPV4, 
		commaddr, 3, FALSE);

	/* Set the interface's status as UP. */
	NetworkManager.LinkStatusChange(pGenif, up, full, _100G);

	/* Everything is OK. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		/* Release the parent if got. */
		if (pParent)
		{
			NetworkManager.ReleaseGenif(pParent);
		}
		/* Release the genif if created. */
		if (pGenif)
		{
			NetworkManager.ReleaseGenif(pGenif);
		}
	}
	return bResult;
}

/* Change GRE VTI's attributes. */
BOOL gretun_set(const char* if_name,
	const char* att_name,
	const char* att_value)
{
	return FALSE;
}
