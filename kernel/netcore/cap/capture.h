//***********************************************************************/
//    Author                    : Garry.Xin
//    Original Date             : Oct 02, 2023
//    Module Name               : capture.h
//    Module Funciton           : 
//                                Common header file for packet capture
//                                functions.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#ifndef __CAPTURE_H__
#define __CAPTURE_H__

#include "pcap.h"

/* Notify the back thread to process new packet. */
#define MSG_CAPTURE_ARRIVAL MSG_USER_START
#define MSG_CAPTURE_START   (MSG_USER_START + 1)
#define MSG_CAPTURE_STOP    (MSG_USER_START + 2)

/* Initializer of packet capturing function. */
BOOL CaptureInitialize();

/* Start capture on the specified genif. */
void CaptureStart(int genif_index);

/* Stop capture on the specified genif. */
void CaptureStop(int genif_index);

#endif //__CAPTURE_H__
