//***********************************************************************/
//    Author                    : Garry
//    Original Date             : DEC 25, 2022
//    Module Name               : genifpp.c
//    Module Funciton           : 
//                                Shadow genif for pppoe constructed netif.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "genifpp.h"
#include "netmgr.h"
#include "routmgr/rtmgr.h"

/* Input routine for pppoe genif. */
static int pppoe_genif_input(__GENERIC_NETIF* pGenif, struct pbuf* p)
{
	return -1;
}

/* Layer 2 output routine for pppoe genif. */
static int pppoe_genif_l2_output(__GENERIC_NETIF* pGenif, struct pbuf* p)
{
	return -1;
}

/*
 * Initializes the pppoe constructed netif object.
 * Create a new genif, associated with the netif, and
 * register it into system.
 */
BOOL __genif_pp_init(struct netif* netif, ip_addr_t* if_addr, ip_addr_t* if_mask)
{
	__GENERIC_NETIF* pGenif = NULL;
	BOOL bResult = FALSE;

	BUG_ON((NULL == netif) || (NULL == if_addr) || (NULL == if_mask));

	/* Release the old genif if exist. */
	if (netif->pGenif)
	{
		NetworkManager.ReleaseGenif(netif->pGenif);
		netif->pGenif = NULL;
	}

	/* Create a new netif. */
	pGenif = NetworkManager.CreateGenif(NULL, NULL);
	if (NULL == pGenif)
	{
		__LOG("can not create pppoe genif.\r\n");
		goto __TERMINAL;
	}

	/* Init the genif. */
	strcpy(pGenif->genif_name, GENIF_NAME_GENIFPP);
	pGenif->genif_input = pppoe_genif_input;
	pGenif->genif_l2_output = pppoe_genif_l2_output;
	pGenif->link_type = lt_shadow;
	pGenif->genif_mtu = netif->mtu;

	/* Register genif into system. */
	if (!NetworkManager.RegisterGenif(pGenif))
	{
		__LOG("can not register pppoe genif.\r\n");
		goto __TERMINAL;
	}

	/* Link genif and netif together. */
	pGenif->proto_binding[0].pIfState = netif;
	netif->pGenif = pGenif;

	/* Map genif's flags into netif's. */
	if (pGenif->genif_flags & GENIF_FLAG_NAT)
	{
		netif->flags |= NETIF_FLAG_NAT;
	}

	/* Set genif's ip addr manually. */
	IP4_ADDR(&pGenif->ip_gw[0], 0, 0, 0, 0);
	pGenif->ip_addr[0].addr = if_addr->addr;
	pGenif->ip_mask[0].addr = if_mask->addr;

	/* Add the corresponding route entry. */
	if (!RoutingManager.add_iproute(&pGenif->ip_addr[0], &pGenif->ip_mask[0],
		ROUTE_METRIC_DIRECTLY,
		NULL, 
		IP_ROUTE_TYPE_DIRECT,
		pGenif->genif_name))
	{
		__LOG("can not add loopback route entry.\r\n");
		goto __TERMINAL;
	}

	/* Pull up the genif's link status. */
	NetworkManager.LinkStatusChange(pGenif, up, full, _1000M);

	bResult = TRUE;

__TERMINAL:
	return bResult;
}
