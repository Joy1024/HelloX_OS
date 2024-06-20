//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 30, 2022
//    Module Name               : iprte.h
//    Module Funciton           : 
//                                IP route table entry's definitions and
//                                related constants, operation routines.
//                                
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __IPRTE_H__
#define __IPRTE_H__

#include "TYPES.H"
#include "lwip/ip_addr.h"
#include "genif.h"

/* Maximal out going interface number in ECMP. */
#define IP_ECMP_NUM 4

/* Route's type. */
typedef enum tag__IP_ROUTE_TYPE {
	IP_ROUTE_TYPE_UNKNOWN = 0,
	IP_ROUTE_TYPE_STATIC  = 1,
	IP_ROUTE_TYPE_OSPF    = 2,
	IP_ROUTE_TYPE_ISIS    = 3,
	IP_ROUTE_TYPE_IBGP    = 4,
	IP_ROUTE_TYPE_EBGP    = 5,
	IP_ROUTE_TYPE_IGRP    = 6,
	IP_ROUTE_TYPE_DIRECT  = 7
}__IP_ROUTE_TYPE;

/* Routing table entry of IP protocol. */
typedef struct tag__IP_RTE {
	ip_addr_t ip_dest;      /* Destination network. */
	ip_addr_t ip_mask;      /* Destination mask. */
	int mask_len;           /* Mask length. */
	int metric;             /* Route matric. */
	__IP_ROUTE_TYPE type;   /* Route type. */
	__GENERIC_NETIF* genif_out[IP_ECMP_NUM]; /* Out-going interface. */
	ip_addr_t next_hop[IP_ECMP_NUM];  /* Next hop address. */

	/* Statistics counters for this entry. */
	unsigned long hit_count;

	/* Link list to construct routing table. */
	struct tag__IP_RTE* pNext;
	struct tag__IP_RTE* pPrev;
}__IP_RTE;

/* IP routing entry for PBR(Policy Based Routing). */
typedef struct tag__IP_RTE_PBR {
	/* 
	 * Keys of the PBR entry, the following 
	 * variables could be specified one or
	 * more, the looking up algorithm will use
	 * them to locate one PBR entry.
	 */
	ip_addr_t ip_src_addr;
	ip_addr_t ip_src_mask;
	ip_addr_t ip_dest_addr;
	ip_addr_t ip_dest_mask;
	uint8_t protocol;
	__GENERIC_NETIF* pGenifIngress;

	/* The out going genif of the entry. */
	__GENERIC_NETIF* pGenifOut;

	/* Link list of all entries. */
	struct tag__IP_RTE_PBR* pPrev;
	struct tag__IP_RTE_PBR* pNext;
}__IP_RTE_PBR;

/* Macros to simplify the out if and next hop's operation. */
#define IP_RTE_PRIMARY_OUTIF(rte) ((rte).genif_out[0])
#define IP_RTE_PRIMARY_NEXTHOP(rte) ((rte).next_hop[0])

/* Routing table of IP. */
typedef struct tag__IP_ROUTING_TABLE {
	/* Head element. */
	__IP_RTE rte_head;
	/* How many rte in this table. */
	unsigned long rte_num;

	/* read write lock to access this table. */
	__RW_SPIN_LOCK rw_lock;

	/* Statistics counters. */
	unsigned long miss_count;

}__IP_ROUTING_TABLE;

#endif //__IPRTE_H__
