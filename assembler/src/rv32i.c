/* 
 * rv32i.c - RV32I Assembler
 *
 * Source for registers and instruction data.
 */

#include <string.h>          
#include <stddef.h>         
#include <ctype.h>           

#include "rv32i.h"          


/* DATA */             

const char* const reg_abi_mnemonics[32] = {
    "zero", "ra",  "sp",  "gp",  "tp",  "t0", "t1", "t2",
    "s0",   "s1",  "a0",  "a1",  "a2",  "a3", "a4", "a5",
    "a6",   "a7",  "s2",  "s3",  "s4",  "s5", "s6", "s7",
    "s8",   "s9",  "s10", "s11", "t3",  "t4", "t5", "t6"
};

const rv32i_instruction_t rv32i_instructions[] = {
    {"add",  INSTR_R, OPCODE_OP,      FUNCT3_ADD_SUB, FUNCT7_ADD},
    {"sub",  INSTR_R, OPCODE_OP,      FUNCT3_ADD_SUB, FUNCT7_SUB},
    {"and",  INSTR_R, OPCODE_OP,      FUNCT3_AND,     0},
    {"or",   INSTR_R, OPCODE_OP,      FUNCT3_OR,      0},
    {"xor",  INSTR_R, OPCODE_OP,      FUNCT3_XOR,     0},
    {"sll",  INSTR_R, OPCODE_OP,      FUNCT3_SLL,     0},
    {"srl",  INSTR_R, OPCODE_OP,      FUNCT3_SR,      FUNCT7_SRL},
    {"sra",  INSTR_R, OPCODE_OP,      FUNCT3_SR,      FUNCT7_SRA},
    {"slt",  INSTR_R, OPCODE_OP,      FUNCT3_SLT,     0},
    {"sltu", INSTR_R, OPCODE_OP,      FUNCT3_SLTU,    0},

    {"addi", INSTR_I, OPCODE_OP_IMM,  FUNCT3_ADD_SUB, 0},
    {"andi", INSTR_I, OPCODE_OP_IMM,  FUNCT3_AND,     0},
    {"ori",  INSTR_I, OPCODE_OP_IMM,  FUNCT3_OR,      0},
    {"xori", INSTR_I, OPCODE_OP_IMM,  FUNCT3_XOR,     0},
    {"slli", INSTR_I, OPCODE_OP_IMM,  FUNCT3_SLL,     FUNCT7_SLLI},
    {"srli", INSTR_I, OPCODE_OP_IMM,  FUNCT3_SR,      FUNCT7_SRL},
    {"srai", INSTR_I, OPCODE_OP_IMM,  FUNCT3_SR,      FUNCT7_SRA},
    {"slti", INSTR_I, OPCODE_OP_IMM,  FUNCT3_SLT,     0},
    {"sltiu",INSTR_I, OPCODE_OP_IMM,  FUNCT3_SLTU,    0},

    {"lb",   INSTR_I, OPCODE_LOAD,    FUNCT3_LB,      0},
    {"lh",   INSTR_I, OPCODE_LOAD,    FUNCT3_LH,      0},
    {"lw",   INSTR_I, OPCODE_LOAD,    FUNCT3_LW,      0},
    {"lbu",  INSTR_I, OPCODE_LOAD,    FUNCT3_LBU,     0},
    {"lhu",  INSTR_I, OPCODE_LOAD,    FUNCT3_LHU,     0},

    {"jalr", INSTR_I, OPCODE_JALR,    FUNCT3_JALR,    0},

    {"sb",   INSTR_S, OPCODE_STORE,   FUNCT3_SB,      0},
    {"sh",   INSTR_S, OPCODE_STORE,   FUNCT3_SH,      0},
    {"sw",   INSTR_S, OPCODE_STORE,   FUNCT3_SW,      0},

    {"beq",  INSTR_B, OPCODE_BRANCH,  FUNCT3_BEQ,     0},
    {"bne",  INSTR_B, OPCODE_BRANCH,  FUNCT3_BNE,     0},
    {"blt",  INSTR_B, OPCODE_BRANCH,  FUNCT3_BLT,     0},
    {"bge",  INSTR_B, OPCODE_BRANCH,  FUNCT3_BGE,     0},
    {"bltu", INSTR_B, OPCODE_BRANCH,  FUNCT3_BLTU,    0},
    {"bgeu", INSTR_B, OPCODE_BRANCH,  FUNCT3_BGEU,    0},

    {"jal",  INSTR_J, OPCODE_JAL,     0,              0},
    {"lui",  INSTR_U, OPCODE_LUI,     0,              0},
    {"auipc",INSTR_U, OPCODE_AUIPC,   0,              0},

    {"ecall", INSTR_I, OPCODE_SYSTEM, FUNCT3_ECALL_EBREAK, 0},
    {"ebreak",INSTR_I, OPCODE_SYSTEM, FUNCT3_ECALL_EBREAK, 0},
};

const size_t rv32i_instruction_count = sizeof(rv32i_instructions) / sizeof(rv32i_instructions[0]);


/* PUBLIC API FUNCTIONS */                        

/*
 * rv32i_register_number()
 * 
 * Returns the corresponding number to a given register name or ABI mnemonic, -1 if not found.
 */
int rv32i_register_number(const char* name) 
{
    if (name == NULL) return -1;

    int len = strlen(name);
    if (len == 2) {
        if (name[0] == 'x' && isdigit(name[1])) return name[1] - 48;
    }
    if (len == 3) {
        if (name[0] == 'x' && 49 <= name[1] && name[1] <= 51 && isdigit(name[2])) return ((name[1] - 48) * 10) + name[2] - 48;
    }

    for (int i = 0; i < 32; i++) {
        if (strcmp(reg_abi_mnemonics[i], name) == 0) {
            return i;
        }
    }
    return -1;  // not found
}

/*
 * rv32i_is_mnemonic()
 * 
 * Returns true if the given name is a valid mnemonic, false if not.
 */
bool rv32i_is_mnemonic(const char* name) 
{
    if (name == NULL) return false;
    for (size_t i = 0; i < rv32i_instruction_count; i++) {
        if (strcmp(rv32i_instructions[i].mnemonic, name) == 0) {
            return true;
        }
    }
    return false;
}

/*
 * rv32i_lookup_instruction()
 * 
 * Returns a pointer to the rv32i_instruction_t corresponding to the given mnemonic, NULL if not found.
 */
const rv32i_instruction_t *rv32i_lookup_instruction(const char* mnemonic) 
{
    if (mnemonic == NULL) return NULL;
    for (size_t i = 0; i < rv32i_instruction_count; i++) {
        if (strcmp(rv32i_instructions[i].mnemonic, mnemonic) == 0) {
            return &rv32i_instructions[i];
        }
    }
    return NULL;
}
