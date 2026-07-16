/* test_benches/tb_rv32i_core.sv
 *
 * Test bench for rtl/rv32i_core.sv
 */

 module tb_rv32i_core;
    logic clk = 0;
    logic rst_n = 0;

    rv32i_core dut (.*);

    always #5 clk = ~ clk;

    always  @(posedge clk) begin
        if (dut.halt) begin
            $write("\nProgram halted: ecall/ebreak detected\n\n");
            $finish;
        end
    end

    initial begin
        $dumpfile("waveforms/tb_rv32i_core.vcd");
        $dumpvars(0, tb_rv32i_core);


        /* Instruction Memory Initialisation */
        $readmemh("test_programs/test_ebreak.hex", dut.instr_memory_init.mem);
        #10

        /* Data Memory Initialisation */
        for (int i = 0; i < dut.data_memory_init.MEM_SIZE; i++) begin
                    dut.data_memory_init.mem[i] = 8'b0;
        end
        dut.data_memory_init.mem[0]  = 8'hEF;   
        dut.data_memory_init.mem[1]  = 8'hBE;
        dut.data_memory_init.mem[2]  = 8'hAD;
        dut.data_memory_init.mem[3]  = 8'hDE;
        dut.data_memory_init.mem[4]  = 8'hDD;   
        dut.data_memory_init.mem[5]  = 8'hCC;
        dut.data_memory_init.mem[6]  = 8'hBB;
        dut.data_memory_init.mem[7]  = 8'hAA;

        /* Release reset and start the program */
        rst_n = 1; $write("\n// Release reset, start program\n");

        repeat(10) @(posedge clk);
        
         
        #1
        $write("\n// Push and release reset => restart program (only the first three instructions)");
        rst_n = 0; 
        #1 
        rst_n = 1; 
        
        @(posedge clk);

        
        $finish;
    end

    initial $monitor("\nTime=0d%0t rst_n=0b%b  pc=0x%h instr=0x%h \nregs[0]=0x%h regs[1]=0x%h regs[2]=0x%h regs[3]=0x%h \ndata[0]=0x%h | data[4]=0x%h\ndata[1]=0x%h | data[5]=0x%h\ndata[2]=0x%h | data[6]=0x%h\ndata[3]=0x%h | data[7]=0x%h",
                     $time, rst_n, dut.pc, dut.instr, dut.regfile_init.regs[0], dut.regfile_init.regs[1], dut.regfile_init.regs[2], dut.regfile_init.regs[3], 
                     dut.data_memory_init.mem[0], dut.data_memory_init.mem[4], dut.data_memory_init.mem[1], dut.data_memory_init.mem[5],
                     dut.data_memory_init.mem[2], dut.data_memory_init.mem[6], dut.data_memory_init.mem[3], dut.data_memory_init.mem[7]);
 endmodule
