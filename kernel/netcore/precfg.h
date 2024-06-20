/* 
 * Pre-configured network parameters,used as
 * builtin configuration of network functions.
 */
#ifndef __PRECFG_H__
#define __PRECFG_H__

/* Enable the pre-configured parameters of PPPoE. */
#define NET_PRECONFIG_ENABLE_PPPOE 0

 /* Default password and user name,to simplify testing. */
#define PPPOE_DEFAULT_USERNAME_QD "053202039989"
#define PPPOE_DEFAULT_PASSWORD_QD "60767168"

/* Default password and user name,to simplify testing. */
#define PPPOE_DEFAULT_USERNAME_BJ "01012187137"
#define PPPOE_DEFAULT_PASSWORD_BJ "858137"

#define PPPOE_DEFAULT_USERNAME PPPOE_DEFAULT_USERNAME_BJ
#define PPPOE_DEFAULT_PASSWORD PPPOE_DEFAULT_PASSWORD_BJ

#define PPPOE_DEFAULT_SESSION_NAME "hellox"
#define PPPOE_DEFAULT_INT_NAME "g1000_if_0"
#define PPPOE_DEFAULT_AUTH_TYPE "chap" //"pap"

/* Enable NAT on the pppoe new created interface. */
#define PPPOE_ENABLE_NAT

/* Enable the pre-configured parameters of dhcp server. */
#define NET_PRECONFIG_ENABLE_DHCPD 1

/* Interfaces that dhcp server should run. */
#define DHCPD_ENABLED_GENIF_1 "g1000_if_1"
#define DHCPD_ENABLED_GENIF_2 "g1000_if_2"
#define DHCPD_ENABLED_GENIF_3 "g1000_if_3"
#define DHCPD_ENABLED_GENIF_4 "g1000_if_4"
#define DHCPD_ENABLED_GENIF_5 "g1000_if_5"

#endif //__PRECFG_H__
