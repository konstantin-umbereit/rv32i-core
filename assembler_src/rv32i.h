/* 
 * rv32i.h - RV32I Assembler
 *
 * Source for register and instruction data.
 */

#ifndef RV32I_H
#define RV32I_H

#include <stdint.h>   
#include <stdbool.h>  
#include <stddef.h>   


/* REGISTERS (x0–x31 + STANDARD ABI NAMES) */

#define REG_ZERO  0
#define REG_RA    1
#define REG_SP    2
#define REG_GP    3
#define REG_TP    4
#define REG_T0    5
#define REG_T1    6
#define REG_T2    7
#define REG_S0    8   /* also fp */
#define REG_S1    9
#define REG_A0   10
#define REG_A1   11
#define REG_A2   12
#define REG_A3   13
#define REG_A4   14
#define REG_A5   15
#define REG_A6   16
#define REG_A7   17
#define REG_S2   18
#define REG_S3   19
#define REG_S4   20
#define REG_S5   21
#define REG_S6   22
#define REG_S7   23
#define REG_S8   24
#define REG_S9   25
#define REG_S10  26
#define REG_S11  27
#define REG_T3   28
#define REG_T4   29
#define REG_T5   30
#define REG_T6   31


/* OPCODES (7-BIT) */

#define OPCODE_OP_IMM  0x13U
#define OPCODE_OP      0x33U
#define OPCODE_LOAD    0x03U
#define OPCODE_STORE   0x23U
#define OPCODE_BRANCH  0x63U
#define OPCODE_JALR    0x67U
#define OPCODE_JAL     0x6FU
#define OPCODE_LUI     0x37U
#define OPCODE_AUIPC   0x17U
#define OPCODE_SYSTEM  0x73U


/* funct3 AND funct7 VALUES */

#define FUNCT3_ADD_SUB 0x0
#define FUNCT3_SLL     0x1
#define FUNCT3_SLT     0x2
#define FUNCT3_SLTU    0x3
#define FUNCT3_XOR     0x4
#define FUNCT3_SR      0x5
#define FUNCT3_OR      0x6
#define FUNCT3_AND     0x7

#define FUNCT7_ADD     0x00
#define FUNCT7_SUB     0x20
#define FUNCT7_SLLI    0x00
#define FUNCT7_SRL     0x00
#define FUNCT7_SRA     0x20

#define FUNCT3_LB      0x0
#define FUNCT3_LH      0x1
#define FUNCT3_LW      0x2
#define FUNCT3_LBU     0x4
#define FUNCT3_LHU     0x5

#define FUNCT3_SB      0x0
#define FUNCT3_SH      0x1
#define FUNCT3_SW      0x2

#define FUNCT3_BEQ     0x0
#define FUNCT3_BNE     0x1
#define FUNCT3_BLT     0x4
#define FUNCT3_BGE     0x5
#define FUNCT3_BLTU    0x6
#define FUNCT3_BGEU    0x7

#define FUNCT3_JALR    0x0

#define FUNCT3_ECALL_EBREAK 0x0


/* BIT-FIELD POSTIONS */

#define RD_SHIFT        7
#define RS1_SHIFT      15
#define RS2_SHIFT      20
#define FUNCT3_SHIFT   12
#define FUNCT7_SHIFT   25

#define I_IMM_SHIFT    20
#define S_IMM_LO_SHIFT  7
#define S_IMM_HI_SHIFT 25
#define U_IMM_SHIFT    12


/* INSTRUCTION ENCODING HELPER MACROS */

/* R-type: funct7 | rs2 | rs1 | funct3 | rd | opcode */
#define ENCODE_R(op, f3, f7, rd, rs1, rs2) \
    ((uint32_t)(op) | \
     ((uint32_t)(f3) << FUNCT3_SHIFT) | \
     ((uint32_t)(f7) << FUNCT7_SHIFT) | \
     ((uint32_t)(rd) << RD_SHIFT) | \
     ((uint32_t)(rs1) << RS1_SHIFT) | \
     ((uint32_t)(rs2) << RS2_SHIFT))

/* I-type: imm[11:0] | rs1 | funct3 | rd | opcode */
#define ENCODE_I(op, f3, rd, rs1, imm) \
    ((uint32_t)(op) | \
     ((uint32_t)(f3) << FUNCT3_SHIFT)| \
     ((uint32_t)(rd) << RD_SHIFT)| \
     ((uint32_t)(rs1) << RS1_SHIFT)| \
     (((uint32_t)(imm) & 0xFFFU) << I_IMM_SHIFT))

/* S-type: imm[11:5] | rs2 | rs1 | funct3 | imm[4:0] | opcode */
#define ENCODE_S(op, f3, rs1, rs2, imm) \
    ((uint32_t)(op) | \
     ((uint32_t)(f3) << FUNCT3_SHIFT) | \
     ((uint32_t)(rs1) << RS1_SHIFT) | \
     ((uint32_t)(rs2) << RS2_SHIFT) | \
     (((uint32_t)(imm) & 0x1FU) << S_IMM_LO_SHIFT) | \
     (((uint32_t)(imm) & 0xFE0U) << (S_IMM_HI_SHIFT - 5)))

/* B-type: imm[12] | imm[10:5] | rs2 | rs1 | funct3 | imm[4:0] | imm[11] | opcode */
#define ENCODE_B(op, f3, rs1, rs2, imm) \
    ((uint32_t)(op) | \
     ((uint32_t)(f3) << FUNCT3_SHIFT) | \
     ((uint32_t)(rs1) << RS1_SHIFT) | \
     ((uint32_t)(rs2) << RS2_SHIFT) | \
     (((uint32_t)(imm) & 0x1EU) << 7) | \
     (((uint32_t)(imm) & 0x7E0U) << 20) | \
     (((uint32_t)(imm) & 0x800U) >> 4) | \
     (((uint32_t)(imm) & 0x1000U) << 19))

/* U-type: imm[31:12] | rd | opcode */
#define ENCODE_U(op, rd, imm) \
    ((uint32_t)(op) | \
     ((uint32_t)(rd) << RD_SHIFT) | \
     (((uint32_t)(imm) & 0xFFFFFU) << U_IMM_SHIFT))

/* J-Type: imm[20] | imm [10:1] | imm[11] | imm [19:12] | rd | opcode */
 #define ENCODE_J(op, rd, imm) \
    ((uint32_t)(op) | \
     ((uint32_t)(rd) << RD_SHIFT) | \
     (((uint32_t)(imm) & 0x7FEU) << 20) | \
     (((uint32_t)(imm) & 0x800U) << 9) | \
     ((uint32_t)(imm) & 0xFF000U) | \
     (((uint32_t)(imm) & 0x100000U) << 11))
     
     
/* TYPE DEFINITIONS */

typedef enum {
    INSTR_R, INSTR_I, INSTR_S, INSTR_B, INSTR_U, INSTR_J,
    INSTR_NONE, /* for pseudo instructions */
} rv32i_format_t;

typedef struct rv32i_instruction_t{
    const char * const mnemonic;
    rv32i_format_t     format;
    uint32_t           opcode;
    uint32_t           funct3;
    uint32_t           funct7;
} rv32i_instruction_t;


/* EXTERNAL DECLERATIONS */

extern const char* const reg_abi_mnemonics[32];
extern const rv32i_instruction_t rv32i_instructions[];
extern const size_t rv32i_instruction_count;


/* PUBLIC API FUNCTIONS*/

/*  Returns the corresponding number to a given register name or ABI mnemonic, returns -1 if not found.*/
int  rv32i_register_number(const char* name);

/* Returns true if given name is a valid mnemonic, false if not. */
bool rv32i_is_mnemonic(const char* name);

/* Returns the corresponding rv32i_instruction to a given mnemonic. */
const rv32i_instruction_t* rv32i_lookup_instruction(const char* mnemonic);


#endif /* RV32I_H */

