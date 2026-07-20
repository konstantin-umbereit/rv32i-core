# RV32I-CORE MAKEFILE


# 1. TEST BENCHES


# 1.1 RISC-V Architectural Certification Tests (ACTs)

arch_tb_rv32i_core:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL --Wno-SYNCASYNCNET --Wno-UNOPTFLAT --Wno-WIDTHTRUNC\
		--top-module arch_tb_rv32i_core \
		rtl/alu.sv rtl/branch.sv rtl/control.sv rtl/data_memory.sv \
		rtl/extend.sv rtl/instr_memory.sv rtl/pc.sv rtl/regfile.sv rtl/load_extend.sv \
		rtl/rv32i_core.sv test_benches/arch_tb_rv32i_core.sv \
		&& ./obj_dir/Varch_tb_rv32i_core


# 1.2 Regular Top Module Testing

tb_rv32i_core:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL --Wno-SYNCASYNCNET --Wno-UNOPTFLAT --Wno-WIDTHTRUNC\
		--top-module tb_rv32i_core \
		rtl/alu.sv rtl/branch.sv rtl/control.sv rtl/data_memory.sv \
		rtl/extend.sv rtl/instr_memory.sv rtl/pc.sv rtl/regfile.sv rtl/load_extend.sv \
		rtl/rv32i_core.sv test_benches/tb_rv32i_core.sv \
		&& ./obj_dir/Vtb_rv32i_core			

# 1.3 Regular Sub Module Testing

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

tb_control:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL \
		--top-module tb_control \
		rtl/control.sv test_benches/tb_control.sv \
		&& ./obj_dir/Vtb_control

tb_data_memory:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL --Wno-SYNCASYNCNET \
		--top-module tb_data_memory \
		rtl/data_memory.sv test_benches/tb_data_memory.sv \
		&& ./obj_dir/Vtb_data_memory

tb_branch:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL --Wno-SYNCASYNCNET \
		--top-module tb_branch \
		rtl/branch.sv test_benches/tb_branch.sv \
		&& ./obj_dir/Vtb_branch

tb_instr_memory:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL --Wno-SYNCASYNCNET \
		--top-module tb_instr_memory \
		rtl/instr_memory.sv test_benches/tb_instr_memory.sv \
		&& ./obj_dir/Vtb_instr_memory

tb_load_extend:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL --Wno-SYNCASYNCNET \
		--top-module tb_load_extend \
		rtl/load_extend.sv test_benches/tb_load_extend.sv \
		&& ./obj_dir/Vtb_load_extend


# 2. SCRIPTS

# 2.1 bin2hex
#
# Writes the raw opcode bytes of an .bin file as hexadecimal
# in a .hex file.
# Replace '%' with the path to the .bin file.
# Requires python3.

%.hex: %.bin scripts/bin2hex.py
	scripts/bin2hex.py $< $@
	






