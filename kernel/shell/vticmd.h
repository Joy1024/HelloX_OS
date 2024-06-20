//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 18, 2022
//    Module Name               : vticmd.h
//    Module Funciton           : 
//    Description               : 
//                                Shell command for VTI(Virtual Tunnel Interface)
//                                management, such as add/del/modify VTI.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef __VTICMD_H__
#define __VTICMD_H__

#include "SHELL.H"

/* Main entry of vti command. */
unsigned long vti_entry(__CMD_PARA_OBJ* lpCmdObj);

#endif //__VTICMD_H__
