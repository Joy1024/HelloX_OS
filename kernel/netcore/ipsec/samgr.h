//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 4, 2022
//    Module Name               : samgr.h
//    Module Funciton           : 
//                                IPSec SA management code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SAMGR_H__
#define __SAMGR_H__

#include "StdAfx.h"
#include "sa.h"

/* Default values of SA. */
#define IPSEC_DEFAULT_PATH_MTU 1450

/* 
 * SA profile, used to describe key members of SA. 
 * SA is created from the SA profile. 
 */
typedef struct tag__SA_PROFILE {
	__u32 sa_spi;
	__u8 protocol; /* ESP or AH. */
	__u8 mode; /* Transparent or tunnel. */
	__u32 seq_number;
	__u8 replay_win;
	__u32 lifetime;
	__u16 path_mtu; /* Maximal path MTU.*/
	__u8 enc_algorithm; /* Encryption algorithm, DES/3DES... */
	BOOL enckey_specified; /* If the key is specified. */
	__u8 enc_key[IPSEC_MAX_ENCKEY_LEN]; /* Encryption key, only when enckey_specified is TRUE. */
	BOOL authkey_specified; /* If the auth key is specified. */
	__u8 auth_key[IPSEC_MAX_AUTHKEY_LEN];
}__SA_PROFILE;

/* 
 * SA manager object.
 * This is the global object to manage all SAs in system,
 * it's the common coding schema of HelloX.
 */
typedef struct tag__SA_MANAGER {
#if defined(__CFG_SYS_SMP)
	__SPIN_LOCK spin_lock;
#endif
	sad_entry* sa_list; /* SA list header. */
	unsigned long sa_number; /* How many SA in system. */

	/* SA operation routines. */
	/* Initializer of this object. */
	BOOL (*Initialize)(struct tag__SA_MANAGER* pManager);
	/* Add one SA into system. */
	BOOL (*RegisterSA)(__SA_PROFILE* profile);
	/* Show all SA in system. */
	void (*ShowAllSA)(); 
	/* Show one SA. */
	void (*ShowSA)(__u32 spi);
	/* Delete one SA from system. */
	BOOL (*ReleaseSA)(__u32 spi);
	/* Get the SA by it's SPI. */
	sad_entry* (*GetSA)(__u32 spi);
}__SA_MANAGER;

/* The global SA manager object. */
extern __SA_MANAGER SAManager;

#endif //__SAMGR_H__
