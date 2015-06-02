//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jul,26 2005
//    Module Name               : SYN_MECH.H
//    Module Funciton           : 
//                                This module countains synchronization code for system kernel.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __SYN_MECH_H__
#define __SYN_MECH_H__

#ifdef __cplusplus
extern "C" {
#endif

//System level critical section operations definition.


#ifdef __I386__
#define __ENTER_CRITICAL_SECTION(lpObj,dwFlags)
#ifdef _POSIX_					\
	__asm__ __volatile__(".code32; pushl %%eax; popl %%eax; movl %%eax,%0; popl %%eax; cli;" : :"r"(dwFlags): "memory");
#else							\
		__asm {					 \
			push eax			 \
			pushfd               \
			pop eax              \
			mov dwFlags,eax      \
			pop eax              \
			cli                  \
		}
#endif

/***
 *
        __asm push eax             \
        __asm pushfd               \
		__asm pop eax              \
		__asm mov dwFlags,eax      \
        __asm pop eax              \
        __asm cli
 */

#else
#define __ENTER_CRITICAL_SECTION(lpObj,dwFlags) \
	(dwFlags = 0);
#endif

#ifdef __I386__
#define __LEAVE_CRITICAL_SECTION(lpObj,dwFlags)
#ifdef _POSIX_		\
		__asm__ __volatile__ (".code32; pushl %0; popfd"::"r"(dwFlags):"memory");
#else
    __asm {				\
		push dwFlags 	\
		popfd			\
	}
#endif

#else
#define __LEAVE_CRITICAL_SECTION(lpObj,dwFlags) \
	(dwFlags = 0);
#endif

//Interrupt enable and disable operation.
#ifdef __I386__
	#define __ENABLE_INTERRUPT() \
	{    \
		__asm push eax    \
		__asm mov al, 0x20    \
		__asm out 0x20, al    \
		__asm out 0xa0, al    \
		__asm pop eax    \
		__asm sti    \
	}
#else
#define __ENABLE_INTERRUPT()
#endif

#ifdef __I386__
#define __DISABLE_INTERRUPT() {__asm cli}
#else
#define __DISABLE_INTERRUPT()
#endif

//
//This macros is used to flush cache's content to memory.
//
#ifdef __I386__
#define FLUSH_CACHE()  \
	__asm wbinvd
#else
#define FLUSH_CACHE
#endif

#define SYNCHRONIZE_MEMORY() FLUSH_CACHE()

//
//This macros is used as a barrier.
//In some CPU,the write operations may not be committed into memory or cache,but 
//be queued in WRITE BUFFER on the CPU,so,in order to commit the write operation
//immediately,the following macro should be used.
//For example,the following code:
// ... ...
// WriteMemory(0xCFFFFFFF,90);
// WriteMemory(0xCFFFFFFC,90);
// ... ...
//0xCFFFFFFF and 0xCFFFFFFC are device mapped control port,in order to commit the
//writing operations into device immediately,the following macro must be called.
//
#ifdef __I386__
#define BARRIER() \
	__asm LOCK add dword ptr [esp],0
#else
#define BARRIER()
#endif


typedef unsigned long __ATOMIC_T;

#define __INIT_ATOMIC(t) (t) = 0

#ifdef __cplusplus
}
#endif

#endif    //__SYN_MECH_H__
