EXTRN PASCAL DOSIODELAYCNT:ABS

.586p
.MODEL FLAT,SYSCALL

.DATA
idtSave DF 0
ALIGN 4
OldMFHandler DF 0


.CODE
ASSUME CS:NOTHING

IRQDisable PROC PUBLIC
    pushfd
    pop eax
    cli
    ret
IRQDisable ENDP

IRQEnable PROC PUBLIC,flags:DWORD
    mov  eax,flags
    test   eax,0200h
    jz short @F
    sti
@@:
    ret
IRQEnable ENDP

IoDelay PROC PUBLIC
    mov eax,DOSIODELAYCNT
ALIGN 4
@@:
    dec eax
    jnz short @B
    ret
IoDelay ENDP

RESUMEFLAG 	EQU 010000h
OPSIZEBIT   EQU 400000h

NewMFHandler PROC
   mov eax,ss                       ; check stack selector if 16-bit segmented stack or 32-bit flat stack
   lar eax,eax
   jnz short skip
   test eax,OPSIZEBIT
   jnz short @F
   and esp,0FFFFh                   ; 16-bit stack (SP) in use, make sure we can use the 32-bit ESP (by zeroing out upper 16 bits)
@@:
   or DWORD PTR [esp+8],RESUMEFLAG  ; set resume flag in EFLAGS image on stack in order to prevent spurious #DB exception
                                    ; in case of simultaneous #DB and #MF exception
                                    ; see Intel manual, chapter 17.3.1.1 "Instruction-Breakpoint Exception Condition"
                                    ; (<quote>... all fault-class exception handlers should set the RF flag in the EFLAGS image</quote>)
skip:
   fnclex                           ; clear all pending exceptions
   push 0                           ; push dummy error code on stack
                                    ; as the original OS/2 #MF handler of the Warp4 kernel
                                    ; erroneously expects an error code to exist on the stack
   jmp  FWORD PTR cs:[OldMFHandler] ; branch to original handler
NewMFHandler ENDP

ReplaceMFHandler PROC PUBLIC USES ebx
   pushfd
   cli                              ; disable interrupts
   sidt [idtSave]
   mov  ebx,DWORD PTR [idtSave+2]   ; load IDT base address into EBX
   add  ebx,16*8                    ; compute address of IDT entry 16 = #MF exception descriptor

   lock and DWORD PTR [ebx+4],0FFFF70FFh ; clear PRESENT bit and type field in descriptor

   mov  edx,[ebx]                   ; lower 4 bytes of #MF descriptor in edx
   mov  eax,[ebx+4]                 ; upper 4 bytes of #MF descriptor in eax
   mov  ax,dx                       ; copy offset 0..15 to lower 16-bit of eax, completing offset 16..31
   shr  edx,16                      ; dx now contains selector
   mov  DWORD PTR [OldMFHandler],eax    ; save 32-bit offset in lower 4 bytes of OldMFHandler
   mov  WORD PTR  [OldMFHandler + 4],dx ; save 16-bit selector in upper 2 bytes of OldMFHandler
   mov  eax,cs
   shl  eax,16                      ; new selector into the upper 16 bits of first DWORD	
   lea  edx,FLAT:NewMFHandler 
   mov  ax,dx                       ; new lower 16 offset bits into lower 16 bits of first DWORD
   mov  ecx,[ebx+4]                 ; load second DWORD of descriptor
   and  ecx,0FFFFh                  ; zero out upper 16 bits in second DWORD
   xor  dx,dx                       ; zero out lower 16 offset bits
   or    ecx,edx                    ; new upper 16 offset bits into upper 16 bits of second DWORD
   mov  [ebx],eax                   ; overwrite first DWORD in IDT
   mov  [ebx+4],ecx                 ; overwrite second DWORD in IDT

   lock or DWORD PTR [ebx+4],000008F00h ; set PRESENT bit and set type to 32-bit trap gate

   xor eax,eax
   cpuid                            ; serialize instruction stream
   pop  eax
   test eax,0200h                   ; reenable interrupts if they where already enabled
   jz short @F
   sti
@@:
   ret
ReplaceMFHandler ENDP

END
