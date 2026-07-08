module tb_regfile;

    logic clk = 0;
    logic rst_n = 0;
    logic [4:0] rs1, rs2, rd;
    logic we;
    logic [31:0] wd;
    logic [31:0] rd1, rd2;

    regfile dut (.*);   

    always #5 clk = ~clk;   

    initial begin
        $dumpfile("waveforms/tb_regfile.vcd");
        $dumpvars(0, tb_regfile);

        #10 rst_n = 1;     /* release reset */
        #10 we = 1; rd = 5; wd = 32'hDEADBEEF;   /* write to x5 */
        #10 we = 0; rs1 = 5; rs2 = 0;            /* read x5 and x0 */
        #10 $finish;
    end 

    initial $monitor("Time=%0t rd1=%h rd2=%h", $time, rd1, rd2);

endmodule
