//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 15 Jun, 2024
//    Module Name               : fs_del.c
//    Module Funciton           : 
//    Description               : del command's implementation code.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#include <StdAfx.h>
#include <kapi.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wildcard.h>

/* For file name string operation. */
#include <../fs/fsstr.h>

#include "shell.h"
#include "fs.h"

/* Check if a name string contains wildcard. */
static BOOL HasWildCard(const char* name_string)
{
	const char* begin = name_string;
	while (*begin)
	{
		if ('*' == *begin)
		{
			return TRUE;
		}
		if ('?' == *begin)
		{
			return TRUE;
		}
		if ('[' == *begin)
		{
			return TRUE;
		}
		if (']' == *begin)
		{
			return TRUE;
		}
		begin++;
	}
	return FALSE;
}

/* 
 * Local helper to delete one file 
 * from current directory.
 */
static BOOL __delete_one_file(const char* dest_file_name)
{
	char full_file_name[MAX_FILE_NAME_LEN];
	BOOL bResult = FALSE;

	/* Current and parent dir can not be removed. */
	if ((0 == strncmp(dest_file_name, ".", 4)) ||
		(0 == strncmp(dest_file_name, "..", 4)))
	{
		return FALSE;
	}

	/* Dest file under current dir. */
	strncpy(full_file_name, FsGlobalData.CurrentDir, MAX_FILE_NAME_LEN);
	strcat(full_file_name, dest_file_name);
	ToCapital(full_file_name);
	/* Delete it. */
	bResult = IOManager.DeleteFile((__COMMON_OBJECT*)&IOManager,
		full_file_name);
	if (!bResult)
	{
		_hx_printf("Del file[%s] error[0xx%X].\r\n", full_file_name,
			GetLastError());
		return FALSE;
	}
	_hx_printf("Del file[%s] OK.\r\n", full_file_name);
	return bResult;
}

/*
 * Handler of del command, under fs view.
 * Usage:
 *   del dest_file
 * The dest_file specified the file to remove.
 * Wildcard can be specified, then any file(s)
 * will be deleted if matches wildcard string.
 */
DWORD __handler_fs_del(__CMD_PARA_OBJ* pCmdObj)
{
	BOOL bResult = FALSE;
	char* file_name = NULL;
	const char* dest_fn = NULL;
	int wc_matches = 0;
	FS_FIND_DATA ffd;
	HANDLE hFindHandle = NULL;
	BOOL bFindNext = FALSE;
	int total_del = 0;

	if (pCmdObj->byParameterNum < 2)
	{
		_hx_printf("  No file specified.\r\n");
		goto __TERMINAL;
	}

	file_name = pCmdObj->Parameter[1];
	ToCapital(file_name);
	if (HasWildCard(file_name))
	{
		/* Contains wildcard, iterate the whole dir. */
		dest_fn = (const char*)ffd.cFileName;
		ffd.bGetLongName = TRUE;
		hFindHandle = FindFirstFile(FsGlobalData.CurrentDir, &ffd);
		if (NULL == hFindHandle)
		{
			_hx_printf("Open current dir error[0x%X].\r\n",
				GetLastError());
			goto __TERMINAL;
		}
		do {
			wc_matches = wc_match(file_name, dest_fn);
			if (wc_matches)
			{
				if (ffd.dwFileAttribute & FILE_ATTR_DIRECTORY)
				{
					/* Directory, just skip. */
				}
				else {
					if (!__delete_one_file(dest_fn))
					{
						break;
					}
					total_del++;
				}
			}
			bFindNext = FindNextFile(FsGlobalData.CurrentDir, hFindHandle, &ffd);
		} while (bFindNext);
		_hx_printf("Total [%d] file(s) deleted.\r\n",
			total_del);
	}
	else {
		/* No wild card. */
		__delete_one_file(file_name);
	}

__TERMINAL:
	if (hFindHandle)
	{
		FindClose(FsGlobalData.CurrentDir, hFindHandle);
	}
	return SHELL_CMD_PARSER_SUCCESS;;
}
