/* test_benches/tb_load_extend.sv
 *
 * Test bench for rtl/load_extend.sv
 */

module tb_load_extend;

logic [31:0] load_data;
logic [2:0]  data_mask;
logic [31:0] load_data_ext;

load_extend dut (.*);

initial begin
    $dumpfile("waveforms/tb_load_extend.vcd");
    $dumpvars(0, tb_load_extend);

    #10 data_mask = 3'b000; load_data = 32'hDEAD_BEEF; /* lb */
    #10 data_mask = 3'b001; load_data = 32'hDEAD_BEEF; /* lh */
    #10 data_mask = 3'b010; load_data = 32'hDEAD_BEEF; /* lw */
    #10 data_mask = 3'b100; load_data = 32'hDEAD_BEEF; /* lbu */
    #10 data_mask = 3'b101; load_data = 32'hDEAD_BEEF; /* lhu */

    $finish;
end

initial $monitor("Time=0d%0t | load_data_ext=0h%h",
                  $time, load_data_ext);
endmodule
