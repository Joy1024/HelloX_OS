//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Oct 10, 2023
//    Module Name               : cfgload.c
//    Module Funciton           : 
//                                Configuration management functions for
//                                network subsystem. Includes the config
//                                file's loading, parsing, retrieve, and
//                                so on.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <cJSON/cJSON.h>

#include "lwip/inet.h"
#include "lwip/netif.h"

#include "netcfg.h"
#include "genif.h"
#include "netmgr.h"
#include "netglob.h"
#include "routmgr/rtmgr.h"
#include "hhtt/hhtt.h"

/* Handle of the configure file. */
static HANDLE cfg_file = NULL;
/* Root json object after parsing. */
static cJSON* cfg_root = NULL;
/* Configure array of all genifs. */
static cJSON* cfg_genif_array = NULL;
/* Array for static routes. */
static cJSON* cfg_route_array = NULL;
/* Array for ht2 vti array. */
static cJSON* cfg_ht2vti_array = NULL;

/* Error code for this file. */
#define CFGLOAD_ERROR_OUTOFMEMORY       -1
#define CFGLOAD_ERROR_OPENFILE          -2
#define CFGLOAD_ERROR_READFILE          -3
#define CFGLOAD_ERROR_FILEEMPTY         -4
#define CFGLOAD_ERROR_FILEPARSE         -5
#define CFGLOAD_ERROR_NOGENIFARRAY      -6
#define CFGLOAD_ERROR_GETJSONITEM       -7
#define CFGLOAD_ERROR_NOROUTEARRAY      -8

/* 
 * Load network configuration file into memory
 * and parse it into json object. So the configurations
 * could be retrieved later.
 */
BOOL LoadNetworkConfig()
{
	BOOL bResult = FALSE;
	unsigned int file_sz = 0, file_read = 0;
	char* file_buff = NULL;
	int error_code = 0;

	/* Try to open the configure file. */
	cfg_file = CreateFile(NETWORK_CONFIG_FILENAME,
		FILE_ACCESS_READWRITE, 0, NULL);
	if (NULL == cfg_file)
	{
		error_code = CFGLOAD_ERROR_OPENFILE;
		goto __TERMINAL;
	}
	file_sz = GetFileSize(cfg_file, NULL);
	if (0 == file_sz)
	{
		error_code = CFGLOAD_ERROR_FILEEMPTY;
		goto __TERMINAL;
	}
	file_buff = (char*)_hx_malloc(file_sz);
	if (NULL == file_buff)
	{
		error_code = CFGLOAD_ERROR_OUTOFMEMORY;
		goto __TERMINAL;
	}
	if (!ReadFile(cfg_file, file_sz, file_buff, &file_read))
	{
		error_code = CFGLOAD_ERROR_READFILE;
		goto __TERMINAL;
	}
	if (file_read != file_sz)
	{
		error_code = CFGLOAD_ERROR_READFILE;
		goto __TERMINAL;
	}

	/* Parse the configure file into json object. */
	cfg_root = cJSON_Parse(file_buff);
	if (NULL == cfg_root)
	{
		error_code = CFGLOAD_ERROR_FILEPARSE;
		goto __TERMINAL;
	}
	/* Load the genif configure array. */
	cfg_genif_array = cJSON_GetObjectItem(cfg_root, "genif_config");
	if (NULL == cfg_genif_array)
	{
		error_code = CFGLOAD_ERROR_NOGENIFARRAY;
		goto __TERMINAL;
	}
	/* Load static route's configure array. */
	cfg_route_array = cJSON_GetObjectItem(cfg_root, "static_route");
	if (NULL == cfg_route_array)
	{
		error_code = CFGLOAD_ERROR_NOROUTEARRAY;
		goto __TERMINAL;
	}

	/* Load HT2 configure array if exist. */
	cfg_ht2vti_array = cJSON_GetObjectItem(cfg_root, "vti_ht2");
	if (NULL == cfg_ht2vti_array)
	{
		/* Common scenario, just log it. */
		__LOG("[%s]no ht2 vti configure found.\r\n", __func__);
	}

	/* Load and parse OK. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		/* Show out error information. */
		_hx_printf("[%s]error code[%d]\r\n", __func__, error_code);
		if (cfg_file)
		{
			/* Close the configure file. */
			CloseFile(cfg_file);
			cfg_file = NULL;
		}
		if (cfg_root)
		{
			cJSON_Delete(cfg_root);
			cfg_root = NULL;
		}
		cfg_genif_array = NULL;
		cfg_route_array = NULL;
	}
	if (file_buff)
	{
		_hx_free(file_buff);
	}
	return bResult;
}

/* Release the loaded configuration registry. */
void ReleaseNetworkConfig()
{
	if (cfg_file)
	{
		CloseFile(cfg_file);
		cfg_file = NULL;
	}
	if (cfg_root)
	{
		cJSON_Delete(cfg_root);
		cfg_root = NULL;
	}
	cfg_genif_array = NULL;
	cfg_route_array = NULL;
	cfg_ht2vti_array = NULL;
}

/* 
 * Local helper routine used to get the
 * configure object from genif's config array
 * given a genif's name.
 */
static cJSON* __get_genif_config(const char* genif_name)
{
	cJSON* cfg_genif = NULL;
	cJSON* cfg_item = NULL;
	int array_sz = 0;
	cJSON* name_item = NULL;
	char* name = NULL;

	/* Get the config object from array. */
	BUG_ON(NULL == cfg_genif_array);
	array_sz = cJSON_GetArraySize(cfg_genif_array);
	if (0 == array_sz)
	{
		goto __TERMINAL;
	}
	for (int i = 0; i < array_sz; i++)
	{
		cfg_item = cJSON_GetArrayItem(cfg_genif_array, i);
		if (NULL == cfg_item)
		{
			goto __TERMINAL;
		}
		name_item = cJSON_GetObjectItem(cfg_item, "genif_name");
		if (NULL == name_item)
		{
			goto __TERMINAL;
		}
		name = cJSON_GetStringValue(name_item);
		if (NULL == name)
		{
			goto __TERMINAL;
		}
		if (strcmp(genif_name, name) == 0)
		{
			cfg_genif = cfg_item;
			goto __TERMINAL;
		}
	}

__TERMINAL:
	return cfg_genif;
}

/*
 * Set genif's attributes according configure object.
 */
static BOOL __set_genif_attr(cJSON* cfg_object, __GENERIC_NETIF* pGenif)
{
	BOOL bResult = FALSE;
	cJSON* config_item = NULL;
	char* config_value = NULL;

	/* Parse pre-defined config one by one. */
	config_item = cJSON_GetObjectItem(cfg_object, "nat_flag");
	if (config_item)
	{
		config_value = cJSON_GetStringValue(config_item);
		if (config_value)
		{
			if (0 == strcmp(config_value, "true"))
			{
				pGenif->genif_flags |= GENIF_FLAG_NAT;
				bResult = TRUE;
			}
		}
	}

	return bResult;
}

/*
 * Get a specified genif's configure information
 * from in memory config database.
 */
BOOL GetGenifConfig(__GENERIC_NETIF* pGenif)
{
	BOOL bResult = FALSE;
	cJSON* genif_config = NULL;

	/* Check if configure loaded into memory. */
	if ((NULL == cfg_file) || (NULL == cfg_root) || (NULL == cfg_genif_array))
	{
		goto __TERMINAL;
	}

	/* Get the configure object from in memory database. */
	genif_config = __get_genif_config(pGenif->genif_name);
	if (NULL == genif_config)
	{
		goto __TERMINAL;
	}

	/* Initializes genif's attributes using config object. */
	bResult = __set_genif_attr(genif_config, pGenif);

__TERMINAL:
	return bResult;
}

/* Helper to install static route. */
static BOOL __parse_install_route(cJSON* route_item)
{
	ip_addr_t dest, mask;
	unsigned int metric = ROUTE_METRIC_STATIC;
	char* genif_name = NULL;
	cJSON* attr_item = NULL;
	char* item_value = NULL;
	BOOL bResult = FALSE;

#define LOCAL_GET_ITEM_VALUE(item_string) \
	attr_item = cJSON_GetObjectItem(route_item, item_string); \
	if (NULL == attr_item) \
	{ \
		goto __TERMINAL; \
	} \
	item_value = cJSON_GetStringValue(attr_item); \
	if (NULL == item_value) \
	{ \
		goto __TERMINAL; \
	}

	LOCAL_GET_ITEM_VALUE("dest_cidr");
	inet_aton(item_value, &dest);
	LOCAL_GET_ITEM_VALUE("dest_mask");
	inet_aton(item_value, &mask);
	LOCAL_GET_ITEM_VALUE("metric");
	metric = atoi(item_value);
	LOCAL_GET_ITEM_VALUE("out-genif");
	genif_name = item_value;

	/* No add the route to routing table. */
	bResult = RoutingManager.add_iproute(&dest, &mask,
		metric, NULL, IP_ROUTE_TYPE_STATIC, genif_name);

__TERMINAL:
	return bResult;
}

/*
 * Install manually configured static route from
 * in memory configure database.
 * It travels all static routes entry in memory
 * and add it through routing manager if the
 * route's out going interface is the specified
 * genif.
 */
BOOL InstallGenifRoute(const char* genif_name)
{
	BOOL bResult = FALSE;
	int array_sz = 0;
	cJSON* route_item = NULL;
	cJSON* attr_item = NULL;
	char* route_genif = NULL;

	if (NULL == cfg_route_array)
	{
		/* May no config file exist. */
		__LOG("[%s]no static route array found.\r\n", __func__);
		goto __TERMINAL;
	}
	array_sz = cJSON_GetArraySize(cfg_route_array);
	if (0 == array_sz)
	{
		__LOG("[%s]no static route configured.\r\n", __func__);
		goto __TERMINAL;
	}
	for (int i = 0; i < array_sz; i++)
	{
		route_item = cJSON_GetArrayItem(cfg_route_array, i);
		if (NULL == route_item)
		{
			__LOG("[%s]can not get route item at[%d]\r\n", __func__, i);
			goto __TERMINAL;
		}

		/* Check out going genif. */
		attr_item = cJSON_GetObjectItem(route_item, "out-genif");
		if (NULL == attr_item)
		{
			/* Object format invalid. */
			goto __TERMINAL;
		}
		route_genif = cJSON_GetStringValue(attr_item);
		if (strcmp(route_genif, genif_name))
		{
			/* 
			 * The route's out going genif does not 
			 * match the specified one, check next. 
			 */
			continue;
		}

		/* Now parse the route item and install it. */
		if (!__parse_install_route(route_item))
		{
			__LOG("[%s]install route fail.\r\n", __func__);
			goto __TERMINAL;
		}
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Release error code for this file. */
#undef CFGLOAD_ERROR_OUTOFMEMORY
#undef CFGLOAD_ERROR_OPENFILE
#undef CFGLOAD_ERROR_READFILE
#undef CFGLOAD_ERROR_FILEEMPTY
#undef CFGLOAD_ERROR_FILEPARSE
#undef CFGLOAD_ERROR_NOGENIFARRAY
#undef CFGLOAD_ERROR_GETJSONITEM
#undef CFGLOAD_ERROR_NOROUTEARRAY
