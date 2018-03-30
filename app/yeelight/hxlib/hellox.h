//***********************************************************************/
//    Author                    : Garry
//    Original Date             : March 10,2018
//    Module Name               : Main header file of HelloX SDK.
//    Module Funciton           : 
//                                Declares all kernel service routines can be
//                                used by other modules in application module.
//                                This file is used by user application,so all
//                                routines declared in it are system calls.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __HELLOX_H__
#define __HELLOX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "__hxcomm.h"

/* General types. */
typedef unsigned char BYTE;
typedef unsigned char UCHAR;
typedef char CHAR;
typedef short WCHAR;
typedef char* LPSTR;
typedef unsigned short WORD;
typedef unsigned short USHORT;
typedef signed short SHORT;
typedef int INT;
typedef unsigned int UINT;
typedef unsigned long DWORD;
typedef unsigned long ULONG;
typedef long LONG;
typedef unsigned long BOOL;
typedef void* LPVOID;
typedef void VOID;

/* 
 * General HANDLE is used to refer any kernel object,such as
 * kernel thread,synchronization object,file,timer,...
 */
typedef LPVOID HANDLE;

												   //If WIDE char is used.
#ifdef __WIDE_CHAR
#define TCHAR WCHAR
#else
#define TCHAR CHAR
#endif

//Declare as wide character or wide char string.
#define WIDE_CHAR L
#define WIDE_STR  L

/*
 * Object signature,used to identify one object.
 */
#define KERNEL_OBJECT_SIGNATURE 0xAA5555AA

//Constants.
#define MAX_DWORD_VALUE    0xFFFFFFFF
#define TRUE               0xFFFFFFFF
#define FALSE              0x00000000
#define NULL               ((void*)0)
#define MAX_FILE_NAME_LEN  512

#define MOUSE_DIM_X   511  //Maximal x dimension of mouse.
#define MOUSE_DIM_Y   511  //y dimension.

//Virtual key maps.
#define ASCII_NUT     0
#define ASCII_SOH     1
#define ASCII_STX     2
#define ASCII_ETX     3
#define ASCII_EOT     4
#define ASCII_ENQ     5
#define ASCII_ACK     6
#define ASCII_BEL     7
#define ASCII_BS      8
#define ASCII_HT      9
#define ASCII_LF      10
#define ASCII_VT      11
#define ASCII_FF      12
#define ASCII_CR      13
#define ASCII_SO      14
#define ASCII_SI      15
#define ASCII_DLE     16
#define ASCII_DC1     17
#define ASCII_DC2     18
#define ASCII_DC3     19
#define ASCII_DC4     20
#define ASCII_NAK     21
#define ASCII_SYN     22
#define ASCII_TB      23
#define ASCII_CAN     24
#define ASCII_EM      25
#define ASCII_SUB     26
#define ASCII_ESC     27
#define ASCII_FS      28
#define ASCII_GS      29
#define ASCII_RS      30
#define ASCII_US      31

#define ASCII_SPACE           32
#define ASCII_EXCALMARK       33   //!
#define ASCII_QUOTATIONMARK   34   //"
#define ASCII_POUND           35   //#
#define ASCII_DOLLAR          36   //$
#define ASCII_PERCENT         37   //%
#define ASCII_QUOTA           38   //&
#define ASCII_MINUTE          39   //'
#define ASCII_LEFT_BRACKET    40   //(
#define ASCII_RIGHT_BRACKET   41   //)
#define ASCII_STAR            42   //*
#define ASCII_ADD             43   //+
#define ASCII_COMMA           44   //,
#define ASCII_SUBSTRACT       45   //-
#define ASCII_DOT             46   //.
#define ASCII_REVERSE_SLASH   47   ///

#define ASCII_0               48
#define ASCII_1               49
#define ASCII_2               50
#define ASCII_3               51
#define ASCII_4               52
#define ASCII_5               53
#define ASCII_6               54
#define ASCII_7               55
#define ASCII_8               56
#define ASCII_9               57
#define ASCII_COLON           58   //:
#define ASCII_SEMICOLON       59   //;
#define ASCII_LESS            60   //<
#define ASCII_EQUAL           61   //=
#define ASCII_GREATER         62   //>
#define ASCII_ASK             63   //?
#define ASCII_AT              64   //@

//
//Virtual keys defined by OS kernel to handle different 
//physical key boards.
//
#define VK_ESC             27
#define VK_RETURN          13
#define VK_TAB             9
#define VK_CAPS_LOCK       20
#define VK_SHIFT           10
#define VK_CONTROL         17
#define VK_MENU            18    //Menu key in windows complying key board.
#define VK_BACKSPACE       8
#define VK_LWIN            91    //Left windows key in windows complying kb.
#define VK_RWIN            92    //Right windows key.
#define VK_INSERT          45
#define VK_HOME            36
#define VK_PAGEUP          33
#define VK_PAGEDOWN        34
#define VK_END             35
#define VK_DELETE          46
#define VK_LEFTARROW       37
#define VK_UPARROW         38
#define VK_RIGHTARROW      39
#define VK_DOWNARROW       40
#define VK_ALT             5     //Alt key,defined by Hello China.

#define VK_F1              112
#define VK_F2              113
#define VK_F3              114
#define VK_F4              115
#define VK_F5              116
#define VK_F6              117
#define VK_F7              118
#define VK_F8              119
#define VK_F9              120
#define VK_F10             121
#define VK_F11             122
#define VK_F12             123

#define VK_NUMLOCK         144
#define VK_NUMPAD0         96
#define VK_NUMPAD1         97
#define VK_NUMPAD2         98
#define VK_NUMPAD3         99
#define VK_NUMPAD4         100
#define VK_NUMPAD5         101
#define VK_NUMPAD6         102
#define VK_NUMPAD7         103
#define VK_NUMPAD8         104
#define VK_NUMPAD9         105
#define VK_DECIMAL         110
#define VK_NUMPADMULTIPLY  106
#define VK_NUMPADADD       107
#define VK_NUMPADSUBSTRACT 109
#define VK_NUMPADDIVIDE    111

#define VK_PAUSE           19
#define VK_SCROLL          145
#define VK_PRINTSC         6    //This value is defined by Hello China.

//System call numbers.
#define SYSCALL_CREATEKERNELTHREAD    0x01     //CreateKernelThread.
#define SYSCALL_DESTROYKERNELTHREAD   0x02     //DestroyKernelThread.
#define SYSCALL_SETLASTERROR          0x03     //SetLastError.
#define SYSCALL_GETLASTERROR          0x04     //GetLastError.
#define SYSCALL_GETTHREADID           0x05     //GetThreadID.
#define SYSCALL_SETTHREADPRIORITY     0x06     //SetThreadPriority.
#define SYSCALL_GETMESSAGE            0x07     //GetMessage.
#define SYSCALL_SENDMESSAGE           0x08     //SendMessage.
#define SYSCALL_SLEEP                 0x09     //Sleep.
#define SYSCALL_SETTIMER              0x0A     //SetTimer.
#define SYSCALL_CANCELTIMER           0x0B     //CancelTimer.
#define SYSCALL_CREATEEVENT           0x0C     //CreateEvent.
#define SYSCALL_DESTROYEVENT          0x0D     //DestroyEvent.
#define SYSCALL_SETEVENT              0x0E     //SetEvent.
#define SYSCALL_RESETEVENT            0x0F     //ResetEvent.
#define SYSCALL_CREATEMUTEX           0x10     //CreateMutex.
#define SYSCALL_DESTROYMUTEX          0x11     //DestroyMutex.
#define SYSCALL_RELEASEMUTEX          0x12     //ReleaseMutex.
#define SYSCALL_WAITFORTHISOBJECT     0x13     //WaitForThisObject.
#define SYSCALL_WAITFORTHISOBJECTEX   0x14     //WaitForThisObjectEx.
#define SYSCALL_CONNECTINTERRUPT      0x15     //ConnectInterrupt.
#define SYSCALL_DISCONNECTINTERRUPT   0x16     //DisconnectInterrupt.
#define SYSCALL_VIRTUALALLOC          0x17     //VirtualAlloc.
#define SYSCALL_VIRTUALFREE           0x18     //VirtualFree.
#define SYSCALL_CREATEFILE            0x19     //CreateFile.
#define SYSCALL_READFILE              0x1A     //ReadFile.
#define SYSCALL_WRITEFILE             0x1B     //WriteFile.
#define SYSCALL_CLOSEFILE             0x1C     //CloseFile.
#define SYSCALL_CREATEDIRECTORY       0x1D     //CreateDirectory.
#define SYSCALL_DELETEFILE            0x1E     //DeleteFile.
#define SYSCALL_FINDFIRSTFILE         0x1F     //FindFirstFile.
#define SYSCALL_FINDNEXTFILE          0x20     //FindNextFile.
#define SYSCALL_FINDCLOSE             0x21     //FindClose.
#define SYSCALL_GETFILEATTRIBUTES     0x22     //GetFileAttributes.
#define SYSCALL_GETFILESIZE           0x23     //GetFileSize.
#define SYSCALL_REMOVEDIRECTORY       0x24     //RemoveDirectory.
#define SYSCALL_SETENDOFFILE          0x25     //SetEndOfFile.
#define SYSCALL_IOCONTROL             0x26     //IOControl.
#define SYSCALL_SETFILEPOINTER        0x27     //SetFilePointer.
#define SYSCALL_FLUSHFILEBUFFERS      0x28     //FlushFileBuffers.
#define SYSCALL_CREATEDEVICE          0x29     //CreateDevice.
#define SYSCALL_DESTROYDEVICE         0x2A     //DestroyDevice.
#define SYSCALL_KMEMALLOC             0x2B     //KMemAlloc.
#define SYSCALL_KMEMFREE              0x2C     //KMemFree.
#define SYSCALL_PRINTLINE             0x2D     //PrintLine.
#define SYSCALL_PRINTCHAR             0x2E     //PrintChar.
#define SYSCALL_REGISTERSYSTEMCALL    0x2F     //RegisterSystemCall.
#define SYSCALL_REPLACESHELL          0x30     //ReplaceShell.
#define SYSCALL_LOADDRIVER            0x31     //LoadDriver.
#define SYSCALL_GETCURRENTTHREAD      0x32     //GetCurrentThread.
#define SYSCALL_GETDEVICE             0x33     //GetDevice.
#define SYSCALL_SWITCHTOGRAPHIC       0x34     //SwitchToGraphic.
#define SYSCALL_SWITCHTOTEXT          0x35     //SwitchToText.
#define SYSCALL_SETFOCUSTHREAD        0x36
#define SYSCALL_GETETHERNETINTERFACESTATE 0x37 //GetEthernetInterfaceState.
#define SYSCALL_GETSYSTEMTIME         0x38
#define SYSCALL_GOTOHOME              0x39
#define SYSCALL_CHANGELINE            0x3A
#define SYSCALL_GETCURSORPOS          0x3B
#define SYSCALL_SETCURSORPOS          0x3C
#define SYSCALL_TERMINATEKERNELTHREAD 0x3D
#define SYSCALL_GOTOPREV              0x3E
#define SYSCALL_PEEKMESSAGE           0x3F

/* System call IDs for socket API. */
#define SYSCALL_SOCKET                0x200     //Socket
#define SYSCALL_BIND                  0x201     //bind
#define SYSCALL_CONNECT               0x202     //connect
#define SYSCALL_RECV                  0x203     //recv
#define SYSCALL_SEND                  0x204     //send
#define SYSCALL_WRITE                 0x205     //write
#define SYSCALL_SELECT                0x206     //select
#define SYSCALL_GETSOCKET             0x207     //getsocktopt
#define SYSCALL_SETSOCKET             0x208     //setsocketopt
#define SYSCALL_GETHOSTBYNAME         0x209     //gethostbyname
#define SYSCALL_ACCEPT                0x20A     //accept
#define SYSCALL_CLOSESOCKET           0x20B     //close socket
#define SYSCALL_LISTEN                0x20C     //listen
#define SYSCALL_RECVFROM              0x20D     //recv from
#define SYSCALL_SENDTO                0x20E     //send to

												   //The following macros are defined to simply the programming.
#ifdef __GCC__
												   //AT&T syntax
#define SYSCALL_PARAM_0(num) 	 \
		{                			 \
	__asm__ __volatile__(		 \
		".code32 			\n\t"   \
		"pushl		$0		\n\t"   \
		"pushl		%0		\n\t"   \
		"int		$0x7F	\n\t"   \
		"popl		%%eax	\n\t"   \
		"popl		%%eax	\n\t"   \
		: :"r"(num): );             \
		}

#define SYSCALL_PARAM_1(num,p1) 		\
		{                					\
	__asm__ __volatile__ (                \
		".code32			\n\t"            \
		"pushl 	%2			\n\t"            \
		"pushl	$0			\n\t"            \
		"pushl	%1			\n\t"            \
		"int	$0x7F		\n\t"            \
		"popl	%%eax		\n\t"            \
		"popl	%%eax		\n\t"            \
		"popl	%0			\n\t"            \
		:"=r"(p1): "r"(num), "r"(p1):);    \
		}
#define SYSCALL_PARAM_2(num,p1,p2) \
		{               									 	 \
	__asm__ __volatile__ (                                   \
		".code32				\n\t"                           \
		"pushl	%4				\n\t"                           \
		"pushl	%3				\n\t"                           \
		"pushl	$0				\n\t"                           \
		"pushl	%2				\n\t"                           \
		"int	$0x7F			\n\t"                           \
		"popl	%%eax			\n\t"                           \
		"popl	%%eax			\n\t"                           \
		"popl	%0				\n\t"                           \
		"popl	%1				\n\t"                           \
		:"=r"(p1),"=r"(p2) :"r"(num), "r"(p1),"r"(p2):);       \
		}

#define SYSCALL_PARAM_3(num,p1,p2,p3) \
		{                	                                               \
	__asm__ __volatile__(                                              \
		".code32				\n\t"                                     \
		"pushl	%6				\n\t"                                     \
		"pushl	%5				\n\t"                                     \
		"pushl	%4				\n\t"                                     \
		"pushl	$0				\n\t"                                     \
		"pushl	%3				\n\t"                           		   \
		"int	$0x7F			\n\t"                                     \
		"popl	%%eax			\n\t"                                     \
		"popl	%%eax			\n\t"                                     \
		"popl	%0				\n\t"                                     \
		"popl	%1				\n\t"                                     \
		"popl	%2				\n\t"                                     \
		:"=r"(p1),"=r"(p2),"=r"(p3): "r"(num), "r"(p1),"r"(p2),"r"(p3): );\
		}

#define SYSCALL_PARAM_4(num,p1,p2,p3,p4) \
		{                \
	__asm__ __volatile__(                                              \
		".code32				\n\t"                                     \
		"pushl	%8				\n\t"                                     \
		"pushl	%7				\n\t"                                     \
		"pushl	%6				\n\t"                                     \
		"pushl	%5				\n\t"                                     \
		"pushl	$0				\n\t"                                     \
		"pushl	%4				\n\t"                                     \
		"int	$0x7F			\n\t"                                     \
		"popl	%%eax			\n\t"                                     \
		"popl	%%eax			\n\t"                                     \
		"popl	%0				\n\t"                                     \
		"popl	%1				\n\t"                                     \
		"popl	%2				\n\t"                                     \
		"popl	%3				\n\t"                                     \
		:"=r"(p1),"=r"(p2),"=r"(p3),"=r"(p4) :"r"(num), "r"(p1),"r"(p2),"r"(p3),"r"(p4): );\
}

#define SYSCALL_PARAM_5(num,p1,p2,p3,p4,p5) 							\
		{                													\
	__asm__ __volatile__(                                              \
		".code32				\n\t"                                     \
		"pushl	%10				\n\t"                                     \
		"pushl	%9  			\n\t"                                     \
		"pushl	%8				\n\t"                                     \
		"pushl	%7				\n\t"                                     \
		"pushl	%6				\n\t"                                     \
		"pushl	$0				\n\t"                                     \
		"pushl	%5				\n\t"                                     \
		"int	$0x7F			\n\t"                                     \
		"popl	%%eax			\n\t"                                     \
		"popl	%%eax			\n\t"                                     \
		"popl	%0				\n\t"                                     \
		"popl	%1				\n\t"                                     \
		"popl	%2				\n\t"                                     \
		"popl	%3				\n\t"                                     \
		"popl	%4				\n\t"                                     \
		:"=r"(p1),"=r"(p2),"=r"(p3),"=r"(p4),"=r"(p5) :"r"(num), "r"(p1),"r"(p2),"r"(p3),"r"(p4),"r"(p5) : );\
		}

#define SYSCALL_PARAM_6(num,p1,p2,p3,p4,p5,p6) \
		{                														\
	__asm__ __volatile__(                                              	   \
			".code32				\n\t"                                     \
			"pushl	%12				\n\t"                                     \
			"pushl	%11				\n\t"                                     \
			"pushl	%10  			\n\t"                                     \
			"pushl	%9  			\n\t"                                     \
			"pushl	%8				\n\t"                                     \
			"pushl	%7				\n\t"                                     \
			"pushl	$0				\n\t"                                     \
			"pushl	%6				\n\t"                                     \
			"int	$0x7F			\n\t"                                     \
			"popl	%%eax			\n\t"                                     \
			"popl	%%eax			\n\t"                                     \
			"popl	%0				\n\t"                                     \
			"popl	%1				\n\t"                                     \
			"popl	%2				\n\t"                                     \
			"popl	%3				\n\t"				                       \
			"popl	%4				\n\t"                                     \
			"popl	%5				\n\t"                                     \
			:"=r"(p1),"=r"(p2),"=r"(p3),"=r"(p4),"=r"(p5),"=r"(p6)		   \
			:"r"(num), "r"(p1),"r"(p2),"r"(p3),"r"(p4),"r"(p5),"r"(p6)	);\
		}

#define SYSCALL_PARAM_7(num,p1,p2,p3,p4,p5,p6,p7) \
		{                														\
	__asm__ __volatile__(                                              	   \
			".code32				\n\t"                                     \
			"pushl	%14				\n\t"                                     \
			"pushl	%13				\n\t"                                     \
			"pushl	%12				\n\t"                                     \
			"pushl	%11				\n\t"                                     \
			"pushl	%10  			\n\t"                                     \
			"pushl	%9  			\n\t"                                     \
			"pushl	%8				\n\t"                                     \
			"pushl	$0				\n\t"                                     \
			"pushl	%7				\n\t"                                     \
			"int	$0x7F			\n\t"                                     \
			"popl	%%eax			\n\t"                                     \
			"popl	%%eax			\n\t"                                     \
			"popl	%0				\n\t"                                     \
			"popl	%1				\n\t"                                     \
			"popl	%2				\n\t"                                     \
			"popl	%3				\n\t"				                       \
			"popl	%4				\n\t"                                     \
			"popl	%5				\n\t"                                     \
			"popl	%6				\n\t"                                     \
			:"=r"(p1), "=r"(p2), "=r"(p3), "=r"(p4), "=r"(p5), "=r"(p6), "=m"(p7)	\
			:"r"(num), "r"(p1), "r"(p2), "r"(p3), "r"(p4), "r"(p5), "r"(p6), "m"(p7)	);\
		}

#else /* Intel syntax. */
#define SYSCALL_PARAM_0(num) \
		{                \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
		}

#define SYSCALL_PARAM_1(num,p1) \
		{                \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
		}

#define SYSCALL_PARAM_2(num,p1,p2) \
		{                \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
		}

#define SYSCALL_PARAM_3(num,p1,p2,p3) \
		{                \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
		}

#define SYSCALL_PARAM_4(num,p1,p2,p3,p4) \
		{                \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
		}

#define SYSCALL_PARAM_5(num,p1,p2,p3,p4,p5) \
		{                \
	__asm push p5    \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	__asm pop p5     \
		}

#define SYSCALL_PARAM_6(num,p1,p2,p3,p4,p5,p6) \
		{                \
	__asm push p6    \
	__asm push p5    \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	__asm pop p5     \
	__asm pop p6     \
		}

#define SYSCALL_PARAM_7(num,p1,p2,p3,p4,p5,p6,p7) \
		{                \
	__asm push p7    \
	__asm push p6    \
	__asm push p5    \
	__asm push p4    \
	__asm push p3    \
	__asm push p2    \
	__asm push p1    \
	__asm push 0     \
	__asm push num   \
	__asm int 0x7F   \
	__asm pop eax    \
	__asm pop eax    \
	__asm pop p1     \
	__asm pop p2     \
	__asm pop p3     \
	__asm pop p4     \
	__asm pop p5     \
	__asm pop p6     \
	__asm pop p7     \
		}
#endif

/* Kernel thread message. */
typedef struct _MSG {
	WORD    wCommand;
	WORD    wParam;
	DWORD   dwParam;
}MSG;

typedef struct {
	DWORD dwHighDateTime;
	DWORD dwLowDateTime;
} __FILE_TIME;

typedef struct {
	DWORD           dwFileAttribute;
	__FILE_TIME     ftCreationTime;    //-------- CAUTION!!! --------
	__FILE_TIME     ftLastAccessTime;  //-------- CAUTION!!! --------
	__FILE_TIME     ftLastWriteTime;   //-------- CAUTION!!! --------
	DWORD           nFileSizeHigh;
	DWORD           nFileSizeLow;
	DWORD           dwReserved0;
	DWORD           dwReserved1;
	CHAR            cFileName[MAX_FILE_NAME_LEN];
	CHAR            cAlternateFileName[13];
} FS_FIND_DATA;

typedef DWORD(*__KERNEL_THREAD_ROUTINE)(LPVOID);             //Kernel thread's start routine.
typedef DWORD(*__DIRECT_TIMER_HANDLER)(LPVOID);              //Timer handler's protype.
typedef BOOL(*__INTERRUPT_HANDLER)(LPVOID lpEsp, LPVOID);    //Interrupt handler's pro-type.

/* Kernel thread's status value. */
#define KERNEL_THREAD_STATUS_RUNNING    0x00000001
#define KERNEL_THREAD_STATUS_READY      0x00000002
#define KERNEL_THREAD_STATUS_SUSPENDED  0x00000003
#define KERNEL_THREAD_STATUS_SLEEPING   0x00000004
#define KERNEL_THREAD_STATUS_TERMINAL   0x00000005
#define KERNEL_THREAD_STATUS_BLOCKED    0x00000006

/* Kernel thread's priority level value. */
#define PRIORITY_LEVEL_CRITICAL         0x00000010
#define PRIORITY_LEVEL_HIGH             0x0000000C
#define PRIORITY_LEVEL_NORMAL           0x00000008
#define PRIORITY_LEVEL_LOW              0x00000004
#define PRIORITY_LEVEL_IDLE             0x00000000

/* Kernel thread related operations. */
HANDLE CreateKernelThread(DWORD dwStackSize,
	DWORD dwInitStatus,
	DWORD dwPriority,
	__KERNEL_THREAD_ROUTINE lpStartRoutine,
	LPVOID lpRoutineParam,
	LPVOID lpReserved,
	LPSTR  lpszName);
VOID DestroyKernelThread(HANDLE hThread);
DWORD SetLastError(DWORD dwNewError);
DWORD GetLastError(void);
DWORD GetThreadID(HANDLE hThread);
DWORD SetThreadPriority(HANDLE hThread, DWORD dwPriority);
VOID TerminateKernelThread(HANDLE hThread, int status);
HANDLE GetCurrentThread(void);

//Flags for GetMessage,especially used by wCommand member of
//MSG structure.
#define KERNEL_MESSAGE_AKDOWN         1        //ASCII key down.
#define KERNEL_MESSAGE_AKUP           2        //ASCII key up.
#define KERNEL_MESSAGE_VKDOWN         203
#define KERNEL_MESSAGE_VKUP           204
#define KERNEL_MESSAGE_TERMINAL       5
#define KERNEL_MESSAGE_TIMER          6
#define KERNEL_MESSAGE_LBUTTONDOWN    301      //Left button down.
#define KERNEL_MESSAGE_LBUTTONUP      302      //Left button released.
#define KERNEL_MESSAGE_RBUTTONDOWN    303      //Right button down.
#define KERNEL_MESSAGE_RBUTTONUP      304      //Right button released.
#define KERNEL_MESSAGE_LBUTTONDBCLK   305      //Left button double clicked.
#define KERNEL_MESSAGE_RBUTTONDBCLK   306      //Right button double clicked.
#define KERNEL_MESSAGE_MOUSEMOVE      307      //Mouse is moving.
#define KERNEL_MESSAGE_WINDOW         308      //Window message,defined for GUI module.
#define KERNEL_MESSAGE_DIALOG         309      //Kernel message for dialog processing.

/* Start message ID of user defined message. */
#define KERNEL_MESSAGE_USER           1024

/* Kernel thread message operations. */
BOOL GetMessage(MSG* lpMsg);
BOOL SendMessage(HANDLE hThread, MSG* lpMsg);
BOOL PeekMessage(MSG* lpMsg);

/* Sleep a specified time. */
BOOL Sleep(DWORD dwMillionSecond);

/* Flags to control the SetTimer's action. */
/* 
 * Set a timer with this flags,the timer only
 * apply once,i.e,the kernel thread who set
 * the timer can receive timer message only
 * once.
 */
#define TIMER_FLAGS_ONCE        0x00000001    

 /* 
  * Set a timer with this flags,the timer will
  * availiable always,only if the kernel thread
  * cancel the timer by calling CancelTimer.
  */
#define TIMER_FLAGS_ALWAYS      0x00000002

/* Setup a new timer. */
HANDLE SetTimer(DWORD dwTimerID,
	DWORD dwMillionSecond,
	__DIRECT_TIMER_HANDLER lpHandler,
	LPVOID lpHandlerParam,
	DWORD dwTimerFlags);
VOID CancelTimer(HANDLE hTimer);

/* Synchronization object operations. */
HANDLE CreateEvent(BOOL bInitialStatus);
VOID DestroyEvent(HANDLE hEvent);
DWORD SetEvent(HANDLE hEvent);
DWORD ResetEvent(HANDLE hEvent);
HANDLE CreateMutex(void);
VOID DestroyMutex(HANDLE hMutex);
DWORD ReleaseMutex(HANDLE hEvent);
DWORD WaitForThisObject(HANDLE hObject);
DWORD WaitForThisObjectEx(HANDLE hObject, DWORD dwMillionSecond);

/* 
 * Connect or disconnect interrupt operations,mainly invoked by 
 * hardware device driver.
 */
HANDLE ConnectInterrupt(__INTERRUPT_HANDLER lpInterruptHandler,
	LPVOID              lpHandlerParam,
	UCHAR               ucVector);
VOID DisconnectInterrupt(HANDLE hInterrupt);

//
//Flags to control the action of VirtualAlloc.
//
//Virtual area's access flags.
//
#define VIRTUAL_AREA_ACCESS_READ        0x00000001    //Read access.
#define VIRTUAL_AREA_ACCESS_WRITE       0x00000002    //Write access.
#define VIRTUAL_AREA_ACCESS_RW          (VIRTUAL_AREA_ACCESS_READ | VIRTUAL_AREA_ACCESS_WRITE)
#define VIRTUAL_AREA_ACCESS_EXEC        0x00000008    //Execute only.
#define VIRTUAL_AREA_ACCESS_NOACCESS    0x00000010    //Access is denied.

//
//Virtual area's cache flags.
//
#define VIRTUAL_AREA_CACHE_NORMAL       0x00000001    //Cachable.
#define VIRTUAL_AREA_CACHE_IO           0x00000002    //Not cached.
#define VIRTUAL_AREA_CACHE_VIDEO        0x00000004

//
//Virtual area's allocate flags.
//
#define VIRTUAL_AREA_ALLOCATE_RESERVE   0x00000001    //Reserved only.
#define VIRTUAL_AREA_ALLOCATE_COMMIT    0x00000002    //Has been committed.
#define VIRTUAL_AREA_ALLOCATE_IO        0x00000004    //Allocated as IO mapping area.
#define VIRTUAL_AREA_ALLOCATE_ALL       0x00000008    //Committed.Only be used by VirtualAlloc
//routine,the dwAllocFlags variable of
//virtual area descriptor never set this
//value.
#define VIRTUAL_AREA_ALLOCATE_DEFAULT   VIRTUAL_AREA_ALLOCATE_ALL

LPVOID VirtualAlloc(LPVOID lpDesiredAddr,
	DWORD  dwSize,
	DWORD  dwAllocateFlags,
	DWORD  dwAccessFlags,
	CHAR*  lpszRegName);
VOID VirtualFree(LPVOID lpVirtualAddr);

//Flags used to control CreateFile's action.
#define FILE_ACCESS_READ         0x00000001    //Read access.
#define FILE_ACCESS_WRITE        0x00000002    //Write access.
#define FILE_ACCESS_READWRITE    0x00000003    //Read and write access.
#define FILE_ACCESS_CREATE       0x00000004    //Create a new file.

#define FILE_OPEN_ALWAYS         0x80000000    //If can not open one,create a new one then open it.
#define FILE_OPEN_NEW            0x40000000    //Create a new file,overwrite existing if.
#define FILE_OPEN_EXISTING       0x20000000    //Open a existing file,return fail if does not exist.

HANDLE CreateFile(LPSTR lpszFileName,
	DWORD dwAccessMode,
	DWORD dwShareMode,
	LPVOID lpReserved);
BOOL ReadFile(HANDLE hFile,
	DWORD dwReadSize,
	LPVOID lpBuffer,
	DWORD* lpdwReadSize);
BOOL WriteFile(HANDLE hFile,
	DWORD dwWriteSize,
	LPVOID lpBuffer,
	DWORD* lpdwWrittenSize);
VOID CloseFile(HANDLE hFile);
BOOL CreateDirectory(LPSTR lpszDirName);
BOOL DeleteFile(LPSTR lpszFileName);
HANDLE FindFirstFile(LPSTR lpszDirName,
	FS_FIND_DATA* pFindData);
BOOL FindNextFile(LPSTR lpszDirName,
	HANDLE hFindHandle,
	FS_FIND_DATA* pFindData);
VOID FindClose(LPSTR lpszDirName,
	HANDLE hFindHandle);

//File attributes.
#define FILE_ATTR_READONLY    0x01
#define FILE_ATTR_HIDDEN      0x02
#define FILE_ATTR_SYSTEM      0x04
#define FILE_ATTR_VOLUMEID    0x08
#define FILE_ATTR_DIRECTORY   0x10
#define FILE_ATTR_ARCHIVE     0x20

DWORD GetFileAttributes(LPSTR lpszFileName);
DWORD GetFileSize(HANDLE hFile, DWORD* lpdwSizeHigh);
BOOL RemoveDirectory(LPSTR lpszDirName);
BOOL SetEndOfFile(HANDLE hFile);
BOOL IOControl(HANDLE hFile,
	DWORD dwCommand,
	DWORD dwInputLen,
	LPVOID lpInputBuffer,
	DWORD dwOutputLen,
	LPVOID lpOutputBuffer,
	DWORD* lpdwFilled);

//Flags to control SetFilePointer.
#define FILE_FROM_BEGIN        0x00000001
#define FILE_FROM_CURRENT      0x00000002

BOOL SetFilePointer(HANDLE hFile,
	DWORD* lpdwDistLow,
	DWORD* lpdwDistHigh,
	DWORD dwMoveFlags);
BOOL FlushFileBuffers(HANDLE hFile);
HANDLE CreateDevice(LPSTR lpszDevName,
	DWORD dwAttributes,
	DWORD dwBlockSize,
	DWORD dwMaxReadSize,
	DWORD dwMaxWriteSize,
	LPVOID lpDevExtension,
	HANDLE hDrvObject);
VOID DestroyDevice(HANDLE hDevice);

/* Kernel memory operations. */
/* Flags to tell kernel where to allocate the requested memory. */
#define KMEM_SIZE_TYPE_ANY       0x00000001
#define KMEM_SIZE_TYPE_1K        0x00000002
#define KMEM_SIZE_TYPE_4K        0x00000003

/* 
 * Allocate or release a piece of kernel memory. 
 * This is the underlay support routine for main/free in C
 * library.
 */
LPVOID KMemAlloc(DWORD dwSize, DWORD dwSizeType);
VOID KMemFree(LPVOID lpMemAddr, DWORD dwSizeType, DWORD dwMemLength);

/* Console IO operations. */
VOID PrintLine(LPSTR lpszInfo);
VOID PrintChar(WORD ch);
VOID GotoHome();
VOID ChangeLine();
VOID GotoPrev();
VOID GetCursorPos(WORD*, WORD*);
VOID SetCursorPos(WORD, WORD);
BOOL SwitchToGraphic(void);
VOID SwitchToText(void);
HANDLE SetFocusThread(HANDLE hNewThread);

/* System call related operations. */
typedef BOOL(*__SYSCALL_DISPATCH_ENTRY)(LPVOID, LPVOID);
BOOL RegisterSystemCall(DWORD dwStartSyscallNum, DWORD dwEndSyscallNum,
	__SYSCALL_DISPATCH_ENTRY sde);
BOOL ReplaceShell(__KERNEL_THREAD_ROUTINE shell);

/* Run a driver. */
typedef BOOL(*__DRIVER_ENTRY)(HANDLE hDriverObject);
BOOL LoadDriver(__DRIVER_ENTRY de);

//Critical section operations.
#define __ENTER_CRITICAL_SECTION(obj,flag) {flag = 0;}
#define __LEAVE_CRITICAL_SECTION(obj,flag)

/* Fetch system time from kernel. */
VOID GetSystemTime(BYTE* time);

/* Parameters that transfered to the application. */
#define CMD_PARAMETER_COUNT   16             //The max count of parameter.
#define CMD_PARAMETER_LEN     64             //The max length of one parameter.
#define CMD_MAX_LEN           64             //The max length of one command input. 

/* Command buffer's length. */
#define CMD_BUFFER_LENGTH     (CMD_PARAMETER_COUNT * (CMD_PARAMETER_LEN + 1))

/* 
 * Get command line's input from user,the argc gives how many
 * parameter(s) the caller want,and argv is an array to contain
 * the returned parameter(s).
 * The return value denotes the actual number of parameters
 * that returned,0 means nothing returned.
 */
int getcmd(char** argv, int argc);

/*
 * Debugging facility used in Kernel,now port to application library.
 */
VOID __BUG(LPSTR, DWORD);
#define BUG() \
	__BUG(__FILE__,__LINE__)

/* Conditional BUG call. */
#define BUG_ON(cond) \
	if(cond) \
				{ \
	BUG(); \
		}while(0)

/* Logging facilities. */
/* Show out log header information. */
void LogHeader(int level);

/*
* Declared in stdio.h file,which include this file.So just
* declare here instead including the stdio.h file,which may
* lead incursive including...
*/
extern int _hx_printf(const char* fmt, ...);

/* Macro to print out log information. */
#define __LOG LogHeader(0); _hx_printf

#ifdef __cplusplus
}
#endif

#endif //__HELLOX_H__
