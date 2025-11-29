# Stage 3: Seccomp

## Setup

`g++ -o pdf_parser main.cpp`

Seccomp allows to restrict the syscalls a process can make. This reduces the surface of attack of the kernel by preventing malicious code from executing arbitrary system calls.

If an attacker successfully exploits a buffer overflow in the pdf_parser, they typically try to execute shellcode (e.g., to spawn /bin/sh). However, if the syscalls required by the shellcode are blocked by Seccomp, the kernel will kill the process, preventing further exploitation.

## Attack

The vulnerability lies in the limitations of Seccomp. When a syscall like open("/etc/passwd", ...) is called, the register contains a memory address (a pointer), not the string "/etc/passwd" itself.

Because Seccomp cannot dereference memory to check what that pointer actually holds, it can't prevent access to arbitrary files, it can only allow or deny usage of a syscall.