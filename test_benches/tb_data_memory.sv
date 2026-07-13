/* test_benches/tb_data_memory.sv
 *
 * Test bench for rtl/data_memory.sv
 */

 module tb_data_memory;
    logic        clk   = 0; 
    logic        we    = 0;           
    logic [1:0]  data_mask;
    logic [31:0] alu_result;  
    logic [31:0] wd;  

    logic [31:0] read_data;

    data_memory dut (.*);

    always #5 clk = ~clk;
    initial begin
        $dumpfile("waveforms/tb_data_memory.vcd");
        $dumpvars(0, tb_data_memory);

        /* Memory Initialization */
        for (int i = 0; i < dut.MEM_SIZE; i++) begin
                    dut.mem[i] = 8'b0;
        end

        @(posedge clk);
        we = 1; data_mask = 2'b00; alu_result = 32'h0000_0000; wd = 32'hDEAD_BEEF; /* write byte, write enabled */
        @(posedge clk);
        we = 0; data_mask = 2'b00; alu_result = 32'h0000_0000; wd = 32'hBEEF_DEAD; /* write byte, write disabled */
        @(posedge clk);
        we = 1; data_mask = 2'b01; alu_result = 32'h0000_0004; wd = 32'hDEAD_BEEF; /* write half, write enabled */
        @(posedge clk);
        we = 0; data_mask = 2'b01; alu_result = 32'h0000_0004; wd = 32'hBEEF_DEAD; /* write half, write disabled */
        @(posedge clk);
        we = 1; data_mask = 2'b10; alu_result = 32'h0000_0008; wd = 32'hDEAD_BEEF; /* write half, write enabled */
        @(posedge clk);
        we = 0; data_mask = 2'b10; alu_result = 32'h0000_0008; wd = 32'hBEEF_DEAD; /* write byte, write disabled */

        $finish;

    end

    initial $monitor("$time=0d%0t we=0b%b data_mask=0b%b alu_result=0h%h wd=0h%h | read_data=0h%h",
                      $time, we, data_mask, alu_result, wd, read_data); 

 endmodule
