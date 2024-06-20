//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 29, 2024
//    Module Name               : pteb.h
//    Module Funciton           : 
//                                PEB(Process Environment Block and TEB(
//                                Thread Environment Block)'s definitions
//                                and operation routines.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __PTEB_H__
#define __PTEB_H__

#include "hellox.h"
#include "../include/mlayout.h"

/* Force to run without all modules loaded. */
#define LOADER_FORCE_RUN 0

/* Maximal module name's length. */
#define MAX_MODULE_NAME_LEN 64

/* Maximal file path's length. */
#define MAX_FILE_PATH_LEN 128

/* Maximal import symbol name's length. */
#define MAX_IMPORT_NAME_LEN 64

/* Maximal module number one process can has. */
#define MAX_MODULE_NUM 256

/* Import symbol's maximal number. */
#define MAX_IMPORT_SYMBOL_NUM 4096

/* 
 * Dynamic load modules(or DLL)'s descriptor. 
 * Each DLL loaded into process's memory space
 * will has one descriptor to manage it.
 */
typedef struct tag__MODULE_DESCRIPTOR {
	char module_name[MAX_MODULE_NAME_LEN];
	unsigned long module_flags; 
	int module_index;       /* Index of the module in this process. */
	int ref_count;          /* Reference counter. */
	HANDLE hFile;           /* Handle of the module file. */
	DWORD entry_point;      /* Entry point. */
	DWORD loaded_addr;      /* Virtuall address. */
	DWORD image_header;     /* Image header, the begining of image. */
	DWORD nt_header;        /* NT header. */

	/* Import directory info. */
	DWORD import_dir_addr;
	DWORD import_dir_len;

	/* 
	 * The module with symbols imported from this module, 
	 * that is, the module depends
	 * on this module. -1 means no other module depend
	 * on this one.
	 */
	int dep_this_index;
	/* Corresponding import descriptor. */
	LPVOID dep_import_desc;
	/* Base address of the module depend on this one. */
	DWORD dep_base_addr;

	/* 
	 * Export symbol information, the addresses 
	 * are adjusted to actually virtual address,
	 * by added RVA and load address.
	 */
	LPVOID export_dir_addr;
	DWORD ordinal_base;
	DWORD number_of_function;
	DWORD number_of_name;
	DWORD addr_of_function;
	DWORD addr_of_name;
	DWORD addr_of_ordinaltable;
}__MODULE_DESCRIPTOR;

/* Flags of a module. */
#define MODULE_FLAG_DLL      (1 << 0)    /* DLL module. */
#define MODULE_FLAG_PE32     (1 << 1)    /* PE32. */
#define MODULE_FLAG_PE32PLUS (1 << 2)    /* PE32+. */
#define MODULE_FLAG_LOADED   (1 << 3)    /* Module loaded into memory. */

/*
 * Process Environment Block, each process has
 * one of this block and is stored in KMEM_PEB_START
 * address, could be accessed in user mode.
 */
typedef struct tag__PEB {
	HANDLE proc_handle;
	unsigned long process_flags;
	char current_dir[MAX_FILE_PATH_LEN];
	char app_name[MAX_MODULE_NAME_LEN];

	/* Modules queue. */
	int mq_head;
	int mq_tail;
	__MODULE_DESCRIPTOR mq[MAX_MODULE_NUM];
}__PEB;

/* Process flags. */
#define PROCESS_FLAGS_BIT64     (1 << 0)

/* Routines to manipulate the PEB. */
__PEB* CreatePEB(int argc, char* argv[]);

#endif //__PTEB_H__
