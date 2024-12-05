.section .text, "ax", @progbits
.align 3
.globl _start

_start:
    li t0, 0xffffffff
    csrw 0x7c0, t0
    csrr t1, 0x7c0
    j exit

#include "_exit.S"
