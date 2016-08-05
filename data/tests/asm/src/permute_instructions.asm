        section .text write
        global _start
_start:                     ; 400080
        mov rax,1
        mov rcx,2
loop:                       ; 40008a
        add rax,rcx         ; 48 01 c8    40008a
        mul rcx             ; 48 f7 e1    40008d
        sub rax,rcx         ; 48 29 c8 	  400090

        mov bl,[loop+1]     ; 400093
        mov bh,[loop+4]
        mov [loop+4],bl
        mov bl,bh
        mov bh,[loop+7]
        mov [loop+7],bl
        mov [loop+1],bh

        mov bl,[loop+2]
        mov bh,[loop+5]
        mov [loop+5],bl
        mov bl,bh
        mov bh,[loop+8]
        mov [loop+8],bl
        mov [loop+2],bh

        jmp loop

