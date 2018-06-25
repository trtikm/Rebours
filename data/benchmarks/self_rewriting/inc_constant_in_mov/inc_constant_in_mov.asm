        section .text write
        global _start
_start:
        mov rax,0x0000000000000000
        inc rax
modify:
        mov rbx,modify+2
        mov [cs:rbx],rax
        jmp _start
        ret

