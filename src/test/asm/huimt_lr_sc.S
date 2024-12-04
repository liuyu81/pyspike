.section .text, "ax", @progbits
.align 3
.globl _start

_start:
    # test lr.w / sc.w
    la      t0, var1
    lr.w    t1, (t0)
    sc.w    t2, t1, (t0)
    # test lr.d / sc.d
    la      t0, var2
    lr.d    t1, (t0)
    sc.d    t2, t1, (t0)
    # test lr.w / sc.d
    la      t0, var1
    lr.w    t1, (t0)
    sc.d    t2, t1, (t0)
    # test lr.d / sc.w
    la      t0, var2
    lr.d    t1, (t0)
    sc.w    t2, t1, (t0)
    j exit

exit:
    la a0, tohost
    li a1, 1
    sd a1, 0(a0)
    auipc t0, 0
    jr 4(t0)

.section .data, "aw", @progbits
.align 3
var0: .word 0xffffffff80025059
var1: .word 0xffffffff80025058
var2: .dword 0xffffffff80025057

.section .tohost, "aw", @progbits
.align 3
.globl tohost

tohost:
    .dword 0

.align 3
.globl fromhost

fromhost:
    .dword 0
