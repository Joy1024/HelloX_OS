//***********************************************************************/
//    Author                    : Garry
//    Original Date             : 15 FEB,2009
//    Module Name               : FS.H
//    Module Funciton           : 
//    Description               : Header file of fs application.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#ifndef __FS_H__
#define __FS_H__

/* fs application's global data. */
typedef struct tag__FS_GLOBAL_DATA {
	__FS_ARRAY_ELEMENT FsArray[FILE_SYSTEM_NUM];
	/* Current file system identifier. */
	BYTE CurrentFs;
	CHAR CurrentDir[MAX_FILE_NAME_LEN];
	/* TRUE if this struct initialized. */
	BOOL bInitialized;
}__FS_GLOBAL_DATA;

extern __FS_GLOBAL_DATA FsGlobalData;

/* Sub command handlers. */
DWORD __handler_fs_dir(__CMD_PARA_OBJ* pCmdObj);
DWORD __handler_fs_copy(__CMD_PARA_OBJ* pCmdObj);
DWORD __handler_fs_del(__CMD_PARA_OBJ* pCmdObj);

/* Main entry of fs application. */
DWORD fsEntry(LPVOID);

#endif //__FS_H__
