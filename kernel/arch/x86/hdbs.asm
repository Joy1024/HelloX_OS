BITS   16   ; ����16λ���������32λ����
SECTION  .TEXT   ; �����
;ORG   7C00H  ; ָ������װ���ڴ����ʼλ��
ORG 0000H

;====================================================================
; 
; ��ͳ�������
; 
;====================================================================
?     EQU  0  ; NASM��֧��DW ?�������﷨������ʹ�������Ķ���
        ; ģ�⣬��ʹ����Ŀɶ��Ը�ǿ
;DATA_BUF_SEG EQU  0200H ; ���ڶ�ȡ��Ŀ¼���ļ����ݵĻ�����(8K) �ε�ַ
;DATA_BUF_OFF EQU  2000H
DATA_BUF_OFF EQU  3000H  ; Modified by Garry.
;STACK_ADDR  EQU  7BD0H ; ��ջջ��(ע�⣺��ջ��СԼΪ20K) 
OSLOADER_ADDR EQU  8000H ; FDOSLDR.BIN�����ڴ��е���ʼλ�ã������ζ��
        ; װ�س��������Դ�ĳߴ粻�ܳ���608K
        ; 8000H - A000H (32K - 640K )
;OSLOADER_SEG EQU  0800H ; ��ʼ�ε�ַ     
OSLOADER_SEG EQU 0100H  ; Start address of OS IMAGE,modified by Garry.
SECOND_SECTOR EQU  03H  ; �ڶ�������������������(���ĸ�����)
SECOND_ADDR  EQU  7E00H ; �ڶ�������������װ��λ��

;====================================================================
; �ö�ջ���������м����( SS = 0 BP = 7C00H )
;====================================================================
FAT_START_SECTOR EQU  4  ; FAT�����ʼ������  DWORD
ROOT_START_SECTOR EQU  8  ; ��Ŀ¼����ʼ������ DWORD
DATA_START_SECTOR EQU  12  ; ��������ʼ������  DWORD
FAT_ENTRY_SECTORS EQU  14  ; FAT����ռ��������  WORD
ROOT_ENTRY_SECTORS EQU  16  ; ��Ŀ¼��ռ�������� WORD
DIR_PER_SECTOR  EQU  17  ; ÿ�����������ɵ�Ŀ¼ BYTE
DISK_EXT_SUPPORT EQU  18     ; �����Ƿ�֧����չBIOS BYTE
CURRENT_CLUSTER  EQU  40  ; ��ǰ���ڴ���Ĵغ� DWORD


;====================================================================  
; ��չ���̷�����ʹ�õĵ�ַ��
;====================================================================
DAP_SECTOR_HIGH  EQU  24  ; ��ʼ�����ŵĸ�32λ ( ÿ�ε�����Ҫ���� ) DWORD
DAP_SECTOR_LOW  EQU  28  ; ��ʼ�����ŵĵ�32λ ( ÿ�ε�����Ҫ���� ) DWORD
DAP_BUFFER_SEG  EQU  30  ; �������ε�ַ   ( ÿ�ε�����Ҫ���� ) WORD
DAP_BUFFER_OFF  EQU  32  ; ������ƫ��   ( ÿ�ε�����Ҫ���� ) WORD  
DAP_RESERVED2  EQU  33  ; �����ֽ�
DAP_READ_SECTORS EQU  34  ; Ҫ�����������(1 - 127 )
DAP_RESERVED1  EQU  35  ; �����ֽ�
DAP_PACKET_SIZE  EQU  36  ; ���Ĵ�СΪ16�ֽ�

;====================================================================
; 
; Ŀ¼��ṹ(ÿ���ṹΪ32�ֽ�)
; 
;====================================================================
OFF_DIR_NAME    EQU  0  ; Ŀ¼���ƫ��  BYTE[11]
OFF_DIR_ATTRIBUTE   EQU  11  ; Ŀ¼����   BYTE
OFF_NT_RESERVED    EQU  12  ; ��������   BYTE
OFF_CREATE_TIME_HUNDREDTH EQU  13  ; ����ʱ��   BYTE
OFF_CREATE_TIME    EQU  14  ; ����ʱ��   WORD
OFF_CREATE_DATE    EQU  16  ; ����ʱ��   WORD
OFF_LAST_ACCESS_DATE  EQU  18  ; �ϴη���ʱ��  WORD
OFF_START_CLUSTER_HIGH  EQU  20  ; ��ʼ�غŸ�λ  WORD
OFF_LAST_UPDATE_TIME  EQU  22  ; �ϴθ���ʱ��  WORD
OFF_LAST_UPDATE_DATE  EQU  24  ; �ϴθ���ʱ��  WORD
OFF_START_CLUSTER_LOW  EQU  26  ; ��ʼ�غŵ�λ  WORD
OFF_FILE_SIZE    EQU  28  ; �ļ��ߴ�   DWORD

; ��س���
DIR_NAME_DELETED   EQU  0E5H ; �����Ѿ���ɾ��
DIR_NAME_FREE    EQU  00H  ; �����ǿ��е�(���Ҳ�ǿ��е�)
DIR_NAME_DOT    EQU  2EH  ; ����Ŀ¼ . �� ..
DIR_NAME_SPACE    EQU  20H  ; ��������ַ�
DIR_ENTRY_SIZE    EQU  32  ; ÿ��Ŀ¼��ĳߴ磬��ṹ������ʾ 

;�ļ�����
DIR_ATTR_READONLY   EQU  01H  ; ֻ���ļ�
DIR_ATTR_HIDDEN    EQU  02H  ; �����ļ�
DIR_ATTR_SYSTEM    EQU  04H  ; ϵͳ�ļ�
DIR_ATTR_VOLUME    EQU  08H  ; ����(ֻ���ܳ����ڸ�Ŀ¼��)
DIR_ATTR_SUBDIR    EQU  10H  ; ��Ŀ¼
DIR_ATTR_ARCHIVE   EQU  20H  ; �鵵����
DIR_ATTR_LONGNAME   EQU  0FH  ; ���ļ���
DIR_ATTR_LONGNAME_MASK  EQU  3FH  ; ���ļ�������

; ������
CLUSTER_MASK    EQU  0FFFFFFFH ; �غ�����(FAT32=>FAT28)
CLUSTER_FREE    EQU  00000000H ; ���ǿ��е�
CLUSTER_RESERVED   EQU  00000001H ; ���Ǳ�����
CLUSTER_MIN_VALID   EQU  00000002H ; ��С��Ч�غ�
CLUSTER_MAX_VALID   EQU  0FFFFFF6H ; �����Ч�غ�
CLUSTER_BAD     EQU  0FFFFFF7H ; ����
CLUSTER_LAST    EQU  0FFFFFF8H   ;0xFFFFFFF8-0xFFFFFFFF��ʾ�ļ������һ����

;Start segment address of bootsect after loaded into memory.
DEF_ORG_START EQU 7C0H

;Base segment address of the bootsect after relocation
;DEF_BOOT_START EQU 9F00H
;DEF_BOOT_START EQU 9B00H  --Modified by Garry in 28.May
DEF_BOOT_START EQU 9000H

;Start address of OS image.
DEF_RINIT_START EQU 1000H

;====================================================================
;
; ��������(512�ֽ�)
;
;====================================================================
_ENTRY_POINT:

; 3�ֽڵ���תָ��
 JMP SHORT _BOOT_CODE ; ��ת����������������
 NOP       ; ��ָ���Ա�֤�ֽ���Ϊ3

;====================================================================
; 
; BPB( BIOS Parameter Block ) 
; 
;====================================================================
SectorsPerCluster  DB 32 ; ÿ���ص������� ( 1 2 4 8 16 32 64 128 )
        ; ������˲��ܳ���32K(������С)
ReservedSectors   DW 36 ; �Ӿ�ĵ�һ��������ʼ�ı���������Ŀ��
        ; ��ֵ����Ϊ0������FAT12/FAT16����ֵͨ��Ϊ1��
        ; ����FAT32������ֵΪ32��
NumberOfFATs   DB 2 ; ����FAT���ݽṹ����Ŀ����ֵͨ��ӦΪ2
HiddenSectors   DD 38 ; ������FAT��ķ���֮ǰ������������

;====================================================================
; 
; EBPB ( Extended BIOS Parameter Block )
; 
;====================================================================
SectorsPerFAT32   DD 14994   ; ����FAT32�����ֶΰ���һ��FAT�Ĵ�С����SectorsPerFAT16
          ; �ֶα���Ϊ0;
RootDirectoryStart  DD 2   ; ��Ŀ¼����ʼ�غţ�ͨ��Ϊ2��
DriveNumber    DB ?   ; ����INT 0x13���������ţ�0x00Ϊ���̣�0x80ΪӲ��

;====================================================================
;
; ����������������⿪ʼ( ƫ�ƣ�0x3E ) 
; �书�����������̵ĸ�Ŀ¼������FDOSLDR.BIN�ļ�����������ڴ沢���С�
;
;====================================================================
;====================================================================
;
; Memory layout:
;     9F000H - A0000H : Bootsector code,buffer,BP and SP
;     9F000H - 9F3FFH : Boot sector code
;     9F400H - 9F7FFH : Base address space.
;     9F800H - 9FFFEH : Stack area of boot sector code.
;     A0000H - A2000H : Temporary buffer for int 13h.
;     Actual start address is 90000H
;     
_BOOT_CODE:

;The following code is added by Garry.
    cli                          ;;Mask all maskable interrupts.
    mov ax,DEF_ORG_START         ;;First,the boot code move itself to DEF_-
                                 ;;BOOT_START from DEF_ORG_START.
    mov ds,ax

    cld
    mov si,0x0000
    mov ax,DEF_BOOT_START
    mov es,ax
    mov di,0x0000
    mov cx,0x0200                ;;The boot sector's size is 512B
    rep movsb

    mov ax,DEF_BOOT_START        ;;Prepare the execute context.
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov bp,0x7f0                 ;!!!!!!! CAUTION !!!!!!!!!!!!!
    mov sp,0xffe
    jmp DEF_BOOT_START : gl_bootbgn  ;;Jump to the DEF_BOOT_START to execute.

gl_bootbgn:
 ; ��ʼ����ؼĴ�������־λ
 ;CLI      ; �ȹص��ж�
 ;CLD      ; ����Ϊ��ǰ����
 ;XOR  AX,AX   ; AX = 0
 ;MOV  DS,AX   ; �������ݶμĴ��� DS:SI
 ;MOV  ES,AX   ; ���ø��ӶμĴ��� ES:DI
 ;MOV  SS,AX   ; ���ö�ջ�μĴ���
 ;MOV  BP,7C00H  ; ���û�ַ�Ĵ���
 ;MOV  SP,STACK_ADDR ; ���ö�ջջ��
 STI      ; �����ж�  --Modified by Garry.xin--

 ;====================================================================
 ; ���������Ĵ��̱��
 ;====================================================================
 MOV  [DriveNumber],DL; ��ֵ��BIOS���ã�����Ǵ�USB��������ֵΪ0x80
       ; ��Ϊ��һ��Ӳ�̵ı�ţ���ֵ�����ں����Ĵ���
       ; ��ȡ����


 ;====================================================================  
 ; ׼��FAT32�ļ�ϵͳ���õĳ������Ա����Ĳ���
 ;====================================================================
 ;
 ; [��������][��������][FAT][DATA]
 ;
 ;====================================================================
 
 MOV  BYTE [BP - DIR_PER_SECTOR],16 ; AL    = DirEntriesPerSector
 
 ; FAT��ʼ����
 ; FAT��ʼ���� = Hidden+Reserved
 MOV  AX ,WORD [ReservedSectors]
 CWD          ; AX => DX : AX
 ADD  AX, WORD [HiddenSectors]
 ADC   DX, WORD [HiddenSectors+2]  
 MOV  WORD[ BP - FAT_START_SECTOR  ],AX 
 MOV   WORD[ BP - FAT_START_SECTOR+2],DX 
 
 
 ; FAT����ռ��������
 ; FAT_SECTORS = NumberOfFAT * SectorsPerFAT
 XOR  EAX,EAX
 MOV  AL, BYTE [NumberOfFATs]  ; FAT����Ŀ
 MOV  EBX,DWORD [SectorsPerFAT32]
 MUL  EBX        ; �˻����� EDX:EAX
 MOV  DWORD [ BP - FAT_ENTRY_SECTORS  ] , EAX
 
 ; ������������ʼ����
 ADD  EAX ,DWORD[ BP - FAT_START_SECTOR  ]
 MOV  DWORD [ BP - DATA_START_SECTOR ],EAX 
 
 
 ;====================================================================
 ;
 ; ��ʼ��DiskAddressPacket
 ; ʹ��ʱֻ��Ҫ�޸��ֶΣ�DATA_BUFFER_OFF DATA_BUFFER_SEG 
 ;       DAP_SECTOR_LOW  DAP_SECTOR_HIGH
 ;
 ;====================================================================
 MOV  DWORD [BP - DAP_SECTOR_HIGH ],00H
 MOV  BYTE  [BP - DAP_RESERVED1   ],00H
 MOV  BYTE  [BP - DAP_RESERVED2   ],00H
 MOV  BYTE  [BP - DAP_PACKET_SIZE ],10H
 MOV  BYTE  [BP - DAP_READ_SECTORS],01H
 ;MOV  WORD  [BP - DAP_BUFFER_SEG  ],00H
 MOV  WORD  [BP - DAP_BUFFER_SEG  ],DEF_BOOT_START
 MOV  BYTE  [BP - DAP_READ_SECTORS],01H  ; ÿ��ֻ��ȡһ������ 
 
 ; ���濪ʼ���Ҹ�Ŀ¼����װ��FDOSLDR.BIN
 JMP  _SEARCH_LOADER
 
;====================================================================
; ������
;====================================================================
 
__ERROR:
 ; ���ü����жϣ��ȴ��û�����
 MOV  AH,00H
 INT  16H

__ERROR_1: 
 ; ���������
 INT  19H 

;====================================================================
; 
; �ӹ���
; 
;====================================================================

;====================================================================
; 
; ��ȡһ����������
; ���룺 �Ѿ�������DAP����Ӧ���ֶ�
; ���ƣ� ���ܶ�ȡ����һ���ص�����   
; 
;====================================================================
ReadSector:

 PUSHA  ; ����Ĵ���
 
;====================================================================
; INT 13H  AH = 42H ��չ���̵���
;====================================================================
 ; ÿ�ζ�ȡһ������
 MOV  AH,42H         ; ���ܺ� 
 LEA  SI ,[BP - DAP_PACKET_SIZE]    ; ��ַ����ַ

 ; ��������
 MOV  DL ,[DriveNumber]      ; ��������
 INT  13H
 JC   __ERROR        ; ��ȡʧ�� ------- DEBUG --------
 POPA       ; �ָ��Ĵ��� 
 RET

;====================================================================
; ������
;====================================================================
LoaderName     db "HCNIMGE BIN"       ; �ڶ��׶��������� FDOSLDR.BIN
ProcessDot     db "."

;====================================================================
; ���Ҹ�Ŀ¼������Ƿ��� FDOSLDR.BIN�ļ�
;====================================================================
_SEARCH_LOADER: 


 ; ���û�����
 MOV  WORD [ BP - DAP_BUFFER_OFF  ], DATA_BUF_OFF ; 0000:1000H
 
 ; ��Ŀ¼��ʼ������
 MOV  EAX,DWORD[RootDirectoryStart]
 MOV  DWORD[ BP - CURRENT_CLUSTER ],EAX

; �����һ����
_NEXT_ROOT_CLUSTER:

 ; ���ݴغż���������
 DEC  EAX
 DEC  EAX  ; EAX = EAX - 2
 XOR  EBX,EBX 
 MOV  BL, BYTE[ SectorsPerCluster]
 MUL  EBX 
 ADD  EAX,DWORD[ BP- DATA_START_SECTOR]
 MOV  DWORD[ BP - DAP_SECTOR_LOW  ], EAX
 MOV  DL,[SectorsPerCluster]

; �����һ������
_NEXT_ROOT_SECTOR:
  
 ; ���ζ�ȡÿ����Ŀ¼����������Ƿ����FDOSLDR.BIN�ļ�
 CALL ReadSector
 
 ; ������������
 MOV  DI,DATA_BUF_OFF
 MOV  BL,BYTE [ BP - DIR_PER_SECTOR]

; ���ÿһ��Ŀ¼��
_NEXT_ROOT_ENTRY:
 CMP  BYTE [DI],DIR_NAME_FREE
 JZ  __ERROR    ; NO MORE DIR ENTRY
 
 ; ����Ƿ�װ�س���
 PUSH  DI       ; ����DI
 MOV  SI,LoaderName
 MOV  CX,11
 REPE  CMPSB 
 JCXZ  _FOUND_LOADER    ; װ��Loader������
  
 ; �Ƿ�����һ��Ŀ¼��(�ڲ�ѭ��)
 POP  DI
 ADD   DI,DIR_ENTRY_SIZE
 DEC  BL 
 JNZ   _NEXT_ROOT_ENTRY
 
 ; ����Ƿ�����һ�������ɶ�(���ѭ��)
 DEC  DL
 JZ  _CHECK_NEXT_ROOT_CLUSTER
 INC  DWORD [ BP - DAP_SECTOR_LOW ] ; ����������
 JMP  _NEXT_ROOT_SECTOR 
 
; �����һ����
_CHECK_NEXT_ROOT_CLUSTER:

 ; ����FAT���ڵĴغź�ƫ�� 
 ; FatOffset = ClusterNum*4
 XOR  EDX,EDX
 MOV  EAX,DWORD[BP - CURRENT_CLUSTER]
 SHL  EAX,2
 XOR  ECX,ECX
 MOV  CX,512
 DIV  ECX  ; EAX = Sector EDX = OFFSET
 ADD  EAX,DWORD [BP - FAT_START_SECTOR  ]
 MOV  DWORD [ BP - DAP_SECTOR_LOW ], EAX 
   
 ; ��ȡ����
 CALL  ReadSector
  
 ; �����һ����
 MOV  DI,DX
 ADD  DI,DATA_BUF_OFF
 MOV  EAX,DWORD[DI]  ; EAX = ��һ��Ҫ���Ĵغ�
 AND  EAX,CLUSTER_MASK
 MOV  DWORD[ BP - CURRENT_CLUSTER ],EAX
 CMP  EAX,CLUSTER_LAST  ; CX >= 0FFFFFF8H������ζ��û�и���Ĵ���
 JB  _NEXT_ROOT_CLUSTER
 JMP  __ERROR

;====================================================================
; װ��FDOSLDR.BIN�ļ�
;====================================================================
_FOUND_LOADER:
 ; Ŀ¼�ṹ��ַ����DI��
 POP  DI
 XOR  EAX,EAX
 MOV  AX,[DI+OFF_START_CLUSTER_HIGH] ; ��ʼ�غŸ�16λ
 ;SHL  AX,16
 SHL  EAX,16 ;----- Modified by Garry.xin----
 MOV  AX,[DI+OFF_START_CLUSTER_LOW]  ; ��ʼ�غŵ�16λ
 MOV  DWORD[ BP - CURRENT_CLUSTER ],EAX
 MOV  CX, OSLOADER_SEG      ; CX  = �������ε�ַ 
 
  
_NEXT_DATA_CLUSTER:
 
 ; ���ݴغż���������
 DEC  EAX
 DEC  EAX  ; EAX = EAX - 2
 XOR  EBX,EBX 
 MOV  BL, BYTE[ SectorsPerCluster]
 MUL  EBX 
 ADD  EAX,DWORD[ BP- DATA_START_SECTOR]
 MOV  DWORD[ BP - DAP_SECTOR_LOW  ], EAX
 MOV  DL,[SectorsPerCluster]

 ; ���û�����
 MOV  WORD [ BP - DAP_BUFFER_SEG   ],CX
 MOV  WORD [ BP - DAP_BUFFER_OFF   ],00H
   
 ; ÿ������Ҫ��ȡ��������
 MOV  BL , BYTE [SectorsPerCluster]

_NEXT_DATA_SECTOR:
 ; ��ȡ���е�ÿ������(�ڲ�ѭ��)
 ; ע�� : ͨ������ļ���С�����Ա����ȡ���һ�������ص����д�С
 ; ��ȡ��������
 CALL  ReadSector

 ;Show loading process,modified by Garry.Xin
 PUSH BX
 MOV  AL,'.'
 MOV  AH,0EH
 MOV  BX,07H
 INT  10H
 POP  BX
 
 ; ���µ�ַ��������ȡ
 MOV  AX, 512
 ADD  WORD  [BP - DAP_BUFFER_OFF],AX 
 INC  DWORD [BP - DAP_SECTOR_LOW]  ; ����������
 DEC  BL        ; �ڲ�ѭ������
 JNZ  _NEXT_DATA_SECTOR
  
 
 ; �����һ����
  
 ; ���¶�ȡ��һ���صĻ�������ַ
 MOV  CL,BYTE [ SectorsPerCluster ]
 MOV  AX ,512
 SHR  AX ,4
 MUL  CL
 ADD  AX ,WORD [ BP - DAP_BUFFER_SEG ] 
 MOV  CX,AX ; ������һ���صĻ������ε�ַ
 
 ;====================================================================
 ;
 ; ����Ƿ�����һ����(��ȡFAT��������Ϣ)
 ;  LET   N = ���ݴغ�
 ;  THUS FAT_BYTES  = N*4  (FAT32)
 ;      FAT_SECTOR = FAT_BYTES / BytesPerSector
 ;    FAT_OFFSET = FAT_BYTES % BytesPerSector
 ;
 ;====================================================================
 
 ; ����FAT���ڵĴغź�ƫ�� 
 MOV  EAX,DWORD [BP - CURRENT_CLUSTER]
 XOR  EDX,EDX
 SHL  EAX,2
 XOR  EBX,EBX
 MOV  BX,512
 DIV  EBX   ; EAX = Sector  EDX = Offset
 
 ; ���û�������ַ
 ADD  EAX,DWORD [BP - FAT_START_SECTOR  ]
 MOV  DWORD [ BP - DAP_SECTOR_LOW ], EAX 
 MOV   WORD [BP - DAP_BUFFER_SEG  ], DEF_BOOT_START 
 MOV  WORD [BP - DAP_BUFFER_OFF  ], DATA_BUF_OFF ; 0000:1000H

 ; ��ȡ����
 CALL  ReadSector
  
 ; �����һ����
 MOV  DI,DX
 ADD  DI,DATA_BUF_OFF
 MOV  EAX,DWORD[DI]  ; EAX = ��һ��Ҫ���Ĵغ�
 AND  EAX,CLUSTER_MASK
 MOV  DWORD[ BP - CURRENT_CLUSTER ],EAX
 CMP  EAX,CLUSTER_LAST  ; CX >= 0FFFFFF8H������ζ��û�и���Ĵ���
 JB  _NEXT_DATA_CLUSTER

;��ȡ���
_RUN_LOADER: 

 ; ����FDOSLDR.BIN
 ;MOV  DL,[DriveNumber]
 JMP  DEF_RINIT_START / 16 : 0

Padding  TIMES 510-($-$$) db  00H
Signature  DW 0AA55H
