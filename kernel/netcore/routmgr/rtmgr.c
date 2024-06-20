//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 30, 2022
//    Module Name               : rtmgr.c
//    Module Funciton           : 
//                                Routing manager's implementation code.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <stdio.h>
#include <lwip/inet.h>

#include "netmgr.h"
#include "iprte.h"
#include "rtmgr.h"

/* 
 * Local helper to add ip route. 
 * Algorithm:
 * 1. Use prefix length as key to travel the whole routing
 *    table to locate the position this route should be inserted;
 * 2. Compare all route entries with this route, if find one with
 *    same dest/mask, then compare the metric, who with the low
 *    metric value will win;
 * 3. If find a entry with same dest/mask and metric, then add
 *    the genif into out-going interface list;
 * 4. If no route find with same dest/mask, then just insert the
 *    route into table and return.
 * Return value:
 *  A route entry base address:
 *  1. NULL: The route entry is added into system;
 *  2. No-NULL: pRouteEntry, or other route entry that
 *     replaced by the pRouteEntry. If dest/mask/interface
 *     all are same but metric is higher than pRouteEntry,
 *     then the old one will be returned.
 *  The caller should release the route entry this function
 *  returns.
 */
static __IP_RTE* __do_add_iproute(__IP_RTE* pRouteEntry)
{
	__IP_ROUTING_TABLE* rout_table = NULL;
	__IP_RTE* curr = NULL;
	__IP_RTE* rte_return = NULL;

	BUG_ON(NULL == pRouteEntry);
	rout_table = &RoutingManager.ip_rt;

	/* Acquire the rw spin lock. */
	__RW_SLOCK_WLOCK(rout_table->rw_lock);
	curr = rout_table->rte_head.pPrev;
	while ((curr->mask_len < pRouteEntry->mask_len) &&
		(curr != &rout_table->rte_head))
	{
		curr = curr->pPrev;
	}
	while ((curr->mask_len == pRouteEntry->mask_len) &&
		(curr != &rout_table->rte_head))
	{
		if ((curr->ip_dest.addr & curr->ip_mask.addr) ==
			(pRouteEntry->ip_dest.addr & pRouteEntry->ip_mask.addr))
		{
			/* Find the same route, use metric to break it. */
			if (curr->metric <= pRouteEntry->metric)
			{
				/* The new one is not the best. */
				rte_return = pRouteEntry;
				__RW_SLOCK_WUNLOCK(rout_table->rw_lock);
				goto __TERMINAL;
			}
			else {
				/* Use new one to replace old one. */
				pRouteEntry->pPrev = curr->pPrev;
				pRouteEntry->pNext = curr->pNext;
				curr->pPrev->pNext = pRouteEntry;
				curr->pNext->pPrev = pRouteEntry;
				rte_return = curr;
				__RW_SLOCK_WUNLOCK(rout_table->rw_lock);
				goto __TERMINAL;
			}
		}
		curr = curr->pPrev;
	}
	/* Now ECMP exist, and just insert current position. */
	pRouteEntry->pPrev = curr;
	pRouteEntry->pNext = curr->pNext;
	curr->pNext->pPrev = pRouteEntry;
	curr->pNext = pRouteEntry;
	rout_table->rte_num++;
	__RW_SLOCK_WUNLOCK(rout_table->rw_lock);

__TERMINAL:
	return rte_return;
}

/* Handlers of deleting a route entry. */
static BOOL __do_del_iproute(ip_addr_t* dest, ip_addr_t* mask)
{
	__IP_RTE* pRouteEntry = NULL;
	__IP_ROUTING_TABLE* pRouteTable = NULL;
	BOOL bResult = FALSE;

	BUG_ON((NULL == dest) || (NULL == mask));
	pRouteTable = &RoutingManager.ip_rt;

	/* Find and delete the specified route entry. */
	__RW_SLOCK_WLOCK(pRouteTable->rw_lock);
	pRouteEntry = pRouteTable->rte_head.pPrev;
	while (pRouteEntry != &pRouteTable->rte_head) 
	{
		if ((pRouteEntry->ip_dest.addr == dest->addr) &&
			(pRouteEntry->ip_mask.addr == mask->addr))
		{
			/* Find the route entry, delete it. */
			pRouteEntry->pNext->pPrev = pRouteEntry->pPrev;
			pRouteEntry->pPrev->pNext = pRouteEntry->pNext;
			pRouteTable->rte_num--;
			bResult = TRUE;
			break;
		}
		pRouteEntry = pRouteEntry->pPrev;
	}
	__RW_SLOCK_WUNLOCK(pRouteTable->rw_lock);

	/* Release the route entry's resource if success. */
	if (bResult)
	{
		NetworkManager.ReleaseGenif(pRouteEntry->genif_out[0]);
		_hx_free(pRouteEntry);
	}

	return bResult;
}

/*
 * Remove all static routes whose out
 * going interface is the specified genif.
 * When genif's link status change to down,
 * this routine should be invoked.
 */
static void __remove_genif_sroute(__GENERIC_NETIF* pGenif)
{
	__IP_RTE* pRouteEntry = NULL;
	__IP_RTE* pNextEntry = NULL;
	__IP_ROUTING_TABLE* pRouteTable = NULL;

	pRouteTable = &RoutingManager.ip_rt;
	/* 
	 * Travel the whole routing table to
	 * find all route entries and delete
	 * them.
	 */
	__RW_SLOCK_WLOCK(pRouteTable->rw_lock);
	pRouteEntry = pRouteTable->rte_head.pPrev;
	while (pRouteEntry != &pRouteTable->rte_head)
	{
		if (pRouteEntry->genif_out[0] == pGenif)
		{
			/* Find the route entry, delete it. */
			pRouteEntry->pNext->pPrev = pRouteEntry->pPrev;
			pRouteEntry->pPrev->pNext = pRouteEntry->pNext;
			pRouteTable->rte_num--;
			/* Release associated genif. */
			NetworkManager.ReleaseGenif(pRouteEntry->genif_out[0]);
			pNextEntry = pRouteEntry->pPrev;
			_hx_free(pRouteEntry);
			pRouteEntry = pNextEntry;
			continue;
		}
		pRouteEntry = pRouteEntry->pPrev;
	}
	__RW_SLOCK_WUNLOCK(pRouteTable->rw_lock);
}

/* Handler of genif state change message. */
static BOOL __genif_state_change(__GENERIC_NETIF* pGenif, unsigned short change_flags)
{
	BOOL bResult = FALSE;

	/* Apply different actions according change flags. */
	if (change_flags & GENIF_STATE_LINKSTATUSUP)
	{
		if (!InstallGenifRoute(pGenif->genif_name))
		{
			goto __TERMINAL;
		}
	}
	if (change_flags & GENIF_STATE_LINKSTATUSDOWN)
	{
		/* 
		 * Remove all static routes associated 
		 * with the genif. 
		 */
		__remove_genif_sroute(pGenif);
	}
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* 
 * Dedicated thread of the routing manager. 
 * Routing table management, and other task
 * are handled in this thread, thus we make
 * all routing management in thread context.
 * Routing table operations in interrupt context,
 * such as add routing entry, is delegated to
 * this thread by message. This mechanism can
 * avoid dead lock since we use read-write
 * spin lock to protect routing tables.
 */
static unsigned long __routing_manager_thread(LPVOID* pData)
{
	__KERNEL_THREAD_MESSAGE msg;
	__IP_RTE* pRouteEntry = NULL;
	__GENERIC_NETIF* pGenif = NULL;
	unsigned short change_flags = 0;

	while (TRUE)
	{
		if (GetMessage(&msg))
		{
			/* Process the msg. */
			switch (msg.wCommand)
			{
			case RTMGR_MSG_ADDIPROUTE:
				/* Add ip route. */
				pRouteEntry = (__IP_RTE*)msg.dwParam;
				BUG_ON(NULL == pRouteEntry);
				pRouteEntry = __do_add_iproute(pRouteEntry);
				if (pRouteEntry)
				{
					/* Should release it. */
					BUG_ON(NULL == pRouteEntry->genif_out[0]);
					NetworkManager.ReleaseGenif(pRouteEntry->genif_out[0]);
					_hx_free(pRouteEntry);
				}
				break;
			case RTMGR_MSG_DELIPROUTE:
				/* Del ip route. */
				pRouteEntry = (__IP_RTE*)msg.dwParam;
				BUG_ON(NULL == pRouteEntry);
				__do_del_iproute(&pRouteEntry->ip_dest, &pRouteEntry->ip_mask);
				/* 
				 * Release route entry object, it's just used 
				 * to xfer parameters, no genif assicaited with it. 
				 */
				_hx_free(pRouteEntry);
				break;
			case RTMGR_MSG_ADDIP6ROUTE:
				/* Add ipv6 route. */
				break;
			case RTMGR_MSG_DELIP6ROUTE:
				/* Del ipv6 route. */
				break;
			case RTMGR_MSG_SHOWIPROUTE:
				/* Print out all IP route. */
				break;
			case RTMGR_MSG_SHOWIP6ROUTE:
				/* Print out all ipv6 route. */
				break;
			case RTMGR_MSG_GENIFSTATECHANGE:
				/* Genif's state changed. */
				pGenif = (__GENERIC_NETIF*)msg.dwParam;
				BUG_ON(NULL == pGenif);
				change_flags = msg.wParam;
				if (change_flags)
				{
					__genif_state_change(pGenif, change_flags);
				}
				/* 
				 * Release the genif object since it's 
				 * obtained in API routine of GenifStateChange.
				 */
				NetworkManager.ReleaseGenif(pGenif);
				break;
			default:
				__LOG("[%s]unknown message type[%d]\r\n", __func__,
					msg.wCommand);
				break;
			}
		}
	}

	return 0;
}

/* Init routine of routing manager. */
static BOOL __Initialize(struct tag__ROUTING_MANAGER* rtmgr)
{
	__KERNEL_THREAD_OBJECT* rm_thread = NULL;
	ip_addr_t dest, mask;

	BUG_ON(NULL == rtmgr);

	/* Init routing table. */
	memset(&rtmgr->ip_rt.rte_head, 0, sizeof(__IP_RTE));
	rtmgr->ip_rt.rte_head.pNext = rtmgr->ip_rt.rte_head.pPrev = &rtmgr->ip_rt.rte_head;
	rtmgr->ip_rt.rte_num = 0;
	rtmgr->ip_rt.miss_count = 0;
	__INIT_RW_SPIN_LOCK(rtmgr->ip_rt.rw_lock);

	/* Create the dedicated routing management thread. */
	rm_thread = (__KERNEL_THREAD_OBJECT*)CreateKernelThread(0, 
		KERNEL_THREAD_STATUS_READY, 
		PRIORITY_LEVEL_NORMAL,
		__routing_manager_thread,
		NULL, NULL, "routing_mgr");
	if (NULL == rm_thread)
	{
		__LOG("[%s]create rm-thread fail.\r\n", __func__);
		return FALSE;
	}
	rtmgr->rm_thread = rm_thread;

	/* 
	 * Install the route entry for loopback interface. 
	 * The loopback genif is created and registered with
	 * loopback netif in lwIP, this procedure is earlier
	 * than routing manager's initialization, so we postpone
	 * to here to add the loopback route entry.
	 * If we add the route entry just after loopback genif
	 * is registered, it will lead system crash since
	 * routing manager is not initialized yet.
	 */
	IP4_ADDR(&dest, 127, 0, 0, 1);
	IP4_ADDR(&mask, 255, 0, 0, 0);
	RoutingManager.add_iproute(&dest, &mask, ROUTE_METRIC_DIRECTLY,
		NULL, IP_ROUTE_TYPE_DIRECT,
		GENIF_NAME_LOOP);

	return TRUE;
}

/* 
 * Add one IP route entry. 
 * This routine is invoked by several scenarios:
 *   1. A genif's status is changed to up from down, so the routine is
 *      invoked in LinkStatusChange routine of protocol;
 *   2. Valid address is set to genif, so the routine is called
 *      in AddGenifAddress routine of protocol;
 *   3. User can add route entry through command line, so this 
 *      routine could be invoked in command handler;
 *   4. Other positions to fit lwIP(currently temporary solution), 
 *      such as in pppoe session up's handler routine.
 */
static BOOL __add_iproute(ip_addr_t* dest, ip_addr_t* mask,
	int metric, ip_addr_t* nexthop,
	__IP_ROUTE_TYPE route_type,
	const char* pGenifName)
{
	__IP_RTE* pRouteEntry = NULL;
	__GENERIC_NETIF* pGenif = NULL;
	uint32_t addr = 0;
	__KERNEL_THREAD_MESSAGE msg;
	BOOL bResult = FALSE;

	BUG_ON((NULL == dest) || (NULL == mask) || (NULL == pGenifName));

	/* Get the genif by it's name. */
	pGenif = NetworkManager.GetGenifByName(pGenifName);
	if (NULL == pGenif)
	{
		__LOG("[%s]No genif[%s] found in system.\r\n", __func__, pGenifName);
		goto __TERMINAL;
	}

	/* Allocate a new route entry and init it. */
	pRouteEntry = _hx_malloc(sizeof(__IP_RTE));
	if (NULL == pRouteEntry)
	{
		__ERROR_HANDLER_OUT_OF_MEMORY;
		goto __TERMINAL;
	}
	memset(pRouteEntry, 0, sizeof(__IP_RTE));
	pRouteEntry->genif_out[0] = pGenif;
	/* Mask out the host part if exist. */
	pRouteEntry->ip_dest.addr = (dest->addr & mask->addr);
	pRouteEntry->ip_mask.addr = mask->addr;
	pRouteEntry->metric = metric;
	pRouteEntry->type = route_type;
	if (nexthop)
	{
		/* Next hop specified. */
		pRouteEntry->next_hop[0].addr = nexthop->addr;
	}
	/* Get prefix's length. */
	addr = mask->addr;
	while (addr)
	{
		if (addr & 0x80000000)
		{
			pRouteEntry->mask_len++;
		}
		addr <<= 1;
	}

	/* Send message to handler thread to add route. */
	msg.wCommand = RTMGR_MSG_ADDIPROUTE;
	msg.dwParam = (unsigned long)pRouteEntry;
	bResult = KernelThreadManager.SendMessage((__COMMON_OBJECT*)RoutingManager.rm_thread, &msg);

__TERMINAL:
	if (!bResult)
	{
		/* Show message and release all resources. */
		__LOG("[%s]add ip route failed.\r\n", __func__);
		if (pRouteEntry) {
			_hx_free(pRouteEntry);
		}
		if (pGenif)
		{
			NetworkManager.ReleaseGenif(pGenif);
		}
	}

	return bResult;
}

/*
 * Delete one IP route entry.
 * This routine is invoked in following scenarios:
 *   1. When a new address is set to genif, so the old route entry
 *      corresponding the old address must be purged, it's called
 *      in AddGenifAddress routine of protocol;
 *   2. When a genif's status is changed from up to down, called in
 *      LinkStatusChange routine of protocol;
 *   3. User could delete route entry from command, so it's could
 *      be invoked in command handler;
 *   4. Other scenarios to fit lwIP temporarily, such as when one
 *      pppoe session is down.
 */
static BOOL __del_iproute(ip_addr_t* dest, ip_addr_t* mask)
{
	__IP_RTE* pRouteEntry = NULL;
	__KERNEL_THREAD_MESSAGE msg;

	if ((NULL == dest) || (NULL == mask))
	{
		return FALSE;
	}

	/*
	 * Allocate a route entry object to hold parameters,
	 * no other usage of this route entry object.
	 */
	pRouteEntry = _hx_malloc(sizeof(__IP_RTE));
	/* Only keep the network part. */
	pRouteEntry->ip_dest.addr = (dest->addr & mask->addr);
	pRouteEntry->ip_mask = *mask;

	/* Tell main thread to purge the route. */
	msg.wCommand = RTMGR_MSG_DELIPROUTE;
	msg.dwParam = (unsigned long)pRouteEntry;
	return KernelThreadManager.SendMessage((__COMMON_OBJECT*)RoutingManager.rm_thread, &msg);
}

/*
 * Look up IP routing table.
 * tag is used to help the routing table look up
 * process, especially in ECMP scenario, used to
 * select one route entry. Not used in current version.:------)
 * Only the out-going interface(genif) is returned
 * after looking up. In case of broadcast network(such
 * as ethernet), the next hop(gateway) is determined
 * in ethernet's output routine(etharp_output in lwIP).
 * This routine is invoked in ip_forward routine of
 * lwIP currently.
 *
 * THIS ROUTINE COULD NOT BE INVOKED IN INTERRUPT CONTEXT.
 */
static __GENERIC_NETIF* __ip_route(ip_addr_t* dest, unsigned long* tag)
{
	__GENERIC_NETIF* pGenif = NULL;
	__IP_RTE* pRouteEntry = NULL;
	__IP_ROUTING_TABLE* pRoutingTable = &RoutingManager.ip_rt;

	BUG_ON(IN_INTERRUPT());

	if (NULL == dest)
	{
		goto __TERMINAL;
	}

	/* Search the matching route entry by traveling whole table. */
	__RW_SLOCK_RLOCK(pRoutingTable->rw_lock);
	pRouteEntry = pRoutingTable->rte_head.pNext;
	while (pRouteEntry != &pRoutingTable->rte_head)
	{
		if ((pRouteEntry->ip_dest.addr & pRouteEntry->ip_mask.addr) ==
			(dest->addr & pRouteEntry->ip_mask.addr))
		{
			/* Found the matched entry. */
			pGenif = pRouteEntry->genif_out[0];
			pRouteEntry->hit_count++;
			break;
		}
		pRouteEntry = pRouteEntry->pNext;
	}
	__RW_SLOCK_RUNLOCK(pRoutingTable->rw_lock);

__TERMINAL:
	if (NULL == pGenif)
	{
		/* No entry found, increase miss counter. */
		pRoutingTable->miss_count++;
	}
	return pGenif;
}

/* Show out all ip route entries in system. */
static void __show_ip_route(ip_addr_t* dest)
{
	__IP_RTE* pRouteEntry = NULL;
	__IP_ROUTING_TABLE* rout_table = &RoutingManager.ip_rt;

	__RW_SLOCK_RLOCK(rout_table->rw_lock);
	pRouteEntry = rout_table->rte_head.pNext;
	_hx_printf("  Total [%d] route entry(s):\r\n", rout_table->rte_num);
	_hx_printf("  -----------------------------------------------------------------\r\n");
	while (pRouteEntry != &rout_table->rte_head)
	{
		/* Show all route entries. */
		_hx_printf("  %16s", inet_ntoa(pRouteEntry->ip_dest));
		_hx_printf("  %16s", inet_ntoa(pRouteEntry->ip_mask));
		_hx_printf("  %4d", pRouteEntry->metric);
		_hx_printf("  %4d", pRouteEntry->mask_len);
		_hx_printf("  %4d", pRouteEntry->hit_count);
		_hx_printf("  %s\r\n", pRouteEntry->genif_out[0]->genif_name);

		pRouteEntry = pRouteEntry->pNext;
	}
	__RW_SLOCK_RUNLOCK(rout_table->rw_lock);
	return;
}

/* Add one pbr(policy based routing) entry. */
static BOOL __add_iproute_pbr(struct ip_hdr* ip_hdr, const char* genif_in,
	ip_addr_t* dest, ip_addr_t* mask, int metric, ip_addr_t* nexthop,
	const char* genif_out)
{
	__GENERIC_NETIF* pGenifIn = NULL;
	__GENERIC_NETIF* pGenifOut = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == genif_out);

	/* Now we only support the ingress-egress genif partern. */
	if (NULL == genif_in)
	{
		goto __TERMINAL;
	}

	pGenifIn = NetworkManager.GetGenifByName(genif_in);
	if (NULL == pGenifIn)
	{
		goto __TERMINAL;
	}
	pGenifOut = NetworkManager.GetGenifByName(genif_out);
	if (NULL == pGenifOut)
	{
		goto __TERMINAL;
	}

	/* Just set the ingress and egress genif in pbr entry. */
	RoutingManager.rte_pbr.pGenifIngress = pGenifIn;
	RoutingManager.rte_pbr.pGenifOut = pGenifOut;
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Delete one pbr entry. */
static BOOL __del_iproute_pbr(struct ip_hdr* ip_hdr, __GENERIC_NETIF* ingress,
	ip_addr_t* dest, ip_addr_t* mask)
{
	return FALSE;
}

/* Lookup pbr routing table. */
static __GENERIC_NETIF* __ip_route_pbr(struct ip_hdr* ip_hdr, __GENERIC_NETIF* ingress,
	ip_addr_t* dest, unsigned long* tag)
{
	/* We only support ingress-egress model now. */
	if (NULL == ingress)
	{
		return NULL;
	}
	if (RoutingManager.rte_pbr.pGenifIngress == ingress)
	{
		return RoutingManager.rte_pbr.pGenifOut;
	}
	return NULL;
}

/* 
 * Notify routing manager that one genif's state 
 * has changed. Routing manager may modify 
 * routing table according this notification, such
 * as install or delete one or more routes that
 * associated with this genif, or so on.
 */
static BOOL __GenifStateChange(const char* genif_name, unsigned short change_flags)
{
	BOOL bResult = FALSE;
	__KERNEL_THREAD_MESSAGE msg;
	__GENERIC_NETIF* pGenif = NULL;

	/* Get the genif by it's name. */
	pGenif = NetworkManager.GetGenifByName(genif_name);
	if (NULL == pGenif)
	{
		__LOG("[%s]No genif[%s] found in system.\r\n", __func__, genif_name);
		goto __TERMINAL;
	}

	/* Send message to back thread. */
	msg.dwParam = (unsigned long)pGenif;
	msg.wParam = change_flags;
	msg.wCommand = RTMGR_MSG_GENIFSTATECHANGE;
	bResult = KernelThreadManager.SendMessage(
		(__COMMON_OBJECT*)RoutingManager.rm_thread, 
		&msg);

__TERMINAL:
	return bResult;
}

/* Global routing manager object. */
__ROUTING_MANAGER RoutingManager = {
	{0},                  /* ip_rt.                 */
	{0},                  /* rte_pbr.               */
	NULL,                 /* rm_thread.             */

	__Initialize,         /* Initialize routine.    */
	__add_iproute,        /* add_iproute routine.   */
	__del_iproute,        /* del_iproute routine.   */
	__ip_route,           /* ip_route routine.      */
	__add_iproute_pbr,    /* add_iproute_pbr.       */
	__del_iproute_pbr,    /* del_iproute_pbr.       */
	__ip_route_pbr,       /* ip_route_pbr.          */
	__GenifStateChange,   /* LinkStatusChange.      */
	__show_ip_route,      /* show_ip_route routine. */
};
