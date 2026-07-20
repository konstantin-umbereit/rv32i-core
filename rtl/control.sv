/* rtl/alu.sv
 *
 * Single-cycle RV32I control unit
 */

 module control (

    /* instruction segments */
    input  logic [6:0]            op,
    input  logic [2:0]            funct3,
    input  logic [6:0]            funct7,

    /* control signal from ALU */
    input  logic                  zero,       /* = (alu_result !=0), (alu_result == 0) */ 

    /* control signals for TOP MODULE muxes*/
    output logic [2:0]            result_src, /* = alu_result, read_data, imm_extm, pc_plus4, pc_target*/
    output logic                  alu_src,    /* = rd2, imm_ext */

    /* control signals for EXTEND*/
    output logic [2:0]            imm_src,    /* I, S, B, U, J-type */

    /* control signals for REGFILE*/
    output logic                  reg_write,  /* write disabled, enabled */

    /* control signals for DATA MEMORY */
    output logic                  mem_write,  /* write disabled, enabled */
    output logic [2:0]            data_mask,  /* = [7:0], ([15:8],[7:0]), ([31:24],[23:16],[15:8],[7:0]) */

    /* control signals for branch unit */
    output logic                  take_branch,/* = branch condition not fulfilled, fulfilled */
    output logic                  jal,        /* = instruction is not jal, is jal */
    output logic                  jalr,       /* = instruction is not jalr, is jalr */
    
    /* control signal for program counter */
    output logic                  halt,

    /* control signal for ALU  */
    output logic [3:0] alu_ctrl                 /* = ADD,SUB,AND,OR,XOR,SLT,SLTU,SLL,SRL,SRA */

 );
    
    /* Main Decoder */
    always_comb begin
        case (op) 
            /* R-type */
            7'b0110011: begin
                result_src       = 3'b000;  /* = alu_result */
                alu_src          = 0;       /* = rd2 */
                imm_src          = 0;       /* no impact, because alu_src = 0; */
                reg_write        = 1;
                mem_write        = 0;
                data_mask        = 0;       /* no impact, because mem_write = 0 */
                take_branch      = 0;
                jal              = 0;
                jalr             = 0;
                halt             = 0;
            end 

            /* I-type: OP-IMM */
            7'b0010011: begin
                result_src       = 3'b000;  /* = alu_result */
                alu_src          = 1;       /* = imm_ext */
                imm_src          = 3'b000;  /* = I-type */
                reg_write        = 1;
                mem_write        = 0;
                data_mask        = 0;       /* no impact, because mem_write = 0 */
                take_branch      = 0;
                jal              = 0;
                jalr             = 0;
                halt             = 0;
            end 

            /* I-type: LOAD */
            7'b0000011: begin
                result_src       = 3'b001;  /* = read_data */
                alu_src          = 1;       /* = imm_ext */
                imm_src          = 3'b000;  /* = I-type */
                reg_write        = 1;
                mem_write        = 0;
                data_mask        = funct3;
                take_branch      = 0;
                jal              = 0;
                jalr             = 0;
                halt             = 0;
            end 

            /* I-type: JALR */
            7'b1100111: begin
                result_src       = 3'b011;  /* = pc_plus4 */
                alu_src          = 1;       /* = imm_ext */
                imm_src          = 3'b000;  /* = I-type */
                reg_write        = 1;
                mem_write        = 0;
                data_mask        = 0;       /* no impact, because mem_write = 0 */
                take_branch      = 0;
                jal              = 0;
                jalr             = 1;
                halt             = 0;
            end 

            /* I-type: SYSTEM(ecall/ebreak) (current handling: halts the programcounter) */
            7'b1110011: begin
                result_src       = 0;       /* no impact, because reg_write = 0; */
                alu_src          = 0;       /* no impact, because mem_write, reg_write = 0 and pc_src = 2'b00; */
                imm_src          = 3'b000;  /* no impact, because alu_src = 0; */
                reg_write        = 1;
                mem_write        = 0;
                data_mask        = 0;       /* no impact, because mem_write = 0 */
                take_branch      = 0;
                jal              = 0;
                jalr             = 0;
                halt             = 1;
            end 
            
            /* S-type */
            7'b0100011: begin
                result_src       = 0;       /* no impact, because reg_write = 0; */
                alu_src          = 1;       /* = imm_ext */
                imm_src          = 3'b001;  /* = S-type */
                reg_write        = 0;
                mem_write        = 1;
                data_mask        = funct3;
                take_branch      = 0;
                jal              = 0;
                jalr             = 0;
                halt             = 0;
            end 

            /* B-type */
            7'b1100011: begin
                result_src       = 0;       /* no impact, because reg_write = 0; */
                alu_src          = 0;       /* = rd2 */
                imm_src          = 3'b010;  /* = B-type */
                reg_write        = 0; 
                mem_write        = 0;   
                data_mask        = 0;       /* no impact, because mem_write = 0 */ 
                case (funct3)
                    3'b000:  take_branch =(zero) ? 1: 0; /* beq */
                    3'b001:  take_branch =(!zero) ? 1: 0; /* bne */
                    3'b100:  take_branch =(!zero) ? 1: 0; /* blt */
                    3'b101:  take_branch =(zero) ? 1: 0; /* bge */
                    3'b110:  take_branch =(!zero) ? 1: 0; /* bltu */
                    3'b111:  take_branch =(zero) ? 1: 0; /* bgeu */

                    default: take_branch =(zero) ? 1: 0; /* default = beq */
                endcase               
                jal              = 0;
                jalr             = 0; 
                halt             = 0;
            end 

            /* U-type: LUI */
            7'b0110111: begin
                result_src       = 3'b010;  /* = imm_ext */
                alu_src          = 0;       /* no impact because result_src = imm_ext, mem_write= 0 and pc_src = 2'b00 */
                imm_src          = 3'b011;  /* = U-type */
                reg_write        = 1;    
                mem_write        = 0;
                data_mask        = 0;       /* no impact, because mem_write = 0 */ 
                take_branch      = 0;
                jal              = 0;
                jalr             = 0;  
                halt             = 0;
            end

            /* U-type: AUIPC  */
            7'b0010111: begin
                result_src       = 3'b100;  /* = pc_target */
                alu_src          = 0;       /* no impact, because result_src = pc_target, mem_write 0 and pc_src = 2'b00*/
                imm_src          = 3'b011;  /* = U-type */
                reg_write        = 1;  
                data_mask        = 0;       /* no impact, because mem_write = 0 */  
                mem_write        = 0;   
                take_branch      = 0;
                jal              = 0;
                jalr             = 0;
                halt             = 0;
            end  

            /* J-type: JAL */
            7'b1101111: begin
                result_src       = 3'b011;  /* = pc_plus4 */
                alu_src          = 0;       /* no impact, because result_src = pc_plus4, mem_write = 0 and pc_src = 2'b01*/
                imm_src          = 3'b100;  /* = J-type */
                reg_write        = 1;   
                data_mask        = 0;       /* no impact, because mem_write = 0 */  
                mem_write        = 0;
                take_branch      = 0;
                jal              = 1;
                jalr             = 0;
                halt             = 0;
            end 

            default: begin
                result_src       = 0;       /* no impact, because reg_write = 0 */
                alu_src          = 0;       /* no impact, because reg_write, mem_write = 0 and pc_src = 2'b00*/
                imm_src          = 0;       /* no impact, because alu_src, reg_write = 0  and pc_src = 2'b00*/
                reg_write        = 0;  
                data_mask        = 0;       /* no impact, because mem_write = 0 */
                mem_write        = 0;
                take_branch      = 0;
                jal              = 0;
                jalr             = 0;
                halt             = 0;
                $display("control module received invalid op=0b%b at time-0d%0t, will be treated as NOP", op, $time);
            end
        endcase
    end      
    
    /* ALU Decoder */
    always_comb begin
        if (op == 7'b1100011)begin /* = branch instruction */
            case (funct3)
                3'b000:  alu_ctrl = 4'b0001; /* SUB for beq */
                3'b001:  alu_ctrl = 4'b0001; /* SUB for bne */
                3'b100:  alu_ctrl = 4'b0101; /* SLT for blt */
                3'b101:  alu_ctrl = 4'b0101; /* SLT for bge */
                3'b110:  alu_ctrl = 4'b0110; /* SLTU for bltu */
                3'b111:  alu_ctrl = 4'b0110; /* SLTU for bgeu */

                default: alu_ctrl = 4'b0001; /* default = SUB */
            endcase               
        end
        else if(op ==7'b0100011  || op ==7'b0000011) begin /* = store or load instruction */
            alu_ctrl = 4'b0000; /* ADD */
        end
        else if(op ==7'b0010011) begin /* = I-type instruction */
            case (funct3)
                3'b000: alu_ctrl = 4'b0000; /* ADD */
                3'b111: alu_ctrl = 4'b0010; /* AND */
                3'b110: alu_ctrl = 4'b0011; /* 0R */
                3'b100: alu_ctrl = 4'b0100; /* X0R */
                3'b010: alu_ctrl = 4'b0101; /* SLT */
                3'b011: alu_ctrl = 4'b0110; /* SLTU */
                3'b001: alu_ctrl = 4'b0111; /* SLL */
                3'b101: alu_ctrl = (funct7[5] == 1) ? 4'b1001 : 4'b1000; /* ? SRA : SRL */
                default: alu_ctrl = 4'b0000; /* default to add */
            endcase
        end 
        else
            case ({funct7[5], funct3})
                4'b0000: alu_ctrl = 4'b0000; /* ADD */
                4'b1000: alu_ctrl = 4'b0001; /* SUB */
                4'b0111: alu_ctrl = 4'b0010; /* AND */
                4'b0110: alu_ctrl = 4'b0011; /* 0R */
                4'b0100: alu_ctrl = 4'b0100; /* X0R */
                4'b0010: alu_ctrl = 4'b0101; /* SLT */
                4'b0011: alu_ctrl = 4'b0110; /* SLTU */
                4'b0001: alu_ctrl = 4'b0111; /* SLL */
                4'b0101: alu_ctrl = 4'b1000; /* SRL */
                4'b1101: alu_ctrl = 4'b1001; /* SRA */
                default: alu_ctrl = 4'b0000; /* default to add */
            endcase
    end

 endmodule

