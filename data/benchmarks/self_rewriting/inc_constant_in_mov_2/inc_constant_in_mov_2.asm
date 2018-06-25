        section .text write
        global _start
_start:
        mov rcx,1000
loop:
        mov rax,0x0000000000000000
        inc rax
        mov [loop+2],rax
        cmp rax,rcx
        jnz loop
        mov rax,0
        add rcx,100
        jmp loop

