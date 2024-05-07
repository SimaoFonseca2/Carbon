global _start
_start:
     mov rax, 3
       push rax
     mov rax, 2
       push rax
      mov rax, QWORD [rsp + 8], 
      mov QWORD [rsp + 0], rax
       push       QWORD [rsp + 0]

     mov rax, 60
       pop rdi
     syscall
     mov rax, 60
     mov rdi, 0
     syscall