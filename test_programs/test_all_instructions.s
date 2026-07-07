.text
.global main
.globl another_label

main:
    nop
    mv      x1, x2
    li      x3, 0x12345678
    la      x4, data_label
    j       loop
    call    subroutine
    tail    exit
    ret

    addi    x5, x0, 42
    andi    x6, x1, -1
    ori     x7, x2, 0xFF
    xori    x8, x3, 0xAB
    slli    x9, x4, 3
    srli    x10, x5, 7
    srai    x11, x6, 2
    slti    x12, x7, 100
    sltiu   x13, x8, 200

    lb      x14,  0(x4)
    lh      x15,  4(x4)
    lw      x16,  8(x4)
    lbu     x17, 12(x4)
    lhu     x18, 16(x4)
    jalr    x19, 0(x1)

    sb      x20, 20(x4)
    sh      x21, 22(x4)
    sw      x22, 24(x4)

    beq     x1, x2, exit
    bne     x3, x0, loop
    blt     x4, x5, exit
    bge     x6, x7, loop
    bltu    x8, x9, exit
    bgeu    x10, x11, loop

    lui     x23, 0x12345
    auipc   x24, 0x6789A
    jal     x25, loop

    ecall
    ebreak

    add     x26, x27, x28
    sub     x29, x30, x31
    and     x1,  x2,  x3
    or      x4,  x5,  x6
    xor     x7,  x8,  x9
    sll     x10, x11, x12
    srl     x13, x14, x15
    sra     x16, x17, x18
    slt     x19, x20, x21
    sltu    x22, x23, x24

loop:
    ret

subroutine:
    ret

exit:
    ret

another_label:

.data
data_label:
    .word   0xdeadbeef, 0xcafebabe
    .half   0x1234, 0x5678
    .byte   0xff, 0x00, 0x42
    .asciz  "Hello, RV32I!"
    .ascii  "rawbytes"
    .align  3
    .word   0x11223344
