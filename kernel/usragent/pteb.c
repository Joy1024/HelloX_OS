//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 29, 2024
//    Module Name               : pteb.c
//    Module Funciton           : 
//                                PEB(Process Environment Block and TEB(
//                                Thread Environment Block)'s implementation
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "../include/mlayout.h"
#include "pteb.h"
#include "stdio.h"
#include "string.h"

/* Create and initializes PEB. */
__PEB* CreatePEB(int argc, char* argv[])
{
	__PEB* peb = NULL;
	__MODULE_DESCRIPTOR* current = NULL;
	BOOL bResult = FALSE;
	char full_name[MAX_FILE_PATH_LEN + MAX_MODULE_NAME_LEN + 1];
	char* name_begin = NULL;

	/* Make sure PEB's space is enough. */
	if (sizeof(__PEB) >= KMEM_PEB_LENGTH)
	{
		_hx_printf("PEB's memory space[%d] is too small.\r\n",
			sizeof(__PEB));
		goto __TERMINAL;
	}

	/* Reserve memory space for PEB. */
	peb = (__PEB*)VirtualAlloc((LPVOID)KMEM_PEB_START, KMEM_PEB_LENGTH,
		VIRTUAL_AREA_ALLOCATE_ALL,
		VIRTUAL_AREA_ACCESS_RW,
		"peb");
	if (peb != (void*)KMEM_PEB_START)
	{
		_hx_printf("Allocate PEB failed.\r\n");
		goto __TERMINAL;
	}
	memset(peb, 0, sizeof(__PEB));

	/* Initialize it. */
	peb->mq_head = peb->mq_tail = 0;
	peb->process_flags = 0;

	/* Current PE as the first module in queue. */
	current = &peb->mq[peb->mq_tail];
	peb->mq_tail++;

	/* 
	 * Init the first module using in stack arguments. 
	 * Members initialized to 0 or -1 will be set
	 * in process of actual module loading procedure.
	 */
	current->dep_this_index = -1;
	current->entry_point = -1;
	current->loaded_addr = -1;
	current->image_header = -1;
	current->module_flags = 0;
	current->ref_count = 0;
	current->module_index = 0;
	if (strlen(argv[1]) > (MAX_MODULE_NAME_LEN + MAX_FILE_PATH_LEN))
	{
		_hx_printf("Module name is too long.\r\n");
		goto __TERMINAL;
	}
	strncpy(full_name, argv[1], (MAX_FILE_PATH_LEN + MAX_MODULE_NAME_LEN));
	name_begin = strrchr(full_name, '\\');
	if (NULL == name_begin)
	{
		_hx_printf("No file path specified[%s].\r\n", full_name);
		goto __TERMINAL;
	}
	*name_begin++ = 0; /* Seperate name and path. */
	strncpy(current->module_name, name_begin, MAX_MODULE_NAME_LEN);

	/* Save current dir and app name. */
	strncpy(peb->current_dir, full_name, MAX_FILE_PATH_LEN);
	strncpy(peb->app_name, name_begin, MAX_MODULE_NAME_LEN);

	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (peb)
		{
			VirtualFree(peb);
			peb = NULL;
		}
	}
	return peb;
}
