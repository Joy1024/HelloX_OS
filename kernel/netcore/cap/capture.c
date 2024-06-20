//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Oct 02, 2023
//    Module Name               : capture.c
//    Module Funciton           : 
//                                Packet capturing functions of HelloX, 
//                                a diagnosing tool used to debug network
//                                related issues.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <genif.h>
#include <netmgr.h>
#include <lwip/pbuf.h>

#include "capture.h"

/* 
 * Thread handle of the capturing thread. 
 * There is a dedicated kernel thread running
 * in background to process packet filtering and
 * saving. 
 * If the capturing function of a genif is enabled,
 * by setting the corresponding flag, then all
 * network packets input/output of the genif will
 * be copied to the capturing list, the capturing
 * thread processes the list, apply filtering, and
 * save the selected packet into file.
 * Other works related packet capturing are also
 * handled by this thread.
 */
static HANDLE capture_thread = NULL;

/* List element to save the captured packet. */
typedef struct tag__CAPTURE_PACKET_ELEMENT{
	struct pbuf* pkt_buff;
	struct tag__CAPTURE_PACKET_ELEMENT* pNext;
}__CAPTURE_PACKET_ELEMENT;

/* 
 * Each genif with capturing enabled has one 
 * capture management block.
 * This block is saved into genif's capture_arg
 * and is used as input parameter when invoke
 * the capture_call back routine.
 */
typedef struct {
	__GENERIC_NETIF* pGenif;    /* genif. */
	HANDLE save_file;           /* File to save packet. */
	__CAPTURE_PACKET_ELEMENT* capture_list_head;
	__CAPTURE_PACKET_ELEMENT* capture_list_tail;
	uint32_t list_size;         /* capture list's size. */
	__SPIN_LOCK spin_lock;      /* spinlock protect list. */
}__CAPTURE_MANAGEMENT_BLOCK;

/* Maximal list's size. */
#define MAX_CAPTURE_LIST_SIZE 256

/* 
 * Only one capturing task is supported yet, 
 * so just use one static management block. 
 */
static __CAPTURE_MANAGEMENT_BLOCK capture_block = { 0 };

/* 
 * Call back routine of genif, in
 * packet input or output procedure.
 * It just link the packet(pbuf) into
 * capture list and send a message 
 * to capture back ground thread.
 */
static int __cap_call_back(__GENERIC_NETIF* pGenif,
	struct pbuf* original_pkt,
	void* parm)
{
	__CAPTURE_PACKET_ELEMENT* pElement = NULL;
	__CAPTURE_MANAGEMENT_BLOCK* pMgnBlock = (__CAPTURE_MANAGEMENT_BLOCK*)parm;
	struct pbuf* new_pkt = NULL;
	unsigned long ulFlags = 0;
	MSG msg;
	int result = -1;

	BUG_ON((NULL == pGenif) || (NULL == original_pkt) || (NULL == pMgnBlock));

	/* Link the packet into list. */
	pElement = (__CAPTURE_PACKET_ELEMENT*)_hx_malloc(sizeof(__CAPTURE_PACKET_ELEMENT));
	if (NULL == pElement)
	{
		goto __TERMINAL;
	}
	pElement->pNext = NULL;

	/* Clone the original packet. */
	new_pkt = pbuf_alloc(PBUF_RAW, original_pkt->tot_len, PBUF_RAM);
	if (NULL == new_pkt)
	{
		goto __TERMINAL;
	}
	pbuf_copy(new_pkt, original_pkt);
	pElement->pkt_buff = new_pkt;

	__ENTER_CRITICAL_SECTION_SMP(pMgnBlock->spin_lock, ulFlags);
	if (pMgnBlock->list_size > MAX_CAPTURE_LIST_SIZE)
	{
		/* List too long, give up. */
		pbuf_free(new_pkt);
		__LEAVE_CRITICAL_SECTION_SMP(pMgnBlock->spin_lock, ulFlags);
		goto __TERMINAL;
	}
	pMgnBlock->list_size++;
	if (NULL == pMgnBlock->capture_list_head)
	{
		/* List is empty. */
		pMgnBlock->capture_list_head = pElement;
		pMgnBlock->capture_list_tail = pElement;
		/* Trigger the capture thread to process it. */
		BUG_ON(NULL == capture_thread);
		msg.wCommand = MSG_CAPTURE_ARRIVAL;
		msg.dwParam = (unsigned long)pMgnBlock;
		SendMessage(capture_thread, &msg);
	}
	else {
		/* List is not empty, just link into tail. */
		pMgnBlock->capture_list_tail->pNext = pElement;
		pMgnBlock->capture_list_tail = pElement;
	}
	__LEAVE_CRITICAL_SECTION_SMP(pMgnBlock->spin_lock, ulFlags);

	/* Mark it's success. */
	result = 1;

__TERMINAL:
	if (result < 0)
	{
		/* Error raise, release all resources. */
		if (pElement)
		{
			_hx_free(pElement);
		}
	}
	return result;
}

/*
 * Work function of new captured packet
 * arrival message, it's runs in capture
 * thread's context.
 * @pMgnBlock is transfered to the thread
 * by thread message's dwParam parameter.
 */
static void __arrival_worker(__CAPTURE_MANAGEMENT_BLOCK* pMgnBlock)
{
	__CAPTURE_PACKET_ELEMENT* pElement = NULL;
	pcap_packet_header pcap_hdr;
	unsigned long ulFlags = 0;
	BOOL bShouldBreak = FALSE;
	unsigned long written = 0;

	BUG_ON(NULL == pMgnBlock);
	/* Save file must be opened. */
	BUG_ON(NULL == pMgnBlock->save_file); 

	/* Process the capture list. */
	while (TRUE)
	{
		__ENTER_CRITICAL_SECTION_SMP(pMgnBlock->spin_lock, ulFlags);
		if (0 == pMgnBlock->list_size)
		{
			/* List is empty. */
			__LEAVE_CRITICAL_SECTION_SMP(pMgnBlock->spin_lock, ulFlags);
			break;
		}
		/* Get the first element object from list. */
		pMgnBlock->list_size--;
		pElement = pMgnBlock->capture_list_head;
		if (0 == pMgnBlock->list_size)
		{
			/* Only one element. */
			pMgnBlock->capture_list_head = NULL;
			pMgnBlock->capture_list_tail = NULL;
			bShouldBreak = TRUE;
		}
		else {
			pMgnBlock->capture_list_head = pElement->pNext;
		}
		__LEAVE_CRITICAL_SECTION_SMP(pMgnBlock->spin_lock, ulFlags);

		/* May apply filtering here, in future. */
		/* Construct the captured packet header. */
		pcap_hdr.incl_len = pElement->pkt_buff->tot_len;
		pcap_hdr.orig_len = pElement->pkt_buff->tot_len;
		/* Capture time, should revise later. */
		pcap_hdr.ts_sec = 0; 
		pcap_hdr.ts_usec = 0;
		/* Save the packet into file. */
		WriteFile(pMgnBlock->save_file, sizeof(pcap_packet_header),
			&pcap_hdr, &written);
		WriteFile(pMgnBlock->save_file, pElement->pkt_buff->tot_len,
			pElement->pkt_buff->payload, &written);

		/* Release the element block object. */
		pbuf_free(pElement->pkt_buff);
		_hx_free(pElement);

		/* Break the loop since we process over. */
		if (bShouldBreak)
		{
			break;
		}
	}
}

/* Start capture on a specified genif. */
static BOOL __start_worker(__GENERIC_NETIF* pGenif)
{
	__CAPTURE_MANAGEMENT_BLOCK* pMgnBlock = NULL;
	char save_file_name[MAX_FILE_NAME_LEN];
	pcap_global_header pgh;
	unsigned long written = 0;
	unsigned long ulFlags = 0;
	BOOL bResult = FALSE;

	BUG_ON(NULL == pGenif);
	pMgnBlock = 
		(__CAPTURE_MANAGEMENT_BLOCK*)_hx_malloc(sizeof(__CAPTURE_MANAGEMENT_BLOCK));
	if (NULL == pMgnBlock)
	{
		_hx_printf("[%s]out of memory.\r\n", __func__);
		goto __TERMINAL;
	}
	memset(pMgnBlock, 0, sizeof(__CAPTURE_MANAGEMENT_BLOCK));
	pMgnBlock->pGenif = pGenif;

	/* Create the save file and write global header. */
	_hx_sprintf(save_file_name, "c:\\syslog\\genif%d.cap", 
		pGenif->if_index);
	pMgnBlock->save_file = CreateFile(
		save_file_name,
		FILE_ACCESS_READWRITE | FILE_OPEN_ALWAYS,
		0, NULL);
	if (NULL == pMgnBlock->save_file)
	{
		_hx_printf("[%s]could not open capture file[%s].\r\n",
			__func__,
			save_file_name);
		goto __TERMINAL;
	}

	/* Write the file's global header. */
	pgh.magic_number = PCAP_MAGIC_NUMBER;
	pgh.linktype = PCAP_LINKTYPE_ETHERNET;
	pgh.sigfigs = 0;
	pgh.snaplen = 0x400;
	pgh.thiszone = 0;
	pgh.version_major = PCAP_VERSION_MAJOR;
	pgh.version_minor = PCAP_VERSION_MINOR;

	if (!WriteFile(pMgnBlock->save_file, sizeof(pcap_global_header),
		&pgh, &written))
	{
		_hx_printf("[%s]write global header error.\r\n",
			__func__);
		goto __TERMINAL;
	}

	/* 
	 * May set genif's flag and callback routine here, 
	 * check genif's type before set.
	 */
	__ENTER_CRITICAL_SECTION_SMP(pGenif->spin_lock, ulFlags);
	pGenif->genif_flags |= GENIF_FLAG_CAPTURE;
	pGenif->capture_callback = __cap_call_back;
	pGenif->capture_param = pMgnBlock;
	__LEAVE_CRITICAL_SECTION_SMP(pGenif->spin_lock, ulFlags);

	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		if (pMgnBlock)
		{
			if (pMgnBlock->save_file)
			{
				CloseFile(pMgnBlock->save_file);
			}
			_hx_free(pMgnBlock);
		}
		/* Decrease the refer counter of genif. */
		NetworkManager.ReleaseGenif(pGenif);
	}
	return bResult;
}

/* Worker routine of stop capturing function. */
static void __stop_worker(__CAPTURE_MANAGEMENT_BLOCK* pMgnBlock)
{
	int if_index = -1;

	BUG_ON(NULL == pMgnBlock);
	BUG_ON(pMgnBlock->list_size != 0);

	/* Close the saving file. */
	CloseFile(pMgnBlock->save_file);
	if_index = pMgnBlock->pGenif->if_index;
	NetworkManager.ReleaseGenif(pMgnBlock->pGenif);
	_hx_free(pMgnBlock);

	_hx_printf("Stop capturing on genif[%d]\r\n", if_index);
	return;
}

/*
 * Back ground thread of packet capturing function.
 * It's this thread that applies capturing filter,
 * and write the packet into file.
 */
static unsigned long __capture_work_thread(void* pData)
{
	MSG msg;
	__CAPTURE_MANAGEMENT_BLOCK* pMgnBlock = NULL;
	__GENERIC_NETIF* pGenif = NULL;
	unsigned long written = 0;
	
	/* Main message pump. */
	while (GetMessage(&msg))
	{
		switch (msg.wCommand)
		{
		case MSG_CAPTURE_ARRIVAL:
			pMgnBlock = (__CAPTURE_MANAGEMENT_BLOCK*)msg.dwParam;
			BUG_ON(NULL == pMgnBlock);
			/* Process the captured packets of this genif. */
			__arrival_worker(pMgnBlock);
			break;
		case MSG_CAPTURE_START:
			pGenif = (__GENERIC_NETIF*)msg.dwParam;
			BUG_ON(NULL == pGenif);
			__start_worker(pGenif);
			break;
		case MSG_CAPTURE_STOP:
			pMgnBlock = (__CAPTURE_MANAGEMENT_BLOCK*)msg.dwParam;
			BUG_ON(NULL == pMgnBlock);
			/* Invoke worker routine. */
			__stop_worker(pMgnBlock);
			break;
		default:
			break;
		}
	}

	return 0;
}

/* Routines that implement the capture commands. */
void CaptureStart(int genif_index)
{
	__GENERIC_NETIF* pGenif = NULL;
	MSG msg;

	/* Get the genif object. */
	pGenif = NetworkManager.GetGenifByIndex(genif_index);
	if (NULL == pGenif)
	{
		_hx_printf("[%s]invalid genif index[%d]\r\n",
			__func__, genif_index);
		return;
	}

	/* Let the background thread to apply start. */
	msg.wCommand = MSG_CAPTURE_START;
	msg.dwParam = (unsigned long)pGenif;
	SendMessage(capture_thread, &msg);
}

void CaptureStop(int genif_index)
{
	__GENERIC_NETIF* pGenif = NULL;
	MSG msg;
	unsigned long ulFlags = 0;

	/* Get the genif object. */
	pGenif = NetworkManager.GetGenifByIndex(genif_index);
	if (NULL == pGenif)
	{
		_hx_printf("[%s]no genif with index[%d] was got.\r\n",
			__func__, genif_index);
		goto __TERMINAL;
	}

	/* Clear the capture flag and callback routine. */
	__ENTER_CRITICAL_SECTION_SMP(pGenif->spin_lock, ulFlags);
	pGenif->genif_flags &= ~GENIF_FLAG_CAPTURE;
	pGenif->capture_callback = NULL;
	msg.dwParam = (unsigned long)pGenif->capture_param;
	pGenif->capture_param = NULL;
	__LEAVE_CRITICAL_SECTION_SMP(pGenif->spin_lock, ulFlags);

	/* Decrease genif's refcounter. */
	NetworkManager.ReleaseGenif(pGenif);

	/* 
	 * Send a stop message to back thread to clear 
	 * all resource associated with the capture task.
	 */
	msg.wCommand = MSG_CAPTURE_STOP;
	SendMessage(capture_thread, &msg);

__TERMINAL:
	return;
}

/* Initializer of packet capturing function. */
BOOL CaptureInitialize()
{
	BOOL bResult = FALSE;
	
	/* Start the background capture thread. */
	capture_thread = CreateKernelThread(
		0, KERNEL_THREAD_STATUS_READY,
		PRIORITY_LEVEL_NORMAL,
		__capture_work_thread,
		NULL, NULL, "capture");
	if (NULL == capture_thread)
	{
		_hx_printf("[%s]create capture thread failed.\r\n",
			__func__);
		goto __TERMINAL;
	}

	bResult = TRUE;

__TERMINAL:
	return bResult;
}
