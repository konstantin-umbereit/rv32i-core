# test_programs/test_arithmetic.s
# 
# Tests R and I format instructions that
# perform basic arithmetic and logical operations.
# (add, sub, xor, or, and, sll, sra, srl, rlt, sltu)
# (addi, xori, ori, andi, slli, srli, srai, slti, sltiu)

addi x1, x0, 3
addi x2, x0, 5

add x3, x2, x1
sub x3, x2, x1
xor x3, x2 ,x1
or x3, x2, x1
and x3, x2, x1
sll x3, x2 ,x1
sra x3, x2, x1
srl x3, x2, x1
slt x3, x1, x2
sltu x3, x2 ,x1
nop
xori x3, x2, 3
ori x3, x2, 3
andi x3, x2 ,3
slli x3, x2, 3
srli x3, x2, 3
srai x3, x2, 3
slti x3, x2, 3
sltiu x3, x2, 3