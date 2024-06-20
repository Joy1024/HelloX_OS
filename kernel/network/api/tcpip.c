/**
* @file
* Sequential API Main thread module
*
*/

/*
* Copyright (c) 2001-2004 Swedish Institute of Computer Science.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
* SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
*
* This file is part of the lwIP TCP/IP stack.
*
* Author: Adam Dunkels <adam@sics.se>
*
*/

#include "lwip/opt.h"

#if !NO_SYS /* don't build if not configured for use in lwipopts.h */

#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "lwip/stats.h"
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

/* Migrate to HelloX kernel. */
#include "arch/lwipext.h"

/* global variables */
static tcpip_init_done_fn tcpip_init_done;
static void *tcpip_init_done_arg;
static sys_mbox_t mbox;

#if LWIP_TCPIP_CORE_LOCKING
/** The global semaphore to lock the stack. */
sys_mutex_t lock_tcpip_core;
#endif /* LWIP_TCPIP_CORE_LOCKING */

/* Routines refered in this module. */
extern BOOL ip_send_directly(struct pbuf* p, struct netif* out_if);

/**
* The main lwIP thread. This thread has exclusive access to lwIP core functions
* (unless access to them is not locked). Other threads communicate with this
* thread using message boxes.
*
* It also starts all the timers to make sure they are running in the right
* thread context.
*
* @param arg unused argument
*/
static void
tcpip_thread(void *arg)
{
	struct tcpip_msg *msg;
	__LWIP_EXTENSION* pExt = NULL;
	__INCOME_IP_PACKET* pIncomPkt = NULL;
	__OUTGOING_IP_PACKET* pOutPkt = NULL;
	DWORD dwFlags;
	BOOL bShouldBreak = FALSE;

	LWIP_UNUSED_ARG(arg);

	if (tcpip_init_done != NULL) {
		tcpip_init_done(tcpip_init_done_arg);
	}

	LOCK_TCPIP_CORE();
	while (1) {                          /* MAIN Loop */
		UNLOCK_TCPIP_CORE();
		LWIP_TCPIP_THREAD_ALIVE();
		/* wait for a message, timeouts are processed while waiting */
		sys_timeouts_mbox_fetch(&mbox, (void **)&msg);
		LOCK_TCPIP_CORE();
		switch (msg->type) {
#if LWIP_NETCONN
		case TCPIP_MSG_API:
			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: API message %p\n", (void *)msg));
			msg->msg.apimsg->function(&(msg->msg.apimsg->msg));
			break;
#endif /* LWIP_NETCONN */

#if !LWIP_TCPIP_CORE_LOCKING_INPUT
		case TCPIP_MSG_INPKT:
			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: PACKET %p\r\n", (void *)msg));
			BUG_ON(NULL == plwipProto);
			pExt = plwipProto->pProtoExtension;
			BUG_ON(NULL == pExt);
			/* 
			 * The packet(s) associated with this message is processed over, 
			 * so should break the while loop,otherwise may lead message queue
			 * full,even in rare case.
			 */
			bShouldBreak = FALSE;

			/* Fetch incoming packet(s) and process it from incoming list. */
			while (TRUE)
			{
				__ENTER_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
				if (0 == pExt->nIncomePktSize) /* No pending packet. */
				{
					BUG_ON(NULL != pExt->pIncomePktFirst);
					BUG_ON(NULL != pExt->pIncomePktLast);
					__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
					break;
				}
				else /* Incoming list is not empty. */
				{
					/* Fetch one pending IP packet. */
					pIncomPkt = pExt->pIncomePktFirst;
					pExt->pIncomePktFirst = pIncomPkt->pNext;
					pExt->nIncomePktSize--;
					if (NULL == pIncomPkt->pNext) /* Last one. */
					{
						/* Incoming pkt size must be 0. */
						BUG_ON(pExt->nIncomePktSize != 0);
						pExt->pIncomePktLast = NULL;
						/* Should break the loop. */
						bShouldBreak = TRUE;
					}
					__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
				}
				/* Process it. */
#if LWIP_ETHERNET
				if (msg->msg.inp.netif->flags & (NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET)) {
					//ethernet_input(msg->msg.inp.p, msg->msg.inp.netif);
					ethernet_input(pIncomPkt->p, pIncomPkt->in_if);
				}
				else
#endif /* LWIP_ETHERNET */
				{
					//ip_input(msg->msg.inp.p, msg->msg.inp.netif);
					ip_input(pIncomPkt->p, pIncomPkt->in_if);
				}
				/* Release the structure. */
				_hx_free(pIncomPkt);
				if (bShouldBreak)
				{
					break;
				}
			}
			memp_free(MEMP_TCPIP_MSG_INPKT, msg);
			break;
#endif /* LWIP_TCPIP_CORE_LOCKING_INPUT */
		case TCPIP_MSG_OUTPKT:
			/* 
			 * Output a IP packet to the given interface,
			 * or lookup routing table to find the out
			 * going interface if not specified by caller.
			 */
			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: PACKET %p\r\n", (void *)msg));
			BUG_ON(NULL == plwipProto);
			pExt = plwipProto->pProtoExtension;
			BUG_ON(NULL == pExt);
			/*
			* The packet(s) associated with this message is processed over,
			* so should break the while loop,otherwise may lead message queue
			* full,even in rare case.
			*/
			bShouldBreak = FALSE;

			/* Fetch out going packet(s) and process it. */
			while (TRUE)
			{
				__ENTER_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
				if (0 == pExt->nOutgoingPktSize) /* No pending packet. */
				{
					BUG_ON(NULL != pExt->pOutgoingPktFirst);
					BUG_ON(NULL != pExt->pOutgoingPktLast);
					__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
					break;
				}
				else /* Out going list is not empty. */
				{
					/* Fetch one pending IP packet. */
					pOutPkt = pExt->pOutgoingPktFirst;
					pExt->pOutgoingPktFirst = pOutPkt->pNext;
					pExt->nOutgoingPktSize--;
					if (NULL == pOutPkt->pNext) /* Last one. */
					{
						/* Out going pkt list size must be 0. */
						BUG_ON(pExt->nOutgoingPktSize != 0);
						pExt->pOutgoingPktLast = NULL;
						/* Should break the loop. */
						bShouldBreak = TRUE;
					}
					__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
				}
				/* Process it, invoke different routine according send flag. */
				if (pOutPkt->send_direct)
				{
					ip_send_directly(pOutPkt->p, pOutPkt->out_if);
				}
				else {
					/* 
					 * Use general ip output routine, the ip header 
					 * is constructed in it.
					 */
					if (ip_output(pOutPkt->p, &pOutPkt->src_addr,
						&pOutPkt->dst_addr, pOutPkt->ttl,
						pOutPkt->tos, pOutPkt->protocol))
					{
						IP_STATS_INC(ip.ctx_xmit_err);
					}
				}
				IP_STATS_INC(ip.ctx_xmit);
				/* Release pbuf object. */
				pbuf_free(pOutPkt->p);
				/* Release the structure. */
				_hx_free(pOutPkt);
				if (bShouldBreak)
				{
					break;
				}
			}
			memp_free(MEMP_TCPIP_MSG_INPKT, msg);
			break;

#if LWIP_NETIF_API
		case TCPIP_MSG_NETIFAPI:
			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: Netif API message %p\n", (void *)msg));
			msg->msg.netifapimsg->function(&(msg->msg.netifapimsg->msg));
			break;
#endif /* LWIP_NETIF_API */

		case TCPIP_MSG_CALLBACK:
			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: CALLBACK %p\n", (void *)msg));
			msg->msg.cb.function(msg->msg.cb.ctx);
			memp_free(MEMP_TCPIP_MSG_API, msg);
			break;

#if LWIP_TCPIP_TIMEOUT
		case TCPIP_MSG_TIMEOUT:
			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: TIMEOUT %p\n", (void *)msg));
			sys_timeout(msg->msg.tmo.msecs, msg->msg.tmo.h, msg->msg.tmo.arg);
			memp_free(MEMP_TCPIP_MSG_API, msg);
			break;
		case TCPIP_MSG_UNTIMEOUT:
			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: UNTIMEOUT %p\n", (void *)msg));
			sys_untimeout(msg->msg.tmo.h, msg->msg.tmo.arg);
			memp_free(MEMP_TCPIP_MSG_API, msg);
			break;
#endif /* LWIP_TCPIP_TIMEOUT */

		default:
			LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_thread: invalid message: %d\n", msg->type));
			_hx_printf("[%s]invalid message[%d]\r\n", __func__, msg->type);
			LWIP_ASSERT("tcpip_thread: invalid message", 0);
			break;
		}
	}
}

/**
* Pass a received packet to tcpip_thread for input processing
*
* @param p the received packet, p->payload pointing to the Ethernet header or
*          to an IP header (if inp doesn't have NETIF_FLAG_ETHARP or
*          NETIF_FLAG_ETHERNET flags)
* @param inp the network interface on which the packet was received
*/
err_t
tcpip_input(struct pbuf *p, struct netif *inp)
{
#if LWIP_TCPIP_CORE_LOCKING_INPUT
	err_t ret;
	LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip_input: PACKET %p/%p\n", (void *)p, (void *)inp));
	LOCK_TCPIP_CORE();
#if LWIP_ETHERNET
	if (inp->flags & (NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET)) {
		ret = ethernet_input(p, inp);
	}
	else
#endif /* LWIP_ETHERNET */
	{
		ret = ip_input(p, inp);
	}
	UNLOCK_TCPIP_CORE();
	return ret;
#else /* LWIP_TCPIP_CORE_LOCKING_INPUT */
	struct tcpip_msg *msg;
	__LWIP_EXTENSION* pExt = NULL;
	__INCOME_IP_PACKET* pIncomPkt = NULL;
	DWORD dwFlags;

	BUG_ON(NULL == plwipProto);
	pExt = (__LWIP_EXTENSION*)plwipProto->pProtoExtension;
	BUG_ON(NULL == pExt);

	/* Create an incoming IP packet struct to hold the incoming packet. */
	pIncomPkt = (__INCOME_IP_PACKET*)_hx_malloc(sizeof(__INCOME_IP_PACKET));
	if (NULL == pIncomPkt)
	{
		return ERR_MEM;
	}
	pIncomPkt->p = p;
	pIncomPkt->in_if = inp;
	pIncomPkt->pNext = NULL;

	/*
	* Try to put the incoming packet to incoming list.
	* Use critical section since this routine maybe called in NIC
	* interrupt handler.
	*/
	__ENTER_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
	if (0 == pExt->nIncomePktSize) /* No packet in list yet. */
	{
		if (sys_mbox_valid(&mbox)) {
			msg = (struct tcpip_msg *)memp_malloc(MEMP_TCPIP_MSG_INPKT);
			if (msg == NULL) {
				__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
				_hx_free(pIncomPkt);
				return ERR_MEM;
			}

			msg->type = TCPIP_MSG_INPKT;
			msg->msg.inp.p = p;
			msg->msg.inp.netif = inp;
			if (sys_mbox_trypost(&mbox, msg) != ERR_OK) {
				__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
				memp_free(MEMP_TCPIP_MSG_INPKT, msg);
				_hx_free(pIncomPkt);
				return ERR_MEM;
			}
			/* Put the incoming packet structure into incoming list. */
			pExt->pIncomePktLast = pIncomPkt;
			pExt->pIncomePktFirst = pIncomPkt;
			pExt->nIncomePktSize++;
			__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
			return ERR_OK;
		}
	}
	else  /* The incoming list already has packet(s). */
	{
		/* Just put the incoming packet into list. */
		pExt->pIncomePktLast->pNext = pIncomPkt;
		pExt->pIncomePktLast = pIncomPkt;
		pExt->nIncomePktSize++;
		__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
		return ERR_OK;
	}
	return ERR_VAL;
#endif /* LWIP_TCPIP_CORE_LOCKING_INPUT */
}

/**
* Pass an IP packet to tcpip_thread for output directly,bypass
* the IP stack's normal checking.It's mainly used by DPI or other
* applications to construct any kind of IP packets and send out
* to wire.
* In order to guarantee the throughput we use two step mechanism,
* just same as IP packet incoming:
* step 1: Link the packet into a global list and send a message
* to TCP/IP thread;
* step 2: TCP/IP thread will dispatch the out going packet in a
* while batch.
*
* NOTE: This routine runs in asynchronous mode, i.e, it send a 
* message to tcpip thread and hook the pbuf in sending list,
* and return immediately. So pbuf must be kept without freeing
* before tcpip thread sending over, but pbuf with PBUF_REF
* type's memory buffer maybe released after this function return,
* thus lead illegal memory accessing. In order to avoid this
* scenario, we just do not support the pbuf with PBUF_REF and
* PBUF_ROM types.
*/
err_t tcpip_output(struct pbuf *p, struct netif *out_if,
	BOOL send_direct,
	ip_addr_t* src, ip_addr_t* dst,
	u8_t ttl, u8_t tos, u8_t protocol)
{
	struct tcpip_msg *msg;
	__LWIP_EXTENSION* pExt = NULL;
	__OUTGOING_IP_PACKET* pOutPkt = NULL;
	struct pbuf* q = p;
	DWORD dwFlags;

	BUG_ON(NULL == plwipProto);
	pExt = (__LWIP_EXTENSION*)plwipProto->pProtoExtension;
	BUG_ON(NULL == pExt);

	/* Check pbuf's type, PBUF_REF/ROM are not supported yet. */
	while (q)
	{
		if ((PBUF_REF == q->type) || (PBUF_ROM == q->type))
		{
			return ERR_ARG;
		}
		q = q->next;
	}

	/* Create an incoming IP packet struct to hold the incoming packet. */
	pOutPkt = (__OUTGOING_IP_PACKET*)_hx_malloc(sizeof(__OUTGOING_IP_PACKET));
	if (NULL == pOutPkt)
	{
		return ERR_MEM;
	}
	pOutPkt->p = p;
	pOutPkt->out_if = out_if;
	pOutPkt->send_direct = send_direct;
	pOutPkt->src_addr = *src;
	pOutPkt->dst_addr = *dst;
	pOutPkt->ttl = ttl;
	pOutPkt->tos = tos;
	pOutPkt->protocol = protocol;
	pOutPkt->pNext = NULL;

	/*
	* Try to put the out going packet to list.
	* Use critical section since this routine maybe called in NIC
	* interrupt handler.
	*/
	__ENTER_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
	if (0 == pExt->nOutgoingPktSize) /* No packet in list yet. */
	{
		if (sys_mbox_valid(&mbox)) {
			msg = (struct tcpip_msg *)memp_malloc(MEMP_TCPIP_MSG_INPKT);
			if (msg == NULL) {
				__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
				_hx_free(pOutPkt);
				return ERR_MEM;
			}

			msg->type = TCPIP_MSG_OUTPKT;
			msg->msg.inp.p = p;
			msg->msg.inp.netif = out_if;
			if (sys_mbox_trypost(&mbox, msg) != ERR_OK) {
				__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
				memp_free(MEMP_TCPIP_MSG_INPKT, msg);
				_hx_free(pOutPkt);
				return ERR_MEM;
			}
			/* Put the incoming packet structure into out going list. */
			pExt->pOutgoingPktLast = pOutPkt;
			pExt->pOutgoingPktFirst = pOutPkt;
			pExt->nOutgoingPktSize++;
			__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
			return ERR_OK;
		}
	}
	else  /* The out going list already has packet(s). */
	{
		/* Just put the packet into list. */
		pExt->pOutgoingPktLast->pNext = pOutPkt;
		pExt->pOutgoingPktLast = pOutPkt;
		pExt->nOutgoingPktSize++;
		__LEAVE_CRITICAL_SECTION_SMP(pExt->spin_lock, dwFlags);
		return ERR_OK;
	}
	return ERR_VAL;
}

/**
* Call a specific function in the thread context of
* tcpip_thread for easy access synchronization.
* A function called in that way may access lwIP core code
* without fearing concurrent access.
*
* @param f the function to call
* @param ctx parameter passed to f
* @param block 1 to block until the request is posted, 0 to non-blocking mode
* @return ERR_OK if the function was called, another err_t if not
*/
err_t
tcpip_callback_with_block(tcpip_callback_fn function, void *ctx, u8_t block)
{
	struct tcpip_msg *msg;

	if (sys_mbox_valid(&mbox)) {
		msg = (struct tcpip_msg *)memp_malloc(MEMP_TCPIP_MSG_API);
		if (msg == NULL) {
			return ERR_MEM;
		}

		msg->type = TCPIP_MSG_CALLBACK;
		msg->msg.cb.function = function;
		msg->msg.cb.ctx = ctx;
		if (block) {
			sys_mbox_post(&mbox, msg);
		}
		else {
			if (sys_mbox_trypost(&mbox, msg) != ERR_OK) {
				memp_free(MEMP_TCPIP_MSG_API, msg);
				return ERR_MEM;
			}
		}
		return ERR_OK;
	}
	return ERR_VAL;
}

#if LWIP_TCPIP_TIMEOUT
/**
* call sys_timeout in tcpip_thread
*
* @param msec time in milliseconds for timeout
* @param h function to be called on timeout
* @param arg argument to pass to timeout function h
* @return ERR_MEM on memory error, ERR_OK otherwise
*/
err_t
tcpip_timeout(u32_t msecs, sys_timeout_handler h, void *arg)
{
	struct tcpip_msg *msg;

	if (sys_mbox_valid(&mbox)) {
		msg = (struct tcpip_msg *)memp_malloc(MEMP_TCPIP_MSG_API);
		if (msg == NULL) {
			return ERR_MEM;
		}

		msg->type = TCPIP_MSG_TIMEOUT;
		msg->msg.tmo.msecs = msecs;
		msg->msg.tmo.h = h;
		msg->msg.tmo.arg = arg;
		sys_mbox_post(&mbox, msg);
		return ERR_OK;
	}
	return ERR_VAL;
}

/**
* call sys_untimeout in tcpip_thread
*
* @param msec time in milliseconds for timeout
* @param h function to be called on timeout
* @param arg argument to pass to timeout function h
* @return ERR_MEM on memory error, ERR_OK otherwise
*/
err_t
tcpip_untimeout(sys_timeout_handler h, void *arg)
{
	struct tcpip_msg *msg;

	if (sys_mbox_valid(&mbox)) {
		msg = (struct tcpip_msg *)memp_malloc(MEMP_TCPIP_MSG_API);
		if (msg == NULL) {
			return ERR_MEM;
		}

		msg->type = TCPIP_MSG_UNTIMEOUT;
		msg->msg.tmo.h = h;
		msg->msg.tmo.arg = arg;
		sys_mbox_post(&mbox, msg);
		return ERR_OK;
	}
	return ERR_VAL;
}
#endif /* LWIP_TCPIP_TIMEOUT */

#if LWIP_NETCONN
/**
* Call the lower part of a netconn_* function
* This function is then running in the thread context
* of tcpip_thread and has exclusive access to lwIP core code.
*
* @param apimsg a struct containing the function to call and its parameters
* @return ERR_OK if the function was called, another err_t if not
*/
err_t
tcpip_apimsg(struct api_msg *apimsg)
{
	struct tcpip_msg msg;
#ifdef LWIP_DEBUG
	/* catch functions that don't set err */
	apimsg->msg.err = ERR_VAL;
#endif

	if (sys_mbox_valid(&mbox)) {
		msg.type = TCPIP_MSG_API;
		msg.msg.apimsg = apimsg;
		sys_mbox_post(&mbox, &msg);
		sys_arch_sem_wait(&apimsg->msg.conn->op_completed, 0);
		return apimsg->msg.err;
	}
	return ERR_VAL;
}

#if LWIP_TCPIP_CORE_LOCKING
/**
* Call the lower part of a netconn_* function
* This function has exclusive access to lwIP core code by locking it
* before the function is called.
*
* @param apimsg a struct containing the function to call and its parameters
* @return ERR_OK (only for compatibility fo tcpip_apimsg())
*/
err_t
tcpip_apimsg_lock(struct api_msg *apimsg)
{
#ifdef LWIP_DEBUG
	/* catch functions that don't set err */
	apimsg->msg.err = ERR_VAL;
#endif

	LOCK_TCPIP_CORE();
	apimsg->function(&(apimsg->msg));
	UNLOCK_TCPIP_CORE();
	return apimsg->msg.err;

}
#endif /* LWIP_TCPIP_CORE_LOCKING */
#endif /* LWIP_NETCONN */

#if LWIP_NETIF_API
#if !LWIP_TCPIP_CORE_LOCKING
/**
* Much like tcpip_apimsg, but calls the lower part of a netifapi_*
* function.
*
* @param netifapimsg a struct containing the function to call and its parameters
* @return error code given back by the function that was called
*/
err_t
tcpip_netifapi(struct netifapi_msg* netifapimsg)
{
	struct tcpip_msg msg;

	if (sys_mbox_valid(&mbox)) {
		err_t err = sys_sem_new(&netifapimsg->msg.sem, 0);
		if (err != ERR_OK) {
			netifapimsg->msg.err = err;
			return err;
		}

		msg.type = TCPIP_MSG_NETIFAPI;
		msg.msg.netifapimsg = netifapimsg;
		sys_mbox_post(&mbox, &msg);
		sys_sem_wait(&netifapimsg->msg.sem);
		sys_sem_free(&netifapimsg->msg.sem);
		return netifapimsg->msg.err;
	}
	return ERR_VAL;
}
#else /* !LWIP_TCPIP_CORE_LOCKING */
/**
* Call the lower part of a netifapi_* function
* This function has exclusive access to lwIP core code by locking it
* before the function is called.
*
* @param netifapimsg a struct containing the function to call and its parameters
* @return ERR_OK (only for compatibility fo tcpip_netifapi())
*/
err_t
tcpip_netifapi_lock(struct netifapi_msg* netifapimsg)
{
	LOCK_TCPIP_CORE();
	netifapimsg->function(&(netifapimsg->msg));
	UNLOCK_TCPIP_CORE();
	return netifapimsg->msg.err;
}
#endif /* !LWIP_TCPIP_CORE_LOCKING */
#endif /* LWIP_NETIF_API */

/**
* Initialize this module:
* - initialize all sub modules
* - start the tcpip_thread
*
* @param initfunc a function to call when tcpip_thread is running and finished initializing
* @param arg argument to pass to initfunc
*/
void
tcpip_init(tcpip_init_done_fn initfunc, void *arg)
{
	lwip_init();

	tcpip_init_done = initfunc;
	tcpip_init_done_arg = arg;
	if (sys_mbox_new(&mbox, TCPIP_MBOX_SIZE) != ERR_OK) {
		LWIP_ASSERT("failed to create tcpip_thread mbox", 0);
	}
#if LWIP_TCPIP_CORE_LOCKING
	if (sys_mutex_new(&lock_tcpip_core) != ERR_OK) {
		LWIP_ASSERT("failed to create lock_tcpip_core", 0);
	}
#endif /* LWIP_TCPIP_CORE_LOCKING */

	sys_thread_new(TCPIP_THREAD_NAME, tcpip_thread, NULL, TCPIP_THREAD_STACKSIZE, TCPIP_THREAD_PRIO);
}

/**
* Simple callback function used with tcpip_callback to free a pbuf
* (pbuf_free has a wrong signature for tcpip_callback)
*
* @param p The pbuf (chain) to be dereferenced.
*/
static void
pbuf_free_int(void *p)
{
	struct pbuf *q = (struct pbuf *)p;
	pbuf_free(q);
}

/**
* A simple wrapper function that allows you to free a pbuf from interrupt context.
*
* @param p The pbuf (chain) to be dereferenced.
* @return ERR_OK if callback could be enqueued, an err_t if not
*/
err_t
pbuf_free_callback(struct pbuf *p)
{
	return tcpip_callback_with_block(pbuf_free_int, p, 0);
}

/**
* A simple wrapper function that allows you to free heap memory from
* interrupt context.
*
* @param m the heap memory to free
* @return ERR_OK if callback could be enqueued, an err_t if not
*/
err_t
mem_free_callback(void *m)
{
	return tcpip_callback_with_block(mem_free, m, 0);
}

#endif /* !NO_SYS */
