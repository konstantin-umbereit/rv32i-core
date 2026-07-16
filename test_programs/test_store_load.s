# test_programs/test_store_load.s
# 
# Tests store and load instructions.
# (sb, sh, sw)
# (lb, lbu, lh, lhu, lw)
# 
# Note: first 4 bytes of data memory
# must be loaded with values
#

addi x1, x0, -1

lb x2, 1(x1)
lbu x2, 0(x0) 
lh x2, 1(x1) 
lhu x2, 0(x0) 
lw x3, 1(x0)

sb x2, 1(x1)
sh x2, 1(x1)
sw x2, 2(x1)


