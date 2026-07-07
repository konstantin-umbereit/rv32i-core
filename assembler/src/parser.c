/* parser.c - RV32I Assembler
 * 
 * Core of the Assembler.
 *
 * Parses the assembly code in 2 passes:
 * 1. Symbol table creation + parsing operands / building statements
 * 2. Encoding statements + emitting
 *
 * Note: The Parser is also the single source of truth for pseudo instruction data.
 */
#define _POSIX_C_SOURCE 200809L /* access to POSIX.1-2008 functions, here: strdup() */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "rv32i.h"
#include "parser.h"
#include "lexer.h"
#include "symbol_table.h"
#include "emitter.h"

#define STATEMENTS_INIT_SIZE 64


/* FORWARD DECLARATIONS */

static bool parser_run_pass1(Parser *parser);
static bool parser_run_pass2(Parser *parser);
static bool parser_add_label_definition(Parser *parser);
static Statement *parser_new_statement(Parser *parser, StatementType type);
static bool parse_mnemonic(Parser *parser);
static bool parse_pseudo_instruction(Parser *parser);
static bool parse_pseudo_operands(Parser *parser, const char *name, Operand *operands);
static bool parse_directive(Parser *parser);
static bool parse_statement_directive(Parser *parser, Statement *stmt);
static bool encode_instruction(Parser *parser, Statement *stmt);
static bool encode_pseudo_instruction(Parser *parser, Statement *stmt);

static bool parse_r_operands(Parser *parser, Operand *operands);
static bool parse_i_operands(Parser *parser, Operand *operands);
static bool parse_s_operands(Parser *parser, Operand *operands);
static bool parse_b_operands(Parser *parser, Operand *operands);
static bool parse_u_operands(Parser *parser, Operand *operands);
static bool parse_j_operands(Parser *parser, Operand *operands);

static bool parse_directive_global(Parser *parser, ParsedDirective *directive);
static bool parse_directive_align(Parser *parser, ParsedDirective *directive);
static bool parse_directive_data(Parser *parser, ParsedDirective *directive);
static bool parse_directive_string(Parser *parser, ParsedDirective *directive);

static bool encode_r_instruction(Parser *parser, Statement *stmt);
static bool encode_i_instruction(Parser *parser, Statement *stmt);
static bool encode_s_instruction(Parser *parser, Statement *stmt);
static bool encode_b_instruction(Parser *parser, Statement *stmt);
static bool encode_u_instruction(Parser *parser, Statement *stmt);
static bool encode_j_instruction(Parser *parser, Statement *stmt);

static bool string_to_directive_type(const char *string, DirectiveType *type);
static bool expect_label_ref_operand(Parser *parser, Operand *operand);
static bool expect_register_operand(Parser *parser, Operand *operand);
static bool expect_comma(Parser *parser);
static bool expect_lparen(Parser *parser);
static bool expect_rparen(Parser *parser);
static bool expect_immediate_operand(Parser *parser, Operand *operand);
static bool expect_memory_operand(Parser *parser, Operand *operand);
static bool expect_base_register_operand(Parser *parser, int32_t *base_reg);

static bool parser_string_to_int32_t(const char* string, int32_t *dest);
static bool parser_string_to_uint32_t(const char* string, uint32_t *dest);

static void parser_error_expected(Parser *parser, const char *expected);
static void parser_error(Parser *parser, const char *message);


/* PUBLIC API FUNCTIONS */

/*
 * Initializes the parser struct.
 *
 * Details:
 * - Allocates for memory for a set amount of statements, corresponding to STATEMENTS_INIT_SIZE macro.
 * - Hands control of the lexer, symbol_table and emitter over to the parser.
 */
bool parser_initialize(Parser *parser, Lexer *lexer, SymbolTable *symbol_table, Emitter *emitter) 
{
    if(parser == NULL) {
        fprintf(stderr, "Error: in parser_initialize(): parser is NULL pointer\n");
        return false;
    }
    if(lexer == NULL) {
        fprintf(stderr, "Error: in parser_initialize(): lexer is NULL pointer\n");
        return false;
    }
    if(symbol_table == NULL) {
        fprintf(stderr, "Error: in parser_initialize(): symbol_table is NULL pointer\n");
        return false;
    }
    if(emitter == NULL) {
        fprintf(stderr, "Error: in parser_initialize(): emitter is NULL pointer\n");
        return false;
    }

    const size_t size = sizeof(Statement) * STATEMENTS_INIT_SIZE;
    parser->statements = malloc(size);
    if(parser->statements== NULL) {
        fprintf(stderr, "Error: in parser_initialize(): memory allocation failed\n");
        return false;
    }
    memset(parser->statements, 0, size);

    parser->statement_capacity = STATEMENTS_INIT_SIZE;
    parser->statement_count    = 0;

    parser->lexer              = lexer;
    parser->symbol_table       = symbol_table;
    parser->emitter            = emitter;
    parser->current_pass       = 0;
    return true;
}

/* 
 * parser_run()
 *
 * Start parser pass 1 and parser pass 2 after.
 */
bool parser_run(Parser *parser)
{
    if (parser == NULL) {
        fprintf(stderr, "Internal Error: parser_run(): parser is NULL\n");
        return false;
    }

    if (!parser_run_pass1(parser)) {
        fprintf(stderr, "Assembly failed in Pass 1 (parsing/symbol collection)\n");
        return false;
    }

    if (!parser_run_pass2(parser)) {
        fprintf(stderr, "Assembly failed in Pass 2 (resolution + code emission)\n");
        return false;
    }

    return true;  
}

/*
 * parser_free()
 *
 * Frees the parser struct.
 * 
 * Details:
 * - Frees allocated memory of the statements 
 * - Nulls pointers to the lexer, symbol table and emitter.
 * - Nulls all other fields.
 */
void parser_free(Parser *parser)
{
    if (parser == NULL) return;

    for (size_t i = 0; i < parser->statement_count; i++) {
        Statement *stmt = &parser->statements[i];
        ParsedInstruction *instr = &stmt->payload.instruction;

        switch (stmt->type) {
            case ST_LABEL_DEFINITION:
                free((void*)stmt->payload.label_name);          
                break;

            case ST_INSTRUCTION: 
                for (int op = 0; op < 3; op++) {
                    Operand *o = &instr->operands[op];
                    if (o->kind == OK_LABEL_REFERENCE) {
                        free((void*)o->value.label_name);
                    } else if (o->kind == OK_MEMORY) {
                        free((void*)o->value.memory.label);     
                    }
                }
                break;

            case ST_PSEUDO_INSTRUCTION: 
                free(instr->mnemonic.pseudo_name);
                for (int op = 0; op < 3; op++) {
                    Operand *o = &instr->operands[op];
                    if (o->kind == OK_LABEL_REFERENCE) {
                        free((void*)o->value.label_name);
                    } else if (o->kind == OK_MEMORY) {
                        free((void*)o->value.memory.label);     
                    }
                }
                break;
            

            case ST_DIRECTIVE: 
                ParsedDirective *d = &stmt->payload.directive;
                switch (d->type) {
                    case DT_GLOBAL:
                        free((void*)d->payload.symbol_name);
                        break;
                    case DT_WORD: case DT_HALF: case DT_BYTE:
                        for (size_t j = 0; j < d->payload.data.count; j++) {
                            if (d->payload.data.values[j].kind == OK_LABEL_REFERENCE) {
                                free((void*)d->payload.data.values[j].value.label_name);
                            }
                        }
                        break;
                    case DT_ASCII: case DT_ASCIZ:
                        free((void*)d->payload.string.value);
                        break;
                    default: break;
                }
                break;
            
            default: break;
        }
    }

    free(parser->statements);

    parser->statements         = NULL;
    parser->statement_count    = 0;
    parser->statement_capacity = 0;
    parser->lexer              = NULL; 
    parser->symbol_table       = NULL;
    parser->emitter            = NULL; 
}

/*
 * parser_is_pseudo_instruction()
 *
 * Checks whether the given mnemonic is a pseudo instruction supported by this parser not. 
 */
bool parser_is_pseudo_instruction(const char *mnemonic)
{
    if (mnemonic == NULL) return false;
    const char *pseudos[] = {
        "nop", "mv", "li", "la", "j", "ret", "call", "tail", NULL
    };
    for (int i = 0; pseudos[i] != NULL; i++) {
        if (strcmp(mnemonic, pseudos[i]) == 0) return true;
    }
    return false;
}


/* PASS 1 FUNCTIONS */

/*
 * parser_run_pass1()
 *
 * Starts the parser pass 1.
 *
 * Details:
 * - Readies the parser and emitter fields for pass 1.
 * - Advances parser to build the first token.
 * - Goes through all tokens til the end of file, fills the symbol table with the labels addresses and builds the statements.
 */
static bool parser_run_pass1(Parser *parser)
{
    parser->current_pass = 1;
    parser->statement_count = 0;

    if(!emitter_begin_pass1(parser->emitter)) return false;

    lexer_advance(parser->lexer);

    while(parser->lexer->current.type != TOKEN_EOF) {

        if (parser->lexer->current.type == TOKEN_LABEL_DEF) {
            if (!parser_add_label_definition(parser)) {
                return false;
            }
        }
        else if (parser->lexer->current.type == TOKEN_MNEMONIC) {
            if (!parse_mnemonic(parser)) {
                return false;
            }
        }
         else if (parser->lexer->current.type == TOKEN_DIRECTIVE) {
            if (!parse_directive(parser)) {
                return false;
            }
        }
        else {
            parser_error_expected(parser, "label definition, mnemonic or directive");
            return false;
        }
    }

    return true;
}

/*
 * parser_new_statement()
 *
 * Creates a new statement and doubles the parsers statement holder capacity if full.
 */
 
static Statement *parser_new_statement(Parser *parser, StatementType type) 
{
    if (parser->statement_count >= parser->statement_capacity) {
        size_t new_capacity = parser->statement_capacity * 2;
        Statement *temp = realloc(parser->statements, sizeof(Statement) * new_capacity);
        if (temp == NULL) {parser_error(parser, "memory allocation failed"); return NULL;}

        memset(&temp[parser->statement_capacity], 0, 
               sizeof(Statement) * (new_capacity - parser->statement_capacity));
        parser->statements = temp;
        parser->statement_capacity = new_capacity;
    }

    Statement *stmt = &parser->statements[parser->statement_count];
    stmt->type = type;
    stmt->section = parser->emitter->current_section; 
    if(!emitter_current_offset(parser->emitter, &stmt->location)) return false; 

    stmt->source_line = parser->lexer->line;
    stmt->source_column = parser->lexer->column;

    parser->statement_count++;

    
    return stmt;
}

/*
 * parser_add_label_definition()
 *
 * Creates a symbol table for the label definition with its address and creats a new statement.
 */
static bool parser_add_label_definition(Parser *parser) 
{
    uint32_t offset;
    if (!emitter_current_offset(parser->emitter, &offset)) return false;
    if (!symbol_table_define(parser->symbol_table, parser->lexer->current.value, offset, parser->emitter->current_section)) {   
        parser_error(parser, "statement building failed"); 
        return false;
    }

    Statement *stmt = parser_new_statement(parser, ST_LABEL_DEFINITION);
    if (!stmt) {
        parser_error(parser, "statement building failed");
         return false;
    }

    stmt->payload.label_name = strdup(parser->lexer->current.value);
    if(stmt->payload.label_name == NULL) {
        parser_error(parser, "memory allocation failed"); 
        return false;
    }

    lexer_advance(parser->lexer);

    return true;
}


/*
 * parse_mnemonic()
 *
 * Parses a mnemonic (real or pseudo instruction);
 *
 * Details:
 * - If pseudo: - dispatches to parse_pseudo_instruction.
 * - Else: - Creates a new statement
 *         -  Dispatches do the operand parser corresponding to the instruction format.
 *         -  Advances the emitters current section offset by 4 bytes.
 */
static bool parse_mnemonic(Parser *parser) 
{
    const char *name = parser->lexer->current.value;

    if (parser_is_pseudo_instruction(name)) {
            if(!parse_pseudo_instruction(parser)) {
                parser_error(parser, "failed parsing pseudo instruction in Pass 1");
                return false;
            }
            return true;
    }

    Statement *stmt = parser_new_statement(parser, ST_INSTRUCTION);
    if (!stmt) {parser_error(parser, "statement building failed"); return false;}

    ParsedInstruction *instr = &stmt->payload.instruction;
    instr->mnemonic.real = rv32i_lookup_instruction(name);
    if(!instr->mnemonic.real) { parser_error(parser, " unknown mnemonic or pseudo"); return false; }
    instr->kind = IK_REAL;

    lexer_advance(parser->lexer);
    
    instr->format = instr->mnemonic.real->format;
    switch (instr->format) {
        case INSTR_R: if(!parse_r_operands(parser, instr->operands)) {parser_error(parser, "operand building failed"); return false;};
                      break;
        case INSTR_I: if(!parse_i_operands(parser, instr->operands)) {parser_error(parser, "operand building failed"); return false;}
                      break;
        case INSTR_S: if(!parse_s_operands(parser, instr->operands)) {parser_error(parser, "operand building failed"); return false;}
                      break;
        case INSTR_B: if(!parse_b_operands(parser, instr->operands)) {parser_error(parser, "operand building failed"); return false;}
                      break;
        case INSTR_U: if(!parse_u_operands(parser, instr->operands)) {parser_error(parser, "operand building failed"); return false;}
                      break;
        case INSTR_J: if(!parse_j_operands(parser, instr->operands)) {parser_error(parser, "operand building failed"); return false;}
                      break;
        default: parser_error(parser, "unsupported instruction format"); return false;
    } 

    if (!emitter_advance(parser->emitter, 4U)) return false;
    
    return true;
}

/*
 * parse_pseudo_instruction()
 *
 * Parses a pseudo instruction;
 *
 * Details:
 * - Creates a new statement.
 * -  Dispatches do the operand parser for pseudo instructions.
 * -  Advances the emitters current section offset by 4 or 8 bytes, depending on the mnemonic.
 */
static bool parse_pseudo_instruction(Parser *parser) 
{   
    const char *name = parser->lexer->current.value;
     
    Statement *stmt = parser_new_statement(parser, ST_PSEUDO_INSTRUCTION);
    if (!stmt) { parser_error(parser, "statement building failed"); return false; }

    ParsedInstruction *instr = &stmt->payload.instruction;
    instr->kind = IK_PSEUDO;
    instr->mnemonic.pseudo_name = malloc(parser->lexer->current.value_len + (size_t)1);
    if(instr->mnemonic.pseudo_name == NULL) {
        parser_error(parser, "memory allocation failed");
        return false;
    }
    strcpy(instr->mnemonic.pseudo_name, name);
    instr->format = INSTR_NONE;     
    
    lexer_advance(parser->lexer);              

    if (!parse_pseudo_operands(parser, instr->mnemonic.pseudo_name, instr->operands)) {
        return false;
    }

    /* Reserve correct size (4 or 8 bytes) */
    if (strcmp(instr->mnemonic.pseudo_name, "j") == 0 || strcmp(instr->mnemonic.pseudo_name, "nop") == 0 || strcmp(instr->mnemonic.pseudo_name, "ret") == 0 || strcmp(instr->mnemonic.pseudo_name, "mv") == 0) {

        if (!emitter_advance(parser->emitter, 4U)) return false;
    } else {

        if (!emitter_advance(parser->emitter, 8U)) return false;        /* la, li, call, tail expand to 2 words */
    }
    return true;
}

/*
 * parse_pseudo_operands()
 *
 * Parses a the operands of a pseudo instruction.
 *
 * Details:
 * - Sets operands to a clean slate.
 * - Parses operands depending on the mnemonic.
 */
static bool parse_pseudo_operands(Parser *parser, const char *name, Operand *operands)
{
    memset(operands, 0, sizeof(Operand) * 3);   /* clean slate */
    
    if (strcmp(name, "nop") == 0 || strcmp(name, "ret") == 0) {
        return true;                            /* zero operands */
    }

    if (strcmp(name, "mv") == 0 || strcmp(name, "li") == 0) {
        if (!expect_register_operand(parser, &operands[0])) { parser_error_expected(parser, "register"); return false; }
        if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }

        if (strcmp(name, "mv") == 0) {
            if (!expect_register_operand(parser, &operands[1])) { parser_error_expected(parser, "register"); return false; }
        } else {  /* li */
            if (parser->lexer->current.type == TOKEN_IMMEDIATE) {
                if (!expect_immediate_operand(parser, &operands[1])) return false;
            } else if (parser->lexer->current.type == TOKEN_LABEL) {
                if (!expect_label_ref_operand(parser, &operands[1])) return false;
            } else {
                parser_error_expected(parser, "immediate or label reference"); return false;
            }
        }
        return true;
    }

    /*la rd, label */
    if (strcmp(name, "la") == 0) {
        if (!expect_register_operand(parser, &operands[0])) { parser_error_expected(parser, "register"); return false; }
        if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }
        if (!expect_label_ref_operand(parser, &operands[1])) { parser_error_expected(parser, "label reference"); return false; }  
        return true;
    }

    /* j label | call label | tail label */
    if (strcmp(name, "j") == 0 || strcmp(name, "call") == 0 || strcmp(name, "tail") == 0) {
        if (!expect_label_ref_operand(parser, &operands[0])) { parser_error_expected(parser, "label reference"); return false;}  
        return true;
    }

    parser_error(parser, "unknown pseudo");
    return false;
}

/*
 * parse_x_operands()
 *
 * Operand parsing functions for each of the six instruction formats (R, I, S, B, U, J);
 */
/* R-type */
static bool parse_r_operands(Parser *parser, Operand *operands) 
{
    if (!expect_register_operand(parser, &operands[0])) { parser_error_expected(parser, "register"); return false; }
    if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }
    if (!expect_register_operand(parser, &operands[1])) { parser_error_expected(parser, "register"); return false; }
    if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }
    if (!expect_register_operand(parser, &operands[2])) { parser_error_expected(parser, "register"); return false; }
    return true;
}
/* I-type */
static bool parse_i_operands(Parser *parser, Operand *operands) 
{   
    
    /* I-type instrucstions with fixed values */
    Statement *stmt = &parser->statements[parser->statement_count - 1];
    if (stmt->payload.instruction.kind == IK_REAL) {
        const char * mnemonic = stmt->payload.instruction.mnemonic.real->mnemonic;
        if (strcmp(mnemonic, "ecall") == 0) {           /* ecall */
            operands[0].kind = OK_REGISTER;
            operands[1].kind = OK_REGISTER;
            operands[2].kind = OK_IMMEDIATE;
            operands[0].value.reg_number = 0;
            operands[1].value.reg_number = 0;
            operands[2].value.imm_value  = 0;
            return true;
        }
        else if (strcmp(mnemonic, "ebreak") == 0) {  /* ebreak */
            operands[0].kind = OK_REGISTER;
            operands[1].kind = OK_REGISTER;
            operands[2].kind = OK_IMMEDIATE;
            operands[0].value.reg_number = 0;
            operands[1].value.reg_number = 0;
            operands[2].value.imm_value  = 1;
            return true;
        }
    }

    /* I-type instrucstions without fixed values */
    if (!expect_register_operand(parser, &operands[0])) { parser_error_expected(parser, "register"); return false; }
    if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }

    if (parser->lexer->current.type == TOKEN_REGISTER) {
        if (!expect_register_operand(parser, &operands[1])) { parser_error_expected(parser, "register"); return false; }
        if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }
        if (!expect_immediate_operand(parser, &operands[2])) { parser_error_expected(parser, "immediate"); return false; }
    }
    else if (parser->lexer->current.type == TOKEN_IMMEDIATE || parser->lexer->current.type == TOKEN_LABEL) {
        if(!expect_memory_operand(parser, &operands[1])) { parser_error_expected(parser, "memory operand"); return false; }
    }
    else { 
        parser_error_expected(parser, "register or memory operand"); 
        return false;
    }
    return true;
}
/* S-type */
static bool parse_s_operands(Parser *parser, Operand *operands)
{
    if (!expect_register_operand(parser, &operands[0])) { parser_error_expected(parser, "register"); return false; }
    if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }
    if(!expect_memory_operand(parser, &operands[1])) { parser_error_expected(parser, "memory operand"); return false; }
    return true;
}
/* B-type */
static bool parse_b_operands(Parser *parser, Operand *operands) 
{
    if (!expect_register_operand(parser, &operands[0])) { parser_error_expected(parser, "register"); return false; }
    if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }
    if (!expect_register_operand(parser, &operands[1])) { parser_error_expected(parser, "register"); return false; }
    if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }

    if (parser->lexer->current.type == TOKEN_LABEL) {
        if (!expect_label_ref_operand(parser, &operands[2])) { parser_error_expected(parser, "label reference"); return false; }
    }
    else if (parser->lexer->current.type == TOKEN_IMMEDIATE) {
        if (!expect_immediate_operand(parser, &operands[2])) { parser_error_expected(parser, "immediate"); return false; }
    }
    else { 
        parser_error_expected(parser, "label reference or immediate"); 
        return false;
    }
    return true;
}
/* U-type */
static bool parse_u_operands(Parser *parser, Operand *operands) 
{
    if (!expect_register_operand(parser, &operands[0])) { parser_error_expected(parser, "register"); return false; }
    if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }
    if (!expect_immediate_operand(parser, &operands[1])) { parser_error_expected(parser, "immediate"); return false; }
    return true;
}
/* J-type */
static bool parse_j_operands(Parser *parser, Operand *operands)
{
    if (!expect_register_operand(parser, &operands[0])) { parser_error_expected(parser, "register"); return false; }
    if (!expect_comma(parser)) { parser_error_expected(parser, "','"); return false; }

    if (parser->lexer->current.type == TOKEN_LABEL) {
        if (!expect_label_ref_operand(parser, &operands[1])) { parser_error_expected(parser, "label reference"); return false; }
    }
    else if (parser->lexer->current.type == TOKEN_IMMEDIATE) {
        if (!expect_immediate_operand(parser, &operands[1])) { parser_error_expected(parser, "immediate"); return false; }
    }
    else { 
        parser_error_expected(parser, "label reference or immediate"); 
        return false;
    }
    return true;
}



/*
 * parse_directive()
 *
 * Parses a directive.
 *
 * Details:
 * - Creates a new statement.
 * - For section directives, changes the current section.
 * - For other directives, dispatches to the corresponding directive parser.
 */
static bool parse_directive(Parser *parser) 
{
    Statement *stmt = parser_new_statement(parser, ST_DIRECTIVE);
    if (!stmt) {
        parser_error(parser, "statement building failed");
        return false;
    }

    ParsedDirective *directive = &stmt->payload.directive;

    if(!string_to_directive_type(parser->lexer->current.value, &directive->type)) {
        parser_error(parser, " unknown or unsupported directive");
        return false;
    }

    switch (directive->type) {
        case DT_TEXT:  
            if(!emitter_switch_section(parser->emitter, ST_TEXT)) return false;
            lexer_advance(parser->lexer);
            break;

        case DT_DATA:  
            if(!emitter_switch_section(parser->emitter, ST_DATA)) return false;
            lexer_advance(parser->lexer);
            break;

        case DT_GLOBAL:
            if (!parse_directive_global(parser, directive)) return false;
            break;

        case DT_ALIGN:
            if (!parse_directive_align(parser, directive)) return false;
            break;

        case DT_WORD:
        case DT_HALF:
        case DT_BYTE:
            if (!parse_directive_data(parser, directive)) return false;
            break;

        case DT_ASCII:
        case DT_ASCIZ:
            if (!parse_directive_string(parser, directive)) return false;
            break;

        default: 
            parser_error(parser, "unsupported directive type");
            return false;
    }

    return true;
}

/*
 * parse_directive_global()
 *
 * Parses a .global directive.
 *
 * Details:
 * - Registers the label reference in the symbol table.
 * - Fetches the symbol table entry and sets the is_global flag to 'true'.
 */
static bool parse_directive_global(Parser *parser, ParsedDirective *directive)
{
    lexer_advance(parser->lexer);

    if (parser->lexer->current.type != TOKEN_LABEL) {
        parser_error_expected(parser, "label reference");
        return false;
    }

    const char *name = parser->lexer->current.value;

    uint32_t offset;
    if (!emitter_current_offset(parser->emitter, &offset)) return false;
    if (!symbol_table_reference(parser->symbol_table, name, offset, parser->emitter->current_section)) {
        parser_error(parser, "failed to register global symbol reference");
        return false; 
    }

    SymbolEntry *symbol_entry = symbol_table_find(parser->symbol_table, name);
    if(symbol_entry == NULL) {
        parser_error(parser, "failed to lookup label in the symbol table");
        return false; 
    }
    symbol_entry->is_global = true;

    directive->payload.symbol_name = strdup(name);
    if (directive->payload.symbol_name == NULL) {
        parser_error(parser, "failed copying symbol name"); 
        return false;
    }

    lexer_advance(parser->lexer);
    return true;
}

/*
 * parse_directive_align()
 *
 * Parses a .align directive.
 *
 * Details:
 * - Parses the immediate.
 * - Dispatches to emitter_align to calculate the padding and adjust the current sections location offset.
 */ 
static bool parse_directive_align(Parser *parser, ParsedDirective *directive)
{
    lexer_advance(parser->lexer);

    if (parser->lexer->current.type != TOKEN_IMMEDIATE) {
        parser_error_expected(parser, "immediate");
        return false;
    }
        

    uint32_t alignment;
    if (!parser_string_to_uint32_t(parser->lexer->current.value, &alignment)) {
        parser_error(parser, "invalid alignment value"); 
        return false;
    }
    
    directive->payload.alignment = alignment;
    if(!emitter_align(parser->emitter, alignment)) return false;
    lexer_advance(parser->lexer);

    return true;
}

/*
 * parse_directive_data()
 *
 * Parses a .word / .half / .byte directive.
 *
 * Details:
 * - Copies all values into the statement.
 * - Advances the current sections location offset depending on the data type and data count.
 */ 
static bool parse_directive_data(Parser *parser, ParsedDirective *directive)
{
    lexer_advance(parser->lexer);

    size_t count = 0;
    Operand *values = directive->payload.data.values;

    while(true) {
        if (count > 15) {
            parser_error(parser, "max. 16 values after data directive");
            return false;
        }

        if (parser->lexer->current.type == TOKEN_IMMEDIATE) {
            if(!expect_immediate_operand(parser, &values[count])) return false;
        }
        else if(parser->lexer->current.type == TOKEN_LABEL) {
            if(!expect_label_ref_operand(parser, &values[count])) return false;
        }   
        else {
            parser_error_expected(parser, "immediate or label reference");
            return false;
        }

        count ++;

        if (parser->lexer->current.type != TOKEN_COMMA) {
            break;
        }

        lexer_advance(parser->lexer);
    }

    directive->payload.data.count = count;

    switch (directive->type) {
        case DT_WORD:   ;
            if (!emitter_advance(parser->emitter, (uint32_t)((size_t)4 * count))) return false;
            break;
        case DT_HALF:   
            if (!emitter_advance(parser->emitter, (uint32_t)((size_t)2 * count))) return false;
            break;
        case DT_BYTE:
            if (!emitter_advance(parser->emitter, (uint32_t)((size_t)1 * count))) return false;
            break;
        default: parser_error_expected(parser, ".word / .half / .byte");
    }

    return true;
}

/*
 * parse_directive_string()
 *
 * Parses a .ascii / .asciz directive.
 *
 * Details:
 * - Copies the string into the statement.
 * - Advances the current sections location offset depending on the string length and type.
 */ 
static bool parse_directive_string(Parser *parser, ParsedDirective *directive)
{
    lexer_advance(parser->lexer);

    if (parser->lexer->current.type != TOKEN_STRING) {
        parser_error_expected(parser, "string literal");
        return false;
    }

    size_t len = parser->lexer->current.value_len;
    if (len >1027) {
        parser_error(parser, "max. string length is 1027");
        return false;
    }

    directive->payload.string.value = strdup(parser->lexer->current.value);
    if (directive->payload.string.value  == NULL) {
        parser_error(parser, "failed copying string literal"); 
        return false;
    }
    
    directive->payload.string.length = len;

    if (directive->type == DT_ASCII) {
        directive->payload.string.is_null_terminated = false;
        if (!emitter_advance(parser->emitter, (uint32_t)len)) return false;
    }
    if (directive->type == DT_ASCIZ) {
        directive->payload.string.is_null_terminated = true;
        if (!emitter_advance(parser->emitter, (uint32_t)len + 1)) return false;
    }

    lexer_advance(parser->lexer);

    return true;
}

/*
 * string_to_directive_type()
 *
 * Helper that returns an enum DirectiveType value corresponding to the given string.
 */
static bool string_to_directive_type(const char *string, DirectiveType * type)
{
    if(string == NULL || type == NULL) return false;

    if (strcmp(string, ".text") == 0)        {*type = DT_TEXT;        return true;}
    if (strcmp(string, ".data") == 0)        {*type = DT_DATA;        return true;}
    if (strcmp(string, ".align") == 0)       {*type = DT_ALIGN;       return true;}
    if (strcmp(string, ".global") == 0)      {*type = DT_GLOBAL;      return true;}
    if (strcmp(string, ".globl") == 0)       {*type = DT_GLOBAL;      return true;}
    if (strcmp(string, ".word") == 0)        {*type = DT_WORD;        return true;}
    if (strcmp(string, ".half") == 0)        {*type = DT_HALF;        return true;}
    if (strcmp(string, ".byte") == 0)        {*type = DT_BYTE;        return true;}
    if (strcmp(string, ".ascii") == 0)       {*type = DT_ASCII;       return true;}
    if (strcmp(string, ".asciz") == 0)       {*type = DT_ASCIZ;       return true;}

    return false;
}


/* PASS 2 FUNCTIONS */

/*
 * parser_run_pass2()
 *
 * Starts the parser pass 2.
 *
 * Details:
 * - Readies the parser and emitter fields for parser pass 2.
 * - Goes through all the statements and dispatches to the function corresponding to the statement type for encoding.
 * - Finally dispatches to emitter_finish() to write all buffers into the output file.
 */
static bool parser_run_pass2(Parser *parser)
{
    parser->current_pass = 2;

    parser->emitter->section_start_text = 0;
    uint32_t start = 0;
    if(!emitter_get_section_offset(parser->emitter, ST_TEXT, &start)) return false;
    parser->emitter->section_start_data = start;
    
    if(!emitter_begin_pass2(parser->emitter)) return false;

    for (size_t i = 0; i < parser->statement_count; i++) {
        Statement *stmt = &parser->statements[i];

        switch (stmt->type) {
            case ST_LABEL_DEFINITION:
                break;
                
            case ST_DIRECTIVE:
                if(!parse_statement_directive(parser, stmt)) {
                    return false;
                }
                break;
            
            case ST_INSTRUCTION: 
                if(!encode_instruction(parser, stmt)) {
                    return false;
                }
                break;    
            
            case ST_PSEUDO_INSTRUCTION: 
                if (!encode_pseudo_instruction(parser, stmt)) {
                    return false;
                }
                break;

            default: 
                parser_error(parser, " unknown statement type in Pass 2");
                return false;
        }

        
    }

    return emitter_finish(parser->emitter);
}

/*
 * encode_instruction()
 *
 * Dispatches to the instruction encoder corresponding to the instruction format (R, B, U, J, I, S).
 */
static bool encode_instruction(Parser *parser, Statement *stmt)
{
    if (parser == NULL) {
        parser_error(parser, "Internal error: encode_instruction(): parser is NULL pointer");
        return false;
    }
    if (stmt == NULL) {
        parser_error(parser, "Internal error: encode_instruction(): stmt is NULL pointer");
        return false;
    }

    ParsedInstruction *instr = &stmt->payload.instruction;
    switch (instr->format) {
        case INSTR_R:
            if(!encode_r_instruction(parser, stmt)) { parser_error(parser, "failed encoding R-format instruction"); return false; }
            return true;

        case INSTR_B:
            if(!encode_b_instruction(parser, stmt)) { parser_error(parser, "failed encoding B-format instruction"); return false; }
            return true;
            
        case INSTR_U:
            if(!encode_u_instruction(parser, stmt)) { parser_error(parser, "failed encoding U-format instruction"); return false; }
            return true;

        case INSTR_J:
            if(!encode_j_instruction(parser, stmt)) { parser_error(parser, "failed encoding J-format instruction"); return false; }
            return true;

        case INSTR_I:
            if(!encode_i_instruction(parser, stmt)) { parser_error(parser, "failed encoding I-format instruction"); return false; }
            return true;

        case INSTR_S:
            if(!encode_s_instruction(parser, stmt)) { parser_error(parser, "failed encoding S-format instruction"); return false; }
            return true;

        default:
            parser_error(parser, "encode_instruction(): unsupported rv32i_format_t");
            return false;
    }

}

/*
 * encode_x_instruction()
 *
 * Encoder functions for each of the six instruction formats (R, I, S, B, U, J);
 *
 * Details:
 * - Hard validates the operands and extracts their values.
 * - Encodes the values into a 32 bit word using the encoding macro corresponding to the instruction format.
 * - Emits the word.
 */
/* R-type */
static bool encode_r_instruction(Parser *parser, Statement *stmt) 
{
    /* Stage 1: Validation and extraction. */
    ParsedInstruction *instr = &stmt->payload.instruction;
    int32_t rd = 0, rs1 = 0, rs2 = 0;

    if (instr->operands[0].kind != OK_REGISTER) { parser_error(parser, "R-type: operand[0] must be OK_REGISTER"); return false; }
    if (instr->operands[1].kind != OK_REGISTER) { parser_error(parser, "R-type: operand[1] must be OK_REGISTER"); return false; }
    if (instr->operands[2].kind != OK_REGISTER) { parser_error(parser, "R-type: operand[2] must be OK_REGISTER"); return false; }
    rd  = instr->operands[0].value.reg_number;
    rs1 = instr->operands[1].value.reg_number;
    rs2 = instr->operands[2].value.reg_number;

    /* Stage 2: Encoding and emitting. */
    const rv32i_instruction_t *real = instr->mnemonic.real;
    uint32_t word = ENCODE_R(real->opcode, real->funct3, real->funct7, rd, rs1, rs2);

    if (!emitter_emit_word(parser->emitter, word)) {
        parser_error(parser, "emitter failed to emit instruction");
        return false;
    }

    return true;
}
/* I-type */
static bool encode_i_instruction(Parser *parser, Statement *stmt) 
{   
    /* Stage 1: Validation and extraction. */
    ParsedInstruction *instr = &stmt->payload.instruction;
    int32_t rd = 0, rs1 = 0, imm = 0;

    if (instr->operands[0].kind != OK_REGISTER) { parser_error(parser, "I-type: operand[0] must be OK_REGISTER (rd)") ;return false; }
    rd = instr->operands[0].value.reg_number;

    if (instr->operands[1].kind == OK_MEMORY) {
        const Memory *mem = &instr->operands[1].value.memory;
        rs1 = mem->base_reg;

        if (mem->label != NULL) {
            SymbolEntry *e = symbol_table_lookup(parser->symbol_table, mem->label);
            if (e == NULL) { parser_error(parser, "undefined label in memory operand"); return false; }
            imm = (int32_t)e->address;
        } else {
            imm = mem->offset;
        }
    }
    else if (instr->operands[1].kind == OK_REGISTER) {
        rs1 = instr->operands[1].value.reg_number;

        if (instr->operands[2].kind != OK_IMMEDIATE && instr->operands[2].kind != OK_LABEL_REFERENCE) {
            parser_error(parser, "arithmetic I-type: operand[2] must be immediate or label");
            return false;
        }
        if (instr->operands[2].kind == OK_LABEL_REFERENCE) {
            SymbolEntry *e = symbol_table_lookup(parser->symbol_table, instr->operands[2].value.label_name);
            if (e == NULL) { parser_error(parser, "undefined label in I-type"); return false; }
            imm = (int32_t)e->address;
        } else {
            imm = instr->operands[2].value.imm_value;
        }
    }
    else {
        parser_error(parser, "I-type: operand[1] must be OK_MEMORY or OK_REGISTER");
        return false;
    }

    /* Stage 2: Encoding and emitting. */
    const rv32i_instruction_t *real = instr->mnemonic.real;

    /* Special handling for shift instructions (slli, srli, srai) */
    uint32_t final_imm = imm;
    if (real->funct3 == FUNCT3_SLL || real->funct3 == FUNCT3_SR) {
        final_imm = (real->funct7 << 5) | (imm & 0x1F);
    }

    uint32_t word = ENCODE_I(real->opcode, real->funct3, rd, rs1, final_imm);
       
    if (!emitter_emit_word(parser->emitter, word)) {
        parser_error(parser, "emitter failed to emit instruction");
        return false;
    }

     
    return true;
}
/* S-type */
static bool encode_s_instruction(Parser *parser, Statement *stmt) 
{
    /* Stage 1: Validation and extraction. */
    ParsedInstruction *instr = &stmt->payload.instruction;
    int32_t rs1 = 0, rs2 = 0, imm = 0;

    if (instr->operands[0].kind != OK_REGISTER) { parser_error(parser, "S-type: operand[0] must be OK_REGISTER (rs2)"); return false; }
    rs2 = instr->operands[0].value.reg_number;

    if (instr->operands[1].kind != OK_MEMORY) { parser_error(parser, "S-type: operand[1] must be OK_MEMORY (rs2)"); return false; }
    const Memory *mem = &instr->operands[1].value.memory;
    rs1 = mem->base_reg;

    if (mem->label != NULL) {
        SymbolEntry *e = symbol_table_lookup(parser->symbol_table, mem->label);
        if (e == NULL) { parser_error(parser, "undefined label in memory operand"); return false; }
        imm = (int32_t)e->address;
    } else {
        imm = mem->offset;
    }

    /* Stage 2: Encoding and emitting. */
    const rv32i_instruction_t *real = instr->mnemonic.real;
    uint32_t word = ENCODE_S(real->opcode, real->funct3, rs1, rs2, imm);

    if (!emitter_emit_word(parser->emitter, word)) {
        parser_error(parser, "emitter failed to emit instruction");
        return false;
    }

     
    return true;
}
/* B-type */
static bool encode_b_instruction(Parser *parser, Statement *stmt) 
{
    /* Stage 1: Validation and extraction. */
    ParsedInstruction *instr = &stmt->payload.instruction;
    int32_t rs1 = 0, rs2 = 0, imm = 0;

    if (instr->operands[0].kind != OK_REGISTER) { parser_error(parser, "B-type: operand[0] must be OK_REGISTER"); return false; }
    if (instr->operands[1].kind != OK_REGISTER) { parser_error(parser, "B-type: operand[1] must be OK_REGISTER"); return false; }
    if (instr->operands[2].kind != OK_LABEL_REFERENCE && instr->operands[2].kind != OK_IMMEDIATE) {
        parser_error(parser, "B-type: operand[2] must be OK_LABEL_REFERENCE or OK_IMMEDIATE");
        return false;
    }
    rs1 = instr->operands[0].value.reg_number;
    rs2 = instr->operands[1].value.reg_number;
    if (instr->operands[2].kind == OK_LABEL_REFERENCE) {
        SymbolEntry *entry = symbol_table_lookup(parser->symbol_table, instr->operands[2].value.label_name);
        if (entry == NULL) { parser_error(parser, "undefined label in B-type"); return false; }
        imm = (int32_t)(entry->address - stmt->location);
    } else {
        imm = instr->operands[2].value.imm_value;
    }
    

    /* Stage 2: Encoding and emitting. */
    const rv32i_instruction_t *real = instr->mnemonic.real;
    uint32_t word = ENCODE_B(real->opcode, real->funct3, rs1, rs2, imm);

    if (!emitter_emit_word(parser->emitter, word)) {
        parser_error(parser, "emitter failed to emit instruction");
        return false;
    }

     
    return true;
}
/* U-type */
static bool encode_u_instruction(Parser *parser, Statement *stmt) 
{
    /* Stage 1: Validation and extraction. */
    ParsedInstruction *instr = &stmt->payload.instruction;
    int32_t rd = 0, imm = 0;

    if (instr->operands[0].kind != OK_REGISTER) { parser_error(parser, "U-type: operand[0] must be OK_REGISTER"); return false; }
    if (instr->operands[1].kind != OK_IMMEDIATE && instr->operands[1].kind != OK_LABEL_REFERENCE) {
        parser_error(parser, "U-type: operand[1] must be OK_IMMEDIATE or OK_LABEL_REFERENCE"); return false;
    }
    rd = instr->operands[0].value.reg_number;
    if (instr->operands[1].kind == OK_LABEL_REFERENCE) {
        SymbolEntry *e = symbol_table_lookup(parser->symbol_table, instr->operands[1].value.label_name);
        if (e == NULL) { parser_error(parser, "undefined label in U-type"); return false; }
        imm = (int32_t)e->address;
    } else {
        imm = instr->operands[1].value.imm_value;
    }    

    /* Stage 2: Encoding and emitting. */
    const rv32i_instruction_t *real = instr->mnemonic.real;
    uint32_t word = ENCODE_U(real->opcode, rd, imm);

    if (!emitter_emit_word(parser->emitter, word)) {
        parser_error(parser, "emitter failed to emit instruction");
        return false;
    }

     
    return true;  
}
/* J-type */
static bool encode_j_instruction(Parser *parser, Statement *stmt) 
{
    /* Stage 1: Validation and extraction. */
    ParsedInstruction *instr = &stmt->payload.instruction;
    int32_t rd = 0, imm = 0;

    if (instr->operands[0].kind != OK_REGISTER) { parser_error(parser, "J-type: operand[0] must be OK_REGISTER"); return false; }
    if (instr->operands[1].kind != OK_LABEL_REFERENCE && instr->operands[1].kind != OK_IMMEDIATE) {
        parser_error(parser, "J-type: operand[1] must be OK_LABEL_REFERENCE or OK_IMMEDIATE"); return false;
    }
    rd = instr->operands[0].value.reg_number;
    if (instr->operands[1].kind == OK_LABEL_REFERENCE) {
        SymbolEntry *entry = symbol_table_lookup(parser->symbol_table, instr->operands[1].value.label_name);
        if (entry == NULL) { parser_error(parser, "undefined label in J-type"); return false; }
        imm = (int32_t)(entry->address - stmt->location);
    } else {
        imm = instr->operands[1].value.imm_value;
    }

    /* Stage 2: Encode */
    const rv32i_instruction_t *real = instr->mnemonic.real;
    uint32_t word = ENCODE_J(real->opcode, rd, imm); 

    if (!emitter_emit_word(parser->emitter, word)) {
        parser_error(parser, "emitter failed to emit instruction");
        return false;
    }

     
    return true;
}

/*
 * encode_pseudo_instruction()
 *
 * Expands (if necessary) and encodes a pseudo instruction.
 * 
 * Details:
 * - Hard validates the operands and extracts their values.
 * - Calculate low and high bit values if the pseudo expands to two instructions.
 * - Encodes the values into a 32 bit word using the encoding macro corresponding to the instruction format.
 * - Emits the word(s).
 */
static bool encode_pseudo_instruction(Parser *parser, Statement *stmt)
{
    ParsedInstruction *instr = &stmt->payload.instruction;
    const char *name = instr->mnemonic.pseudo_name;

    /* nop -> addi x0, x0, 0 */
    if (strcmp(name, "nop") == 0) {
        uint32_t word = ENCODE_I(OPCODE_OP_IMM, FUNCT3_ADD_SUB, 0, 0, 0);
        if (!emitter_emit_word(parser->emitter, word)) return false;
         
        return true;
    }

    /* ret -> jalr x0, 0(x1) */
    if (strcmp(name, "ret") == 0) {
        uint32_t word = ENCODE_I(OPCODE_JALR, FUNCT3_JALR, 0, REG_RA, 0);
        if (!emitter_emit_word(parser->emitter, word)) return false;
         
        return true;
    }

    /* mv rd, rs -> addi rd, rs, 0 */
    if (strcmp(name, "mv") == 0) {
        if (instr->operands[0].kind != OK_REGISTER) { parser_error(parser, "pseudo 'mv': operand[0] must be OK_REGISTER"); return false; }
        if (instr->operands[1].kind != OK_REGISTER) { parser_error(parser, "pseudo 'mv': operand[1] must be OK_REGISTER"); return false; }
        uint32_t rd = instr->operands[0].value.reg_number;
        uint32_t rs1 = instr->operands[1].value.reg_number;
        uint32_t word = ENCODE_I(OPCODE_OP_IMM, FUNCT3_ADD_SUB, rd, rs1, 0);
        if (!emitter_emit_word(parser->emitter, word)) return false;
         
        return true;
    }

    /* li rd, imm -> lui rd, hi + addi rd, rd, lo */
    if (strcmp(name, "li") == 0) {
        if (instr->operands[0].kind != OK_REGISTER) { parser_error(parser, "pseudo 'li': operand[0] must be OK_REGISTER"); return false; }
        uint32_t rd = instr->operands[0].value.reg_number;
        int32_t imm;
        if (instr->operands[1].kind == OK_IMMEDIATE) { imm = instr->operands[1].value.imm_value; }
        else if (instr->operands[1].kind == OK_LABEL_REFERENCE) {
            SymbolEntry *entry = symbol_table_lookup(parser->symbol_table, instr->operands[1].value.label_name);
            if (!entry) { parser_error(parser, "pseudo 'li': undefined label"); return false; }
            imm = entry->address;
        }
        else { parser_error(parser, "pseudo 'li': operand[1] must be OK_IMMEDIATE or OK_LABEL_REFERENCE"); return false; }
        uint32_t hi, lo;
        if (imm >=0) {                                  /* negative imm */
            hi = (uint32_t)(imm + (int32_t)0x800) >> 12;
            lo = (uint32_t)(imm & 0xFFF);
        }
        else {                                          /* positive imm*/
            hi = (uint32_t)(((uint32_t)imm) >> 12);
            lo = (uint32_t)(imm & 0xFFF);
        }
        uint32_t word1 = ENCODE_U(OPCODE_LUI, rd, hi);
        uint32_t word2 = ENCODE_I(OPCODE_OP_IMM, FUNCT3_ADD_SUB, rd, rd, lo);

        if (!emitter_emit_word(parser->emitter, word1)) return false;
        if (!emitter_emit_word(parser->emitter, word2)) return false;
         
        return true;
    }

    /* j label -> jal x0, label */
    if (strcmp(name, "j") == 0) {
        if (instr->operands[0].kind != OK_LABEL_REFERENCE) { parser_error(parser, "pseudo 'j': operand[0] must be OK_LABEL_REFERENCE"); return false; }
        SymbolEntry *entry = symbol_table_lookup(parser->symbol_table, instr->operands[0].value.label_name);
        if (!entry) { parser_error(parser, "pseudo 'j': undefined label"); return false; }
         
        int32_t imm = (int32_t)entry->address - (int32_t)stmt->location;
        uint32_t word = ENCODE_J(OPCODE_JAL, 0, imm);
        if (!emitter_emit_word(parser->emitter, word)) return false;
         
        return true;
    }

    /* la rd label -> auipc rd, hi + addi rd lo*/
    if (strcmp(name, "la") == 0) {

        if (instr->operands[0].kind != OK_REGISTER) { parser_error(parser, "pseudo 'la': operand[0] must be OK_REGISTER"); return false; }
        if (instr->operands[1].kind != OK_LABEL_REFERENCE) { parser_error(parser, "pseudo 'la': operand[1] must be OK_LABEL_REFERENCE"); return false; }

        uint32_t rd = instr->operands[0].value.reg_number;
        SymbolEntry *entry = symbol_table_lookup(parser->symbol_table, instr->operands[1].value.label_name);
        if (!entry) { parser_error(parser, "pseudo 'la': undefined label"); return false; }

        int32_t imm = ((int32_t) parser->emitter->section_start_data - (int32_t)entry->address) - (int32_t)stmt->location;

        uint32_t hi = (uint32_t)(((int32_t)imm + 0x800) >> 12);
        uint32_t lo = (uint32_t)(imm & 0xFFF);
        uint32_t word1 = ENCODE_U(OPCODE_AUIPC, rd, hi);
        uint32_t word2 = ENCODE_I(OPCODE_OP_IMM, FUNCT3_ADD_SUB, rd, rd, lo);

        if(!emitter_emit_word(parser->emitter, word1)) return false;
        if(!emitter_emit_word(parser->emitter, word2)) return false;
         
        return true;
    }
    /* call / tail label – auipc ra, hi + jalr ra lo(ra)  /  auipc t1, hi + jalr x0 lo(t1) */
    if (strcmp(name, "call") == 0 || strcmp(name, "tail") == 0) {
        if (instr->operands[0].kind != OK_LABEL_REFERENCE) { parser_error(parser, "pseudo 'call/tail': operand[0] must be OK_LABEL_REFERENCE"); return false; }
        SymbolEntry *entry = symbol_table_lookup(parser->symbol_table, instr->operands[0].value.label_name);
        if (!entry) { parser_error(parser, "pseudo 'call/tail': undefined label"); return false; }
        int32_t imm = (int32_t)entry->address - (int32_t)stmt->location;

        uint32_t hi = (uint32_t)(((int32_t)imm + 0x800) >> 12);
        uint32_t lo = (uint32_t)(imm & 0xFFF);

         uint32_t word1, word2;
        if (strcmp(name, "call") == 0) {
            word1 = ENCODE_U(OPCODE_AUIPC, REG_RA, hi);
            word2 = ENCODE_I(OPCODE_JALR, FUNCT3_JALR, REG_RA, REG_RA, lo);
        }
        else {
            word1 = ENCODE_U(OPCODE_AUIPC, REG_T1, hi);
            word2 = ENCODE_I(OPCODE_JALR, FUNCT3_JALR, 0, REG_T1, lo);
        }

        if(!emitter_emit_word(parser->emitter, word1)) return false;
        if(!emitter_emit_word(parser->emitter, word2)) return false;

        return true;
    }

    parser_error(parser, "pseudo expansion not implemented");
    return false;
}

/*
 * parse_statement_directive()
 *
 * Parses a directive statement in parser pass 2.
 * 
 * Details:
 * - Either: - Switches the current section buffer.
 *           - Or emits alignment (padding) bytes into the current section buffer.
 *           - Or emits data (words / halfs / bytes) into the current section buffer.
 *           - Or emits a string into the current section buffer.
 */
static bool parse_statement_directive(Parser *parser, Statement * stmt) 
{
    ParsedDirective * d = &stmt->payload.directive;

    switch (d->type) {
        case DT_TEXT:  
            if(!emitter_switch_section(parser->emitter, ST_TEXT)) return false;//???
            break;

        case DT_DATA:  
            if(!emitter_switch_section(parser->emitter, ST_DATA)) return false; //???
            break;

        case DT_GLOBAL:
            break;    

        case DT_ALIGN:
            if (!emitter_align(parser->emitter, d->payload.alignment)) {
                return false;
            }
            break;

        case DT_WORD:
        case DT_HALF:
        case DT_BYTE:
            for (size_t count = 0 ; count < d->payload.data.count; count++) {
                Operand *op = &d->payload.data.values[count];
                uint32_t value;

                if(op->kind == OK_IMMEDIATE) {
                    value = (uint32_t)op->value.imm_value;
                }
                else if(op->kind == OK_LABEL_REFERENCE) {
                    SymbolEntry *entry = symbol_table_lookup(parser->symbol_table, op->value.label_name);
                    if(entry == NULL) {
                        parser_error(parser, "undefined label in data directive");
                        return false; 
                    }
                    value = entry->address;
                }
                else {
                    parser_error(parser, "unsupported operand in data directive");
                    return false;
                }

                if (d->type == DT_WORD) {
                    if (!emitter_emit_word(parser->emitter, value)) return false;
                } else if (d->type == DT_HALF) {
                    if (!emitter_emit_half(parser->emitter, (uint16_t)value)) return false;
                } else {
                    if (!emitter_emit_byte(parser->emitter, (uint8_t)value)) return false;
                }
            }
            break;

        case DT_ASCII:
            if(!emitter_emit_bytes(parser->emitter,
                                   (const uint8_t*)d->payload.string.value,
                                   d->payload.string.length)) {
                return false;
            }
            break;

        case DT_ASCIZ:
            if(!emitter_emit_bytes(parser->emitter,
                                   (const uint8_t*)d->payload.string.value,
                                   d->payload.string.length + 1)) {
                return false;
            }
            break;

        default: 
            parser_error(parser, "unsupported directive type in Pass 2");
            return false;
    }

    return true;
}


/* HELPER FUNCTIONS */

/* 
 * expect_x_operand()
 *
 * Helpers that validate the expected lexer tokens and copy their values into the operand fields of the current statement.
 */
static bool expect_register_operand(Parser *parser, Operand *operand)
{
    if (parser->lexer->current.type != TOKEN_REGISTER) return false;
    operand->kind = OK_REGISTER;
    operand->value.reg_number = rv32i_register_number(parser->lexer->current.value);
    if(operand->value.reg_number == -1) return false;
    lexer_advance(parser->lexer);
    return true;
}
static bool expect_label_ref_operand(Parser *parser, Operand *operand) 
{       
    operand->kind = OK_LABEL_REFERENCE;
    operand->value.label_name = NULL;

    uint32_t offset;
    if (!emitter_current_offset(parser->emitter, &offset)) return false;
    if(!symbol_table_reference(parser->symbol_table, parser->lexer->current.value, offset, parser->emitter->current_section)) {
        parser_error(parser, "failed to register label reference");
        return false; 
    }

    operand->value.label_name = strdup(parser->lexer->current.value);
    if (operand->value.label_name  == NULL) {
        parser_error(parser, "failed copying label name"); 
        return false;
    }


    lexer_advance(parser->lexer);
    return true;
}
static bool expect_comma(Parser *parser) 
{
    if (parser->lexer->current.type != TOKEN_COMMA) return false;
    lexer_advance(parser->lexer);
    return true;
}
static bool expect_lparen(Parser *parser) 
{
    if (parser->lexer->current.type != TOKEN_LPAREN) return false;
    lexer_advance(parser->lexer);
    return true;
}
static bool expect_rparen(Parser *parser) 
{
    if (parser->lexer->current.type != TOKEN_RPAREN) return false;
    lexer_advance(parser->lexer);
    return true;
}
static bool expect_immediate_operand(Parser *parser, Operand *operand)
{
    if (parser->lexer->current.type != TOKEN_IMMEDIATE) return false;

    operand->kind = OK_IMMEDIATE;

    if(!parser_string_to_int32_t(parser->lexer->current.value, &operand->value.imm_value)) {
        parser_error(parser, "invalid immediate"); 
        return false;
    }

    lexer_advance(parser->lexer);
    return true;
}
static bool expect_memory_operand(Parser *parser, Operand *operand) 
{
    operand->kind = OK_MEMORY;
    operand->value.memory.label = NULL; 
    operand->value.memory.offset = 0; 
    operand->value.memory.base_reg = 0;

    if (parser->lexer->current.type == TOKEN_IMMEDIATE) {
        if (!parser_string_to_int32_t(parser->lexer->current.value, &operand->value.memory.offset)) {
            parser_error(parser, "invalid immediate"); 
            return false;
        }
        lexer_advance(parser->lexer);
    }
    else if(parser->lexer->current.type == TOKEN_LABEL) {
        uint32_t offset;
        if (!emitter_current_offset(parser->emitter, &offset)) return false;
        if(!symbol_table_reference(parser->symbol_table, parser->lexer->current.value, offset, parser->emitter->current_section)) {
            parser_error(parser, "failed to register label reference");
            return false; 
        }
        
        operand->value.memory.label = strdup(parser->lexer->current.value);
        if (operand->value.memory.label == NULL) {parser_error(parser, "failed copying label name"); return false;}

        lexer_advance(parser->lexer);
    }
    else return false;

    if (!expect_lparen(parser)) {parser_error_expected(parser, "'('"); return false;}
    if (!expect_base_register_operand(parser, &operand->value.memory.base_reg)) {parser_error_expected(parser, "register"); return false;}
    if (!expect_rparen(parser)) {parser_error_expected(parser, "')'"); return false;}

    return true;
}
static bool expect_base_register_operand(Parser *parser, int32_t *base_reg)
{
    if (parser->lexer->current.type != TOKEN_REGISTER) return false;

    *base_reg = rv32i_register_number(parser->lexer->current.value);
    if(*base_reg == -1) return false;

    lexer_advance(parser->lexer);
    return true;
}

/*
 * parser_string_to_x
 * 
 * Helpers that convert a given string to an int32_t / uint32_t and store it in the given destination.
 */
 static bool parser_string_to_int32_t(const char* string, int32_t *dest)
{
    if (string == NULL) return false;

    char *endptr;
    errno = 0;
    long val = strtol(string, &endptr, 0);

    if(*endptr != '\0' || errno == ERANGE) return false;
    



    *dest = (int32_t) val;
    return true;

}
static bool parser_string_to_uint32_t(const char* string, uint32_t *dest)
{
    if (string == NULL || dest == NULL) return false;
    if (string[0] == '-') return false;

    char *endptr;
    errno = 0;

    unsigned long val = strtoul(string, &endptr, 0);

    if (*endptr != '\0' || errno == ERANGE || val > UINT32_MAX) return false;

    *dest = (uint32_t)val;
    return true;
}



/* ERROR REPORTING */

/* 
 * parser_error()
 * 
 * Writes a given error message with line and column into 'stderr'.
 */
 static void parser_error(Parser *parser, const char *message) 
{
    if(parser == NULL || parser->lexer == NULL) {  
        fprintf(stderr, "Internal Error: %s\n", message != NULL ? message : "");
        return;
    }

    const char *got = (parser->lexer->current.value != NULL) 
        ? parser->lexer->current.value 
        : "<unknown>";

    if(message == NULL) {
        fprintf(stderr, "Error: at line %zu, column %zu\n", parser->lexer->line, parser->lexer->column);
        return;
    }
    fprintf(stderr, "Error at line %zu, column %zu: %s '%s'\n", 
            parser->lexer->line, parser->lexer->column, 
            message != NULL ? message : "", 
            got);
}
/* 
 * parser_error()
 * 
 * Writes an error message for expected values/types/etc. with line and column into 'stderr'.
 */
static void parser_error_expected(Parser *parser, const char *expected) 
{
    if(parser == NULL || parser->lexer == NULL) {
        fprintf(stderr, "Error: expected %s: got <unknown>\n",
                expected !=NULL ? expected : "<unknown>");
        return;
    }

    const char *got = (parser->lexer->current.value != NULL) ? parser->lexer->current.value : "<unknown>";

    fprintf(stderr, "Error at line %zu, column %zu: expected %s, got '%s'\n",
            parser->lexer->line, parser->lexer->column,
            expected !=NULL ? expected : "<unknown>",
            got);
}

