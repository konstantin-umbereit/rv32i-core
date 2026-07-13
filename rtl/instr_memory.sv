/* rtl/instr_memory.sv
 *
 * Single-cycle RV32I instruction memory
 */

 module instr_memory #(
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 32,
    parameter MEM_SIZE = 1024
 ) (
    input  logic [ADDR_WIDTH-1:0] pc,           /* PC */

    output logic [DATA_WIDTH-1:0] instr         /* instruction output */
 );
    /* Memory: 1024 x 8bit */
    logic [7:0] mem [0:MEM_SIZE-1];

    /* Instruction output */
    assign instr = {mem[pc+3],mem[pc+2],mem[pc+1],mem[pc]};
    
 endmodule
 

