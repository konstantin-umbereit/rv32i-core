/* 
 * parser.h - RV32I Assembler
 *
 * Public API of the parser.
 */

#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>         

#include "rv32i.h"          
#include "lexer.h"         
#include "symbol_table.h"   
#include "emitter.h"


/* ENUMS */

typedef enum {
    IK_REAL, 
    IK_PSEUDO,
} InstrKind;

typedef enum {
    ST_LABEL_DEFINITION, 
    ST_INSTRUCTION, 
    ST_DIRECTIVE, 
    ST_PSEUDO_INSTRUCTION,
} StatementType;

typedef enum {
    OK_NONE,/* default value after nulling */

    OK_REGISTER, 
    OK_IMMEDIATE, 
    OK_LABEL_REFERENCE, 
    OK_MEMORY,
} OperandKind;

typedef enum {
    /* Control / Section directives */
    DT_TEXT, 
    DT_DATA, 
    DT_SECTION,
    DT_ALIGN, 
    DT_GLOBAL,

    /* Data directives */
    DT_WORD, 
    DT_HALF, 
    DT_BYTE, 

    /* String directives */
    DT_ASCII, 
    DT_ASCIZ,
} DirectiveType;


/* HELPER TYPES */

typedef struct StringPayload {/* Helper for .ascii / .asciz directives */
    const char *value;
    size_t      length;
    bool        is_null_terminated; /* = true for .asciz, = false for .ascii */
} StringPayload;

typedef struct Memory {/* Helper for memory addressing mode offset(base) - Supports both immediate(register) and label(register).*/
    const char *label;      /* NULL for immediate offset */             
    int32_t     offset;     /* signed 12-bit immediate, used only when label = NULL */
    int32_t    base_reg;   /* base register */
} Memory;

typedef struct Operand {/* A discriminated union that can hold any kind of operand. */
    OperandKind kind;
    union {
        int32_t    reg_number;        /* OK_REGISTER */
        int32_t     imm_value;         /* OK_IMMEDIATE */
        const char *label_name;        /* OK_LABEL_REFERENCE */
        Memory      memory;            /* OK_MEMORY */
    } value;
} Operand;


/* STATEMENT STRUCTS */

typedef struct ParsedInstruction {/* Represents a fully parsed RISC-V instruction (real or pseudo). */
    InstrKind                      kind; /* IK_REAL or IK_PSEUDO*/
    union {
        const rv32i_instruction_t *real;    
        char                      *pseudo_name;
    } mnemonic;

    rv32i_format_t                 format;
    Operand                        operands[3];
} ParsedInstruction;

typedef struct ParsedDirective {/* Represents any .xxx directive. */
    DirectiveType      type; 

    union {
        /* Data directives (.word / .half / .byte) */ 
        struct {
            Operand    values[16];    /* each can be either OK_IMMEDIATE or OK_LABEL_REFERENCE */
            size_t     count;
        } data;

        StringPayload string;         /* .ascii / .asciz */

        uint32_t       alignment;      /* .align */
        const char    *symbol_name;    /* .global */
    } payload;
} ParsedDirective;

typedef struct Statement {/* Represents one logical statement in the source file. */
    StatementType type;              /* discriminator */

    SectionType   section;
    uint32_t      location;          

    size_t        source_line;
    size_t        source_column;

    union {
        const char        *label_name;     /* ST_LABEL_DEFINITION */
        ParsedInstruction  instruction;    /* ST_INSTRUCTION / ST_PSEUDO_INSTRUCTION */
        ParsedDirective    directive;      /* ST_DIRECTIVE */
    } payload;
} Statement;


/* MAIN STRUCT */

typedef struct Parser {/* Holds the runtime state of the parser during both passes. */
    Lexer       *lexer;
    SymbolTable *symbol_table;           /* gets filed during pass 1 */
    Emitter     *emitter;
    int          current_pass;           /* 1 or 2 */
    
    Statement   *statements;             /* gets filled  during pass 1 */
    size_t       statement_count;
    size_t       statement_capacity;
} Parser;


/* PUBLIC API FUNCTIONS */

/* Initializes the parser struct. */
bool parser_initialize(Parser *parser, Lexer *lexer, SymbolTable *symbol_table, Emitter *emitter);
/* Frees the parser struct. */
void parser_free(Parser *parser);
/* Start parser pass 1 and parser pass 2 after. */
bool parser_run(Parser *parser);

bool parser_is_pseudo_instruction(const char *mnemonic);


#endif /* PARSER_H */
