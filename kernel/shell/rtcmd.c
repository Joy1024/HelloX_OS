//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 11, 2022
//    Module Name               : rtcmd.c
//    Module Funciton           : 
//    Description               : 
//                                Shell command for ip rouing function, 
//                                route add, delete, list, and others.
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
#include <lwip/inet.h>
#include <genif.h>

#include "routmgr/rtmgr.h"
#include "routmgr/iprte.h"
#include "rtcmd.h"

/* Add one route entry into system. */
static unsigned long __iproute_add(const char* argv[], int argc)
{
	ip_addr_t dest, mask, gw;
	char* genif_name = NULL;
	unsigned int metric = 0;

	if (argc < 5)
	{
		_hx_printf("Usage: iproute add [dest] [mask] [metric] [if_name] [next-hop]\r\n");
		goto __TERMINAL;
	}
	/* Convert user input as parameters. */
	if (0 == inet_aton(argv[1], &dest))
	{
		_hx_printf("  Invalid ip address[%s]", argv[1]);
		goto __TERMINAL;
	}
	if (0 == inet_aton(argv[2], &mask))
	{
		_hx_printf("  Invalid ip address[%s]", argv[2]);
		goto __TERMINAL;
	}
	metric = atol(argv[3]);
	if (0 == metric)
	{
		/* Use default metric value. */
		metric = ROUTE_METRIC_STATIC;
	}
	gw.addr = 0;
	if (argc >= 6)
	{
		if (0 == inet_aton(argv[5], &gw))
		{
			_hx_printf("  Invalid ip address[%s]", argv[5]);
			goto __TERMINAL;
		}
	}

	/* All parameters OK, invoke the handler. */
	if (RoutingManager.add_iproute(&dest, &mask, metric, &gw, IP_ROUTE_TYPE_STATIC, argv[4]))
	{
		_hx_printf("  Add route success.\r\n");
	}
	else {
		_hx_printf("  Failed to add route.\r\n");
	}

__TERMINAL:
	return 0;
}

/* Delete one route entry from system. */
static unsigned long __iproute_del(const char* argv[], int argc)
{
	ip_addr_t dest, mask;

	if (argc < 3)
	{
		_hx_printf("Usage: iproute del [dest] [mask] \r\n");
		goto __TERMINAL;
	}
	/* Convert user input as parameters. */
	if (0 == inet_aton(argv[1], &dest))
	{
		_hx_printf("  Invalid ip address[%s]", argv[1]);
		goto __TERMINAL;
	}
	if (0 == inet_aton(argv[2], &mask))
	{
		_hx_printf("  Invalid ip address[%s]", argv[2]);
		goto __TERMINAL;
	}

	if (RoutingManager.del_iproute(&dest, &mask))
	{
		_hx_printf("  Delete route success.\r\n");
	}
	else {
		_hx_printf("  Failed to delete route.\r\n");
	}

__TERMINAL:
	return 0;
}

/* List all route entry(s) in system. */
static unsigned long __iproute_list(const char* argv[], int argc)
{
	RoutingManager.show_ip_route(NULL);
	return 0;
}

/* Look up the routing table, just for debugging. */
static unsigned long __iproute_lookup(const char* argv[], int argc)
{
	ip_addr_t dest;
	__GENERIC_NETIF* pGenif = NULL;

	if (argc < 2)
	{
		_hx_printf("  Usage: iproute lookup [destination].\r\n");
		goto __TERMINAL;
	}
	/* Get destination address. */
	if (0 == inet_aton(argv[1], &dest))
	{
		_hx_printf("  Invalid ip address[%s]", argv[1]);
		goto __TERMINAL;
	}

	pGenif = RoutingManager.ip_route(&dest, NULL);
	if (NULL == pGenif)
	{
		_hx_printf("  No route found for[%s]\r\n", argv[1]);
	}
	else {
		_hx_printf("  Output interface: %s\r\n", pGenif->genif_name);
	}

__TERMINAL:
	return 0;
}

/* Add one PBR routing entry into system. */
static unsigned long __iproute_pbradd(const char* argv[], int argc)
{
	const char* ingress = NULL;
	const char* egress = NULL;

	if (argc < 3)
	{
		_hx_printf("Usage: iproute pbradd [genif-ingress] [genif-egress] \r\n");
		goto __TERMINAL;
	}
	
	/* Convert user input as parameters. */
	ingress = argv[1];
	egress = argv[2];
	if (RoutingManager.add_iproute_pbr(NULL, ingress, NULL, NULL, 0, NULL, egress))
	{
		_hx_printf("Add PBR entry success.\r\n");
	}
	else {
		_hx_printf("Add PBR entry failed.\r\n");
	}

__TERMINAL:
	return 0;
}

/*
 * The map between iproute command and it's
 * hander, it's description string.
 * The iproute command just search this map
 * using the subcommand string, to locate the handler
 * of sub command.
 */
static struct __iproute_subcmd_struct {
	const char* subcmd_name;
	unsigned long(*CommandHandler)(const char*[], int);
	const char* subcmd_description;
}iproute_subcmd_map[] = {
	{ "add",    __iproute_add,       "  iproute add    : Add one ip route entry into system."},
	{ "del",    __iproute_del,       "  iproute del    : Del one ip route entry from system."},
	{ "list",   __iproute_list,      "  iproute list   : List all route entry(s) in system."},
	{ "pbradd", __iproute_pbradd,    "  iproute pbradd : Add one PBR entry into system."},
	{ "lookup", __iproute_lookup,    "  iproute lookup : Look up routing table using IP addr."},
	{ NULL,        NULL,            NULL},
};

/* Main entry of iproute command. */
unsigned long iproute_entry(__CMD_PARA_OBJ* lpCmdObj)
{
	int argc = 0;

	argc = lpCmdObj->byParameterNum;
	if (argc < 2)
	{
		/* Just show out all available commands under ipsec command. */
		for (int i = 0; ; i++)
		{
			if (NULL == iproute_subcmd_map[i].subcmd_name)
			{
				break;
			}
			printf("%s\r\n", iproute_subcmd_map[i].subcmd_description);
		}
		goto __TERMINAL;
	}

	/* Lookup subcommand map and invoke the handler. */
	for (int i = 0; ; i++)
	{
		if (NULL == iproute_subcmd_map[i].subcmd_name)
		{
			/* No subcmd matched. */
			printf("  Invalid sub command.\r\n");
			break;
		}
		if (0 == strcmp(lpCmdObj->Parameter[1], iproute_subcmd_map[i].subcmd_name))
		{
			iproute_subcmd_map[i].CommandHandler(&lpCmdObj->Parameter[1], argc - 1);
			break;
		}
	}

__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;
}
