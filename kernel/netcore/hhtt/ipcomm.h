//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Feb 11, 2023
//    Module Name               : ipcomm.h
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

#ifndef __IPCOMM_H__
#define __IPCOMM_H__

#include "lwip/tcp_impl.h"
#include "genif.h"

/* Output NAT debugging information. */
#ifdef NAT_DEBUG
#ifdef __MS_VC__
#define __NATDEBUG(fmt,...) _hx_printf(fmt,__VA_ARGS__)
#else
#define __NATDEBUG(fmt,args...) _hx_printf(fmt,##args)
#endif //__MS_VC__
#else
#ifdef __MS_VC__
#define __NATDEBUG(fmt,...)
#else
#define __NATDEBUG(fmt,args...)
#endif //__MS_VC__
#endif  //NAT_DEBUG

/* TCP option kind value. */
#define TCP_OPTION_KIND_END   0
#define TCP_OPTION_KIND_NOP   1
#define TCP_OPTION_KIND_MSS   2

/* TCP header's fixed length,without options. */
#define TCP_FIXED_HEADER_LEN  20
/* IP header's fixed length. */
#define IP_FIXED_HEADER_LEN   20

/* 
 * Calculate ip header's check sum and the 
 * transport layer's check sum,such as TCP
 * and UPD header's checksum.
 */
void _iphdr_check_sum(struct ip_hdr* p, struct pbuf* pb);

/* Adjust tcp's mss according genif's mtu. */
BOOL tcp_mss_adjust(__GENERIC_NETIF* pGenif, struct tcp_hdr* pTcpHdr);

#endif //__IPCOMM_H__
