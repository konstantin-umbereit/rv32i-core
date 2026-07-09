module tb_pc;

    logic clk = 0;
    logic rst_n =0;
    logic [31:0] pc_next;
    logic [31:0] pc_out;

    pc dut (.*);

    always #5 clk = ~clk;

    initial begin
        $dumpfile("waveforms/tb_pc.vcd");
        $dumpvars(0, tb_pc);

        @(posedge clk);
        rst_n = 1;                  /* release reset */
        @(posedge clk);
        pc_next = pc_next + 4;      /* increase PC by 4 */
        @(posedge clk);
        pc_next = 32'h0000_1000;    /* set PC to target */
        @(posedge clk);
        pc_next = pc_next + 4;      /* increase PC by 4 */
        @(posedge clk);
        rst_n = 0;                  /* push reset */
        @(posedge clk);
        $finish;
    end

    initial $monitor("Time=0d%0t pc_out=0x%0h", $time, pc_out);
endmodule
