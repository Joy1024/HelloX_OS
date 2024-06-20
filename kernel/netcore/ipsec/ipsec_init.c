//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Sep 4, 2022
//    Module Name               : init.c
//    Module Funciton           : 
//                                IPSec initialization code, will be invoked
//                                before IPSec work.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <stdio.h>

#include "samgr.h"

/* Entry point of initializaiton. */
BOOL __ipsec_init()
{
	BOOL bResult = FALSE;

	bResult = SAManager.Initialize(&SAManager);
	if (!bResult)
	{
		__LOG("[%s]Initialize SA manager failed.\r\n", __func__);
		goto __TERMINAL;
	}

__TERMINAL:
	return bResult;
}
