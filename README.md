RV32I-Assembler (written in C) + single-cycle CPU


1. SUPPORTED INSTRUCTIONS

 Base instructions: 
    - add, sub, and, or, xor, sll, srl, sra, slt, sltu
    - addi, andi, ori, xori, slli, srli, srai, slti, sltiu,
      lb, lh, lw, lbu, lhu, jalr, ecall, ebreak
    - sb, sh, sw,
    - beq, bne, blt, bge, bltu, bgeu,
    - jal
    - lui, auipic

 Pseudo instructions:
    - nop, mv, li, la, j, ret, call, tail

 Directives:
    - .text, .data, .globl / .global, .align, .word, .half, .byte,
      .ascii, .asciz

2. USAGE ASSEMBLER
    1. Change directory to /assembler
    2. 'make'
    3. './assembler /path_to_source_file.s'

3. CPU
    -- work in progress --
