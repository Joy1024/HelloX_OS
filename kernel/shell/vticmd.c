//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 18, 2022
//    Module Name               : vticmd.c
//    Module Funciton           : 
//    Description               : 
//                                Shell command for VTI management functions,
//                                such as add/del/modify VTI.
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

#include "genif.h"
#include "ipsec/ipsectun.h"
#include "gre/gretun.h"
#include "hhtt/hhtt.h"
#include "shell.h"
#include "vticmd.h"

/* Handlers to process vti sub command. */
static unsigned long __vti_add(const char* argv[], int argc);
static unsigned long __vti_del(const char* argv[], int argc);
static unsigned long __vti_set(const char* argv[], int argc);

/*
 * The map between vti command and it's
 * hander, it's description string.
 * The vti command just search this map
 * using the subcommand string, to locate the handler
 * of sub command.
 */
static struct __vti_subcmd_struct {
	const char* subcmd_name;
	unsigned long(*CommandHandler)(const char*[], int);
	const char* subcmd_description;
}vti_subcmd_map[] = {
	{ "add",    __vti_add,       "  vti add : Add one VTI into system."},
	{ "del",    __vti_del,       "  vti del : Del one VTI from system."},
	{ "set",    __vti_set,       "  vti set : Set one VTI's attribute."},
	{ NULL,        NULL,            NULL},
};

/* Add one IPSec VTI interface into system. */
static unsigned long __vti_add(const char* argv[], int argc)
{
	BOOL bResult = FALSE;

	if (argc < 5)
	{
		_hx_printf("Usage: vti add type[ipsec/gre/ht2] name parent_name ipaddr\r\n");
		goto __TERMINAL;
	}
	
	/* Invoke specific add routine by type. */
	if (0 == strcmp(argv[1], "ipsec"))
	{
		bResult = ipsectun_add(argv[2], argv[3], argv[4]);
	}
	if (0 == strcmp(argv[1], "gre"))
	{
		bResult = gretun_add(argv[2], argv[3], argv[4]);
	}
	if (0 == strcmp(argv[1], "ht2"))
	{
		bResult = ht2tun_add(argv[2], argv[3], argv[4]);
	}

__TERMINAL:
	if (bResult)
	{
		_hx_printf("Add vti success.\r\n");
	}
	else {
		_hx_printf("Failed to add vti.\r\n");
	}
	return 0;
}

/* Del one IPSec VTI interface from system. */
static unsigned long __vti_del(const char* argv[], int argc)
{
	BOOL bResult = FALSE;

	if (argc < 3)
	{
		_hx_printf("Usage: vti del type[ipsec/gre/ht2] name\r\n");
		goto __TERMINAL;
	}

	/* Invoke specific add routine by type. */
	if (0 == strcmp(argv[1], "ipsec"))
	{
	}
	if (0 == strcmp(argv[1], "gre"))
	{
	}
	if (0 == strcmp(argv[1], "ht2"))
	{
		bResult = ht2tun_del(argv[2]);
	}

__TERMINAL:
	if (bResult)
	{
		_hx_printf("delete vti success.\r\n");
	}
	else {
		_hx_printf("Failed to delete vti.\r\n");
	}
	return 0;
}

/* Local helper to show the usage of setter. */
static void __usage_set()
{
	_hx_printf("Usage: vti set type[ht2/ipsec] ifname attribute value\r\n");
}

/* Change attributes of one VTI interface. */
static unsigned long __vti_set(const char* argv[], int argc)
{
	if (argc < 5)
	{
		/* Show usage. */
		__usage_set();
		goto __TERMINAL;
	}
	
	/* Invoke protocol specific setting routine. */
	if (0 == strcmp("ht2", argv[1]))
	{
		/* Set HT2 tunnel attributes. */
		ht2tun_set(argv[2], argv[3], argv[4]);
		goto __TERMINAL;
	}

	/* Invalid protocol. */
	__usage_set();

__TERMINAL:
	return 0;
}

/* Main entry routine of vti command. */
unsigned long vti_entry(__CMD_PARA_OBJ* lpCmdObj)
{
	int argc = 0;

	argc = lpCmdObj->byParameterNum;
	if (argc < 2)
	{
		/* Just show out all available commands under ipsec command. */
		for (int i = 0; ; i++)
		{
			if (NULL == vti_subcmd_map[i].subcmd_name)
			{
				break;
			}
			printf("%s\r\n", vti_subcmd_map[i].subcmd_description);
		}
		goto __TERMINAL;
	}

	/* Lookup subcommand map and invoke the handler. */
	for (int i = 0; ; i++)
	{
		if (NULL == vti_subcmd_map[i].subcmd_name)
		{
			/* No subcmd matched. */
			printf("  Invalid sub command.\r\n");
			break;
		}
		if (0 == strcmp(lpCmdObj->Parameter[1], vti_subcmd_map[i].subcmd_name))
		{
			vti_subcmd_map[i].CommandHandler(&lpCmdObj->Parameter[1], argc - 1);
			break;
		}
	}

__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;
}
