        section .text write
        global _start
block:                          ; 400080
        inc rbx
        mov rax,loop
        jmp rax
_start:                         ; 40008f
        mov rcx,0
        mov rbx,0
loop:                           ; 400099
        inc rcx
        mov rdx,0
        mov rax,rcx
        mov rdi,10
        div rdi
        mov rcx,rdx
        mov rdi,array
        add rdi,rcx
        mov rdx,0
copy:                           ; 4000c1
        mov al,[rdx+block]
        mov [rdi+rdx],al
        inc rdx
        cmp rdx,_start-block
        jnz copy
        jmp rdi
array:                          ; 4000d5
        nop
        nop
        nop
        nop
        nop
        nop
        nop
       ;...

