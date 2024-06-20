//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 30 Apri, 2024
//    Module Name               : fs_dir.c
//    Module Funciton           : 
//    Description               : dir command's implementation code.
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

#include "shell.h"
#include "fs.h"

/* 
 * A local helper routine to print a file or
 * sub dir's detail info, invoked by dir command.
 */
static int __print_file_detail(FS_FIND_DATA* pFindData, BOOL has_wc, const char* wc_str)
{
	char Buffer[MAX_FILE_NAME_LEN] = { 0 };
	char file_name[MAX_FILE_NAME_LEN];
	int matches = 0;
	unsigned long long file_sz = 0;

	/* Get file's size. */
	file_sz = pFindData->nFileSizeHigh;
	file_sz <<= sizeof(pFindData->nFileSizeHigh) * 8;
	file_sz += pFindData->nFileSizeLow;

	if (pFindData->bGetLongName)
	{
		_hx_sprintf(Buffer, "%s\t%s", strlen(pFindData->cFileName) ?
			pFindData->cFileName : pFindData->cAlternateFileName,
			pFindData->cAlternateFileName);
		strncpy(file_name, Buffer, MAX_FILE_NAME_LEN);
	}
	else
	{
		if (pFindData->dwFileAttribute & FILE_ATTR_DIRECTORY)
		{
			/* Directory. */
			_hx_sprintf(file_name, "[%s]", pFindData->cAlternateFileName);
			_hx_sprintf(Buffer, "%13s\t %8lld\t %4s",
				file_name,
				file_sz, "DIR");
		}
		else {
			/* Normal file. */
			_hx_sprintf(Buffer, "%13s\t %8lld\t %4s",
				pFindData->cAlternateFileName,
				file_sz, "FILE");
		}
		strncpy(file_name, pFindData->cAlternateFileName, MAX_FILE_NAME_LEN);
	}

	/* Match the wildcard string. */
	if (has_wc)
	{
		matches = wc_match(wc_str, (const char*)file_name);
	}
	else {
		matches = 1;
	}

	if (matches)
	{
		_hx_printf("%s\r\n", Buffer);
	}
	return matches;
}

/*
 * Handler of dir command.
 * Usage:
 *   dir [wildcard_str] [options]
 * @wildcard_str is a string contains wildcard
 * to filter files and directories. Current dir
 * is used to show, for example:
 *   dir *.dll
 * All suffix with dll will be displayed.
 * @options is some options to tell dir how
 * to show, only '-l' is available now, to
 * show out long name of all files.
 */
DWORD __handler_fs_dir(__CMD_PARA_OBJ* pCmdObj)
{
	FS_FIND_DATA ffd = { 0 };
	__COMMON_OBJECT* pFindHandle = NULL;
	BOOL show_ln = FALSE, has_wc = FALSE;
	CHAR Buffer[MAX_FILE_NAME_LEN];
	CHAR wc_str[MAX_FILE_NAME_LEN];
	BOOL bFindNext = FALSE;
	int total_files = 0;

	strncpy(Buffer, FsGlobalData.CurrentDir, MAX_FILE_NAME_LEN);
	ToCapital(Buffer);

	int index = 1;
	while (index < pCmdObj->byParameterNum)
	{
		if (0 == strncmp(pCmdObj->Parameter[index], "/l", CMD_PARAMETER_LEN))
		{
			show_ln = TRUE;
			index++;
			continue;
		}
		/* Parameter is wildcard string. */
		strncpy(wc_str, pCmdObj->Parameter[index], CMD_PARAMETER_LEN);
		ToCapital(wc_str);
		has_wc = TRUE;
		index++;
	}

	ffd.bGetLongName = show_ln;

	pFindHandle = IOManager.FindFirstFile((__COMMON_OBJECT*)&IOManager, Buffer, &ffd);
	if (NULL == pFindHandle)
	{
		_hx_printf("Can not open directory[%s].\r\n", Buffer);
		goto __TERMINAL;
	}

	/* Show out detail information. */
	if (ffd.bGetLongName)
	{
		_hx_printf("File name[long short]:\r\n");
	}
	else
	{
		_hx_printf("    Name\t Size\t     F_D\r\n");
	}
	_hx_printf("---------------------------------------\r\n");

	do {
		if (__print_file_detail(&ffd, has_wc, wc_str))
		{
			total_files++;
		}
		bFindNext = IOManager.FindNextFile((__COMMON_OBJECT*)&IOManager,
			Buffer,
			pFindHandle,
			&ffd);
	} while (bFindNext);

	_hx_printf("    \r\nTotal [%d] file(s) listed.\r\n", total_files);

	/* Close the iteration transaction. */
	IOManager.FindClose((__COMMON_OBJECT*)&IOManager,
		Buffer,
		pFindHandle);

__TERMINAL:
	return SHELL_CMD_PARSER_SUCCESS;;
}
