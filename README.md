# RV32I Single-Cycle Core

 

## 1. Verification State:

No official risc-v verification acquired yet. 
    
The cpu passes basic self written test for all rv32i basic integer instructions:
- add, sub, and, or, xor, sll, srl, sra, slt, sltu
- addi, andi, ori, xori, slli, srli, srai, slti, sltiu,
- lb, lh, lw, lbu, lhu
- sb, sh, sw,
- beq, bne, blt, bge, bltu, bgeu,
- jal, jalr
- lui, auipic
- ecall, ebreak

## 2. Specifications

- Modules are written in SystemVerilog.

- Top module is 'rv32i_core.sv' .

- The CPU executes one instruction per clock cycle before fetching the next instruction (= single cycle CPU).

- An active-low asynchronous reset signal resets the program counter and regfile module.

- System call (ebreak/ecall) handling: Control module sends a control signal to halt the program counter.

## 3. Test Bench Usage

Every module can be tested via its corresponding test bench file.

### 3.1 Top Module Testing

1. Source file must be of type '.hex', one instruction per line (little endian) and no in-file metadata.
(Note: To convert raw binary to '.hex', move the '.bin' file into /test_programs 
and call * *make test_programs/%.hex* *, where '%' is the source file name (requires python3)).

2. Open tb_rv32i_core.sv in a text editior and change argument 0 of '$readmemh' statement to the path to the '.hex' file.

  3.  
```bash 
        make tb_rv32i_core
```

4. Read console ouput or open /waveforms to 
access the created '.vdc' file.

    
### 3.2 Sub Module Testing

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
    

