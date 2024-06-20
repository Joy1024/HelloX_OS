//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 30, 2024
//    Module Name               : datadir.c
//    Module Funciton           : 
//                                Data directory's processing code for
//                                PE32/PE32+ format.
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
#include "loader.h"
#include "fmt_pe.h"
#include "stdio.h"
#include "string.h"

/* Debug routine to show out import tables. */
static void ShowImportTable(DWORD base_addr, DWORD rba)
{
	IMAGE_IMPORT_DESCRIPTOR* pImportTable;
	char idll_name[MAX_MODULE_NAME_LEN];
	char import_name[MAX_IMPORT_NAME_LEN];
	int mod_index = 0;
	unsigned short ordinal = 0;
	char* hint_name = NULL;
	unsigned short hint = 0;
	int total_import_num = 0;
	int total_mod_num = 0;
	DWORD* ilt_entry32 = NULL;

	pImportTable = (IMAGE_IMPORT_DESCRIPTOR*)(rba + base_addr);
	while (pImportTable->DUMMYUNIONNAME.OriginalFirstThunk)
	{
		strncpy(idll_name, (const char*)(base_addr + pImportTable->Name),
			MAX_MODULE_NAME_LEN);
		printf("Import dll: %s, iat: 0x%X\r\n", idll_name,
			(pImportTable->FirstThunk + base_addr));
		ilt_entry32 = (unsigned long*)(pImportTable->DUMMYUNIONNAME.OriginalFirstThunk + base_addr);
		while (*ilt_entry32)
		{
			if (*ilt_entry32 & 0x80000000)
			{
				/* Ordinal is used. */
				ordinal = (unsigned short)(*ilt_entry32 & 0xFFFF);
			}
			else
			{
				/* Hint/Name is used. */
				hint_name = (char*)(*ilt_entry32 + base_addr);
				hint = *(unsigned short*)hint_name;
				hint_name += 2;
				strncpy(import_name, hint_name, MAX_IMPORT_NAME_LEN);
				import_name[MAX_IMPORT_NAME_LEN - 1] = 0;
			}
			/* Show out result. */
			printf("  ordinal:%d, symbol:[%s], hint:%d\r\n", ordinal, import_name, hint);

			/* Check maximal symbol number. */
			total_import_num++;
			if (total_import_num > MAX_IMPORT_SYMBOL_NUM)
			{
				_hx_printf("Import too many symbols.\r\n");
				return;
			}
			ilt_entry32++;
		}

		total_mod_num++;
		if (total_mod_num > MAX_MODULE_NUM)
		{
			_hx_printf("Depend too many DLLs.\r\n");
			return;
		}

		pImportTable++;
	}
}

/* Get the loaded module given it's name. */
static __MODULE_DESCRIPTOR* GetLoadedModule(__PEB* peb, const char* module_name)
{
	int index = 0;
	while (index < peb->mq_tail)
	{
		if (0 == strncmp(peb->mq[index].module_name, module_name, MAX_MODULE_NAME_LEN))
		{
			return &peb->mq[index];
		}
		index++;
	}
	return NULL;
}

/* Get a symbol's virtual address by it's name. */
static DWORD GetSymbolByName(__MODULE_DESCRIPTOR* pModule, 
	const char* symbol_name)
{
	DWORD virtual_addr = 0;
	DWORD* name_table = NULL;
	const char* name_ptr = NULL;
	IMAGE_EXPORT_DIRECTORY* pExpDir = NULL;
	int name_index = -1;
	unsigned short ordinal, *ordinal_table;

	pExpDir = (IMAGE_EXPORT_DIRECTORY*)pModule->export_dir_addr;
	ordinal_table = (unsigned short*)pModule->addr_of_ordinaltable;
	name_table = (DWORD*)pModule->addr_of_name;

#if 0
	for (unsigned int index = 0; index < pExpDir->NumberOfNames; index++)
	{
		name_ptr = (char*)(name_table[index] + pModule->loaded_addr);
		if (0 == strncmp(name_ptr, (char*)symbol_name, MAX_IMPORT_NAME_LEN))
		{
			name_index = index;
			break;
		}
	}
#endif

	/* 
	 * Search the name table to find the symbol's 
	 * index. Use binary search to save time. 
	 */
	int base = 0;
	int top = pExpDir->NumberOfNames - 1;
	int mid = 0;
	int cmp_result = 0;
	while (base <= top)
	{
		mid = (base + top) / 2;
		name_ptr = (const char*)(name_table[mid] + pModule->loaded_addr);
		cmp_result = strncmp(symbol_name, name_ptr, MAX_IMPORT_NAME_LEN);
		if (0 == cmp_result)
		{
			name_index = mid;
			break;
		}
		if (cmp_result > 0)
		{
			/* Move forward. */
			base = mid + 1;
		}
		else {
			/* Move backword. */
			top = mid - 1;
		}
	}

	if (name_index < 0)
	{
		goto __TERMINAL;
	}
	ordinal = ordinal_table[name_index];

	/* Get the virtual address. */
	virtual_addr = ((DWORD*)pModule->addr_of_function)[ordinal];
	virtual_addr += pModule->loaded_addr;

	//_hx_printf("[%s]ordinal:%d, index:%d, VA:0x%X, symbol_name:%s\r\n",
	//	__func__, ordinal, name_index, virtual_addr, symbol_name);

__TERMINAL:
	return virtual_addr;
}

/* Get a symbol's virtual address by it's ordinal. */
static DWORD GetSymbolByOrdinal(__MODULE_DESCRIPTOR* pModule, 
	unsigned short biased_ordinal)
{
	DWORD virtual_addr = 0;
	unsigned int index = 0;
	unsigned short ordinal = 0;
	unsigned short* ordinal_table = NULL;
	IMAGE_EXPORT_DIRECTORY* pExportDir = NULL;
	char* symbol_name = NULL;

	pExportDir = (IMAGE_EXPORT_DIRECTORY*)pModule->export_dir_addr;
	ordinal_table = (unsigned short*)pModule->addr_of_ordinaltable;
	ordinal = biased_ordinal - (unsigned short)pModule->ordinal_base;
	if (ordinal > pExportDir->NumberOfFunctions)
	{
		/* Exceed range. */
		goto __TERMINAL;
	}

	/* Search index of ordinal in ordinal table. */
	for (index = 0; index < pExportDir->NumberOfNames; index++)
	{
		if (ordinal == ordinal_table[index])
		{
			break;
		}
	}
	if (index == pExportDir->NumberOfNames)
	{
		/* Can not find the ordinal. */
		goto __TERMINAL;
	}
	symbol_name = (char*)(((DWORD*)pModule->addr_of_name)[index] + pModule->loaded_addr);
	//_hx_printf("[%s]ordinal:%d, index:%d, symbol_name:%s\r\n",
	//	__func__, ordinal, index, symbol_name);

	/* Get the virtual address. */
	virtual_addr = ((DWORD*)pModule->addr_of_function)[ordinal];
	virtual_addr += pModule->loaded_addr;

__TERMINAL:
	return virtual_addr;
}

/*
 * Process Import Lookup Table for PE32, apply
 * binding from export table to import.
 */
static BOOL __bind_ilt_32(__MODULE_DESCRIPTOR* depend_mod,
	__MODULE_DESCRIPTOR* original_mod, 
	IMAGE_IMPORT_DESCRIPTOR* pImportTable,
	unsigned long* bind_num)
{
	unsigned long total_import_num = 0;
	DWORD* ilt_entry = NULL;
	DWORD* iat_entry = NULL;
	char import_symbol_name[MAX_IMPORT_NAME_LEN];
	DWORD virtual_addr = 0;
	unsigned short ordinal = 0;
	char* hint_name = NULL;
	unsigned short hint = 0;
	BOOL bResult = FALSE, ordinal_used = FALSE;

	if (pImportTable->DUMMYUNIONNAME.OriginalFirstThunk)
	{
		ilt_entry = (unsigned long*)(pImportTable->DUMMYUNIONNAME.OriginalFirstThunk +
			original_mod->loaded_addr);
		iat_entry = (unsigned long*)(pImportTable->FirstThunk +
			original_mod->loaded_addr);
		while (*ilt_entry)
		{
			if (*ilt_entry & 0x80000000)
			{
				/* Ordinal is used. */
				ordinal = (unsigned short)(*ilt_entry & 0xFFFF);
				virtual_addr = GetSymbolByOrdinal(depend_mod, ordinal);
				ordinal_used = TRUE;
			}
			else
			{
				/* Hint/Name is used. */
				hint_name = (char*)(*ilt_entry + original_mod->loaded_addr);
				hint = *(unsigned short*)hint_name;
				hint_name += 2;
				strncpy(import_symbol_name, hint_name, MAX_IMPORT_NAME_LEN);
				import_symbol_name[MAX_IMPORT_NAME_LEN - 1] = 0;
				virtual_addr = GetSymbolByName(depend_mod, import_symbol_name);
				ordinal_used = FALSE;
			}
			if (0 == virtual_addr)
			{
				/* Can not find the symbol. */
				_hx_printf("[%s]No symbol[ordinal:%d(%s), name:%s, from:%s] found.\r\n",
					__func__, ordinal,
					ordinal_used? "used" : "unused",
					import_symbol_name,
					depend_mod->module_name);
				goto __TERMINAL;
			}
			/* Bind the symbol. */
			*iat_entry = virtual_addr;

			//printf("[%s]ordinal:%d, symbol:[%s], hint:%d, virtual_addr:0x%X\r\n", 
			//	__func__, ordinal, import_symbol_name, hint, virtual_addr);

			/* Check maximal symbol number. */
			total_import_num++;
			if (total_import_num > MAX_IMPORT_SYMBOL_NUM)
			{
				_hx_printf("Module[%s] imported too many symbols.\r\n",
					original_mod->module_name);
				goto __TERMINAL;
			}
			ilt_entry++;
			iat_entry++;
		}
	}

	bResult = TRUE;

__TERMINAL:
	if (bind_num)
	{
		*bind_num = total_import_num;
	}
	return bResult;
}

/* 64 bits counterpart of __bind_ilt_32. */
static BOOL __bind_ilt_64(__MODULE_DESCRIPTOR* depend_mod,
	__MODULE_DESCRIPTOR* original_mod, 
	IMAGE_IMPORT_DESCRIPTOR* pImportTable,
	unsigned long* bind_num)
{
	unsigned long total_import_num = 0;
	ULONGLONG* ilt_entry64 = NULL;
	ULONGLONG* iat_entry64 = NULL;
	char import_symbol_name[MAX_IMPORT_NAME_LEN];
	ULONGLONG virtual_addr = 0;
	unsigned short ordinal = 0;
	char* hint_name = NULL;
	unsigned short hint = 0;
	BOOL bResult = FALSE, ordinal_used = FALSE;

	if (pImportTable->DUMMYUNIONNAME.OriginalFirstThunk)
	{
		ilt_entry64 = (unsigned long long*)(pImportTable->DUMMYUNIONNAME.OriginalFirstThunk +
			original_mod->loaded_addr);
		iat_entry64 = (unsigned long long*)(pImportTable->FirstThunk +
			original_mod->loaded_addr);
		while (*ilt_entry64)
		{
			if (*ilt_entry64 & 0x8000000000000000)
			{
				/* Ordinal is used. */
				ordinal = (unsigned short)(*ilt_entry64 & 0xFFFF);
				virtual_addr = GetSymbolByOrdinal(depend_mod, ordinal);
				ordinal_used = TRUE;
			}
			else
			{
				/* Hint/Name is used. */
				hint_name = (char*)(*ilt_entry64 + original_mod->loaded_addr);
				hint = *(unsigned short*)hint_name;
				hint_name += 2;
				strncpy(import_symbol_name, hint_name, MAX_IMPORT_NAME_LEN);
				import_symbol_name[MAX_IMPORT_NAME_LEN - 1] = 0;
				virtual_addr = GetSymbolByName(depend_mod, import_symbol_name);
				ordinal_used = FALSE;
			}
			if (0 == virtual_addr)
			{
				/* Can not find the symbol. */
				_hx_printf("[%s]No symbol[ordinal:%d(%s), name:%s, from:%s] found.\r\n",
					__func__, ordinal, 
					ordinal_used? "used" : "unused",
					import_symbol_name,
					depend_mod->module_name);
				goto __TERMINAL;
			}
			/* Bind the symbol. */
			*iat_entry64 = (unsigned long long)virtual_addr;

			//printf("[%s]ordinal:%d, symbol:[%s], hint:%d, virtual_addr:0x%X\r\n", 
			//	__func__, ordinal, import_symbol_name, hint, virtual_addr);

			/* Check maximal symbol number. */
			total_import_num++;
			if (total_import_num > MAX_IMPORT_SYMBOL_NUM)
			{
				_hx_printf("Module[%s] imported too many symbols.\r\n",
					original_mod->module_name);
				goto __TERMINAL;
			}
			ilt_entry64++;
			iat_entry64++;
		}
	}

	bResult = TRUE;

__TERMINAL:
	if (bind_num)
	{
		*bind_num = total_import_num;
	}
	return bResult;
}

/* 
 * Bind symbols that depend on this module. 
 * After this module is loaded into memory and
 * processed over, then should check back the
 * module that depends on this one and bind
 * the symbols.
 * pModule is the current module, not the original
 * module that depends on this one.
 */
static BOOL __bind_one_entry(__MODULE_DESCRIPTOR* pModule, __PEB* peb)
{
	BOOL bResult = FALSE;
	IMAGE_IMPORT_DESCRIPTOR* pImportTable = NULL;
	__MODULE_DESCRIPTOR* original_mod = NULL;
	const char* container_name = NULL;
	int total_import_num = 0;
	DWORD virtual_addr = 0;

	if (pModule->dep_this_index < 0)
	{
		/* No module depend on this. */
		goto __TERMINAL;
	}

	pImportTable = (IMAGE_IMPORT_DESCRIPTOR*)pModule->dep_import_desc;
	original_mod = &peb->mq[pModule->dep_this_index];
	container_name = (const char*)(pImportTable->Name + original_mod->loaded_addr);
	if (!(original_mod->module_flags & MODULE_FLAG_LOADED))
	{
		/* Exception case. */
		_hx_printf("Bad binding relationship:[%s] depends on [%s].\r\n",
			original_mod->module_name, pModule->module_name);
		goto __TERMINAL;
	}

	if (original_mod->module_flags & MODULE_FLAG_PE32)
	{
		bResult = __bind_ilt_32(pModule, original_mod, pImportTable, &total_import_num);
	}
	else {
		if (original_mod->module_flags & MODULE_FLAG_PE32PLUS)
		{
			bResult = __bind_ilt_64(pModule, original_mod, pImportTable, &total_import_num);
		}
	}

	if (!bResult)
	{
		_hx_printf("[%s]Bind fail for [%s] in [%s], from [%s].\r\n",
			__func__, container_name,
			original_mod->module_name,
			pModule->module_name);
	}

__TERMINAL:
	return bResult;
}

/*
 * Bind the module's import symbol to real
 * Virtual Address, since the module contains
 * all symbols are loaded into memory.
 * This routine is invoked by binding process,
 * just after the loading process.
 * All required information, is saved into
 * module object.
 */
static BOOL BindModuleSymbol(__PEB* peb, __MODULE_DESCRIPTOR* module)
{
	IMAGE_IMPORT_DESCRIPTOR* pImportTable = NULL;
	int import_entry_num = 0;
	DWORD base_addr = 0;
	const char* cont_dll_name = NULL;
	int mod_index = 0;
	__MODULE_DESCRIPTOR* depend_mod = NULL;
	BOOL bResult = FALSE;

	pImportTable = (IMAGE_IMPORT_DESCRIPTOR*)(module->import_dir_addr);
	base_addr = module->loaded_addr;
	import_entry_num = module->import_dir_len / sizeof(IMAGE_IMPORT_DESCRIPTOR);
	while (pImportTable && (pImportTable->DUMMYUNIONNAME.OriginalFirstThunk) &&
		import_entry_num)
	{
		/*
		 * Get the container DLL's name, in case the
		 * imported module is an API SET.
		 */
		cont_dll_name = GetContainerDllName((const char*)(base_addr + pImportTable->Name));
		if (NULL == cont_dll_name)
		{
			cont_dll_name = (const char*)(base_addr + pImportTable->Name);
		}

		/* The import module must be loaded. */
		depend_mod = GetLoadedModule(peb, cont_dll_name);
		if (NULL == depend_mod)
		{
			/* BUG raise. */
			_hx_printf("[%s]EXCEPTION: Module not found after loading process.\r\n", __func__);
			goto __TERMINAL;
		}
		/* Just bind it. */
		depend_mod->dep_this_index = module->module_index;
		depend_mod->dep_import_desc = (LPVOID)pImportTable;
		depend_mod->dep_base_addr = base_addr;
		if (!__bind_one_entry(depend_mod, peb))
		{
			/* May missing symbol, giveup. */
			goto __TERMINAL;
		}
		pImportTable++;
		import_entry_num--;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Bind each module's import symbol. */
unsigned long BindModulesSymbol(__PEB* peb)
{
	int index = 0;
	__MODULE_DESCRIPTOR* pModule = NULL;

	for (; index < peb->mq_tail; index++)
	{
		pModule = &peb->mq[index];
		if (!BindModuleSymbol(peb, pModule))
		{
			_hx_printf("[%s]Bind module[%s] fail.\r\n", __func__,
				pModule->module_name);
			return LOADER_ERROR_BIND;
		}
	}
	_hx_printf("[%s]total [%d] modules symbols are bound.\r\n",
		__func__, index);
	return LOADER_ERROR_OK;
}

/* Process export data directory. */
static BOOL ProcessExportDirectory(__MODULE_DESCRIPTOR* module, __PEB* peb,
	DWORD base_addr, DWORD rba)
{
	IMAGE_EXPORT_DIRECTORY* pExportDir = NULL;
	BOOL bResult = FALSE;

	pExportDir = (IMAGE_EXPORT_DIRECTORY*)(base_addr + rba);
	if (pExportDir->Characteristics)
	{
		goto __TERMINAL;
	}
	module->ordinal_base = pExportDir->OrdinalBase;
	module->addr_of_function = (pExportDir->AddressOfFunctions + base_addr);
	module->addr_of_name = (pExportDir->AddressOfNames + base_addr);
	module->addr_of_ordinaltable = (pExportDir->AddressOfNameOrdinals + base_addr);
	module->export_dir_addr = (LPVOID)pExportDir;

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Process import data directory. */
static BOOL ProcessImportDirectory(__MODULE_DESCRIPTOR* module, __PEB* peb,
	DWORD base_addr, DWORD rba, DWORD length)
{
	IMAGE_IMPORT_DESCRIPTOR* pImportTable = NULL;
	int table_entry_num = 0;
	const char* cont_dll_name = NULL;
	int mod_index = 0;
	int total_dll_num = 0;
	__MODULE_DESCRIPTOR* depend_mod = NULL;
	BOOL bResult = FALSE;

	total_dll_num = peb->mq_tail;
	pImportTable = (IMAGE_IMPORT_DESCRIPTOR*)(rba + base_addr);
	/* Get the entry number from data directory's length. */
	table_entry_num = length / sizeof(IMAGE_IMPORT_DESCRIPTOR);
	while (pImportTable->DUMMYUNIONNAME.OriginalFirstThunk && table_entry_num)
	{
		/* 
		 * Get the container DLL's name, in case the 
		 * imported module is API SET.
		 */
		cont_dll_name = GetContainerDllName((const char*)(base_addr + pImportTable->Name));
		if (NULL == cont_dll_name)
		{
			cont_dll_name = (const char*)(base_addr + pImportTable->Name);
		}

		/* Check if the depend module already loaded. */
		depend_mod = GetLoadedModule(peb, cont_dll_name);
		if (depend_mod)
		{
			pImportTable++;
			table_entry_num--;
			continue;
		}

		/*
		 * In case of file corruption or varius, too
		 * many imported DLL may reach bound.
		 */
		total_dll_num++;
		if (total_dll_num > MAX_MODULE_NUM)
		{
			_hx_printf("[%s]Depend too many DLLs.\r\n", __func__);
			goto __TERMINAL;
		}

		/* 
		 * Push the module into queue, so as the 
		 * loader will load it into memory and resolve
		 * import symbols later.
		 */
		mod_index = peb->mq_tail;
		peb->mq_tail++;
		depend_mod = &peb->mq[mod_index];
		strncpy(depend_mod->module_name, cont_dll_name, MAX_MODULE_NAME_LEN);
		depend_mod->module_flags = 0;
		depend_mod->module_flags |= MODULE_FLAG_DLL;
		depend_mod->module_index = mod_index;
		//depend_mod->dep_this_index = module->module_index;
		//depend_mod->dep_import_desc = (LPVOID)pImportTable;
		depend_mod->dep_base_addr = base_addr;

		pImportTable++;
		table_entry_num--;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Process resource data directory. */
static BOOL ProcessResourceDirectory(__MODULE_DESCRIPTOR* module, __PEB* peb,
	DWORD base_addr, DWORD rba)
{
	//printf("[FIXME]Resource directory unprocessed[rba:0x%X].\r\n", rba);
	return TRUE;
}

/*
 * Process image's data directories, it's the
 * unified entry and corresponding sub-routine
 * will be invoked according data directory
 * types.
 */
BOOL ProcessDataDirectory(__MODULE_DESCRIPTOR* module, __PEB* peb,
	IMAGE_NT_HEADERS* nt_hdr, DWORD base_addr)
{
	BOOL bResult = FALSE;
	DWORD rba = 0;

	/* Export directory. */
	rba = nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	if (rba)
	{
		bResult = ProcessExportDirectory(module, peb, base_addr, rba);
		if (!bResult)
		{
			goto __TERMINAL;
		}
	}

	/* Import directory. */
	rba = nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	if (rba)
	{
		/* Save import dir into module. */
		module->import_dir_addr = (base_addr + rba);
		module->import_dir_len = nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		bResult = ProcessImportDirectory(module, peb, base_addr, rba, module->import_dir_len);
		if (!bResult)
		{
			goto __TERMINAL;
		}
	}

	/* Resource directory. */
	rba = nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	if (rba)
	{
		bResult = ProcessResourceDirectory(module, peb, base_addr, rba);
		if (!bResult)
		{
			goto __TERMINAL;
		}
	}

	/* Process over. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* PE32plus version. */
BOOL ProcessDataDirectory64(__MODULE_DESCRIPTOR* module, __PEB* peb,
	IMAGE_NT_HEADERS64* nt_hdr, DWORD base_addr)
{
	BOOL bResult = FALSE;
	DWORD rba = 0;

	/* Export directory. */
	rba = nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
	if (rba)
	{
		bResult = ProcessExportDirectory(module, peb, base_addr, rba);
		if (!bResult)
		{
			goto __TERMINAL;
		}
	}

	/* Import directory. */
	rba = nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	if (rba)
	{
		module->import_dir_addr = (base_addr + rba);
		module->import_dir_len = nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
		bResult = ProcessImportDirectory(module, peb, base_addr, rba, module->import_dir_len);
		if (!bResult)
		{
			goto __TERMINAL;
		}
	}

	/* Resource directory. */
	rba = nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
	if (rba)
	{
		bResult = ProcessResourceDirectory(module, peb, base_addr, rba);
		if (!bResult)
		{
			goto __TERMINAL;
		}
	}

	/* Process over. */
	bResult = TRUE;

__TERMINAL:
	return bResult;
}

