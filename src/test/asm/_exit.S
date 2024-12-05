exit:
    la a0, tohost
    li a1, 1
    sd a1, 0(a0)
    auipc t0, 0
    jr 4(t0)

.section .tohost, "aw", @progbits
.align 3
.globl tohost

tohost:
    .dword 0

.align 3
.globl fromhost

fromhost:
    .dword 0
