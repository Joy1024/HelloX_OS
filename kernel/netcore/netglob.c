//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 29,2017
//    Module Name               : netglob.c
//    Module Funciton           : 
//                                Global variables of network module,of
//                                HelloX.Arrange all these global variables
//                                into an object,to make management easy.
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
#include <string.h>

#include "lwip/inet.h"
#include "netcfg.h"
#include "netmgr.h"
#include "netglob.h"

/* Initializer of network global object. */
static BOOL NetGlobInitialize(__NETWORK_GLOBAL* pNetGlob)
{
	pNetGlob->ResetDNSServer();
	strncpy(pNetGlob->domain_name, DEFAULT_DOMAIN_NAME, DOMAIN_NAME_LENGTH);
	return TRUE;
}

/* Reset DNS server to default value. */
static void ResetDNSServer()
{
	NetworkGlobal.dns_primary.addr = 0;
	NetworkGlobal.dns_secondary.addr = 0;
}

/* Return system level network information. */
static BOOL __GetNetworkInfo(__SYSTEM_NETWORK_INFO* pNetInfo)
{
	BOOL bResult = FALSE;
	ip_addr_t addr;

	BUG_ON(NULL == pNetInfo);
	if (pNetInfo->version != 1)
	{
		goto __TERMINAL;
	}
	/* Clear the whole structure. */
	memset(pNetInfo, 0, sizeof(__SYSTEM_NETWORK_INFO));
	pNetInfo->version = 1;
	pNetInfo->genif_num = NetworkManager.genif_num;
	//pNetInfo->v4dns_p = NetworkGlobal.dns_primary.addr;
	//pNetInfo->v4dns_s = NetworkGlobal.dns_secondary.addr;

	/* Use static specified global DNS server. */
	inet_aton("8.8.8.8", &addr);
	pNetInfo->v4dns_p = addr.addr;
	inet_aton("8.8.4.4", &addr);
	pNetInfo->v4dns_s = addr.addr;

	/* Return domain name. */
	strncpy(pNetInfo->domain_name, NetworkGlobal.domain_name, DOMAIN_NAME_LENGTH);

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Network global object. */
__NETWORK_GLOBAL NetworkGlobal = {
	0,                   //dns_primary.
	0,                   //dns_secondary.
	{ 0 },               //domain_name.
	ResetDNSServer,      //ResetDNSServer.
	NetGlobInitialize,   //Intialize.
	__GetNetworkInfo,    //GetNetworkInfo.
};
