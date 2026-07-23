# RV32I Single-Cycle Core

 

## 1. Verification State:

The CPU passes the complete set of official riscv-arch-test suites for the base integer ISA.

See riscv_arch_config/ for the config files and riscv_arch_test_programs/ for the generated test files.

## 2. Specifications

- Modules are written in SystemVerilog.

- Top module is 'rv32i_core.sv' .

- The CPU executes one instruction per clock cycle before fetching the next instruction (= single cycle CPU).

- An active-low asynchronous reset signal resets the program counter and regfile module.

- System call (ebreak/ecall) handling: Control module sends a control signal to halt the program counter.
 
- 'fence' instruction handling: treated as 'nop'

## 3. Test Bench Usage
Simulation Tool: Verilator 5.032 (Note: These test benches don't work yet on some newer versions.)

Every module can be tested via its corresponding test bench file.

In addition, their is a separate test bench specifically for the riscv-arch-test suite.

All test benches generate a corresponding .vcd file in the waveforms/ directory.

### 3.1 Top Module Testing with riscv-arch-test files.

1. Source file must be of type '.hex', one instruction per line (little endian) and no in-file metadata.
(Note: To convert raw binary to '.hex', call * *make %.hex* *, where '%' is the binary source file path (requires python3)).

2. Open arch_tb_rv32i_core.sv in a text editior and change both the argument 0 of the '$readmemh' statement and the argument 0 the 'load_hex_to_byte_mem' call to the '.hex' file path.

3.  
```bash 
        make arch_tb_rv32i_core
```

### 3.2 Top Module Testing with regular test files.

1. Source file must be of type '.hex', one instruction per line (little endian) and no in-file metadata.
(Note: To convert raw binary to '.hex', call * *make %.hex* *, where '%' is the binary source file path (requires python3)).

2. Open tb_rv32i_core.sv in a text editior and change argument 0 of '$readmemh' statement to the '.hex' file path.

3.  
```bash 
        make tb_rv32i_core
```
    
### 3.3 Sub Module Testing

```bash 
        make tb_modulename
```

## 4. Copyright

MIT License

Copyright (c) 2026 Konstantin Umbereit

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
    

