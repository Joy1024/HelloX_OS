//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jan 13, 2023
//    Module Name               : hhtt.c
//    Module Funciton           : 
//    Description               : 
//                                Implementation code of HHTT tunnel protocol.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#include <StdAfx.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <genif.h>
#include <proto.h>
#include <netmgr.h>
#include <lwip/ip.h>
#include <lwip/inet.h>
#include <lwip/udp.h>
#include <lwip/tcpip.h>

#include "routmgr/rtmgr.h"
#include "ipsec/aes.h"
#include "hhtt.h"
#include "ipcomm.h"

/* HT2 tunnel ID. */
static unsigned long global_tunnel_id = 0;
/* Global session id. */
static uint16_t global_session_id = 0;

/* How many ht2 tunnel(s) totally. */
static int ht2_tun_num = 0;

/* The background thread of ht2 protocol. */
static HANDLE ht2_thread = NULL;

/* tx queue pointers and it's length. */
static __XMIT_QUEUE_ELEMENT* txq_head = NULL;
static __XMIT_QUEUE_ELEMENT* txq_tail = NULL;
static int txq_length = 0;
static int txq_max_length = 0;

/* rx queue pointers and it's length. */
static __XMIT_QUEUE_ELEMENT* rxq_head = NULL;
static __XMIT_QUEUE_ELEMENT* rxq_tail = NULL;
static int rxq_length = 0;
static int rxq_max_length = 0;

/* spin lock to protect the rx/tx queues. */
__SPIN_LOCK rx_lock = SPIN_LOCK_INIT_VALUE;
__SPIN_LOCK tx_lock = SPIN_LOCK_INIT_VALUE;

/* Load cipher key from storage. */
static BOOL __old_load_cipher_key(__HT2_TUNNEL_OBJECT* ht2_tunnel, int key_bits)
{
	static unsigned char __key[] =
	{ 0x18, 0x96, 0xC9, 0x8E, 0x72, 0xCE, 0x56, 0xCF,
	  0x8E, 0xDE, 0x24, 0xD7, 0x84, 0xFF, 0x62, 0xCB,
	  0xE9, 0x42, 0x36, 0xCF, 0x92, 0xDC, 0x24, 0x90,
	  0x36, 0xA0, 0x8F, 0x0F, 0x99, 0x38, 0xFD, 0x82 };

	if (key_bits != 256)
	{
		return FALSE;
	}

	/* Use build in key for debugging currently. */
	memcpy(ht2_tunnel->cipher_key, __key, HT2_CIPHER_KEY_LENGTH);
	return TRUE;
}

/*
 * Initialize the cipher key of ht2 object.
 * It use random numbers to fill the key, so
 * if the user does not specifiy key file,
 * packet also could be encrypted, but the
 * peer side can not decrypt it. This schema
 * guarantees the secret of packet.
 * If we use fixed value(such as 0) to initialize
 * the key, hacker can decrypt the packet since
 * key is open.
 */
static void __init_cipher_key(__HT2_TUNNEL_OBJECT* ht2_tunnel)
{
	int key_length = HT2_CIPHER_KEY_LENGTH;

	BUG_ON(NULL == ht2_tunnel);
	/*
	 * Init random seed using system
	 * tick counter.
	 */
	srand(System.dwClockTickCounter);

	for (int i = 0; i < HT2_CIPHER_KEY_LENGTH; i++)
	{
		ht2_tunnel->cipher_key[i] = (unsigned char)rand();
	}
}

/* Load cipher key from storage. */
static BOOL __load_key_from_file(__HT2_TUNNEL_OBJECT* ht2_tunnel, 
	const char* key_file)
{
	char key_file_name[MAX_FILE_NAME_LEN];
	unsigned char __key[HT2_CIPHER_KEY_LENGTH + 16];
	HANDLE hFile = NULL;
	BOOL bResult = FALSE;

	/* Validates the key file name. */
	if (strlen(key_file) >= MAX_FILE_NAME_LEN)
	{
		_hx_printf("Key file name too long[%d]\r\n",
			strlen(key_file));
		goto __TERMINAL;
	}

	/* Use local file name. */
	strcpy(key_file_name, key_file);
	
	/* Try to open the key file. */
	hFile = CreateFile(key_file_name, FILE_ACCESS_READ, 0, NULL);
	if (NULL == hFile)
	{
		_hx_printf("[%s]can not open key file[%s]\r\n",
			__func__, key_file_name);
		goto __TERMINAL;
	}

	/* Load key from file. */
	unsigned long read_sz = 0;
	if (!ReadFile(hFile, HT2_CIPHER_KEY_LENGTH, __key, &read_sz))
	{
		_hx_printf("[%s]can not load key.\r\n", __func__);
		goto __TERMINAL;
	}
	if (read_sz != HT2_CIPHER_KEY_LENGTH)
	{
		_hx_printf("[%s]key length is incorrect[%d].\r\n",
			__func__, read_sz);
		goto __TERMINAL;
	}

	/* Save key to ht2 tunnel object. */
	memcpy(ht2_tunnel->cipher_key, __key, HT2_CIPHER_KEY_LENGTH);

	bResult = TRUE;

__TERMINAL:
	if (hFile)
	{
		CloseFile(hFile);
	}
	return bResult;
}

/* 
 * Init a initial vector. 
 * It just generate 16 random numbers using
 * system random routine.
 */
static void __init_aes_iv(unsigned char* iv)
{
	/* 
	 * Init random seed using system 
	 * tick counter.
	 */
	srand(System.dwClockTickCounter);

	for (int i = 0; i < AES_IV_LENGTH; i++)
	{
		iv[i] = (unsigned char)rand();
	}
}

/* Init the cipher context. */
static BOOL __init_cipher_context(__HT2_TUNNEL_OBJECT* ht2_tunnel)
{
	mbedtls_aes_context* pCtx = NULL;

	BUG_ON(NULL == ht2_tunnel);

	/* Initializes cipher key. */
	__init_cipher_key(ht2_tunnel);

	/* Create algorithm specified context. */
	pCtx = _hx_malloc(sizeof(*pCtx));
	if (NULL == pCtx)
	{
		return FALSE;
	}

	/* Init it. */
	mbedtls_aes_init(pCtx);

	ht2_tunnel->cipher_ctx = pCtx;
	return TRUE;
}

/* Release the cipher context. */
static void __release_cipher_context(__HT2_TUNNEL_OBJECT* ht2_tunnel)
{
	BUG_ON(NULL == ht2_tunnel);
	BUG_ON(NULL == ht2_tunnel->cipher_ctx);
	_hx_free(ht2_tunnel->cipher_ctx);
	memset(ht2_tunnel->cipher_key, 0, sizeof(HT2_CIPHER_KEY_LENGTH));
}

/*
 * Adjust tcp session's MSS to fit the ht2 tunnel.
 * The tcp mss is set by OS according it's local net
 * interface's mtu, for example 1500 bytes in ethernet,
 * the mss will be set as 1460(exclude ip and tcp hdrs).
 * For HT2 protocol extra headers are added in front
 * of the original ip packet, so the mss must be adjusted
 * to deduct the extra headers, include underly ip,
 * udp, and ht2 header.
 */
static BOOL __adjust_tcp_mss(__GENERIC_NETIF* pGenif,
	struct ip_hdr* ip_hdr, struct pbuf* pkt)
{
	BOOL bResult = FALSE;
	struct tcp_hdr* tcp_hdr = NULL;
	int iph_len = 0;
	u8_t tcp_flags;

	BUG_ON((NULL == pGenif) || (NULL == ip_hdr) || NULL == (pkt));

	/* Only work for tcp. */
	if (ip_hdr->_proto != IP_PROTO_TCP)
	{
		goto __TERMINAL;
	}

	/* get tcp header. */
	iph_len = IPH_HL(ip_hdr);
	iph_len *= 4;
	tcp_hdr = (struct tcp_hdr*)((char*)ip_hdr + iph_len);

	/* Apply tcp mss adjust, if it's syn segment. */
	tcp_flags = TCPH_FLAGS(tcp_hdr);
	if (tcp_flags & TCP_SYN)
	{
		bResult = tcp_mss_adjust(pGenif, tcp_hdr);
		if (!bResult)
		{
			goto __TERMINAL;
		}

		/*
		 * Reset the check sum since tcp header
		 * is modified.
		 */
		_iphdr_check_sum(ip_hdr, pkt);
	}

__TERMINAL:
	return bResult;
}

/* 
 * If we can accept the packet. 
 * The routine matches protocol in IP header,
 * and udp source and destination port in
 * UDP header, if they are all match, then
 * the packet is HT2 and for us.
 */
static BOOL ht2_packet_for_me(__GENERIC_NETIF* genif_parent,
	__GENERIC_NETIF* genif_this, struct pbuf* pkt)
{
	struct ip_hdr* pIpHdr = NULL;
	struct udp_hdr* pUdpHdr = NULL;
	__HT2_TUNNEL_OBJECT* ht2_tunnel = NULL;
	int ip_hdr_len = 0;

	BUG_ON(NULL == genif_parent || NULL == genif_this || NULL == pkt);
	ht2_tunnel = (__HT2_TUNNEL_OBJECT*)genif_this->pGenifExtension;
	BUG_ON(NULL == ht2_tunnel);

	/* Validate packet's size. */
	if (pkt->tot_len < sizeof(struct ip_hdr) + sizeof(struct udp_hdr))
	{
		return FALSE;
	}

	/* Check the IP header. */
	pIpHdr = (struct ip_hdr*)pkt->payload;
	if ((pIpHdr->src.addr != ht2_tunnel->dest_addr.addr) ||
		(pIpHdr->dest.addr != ht2_tunnel->src_addr.addr) ||
		(pIpHdr->_proto != IP_PROTO_UDP))
	{
		return FALSE;
	}

	/* Locate UDP header and check it. */
	ip_hdr_len = IPH_HL(pIpHdr) * 4;
	if (ip_hdr_len != sizeof(struct ip_hdr))
	{
		return FALSE;
	}
	pUdpHdr = (struct udp_hdr*)((char*)pkt->payload + ip_hdr_len);
	if ((pUdpHdr->src != htons(ht2_tunnel->dest_port)) ||
		(pUdpHdr->dest != htons(ht2_tunnel->src_port)))
	{
		return FALSE;
	}

	ht2_tunnel->for_me_match++;

	return TRUE;
}

/*
 * Input routine of ht2 interface, it runs in the
 * context of the caller, such as another kernel thread
 * that want to send packet through this ht2 interface.
 * It just link the raw packet into rx queue of ht2
 * back thread, and sends a message to the thread if
 * the queue is empty, to drive back thread work.
 */
static int ht2tun_input(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt)
{
	MSG msg;
	__XMIT_QUEUE_ELEMENT* rxq_element = NULL;
	__HT2_TUNNEL_OBJECT* ht2_tunnel = NULL;
	unsigned long ulFlags;
	int result = -1;

	BUG_ON((NULL == pGenif) || (NULL == pkt));
	
	/* Allocate new xmit queue element and init it. */
	rxq_element = (__XMIT_QUEUE_ELEMENT*)_hx_malloc(sizeof(__XMIT_QUEUE_ELEMENT));
	if (NULL == rxq_element)
	{
		goto __TERMINAL;
	}
	rxq_element->ht2_tunnel = (__HT2_TUNNEL_OBJECT*)pGenif->pGenifExtension;
	BUG_ON(NULL == rxq_element->ht2_tunnel);
	rxq_element->pGenif = pGenif;
	/*
	 * Increase the ref counter of pbuf, to
	 * avoid it is released in caller of this
	 * routine. It must be freed in back thread.
	 */
	pbuf_ref(pkt);
	rxq_element->p = pkt;
	rxq_element->pNext = NULL;

	/* Link it into rx queue. */
	__ENTER_CRITICAL_SECTION_SMP(rx_lock, ulFlags);
	if (0 == rxq_length)
	{
		/* No element in queue. */
		BUG_ON(rxq_head || rxq_tail);
		rxq_head = rxq_tail = rxq_element;
		rxq_length++;
		__LEAVE_CRITICAL_SECTION_SMP(rx_lock, ulFlags);
		
		/* Trigger the thread to work. */
		msg.wCommand = HT2_MESSAGE_RECV;
		msg.wParam = 0;
		msg.dwParam = 0;
		if (!SendMessage(ht2_thread, &msg))
		{
			/* Message queue may full, drop the packet. */
			__ENTER_CRITICAL_SECTION_SMP(rx_lock, ulFlags);
			rxq_head = rxq_tail = NULL;
			rxq_length--;
			__LEAVE_CRITICAL_SECTION_SMP(rx_lock, ulFlags);
			goto __TERMINAL;
		}
	}
	else {
		/* queue is not full, link it. */
		BUG_ON((NULL == rxq_head) || (NULL == rxq_tail));
		rxq_tail->pNext = rxq_element;
		rxq_tail = rxq_element;
		rxq_length++;
		__LEAVE_CRITICAL_SECTION_SMP(rx_lock, ulFlags);
	}

	/* Mark return success. */
	result = 0;

__TERMINAL:
	if (result)
	{
		/* error, clean all resources. */
		if (rxq_element)
		{
			pbuf_free(rxq_element->p);
			_hx_free(rxq_element);
		}
	}
	return result;
}

/* 
 * Output routine for l3. 
 * This routine runs in the caller's context,
 * such as another thread that wants send packet
 * through the ht2 tunnel interface. It's
 * just link the raw output packet into tx queue
 * of ht2 background thread, and sends a message
 * to the thread if queue is empty.
 * Since encryption will be applied to the packet,
 * it just creates a new pbuf and copy from old one,
 * then let the old one go, and use the new one
 * to continue.
 */
static int ht2tun_output_l3(__GENERIC_NETIF* pGenif, struct pbuf* pkt,
	__COMMON_NETWORK_ADDRESS* comm_addr)
{
	MSG msg;
	__XMIT_QUEUE_ELEMENT* txq_element = NULL;
	__HT2_TUNNEL_OBJECT* ht2_tunnel = NULL;
	struct pbuf* buf_new = NULL;
	uint16_t new_buff_len = 0;
	unsigned long ulFlags;
	int result = -1;

	BUG_ON((NULL == pGenif) || (NULL == pkt));

	/* 
	 * Create a new pbuf to instead the original one. 
	 * No need to reserve header space for the new pbuf,
	 * since the ht2 protocol will add a new pbuf to
	 * hold tunnel and underlay headers. 
	 * But the buffer should align with the alignment
	 * of AES, since we use the buffer to hold encrypted
	 * packet.
	 */
	new_buff_len = pkt->tot_len;
	new_buff_len = AES_BUFFER_LENGTH_ALIGN(new_buff_len);
	buf_new = pbuf_alloc(PBUF_RAW, new_buff_len, PBUF_RAM);
	if (NULL == buf_new)
	{
		goto __TERMINAL;
	}
	pbuf_copy(buf_new, pkt);
	/* Pad the rest of buffer with 0. */
	for (int i = pkt->tot_len; i < new_buff_len; i++)
	{
		((char*)buf_new->payload)[i] = 0;
	}

	/* Allocate new xmit queue element and init it. */
	txq_element = (__XMIT_QUEUE_ELEMENT*)_hx_malloc(sizeof(__XMIT_QUEUE_ELEMENT));
	if (NULL == txq_element)
	{
		goto __TERMINAL;
	}
	txq_element->ht2_tunnel = (__HT2_TUNNEL_OBJECT*)pGenif->pGenifExtension;
	BUG_ON(NULL == txq_element->ht2_tunnel);
	txq_element->pGenif = pGenif;
	txq_element->p = buf_new;
	/* 
	 * Save old pbuf's length, since the 
	 * new pbuf's length is not same as
	 * the old one since alignment.
	 */
	txq_element->original_sz = pkt->tot_len;
	txq_element->pNext = NULL;

	/* Link it into tx queue. */
	__ENTER_CRITICAL_SECTION_SMP(tx_lock, ulFlags);
	if (0 == txq_length)
	{
		/* No element in queue. */
		BUG_ON(txq_head || txq_tail);
		txq_head = txq_tail = txq_element;
		txq_length++;
		__LEAVE_CRITICAL_SECTION_SMP(tx_lock, ulFlags);

		/* Trigger the thread to work. */
		msg.wCommand = HT2_MESSAGE_SEND;
		msg.wParam = 0;
		msg.dwParam = 0;
		if (!SendMessage(ht2_thread, &msg))
		{
			/* Message queue may full, drop the packet. */
			__ENTER_CRITICAL_SECTION_SMP(tx_lock, ulFlags);
			txq_head = txq_tail = NULL;
			txq_length--;
			__LEAVE_CRITICAL_SECTION_SMP(tx_lock, ulFlags);
			goto __TERMINAL;
		}
	}
	else {
		/* queue is not full, link it. */
		BUG_ON((NULL == txq_head) || (NULL == txq_tail));
		txq_tail->pNext = txq_element;
		txq_tail = txq_element;
		txq_length++;
		__LEAVE_CRITICAL_SECTION_SMP(tx_lock, ulFlags);
	}

	/* Mark return success. */
	result = 0;

__TERMINAL:
	if (result)
	{
		/* error, clean all resources. */
		if (txq_element)
		{
			pbuf_free(txq_element->p);
			_hx_free(txq_element);
		}
		if (buf_new)
		{
			pbuf_free(buf_new);
		}
	}
	return result;
}

/* Output routine for HHTT VTI, for Layer 2 mode. */
static int ht2tun_output_l2(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt)
{
	return -1;
}

#define DEBUG_NO_ENCRYPT

/*
 * Construct the transport layer(udp) header and
 * ht2 tunnel header, and chain the
 * original data pbuf together and return it.
 */
static struct pbuf* construct_ht2_header(__HT2_TUNNEL_OBJECT* ht2_tunnel, 
	struct pbuf* p, int pad)
{
	struct udp_hdr* pUdpHdr = NULL;
	struct pbuf* tunnel_header = NULL;
	__HT2_HEADER* ht2_hdr = NULL;
	uint16_t tunnel_header_sz = 0;
	unsigned char* payload = NULL;

	/*
	 * create a new pbuf to hold the upd and ht2 hdr.
	 * let ip_output routine to fill IP header, 
	 * so we must reserve ip and ethernet header space in 
	 * this pbuf object, by using PBUF_IP layer flag.
	 */
	tunnel_header_sz = sizeof(struct udp_hdr);
	tunnel_header_sz += sizeof(__HT2_HEADER);
	tunnel_header = pbuf_alloc(PBUF_IP, tunnel_header_sz, PBUF_RAM);
	if (NULL == tunnel_header)
	{
		__LOG("[%s]out of memory.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Construct ht2 and udp header. */
	payload = (unsigned char*)tunnel_header->payload;
	/* UDP hdr. */
	pUdpHdr = (struct udp_hdr*)payload;
	pUdpHdr->src = htons(ht2_tunnel->src_port);
	pUdpHdr->dest = htons(ht2_tunnel->dest_port);
	pUdpHdr->len = htons(tunnel_header_sz + p->tot_len);
	pUdpHdr->chksum = 0;

	/* HT2 hdr. */
	payload += sizeof(struct udp_hdr);
	ht2_hdr = (__HT2_HEADER*)payload;
	ht2_hdr->ver = 0;
	ht2_hdr->pad = pad;
	ht2_hdr->session_id = htons(ht2_tunnel->session_id);
	ht2_hdr->seq_no = htonl(ht2_tunnel->seqno_tx);
	ht2_tunnel->seqno_tx++;
	memcpy(ht2_hdr->iv, ht2_tunnel->iv, AES_IV_LENGTH);

	/* Chain them together. */
	pbuf_chain(tunnel_header, p);

__TERMINAL:
	return tunnel_header;
}

/* 
 * Encrypt one ht2 pakcet. 
 * Return value:
 *  < 0  : error, returns the error code;
 *  >= 0 : ok, returns the pad number in buffer tail,
 *         to align with AES alignment.
 * NOTE: The pbuf's length must align with AES
 *       alignment value, it's the duty of l3_output
 *       routine of the ht2 vti.
 */
static int encrypt_ht2_packet(__HT2_TUNNEL_OBJECT* ht2_tunnel, struct pbuf* pkt)
{
	mbedtls_aes_context* pCtx = NULL;
	int error = -1, align_len = 0;
	unsigned char local_iv[16];

	BUG_ON((NULL == ht2_tunnel) || (NULL == pkt));
	BUG_ON(pkt->tot_len <= 0);
	BUG_ON(NULL == ht2_tunnel->cipher_ctx);
	/* buffer's length must be aligned. */
	align_len = pkt->tot_len;
	align_len = AES_BUFFER_LENGTH_ALIGN(align_len);
	BUG_ON(pkt->tot_len != align_len);

	/* Prepare the encryption context. */
	pCtx = ht2_tunnel->cipher_ctx;
	mbedtls_aes_init(pCtx);
	error = mbedtls_aes_setkey_enc(pCtx, ht2_tunnel->cipher_key, 
		HT2_CIPHER_KEY_LENGTH * 8);
	if (error)
	{
		goto __exit;
	}
	__init_aes_iv(local_iv);
	/* Save the initial IV to fill into ht2 header. */
	memcpy(ht2_tunnel->iv, local_iv, 16);

	/* Carry out encryption. */
	error = mbedtls_aes_crypt_cbc(pCtx, MBEDTLS_AES_ENCRYPT, 
		align_len,
		local_iv, /* Must use local iv. */
		pkt->payload, pkt->payload);
	if (error)
	{
		printf("[%s]decrypt packet error: %d\r\n", error);
	}

__exit:
	return error;
}

/* 
 * Output work routine for HHTT VTI, for Layer 3 mode. 
 * This routine runs in ht2 background thread.
 * Procedure of this routine:
 * 1. Allocate a new pbuf(PBUF_IP) to hold the ht2 tunnel
 *    header, udp header;
 * 2. Construct the headers and put into pbuf;
 * 3. Chain the buf with pkt;
 * 4. Send out by invoking ip_output routine;
 */
static int __ht2tun_output_l3_work(__GENERIC_NETIF* pGenif, 
	struct pbuf* pkt,
	uint16_t original_sz,
	__COMMON_NETWORK_ADDRESS* comm_addr)
{
	__HT2_TUNNEL_OBJECT* ht2_tunnel = NULL;
	struct pbuf* q = NULL;
	struct ip_hdr* ip_hdr = NULL;
	int error = -1;

	/* Get ht2 tunnel object. */
	ht2_tunnel = (__HT2_TUNNEL_OBJECT*)pGenif->pGenifExtension;
	BUG_ON(NULL == ht2_tunnel);

	/* Adjust tcp mss. */
	ip_hdr = (struct ip_hdr*)pkt->payload;
	__adjust_tcp_mss(pGenif, ip_hdr, pkt);

	/* 
	 * Encrypt the packet. error indicates the 
	 * paded byte number in buffer after encryption,
	 * when >= 0. Otherwise is the error code.
	 */
	error = encrypt_ht2_packet(ht2_tunnel, pkt);
	if (error < 0)
	{
		return error;
	}

	/* 
	 * Append the transport and tunnel header. 
	 * Should omit the pad in rear of buffer.
	 */
	int pad = pkt->tot_len - original_sz;
	q = construct_ht2_header(ht2_tunnel, pkt, pad);
	if (NULL == q)
	{
		return -1;
	}
	ht2_tunnel->pkt_send++;

	/* q will be released in tcpip_output. */
	return tcpip_output(q, NULL, FALSE, &ht2_tunnel->src_addr, &ht2_tunnel->dest_addr,
		ht2_tunnel->ttl,
		ht2_tunnel->qos,
		IP_PROTO_UDP);
}

/* 
 * Input work routine for HHTT VTI. 
 * This routine runs in ht2 background thread.
 * It decrypts the cipher packet, get the clear
 * text packet and delivery it to general ip
 * input routine.
 */
static int __ht2tun_input_work(struct __GENERIC_NETIF* pGenif, struct pbuf* pkt)
{
	struct ip_hdr* pIpHdr = NULL;
	struct udp_hdr* pUdpHdr = NULL;
	int offset = 0, cipher_length = 0;
	__HT2_TUNNEL_OBJECT* ht2_tunnel = NULL;
	struct netif* netif = NULL;
	__HT2_HEADER* ht2_hdr = NULL;
	unsigned char* cipher_txt = NULL;
	unsigned char iv[16];

	BUG_ON(NULL == pGenif || NULL == pkt);
	ht2_tunnel = (__HT2_TUNNEL_OBJECT*)pGenif->pGenifExtension;
	BUG_ON(NULL == ht2_tunnel);

	/* 
	 * The whole packet must be in one continous 
	 * buffer, can not support buffer fragment now.
	 */
	if (pkt->len != pkt->tot_len)
	{
		__LOG("[%s]buffer fragmented.\r\n", __func__);
		return -1;
	}
	/* Check the length value. */
	if (pkt->tot_len <= sizeof(struct udp_hdr) + sizeof(__HT2_HEADER))
	{
		__LOG("[%s]invalid packet.\r\n", __func__);
		return -1;
	}

	/* Get the IP header and strip it. */
	cipher_txt = (unsigned char*)pkt->payload;
	pIpHdr = (struct ip_hdr*)cipher_txt;
	offset = IPH_HL(pIpHdr) * 4;
	
	/* 
	 * skip udp header. 
	 * TO DO: Should validate the UDP header.
	 */
	cipher_txt += offset;
	pUdpHdr = (struct udp_hdr*)cipher_txt;
	offset += sizeof(struct udp_hdr);
	
	/* Now is the ht2 header. */
	cipher_txt += sizeof(struct udp_hdr);
	ht2_hdr = (__HT2_HEADER*)cipher_txt;
	offset += sizeof(__HT2_HEADER);
	/* get iv. */
	memcpy(iv, ht2_hdr->iv, 16);

	/* Reach the real cipher text. */
	cipher_txt += sizeof(__HT2_HEADER);
	cipher_length = pkt->tot_len;
	cipher_length -= offset;
	/* Cipher text length must be AES aligned. */
	if (cipher_length != AES_BUFFER_LENGTH_ALIGN(cipher_length))
	{
		__LOG("[%s]corruption encrypted packet.\r\n", __func__);
		return -1;
	}

	/* Set up decrypt environment. */
	mbedtls_aes_init(ht2_tunnel->cipher_ctx);
	mbedtls_aes_setkey_dec(ht2_tunnel->cipher_ctx, ht2_tunnel->cipher_key,
		HT2_CIPHER_KEY_LENGTH * 8);

	/* Decrypt the cipher text. */
	int error = mbedtls_aes_crypt_cbc(ht2_tunnel->cipher_ctx,
		MBEDTLS_AES_DECRYPT,
		cipher_length,
		iv,
		cipher_txt,
		cipher_txt);
	if (error < 0)
	{
		__LOG("[%s]decrypt packet failed[%d]\r\n", __func__,
			error);
		return -1;
	}

	/* 
	 * Skip all headers, and modify length 
	 * fields of pbuf to skip rear pading. 
	 * NOTE: this may lead bugs since we
	 * modify pbuf's fields without api functions.
	 * THIS IS THE FIRST PLACE TO CHECK WHEN
	 * SYSTEM LEVEL BUG RAISE.
	 */
	pbuf_header(pkt, -offset);
	pkt->len -= ht2_hdr->pad;
	pkt->tot_len -= ht2_hdr->pad;

	/* 
	 * Now pbuf pkt contains the nake and clear
	 * IP pakcet, adjust the tcp mss if necessary.
	 */
	pIpHdr = (struct ip_hdr*)pkt->payload;
	__adjust_tcp_mss(pGenif, pIpHdr, pkt);
	 
	/* 
	 *Then just drop to general intput routine 
	 * to make farther processing.
	 */
	ht2_tunnel->pkt_recv++;
	netif = (struct netif*)pGenif->proto_binding[0].pIfState;
	BUG_ON(NULL == netif);
	return general_ip_input(pkt, netif);
}

/* Specific show routine of HHTT VTI. */
static void ht2tun_specific_show(__GENERIC_NETIF* genif)
{
	__HT2_TUNNEL_OBJECT* ht2_tunnel = NULL;
	unsigned long ulFlags = 0;
	__HT2_TUNNEL_OBJECT* ht2_new = NULL;

	BUG_ON(NULL == genif);
	ht2_new = (__HT2_TUNNEL_OBJECT*)_hx_malloc(sizeof(__HT2_TUNNEL_OBJECT));
	if (NULL == ht2_new)
	{
		return;
	}
	
	/* 
	 * The tunnel object maybe being destroyed in 
	 * other thread context, so use spin lock to
	 * protect it and save a copy to display.
	 */
	__ENTER_CRITICAL_SECTION_SMP(genif->spin_lock, ulFlags);
	ht2_tunnel = (__HT2_TUNNEL_OBJECT*)genif->pGenifExtension;
	if (NULL == ht2_tunnel)
	{
		/* The tunnel genif maybe destroyed. */
		__LEAVE_CRITICAL_SECTION_SMP(genif->spin_lock, ulFlags);
		_hx_printf("  [WARNING]genif maybe destroyed.\r\n");
		return;
	}
	memcpy(ht2_new, ht2_tunnel, sizeof(__HT2_TUNNEL_OBJECT));
	__LEAVE_CRITICAL_SECTION_SMP(genif->spin_lock, ulFlags);

	/* Just show out. */
	_hx_printf("  ht2 tunnel object info:\r\n");
	_hx_printf("    tunnel dest: %s\r\n", inet_ntoa(ht2_new->dest_addr));
	_hx_printf("    tunnel src: %s\r\n", inet_ntoa(ht2_new->src_addr));
	_hx_printf("    pkt_send/pkt_recv: %d/%d\r\n", ht2_new->pkt_send, ht2_new->pkt_recv);
	_hx_printf("    for me pkt match: %d\r\n", ht2_new->for_me_match);
	_hx_printf("    rxq/txq length: %d\r\n", rxq_length, txq_length);
	_hx_printf("    max rxq/txq length: %d/%d\r\n", rxq_max_length, txq_max_length);
	return;
}

/*
 * Background thread work routine of ht2.
 * The incoming and out going ht2 packets, will be
 * delivered to this thread to process, include
 * encryption/decryption, by wrapped using xmit
 * queue element.
 */
static unsigned long __ht2_background_thread(void* pData)
{
	MSG msg;
	unsigned long ulFlags;
	__XMIT_QUEUE_ELEMENT* rxq_element = NULL;
	__XMIT_QUEUE_ELEMENT* txq_element = NULL;

	while (GetMessage(&msg))
	{
		switch (msg.wCommand)
		{
		case HT2_MESSAGE_RECV:
			/* Do recv side process. */
			while (TRUE)
			{
				__ENTER_CRITICAL_SECTION_SMP(rx_lock, ulFlags);
				
				/* Save maximal rxq length. */
				if (rxq_length > rxq_max_length)
				{
					rxq_max_length = rxq_length;
				}

				if (0 == rxq_length)
				{
					/* queue is empty. */
					BUG_ON(rxq_head || rxq_tail);
					__LEAVE_CRITICAL_SECTION_SMP(rx_lock, ulFlags);
					break;
				}
				/* queue is not empty, fetch one element. */
				rxq_element = rxq_head;
				rxq_head = rxq_element->pNext;
				rxq_length--;
				if (0 == rxq_length)
				{
					/* Last element. */
					rxq_tail = NULL;
				}
				__LEAVE_CRITICAL_SECTION_SMP(rx_lock, ulFlags);
				/* Invoke work routine to process the pkt. */
				__ht2tun_input_work(rxq_element->pGenif, rxq_element->p);
				/* Free pbuf since it's refered. */
				pbuf_free(rxq_element->p);
				/* Release the element. */
				_hx_free(rxq_element);
			}
			break;
		case HT2_MESSAGE_SEND:
			/* Do send side process. */
			while (TRUE)
			{
				__ENTER_CRITICAL_SECTION_SMP(tx_lock, ulFlags);
				if (txq_length > txq_max_length)
				{
					txq_max_length = txq_length;
				}

				if (0 == txq_length)
				{
					/* queue is empty. */
					BUG_ON(txq_head || txq_tail);
					__LEAVE_CRITICAL_SECTION_SMP(tx_lock, ulFlags);
					break;
				}
				/* queue is not empty, fetch one element. */
				txq_element = txq_head;
				txq_head = txq_element->pNext;
				txq_length--;
				if (0 == txq_length)
				{
					/* Last element. */
					txq_tail = NULL;
				}
				__LEAVE_CRITICAL_SECTION_SMP(tx_lock, ulFlags);
				/* Invoke work routine to process the pkt. */
				__ht2tun_output_l3_work(txq_element->pGenif, 
					txq_element->p, 
					txq_element->original_sz,
					NULL);
				/* Free pbuf since it's refered. */
				pbuf_free(txq_element->p);
				/* Release the element. */
				_hx_free(txq_element);
			}
			break;
		case KERNEL_MESSAGE_TERMINAL:
			/* Exit the thread. */
			goto __EXIT;
		default:
			__LOG("[%s]unknown message[%d]\r\n", __func__, msg.wCommand);
			break;
		}
	}

__EXIT:
	_hx_printf("ht2 back thread exit.\r\n");
	return 0;
}

/* Destroy the back thread. */
static void __destroy_ht2_thread(HANDLE back_thread)
{
	MSG msg;

	/* Send a exit message to the thread. */
	msg.wCommand = KERNEL_MESSAGE_TERMINAL;
	SendMessage(back_thread, &msg);

	/* Wait the thread run over. */
	WaitForThisObject(back_thread);

	/* Destroy the thread. */
	DestroyKernelThread(back_thread);
}

/* Add one HHTT VTI into system. */
BOOL ht2tun_add(const char* if_name,
	const char* parent_name,
	const char* address)
{
	ip_addr_t ipaddr, mask, gw;
	__COMMON_NETWORK_ADDRESS commaddr[3];
	__GENERIC_NETIF* pParent = NULL;
	__GENERIC_NETIF* pGenif = NULL;
	__HT2_TUNNEL_OBJECT* ht2_tunnel = NULL;
	BOOL bResult = FALSE;

	BUG_ON((NULL == if_name) || (NULL == parent_name));
	BUG_ON(NULL == address);

	if (strlen(if_name) >= GENIF_NAME_LENGTH)
	{
		/* Name is too long. */
		goto __TERMINAL;
	}
	if (strlen(parent_name) >= GENIF_NAME_LENGTH)
	{
		goto __TERMINAL;
	}
	/* Validate the source and destination address. */
	if (!ipaddr_aton(address, &ipaddr))
	{
		goto __TERMINAL;
	}

	/* Start background thread if not yet. */
	if (NULL == ht2_thread)
	{
		int nAffinity = 0;
		ht2_thread = CreateKernelThread(0,
			KERNEL_THREAD_STATUS_SUSPENDED,
			HT2_BACKTHREAD_PRIO,
			__ht2_background_thread,
			NULL, NULL,
			"ht2_thread");
		if (NULL == ht2_thread)
		{
			__LOG("[%s]can not start back thread.\r\n", __func__);
			goto __TERMINAL;
		}

		/* Schedule the thread to a proper CPU. */
#if defined(__CFG_SYS_SMP)
		nAffinity = GetScheduleCPU();
#endif
		ChangeAffinity(ht2_thread, nAffinity);
		ResumeKernelThread(ht2_thread);
	}

	/* Get the parent interface. */
	pParent = NetworkManager.GetGenifByName(parent_name);
	if (NULL == pParent)
	{
		_hx_printf("[%s]invalid parent name.\r\n", __func__);
		goto __TERMINAL;
	}

	/* create tunnel object and init it. */
	ht2_tunnel = (__HT2_TUNNEL_OBJECT*)_hx_malloc(sizeof(__HT2_TUNNEL_OBJECT));
	if (NULL == ht2_tunnel)
	{
		__LOG("[%s]out of memory for tunnel object.\r\n", __func__);
		goto __TERMINAL;
	}
	memset(ht2_tunnel, 0, sizeof(__HT2_TUNNEL_OBJECT));
	ht2_tunnel->tunnel_id = global_tunnel_id++;
	/* Use parent's address as source. */
	ht2_tunnel->src_addr = pParent->ip_addr[0];
	inet_aton(HT2_SERVER_HOST, &ht2_tunnel->dest_addr);
	ht2_tunnel->src_port = HT2_PORT_CLIENT;
	ht2_tunnel->dest_port = HT2_PORT_SERVER;
	ht2_tunnel->qos = HT2_DEFAULT_QOS;
	ht2_tunnel->ttl = HT2_DEFAULT_TTL;
	ht2_tunnel->session_id = global_session_id++;

	/* Create a new generic interface. */
	pGenif = NetworkManager.CreateGenif(pParent, NULL);
	if (NULL == pGenif)
	{
		_hx_printf("[%s]create new genif failed.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Initialize it. */
	strcpy(pGenif->genif_name, if_name);
	pGenif->link_type = lt_tunnel;
	pGenif->vti_type = vti_ht2;
	pGenif->packet_for_me = ht2_packet_for_me;
	
	pGenif->genif_input = ht2tun_input;
	//pGenif->genif_input = __ht2tun_input_work;	
	pGenif->genif_l3_output = ht2tun_output_l3;
	//pGenif->genif_l3_output = __ht2tun_output_l3_work;
	
	pGenif->genif_l2_output = ht2tun_output_l2;
	pGenif->specific_show = ht2tun_specific_show;
	pGenif->pGenifParent = pParent;
	pGenif->genif_mtu = pParent->genif_mtu - HT2_TOTAL_HEADER_LENGTH;
	/* Save tunnel's source and dest to genif. */
	pGenif->vti_source = ht2_tunnel->src_addr;
	pGenif->vti_destination = ht2_tunnel->dest_addr;

	/* Save ht2 management object. */
	pGenif->pGenifExtension = ht2_tunnel;

	/* Save genif to tunnel object. */
	ht2_tunnel->pGenif = pGenif;

	/* 
	 * Init cipher context. 
	 * Must follow after the initialization of ht2
	 * tunnel object, since it will use genif's
	 * name to load key from storage.
	 */
	if (!__init_cipher_context(ht2_tunnel))
	{
		_hx_printf("[%s]failed to init cipher context.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Register it to system. */
	if (!NetworkManager.RegisterGenif(pGenif))
	{
		_hx_printf("[%s]failed to register genif.\r\n", __func__);
		goto __TERMINAL;
	}

	/* Set it's ip address. */
	ipaddr_aton("255.255.255.255", &mask);
	ipaddr_aton("0.0.0.0", &gw);
	commaddr[0].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	commaddr[0].Address.ipv4_addr = ipaddr.addr;
	commaddr[1].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	commaddr[1].Address.ipv4_addr = mask.addr;
	commaddr[2].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	commaddr[2].Address.ipv4_addr = gw.addr;
	NetworkManager.AddGenifAddress(pGenif->if_index,
		NETWORK_PROTOCOL_TYPE_IPV4,
		commaddr, 3, FALSE);

	/* Increment the tunnel counter. */
	ht2_tun_num++;

	/* Set the interface's status as UP. */
	NetworkManager.LinkStatusChange(pGenif, up, full, _100G);

	/* 
	 * Install a specific route to tunnel's server side, 
	 * make sure the underlay packet of tunnel could be
	 * routed to parent interface.
	 */
	if (ht2_tunnel->dest_addr.addr != IPADDR_ANY)
	{
		ipaddr_aton("255.255.255.255", &mask);
		RoutingManager.add_iproute(&ht2_tunnel->dest_addr, &mask,
			ROUTE_METRIC_STATIC, NULL,
			IP_ROUTE_TYPE_STATIC,
			pParent->genif_name);
	}

	/* Everything is OK. */
	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		/* Destroy  the back thread if created. */
		if (ht2_thread)
		{
			__destroy_ht2_thread(ht2_thread);
			ht2_thread = NULL;
		}

		/* Release tunnel object. */
		if (ht2_tunnel)
		{
			if (ht2_tunnel->cipher_ctx)
			{
				/* Release cipher context. */
				__release_cipher_context(ht2_tunnel);
			}
			_hx_free(ht2_tunnel);
		}
		/* Release the parent if got. */
		if (pParent)
		{
			NetworkManager.ReleaseGenif(pParent);
		}
		/* Release the genif if created. */
		if (pGenif)
		{
			NetworkManager.ReleaseGenif(pGenif);
		}
	}
	return bResult;
}

/* 
 * Change HHTT VTI's attributes. 
 * if_name denotes the genif's name whose attribute(s)
 * will be set or change, att_name/att_value denote
 * the pair of attribute and it's value.
 */
BOOL ht2tun_set(const char* if_name,
	const char* att_name,
	const char* att_value)
{
	__GENERIC_NETIF* pGenif = NULL;
	__GENERIC_NETIF* pParent = NULL;
	__HT2_TUNNEL_OBJECT* ht2_tunnel = NULL;
	ip_addr_t peer_addr, mask_addr;
	uint16_t peer_port;
	BOOL bResult = FALSE;

	BUG_ON((NULL == if_name) || (NULL == att_name) || (NULL == att_value));
	/* Get the genif by it's name. */
	pGenif = NetworkManager.GetGenifByName(if_name);
	if (NULL == pGenif)
	{
		_hx_printf("[%s]no genif with name[%s] found.\r\n", __func__,
			if_name);
		goto __TERMINAL;
	}

	/* Validate the genif, make sure it's a ht2 tunnel. */
	if (pGenif->link_type != lt_tunnel)
	{
		_hx_printf("[%s] is not tunnel.\r\n", if_name);
		goto __TERMINAL;
	}
	if (pGenif->vti_type != vti_ht2)
	{
		_hx_printf("[%s] is not ht2 tunnel.\r\n", if_name);
		goto __TERMINAL;
	}

	/* Get the ht2 tunnel object. */
	ht2_tunnel = (__HT2_TUNNEL_OBJECT*)pGenif->pGenifExtension;
	BUG_ON(NULL == ht2_tunnel);

	/* Parse the attribute and it's value. */
	if (0 == strcmp("peer-addr", att_name))
	{
		/* Set the tunnel's peer address. */
		if (inet_aton(att_value, &peer_addr))
		{
			ht2_tunnel->dest_addr = peer_addr;
			pGenif->vti_destination = peer_addr;
			/* 
			 * Install the route to peer, using
			 * parent genif as out going interface.
			 */
			ipaddr_aton("255.255.255.255", &mask_addr);
			pParent = pGenif->pGenifParent;
			BUG_ON(NULL == pParent);
			RoutingManager.add_iproute(&ht2_tunnel->dest_addr, &mask_addr,
				ROUTE_METRIC_STATIC, NULL,
				IP_ROUTE_TYPE_STATIC,
				pParent->genif_name);
		}
		else{
			_hx_printf("invalid attribute value[%s]\r\n", att_value);
		}
		goto __TERMINAL;
	}
	if (0 == strcmp("peer-port", att_name))
	{
		/* Set tunnel's peer udp port. */
		peer_port = (uint16_t)atol(att_value);
		if (peer_port)
		{
			ht2_tunnel->dest_port = ntohs(peer_port);
		}
		else {
			_hx_printf("invalid attribute value[%s]\r\n", att_value);
		}
		goto __TERMINAL;
	}
	if (0 == strcmp("key-file", att_name))
	{
		/* Set tunnel's cipher key. */
		__load_key_from_file(ht2_tunnel, att_value);
		goto __TERMINAL;
	}

	/* Other attributes changing apped here. */

	/* Unknow attribute name. */
	_hx_printf("invalid attribute name[%s]\r\n", att_name);

__TERMINAL:
	if (pGenif)
	{
		/* Release the genif object. */
		NetworkManager.ReleaseGenif(pGenif);
	}
	return bResult;
}

/* 
 * Helper routine to release one vti's resource. 
 * The routine is not fully tested yet...
 */
static BOOL __ht2_cleanup(__GENERIC_NETIF* pGenif)
{
	__HT2_TUNNEL_OBJECT* ht2_tunnel = NULL;
	unsigned long ulFlags;

	/* Release the ht2 tunnel object. */
	ht2_tunnel = (__HT2_TUNNEL_OBJECT*)pGenif->pGenifExtension;
	BUG_ON(NULL == ht2_tunnel);
	/* 
	 * Clear the genif's extension since 
	 * the tunnel object will be destroyed. 
	 */
	__ENTER_CRITICAL_SECTION_SMP(pGenif->spin_lock, ulFlags);
	pGenif->pGenifExtension = NULL;
	__LEAVE_CRITICAL_SECTION_SMP(pGenif->spin_lock, ulFlags);

	_hx_free(ht2_tunnel);
	/* Decrement the tunnel number counter. */
	ht2_tun_num--;
	if (0 == ht2_tun_num)
	{
		/* Stop the background thread. */
		__destroy_ht2_thread(ht2_thread);
		/* Clear state. */
		ht2_thread = NULL;
	}

	/* Release tx/rx queue. */

	/* Release the genif object. */
	NetworkManager.ReleaseGenif(pGenif);

	return TRUE;
}

/* 
 * Remove one ht2 vti from system. 
 * It releases all resources associated to the
 * specified vti, and stop the background kernel
 * thread if the last one.
 */
BOOL ht2tun_del(const char* if_name)
{
	__GENERIC_NETIF* pGenif = NULL;
	BOOL bResult = FALSE;

	BUG_ON(NULL == if_name);
	/* Get the vti's genif object. */
	pGenif = NetworkManager.GetGenifByName(if_name);
	if (NULL == pGenif)
	{
		_hx_printf("[%s]No genif[%s] found.\r\n", __func__, if_name);
		goto __TERMINAL;
	}

	/* Make sure the genif is a ht2 vti. */
	if ((pGenif->link_type != lt_tunnel) || (pGenif->vti_type != vti_ht2))
	{
		NetworkManager.ReleaseGenif(pGenif);
		goto __TERMINAL;
	}

	/* Mark link status down first. */
	NetworkManager.LinkStatusChange(pGenif, down, full, _1000M);

	/* Clean all associated resources. */
	bResult = __ht2_cleanup(pGenif);
	/* Release the genif since we get it first. */
	NetworkManager.ReleaseGenif(pGenif);

__TERMINAL:
	return bResult;
}
