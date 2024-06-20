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

/* For APIC and interrupt controller object. */
#include "apic.h"

/*
 * Debugging feature of HelloX's spin lock in SMP.
 * If a kernel thread spins for a spin lock for so many
 * times,it may lead by deadlock,so we will give up and
 * show useful information,then halt.
 */
#define SPIN_LOCK_DEBUG_MAX_SPINTIMES (65536 * 4096)

/* CPU feature flags. */
#define CPU_FEATURE_MASK_HTT (1 << 28)

/* Get CPU supported feature flags. */
uint32_t GetCPUFeature();

/* x86 chip specific information. */
typedef struct tag__X86_CHIP_SPECIFIC {
	unsigned long ioapic_base; /* Base address of IOAPIC. */
	unsigned int global_intbase; /* Global interrupt base of IOAPIC. */
	unsigned long lapic_base;  /* Base address of local APIC. */
}__X86_CHIP_SPECIFIC;

/* Initialize the IOAPIC controller. */
BOOL Init_IOAPIC();

/* 
 * Initialize the local APIC controller,it will be invoked by
 * BSP and each AP processors.
 */
BOOL Init_LocalAPIC();

/* Start all application processors. */
BOOL Start_AP();

/* Stop all application processors. */
BOOL Stop_AP();

/* 
 * CAS(Compare and Swap) intrutions under x86 SMP. 
 * Compare [dest] with old_val, if equal then load [dest] with new_val and
 * return the old_val, otherwise return the new_val.
 */
inline uint32_t __compare_and_swap(volatile uint32_t* dest, uint32_t new_val, uint32_t old_val)
{
	uint32_t ret_val = old_val;

	__asm {
		push ebx
		push edx
		mov ebx, dest
		mov eax, old_val
		mov edx, new_val
		lock cmpxchg dword ptr[ebx], edx
		mov ret_val, eax
		pop edx
		pop ebx
	}
	return ret_val;
}

#endif  //__SMPX86_H__
