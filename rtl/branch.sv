/* rtl/branch.sv
 *
 * Single-cycle RV32I control unit
 */

module branch #(
    parameter DATA_WIDTH = 32
) ( 
    input  logic [DATA_WIDTH-1:0] pc,           /* current pc (not increased by 4!) */
    input  logic [DATA_WIDTH-1:0] imm_ext,      /* output from immediate extender */
    input  logic [DATA_WIDTH-1:0] rs1,        

    input  logic                  jal,          /* from control modul */
    input  logic                  jalr,         /* from control modul */
    input  logic                  take_branch,  /* from control modul */
    
    output logic [DATA_WIDTH-1:0] pc_target,    /* branch/jump target address */
    
    output logic                  pc_src        /* src for next PC value */
);

    /* PC_TARGET */
    always_comb begin
        if(take_branch || jal) pc_target = pc + imm_ext;  /* branch permitted or jal */
        else pc_target = rs1 + imm_ext;                            /* jalr */
    end 

    /* PC_SRC */
    always_comb begin
        if(jal || jalr || take_branch) pc_src = 1;    /* = pc_target */
        else                           pc_src = 0;    /* = pc_plus4 */ 
    end
endmodule


