//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Oct 02, 2023
//    Module Name               : pcap.h
//    Module Funciton           : 
//                                Common header file for packet capture
//                                functions, it's the duplicated version
//                                of pcap.h under windows/linux.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __PCAP_H__
#define __PCAP_H__

#include <config.h>
#include <TYPES.H>

/* Constants for pcap file, endian dependent. */
#if defined(__CFG_CPU_LE)
#define PCAP_MAGIC_NUMBER 0xA1B2C3D4
#define PCAP_VERSION_MAJOR 0x0002
#define PCAP_VERSION_MINOR 0x0004
#define PCAP_LINKTYPE_ETHERNET 0x00000001
#else
#define PCAP_MAGIC_NUMBER 0xD4C3B2A1
#define PCAP_VERSION_MAJOR 0x0200
#define PCAP_VERSION_MINOR 0x0400
#define PCAP_LINKTYPE_ETHERNET 0x01000000
#endif

/* Structures for pcap file,must be packed. */
#pragma pack(push, 1)
/* pcap global header. */
typedef struct tag__pcap_global_header {
	__u32 magic_number;
	__u16 version_major;
	__u16 version_minor;
	__s32 thiszone;
	__u32 sigfigs;
	__u32 snaplen;
	__u32 linktype;
}pcap_global_header;

/* header for a captured packet. */
typedef struct tag__pcap_packet_header {
	__u32 ts_sec;
	__u32 ts_usec;
	__u32 incl_len;
	__u32 orig_len;
}pcap_packet_header;

#pragma pack(pop)

#endif //__PCAP_H__
