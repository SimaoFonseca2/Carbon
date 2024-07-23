global _start
section .text
_start:
     mov rax, 5
       push rax
    mov rax, 60
    mov rdi, 0
    syscall
section .data
   
section .bss
   num_str resb 20
