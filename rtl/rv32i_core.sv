/* rtl/rv32i_core.sv
 *
 * Single-cycle RV32I top module
 */

 module rv32i_core #(
    parameter DATA_WIDTH = 32      
 ) (
    input logic clk,                      /* rising edge clock                         */
    input logic rst_n                     /* active-low asynchronous reset             */
 );
    
    logic [DATA_WIDTH-1:0] pc;            /* pc            -> instr_mem, 4adder        */

    logic [DATA_WIDTH-1:0] pc_plus4;      /* 4adder        -> pc_next_mux              */

    logic [DATA_WIDTH-1:0] pc_next;       /* pc_next_mux   -> pc                       */

    logic [DATA_WIDTH-1:0] instr;         /* instr_memory  -> control, regfile, extend */

    logic [2:0]             result_src;   /* control       -> result_mux               */
    logic                   alu_src;      /* control       -> alu_src_mux              */
    logic [2:0]             imm_src;      /* control       -> extender                 */
    logic                   reg_write;    /* control       -> regfile                  */
    logic                   mem_write;    /* control       -> data_memory              */
    logic [2:0]             data_mask;    /* control       -> data_memory              */
    logic                   take_branch;  /* control       -> branch                   */
    logic                   jal;          /* control       -> branch                   */
    logic                   jalr;         /* control       -> branch                   */
    logic [3:0]             alu_ctrl;     /* control       -> alu                      */
    logic                   halt;         /* control       -> pc                       */

    logic [DATA_WIDTH-1:0] result;        /* result_mux    -> regfile                  */

    logic [DATA_WIDTH-1:0] rd1;           /* regfile       -> alu                      */
    logic [DATA_WIDTH-1:0] rd2;           /* regfile       -> alu_src_mux, data_memory */

    logic [DATA_WIDTH-1:0] imm_ext;       /* extend        -> alu_src_mux              */

    logic [DATA_WIDTH-1:0] src_b;         /* alu_src_mux   -> alu                      */

    logic [DATA_WIDTH-1:0] alu_result;    /* alu           -> data_memory, result_mux  */
    logic                   zero;         /* alu           -> control                  */

    logic [DATA_WIDTH-1:0] load_data;     /* data_memory   -> load_extender              */
    logic [DATA_WIDTH-1:0] load_data_ext; /* load_extender -> result_mux               */
    
    logic [DATA_WIDTH-1:0] pc_target;     /* branch        -> pc_next_mux              */
    logic                  pc_src;        /* branch        -> pc_next_mux              */     
    
    /* Program Counter */
    pc pc_init (
        .clk            (clk),
        .rst_n          (rst_n),
        .halt           (halt),
        .pc_next        (pc_next),
        
        .pc_out         (pc)
    );

    /* 4adder */
    assign pc_plus4 = pc + 4;

    /* pc_next_mux*/
    assign pc_next = (pc_src) ? pc_target : pc_plus4;

    /* Instruction Memory */
    instr_memory instr_memory_init(
        .pc             (pc),

        .instr          (instr)
    );

    /* Control Unit */
    control  control_init (
        .op             (instr[6:0]),
        .funct3         (instr[14:12]),
        .funct7         (instr[31:25]),
        .zero           (zero),

        .result_src     (result_src),
        .alu_src        (alu_src),
        .imm_src        (imm_src),
        .reg_write      (reg_write),
        .mem_write      (mem_write),
        .data_mask      (data_mask),
        .take_branch    (take_branch),
        .jal            (jal),
        .jalr           (jalr),
        .alu_ctrl       (alu_ctrl),
        .halt           (halt)
    );

    /* Regfile */
    regfile regfile_init (
        .clk            (clk),
        .rst_n          (rst_n),
        .rs1            (instr[19:15]),
        .rs2            (instr[24:20]),
        .we             (reg_write),
        .rd             (instr[11:7]),
        .wd             (result),

        .rd1            (rd1),
        .rd2            (rd2)

    );

    /* Imm Extender */
    extend extend_init (
        .imm_src        (imm_src),
        .instr          (instr),

        .imm_ext        (imm_ext)
    );

    /* alu_src_max */
    assign src_b = (alu_src) ? imm_ext : rd2;

    /* ALU */
    alu alu_init (
        .alu_ctrl       (alu_ctrl),
        .src_a          (rd1),
        .src_b          (src_b),

        .alu_result     (alu_result),
        .zero           (zero)
    );

    /* Data Memory */
    data_memory data_memory_init (
        .clk            (clk),
        .we             (mem_write),
        .data_mask      (data_mask[1:0]),
        .alu_result     (alu_result),
        .wd             (rd2),

        .load_data      (load_data)
    );

    /* result_mux */
    always_comb begin
        case (result_src)
            3'b000: result = alu_result;
            3'b001: result = load_data_ext;
            3'b010: result = imm_ext;
            3'b011: result = pc_plus4;
            3'b100: result = pc_target;
            default: result = alu_result;
        endcase
    end

    /* Branch Unit */
    branch branch_unit (
        .pc             (pc),
        .imm_ext        (imm_ext),
        .rs1            (rd1),
        .jal            (jal),
        .jalr           (jalr),
        .take_branch    (take_branch),

        .pc_target      (pc_target),
        .pc_src         (pc_src)
    );

    load_extend load_extend_init (
        .load_data(load_data),
        .data_mask(data_mask),

        .load_data_ext(load_data_ext)
    );

 endmodule

