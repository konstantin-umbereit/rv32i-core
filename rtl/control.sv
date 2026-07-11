/* rtl/alu.sv
 *
 * Single-cycle RV32I control unit
 */

 module control (
    input  logic [6:0] op,
    input  logic [2:0] funct3,
    input  logic [6:0] funct7,
    input  logic       zero,

    output logic [1:0] pc_src,      
    output logic [2:0] result_src,  
    output logic       mem_write,   
    output logic       alu_src,     
    output logic [2:0] imm_src,     
    output logic       reg_write,   
    output logic [1:0] data_mask,

    output logic [3:0] alu_ctrl     

 );
    
    /* Main Decoder */
    always_comb begin
        case (op) 
            /* R-type */
            7'b0110011: begin
                pc_src           = 2'b00;   /* = pc_plus4 */
                result_src       = 3'b000;  /* = alu_result */
                mem_write        = 0;
                alu_src          = 0;       /* = rd2 */
                imm_src          = 0;       /* no impact, because alu_src = 0; */
                reg_write        = 1;
                data_mask        = 2'b00;   /* no impact, because mem_write = 0 */
            end 

            /* I-type: OP-IMM */
            7'b0010011: begin
                pc_src           = 2'b00;   /* = pc_plus4 */
                result_src       = 3'b000;  /* = alu_result */
                mem_write        = 0;
                alu_src          = 1;       /* = imm_ext */
                imm_src          = 3'b000;  /* = I-type */
                reg_write        = 1;
            end 

            /* I-type: LOAD */
            7'b0000011: begin
                pc_src           = 2'b00;   /* = pc_plus4 */
                result_src       = 3'b001;  /* = read_data */
                mem_write        = 0;
                alu_src          = 1;       /* = imm_ext */
                imm_src          = 3'b000;  /* = I-type */
                reg_write        = 1;
            end 

            /* I-type: JALR */
            7'b1100111: begin
                pc_src           = 2'b10;   /* = alu_result */
                result_src       = 3'b100;  /* = pc_plus4 */
                mem_write        = 0;
                alu_src          = 1;       /* = imm_ext */
                imm_src          = 3'b100;  /* = J-type */
                reg_write        = 1;
            end 

            /* I-type: SYSTEM */
            7'b1110011: begin
                pc_src           = 2'b00;   /* = pc_plus4 */
                result_src       = 0;       /* no impact, because reg_write = 0; */
                mem_write        = 0;
                alu_src          = 0;       /* no impact, because mem_write, reg_write = 0 and pc_src = 2'b00; */
                imm_src          = 3'b000;  /* no impact, because alu_src = 0; */
                reg_write        = 1;
            end 
            
            /* S-type */
            7'b0100011: begin
                pc_src           = 2'b00;   /* = pc_plus4 */
                result_src       = 0;       /* no impact, because reg_write = 0; */
                mem_write        = 1;
                alu_src          = 0;       /* = rd2 */
                imm_src          = 3'b001;  /* = S-type */
                reg_write        = 0;
                case (funct3)
                    3'b000:  data_mask = 2'b00; /* [7:0] */
                    3'b001:  data_mask = 2'b01; /* [15:0] */
                    3'b010:  data_mask = 2'b10; /* [31:0] */
                    default: data_mask = 2'b10; /* [7:0] */

                endcase       

            end 

            /* B-type */
            7'b1100011: begin
                pc_src           = (zero == 1'b1) ? 2'b01: 2'b00;   /* ? pc_plus4 : imm_ext*/
                result_src       = 0;       /* no impact, because reg_write = 0; */
                mem_write        = 0;
                alu_src          = 0;       /* = rd2 */
                imm_src          = 3'b010;  /* = B-type */
                reg_write        = 0;       
            end 

            /* U-type: LUI */
            7'b0110111: begin
                pc_src           = 2'b00;   /* = pc_plus4 */
                result_src       = 3'b010;  /* = imm_ext */
                mem_write        = 0;
                alu_src          = 0;       /* no impact because result_src = imm_ext, mem_write= 0 and pc_src = 2'b00 */
                imm_src          = 3'b011;  /* = U-type */
                reg_write        = 1;       
                
            end

            /* U-type: AUIPC */
            7'b0010111: begin
                pc_src           = 2'b00;   /* = pc_plus4 */
                result_src       = 3'b011;  /* = pc_target */
                mem_write        = 0;
                alu_src          = 0;       /* no impact, because result_src = pc_target, mem_write 0 and pc_src = 2'b00*/
                imm_src          = 3'b011;  /* = U-type */
                reg_write        = 1;       
                
            end  

            /* J-type: JAL */
            7'b1101111: begin
                pc_src           = 2'b01;   /* = imm_ext */
                result_src       = 3'b100;  /* = pc_plus4 */
                mem_write        = 0;
                alu_src          = 0;       /* no impact, because result_src = pc_plus4, mem_write = 0 and pc_src = 2'b01*/
                imm_src          = 3'b100;  /* = J-type */
                reg_write        = 1;     
            end 

            default: begin
                pc_src           = 2'b00;   /* = pc_plus4 */
                result_src       = 0;       /* no impact, because reg_write = 0 */
                mem_write        = 0;
                alu_src          = 0;       /* no impact, because reg_write, mem_write = 0 and pc_src = 2'b00*/
                imm_src          = 0;       /* no impact, because alu_src, reg_write = 0  and pc_src = 2'b00*/
                reg_write        = 0;  
                $display("control module received invalid op=0b%b at time-0d%0t", op, $time);
            end
        endcase
    end      
    
    /* ALU Decoder */
    always_comb begin
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

