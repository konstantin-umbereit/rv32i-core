# Assembler

CC = gcc
CFLAGS = -Wall -Wextra -std=c11

SRCS = assembler_src/main.c \
       assembler_src/lexer.c \
       assembler_src/parser.c\
       assembler_src/emitter.c\
       assembler_src/symbol_table.c\
       assembler_src/rv32i.c

assembler: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o assembler
	
assembler_clean:
	rm -f assembler

.PHONY: clean

# Test benches

tb_regfile:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ \
		--top-module tb_regfile \
		rtl/regfile.sv test_benches/tb_regfile.sv \
		&& ./obj_dir/Vtb_regfile

tb_pc:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ \
		--top-module tb_pc \
		rtl/pc.sv test_benches/tb_pc.sv \
		&& ./obj_dir/Vtb_pc

tb_extend:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL \
		--top-module tb_extend \
		rtl/extend.sv test_benches/tb_extend.sv \
		&& ./obj_dir/Vtb_extend

tb_alu:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL \
		--top-module tb_alu \
		rtl/alu.sv test_benches/tb_alu.sv \
		&& ./obj_dir/Vtb_alu



