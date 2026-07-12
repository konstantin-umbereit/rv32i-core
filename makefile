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

tb_wb_mux:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL --Wno-SYNCASYNCNET \
		--top-module tb_wb_mux \
		rtl/wb_mux.sv test_benches/tb_wb_mux.sv \
		&& ./obj_dir/Vtb_wb_mux

tb_branch:
	rm -rf obj_dir && \
	verilator --cc --exe --build -Wall --trace --timing --main \
		--Wno-BLKSEQ --Wno-UNUSEDSIGNAL --Wno-SYNCASYNCNET \
		--top-module tb_branch \
		rtl/branch.sv test_benches/tb_branch.sv \
		&& ./obj_dir/Vtb_branch




