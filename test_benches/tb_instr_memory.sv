/* test_benches/tb_instr_memory.sv
 *
 * Test bench for rtl/instr_memory.sv
 */

 module tb_instr_memory;
    logic [31:0] pc;           /* PC */

    logic [31:0] instr;        /* instruction output */

    instr_memory dut (.*);

    initial begin
        $dumpfile("waveforms/tb_instr_memory.vcd");
        $dumpvars(0, tb_instr_memory);

        /* Memory Initialisation */
        for (int i = 0; i < dut.MEM_SIZE / 4; i = i + 4) begin
            dut.mem[i] = 32'b0;
        end
        
        dut.mem[32'h0000_0000] = 32'hDEAD_BEEF;
        dut.mem[32'h0000_0001] = 32'HBEEF_DEAD;
        dut.mem[32'h0000_0030] = 32'hAABB_CCDD;
        

        /* Change program counter */
        #10 pc = 32'h0000_0000;     /* get first    32bit instruction */
        #10 pc = 32'h0000_0004;     /* get second   32bit instruction */
        #10 pc = 32'h0000_00c0;     /* get a target 32bit instruction */

        #10 $finish;

    end

    initial $monitor("$time=0d%0t pc=0h%h| instr=0h%h",
                      $time, pc, instr); 

 endmodule

