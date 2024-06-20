//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 18, 2022
//    Module Name               : gretun.h
//    Module Funciton           : 
//    Description               : 
//                                GRE VTI's definition and routines.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef __GRETUN_H__
#define __GRETUN_H__

#include "lwip/ip_addr.h"

/* Default GRE VTI's MTU. */
#define GRE_DEFAULT_MTU 1500

/* Header of GRE tunnel, 32 bits. */
#pragma pack(push, 1)
typedef struct tag__GRE_HEADER {
	/* Flag bits. */
	unsigned long _C : 1;
	unsigned long _R : 1;
	unsigned long _K : 1;
	unsigned long _S : 1;
	unsigned long _s : 1;
	/* Recursive control. */
	unsigned long _rc : 3;
	/* Reserved flags. */
	unsigned long _flags : 5;
	/* Version. */
	unsigned long _ver : 3;
	/* Protocol type. */
	unsigned long _proto : 16;
}__GRE_HEADER;
#pragma pack(pop)

/* Add one GRE VTI into system. */
BOOL gretun_add(const char* if_name,
	const char* parent_name,
	const char* address);

/* Change GRE VTI's attributes. */
BOOL gretun_set(const char* if_name,
	const char* att_name,
	const char* att_value);

#endif //__GRETUN_H__
