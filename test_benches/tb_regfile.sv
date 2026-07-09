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
        
        @(posedge clk);                      /* release reset */
        rst_n = 1;
        @(posedge clk);                           
        we = 1; rd = 5; wd = 32'hDEADBEEF;   /* write to x5 */
        @(posedge clk);
        we = 0; rs1 = 5; rs2 = 0;            /* read x5 and x0 */
        @(posedge clk);                      
        rst_n = 0;                           /* push reset */
        @(posedge clk);
        $finish;
    end 

    initial $monitor("Time=0d%0t rd1=0x%h rd2=0x%h", $time, rd1, rd2);

endmodule
