//***********************************************************************/
//    Author                    : Garry
//    Original Date             : July 10, 2022
//    Module Name               : ipseccmd.c
//    Module Funciton           : 
//    Description               : 
//                                Shell command for IPSec function, such as IPSec
//                                diagnostic command and operations.
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
#include <ctype.h>

#include "ipseccmd.h"
#include "ipsec/test/test.h"
#include "ipsec/samgr.h"
#include "ipsec/ipsec.h"
#include "ipsec/samgr.h"
#include "ipsec/aes.h"
#include "hhtt/hhtt.h"

/* Global object to hold testing result. */
static test_result global_result = { 0 };

/* Handlers used to process sub command of IPSec. */
static unsigned long __sa_add(const char* argv[], int argc);
static unsigned long __sa_del(const char* argv[], int argc);
static unsigned long __sa_list(const char* argv[], int argc);
static unsigned long __sa_set(const char* argv[], int argc);
static unsigned long __uni_test(const char* argv[], int argc);
static unsigned long __vti_add(const char* argv[], int argc);
static unsigned long __vti_del(const char* argv[], int argc);
static unsigned long __vti_set(const char* argv[], int argc);
static unsigned long __vti_list(const char* argv[], int argc);
static unsigned long __key_gen(const char* argv[], int argc);

/*
 * The map between ipsec command and it's
 * hander, it's description string.
 * The ipsec command just search this map
 * using the subcommand string, to locate the handler
 * of sub command.
 */
static struct __ipsec_subcmd_struct {
	const char* subcmd_name; 
	unsigned long (*CommandHandler)(const char*[], int);
	const char* subcmd_description;
}ipsec_subcmd_map[] = {
	{ "saadd",     __sa_add,        "  saadd   : Add a SA manually."},
	{ "sadel",     __sa_del,        "  sadel   : Del a SA manually."},
	{ "saset",     __sa_set,        "  saset   : Set SA attributes."},
	{ "salist",    __sa_list,       "  salist  : List all or one SA in system."},
	{ "vtiadd",    __vti_add,       "  vtiadd  : Add one IPSec VTI."},
	{ "vtidel",    __vti_del,       "  vtidel  : Del one IPSec VTI."},
	{ "vtiset",    __vti_set,       "  vtiset  : Set IPSec VTI's attribute."},
	{ "vtilist",   __vti_list,      "  vtilist : List all or one IPSec VTI in system."},
	{ "unitest",   __uni_test,      "  unitest : Invoke ESP testing routines."},
	{ "keygen",    __key_gen,       "  keygen  : Generate a key."},
	{ NULL,        NULL,            NULL},
};

/* 
 * Local helper to convert a user specified string to 
 * enc/decryption or authentication key. 
 */
static BOOL __string_to_key(const char* init_key_string, char key_array[], unsigned int key_length)
{
	/* User specified key string, save it. */
	int key_str_len = 0;
	char key_number = 0;
	int i = 0;
	char key_string[IPSEC_MAX_KEY_LEN * 2 + 1];
	BOOL bResult = FALSE;

	BUG_ON((NULL == init_key_string) || (NULL == key_array));
	BUG_ON(key_length > IPSEC_MAX_KEY_LEN);

	key_str_len = strlen(init_key_string);
	if (key_str_len > IPSEC_MAX_KEY_LEN * 2)
	{
		printf("[%s]key string is too long.\r\n", __func__);
		goto __TERMINAL;
	}
	strcpy(key_string, init_key_string);

	if (key_str_len % 2)
	{
		printf("[%s]key string must be odd characters.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Convert all lower char to upper. */
	for (i = 0; i < key_str_len; i++)
	{
		if (isalpha(key_string[i]))
		{
			if (islower(key_string[i]))
			{
				key_string[i] = toupper(key_string[i]);
			}
		}
	}

	for (i = 0; i < key_str_len / 2; i++)
	{
		if (isdigit(key_string[i * 2]))
		{
			key_number = key_string[i * 2] - '0';
		}
		else {
			key_number = key_string[i * 2] - 'A' + 0x0A;
		}
		key_number <<= 4;

		if (isdigit(key_string[i * 2 + 1]))
		{
			key_number += key_string[i * 2 + 1] - '0';
		}
		else {
			key_number += key_string[i * 2 + 1] - 'A' + 0x0A;
		}
		key_array[i] = key_number;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Add one SA entry to SAD. */
static unsigned long __sa_add(const char* argv[], int argc)
{
	__SA_PROFILE sa_profile = { 0 };

	if (argc <= 4)
	{
		printf("saadd: spi mode[tunnel/trans] proto[ah/esp] enc_alg[3des/des] key(optional)");
		goto __TERMINAL;
	}
	
	/* Convert to SA profile object. */
	/* spi */
	sa_profile.sa_spi = atol(argv[1]);
	if (0 == sa_profile.sa_spi)
	{
		printf("[%s]invalid SPI specified.\r\n", __func__);
		goto __TERMINAL;
	}

	/* mode */
	if (0 == strcmp(argv[2], "tunnel"))
	{
		sa_profile.mode = IPSEC_TUNNEL;
	}
	else {
		sa_profile.mode = IPSEC_TRANSPORT;
	}

	/* protocol */
	if (0 == strcmp(argv[3], "esp"))
	{
		sa_profile.protocol = IPSEC_PROTO_ESP;
	}
	else {
		if (0 == strcmp(argv[3], "ah"))
		{
			sa_profile.protocol = IPSEC_PROTO_AH;
		}
		else {
			printf("[%s]invalid protocol specified.\r\n", __func__);
			goto __TERMINAL;
		}
	}

	/* algorithm */
	if (0 == strcmp(argv[4], "3des"))
	{
		sa_profile.enc_algorithm = IPSEC_3DES;
	}
	else
	{
		/* Not supported yet. */
		printf("[%s]invalid algorithm specified.\r\n", __func__);
		goto __TERMINAL;
	}

	/* key. */
	if (argc == 6)
	{
		if (__string_to_key(argv[5], &sa_profile.enc_key[0], IPSEC_MAX_ENCKEY_LEN))
		{
			sa_profile.enckey_specified = TRUE;
		}
		else {
			/* Invalid key string specified. */
			goto __TERMINAL;
		}
	}

	/* Use default value to fill others. */
	sa_profile.path_mtu = IPSEC_DEFAULT_PATH_MTU;
	sa_profile.seq_number = 0;
	sa_profile.replay_win = 0;
	sa_profile.lifetime = 0;

	/* Register the user specified SA into system. */
	if (!SAManager.RegisterSA(&sa_profile))
	{
		printf("[%s]failed to register SA.\r\n", __func__);
	}

__TERMINAL:
	return 0;
}

/* Del one SA from SAD. */
static unsigned long __sa_del(const char* argv[], int argc)
{
	__u32 spi = 0;

	if (argc < 2)
	{
		printf("[%s]no SPI specified.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Get SPI of the SA to be deleted. */
	spi = atol(argv[1]);
	if (0 == spi)
	{
		printf("[%s]invalid SPI specified.\r\n");
		goto __TERMINAL;
	}

	/* Delete the SA. */
	if (SAManager.ReleaseSA(spi))
	{
		printf("SA delete success.\r\n");
	}

__TERMINAL:
	return 0;
}

/* Usage info for 'saset' command. */
static void __saset_usage()
{
	printf("  Usage:\r\n");
	printf("  saset [spi] dest [dest ip addr]\r\n");
	printf("  saset [spi] mask [dest ip mask]\r\n");
	printf("  saset [spi] authalg [md5/sha1]\r\n");
	printf("  saset [spi] enckey [encryption key]\r\n");
	printf("  saset [spi] authkey [auth key]\r\n");
}

/* Change attributes of one SA. */
static unsigned long __sa_set(const char* argv[], int argc)
{
	sad_entry* sa_entry = NULL;
	struct ipsec_in_addr addr;
	char auth_key[IPSEC_MAX_AUTHKEY_LEN];
	char enc_key[IPSEC_MAX_ENCKEY_LEN];

	if (argc < 4)
	{
		__saset_usage();
		goto end;
	}

	/* Get the user specified SA entry. */
	sa_entry = SAManager.GetSA(atol(argv[1]));
	if (NULL == sa_entry)
	{
		printf("[%s]no SA find.\r\n", __func__);
		goto end;
	}

	/* Change SA attributes accordingly. */
	if (0 == strcmp("dest", argv[2]))
	{
		/* Set SA's destination address. */
		if (!ipsec_inet_aton(argv[3], &addr))
		{
			printf("[%s]invalid IP address.\r\n", __func__);
			goto end;
		}
		sa_entry->dest = addr.s_addr;
	}
	if (0 == strcmp("mask", argv[2]))
	{
		/* Set SA's destination mask. */
		if (!ipsec_inet_aton(argv[3], &addr))
		{
			printf("[%s]invalid mask specified.\r\n", __func__);
			goto end;
		}
		sa_entry->dest_netaddr = addr.s_addr;
	}
	if (0 == strcmp("authkey", argv[2]))
	{
		memset(&auth_key[0], 0, IPSEC_MAX_AUTHKEY_LEN);
		if (!__string_to_key(argv[3], auth_key, IPSEC_MAX_AUTHKEY_LEN))
		{
			printf("[%s]invalid auth key specified.\r\n", __func__);
			goto end;
		}
		/* Clear the old key and set the new one. */
		memset(&sa_entry->authkey[0], 0, IPSEC_MAX_AUTHKEY_LEN);
		memcpy(&sa_entry->authkey[0], &auth_key[0], IPSEC_MAX_AUTHKEY_LEN);
	}
	if (0 == strcmp("enckey", argv[2]))
	{
		memset(&enc_key[0], 0, IPSEC_MAX_ENCKEY_LEN);
		if (!__string_to_key(argv[3], enc_key, IPSEC_MAX_ENCKEY_LEN))
		{
			printf("[%s]invalid enc key specified.\r\n", __func__);
			goto end;
		}
		/* Clear the old key and set the new one. */
		memset(&sa_entry->enckey[0], 0, IPSEC_MAX_ENCKEY_LEN);
		memcpy(&sa_entry->enckey[0], &enc_key[0], IPSEC_MAX_ENCKEY_LEN);
	}
	if (0 == strcmp("authalg", argv[2]))
	{
		if (0 == strcmp("md5", argv[3]))
		{
			sa_entry->auth_alg = IPSEC_HMAC_MD5;
			goto end;
		}
		if (0 == strcmp("sha1", argv[3]))
		{
			sa_entry->auth_alg = IPSEC_HMAC_SHA1;
			goto end;
		}
		printf("[%s]invalid hash algorithm.\r\n", __func__);
	}

end:
	if (sa_entry)
	{
		/* Should release it. */
		SAManager.ReleaseSA(sa_entry->spi);
	}
	return 0;
}

/* List all SAs in SAD. */
static unsigned long __sa_list(const char* argv[], int argc)
{
	__u32 spi = 0;

	if (argc < 2)
	{
		/* No SPI specified, show all. */
		SAManager.ShowAllSA();
	}
	else {
		/* Show a specified SA. */
		spi = atol(argv[1]);
		if (0 == spi)
		{
			printf("[%s]invalid SPI value.\r\n", __func__);
			return 0;
		}
		SAManager.ShowSA(spi);
	}
	return 0;
}

/* Add one IPSec VTI interface into system. */
static unsigned long __vti_add(const char* argv[], int argc)
{
	return 0;
}

/* Del one IPSec VTI interface from system. */
static unsigned long __vti_del(const char* argv[], int argc)
{
	return 0;
}

/* Change attributes of one VTI interface. */
static unsigned long __vti_set(const char* argv[], int argc)
{
	return 0;
}

/* List all VTI interface(s) in system. */
static unsigned long __vti_list(const char* argv[], int argc)
{
	return 0;
}

/* ESP testing. */
static unsigned long __uni_test(const char* argv[], int argc)
{
	if (argc < 2)
	{
		printf("No testing function specified.\r\n");
		goto __TERMINAL;
	}
	if (0 == strcmp("md5", argv[1]))
	{
		/* Launch MD5 testing. */
		md5_test(&global_result);
	}
	if (0 == strcmp("des", argv[1]))
	{
		/* Launch DES testing. */
		des_test(&global_result);
	}
	if (0 == strcmp("sha", argv[1]))
	{
		/* Launch SHA-1 testing. */
		sha1_test(&global_result);
	}
	if (0 == strcmp("util", argv[1]))
	{
		/* Launch util testing. */
		util_debug_test(&global_result);
	}
	if (0 == strcmp("sa", argv[1]))
	{
		/* SA function testing. */
		sa_test(&global_result);
	}
	if (0 == strcmp("esp", argv[1]))
	{
		/* Launch ESP testing. */
		esp_test(&global_result);
	}
	if (0 == strcmp("ah", argv[1]))
	{
		/* Launch AH testing. */
		ah_test(&global_result);
	}
	if (0 == strcmp("aes", argv[1]))
	{
		/* Launch AES testing. */
		mbedtls_aes_self_test(1);
	}

__TERMINAL:
	return 0;
}

/* AH testing. */
static unsigned long __ah_test(const char* argv[], int argc)
{
	return 0;
}

/* Main entry routine of ipsec command. */
unsigned long ipsec_entry(__CMD_PARA_OBJ* lpCmdObj)
{
	int argc = 0;

	argc = lpCmdObj->byParameterNum;
	if (argc < 2)
	{
		/* Just show out all available commands under ipsec command. */
		for (int i = 0; ; i++)
		{
			if (NULL == ipsec_subcmd_map[i].subcmd_name)
			{
				break;
			}
			printf("%s\r\n", ipsec_subcmd_map[i].subcmd_description);
		}
		goto __TERMINAL;
	}

	/* Lookup subcommand map and invoke the handler. */
	for (int i = 0; ; i++)
	{
		if (NULL == ipsec_subcmd_map[i].subcmd_name)
		{
			/* No subcmd matched. */
			printf("  Invalid sub command.\r\n");
			break;
		}
		if (0 == strcmp(lpCmdObj->Parameter[1], ipsec_subcmd_map[i].subcmd_name))
		{
			ipsec_subcmd_map[i].CommandHandler(&lpCmdObj->Parameter[1], argc - 1);
			break;
		}
	}

__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;
}

/* Generate a ipsec key. */
static BOOL __generate_key(unsigned char* key, int key_length)
{
	int key_bytes = key_length / 8;
	unsigned long seed = System.dwClockTickCounter;

#if 0
	static unsigned char __key[] =
	{ 0x18, 0x96, 0xC9, 0x8E, 0x72, 0xCE, 0x56, 0xCF,
	  0x8E, 0xDE, 0x24, 0xD7, 0x84, 0xFF, 0x62, 0xCB,
	  0xE9, 0x42, 0x36, 0xCF, 0x92, 0xDC, 0x24, 0x90,
	  0x36, 0xA0, 0x8F, 0x0F, 0x99, 0x38, 0xFD, 0x82 };

	memcpy(key, __key, 32);
#endif

	srand(seed);
	for (int i = 0; i < key_bytes; i++)
	{
		key[i] = (unsigned char)rand();
	}
	return TRUE;
}

/*
 * Generate a encryption/decryption key and save
 * it into file.
 */
static unsigned long __key_gen(const char* argv[], int argc)
{
	int key_length = 0;
	unsigned char key[IPSEC_KEY_MAX_LENGTH];
	char key_file_name[MAX_FILE_NAME_LEN + 16];
	HANDLE hKeyFile = NULL;

	if (argc < 3)
	{
		/* Show out usage. */
		_hx_printf("  ipsec keygen [key-bits] [key-file-name]\r\n");
		return 0;
	}

	/* Get user specified key length. */
	if (0 == strcmp(argv[1], "256"))
	{
		key_length = 256;
	}
	if (0 == strcmp(argv[1], "192"))
	{
		key_length = 192;
	}
	if (0 == strcmp(argv[1], "128"))
	{
		key_length = 128;
	}

	if (0 == key_length)
	{
		/* User specified invalid key length. */
		_hx_printf("  Invalid key length[only 256/192/128]\r\n");
		return 0;
	}

	if ((strlen(argv[2]) < 3) || strlen(argv[2]) >= MAX_FILE_NAME_LEN)
	{
		_hx_printf("  Invalid key file name.\r\n");
		return 0;
	}
	if (':' == argv[2][1])
	{
		/* User specified key file's path. */
		strcpy(key_file_name, argv[2]);
	}
	else
	{
		/* No direct specified, use default one. */
		strcpy(key_file_name, IPSEC_KEY_FILEDIR);
		strcat(key_file_name, argv[2]);
	}

	/* Create the key file. */
	hKeyFile = CreateFile(key_file_name, FILE_OPEN_ALWAYS,
		0, NULL);
	if (NULL == hKeyFile)
	{
		_hx_printf("  Can not create key file[%s]\r\n", key_file_name);
		goto __TERMINAL;
	}

	/* Generate the key. */
	if (!__generate_key(key, key_length))
	{
		_hx_printf("  Generate key fail.\r\n");
		goto __TERMINAL;
	}

	/* Now write the key into file. */
	if (!WriteFile(hKeyFile, key_length / 8, key, NULL))
	{
		_hx_printf("  Write file fail.\r\n");
		goto __TERMINAL;
	}

__TERMINAL:
	if (hKeyFile)
	{
		/* Close it. */
		CloseFile(hKeyFile);
	}
	return 0;
}
