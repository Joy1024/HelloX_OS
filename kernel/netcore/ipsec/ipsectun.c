//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 18, 2022
//    Module Name               : ipsectun.c
//    Module Funciton           : 
//    Description               : 
//                                IPSec VTI's implementation code.
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

#include "ipsec/ipsec.h"
#include "ipsec/util.h"
#include "ipsectun.h"

/* Output routine for ipsec VTI, layer 3 mode. */
static int ipsectun_output_l3(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt,
	__COMMON_NETWORK_ADDRESS* comm_addr)
{
	return -1;
}

/* Output routine for ipsec VTI, layer 2 mode. */
static int ipsectun_output_l2(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt)
{
	return -1;
}

/* Input routine for ipsec VTI. */
static int ipsectun_input(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt)
{
	return -1;
}

/* Add one IPSec VTI into system. */
BOOL ipsectun_add(const char* if_name,
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
	pGenif->vti_type = vti_ipsec;
	pGenif->genif_input = ipsectun_input;
	pGenif->genif_l3_output = ipsectun_output_l3;
	pGenif->genif_l2_output = ipsectun_output_l2;
	pGenif->pGenifParent = pParent;
	pGenif->genif_mtu = IPSEC_MTU;

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

/* Change IPSec VTI's attributes. */
BOOL ipsectun_set(const char* if_name,
	const char* att_name,
	const char* att_value)
{
	return FALSE;
}
