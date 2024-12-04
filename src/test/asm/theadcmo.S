.section .text, "ax", @progbits
.align 3
.globl _start

_start:
    csrr t0, 0x7c0
    li t0, 0x00  # rd / x5
    li t1, 0x02  # rs1 / x6
    li t2, 0x01  # rs2 / x7
    // th.addsl t0, t1, t2, 2
    // 0b00000_10_00111_00110_001_00101_0001011
    .insn 4, 0x0473128b
    j exit

exit:
    la a0, tohost
    li a1, 1
    sd a1, 0(a0)
    auipc t0, 0
    jr 4(t0)

.section .data, "aw", @progbits
.align 3

.section .tohost, "aw", @progbits
.align 3
.globl tohost

tohost:
    .dword 0

.align 3
.globl fromhost

fromhost:
    .dword 0
