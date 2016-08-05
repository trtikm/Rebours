        section .text
        global _start

_start:
        mov rax,arg0
        mov rbx,arg1
        mov rcx,4
        jmp swap

exit:
        mov eax,1       ;system call number (sys_exit)
        int 0x80        ;call kernel

swap:
        mov dl, [rax]
        mov dh,[rbx]
        mov [rax],dh
        mov [rbx],dl
        inc rax
        inc rbx
        dec rcx
        jz exit
        jmp swap


        section .data

arg0    db 0x0a, 0x0b, 0x0c, 0x0d
arg1    db 0x01, 0x02, 0x03, 0x04

