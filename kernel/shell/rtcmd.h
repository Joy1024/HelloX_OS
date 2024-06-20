//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Nov 11, 2022
//    Module Name               : rtcmd.h
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

#ifndef __RTCMD_H__
#define __RTCMD_H__

#include "SHELL.H"

/* Main entry of iproute command. */
unsigned long iproute_entry(__CMD_PARA_OBJ* lpCmdObj);

#endif //__RTCMD_H__
