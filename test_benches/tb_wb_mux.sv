/* test_benches/tb_wb_mux.sv
 *
 * Test bench for rtl/wb_mux.sv
 */

 module tb_wb_mux;
    logic [2:0]  result_src;   /* control from control unit */

    logic [31:0] alu_result;   /* output from alu */
    logic [31:0] result_data;  /* output from data memory */
    logic [31:0] imm_ext;      /* output from extender */
    logic [31:0] pc_target;    /* PC + imm_ext */
    logic [31:0] pc_plus4;     /* PC + 4 */

    logic [31:0] result;

    wb_mux dut (.*);

    initial begin
        $dumpfile("waveforms/tb_wb_mux.vcd");
        $dumpvars(0, tb_wb_mux);

        #10 result_src = 3'b000; alu_result  = 32'hDEAD_BEEF;
            {result_data, imm_ext, pc_target, pc_plus4}    = '0; 

        #10 result_src = 3'b001; result_data = 32'hDEAD_BEEF;
            {alu_result, imm_ext, pc_target, pc_plus4}     = '0; 

        #10 result_src = 3'b010; imm_ext     = 32'hDEAD_BEEF;
            {result_data, alu_result, pc_target, pc_plus4} = '0; 

        #10 result_src = 3'b011; pc_target   = 32'hDEAD_BEEF;
            {result_data, imm_ext, alu_result, pc_plus4}   = '0; 

        #10 result_src = 3'b100; pc_plus4 = 32'hDEAD_BEEF;
            {result_data, imm_ext, pc_target, alu_result}  = '0; 

        #10 $finish;
    end

    initial $monitor("Time=0d%0t result_src=0b%b alu_result=0h%h result_data=0h%h imm_ext=0h%h pc_target=0h%h pc_plus4=0h%h| result=0h%h",
                    $time, result_src, alu_result, result_data, imm_ext, pc_target, pc_plus4, result);


 endmodule 

