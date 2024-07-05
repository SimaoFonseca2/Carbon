global _start
_start:
     mov rax, 1
       push rax
      mov QWORD [rsp + 0], 22
     mov rax, 22
       push rax
       pop rax
      cmp QWORD [rsp + 0], rax
      jnz Label0
     mov rax, 23
       push rax
     mov rax, 23
       push rax
       pop rax
      cmp QWORD [rsp + 0], rax
      jnz Label1
    mov eax, 4
    mov ebx, 1
    mov ecx, msg0
    mov edx, 4
    int 0x80

    mov eax, 1
    xor ebx, ebx
    int 0x80
msg0 db 'nice', 0xA
       add rsp, 0
Label1:
       add rsp, 8
Label0:
     mov rax, 60
     mov rdi, 0
     syscall