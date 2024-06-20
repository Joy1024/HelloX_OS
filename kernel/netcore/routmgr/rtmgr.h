//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 30, 2022
//    Module Name               : rtmgr.h
//    Module Funciton           : 
//                                Routing manager's definition and it's
//                                contants, operations,...
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __RTMGR_H__
#define __RTMGR_H__

#include "iprte.h"

/* Message values used by routing manager thread. */
#define RTMGR_MSG_ADDIPROUTE        (MSG_USER_START + 1)   /* Add ip route. */
#define RTMGR_MSG_DELIPROUTE        (MSG_USER_START + 2)   /* Del ip route. */
#define RTMGR_MSG_ADDIP6ROUTE       (MSG_USER_START + 3)   /* Add ipv6 route. */
#define RTMGR_MSG_DELIP6ROUTE       (MSG_USER_START + 4)   /* Del ipv6 route. */
#define RTMGR_MSG_SHOWIPROUTE       (MSG_USER_START + 5)   /* Show ip route. */
#define RTMGR_MSG_SHOWIP6ROUTE      (MSG_USER_START + 6)   /* Show ipv6 route. */
#define RTMGR_MSG_GENIFSTATECHANGE  (MSG_USER_START + 7)   /* Link status chage on genif. */

/* Memory out log info of this module. */
#define __ERROR_HANDLER_OUT_OF_MEMORY __LOG("[%s/%d]out of memory.\r\n", __func__, __line__)

/* 
 * Default route's metric value for different 
 * routing protocols or route types. 
 */
#define ROUTE_METRIC_DIRECTLY     20
#define ROUTE_METRIC_STATIC       50
#define ROUTE_METRIC_OSPF         110
#define ROUTE_METRIC_OSPF_ASE     220
#define ROUTE_METRIC_IBGP         180
#define ROUTE_METRIC_EBGP         300

/* Flags of the state that changed in genif. */
#define GENIF_STATE_LINKSTATUSUP        (1 << 0)
#define GENIF_STATE_LINKSTATUSDOWN      (1 << 1)
#define GENIF_STATE_ADDRADD             (1 << 2)
#define GENIF_STATE_ADDRREMOVE          (1 << 3)

/* Routing manager's definition. */
typedef struct tag__ROUTING_MANAGER {
	/* Routing table for IP protocol. */
	__IP_ROUTING_TABLE ip_rt;

	/* PBR routing table, just support one entry now. */
	__IP_RTE_PBR rte_pbr;

	/* Dedicated thread to manage routing tables. */
	__KERNEL_THREAD_OBJECT* rm_thread;

	/* Operating routines. */
	BOOL (*Initialize)(struct tag__ROUTING_MANAGER* rtmgr);
	/* Add one IP route entry. */
	BOOL (*add_iproute)(ip_addr_t* dest, ip_addr_t* mask,
		int metric, ip_addr_t* nexthop,
		__IP_ROUTE_TYPE route_type,
		const char* pGenifName);
	/* Delete one IP route entry. */
	BOOL (*del_iproute)(ip_addr_t* dest, ip_addr_t* mask);
	/* Look up IP routing table. */
	__GENERIC_NETIF* (*ip_route)(ip_addr_t* dest, unsigned long* tag);
	/* List out all ip route entries. */
	
	/* Add one pbr(policy based routing) entry. */
	BOOL (*add_iproute_pbr)(struct ip_hdr* ip_hdr, const char* genif_in,
		ip_addr_t* dest, ip_addr_t* mask, int metric, ip_addr_t* nexthop,
		const char* genif_out);
	/* Delete one pbr entry. */
	BOOL (*del_iproute_pbr)(struct ip_hdr* ip_hdr, __GENERIC_NETIF* ingress,
		ip_addr_t* dest, ip_addr_t* mask);
	/* Lookup pbr routing table. */
	__GENERIC_NETIF* (*ip_route_pbr)(struct ip_hdr* ip_hdr, __GENERIC_NETIF* ingress,
		ip_addr_t* dest, unsigned long* tag);

	/* Notify routing manager that one genif's state changed. */
	BOOL (*GenifStateChange)(const char* genif_name, unsigned short change_flags);

	void (*show_ip_route)(ip_addr_t* dest);
}__ROUTING_MANAGER;

/* Global external routing manager object. */
extern __ROUTING_MANAGER RoutingManager;

#endif //__RTMGR_H__
