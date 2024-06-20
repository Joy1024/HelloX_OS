//***********************************************************************/
//    Author                    : Garry
//    Original Date             : DEC 25, 2022
//    Module Name               : genifpp.h
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

#ifndef __GENIFPP_H__
#define __GENIFPP_H__

#include "TYPES.H"

#include "genif.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"

/* genifpp's name. */
#define GENIF_NAME_GENIFPP "pppoe_genif"

/* 
 * Initializes the pppoe constructed netif object. 
 * Create a new genif, associated with the netif, and
 * register it into system.
 */
BOOL __genif_pp_init(struct netif* netif, ip_addr_t* if_addr, ip_addr_t* if_mask);

#endif //__GENIFPP_H__
