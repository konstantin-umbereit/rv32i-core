/* test_benches/arch_tb_rv32i_core.sv
 *
 * Test bench for rtl/rv32i_core.sv
 *
 * Details:
 * - This test bench is written for  
 *   RISC-V Architectural Certification Tests (ACTs)
 *   as generated according to this CPUs config files.
 *
 * - Final State: (halt == 1 && a0 == 0) -> Test passed.
 *                (halt == 1 && a0 == 1) -> Test failed.
 */

 module arch_tb_rv32i_core;
    logic clk = 0;
    logic rst_n = 0;

    rv32i_core dut (.*);

    always #5 clk = ~ clk;

    /* loads instructions from .hex file into byte addressable memory */
    task automatic load_hex_to_byte_mem(
        input string hex_file,
        ref logic [7:0] mem[0:(256 * 1024)-1]
    );        
        integer fd, fgets_code, sscanf_code;
        string word;
        integer addr = 0;
        logic [31:0] word_value;
        $display("Loading file: %s", hex_file);

        fd = $fopen(hex_file, "r");                         /* open file for reading */

        if (fd == 0) begin
            $error("Failed to open file %s", hex_file);
            $finish;
        end

        fgets_code = $fgets(word, fd);
        while(fgets_code != 0) begin
            
            sscanf_code = $sscanf(word, "%h", word_value);  /* writes ascii characters of the string as bytes into word_value */

            if (sscanf_code != 0) begin                     /* writes the 4 bytes in little endian order into the memory */
                mem[addr + 0] = word_value[7:0];
                mem[addr + 1] = word_value[15:8];
                mem[addr + 2] = word_value[23:16];
                mem[addr + 3] = word_value[31:24];

                addr = addr + 4;
            end

            fgets_code = $fgets(word, fd);
            
        end
        $display("Loading finished. Last addr written = 0x%h", addr);
    endtask

    always  @(posedge clk) begin

        /* Check for halt signal */
        if (dut.halt) begin

            /* Finish testing and display the final state */
            $finish;
            $display("\nFINAL STATE:");
            $display("PC          = 0x%h", dut.pc);
            $display("a0 (x10)    = 0x%h", dut.regfile_init.regs[10]);
            $display("halt        = %b", dut.halt);
            $display("instr       = 0x%h", dut.instr);
            $write("\nProgram halted: ecall/ebreak detected\n\n");
            
        end
            
    end

    initial begin
        /* Dump signal changes */
        $dumpfile("waveforms/tb_rv32i_core.vcd");
        $dumpvars(0, tb_rv32i_core);


        /* Instruction Memory Loading */
        $readmemh("riscv_arch_test_programs/I-beq-00.hex", dut.instr_memory_init.mem);

        /* Data Memory Loading */
        load_hex_to_byte_mem("riscv_arch_test_programs/I-beq-00.hex", dut.data_memory_init.mem);

        /* Release reset and start the program */
        rst_n = 1; $write("\n// Release reset, start program\n");

        /* Run 1000000 clock cycles */
        repeat(1000000) @(posedge clk);
        
         
        /* Finish testing and display the final state */ 
        $finish;
        $display("FINAL STATE:");
        $display("PC          = 0x%h", dut.pc);
        $display("a0 (x10)    = 0x%h", dut.regfile_init.regs[10]);
        $display("halt        = %b", dut.halt);
        $display("instr       = 0x%h", dut.instr);
    end

 endmodule

