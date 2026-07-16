# test_programs/test_branch_jump.s
# 
# Tests branch and jump instructions.
# (beq, bne, blt, bge, bge, bltu, bgeu)
# (jal, jalr)

addi x1, x0, 1
addi x2, x0, 1
beq x1, x2, l_beq
nop
nop
addi x1, x0, 1
addi x2, x0, 2
l_beq:
addi x1, x0, 1
addi x2, x0, 2
bne x1, x2, l_bne
nop
nop
l_bne:
addi x1, x0, 1
addi x2, x0, 2
blt x1, x2, l_blt
nop
nop
l_blt:
addi x1, x0, 2
addi x2, x0, 2
bge x1, x2, l_bge
nop
nop
l_bge:
addi x1, x0, -1
addi x2, x0, -2
bltu x1, x2, l_bltu
nop
nop
l_bltu:
addi x1, x0, -2
addi x2, x0, -1
bgeu x1, x2, l_bgeu
nop
nop
l_bgeu:
jal x1, l_jal
nop
nop
l_jal:
jalr x1, 0(x1)
nop
nop

