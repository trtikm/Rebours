; Questions: What is the value of PC after execution of 57 instructions?
;            How Microcode would look like after those 57 instructions?
;            How we identify PC in the produced Microcode (consider also a
;            case, when the number 10000000 inside the cmp instruction bellow
;            is replaced by 10000001)?

        section .text write
        global _start

_start:                     ; 400080
        mov rcx,0

loop:                       ; 400085
        nop
        nop
        add rcx,2
        cmp rcx,10000000    ; What happens, if we put here 10000001
        jnz loop
        mov rax,loop
        mov bx,0xffe0
        mov [rax],bx
        mov rax,continue
        jmp loop

continue:                   ; 4000b1
        ; ...               ; Here can be a LOT of code!!
        mov eax,1           ; System function: sys_exit
        int 0x80            ; Call the kernel

