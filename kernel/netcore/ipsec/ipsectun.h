//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 18, 2022
//    Module Name               : ipsectun.h
//    Module Funciton           : 
//    Description               : 
//                                IPSec VTI's definition and routines.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef __IPSECTUN_H__
#define __IPSECTUN_H__

#include "lwip/ip_addr.h"

/* Add one IPSec VTI into system. */
BOOL ipsectun_add(const char* if_name,
	const char* parent_name,
	const char* address);

/* Change IPSec VTI's attributes. */
BOOL ipsectun_set(const char* if_name,
	const char* att_name,
	const char* att_value);

#endif //__IPSECTUN_H__
