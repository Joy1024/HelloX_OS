//***********************************************************************/
//    Author                    : Garry
//    Original Date             : July 10, 2022
//    Module Name               : ipseccmd.h
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

#ifndef __IPSECCMD_H__
#define __IPSECCMD_H__

#include "SHELL.H"

/* Maximal key length, 16 bytes. */
#define IPSEC_KEY_MAX_LENGTH 32

/* Main entry of ipsec command. */
unsigned long ipsec_entry(__CMD_PARA_OBJ* lpCmdObj);

#endif //__IPSECCMD_H__
