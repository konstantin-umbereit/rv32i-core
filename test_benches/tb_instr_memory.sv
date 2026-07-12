/* test_benches/tb_instr_memory.sv
 *
 * Test bench for rtl/instr_memory.sv
 */

 module tb_instr_memory;
    logic        rst_n;        /* active-low asynchronous reset */
    logic [31:0] pc;           /* PC */

    logic [31:0] instr;        /* instruction output */

    instr_memory dut (.*);

    initial begin
        $dumpfile("waveforms/tb_instr_memory.vcd");
        $dumpvars(0, tb_instr_memory);

        /* Memory initialisation */
        for (int i = 0; i < dut.MEM_SIZE; i++) begin
            dut.mem[i] = 8'b0;
        end

        dut.mem[32'h0000_0000] = 8'hEF;
        dut.mem[32'h0000_0001] = 8'hBE;
        dut.mem[32'h0000_0002] = 8'hAD;
        dut.mem[32'h0000_0003] = 8'hDE;

        dut.mem[32'h0000_0004] = 8'hEF;
        dut.mem[32'h0000_0005] = 8'hBE;
        dut.mem[32'h0000_0006] = 8'hAD;
        dut.mem[32'h0000_0007] = 8'hDE;

        dut.mem[32'h0000_00c0] = 8'hEF;
        dut.mem[32'h0000_00c1] = 8'hBE;
        dut.mem[32'h0000_00c2] = 8'hAD;
        dut.mem[32'h0000_00c3] = 8'hDE;

        /* Change program counter */
        #10 rst_n = 1;              /* release reset */
        #10 pc = 32'h0000_0000;     /* get first    32bit instruction */
        #10 pc = 32'h0000_0004;     /* get second   32bit instruction */
        #10 pc = 32'h0000_00c0;     /* get a target 32bit instruction */
        #10 rst_n = 0;              /* push reset */

        #10 $finish;

    end

    initial $monitor("$time=0d%0t rst_n=0b%b pc=0h%h| instr=0h%h",
                      $time, rst_n, pc, instr); 

 endmodule

