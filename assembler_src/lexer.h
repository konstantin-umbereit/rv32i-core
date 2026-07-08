/* 
 * lexer.h - RV32I Assembler
 *
 * Public API of the lexer.
 */

#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>      
#include <stdbool.h>    


/* ENUMS */

/*
 * TokenType
 * 
 * Represents all possible token types.
 */
typedef enum{
    TOKEN_NONE,      /* For initialization */
    TOKEN_EOF,       /* End of file has been reached */ 

    TOKEN_DIRECTIVE,
    TOKEN_MNEMONIC,
    TOKEN_REGISTER,
    TOKEN_IMMEDIATE,
    TOKEN_STRING,
    TOKEN_LABEL,
    TOKEN_LABEL_DEF,
    TOKEN_COMMA,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
} TokenType;


/* STRUCTS */

/*
 * Token
 * 
 * Represents a single token.
 */
typedef struct Token{
    TokenType type;
    char     *value;
    size_t    value_len; /*  - manually counted string length because strlen() returns wrong value for 
                               strings that have multiple '\0's 
                             - used when emitting bytes for DT_ASCII / DT_ASCIZ s*/
} Token;

/*
 * Lexer
 *
 * State holder of the lexer.
 */
typedef struct Lexer{   
    FILE *input_file; 
    size_t   line;
    size_t   column;
    char  current_char;
    Token current;
    Token lookahead; 
    bool has_lookahead;
} Lexer;


/* PUBLIC API FUNCTIONS */

/* Initializes a lexer with the given source file. */
bool lexer_initialize(Lexer *lexer, const char *file_path);

/* Releases all resources owned by the lexer (including closing the input file). */
void lexer_free(Lexer *lexer);

/* Consumes the current token and advances to the next one. */
void lexer_advance(Lexer *lexer);

/* Gets the next token without consuming it. */
void lexer_peek(Lexer *lexer);

/* Returns the next token from the input stream. */
Token lexer_get_next_token(Lexer *lexer);
/*  Helper that returns the given token type as a string. */
const char *lexer_token_type_to_string(TokenType type);

#endif /* LEXER_H */

