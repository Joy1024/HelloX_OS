//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 29,2014
//    Module Name               : network.h
//    Module Funciton           : 
//    Description               : 
//                                Network diagnostic application,common used
//                                network tools such as ping/tracert,are implemented
//                                in network.c file.
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

#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "lwip/stats.h"  /* For IP layer statistics counter. */
#include "ethmgr.h"
#include "proto.h"
#include "netcfg.h"
#include "netmgr.h"
#include "cap/capture.h"

#include "kapi.h"
#include "shell.h"
#include "network.h"
#include "ipseccmd.h"
#include "vticmd.h"
#include "rtcmd.h"

#ifdef __CFG_NET_DHCP_SERVER
#include "dhcp_srv/dhcp_srv.h"
#endif
#ifdef __CFG_NET_PPPOE
#include "pppox/oemgr.h"
#endif
#ifdef __CFG_NET_NAT
#include "nat/nat.h"
#endif
#ifdef __CFG_NET_DPI
#include "dpi/dpimgr.h"
#endif

#define  NETWORK_PROMPT_STR   "[network_view]"

//
//Pre-declare routines.
//
static DWORD CommandParser(LPCSTR);
static DWORD help(__CMD_PARA_OBJ*);       //help sub-command's handler.
static DWORD _exit(__CMD_PARA_OBJ*);      //exit sub-command's handler.
static DWORD iflist(__CMD_PARA_OBJ*);
static DWORD ping(__CMD_PARA_OBJ*);
static DWORD netstat(__CMD_PARA_OBJ*);    //Show out IP layer statistics counter.
static DWORD showint(__CMD_PARA_OBJ*);    //Display ethernet interface's statistics information.
static DWORD showdbg(__CMD_PARA_OBJ*);    //Display ethernet related debugging information.
static DWORD assoc(__CMD_PARA_OBJ*);      //Associate to a specified WiFi SSID.
static DWORD scan(__CMD_PARA_OBJ*);       //Rescan the WiFi networks.
static DWORD setif(__CMD_PARA_OBJ*);      //Set a given interface's configurations.
static DWORD showgif(__CMD_PARA_OBJ*);
static DWORD setgif(__CMD_PARA_OBJ*);
static DWORD capstart(__CMD_PARA_OBJ*);
static DWORD capstop(__CMD_PARA_OBJ*);

/* DHCP Server control command. */
#ifdef __CFG_NET_DHCP_SERVER
static DWORD dhcpd(__CMD_PARA_OBJ*);
#endif

/* PPPoE control command. */
#ifdef __CFG_NET_PPPOE
static DWORD pppoe(__CMD_PARA_OBJ*);
#endif

/* NAT control command. */
#ifdef __CFG_NET_NAT
static DWORD nat(__CMD_PARA_OBJ*);
#endif

/* DPI control command. */
#ifdef __CFG_NET_DPI
static DWORD dpi(__CMD_PARA_OBJ*);
#endif

//
//The following is a map between command and it's handler.
//
static struct __FDISK_CMD_MAP{
	LPSTR                lpszCommand;
	DWORD                (*CommandHandler)(__CMD_PARA_OBJ*);
	LPSTR                lpszHelpInfo;
}SysDiagCmdMap[] = {
	{ "iflist",     iflist,        "  iflist   : Show all network interface(s) in system."},
	{ "ping",       ping,          "  ping     : Check a specified host's reachbility."},
	{ "showint",    showint,       "  showint  : Display ethernet interface's statistics information."},
	{ "showdbg",    showdbg,       "  showdbg  : Display ethernet related debugging info." },
	{ "netstat",    netstat,       "  netstat  : Show out network statistics counter." },
	{ "assoc",      assoc,         "  assoc    : Associate to a specified WiFi SSID."},
	{ "scan",       scan,          "  scan     : Scan WiFi networks and show result."},
	{ "iproute",    iproute_entry, "  iproute  : Manages ip routing table fo the system."},
	{ "setif",      setif,         "  setif    : Set IP configurations to a given interface."},
	{ "showgif",    showgif,       "  showgif  : Show all generic netif in system."},
	{ "setgif",     setgif,        "  setgif   : Set configuration of a genif."},
	{ "capstart",   capstart,      "  capstart : Start capture packet on genif."},
	{ "capstop",    capstop,       "  capstop  : Stop capture packet on genif."},
#ifdef __CFG_NET_DHCP_SERVER
	{ "dhcpd",      dhcpd,         "  dhcpd    : DHCP Server control commands." },
#endif
#ifdef __CFG_NET_PPPOE
	{ "pppoe",      pppoe,         "  pppoe    : PPPoE function control commands." },
#endif
#ifdef __CFG_NET_NAT
	{ "nat",        nat,           "  nat      : NAT control commands." },
#endif
#ifdef __CFG_NET_DPI
	{ "dpi",        dpi,           "  dpi      : DPI function control commands."},
#endif
	{ "ipsec",      ipsec_entry,   "  ipsec    : ipsec diagnostic or operation."},
	{ "vti",        vti_entry,     "  vti      : Virtual tunnel interface management."},
	{ "help",       help,          "  help     : Print out this screen." },
	{ "exit",       _exit,         "  exit     : Exit the application." },
	{ NULL,		    NULL,          NULL}
};

static DWORD QueryCmdName(LPSTR pMatchBuf,INT nBufLen)
{
	static DWORD dwIndex = 0;

	if(pMatchBuf == NULL)
	{
		dwIndex    = 0;	
		return SHELL_QUERY_CONTINUE;
	}

	if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
	{
		dwIndex = 0;
		return SHELL_QUERY_CANCEL;	
	}

	strncpy(pMatchBuf,SysDiagCmdMap[dwIndex].lpszCommand,nBufLen);
	dwIndex ++;

	return SHELL_QUERY_CONTINUE;	
}

//
//The following routine processes the input command string.
//
static DWORD CommandParser(LPCSTR lpszCmdLine)
{
	DWORD                  dwRetVal          = SHELL_CMD_PARSER_INVALID;
	DWORD                  dwIndex           = 0;
	__CMD_PARA_OBJ*        lpCmdParamObj     = NULL;

	if((NULL == lpszCmdLine) || (0 == lpszCmdLine[0]))    //Parameter check
	{
		return SHELL_CMD_PARSER_INVALID;
	}

	lpCmdParamObj = FormParameterObj(lpszCmdLine);
	
	
	if(NULL == lpCmdParamObj)    //Can not form a valid command parameter object.
	{
		return SHELL_CMD_PARSER_FAILED;
	}

	//if(0 == lpCmdParamObj->byParameterNum)  //There is not any parameter.
	//{
	//	return SHELL_CMD_PARSER_FAILED;
	//}
	//CD_PrintString(lpCmdParamObj->Parameter[0],TRUE);

	//
	//The following code looks up the command map,to find the correct handler that handle
	//the current command.Calls the corresponding command handler if found,otherwise SYS_DIAG_CMD_PARSER_INVALID
	//will be returned to indicate this case.
	//
	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszCommand)
		{
			dwRetVal = SHELL_CMD_PARSER_INVALID;
			break;
		}
		if(StrCmp(SysDiagCmdMap[dwIndex].lpszCommand,lpCmdParamObj->Parameter[0]))  //Find the handler.
		{
			dwRetVal = SysDiagCmdMap[dwIndex].CommandHandler(lpCmdParamObj);
			break;
		}
		else
		{
			dwIndex ++;
		}
	}

	//Release parameter object.
	if(NULL != lpCmdParamObj)
	{
		ReleaseParameterObj(lpCmdParamObj);
	}

	return dwRetVal;
}

//
//This is the application's entry point.
//
DWORD networkEntry(LPVOID p)
{
	return Shell_Msg_Loop(NETWORK_PROMPT_STR,CommandParser,QueryCmdName);	
}

//
//The exit command's handler.
//
static DWORD _exit(__CMD_PARA_OBJ* lpCmdObj)
{
	return SHELL_CMD_PARSER_TERMINAL;
}

//
//The help command's handler.
//
static DWORD help(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD               dwIndex = 0;

	while(TRUE)
	{
		if(NULL == SysDiagCmdMap[dwIndex].lpszHelpInfo)
			break;

		PrintLine(SysDiagCmdMap[dwIndex].lpszHelpInfo);
		dwIndex ++;
	}
	return SHELL_CMD_PARSER_SUCCESS;
}

//ping command's implementation.
static DWORD ping(__CMD_PARA_OBJ* lpCmdObj)
{	
	__PING_PARAM     PingParam;
	ip_addr_t        ipAddr;
	int              count      = 3;    //Ping counter.
	int              size       = 64;   //Ping packet size.
	BYTE             index      = 1;
	DWORD            dwRetVal   = SHELL_CMD_PARSER_FAILED;
	__CMD_PARA_OBJ*  pCurCmdObj = lpCmdObj;
	

	if(pCurCmdObj->byParameterNum <= 1)
	{
		return dwRetVal;
	}

	while(index < lpCmdObj->byParameterNum)
	{
		if(strcmp(pCurCmdObj->Parameter[index],"/c") == 0)
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				break;
			}
			count    = atoi(pCurCmdObj->Parameter[index]);
		}
		else if(strcmp(pCurCmdObj->Parameter[index],"/l") == 0)
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				break;
			}
			size   = atoi(pCurCmdObj->Parameter[index]);
		}
		else
		{
			ipAddr.addr = inet_addr(pCurCmdObj->Parameter[index]);
		}

		index ++;
	}
	
	if(ipAddr.addr != 0)
	{
		dwRetVal    = SHELL_CMD_PARSER_SUCCESS;
	}

	PingParam.count      = count;
	PingParam.targetAddr = ipAddr;
	PingParam.size       = size;

	//Call ping entry routine.
	ping_Entry((void*)&PingParam);
	
	return dwRetVal;
}

/* Change network interface's configuration. */
static DWORD setif(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD dwRetVal = SHELL_CMD_PARSER_FAILED;
	__ETH_IP_CONFIG* pifConfig = NULL;
	char index = 1;
	char* errmsg = "  Error: Invalid parameter(s).\r\n";
	BOOL bAddrOK = FALSE, bMaskOK = FALSE;

	/*
	 * Allocate a association information object,
	 * to contain user specified associating info.
	 * This object will be destroyed by ethernet thread.
	 */
	pifConfig = (__ETH_IP_CONFIG*)KMemAlloc(sizeof(__ETH_IP_CONFIG),KMEM_SIZE_TYPE_ANY);
	if(NULL == pifConfig)
	{
		goto __TERMINAL;
	}
	memset(pifConfig,0,sizeof(__ETH_IP_CONFIG));
	
	if(lpCmdObj->byParameterNum <= 1)
	{
		goto __TERMINAL;
	}

	/* Parse command line. */
	while(index < lpCmdObj->byParameterNum)
	{		
		if(strcmp(lpCmdObj->Parameter[index],"/d") == 0)
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			if(strcmp(lpCmdObj->Parameter[index],"enable") == 0)
			{
				/* Enable DHCP functions. */
				pifConfig->dwDHCPFlags = ETH_DHCPFLAGS_ENABLE;
			}
			else if(strcmp(lpCmdObj->Parameter[index],"disable") == 0)
			{
				/* Disable DHCP functions. */
				pifConfig->dwDHCPFlags = ETH_DHCPFLAGS_DISABLE;
			}
			else if(strcmp(lpCmdObj->Parameter[index],"restart") == 0)
			{
				/* Restart DHCP. */
				pifConfig->dwDHCPFlags = ETH_DHCPFLAGS_RESTART;
			}
			else if(strcmp(lpCmdObj->Parameter[index],"release") == 0)
			{
				/* Release DHCP configurations. */
				pifConfig->dwDHCPFlags = ETH_DHCPFLAGS_RELEASE;
			}
			else
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
		}
		else if(strcmp(lpCmdObj->Parameter[index],"/a") == 0)
		{
			/* Set IP address. */
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}						
			pifConfig->ipaddr.Address.ipv4_addr = inet_addr(lpCmdObj->Parameter[index]);
			pifConfig->ipaddr.AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			bAddrOK = TRUE;
		}		
		else if(strcmp(lpCmdObj->Parameter[index],"/m") == 0)
		{
			/* Set IP subnet mask. */
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			pifConfig->mask.Address.ipv4_addr = inet_addr(lpCmdObj->Parameter[index]);
			pifConfig->mask.AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			bMaskOK = TRUE;
		}
		else if(strcmp(lpCmdObj->Parameter[index],"/g") == 0)
		{
			/* Set default gateway. */
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			pifConfig->defgw.Address.ipv4_addr = inet_addr(lpCmdObj->Parameter[index]);
			pifConfig->defgw.AddressType = NETWORK_ADDRESS_TYPE_IPV4;
		}
		else
		{
			if(strlen(lpCmdObj->Parameter[index]) < 2)
			{
				/* Invalid interface name. */
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			strcpy(pifConfig->ethName,lpCmdObj->Parameter[index]);
		}
		index ++;
	}
	
	/*
	 * If IP address and mask are all specified 
	 * correctly,then assume the DHCP client
	 * functions on this interface should be disabled.
	 */
	if(bAddrOK && bMaskOK)
	{
		pifConfig->dwDHCPFlags = ETH_DHCPFLAGS_DISABLE;
	}
	
	/* Only one parameter specified,it's an error. */
	if((bAddrOK && !bMaskOK) || (!bAddrOK & bMaskOK))
	{
		_hx_printf(errmsg);
		goto __TERMINAL;
	}
	
	/* 
	 * Everything is OK,send a message to EthernetManager 
	 * to carry out the modification.
	 */
	pifConfig->protoType = NETWORK_PROTOCOL_TYPE_IPV4;
	EthernetManager.ConfigInterface(pifConfig->ethName,pifConfig);
	
	dwRetVal = SHELL_CMD_PARSER_SUCCESS;
	
__TERMINAL:
	if(pifConfig)
	{
		KMemFree(pifConfig,KMEM_SIZE_TYPE_ANY,0);
	}
	return dwRetVal;
}

/* 
 * Handler of showif command, dumpout a specified 
 * netif object in system. 
 */
static void ShowIf(struct netif* pIf)
{
	__GENERIC_NETIF* pGenif = pIf->pGenif;
	char* genif_name = "[NULL]";
	
	if (pGenif)
	{
		genif_name = pGenif->genif_name;
	}

	_hx_printf("  --------------------------------------\r\n");
	_hx_printf("  Inetface name : %c%c\r\n",pIf->name[0],pIf->name[1]);
	_hx_printf("    IPv4 address     : %s\r\n",inet_ntoa(pIf->ip_addr));
	_hx_printf("    IPv4 mask        : %s\r\n",inet_ntoa(pIf->netmask));
	_hx_printf("    IPv4 gateway     : %s\r\n",inet_ntoa(pIf->gw));
	_hx_printf("    Interface MTU    : %d\r\n",pIf->mtu);
	_hx_printf("    If flags         : 0x%X\r\n", pIf->flags);
	_hx_printf("    Associated genif : %s\r\n", genif_name);
	_hx_printf("    I/O routines     : 0x%X/0x%X\r\n", pIf->input, pIf->output);
}

/* Handler of iflist command. */
static DWORD iflist(__CMD_PARA_OBJ* lpCmdObj)
{
	struct netif* pIfList = netif_list;
	char if_name[MAX_ETH_NAME_LEN];

	if (lpCmdObj->byParameterNum < 2) 
	{
		/* No interface name specified,show all. */
		while (pIfList)
		{
			ShowIf(pIfList);
			pIfList = pIfList->next;
		}
	}
	else 
	{
		/* Show out the specified netif. */
		strncpy(if_name, lpCmdObj->Parameter[1], sizeof(if_name));
		if (strlen(if_name) < 2)
		{
			_hx_printf("Invalid interface name.\r\n");
			goto __TERMINAL;
		}
		while (pIfList)
		{
			if ((pIfList->name[0] == if_name[0]) && (pIfList->name[1] == if_name[1]))
			{
				break;
			}
			pIfList = pIfList->next;
		}
		if (pIfList)
		{
			ShowIf(pIfList);
		}
		else
		{
			_hx_printf("No interface found.\r\n");
			goto __TERMINAL;
		}
	}

__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;
}

/* Show out all genif in system. */
static DWORD showgif(__CMD_PARA_OBJ* pCmdObj)
{
	int nIndex = -1;
	int nShowed = 0;
	
	if (pCmdObj->byParameterNum < 2)
	{
		/* No genif index specified. */
		nShowed = NetworkManager.ShowGenif(-1);
	}
	else
	{
		nIndex = atoi(pCmdObj->Parameter[1]);
		nShowed = NetworkManager.ShowGenif(nIndex);
	}
	_hx_printf("  \r\n[%d] genif listed.\r\n", nShowed);
	return SHELL_CMD_PARSER_SUCCESS;
}

/* Set configuration to a genif. */
static DWORD setgif(__CMD_PARA_OBJ* lpCmdObj)
{
	DWORD dwRetVal = SHELL_CMD_PARSER_FAILED;
	char index = 1;
	char* errmsg = "  Bad parameter(s).\r\n";
	__COMMON_NETWORK_ADDRESS comm_addr[3];
	BOOL bAddrOk = FALSE, bMaskOk = FALSE, bGwOk = FALSE;
	int genif_index = -1;

	if (lpCmdObj->byParameterNum <= 1)
	{
		/* Too few inputs. */
		_hx_printf("setgif gif_index /a ip-addr /m mask /g gw\r\n");
		dwRetVal = SHELL_CMD_PARSER_SUCCESS;
		goto __TERMINAL;
	}

	/* Parse command line. */
	while (index < lpCmdObj->byParameterNum)
	{
		if (strcmp(lpCmdObj->Parameter[index], "/a") == 0)
		{
			/* Set IP address. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			comm_addr[0].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			comm_addr[0].Address.ipv4_addr = inet_addr(lpCmdObj->Parameter[index]);
			bAddrOk = TRUE;
		}
		else if (strcmp(lpCmdObj->Parameter[index], "/m") == 0)
		{
			/* Set IP subnet mask. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			comm_addr[1].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			comm_addr[1].Address.ipv4_addr = inet_addr(lpCmdObj->Parameter[index]);
			bMaskOk = TRUE;
		}
		else if (strcmp(lpCmdObj->Parameter[index], "/g") == 0)
		{
			/* Set default gateway. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				_hx_printf(errmsg);
				goto __TERMINAL;
			}
			comm_addr[2].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
			comm_addr[2].Address.ipv4_addr = inet_addr(lpCmdObj->Parameter[index]);
			bGwOk = TRUE;
		}
		else
		{
			genif_index = atoi(lpCmdObj->Parameter[index]);
		}
		index++;
	}

	/* Just configure the genif if all parameters are OK. */
	if (bAddrOk && bMaskOk && bGwOk)
	{
		/*
		 * Release DHCP configuration on this genif
		 * and disable DHCP on the genif, since
		 * static address is used.
		 */
		NetworkManager.SetGenifConfig(genif_index, GENIF_CONFIG_DHCP, 2, NULL);
		NetworkManager.SetGenifConfig(genif_index, GENIF_CONFIG_DHCP, 1, NULL);

		int ret_val = NetworkManager.AddGenifAddress(genif_index,
			NETWORK_PROTOCOL_TYPE_IPV4,
			comm_addr,
			3, FALSE);
		if (ret_val == ERR_OK)
		{
			_hx_printf("set genif addr ok.\r\n");
		}
		else
		{
			_hx_printf("set genif addr fail[%d].\r\n", ret_val);
		}
	}

	dwRetVal = SHELL_CMD_PARSER_SUCCESS;

__TERMINAL:
	return dwRetVal;
}

/* 
 * showint command,display statistics 
 * information of ethernet interface. 
 */
static DWORD showint(__CMD_PARA_OBJ* lpCmdObj)
{
	char if_name[MAX_ETH_NAME_LEN];

	if (lpCmdObj->byParameterNum < 2) /* No interface name specified. */
	{
		//Just send a message to ethernet main thread.
		EthernetManager.ShowInt(NULL);
	}
	else /* Interface name specified. */
	{
		strncpy(if_name, lpCmdObj->Parameter[1], sizeof(if_name));
		if (strlen(if_name) < 2)
		{
			_hx_printf("Invalid interface name.\r\n");
			return NET_CMD_SUCCESS;
		}
		EthernetManager.ShowInt(if_name);
	}
	return NET_CMD_SUCCESS;
}

static DWORD showdbg(__CMD_PARA_OBJ* lpCmdObj)
{
	/*
	 * Dump out ethernet related statistics information
	 * directly.
	 */
	_hx_printf("Total ethernet buff number: %d.\r\n",
		EthernetManager.nTotalEthernetBuffs);
	_hx_printf("Pending queue size: %d.\r\n",
		EthernetManager.nBuffListSize);
	_hx_printf("Broadcast queue size: %d.\r\n",
		EthernetManager.nBroadcastSize);
	_hx_printf("Total driver send_queue sz: %d.\r\n",
		EthernetManager.nDrvSendingQueueSz);
	_hx_printf("Droped broadcast frame: %d.\r\n",
		EthernetManager.nDropedBcastSize);
	return NET_CMD_SUCCESS;
}

/* Show out IP layer statistics counter. */
static void ipstat()
{
	/* Just show out lwIP statistics counter. */
	_hx_printf("IP layer statistics counter:\r\n");
	_hx_printf("  rx             : %d\r\n", lwip_stats.ip.recv);
	_hx_printf("  tx             : %d\r\n", lwip_stats.ip.xmit);
	_hx_printf("  forwarded      : %d\r\n", lwip_stats.ip.fw);
	_hx_printf("  ctx_xmit       : %d\r\n", lwip_stats.ip.ctx_xmit);
	_hx_printf("  ctx_xmit_err   : %d\r\n", lwip_stats.ip.ctx_xmit_err);
	_hx_printf("  droped         : %d\r\n", lwip_stats.ip.drop);
	_hx_printf("  invalid length : %d\r\n", lwip_stats.ip.lenerr);
	_hx_printf("  checksum err   : %d\r\n", lwip_stats.ip.chkerr);
	_hx_printf("  routing err    : %d\r\n", lwip_stats.ip.rterr);
	_hx_printf("  out of memory  : %d\r\n", lwip_stats.ip.memerr);
	_hx_printf("  options err    : %d\r\n", lwip_stats.ip.opterr);
	_hx_printf("  protocol err   : %d\r\n", lwip_stats.ip.proterr);
	_hx_printf("  cachehit       : %d\r\n", lwip_stats.ip.cachehit);
	_hx_printf("  general err    : %d\r\n", lwip_stats.ip.err);
}

/* Show out TCP statistics counter. */
static void tcpstat()
{
	/* Just show out lwIP statistics counter. */
	_hx_printf("TCP statistics counter:\r\n");
	_hx_printf("  rx             : %d\r\n", lwip_stats.tcp.recv);
	_hx_printf("  tx             : %d\r\n", lwip_stats.tcp.xmit);
	_hx_printf("  forwarded      : %d\r\n", lwip_stats.tcp.fw);
	_hx_printf("  droped         : %d\r\n", lwip_stats.tcp.drop);
	_hx_printf("  invalid length : %d\r\n", lwip_stats.tcp.lenerr);
	_hx_printf("  checksum err   : %d\r\n", lwip_stats.tcp.chkerr);
	_hx_printf("  routing err    : %d\r\n", lwip_stats.tcp.rterr);
	_hx_printf("  out of memory  : %d\r\n", lwip_stats.tcp.memerr);
	_hx_printf("  options err    : %d\r\n", lwip_stats.tcp.opterr);
	_hx_printf("  protocol err   : %d\r\n", lwip_stats.tcp.proterr);
	_hx_printf("  cachehit       : %d\r\n", lwip_stats.tcp.cachehit);
	_hx_printf("  general err    : %d\r\n", lwip_stats.tcp.err);
}

/* Show out link layer statistics counter. */
static void linkstat()
{
	_hx_printf("Link layer statistics counter:\r\n");
	_hx_printf("  rx             : %d\r\n", lwip_stats.link.recv);
	_hx_printf("  tx             : %d\r\n", lwip_stats.link.xmit);
	_hx_printf("  forwarded      : %d\r\n", lwip_stats.link.fw);
	_hx_printf("  droped         : %d\r\n", lwip_stats.link.drop);
	_hx_printf("  invalid length : %d\r\n", lwip_stats.link.lenerr);
	_hx_printf("  checksum err   : %d\r\n", lwip_stats.link.chkerr);
	_hx_printf("  routing err    : %d\r\n", lwip_stats.link.rterr);
	_hx_printf("  out of memory  : %d\r\n", lwip_stats.link.memerr);
	_hx_printf("  options err    : %d\r\n", lwip_stats.link.opterr);
	_hx_printf("  protocol err   : %d\r\n", lwip_stats.link.proterr);
	_hx_printf("  cachehit       : %d\r\n", lwip_stats.link.cachehit);
	_hx_printf("  general err    : %d\r\n", lwip_stats.link.err);
}

/* Show usage of netstat. */
static void netstatUsage(char* sub_cmd)
{
	_hx_printf("Usage:\r\n");
	_hx_printf("  netstat [ip] : Show IP layer statistics counter.\r\n");
	_hx_printf("  netstat [link] : Show link layer statistics counter.\r\n");
	_hx_printf("  netstat [tcp] : Show tcp statistics counter.\r\n");
	return;
}

/* sub command types. */
typedef enum{
	ip,
	link,
	tcp,
}__NETSTAT_SUB_CMD;

/* Entry point of netstat command. */
static DWORD netstat(__CMD_PARA_OBJ* lpCmdObj)
{
	__CMD_PARA_OBJ* pCurCmdObj = lpCmdObj;
	int index = 1;
	__NETSTAT_SUB_CMD sub_cmd;

	BUG_ON(NULL == pCurCmdObj);

	if (pCurCmdObj->byParameterNum <= 1)
	{
		netstatUsage(NULL);
		goto __TERMINAL;
	}

	while (index < lpCmdObj->byParameterNum)
	{
		if (strcmp(pCurCmdObj->Parameter[index], "ip") == 0)
		{
			/* Obtain the number of NAT entry to show. */
			index++;
			sub_cmd = ip;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "link") == 0)
		{
			/* Obtain interface name. */
			index++;
			sub_cmd = link;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "tcp") == 0)
		{
			/* Obtain interface name. */
			index++;
			sub_cmd = tcp;
		}
		else
		{
			netstatUsage(NULL);
			goto __TERMINAL;
		}
		index++;
	}

	switch (sub_cmd)
	{
	case ip:
		ipstat();
		break;
	case link:
		linkstat();
		break;
	case tcp:
		tcpstat();
		break;
	default:
		break;
	}

__TERMINAL:
	return NET_CMD_SUCCESS;
}

//WiFi association operation,associate to a specified WiFi SSID.
static DWORD assoc(__CMD_PARA_OBJ* lpCmdObj)
{
	 DWORD                  dwRetVal   = SHELL_CMD_PARSER_FAILED;
	__WIFI_ASSOC_INFO       AssocInfo;
	BYTE                    index       = 1;

	//Initialize to default value.
	AssocInfo.ssid[0] = 0;
	AssocInfo.key[0]  = 0;
	AssocInfo.mode    = 0;
	AssocInfo.channel = 0;
	
	if(lpCmdObj->byParameterNum <= 1)
	{
		goto __TERMINAL;
	}

	while(index < lpCmdObj->byParameterNum)
	{		
		if(strcmp(lpCmdObj->Parameter[index],"/k") == 0) //Key of association.		
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}
			if(strlen(lpCmdObj->Parameter[index]) >= 24)  //Key is too long.
			{
				_hx_printf("  Error: The key you specified is too long.\r\n");
				goto __TERMINAL;
			}
			//Copy the key into information object.
			strcpy(AssocInfo.key,lpCmdObj->Parameter[index]);
		}
		else if(strcmp(lpCmdObj->Parameter[index],"/m") == 0) //Assoction mode.
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}						
			if(strcmp(lpCmdObj->Parameter[index],"adhoc") == 0)  //AdHoc mode.
			{
				AssocInfo.mode = 1;
			}
		}
		else if(strcmp(lpCmdObj->Parameter[index],"/c") == 0)  //Association channel.
		{
			index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}
			AssocInfo.channel = atoi(lpCmdObj->Parameter[index]);
		}
		else
		{
			//Default parameter as SSID.
			//index ++;
			if(index >= lpCmdObj->byParameterNum)
			{
				_hx_printf("  Error: Invalid parameter.\r\n");
				goto __TERMINAL;
			}

			if(strlen(lpCmdObj->Parameter[index]) >= 24)  //SSID too long.
			{
				_hx_printf("  Error: SSID you specified is too long.\r\n");
				goto __TERMINAL;
			}

			//Copy the SSID into information object.
			strcpy(AssocInfo.ssid,lpCmdObj->Parameter[index]);		
		}
		index ++;
	}
	
	//Re-check the parameters.
	if(0 == AssocInfo.ssid[0])
	{
		_hx_printf("  Error: Please specifiy the SSID to associate with.\r\n");
		goto __TERMINAL;
	}
	
	//Everything is OK.
	EthernetManager.Assoc(NULL,&AssocInfo);
	
	dwRetVal = SHELL_CMD_PARSER_SUCCESS;
	
__TERMINAL:
	return dwRetVal;
}


//Scan WiFi networks and show the scanning result.
static DWORD scan(__CMD_PARA_OBJ* lpCmdObj)
{
	EthernetManager.Rescan(NULL);
	return NET_CMD_SUCCESS;
}

#ifdef __CFG_NET_DHCP_SERVER
/* Show out dhcpd's usage. */
static void dhcpdUsage()
{
	_hx_printf("  dhcpd: start or stop DHCP server on a given interface.\r\n");
	return;
}
/*
 * DHCP Server control command,used to start or stop DHCP server damon,change
 * configurations of DHCP server.
 */
static DWORD dhcpd(__CMD_PARA_OBJ* lpCmdObj)
{
	char* subcmd = NULL;

	if (lpCmdObj->byParameterNum < 2)
	{
		dhcpdUsage();
		return NET_CMD_SUCCESS;
	}
	subcmd = lpCmdObj->Parameter[1];
	if (strlen(subcmd) < 2)
	{
		_hx_printf("  Invalid interface name.\r\n");
		return NET_CMD_SUCCESS;
	}
	if (strcmp(subcmd, "list") == 0)
	{
		ShowDhcpAlloc();
		return NET_CMD_SUCCESS;
	}
	_hx_printf("  Wait a short while...\r\n");
	DHCPSrv_Start_Onif(subcmd);
	return NET_CMD_SUCCESS;
}
#endif

#ifdef __CFG_NET_PPPOE
/*
 * PPPoE function control command.
 * Used to start,stop,or configure PPPoE functions.
 * Command and it's options:
 * pppoe create [instance_name] [eth_if] /u [username] /p [passwd] /a [auth_type]
 * pppoe start instance_name
 * pppoe stop instance_name
 * pppoe delete instance_name
 */
static void pppoeShowUsage(char* subcmd)
{
	if (NULL == subcmd)
	{
	__BEGIN:
		_hx_printf("Usage:\r\n");
		_hx_printf("  pppoe create [inst_name] [eth_if] /u [username] /p [passwd] /a [auth_type]\r\n");
		_hx_printf("  pppoe start [inst_name]\r\n");
		_hx_printf("  pppoe stop [inst_name]\r\n");
		_hx_printf("  pppoe delete [inst_name]\r\n");
		_hx_printf("  pppoe list all\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "create"))
	{
		_hx_printf("  pppoe create [inst_name] [eth_if] /u [username] /p [passwd] /a [auth_type]\r\n");
		_hx_printf("  where:\r\n");
		_hx_printf("    eth_if: ethernet interface name.\r\n");
		_hx_printf("    username: username used to authenticate.\r\n");
		_hx_printf("    passwd: password used to authenticate.\r\n");
		_hx_printf("    auth_type: authentication type,none,pap,or chap.\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "start"))
	{
		_hx_printf("  Please specify the PPPoE instance name.\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "stop"))
	{
		_hx_printf("  Please specify the PPPoE instance to stop.\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "delete"))
	{
		_hx_printf("  Please specify the PPPoE instance to delete.\r\n");
		return;
	}
	/* All other subcmd will lead the showing of all usage. */
	goto __BEGIN;
}

/* Local helpers to make command process procedure simple. */
static void __CreatePPPoEInstance(char* inst_name, char* eth_if, char* username, char* passwd,
	char* auth_type)
{
	__PPPOE_AUTH_TYPE auth = PPPOE_AUTH_NONE;

	if (0 == strcmp(auth_type, "pap"))
	{
		auth = PPPOE_AUTH_PAP;
	}
	if (0 == strcmp(auth_type, "chap"))
	{
		auth = PPPOE_AUTH_CHAP;
	}
	if (NULL != pppoeManager.CreatePPPoEInstance(eth_if, inst_name, username, passwd, auth))
	{
		_hx_printf("  Create PPPoE instance[%s] successfully.\r\n", inst_name);
	}
	else
	{
		_hx_printf("  Failed to create PPPoE instance[%s].\r\n",inst_name);
	}
	return;
}

/* Identifier of pppoe sub-command. */
typedef enum {
	create = 0,
	start,
	stop,
	delete,
	test
}__PPPOE_SUB_CMD;

/* Test PPPoE's performance and debugging it. */
static void testPPPoE(int times)
{
	__PPPOE_INSTANCE* pInstance = pppoeManager.pInstanceList;
	int count = 0;

	if (NULL == pInstance) /* No instance yet. */
	{
		return;
	}
	/* Only test the first PPPoE instance. */
	for (int i = 0; i < times; i++)
	{
		_hx_printf(">>>>>>>>> PPPoE test begin: iteration# %d\r\n", i);
		if (!pppoeManager.StartPPPoEByName(pInstance->session_name))
		{
			_hx_printf(">>>>>>>>> Start PPPoE failed.\r\n");
			return;
		}
		Sleep(1000);
		if (!pppoeManager.StopPPPoEByName(pInstance->session_name))
		{
			_hx_printf(">>>>>>>>> Stop PPPoE session failed.\r\n");
			return;
		}
		_hx_printf(">>>>>>>>> PPPoE test end: iteration# %d\r\n", i);
		Sleep(1000);

		/* Pause 5 minutes after 500 timers. */
		count++;
		if (count > 500)
		{
			count = 0;
			Sleep(1000 * 60 * 5);
		}
	}
}

/* Default password and user name,to simplify testing. */
#define PPPOE_DEFAULT_USERNAME_QD "053202039989"
#define PPPOE_DEFAULT_PASSWORD_QD "60767168"

/* Default password and user name,to simplify testing. */
#define PPPOE_DEFAULT_USERNAME_BJ "01012187137"
#define PPPOE_DEFAULT_PASSWORD_BJ "858137"

#define PPPOE_DEFAULT_USERNAME PPPOE_DEFAULT_USERNAME_BJ
#define PPPOE_DEFAULT_PASSWORD PPPOE_DEFAULT_PASSWORD_BJ

static DWORD pppoe(__CMD_PARA_OBJ* lpCmdObj)
{
	char instance[32];
	char eth_if[32];
	char username[32];
	char passwd[32];
	char auth_type[8];
	int index = 1;
	__PPPOE_SUB_CMD sub_cmd;
	DWORD dwRetVal = SHELL_CMD_PARSER_SUCCESS;
	__CMD_PARA_OBJ*  pCurCmdObj = lpCmdObj;
	int test_times = 0;
	char str_test_times[16];

	//pppoeCmdEntry();
	//goto __TERMINAL;

	if (pCurCmdObj->byParameterNum <= 1)
	{
		pppoeShowUsage(NULL);
		goto __TERMINAL;
	}
	if (pCurCmdObj->byParameterNum <= 2)
	{
		pppoeShowUsage(pCurCmdObj->Parameter[1]);
		goto __TERMINAL;
	}

	/* Set default value of all command options. */
	instance[0] = eth_if[0] = 0;
	strncpy(username, PPPOE_DEFAULT_USERNAME, sizeof(username));
	strncpy(passwd, PPPOE_DEFAULT_PASSWORD, sizeof(passwd));
	strncpy(auth_type, "none",sizeof(auth_type));

	while (index < lpCmdObj->byParameterNum)
	{
		if (strcmp(pCurCmdObj->Parameter[index], "create") == 0)
		{
			/* Obtain PPPoE instance name. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(instance,pCurCmdObj->Parameter[index],sizeof(instance));
			/* Obtain ethernet interface name. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(eth_if, pCurCmdObj->Parameter[index], sizeof(eth_if));
			sub_cmd = create;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "start") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(instance, pCurCmdObj->Parameter[index], sizeof(instance));
			sub_cmd = start;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "stop") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(instance, pCurCmdObj->Parameter[index], sizeof(instance));
			sub_cmd = stop;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "delete") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(instance, pCurCmdObj->Parameter[index], sizeof(instance));
			sub_cmd = delete;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "list") == 0)
		{
			ListPPPoE();
			goto __TERMINAL;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "test") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(str_test_times, pCurCmdObj->Parameter[index], sizeof(str_test_times));
			test_times = atol(str_test_times);
			sub_cmd = test;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "/u") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(username, pCurCmdObj->Parameter[index], sizeof(username));
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "/p") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(passwd, pCurCmdObj->Parameter[index], sizeof(passwd));
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "/a") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				pppoeShowUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(auth_type, pCurCmdObj->Parameter[index], sizeof(auth_type));
		}
		else
		{
			pppoeShowUsage(pCurCmdObj->Parameter[1]);
			goto __TERMINAL;
		}
		index++;
	}

	/* Call the sub command handler accordingly. */
	switch (sub_cmd)
	{
	case create:
		__CreatePPPoEInstance(instance, eth_if, username, passwd, auth_type);
		break;
	case start:
		pppoeManager.StartPPPoEByName(instance);
		break;
	case stop:
		pppoeManager.StopPPPoEByName(instance);
		break;
	case delete:
		pppoeManager.DestroyPPPoEInstanceByName(instance);
		break;
	case test:
		testPPPoE(test_times);
		break;
	default:
		BUG();
	}

__TERMINAL:
	return dwRetVal;
}
#endif

/* Sub command of NAT/DPI... */
typedef enum {
	enable = 0,
	disable,
	list,
	stat,
}__SUB_CMD;

/* NAT control command. */
#ifdef __CFG_NET_NAT

/* Usage of the command. */
static void natUsage(char* subcmd)
{
	if (NULL == subcmd)
	{
	__BEGIN:
		_hx_printf("Usage:\r\n");
		_hx_printf("  nat enable [int_name]\r\n");
		_hx_printf("  nat disable [int_name]\r\n");
		_hx_printf("  nat stat\r\n");
		_hx_printf("  nat list [entry_num]\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "enable"))
	{
		_hx_printf("  nat enable [int_name]\r\n");
		_hx_printf("  where:\r\n");
		_hx_printf("    int_name: interface name to enable NAT.\r\n");
		return;
	}
	if (0 == strcmp(subcmd, "disable"))
	{
		_hx_printf("  nat disable [int_name]\r\n");
		_hx_printf("  where:\r\n");
		_hx_printf("    int_name: interface name to disable NAT.\r\n");
		return;
	}
	/* All other sub-command string will lead showing of general usage. */
	goto __BEGIN;
}

/* Local helper routine to show out one easy NAT entry. */
static void ShowNatEntry(__EASY_NAT_ENTRY* pEntry)
{
	/* Output as: [x.x.x.x : p1]->[y.y.y.y : p2], pro:17, ms:10, mt:1000 */
	_hx_printf("    [%s:%d]->", inet_ntoa(pEntry->srcAddr_bef), pEntry->srcPort_bef);
	_hx_printf("[%s:%d]\r\n", inet_ntoa(pEntry->srcAddr_aft), pEntry->srcPort_aft);
	_hx_printf("    dst[%s:%d], pro:%d, ms:%d, mt:%d\r\n",
		inet_ntoa(pEntry->dstAddr_bef),
		pEntry->dstPort_bef,
		pEntry->protocol,
		pEntry->ms,
		pEntry->match_times);
}

static DWORD nat(__CMD_PARA_OBJ* lpCmdObj)
{
	__CMD_PARA_OBJ* pCurCmdObj = lpCmdObj;
	int index = 1;
	char if_name[MAX_ETH_NAME_LEN];
	__SUB_CMD sub_cmd;
	BOOL bEnable = FALSE;
	size_t ss_num = 0;

	BUG_ON(NULL == pCurCmdObj);

	if (pCurCmdObj->byParameterNum <= 1)
	{
		natUsage(NULL);
		goto __TERMINAL;
	}

	while (index < lpCmdObj->byParameterNum)
	{
		if (strcmp(pCurCmdObj->Parameter[index], "list") == 0)
		{
			/* Obtain the number of NAT entry to show. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				ss_num = 0;
			}
			else
			{
				ss_num = atoi(pCurCmdObj->Parameter[index]);
			}
			sub_cmd = list;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "enable") == 0)
		{
			/* Obtain interface name. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				natUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(if_name, pCurCmdObj->Parameter[index], sizeof(if_name));
			sub_cmd = enable;
			bEnable = TRUE;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "disable") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				natUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(if_name, pCurCmdObj->Parameter[index], sizeof(if_name));
			sub_cmd = disable;
			bEnable = FALSE;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "stat") == 0)
		{
			index++;
			sub_cmd = stat;
		}
		else
		{
			natUsage(NULL);
			goto __TERMINAL;
		}
		index++;
	}

	/* Enable or disable NAT on interface. */
	if ((sub_cmd == enable) || (sub_cmd == disable))
	{
		if (NatManager.enatEnable(if_name, bEnable, TRUE))
		{
			_hx_printf("Set NAT flag on interface[%s] OK.\r\n", if_name);
		}
		else
		{
			_hx_printf("Set NAT flag on interface[%s] failed.\r\n", if_name);
		}
	}
	else if (sub_cmd == list)
	{
		NatManager.ShowNatSession(&NatManager, ss_num);
	}
	else if (sub_cmd == stat)
	{
		_hx_printf("  Nat entry num: %d\r\n", NatManager.stat.entry_num);
		_hx_printf("  Hash deep: %d\r\n", NatManager.stat.hash_deep);
		_hx_printf("  Total match times: %d\r\n", NatManager.stat.match_times);
		_hx_printf("  Total trans times: %d\r\n", NatManager.stat.trans_times);
		_hx_printf("  NAT entry with the maximal hash-deep:\r\n");
		ShowNatEntry(&NatManager.stat.deepNat);
	}

__TERMINAL:
	return NET_CMD_SUCCESS;
}

#endif  //__CFG_NET_NAT.

/* DPI function command. */
#if defined(__CFG_NET_DPI)

/* DPI usage. */
static void dpiUsage(char* sub_cmd)
{
	_hx_printf("Usage:\r\n");
	_hx_printf("  dpi enable [int_name]\r\n");
	_hx_printf("  dpi disable [int_name]\r\n");
	_hx_printf("  dpi stat\r\n");
}

static DWORD dpi(__CMD_PARA_OBJ* lpCmdObj)
{
	__CMD_PARA_OBJ* pCurCmdObj = lpCmdObj;
	int index = 1;
	char if_name[MAX_ETH_NAME_LEN];
	__SUB_CMD sub_cmd;
	BOOL bEnable = FALSE;
	size_t ss_num = 0;

	BUG_ON(NULL == pCurCmdObj);

	if (pCurCmdObj->byParameterNum <= 1)
	{
		dpiUsage(NULL);
		goto __TERMINAL;
	}

	while (index < lpCmdObj->byParameterNum)
	{
		if (strcmp(pCurCmdObj->Parameter[index], "enable") == 0)
		{
			/* Obtain interface name. */
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				natUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(if_name, pCurCmdObj->Parameter[index], sizeof(if_name));
			sub_cmd = enable;
			bEnable = TRUE;
		}
		else if (strcmp(pCurCmdObj->Parameter[index], "disable") == 0)
		{
			index++;
			if (index >= lpCmdObj->byParameterNum)
			{
				natUsage(pCurCmdObj->Parameter[1]);
				goto __TERMINAL;
			}
			strncpy(if_name, pCurCmdObj->Parameter[index], sizeof(if_name));
			sub_cmd = disable;
			bEnable = FALSE;
		}
		else
		{
			dpiUsage(NULL);
			goto __TERMINAL;
		}
		index++;
	}

	/* Enable or disable NAT on interface. */
	if ((sub_cmd == enable) || (sub_cmd == disable))
	{
		if (DPIManager.dpiEnable(if_name, bEnable))
		{
			_hx_printf("Set DPI flag on interface[%s] OK.\r\n", if_name);
		}
		else
		{
			_hx_printf("Set DPI flag on interface[%s] failed.\r\n", if_name);
		}
	}
	else if (sub_cmd == stat)
	{
		/* Show DPI statistics information. */
	}

__TERMINAL:
	return NET_CMD_SUCCESS;
}

/* Start or stop capture on a specified genif. */
static DWORD capstart(__CMD_PARA_OBJ* lpCmdObj)
{
	int genif_index = -1;
	
	if (lpCmdObj->byParameterNum <= 1)
	{
		_hx_printf("  Usage: capstart [genif-index]\r\n");
		return NET_CMD_SUCCESS;
	}
	genif_index = atol(lpCmdObj->Parameter[1]);
	/* Just invoke the command handler. */
	CaptureStart(genif_index);
	return NET_CMD_SUCCESS;
}

static DWORD capstop(__CMD_PARA_OBJ* lpCmdObj)
{
	int genif_index = -1;

	if (lpCmdObj->byParameterNum <= 1)
	{
		_hx_printf("  Usage: capstop [genif-index]\r\n");
		return NET_CMD_SUCCESS;
	}
	genif_index = atol(lpCmdObj->Parameter[1]);
	/* Just invoke the command handler. */
	CaptureStop(genif_index);
	return NET_CMD_SUCCESS;
}

#endif //__CFG_NET_DPI
