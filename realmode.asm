.286p
.MODEL LARGE,C
.686p

.CODE

QueryVideoMode PROC
   push bp
   mov  ah,0fh
   int  10h
   movzx eax,al
   pop  bp
   ret
QueryVideoMode ENDP

SetVideoMode PROC
   push bp
   mov  al,[si]
   xor  ah,ah
   int  10h
   xor  eax,eax
   pop  bp
   ret
SetVideoMode ENDP

ClearScreen PROC
   push bp
   push bx
   xor  al,al
   mov  ah,05h
   int 10h	; select first screen page
   xor al,al
   xor cx,cx
   mov dh,[si]
   dec dh
   mov dl,[si+4]
   dec dl
   mov bh,0fh
   mov ah,06h
   int 10h	; fully scroll screen up (black background, white foreground)
   xor bh,bh
   xor dx,dx
   mov ah,02h
   int 10h	; set cursor to upper left corner
   xor eax,eax
   pop  bx
   pop  bp
   ret
ClearScreen ENDP

WriteString PROC
   push bp
   push bx
   push es
   xor  bh,bh
   xor  dx,dx
   mov  bl,0fh
   mov  cx,[si]
   mov  ax,ds
   mov  es,ax
   lea  bp,[si+4]
   mov  al,1
   mov  ah,13h
   int  10h	; write string (at dh,dl)
   xor  eax,eax
   pop  es
   pop  bx
   pop  bp
   ret
WriteString ENDP

END

