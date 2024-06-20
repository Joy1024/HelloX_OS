;;------------------------------------------------------------------------
;;  Original Author           : Garry
;;  Original Finished date    :
;;  Module name               : newldr.asm
;;  Usage                     : This module contains the implementation code
;;                              of new loading process(new loader).
;;                              The new loader is introduced in V1.89 of 
;;                              HelloX OS under x86 arch, it solves the issue
;;                              of kernel size's limitation before V1.88,
;;                              which size is 567K at most, since the OLD
;;                              loader just loads kernel into low 1M end
;;                              memory of real mode, the usable RAM is 640K
;;                              at most.
;;  Last modified date        :
;;  Last modified author      :
;;  Lines Number              :
;;------------------------------------------------------------------------

;;------------------------------------------------------------------------
;;  Memory layout of this module(load address when running is in bracket):
;;    0x00000(0x12000) - 0x10000(0x22000): The whole module of new loader,
;;                                         include code, data, stack, buff,
;;                                         size is 64K.
;;    0x00000(0x12000) - 0x01000(0x13000): Code section of new loader in
;;                                         real mode, 16 bits, size is 4K;
;;    0x01000(0x13000) - 0x01800(0x13800): Segment descriptors, 2K;
;;    0x01800(0x13800) - 0x01C00(0x13C00): Code section in protect mode,
;;                                         32 bits, size is 1K;
;;    0x01C00(0x13C00) - 0x02000(0x14000): Code section when return back
;;                                         from protect mode, size is 1K;
;;    0x02000(0x14000) - 0x03000(0x15000): Code section to initialize protect
;;                                         mode's environment.
;;    0x03000(0x15000) - 0x04000(0x16000): Stack of this module, 4K;
;;    0x04000(0x16000) - 0x08000(0x1A000): Reserved, 16K;
;;    0x08000(0x1A000) - 0x10000(0x22000): Root directory's buffer, 32K;
;;
;;  Here is the OS kernel's buffer in real mode, i.e, oskernl.bin will
;;  be loaded into this memory, and will be copied to higher 1M space
;;  when 320K is loaded:
;;    0x22000 - 0x72000 : oskernel's buffer in real mode.
;;
;;  This module is linked together by batch.bat file with realinit.bin, 
;;  miniker.bin, into osloadr.bin file. osloadr.bin file is loaded into
;;  memory by boot sector. The memory layout after this module is loaded
;;  as follows:
;;    0x00000 ~ 0x01000: reserved, unchanged BIOS area, 4K;
;;    0x01000 ~ 0x02000: realinit.bin, 4K;
;;    0x02000 ~ 0x12000: miniker.bin, 64K;
;;    0x12000 ~ 0x22000: newldr.bin, 64K;
;;  Boot sector transfers control to realinit.bin after load, and
;;  realinit.bin will jump to newldr.bin to continue loading OS kernel
;;  after initializes real mode environment.
;;  The new loader should load oskernl.bin into upper 1M memory, and
;;  then copy miniker.bin into 1M position, then jump to miniker.bin
;;  to run.
;;------------------------------------------------------------------------

;;------------------------------------------------------------------------
;; Constants used in this module.
;;------------------------------------------------------------------------

;; Segment address of the new loader in memory.
%define SEGMENT_OF_NEW_LOADER 0x1200
;; Base address of new loader in physical address.
%define BASE_OF_NEW_LOADER 0x12000
;; Stack top pointer of the new loader, @16K of the segment.
%define STACK_OF_NEW_LOADER 0x04000
;; Code begin address in protect mode, physical address.
%define CODE32_BEGIN_ADDRESS 0x13800
;; Code begin address when back from protect mode.
%define CODE16_BEGIN_ADDRESS 0x13C00
;; Code to init protect mode running environment.
%define CODE32_INIT_PROTECT_ENV 0x14000

; Segment address of the boot loader(boot sector) after relocation.
; The boot loader is loaded into 0x7C00 by BIOS, it will
; relocate itself to 0x90000(576K) to yeild space for
; HelloX's OS loader, which is start from 0x1000(4K).
; The partition table also be loaded into this segment, and 
; it will be used by the new loader again. We just address it
; since the partition table is unchanged since then.
%define __BOOT_LOADER_SEGMENT 0x9000
; This defines offset to first MBR entry-0x10 relate
; to boot loader segment.
; Please refer fat32bs.asm for detail information.
%define MBR 0x7DAE

; Defines address to a buffer for loading root directory clusters.
; This buffer's size is 32K,it's enough to hold one cluster.
%define __ROOT_BUFFER_SEG 0x1200 ; Same as SEGMENT_OF_NEW_LOADER
%define __ROOT_BUFFER_OFF 0x08000

; OS kernel's buffer address and size in real mode.
%define __OSK_BUFFER_ADDRESS 0x22000
%define __OSK_BUFFER_SEGMENT 0x2200
%define __OSK_BUFFER_LENGTH  0x50000

; OS kernel's running address, 1M + 64 position,
; where 64K is the miniker.bin's size.
%define __OSK_RUNNING_ADDRESS 0x110000

; OS kernel file's maximal size, 2M at most.
; Add running address to it, to simplify the comparing,
; see code for detail information.
%define __OSK_MAX_SIZE (0x200000 + __OSK_RUNNING_ADDRESS)

; Mini kernel's loaded address in real mode.
%define __MINIKER_REAL_ADDRESS 0x2000
; Mini kernel's size.
%define __MINIKER_SIZE 0x10000
; Mini kernel's running address, @1M.
%define __MINIKER_RUNNING_ADDRESS 0x100000

; Error code and their meanings used in this module.
%define ERR_NO_EXT      0x30 ; <-- Extended BIOS interrupts not supported
%define ERR_NO_ACTIVE   0x31 ; <-- No active partition found
%define ERR_NOT_FAT32   0x32 ; <-- Active partition is not FAT32
%define ERR_NOT_FOUND   0x33 ; <-- File is not present in root directory
%define ERR_HARDWARE    0x34 ; <-- Loading sector error(probably hardware error)
%define ERR_OSK_SIZE    0x35 ; <-- OS kernel file's size exceed.

; Some offsets in loaded BPB for calculating FAT values.
; BPB is defined as a label later.
 %define BytesPerSec   0x0B ; <--Word  - Points to Bytes Per Sector
 %define SecPerCluster 0x0D ; <--Byte  - Points to Sectors Per Cluster
 %define ResvdSecs     0x0E ; <--Word  - Points to Reserved Sectors
 %define FatCount      0x10 ; <--Byte  - Points to Number of FATs
 %define FatSize       0x24 ; <--Dword - Points to Size of one FAT
 %define RootCluster   0x2C ; <--Dword - Points to Root Cluster Number

; Fat32 partition types.
%define FAT32 0x0B
%define FAT32LBA 0x0C
%define HIDFAT32 0x1B
%define HIDFAT32LBA 0x1C

; Fixed bytes per sector as 512.
%define __BYTES_PER_SECTOR 0x200

;;------------------------------------------------------------------------
;;  Code of this module.
;;------------------------------------------------------------------------

    bits 16
	org 0x0000
newldr_start:
	; Initializes all registers.
	mov ax, SEGMENT_OF_NEW_LOADER
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, STACK_OF_NEW_LOADER

	; Save boot device number.
	mov byte[boot_device], dl

	; Load kernel into memory.
	; Show out 'Loading...' string
	; first.
	mov ax, load_message
	call np_strlen
	mov ax, load_message
	call np_printmsg

	; Check partition table, get the first
	; sector of the activate partition.
	call gl_check_partition
	cmp eax, 0x00
	je .enter_deadloop

	; Load the BPB(first sector) of 
	; activate partition, and initializes
	; FAT32 related variables.
	call gl_load_bpb

	; Search the root directory to find
	; os kernel file - oskernl.bin.
	call gl_search_kernel

	; OS kernel is located, so we load it
	; into memory.
	call gl_load_kernel

	; Then move miniker.bin to it's running
	; position(1M address).
	call gl_copy_miniker

	; Launch OS, this routine will never return.
	call gl_launch_os

	; Any exception will come here.
.enter_deadloop:
	; Enter dead loop.
	call np_deadloop

; Switch to protect mode, and launch OS.
gl_launch_os:
	; Switch to protect mode, and jump
	; to start address to launch OS.
	cli
	; Calculate the physical address of GDT.
	xor eax, eax
	mov ax, ds
	shl eax, 0x04
	add eax, gl_sysgdt
	mov dword[gl_gdt_base], eax
	lgdt [gl_gdtr_ldr]
	; Switch to protect mode.
	mov eax, cr0
	or eax, 0x01
	mov cr0, eax
	; Jump to the code to init protect environment.
	jmp dword 0x08 : CODE32_INIT_PROTECT_ENV

; Move the miniker.bin into upper 1M space
; from the original loaded position in real
; mode.
gl_copy_miniker:
	mov esi, __MINIKER_REAL_ADDRESS
	mov edi, __MINIKER_RUNNING_ADDRESS
	mov ecx, __MINIKER_SIZE
	call protect_copy
	ret

; Load kernel into memory.
; It first load batch of the kernel file
; into low end memory, then copy it upper 1M
; space by switching to protect mode, and
; switch back to continue loading process.
; The kernel file's first cluster is saved
; into [CLUSTER], so just use it.
gl_load_kernel:
	mov ecx, __OSK_BUFFER_LENGTH
	; Get cluster's size in byte.
	mov al, byte[BPB+SecPerCluster]
	mov ebx, __BYTES_PER_SECTOR
	mul ebx ;eax contains the cluster's size now.
	mov word [DAP.segment], __OSK_BUFFER_SEGMENT
	mov word [DAP.offset], 0
	; and load it into memory.
.loadfile:
	; Save eax/ecx/edi since they maybe changed in lnc.
	push eax
	push ecx
	push edi
	call gl_load_next_cluster
	; Show out a dot to indicate progress: al = 0x2E('.')
	mov ax, 0x0E2E
	int 0x10
	pop edi
	pop ecx
	pop eax
	add dword[kernel_to_move], eax
	sub ecx, eax
	jne .real_continue
	; Should move this batch of kernel into upper space.
	mov esi, __OSK_BUFFER_ADDRESS
	mov edi, dword[kernel_target_pos]
	mov ecx, __OSK_BUFFER_LENGTH
	push eax
	call protect_copy
	; Show out a 'M' to indicate the data moving. al = 0x4D('M')
	mov ax, 0x0E4D
	int 0x10
	pop eax
	; Re-init ecx, accumulate the destination with buffer length.
	mov ecx, __OSK_BUFFER_LENGTH
	add dword[kernel_target_pos], __OSK_BUFFER_LENGTH
	mov dword[kernel_to_move], 0
	; Check OS kernel's file size.
	cmp dword[kernel_target_pos], __OSK_MAX_SIZE
	jae .err_osk_size
	; Reset loading buffer in real mode.
	mov word [DAP.segment], __OSK_BUFFER_SEGMENT
	mov word [DAP.offset], 0
.real_continue:
	cmp dword [CLUSTER], 0x0FFFFFF8
	jb .loadfile
	; File loaded over, copy the remaind to upper 1M space.
	mov esi, __OSK_BUFFER_ADDRESS
	mov edi, dword[kernel_target_pos]
	mov ecx, dword[kernel_to_move]
	push eax
	call protect_copy
	; Show out a 'M' to indicate the data moving. al = 0x4D('M')
	mov ax, 0x0E4D
	int 0x10
	pop eax
	ret
.err_osk_size:
	mov al, ERR_OSK_SIZE
	jmp ErrorRoutine
; Global data used by load kernel routine.
    align 4
kernel_target_pos: dd __OSK_RUNNING_ADDRESS
kernel_moved_size: dd 0x00
kernel_to_move: dd 0x00 ; Bytes remaind to be move to upper 1M space.

; Search the root directory of FAT32 partition,
; to find the OS kernel file(oskernl.bin).
; Save the cluster number into [CLUSTER] if found,
; and jump to error handler if can not find.
gl_search_kernel:
	cmp dword [CLUSTER], 0x0FFFFFF8
	mov al, ERR_NOT_FOUND ; if file wasn't found(searched whole root directory)-save error code
	jae ErrorRoutine      ; and jump to error routine.
	; Load one cluster of root directory into root cluster buffer.
	; Init the DAP's segment as root cluster buffer's segment, and
	; set the DAP's offset as the offset of root cluster buffer.
	mov word [DAP.segment], __ROOT_BUFFER_SEG
	mov word [DAP.offset], __ROOT_BUFFER_OFF
	call gl_load_next_cluster
	; Prepare registers for searching cluster.
	; In lnc routine, [DAP.segent] is increased
	; one cluster << 4bits, the upper boundary of cluster buffer.
	; So use this value as loop's end condition.
	; xor edx, edx
	; mov dx, word [DAP.segment]
	; shl edx, 0x04
	mov edx, dword[CLUSTER_SIZE]
	add edx, __ROOT_BUFFER_OFF
	mov ebx, __ROOT_BUFFER_OFF - 0x20 ; <-- Offset to Root Buffer-0x20.
	; Prepare registers for each filename comparison.
.searchPrep:
	add bx, 0x0020
	cmp edx, ebx ; <--- if whole cluster was checked - load another one.
	je gl_search_kernel
	; Check if entry has attribute VOLUME_ID or DIRECTORY.
	test byte[es:bx+0x0b], 0x18
    jnz .searchPrep
	mov di, bx
	mov cx, 0x000B ; 11 bytes file name.
	mov si, __OS_KERNEL
	; Compare single characters of filenames from root directory with OS loader.
	repe cmpsb
	jne .searchPrep ;<--- if filenames don't match, try another one.

	; If filename matches then save this file's cluster number
	; to [CLUSTER] for future's usage.
match:
	mov ax, word[es:bx+0x14]
	mov dx, word[es:bx+0x1A]
	mov word [CLUSTER], dx
	mov word [CLUSTER+2], ax
	ret

; Helper routine to load the BPB(first sector) of
; the activate partition.
; eax contains the sector index value of BPB.
gl_load_bpb:
    push ebx
	push ecx
    mov dword [DAP.address], eax
	mov word [DAP.segment], __ROOT_BUFFER_SEG
	mov word [DAP.offset], BPB
	mov word [DAP.count], 0x01
	call dapLoad ; BPB will be loaded into [BPB].

	; Then calculate and save FAT information.
	; LBA of first FAT sector.
	mov ecx, dword [DAP.address]
	mov dword [BPB_LBA], ecx
	;add cx, word [BPB+ResvdSecs] ; May overflow here.
	xor eax, eax
	mov ax, word[BPB+ResvdSecs]
	add ecx, eax
	mov dword [FAT_LBA], ecx

	; LBA of first DATA sector
	xor eax, eax
	mov al, byte [BPB+FatCount]
	mul dword [BPB+FatSize]
	add ecx, eax
	xor eax, eax
	mov al, byte [BPB+SecPerCluster]
	sub ecx, eax  ; First 2 cluster number is reserved.
	sub ecx, eax
	mov dword[DATA_LBA], ecx

	; Save root cluster number into [CLUSTER]
	; for future's usage.
	mov ecx, dword[BPB+RootCluster]
	mov dword[CLUSTER], ecx

	; Calculate and save cluster size.
	mov al, byte[BPB+SecPerCluster]
	mov ebx, __BYTES_PER_SECTOR
	mul ebx ;eax contains the cluster's size now.
	mov dword[CLUSTER_SIZE], eax

.ok_return:
    pop ecx
	pop ebx
    ret

; Helper routine to check the partition table,
; to find the activate part, make sure the partition
; is FAT32, and return the first sector if success.
; Jump to error handler in case of failure.
gl_check_partition:
	push ebx
	; Address the boot loader segment for data.
	mov ax, __BOOT_LOADER_SEGMENT
	mov ds, ax
	; Try to find active partition.
	mov bx, MBR
.findActivePart:
	add bl, 0x10
	cmp bl, 0xFE
    mov al, ERR_NO_ACTIVE ;if no bootable partition was found,
	                      ;just save error code and jump to error routine.
    je ErrorRoutine
	cmp byte [bx], 0x80
	jne .findActivePart;

	; Activate partition found, then
	; check active partition type.
	; only FAT32 is supported up to now.
	cmp byte[bx+0x04], FAT32
	je .ok_return
	cmp byte[bx+0x04], FAT32LBA
	je .ok_return
	cmp byte[bx+0x04], HIDFAT32
	je .ok_return
	cmp byte[bx+0x04], HIDFAT32LBA
	je .ok_return
	mov al, ERR_NOT_FAT32
	jmp ErrorRoutine
.ok_return:
    ; Save the first sector of this part.
    push dword [bx+0x08]
	; Restore the segment register.
	mov ax, SEGMENT_OF_NEW_LOADER
	mov ds, ax
	pop eax ; Return the first sector.
	pop ebx
	ret

; Loads current cluster([CLUSTER])'s content into memory, and
; replace current cluster number([CLUSTER]) with next one.
; The memory position where content is loaded into is specified
; by DAP.segment and DAP.offset, which are initialized by
; this routine's caller.
; It's also updated the segment address of data buffer after
; loaded to ready to hold next cluster.
gl_load_next_cluster:
  ; this part calculates next cluster's number, and
  ; save it to [CLUSTER].
  push dword [CLUSTER]
  push word [DAP.segment]
  push word [DAP.offset]
  xor eax, eax
  mov al, 0x04
  mul dword[CLUSTER] ; eax contains the offset of the cluster in fat.
  xor ebx, ebx
  mov bx, __BYTES_PER_SECTOR ;word [BPB+BytesPerSec]
  div ebx ; now eax contains the sector index of next cluster, edx contains
          ; the offset of next cluster number in sector.
  add eax, dword[FAT_LBA] ; eax then contains the obsolute sector index of cluster
                          ; in FAT.
  ; Load the sector that contains next cluster's index into memory.
  ; Use root directory buffer as sector buffer.
  mov dword[DAP.address], eax
  mov word [DAP.segment], __ROOT_BUFFER_SEG
  mov word [DAP.offset], __ROOT_BUFFER_OFF
  mov word [DAP.count], 0x0001
  xchg bx, dx ; bx contains the offset of next cluster number in sector,
              ; so the bx's value can not exceed sector's size.
  call dapLoad
  mov eax, dword [bx+__ROOT_BUFFER_OFF] ; eax contains the next cluster number.
  and eax, 0x0FFFFFFF ; clear the upper 4 bits.
  mov dword [CLUSTER], eax ; Now [CLUSTER] contains the next cluster number.

  ; This part loads current cluster into memory.
  pop word [DAP.offset]
  pop word [DAP.segment]
  xor eax, eax
  mov al, byte[BPB+SecPerCluster]
  mov byte[DAP.count], al
  pop dword ebx ; ebx contains the cluster number to load.
  mul ebx ; Now eax contains the sector number relative to
          ; data start sector to load.
  add eax, dword[DATA_LBA] ; Obsolute sector number.
  mov dword[DAP.address], eax
  call dapLoad
  ; Also sets buffer for next cluster right after loaded one.
  xor eax, eax
  mov al, byte[BPB+SecPerCluster]
  ;mul word [BPB+BytesPerSec] ;
  ;shl eax, 9 ; eax * __BYTES_PER_SECTOR
  ;shr ax, 0x04
  shl eax, 5 ; combine uper 2 instructions.
  add word [DAP.segment], ax
  ret

; Simple Error routine(only prints one character - error code)
; This routine is used by FAT32 reading code.
ErrorRoutine:
    push eax
	mov ax, error_message
	call np_strlen
	mov ax, error_message
	call np_printmsg
	pop eax
	mov ah, 0x0E
	int 0x10
	jmp $

; Simple routine to load sectors from disk
; according to DAP.
dapLoad:
  mov ah, 0x42
  mov si, DAP
  mov dl, byte [boot_device]
  int 0x13
  jnc .end
  mov al, ERR_HARDWARE
  jmp ErrorRoutine
.end:
  ret

;; Calculate a given string's length.
;; @ax: base address of string;
;; Return:
;; @cx: contains the string's length.
;; Note:
;; @ax's value maybe changed.
np_strlen:
    push bp
    push bx
    mov bx,ax
    mov al,0x00
    xor cx,cx
.ll_begin:
    cmp byte [bx],al
    je .ll_end
    inc bx
    inc cx
    jmp .ll_begin
.ll_end:
    pop bx
    pop bp
    ret                          ;;End of the proc.

;; Print out a string on current cursor
;; position of screen.
;; @ax: base address of string;
;; @cx: length of string;
;; Return:
;; @ax value maybe changed.
np_printmsg:                
    push bp                      ;;Save the registers used by this proc.
    push bx
    push ax                      ;;Some registers not used in this procedu-
                                 ;;re maybe used in the int 0x010,so we must
                                 ;;save them too.Fuck BIOS!!!!!
    push cx
    mov ah,0x03
    mov bh,0x00
    int 0x010                    ;;Read the position of cursor.
    mov bx,0x0007
    mov ax,0x1301
    pop cx
    pop bp
    int 0x010                    ;;Print out the the message.
    pop bx
    pop bp
    ret                          ;;End of the procedure.

;; Dead loop procedure, just show
;; out a message and enter infinite loop.
np_deadloop:
    mov ax, dead_message
    call np_strlen
    mov ax, dead_message
    call np_printmsg 
    mov cx,0x01
.ll_dead:
    inc cx
    loop .ll_dead
    ret                          ;;The end of the procedure.

;; Helper routine to copy data from
;; real mode into protect mode upper 1M space.
;; The routine switch to protect mode first,
;; then copy data to the specified location, and
;; back to real mode.
;; The interrupt must be disabled in the whole
;; routine.
;; Input:
;;   @esi : base address of data source;
;;   @edi : base address of data destination;
;;   @ecx : byte amount to copy.
;; Exception: Just enter dead loop.
protect_copy:
    push eax
	pushfd
	cli
	; Calculate the physical address of GDT.
	xor eax, eax
	mov ax, ds
	shl eax, 0x04
	add eax, gl_sysgdt
	mov dword[gl_gdt_base], eax
	lgdt [gl_gdtr_ldr]

	; Switch to protect mode.
	mov eax, cr0
	or eax, 0x01
	mov cr0, eax
	; Jump to 32 bits begin address, physical
	; address must be used here, it's corresponding
	; the label gl_pm32_begin.
	jmp dword 0x08 : CODE32_BEGIN_ADDRESS

	; Return from protect mode, continue
	; execute from here.
	align 4
gl_return_from_protect:
    popfd
	pop eax
	ret  ; End of the protect_dopy.

	align 4
;; Dead loop message.
dead_message:
    db 0x0d
	db 0x0a
    db '[ERROR][new_ldr]Please restart system.'
    db 0x0d
    db 0x0a
    db 0x00

	align 4
;; Error information.
error_message:
    db '[ERROR]code is:'
    db 0x00

	align 4
;; Message show out before load kernel.
load_message:
    db 'Loading:'
	db 0x0d
	db 0x0a
	db 0x00

	align 4
; File name of OS kernel in 8/3 format.
__OS_KERNEL : db 'OSKERNL BIN' 

	align 4
;; Boot device number will be saved here from
;; dl register at the beginning of new loader.
boot_device:
    db 0x00

	align 4
; Disk Address Packet used by int 13 to load
; sector from disk.
DAP:
	.size:    db 0x10   ; Size of DAP structure.
	.null:    db 0x00   ; Reserved.
	.count:   dw 0x0001 ; Sector counter to xfer.
	.offset:  dw 0x0000 ; Offset of data buffer.
	.segment  dw 0x0000 ; Segment address of data buffer.
	.address: dq 0x0000 ; Start block number.

	align 4
; Data variables used to hold BPB/FAT key information.
; These data will be accessed through the whole module.
BPB_LBA:
    dd 0x00
FAT_LBA:
    dd 0x00
DATA_LBA:
	dd 0x00
CLUSTER:
    dd 0x00
CLUSTER_SIZE:
	dd 0x00

	align 4
; FAT BPB buffer, size is 512 bytes.
BPB:
    times 512 db 0x00

;; End of the code section.
end_code:
    times 4*1024 - ($ - $$) db 0x00

;;------------------------------------------------------------------------
;; Segment descriptors used in the new loader module.
;;------------------------------------------------------------------------

;; GDT table when xfer to protected mode, they are copied from miniker.bin
;; with unchanged(and must keep unchanged).
gl_sysgdt:
    ;; First entry of GDT.
    gl_gdt_null:
        dd 0
        dd 0
	;; Code segment.
    gl_gdt_syscode:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        dw 0xCF9B
        db 0x00
	;; Data segment.
    gl_gdt_sysdata:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        dw 0xCF93
        db 0x00
	;; Stack segment.
    gl_gdt_sysstack:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        dw 0xCF93
        db 0x00
	;; Extend segment.
    gl_gdt_sysext:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        dw 0xCF93
        db 0x00
	;; Segment dedicated for VGA.
	;; This segment could loaded into GS.
    gl_gdt_sysvga:
        dw 0x0048    ;;The segment's limit is 0x48 * 4K
                     ;;= 288K,countains the vag text m-
                     ;;ode buffer and graphic mode buffer.
        dw 0x8000
        db 0x0b      ;;This segment's base address is
                     ;;0x0b8000,which is the display's
                     ;;text mode buffer address.
        dw 0xc093
        db 0x00
	;; Segment for normalization, when switch running
	;; mode to real from protected.
    gl_gdt_normal:
	    dw 0xFFFF
        dw 0x0000
        db 0x00
        dw 0x0092
        db 0x00
	;; Segment for code 16, used when switching back
	;; to real mode from protected mode.
	;; Please be noted that the segment base address
	;; is same as real mode, but with 4 bits right shifted.
	;; So when new loader's segment base is 0x1200, the
	;; segment base is 0x12000 here.
	;; SEGMENT_OF_NEW_LOADER macro must be same as here.
    gl_gdt_code16:
        dw 0xFFFF
        dw 0x2000 ;segment base, 0~16 bits;
        db 0x01   ;segment base, 17~24 bits;
        dw 0x0098
        db 0x00
	;; Segment for user code.
	gl_gdt_usrcode:
        dw 0xFFFF
		dw 0x0000
		db 0x00
		dw 0xCFFB
		db 0x00
	;; Segment for user data.
	gl_gdt_usrdata:
        dw 0xFFFF
		dw 0x0000
		db 0x00
		dw 0xCFF3
		db 0x00
	;; Space reserved for TSS,each processor has one,
	;; 64 elements at most.
	gl_gdt_tss_space:
        times 64 dd 0
		times 64 dd 0

align 16
;; Contains address and length boundary
;; information about GDT, which is used
;; by lgdt instruction.
gl_gdtr_ldr:
		dw (74 * 8 - 1)
gl_gdt_base:
        dd 0x00

;; End of the segment descriptor section.
end_segment_desc:
    times 6*1024 - ($ - $$) db 0x00

;;------------------------------------------------------------------------
;; Code in protect mode, 32 bits, it's corresponding the macro
;; CODE32_BEGIN_ADDRESS, size is 1K.
;;------------------------------------------------------------------------

	; Execute in protect mode now.
	; This routine is invoked in protect_copy routine
	; runs in real mode.
	bits 32
gl_pm32_begin:
    ; Just init ds/es is enough.
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	; Move data now. esi/edi: source and destinantion
	; address of data buffer, ecx: byte counter.
	shr ecx, 0x02
	cld
	rep movsd

	; Switch back to real mode.
	jmp 0x38 : gl_code16_from_pm

	; End of the protect mode code section.
end_pm32_code:
    times 7*1024 - ($ - $$) db 0x00

;;------------------------------------------------------------------------
;; Code sections executed when return back from protect mode, it's same
;; as the macro CODE16_BEGIN_ADDRESS, size is 1K.
;;------------------------------------------------------------------------

    bits 16
gl_code16_from_pm:
    ; Flush the cache of segment registers.
	; Just flush ds/es, we only used these registers in protect mode.
	mov ax, 0x30
	mov ds, ax
	mov es, ax
	; Clear PM bit.
	mov eax, cr0
	and al, 0xFE
	mov cr0, eax
	jmp SEGMENT_OF_NEW_LOADER : gl_code16_begin

	align 4
gl_code16_begin:
    mov ax, SEGMENT_OF_NEW_LOADER
	mov ds, ax
	mov es, ax
	; Just return the address where protect mode
	; is enter first.
	jmp gl_return_from_protect

	; End of code 16 section back from protect mode.
end_code16_from_pm:
    times 8*1024 - ($ - $$) db 0x00

;;------------------------------------------------------------------------
;; A small code segment to initializes ds/es... registers after transmit
;; to protect mode, before jump to mini-kernel. It corresponds the macro
;; CODE32_INIT_PROTECT_ENV.
;; Each module, must prepare an full environment to support other module's
;; running.
;;------------------------------------------------------------------------
	
	align 4
	bits 32
gl_init_protect_env:
    mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Now just to mini-kernel's start address.
	jmp dword 0x08 : __MINIKER_RUNNING_ADDRESS

	; End of protect mode init code section.
end_init_protect_env:
    times 16*1024 - ($ - $$) db 0x00
