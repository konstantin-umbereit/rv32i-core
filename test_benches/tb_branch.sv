/* test_benches/tb_control.sv
 *
 * Test bench for rtl/control.sv
 */

 module tb_branch;

    logic [31:0] pc;           /* current pc (not increased by 4!) */
    logic [31:0] imm_ext;      /* output from immediate extender */
    logic [31:0] rs1;        

    logic        jal;          /* from control modul */
    logic        jalr;         /* from control modul */
    logic        take_branch;  /* from control modul */

    logic [31:0] pc_target;    /* branch/jump target address */
    
    logic        pc_src;       /* src for next PC value */

    branch dus(.*);

    initial begin
        $dumpfile("waveforms/tb_branch.vcd");
        $dumpvars(0, tb_branch);

        #10 pc = 32'h0000_0000; imm_ext = 32'h0000_0001; rs1 = 32'h0000_0002;  /* no jump/branch */
            jal = 0; jalr = 0; take_branch = 0;

        #10 pc = 32'h0000_0000; imm_ext = 32'h0000_0001; rs1 = 32'h0000_0002;  /* jal */
            jal = 1; jalr = 0; take_branch = 0;

        #10 pc = 32'h0000_0000; imm_ext = 32'h0000_0001; rs1 = 32'h0000_0002;  /* jalr */
            jal = 0; jalr = 1; take_branch = 0;

         #10 pc = 32'h0000_0000; imm_ext = 32'h0000_0001; rs1 = 32'h0000_0002; /* branch */
            jal = 0; jalr = 0; take_branch = 1;

        #10 $finish;
    end

    initial $monitor("Time=0d%0t pc=0x%h imm_ext=0x%h rs1=0x%h jal=0b%b jalr=0b%b take_branch=0b%b | pc_target=0x%h pc_src=0b%b", 
                     $time, pc, imm_ext, rs1, jal, jalr, take_branch, pc_target, pc_src);
 endmodule
