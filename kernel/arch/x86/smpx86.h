//***********************************************************************/
//    Author                    : Garry
//    Original Date             : July 7,2018
//    Module Name               : smpx86.h
//    Module Funciton           : 
//                                Symmentric Multiple Processor related constants,
//                                structures and routines,for x86 architecture.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SMPX86_H__
#define __SMPX86_H__

/* For common SMP constants and structures. */
#include <smp.h>

/* CPU feature flags. */
#define CPU_FEATURE_MASK_HTT (1 << 28)

/* Get CPU supported feature flags. */
uint32_t GetCPUFeature();

/* Get local processor's ID. */
unsigned int __GetProcessorID();

#endif  //__SMPX86_H__