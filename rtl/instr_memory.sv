/* rtl/instr_memory.sv
 *
 * Single-cycle RV32I instruction memory
 */

 module instr_memory #(
    parameter DATA_WIDTH = 32,
    parameter ADDR_WIDTH = 32,
    parameter MEM_SIZE = 256 * 1024 /* 256KB instruction memory */
 ) (
    input  logic [ADDR_WIDTH-1:0] pc,           /* PC */

    output logic [DATA_WIDTH-1:0] instr         /* instruction output */
 );
    /* Memory: word addressable */
    logic [DATA_WIDTH-1:0] mem [0:MEM_SIZE-1];

    /* Instruction output */
    assign instr = mem[pc[$clog2(MEM_SIZE)+1 : 2] ]; /* calculates the amount of bits needed to address a MEMSIZE large array */
    
 endmodule
 

