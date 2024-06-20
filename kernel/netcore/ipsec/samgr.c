//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 4, 2022
//    Module Name               : samgr.c
//    Module Funciton           : 
//                                IPSec SA management code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <stdio.h>

#include "samgr.h"

/* static encryption key. */
static char __local_enc_key[] = {
	0x01, 0x23, 0x45, 0x67,
	0x01, 0x23, 0x45, 0x67,
	0x01, 0x23, 0x45, 0x67,
	0x01, 0x23, 0x45, 0x67,
	0x01, 0x23, 0x45, 0x67,
	0x01, 0x23, 0x45, 0x67
};

/* Local helper used to generate a encryption key for SA. */
static void __generate_encryption_key(sad_entry* sa_entry)
{
	BUG_ON(NULL == sa_entry);
	for (int i = 0; i < IPSEC_MAX_ENCKEY_LEN; i++)
	{
		//sa_entry->enckey[i] = (__u8)rand();
		sa_entry->enckey[i] = __local_enc_key[i];
	}
}

/* Local helper used to generate a authentication key for SA. */
static void __generate_authentication_key(sad_entry* sa_entry)
{
	BUG_ON(NULL == sa_entry);
	for (int i = 0; i < IPSEC_MAX_AUTHKEY_LEN; i++)
	{
		sa_entry->authkey[i] = (__u8)rand();
	}
}

/* Initializer of the SA Manager. */
static BOOL __Initializer(__SA_MANAGER* pManager)
{
	BUG_ON(NULL == pManager);
#if defined(__CFG_SYS_SMP)
	__INIT_SPIN_LOCK(pManager->spin_lock, "sa_mgr");
#endif
	return TRUE;
}

/* 
 * Add one SA into system. 
 * The SA profile contains key information of SA
 * to be created.
 */
static BOOL __RegisterSA(__SA_PROFILE* profile)
{
	BOOL bResult = FALSE;
	sad_entry* sa_entry = NULL;

	BUG_ON(NULL == profile);
	/* Allocate a new SA first. */
	sa_entry = _hx_malloc(sizeof(sad_entry));
	if (NULL == sa_entry)
	{
		__LOG("[%s]out of memory\r\n", __func__);
		goto __TERMINAL;
	}
	memset(sa_entry, 0, sizeof(sad_entry));
	
	/* Copy key information element one by one. */
	sa_entry->spi = profile->sa_spi;
	sa_entry->enc_alg = profile->enc_algorithm;
	memcpy(sa_entry->enckey, profile->enc_key, sizeof(sa_entry->enckey));
	sa_entry->mode = profile->mode;
	sa_entry->protocol = profile->protocol;
	sa_entry->path_mtu = profile->path_mtu;
	sa_entry->sequence_number = profile->seq_number;
	sa_entry->replay_win = profile->replay_win;
	sa_entry->lifetime = profile->lifetime;
	
	/* Copy or generate the encryption key. */
	if (profile->enckey_specified)
	{
		memcpy(&sa_entry->enckey[0], &profile->enc_key[0], IPSEC_MAX_ENCKEY_LEN);
	}
	else {
		/* General a encryption key. */
		__generate_encryption_key(sa_entry);
	}

	/* Copy or generate the authentication key. */
	if (profile->authkey_specified)
	{
		memcpy(&sa_entry->authkey[0], &profile->auth_key[0], IPSEC_MAX_AUTHKEY_LEN);
	}
	else {
		/* Generate a authentication key. */
		__generate_authentication_key(sa_entry);
	}
	sa_entry->next = NULL;

	/* Link the SA into system list. */
	__ACQUIRE_SPIN_LOCK(SAManager.spin_lock);
	__ATOMIC_INCREASE(&sa_entry->ref_counter);
	SAManager.sa_number++;
	sa_entry->index = SAManager.sa_number;
	if (NULL == SAManager.sa_list)
	{
		/* The first one. */
		SAManager.sa_list = sa_entry;
	}
	else {
		/* Put to head. */
		sa_entry->next = SAManager.sa_list;
		SAManager.sa_list = sa_entry;
	}
	__RELEASE_SPIN_LOCK(SAManager.spin_lock);
	
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Show all SA in system. */
static void __ShowAllSA()
{
	sad_entry* sa_entry = NULL;

	/* Print a table header. */
	printf("index\tspi\tproto\tmode\th-alg\trefcont\t\r\n");
	printf("------------------------------------------------------\r\n");
	/* Print out one by one. */
	__ACQUIRE_SPIN_LOCK(SAManager.spin_lock);
	sa_entry = SAManager.sa_list;
	while (sa_entry)
	{
		printf("%d\t", sa_entry->index);
		printf("0x%X\t", sa_entry->spi);
		printf("%d\t", sa_entry->protocol);
		printf("%d\t", sa_entry->mode);
		printf("%d\t", sa_entry->auth_alg);
		printf("%d\t", sa_entry->ref_counter);
		printf("\r\n");
		sa_entry = sa_entry->next;
	}
	__RELEASE_SPIN_LOCK(SAManager.spin_lock);
}

/* Show out one SA by given it's SPI. */
static void __ShowSA(__u32 spi)
{
	sad_entry* sa_curr = NULL;

	__ACQUIRE_SPIN_LOCK(SAManager.spin_lock);
	sa_curr = SAManager.sa_list;
	while (sa_curr)
	{
		if (spi == sa_curr->spi)
		{
			break;
		}
		sa_curr = sa_curr->next;
	}
	if (sa_curr)
	{
		printf("  SA information:\r\n");
		printf("    spi:         0x%X\r\n", sa_curr->spi);
		printf("    dest:        %s\r\n", ipsec_inet_ntoa(sa_curr->dest));
		printf("    mask:        %s\r\n", ipsec_inet_ntoa(sa_curr->dest_netaddr));
		printf("    path-mtu:    %d\r\n", sa_curr->path_mtu);
		printf("    seq_no:      %d\r\n", sa_curr->sequence_number);
		printf("    replay-win:  %d\r\n", sa_curr->replay_win);
		printf("    lifetime:    %d\r\n", sa_curr->lifetime);
		
		/* Show authentication key. */
		printf("    aukey:       ");
		for (int i = 0; i < IPSEC_MAX_AUTHKEY_LEN; i++)
		{
			printf("%02X", sa_curr->authkey[i]);
		}
		printf("\r\n");

		/* Show encryption key. */
		printf("    enckey:      ");
		for (int i = 0; i < IPSEC_MAX_ENCKEY_LEN; i++)
		{
			printf("%02X", sa_curr->enckey[i]);
		}
		printf("\r\n");
	}
	__RELEASE_SPIN_LOCK(SAManager.spin_lock);
}

/* 
 * Release one SA from system. 
 * Locate the SA with the specified spi, decrease
 * the reference counter, release it if reach
 * 0, and TRUE will be returned. Otherwise just return FALSE.
 */
static BOOL  __ReleaseSA(__u32 spi)
{
	BOOL bShouldRelease = FALSE;
	sad_entry* sa_prev = NULL;
	sad_entry* sa_curr = NULL;

	BUG_ON(0 == spi);
	__ACQUIRE_SPIN_LOCK(SAManager.spin_lock);
	sa_prev = sa_curr = SAManager.sa_list;
	while (sa_curr)
	{
		if (spi == sa_curr->spi)
		{
			__ATOMIC_DECREASE(&sa_curr->ref_counter);
			if (0 == sa_curr->ref_counter)
			{
				/* Should destroy this SA. */
				if (sa_curr == sa_prev)
				{
					SAManager.sa_list = sa_curr->next;
				}
				else {
					sa_prev->next = sa_curr->next;
				}
				bShouldRelease = TRUE;
			}
			__RELEASE_SPIN_LOCK(SAManager.spin_lock);
			goto __TERMINAL;
		}
		/* Check next one. */
		sa_prev = sa_curr;
		sa_curr = sa_curr->next;
	}
	__RELEASE_SPIN_LOCK(SAManager.spin_lock);

__TERMINAL:
	if (bShouldRelease)
	{
		_hx_free(sa_curr);
	}
	return bShouldRelease;
}

/* 
 * Get the SA by it's SPI value. 
 * The SA will be returned after reference counter
 * increased by SA Manager, otherwise NULL
 * will be returned.
 * The caller must release the SA after using it
 * by invoking ReleaseSA routine.
 */
static sad_entry* __GetSA(__u32 spi)
{
	sad_entry* sa_curr = NULL;

	BUG_ON(0 == spi);
	__ACQUIRE_SPIN_LOCK(SAManager.spin_lock);
	sa_curr = SAManager.sa_list;
	while (sa_curr)
	{
		if (spi == sa_curr->spi)
		{
			__ATOMIC_INCREASE(&sa_curr->ref_counter);
			__RELEASE_SPIN_LOCK(SAManager.spin_lock);
			return sa_curr;
		}
		sa_curr = sa_curr->next;
	}
	__RELEASE_SPIN_LOCK(SAManager.spin_lock);
	return sa_curr;
}

/* The global SA manager object. */
__SA_MANAGER SAManager = {
#if defined(__CFG_SYS_SMP)
	SPIN_LOCK_INIT_VALUE,
#endif
	NULL,   /* SA list header. */
	0,      /* SA counter. */

	__Initializer,   /* Initialize routine. */
	__RegisterSA,    /* RegisterSA routine. */
	__ShowAllSA,     /* ShowAllSA routine. */
	__ShowSA,        /* ShowSA routine. */
	__ReleaseSA,     /* RemoveSA routine. */
	__GetSA,         /* GetSA routine. */
};
