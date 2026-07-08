/*
 * lexer.c - RV32I Assembler
 *
 * Converts a raw character stream into tokens for the parser.
 */

#include <stdlib.h>          
#include <string.h>          
#include <ctype.h>           

#include "rv32i.h"           
#include "lexer.h"           
#include "parser.h"          

static void lexer_skip_whitespace_and_comment(Lexer *lexer);
static void lexer_skip_single_line_comment(Lexer *lexer);

static Token lexer_read_comma(Lexer *lexer);
static Token lexer_read_lparen(Lexer *lexer);
static Token lexer_read_rparen(Lexer *lexer);
static Token lexer_read_string(Lexer *lexer);
static Token lexer_read_immediate(Lexer *lexer);
static Token lexer_read_remaining(Lexer *lexer);

static bool lexer_is_immediate(Lexer *lexer, const char *value, const int value_len);
static bool lexer_is_binary(const char *value);
static bool lexer_is_hex(const char *value);
static bool lexer_is_decimal(const char *value);
static bool lexer_is_directive(const char *value);
static bool lexer_is_label(const char *value, const int i);

static int lexer_char_counter(const char *value, const int value_len, const char c);
static void lexer_error(const Lexer *lexer, const char *message);


/* PUBLIC API FUNCTIONS */               

/*
 * lexer_initialize()
 * Initializes a lexer with the given source file.
 */
bool lexer_initialize(Lexer *lexer, const char *file_path)
{   
    if (lexer == NULL) {
        fprintf(stderr, "Error: lexer_initialize(): lexer is NULL pointer\n");
        return false;
    }
    if (file_path == NULL) {
        fprintf(stderr, "Error: lexer_initialize(): filePath is NULL pointer\n");
        return false;
    }

    if (lexer->input_file != NULL) {
        fclose(lexer->input_file );
        lexer->input_file  = NULL;
    }
    if (lexer->current.value  != NULL) {
        free(lexer->current.value);
        lexer->current.value = NULL;
    }
    if (lexer->lookahead.value  != NULL) {
        free(lexer->lookahead.value);
        lexer->lookahead.value = NULL;
    }

    lexer->input_file = fopen(file_path, "r");
    if (lexer->input_file == NULL)
    {
        fprintf(stderr, "Error: lexer_initialize(): cannot open file %s\n", file_path);
        return false;
    }
    lexer->line = 1;
    lexer->column = 1;

    lexer->current_char = fgetc(lexer->input_file);

    lexer->current.value      = NULL;
    lexer->current.type       = TOKEN_NONE;
    lexer->current.value_len = 0;
    lexer->lookahead.value    = NULL;
    lexer->lookahead.type     = TOKEN_NONE;
    lexer->lookahead.value_len = 0;
    lexer->has_lookahead      = false;


    return true;
}

/*
 * lexer_free()
 * 
 * Releases all resources owned by the lexer (including closing the input file).
 */
void lexer_free(Lexer *lexer)
{
    if(lexer == NULL) {
       fprintf(stderr, "Error: lexer_free(): lexer is NULL pointer \n");
    }

    if (lexer->input_file != NULL) {
        fclose(lexer->input_file);
        lexer->input_file = NULL;
    }
    if (lexer->current.value != NULL) {
        free(lexer->current.value);
        lexer->current.value = NULL;
    }
    if (lexer->lookahead.value != NULL) {
        free(lexer->lookahead.value);
        lexer->lookahead.value = NULL;
    }

}

/*
 * lexer_advance()
 *
 * Consumes the current token and advances to the next one.
 */
void lexer_advance(Lexer *lexer)
{
    if (lexer->has_lookahead) {
        if (lexer->current.value != NULL) {   
            free(lexer->current.value);
            lexer->current.value = NULL;  
        }
        lexer->current = lexer->lookahead; 
        lexer->lookahead.value = NULL;
        lexer->has_lookahead = false;

    }
    else {
        if (lexer->current.value != NULL) {
            free(lexer->current.value);
            lexer->current.value = NULL;    
        }
        lexer->current = lexer_get_next_token(lexer);
        

    }

    if (lexer->current.value != NULL) {
        lexer->column += (int)strlen(lexer->current.value);
    }
}

/*
 * lexer_peek()
 *
 * Gets the next token without consuming it.
 */
void lexer_peek(Lexer *lexer){
    if (!lexer->has_lookahead)
    {
        lexer->lookahead = lexer_get_next_token(lexer);
        lexer->has_lookahead = true;
    }
}

/*
 * lexer_get_next_token()
 * 
 * Returns the next token from the input stream.
 */
Token lexer_get_next_token(Lexer *lexer)
{   
    Token token = {0};

    lexer_skip_whitespace_and_comment(lexer);

    if (lexer->current_char == EOF) {
        token.type = TOKEN_EOF;
        return token;
    }
    else if (lexer->current_char == ',') {
        return lexer_read_comma(lexer);
    }
    else if (lexer->current_char == '(') {
        return lexer_read_lparen(lexer);
    }
    else if (lexer->current_char == ')') {
        return lexer_read_rparen(lexer);
    }
     else if (lexer->current_char == '"') {
        return lexer_read_string(lexer);
    }
    else if(lexer->current_char == '-' || isdigit(lexer->current_char)) {
        return lexer_read_immediate(lexer);
    }
    else {
        return lexer_read_remaining(lexer);
    }
}

/*
 * lexer_token_type_to_string()
 *
 * Helper that returns the given token type as a string.
 */
const char *lexer_token_type_to_string(TokenType type) {
    switch (type) {
    case TOKEN_NONE:        return "none";
    case TOKEN_EOF:         return "eof";
    case TOKEN_DIRECTIVE:   return "directive";
    case TOKEN_MNEMONIC:    return "mnemonic";
    case TOKEN_REGISTER:    return "register";
    case TOKEN_IMMEDIATE:   return "immediate";
    case TOKEN_STRING:      return "string";
    case TOKEN_LABEL:       return "label";
    case TOKEN_LABEL_DEF:   return "label_def";
    case TOKEN_COMMA:       return "comma";
    case TOKEN_LPAREN:      return "lparen";
    case TOKEN_RPAREN:      return "rparen";
    default:                return "";
    }
}


/* WHITESPACE & COMMENT SKIPPING */

/*
 * lexer_skip_whitespace_and_comments()
 * 
 * Skips whitespace and comments in the input stream.
 * 
 * Details:
 * - Skips spaces, tabs, new lines and , carriage returns 
 * - Skips single and multi line comments
 * - Terminates when next valid character or end of file is reached
 * - Leaves the first valid character in lexer->current_char
 */
static void lexer_skip_whitespace_and_comment(Lexer *lexer)
{   
    while (lexer->current_char != EOF) { 
        if (lexer->current_char == ' ' || lexer->current_char == '\t' || lexer->current_char == '\r') {
        } 
        else if (lexer->current_char == '\n') {       
            lexer->line++;
            lexer->column = 0;
        }
        else if (lexer->current_char == '#') {   
            lexer_skip_single_line_comment(lexer);
        }
        else { 
            break; 
        }
        lexer->current_char = fgetc(lexer->input_file);
        lexer->column++;

    }

}

/*
 * lexer_skip_single_line_comments()
 * 
 * Helper that skips a single-line comment.
 */
static void lexer_skip_single_line_comment(Lexer *lexer) 
{
    while (lexer->current_char != EOF ) {
        if (lexer->current_char == '\n') {
            lexer->line++;
            lexer->column = 0;
            break;
        }
        lexer->current_char = fgetc(lexer->input_file);
        lexer->column++;
    }
}


/* TOKEN READING FUNCTIONS  */

/*
 * lexer_read_comma()
 * 
 * Reads and builds a token of type TOKEN_COMMA.
 */
static Token lexer_read_comma(Lexer *lexer) {
    Token token = {0};

    token.value = malloc(2);
    if (token.value == NULL) {
        lexer_error(lexer, "memory allocation failed");      
        return token;
    }
    token.value[0] = ',';
    token.value[1] = '\0';
    token.type = TOKEN_COMMA;
    token.value_len = 1;
    lexer->current_char = fgetc(lexer->input_file);
    
    return token;
}

/*
 * lexer_read_lparen()
 * 
 * Reads and builds a token of type TOKEN_LPAREN.
 */
static Token lexer_read_lparen(Lexer *lexer) {
    Token token = {0};

    token.value = malloc(2);
    if (token.value == NULL) {
        lexer_error(lexer, "memory allocation failed");      
        return token;
    }
    token.value[0] = '(';
    token.value[1] = '\0';
    token.type = TOKEN_LPAREN;
    token.value_len = 1;
    lexer->current_char = fgetc(lexer->input_file);

    return token;
}

/*
 * lexer_read_rparen()
 * 
 * Reads and builds a token of type TOKEN_RPAREN.
 */
static Token lexer_read_rparen(Lexer *lexer) {
    Token token = {0};

    token.value = malloc(2);
    if (token.value == NULL) {
        lexer_error(lexer, "memory allocation failed");      
        return token;
    }
    token.value[0] = ')';
    token.value[1] = '\0';
    token.type = TOKEN_RPAREN;
    token.value_len = 1;
    lexer->current_char = fgetc(lexer->input_file);

    return token;
}

static Token lexer_read_string(Lexer *lexer) 
{
    Token token = {0};
    char value[1028];
    size_t i = 0;

    /* consume opening '"' */
    lexer->current_char = fgetc(lexer->input_file);

    while (lexer->current_char!= EOF && lexer->current_char != '"') { 
        if (i > 1026) {
            lexer_error(lexer, "string is too long (max 1027 characters)");
            return token;
        }

        if (lexer->current_char != '\\') {
            value[i++] = lexer->current_char;
        }
        else {
            lexer->current_char = fgetc(lexer->input_file);
             /* escape character handling*/
            switch (lexer->current_char) {
                case '"':   value[i++] = '"'; break;
                case '\\':  value[i++] = '\\'; break;
                case 'n':   value[i++] = '\n'; break;
                case 't':   value[i++] = '\t'; break;
                case 'r':   value[i++] = '\r'; break;
                case '0':   value[i++] = '\0'; break;
                default :  lexer_error(lexer, "invalid escape sequence"); return token;
            }
        }

        lexer->current_char = fgetc(lexer->input_file);
    }
    value[i] = '\0';

    if (lexer->current_char != '"') {
       lexer_error(lexer, "unclosed string");
       return token;
    }

    /* consume closing '"' */
    lexer->current_char = fgetc(lexer->input_file);


    token.value = malloc(strlen(value) + 1);
    if(token.value == NULL) {
        lexer_error(lexer, "memory allocation failed");
        return token;
    } 
    strcpy(token.value, value);
    token.type = TOKEN_STRING;
    token.value_len = i;
    return token;
}

/*
 * lexer_read_immediate()
 * 
 * Reads and builds a token of type TOKEN_IMMEDIATE.
 */
static Token lexer_read_immediate(Lexer *lexer) 
{
    Token token = {0};
    char value[128];
    size_t i = 0;

    value[i++] = lexer->current_char;
    lexer->current_char = fgetc(lexer->input_file);
    while (lexer->current_char!= EOF && (isdigit(lexer->current_char) || isalpha(lexer->current_char))) { 
        if (i > 126) {
            lexer_error(lexer, "immediate is too long (max 127 characters)");
            return token;
        }
        value[i++] = lexer->current_char;
        lexer->current_char = fgetc(lexer->input_file);


    }
    value[i] = '\0';


    if (!lexer_is_immediate(lexer, value, i)) {
        lexer_error(lexer, "invalid immediate");
        return token;
    }
    
    token.value = malloc(strlen(value) + 1);
    if(token.value == NULL) {
        lexer_error(lexer, "memory allocation failed");
        return token;
    } 
    strcpy(token.value, value);
    token.type = TOKEN_IMMEDIATE;
    token.value_len = i;

    return token;
}

static Token lexer_read_remaining(Lexer *lexer) 
{
    Token token = {0};
    char value[1024];
    size_t i = 0;

    value[i++] = lexer->current_char;
    lexer->current_char = fgetc(lexer->input_file);
    while (lexer->current_char!= EOF && 
           (isalpha(lexer->current_char) || isdigit(lexer->current_char) || lexer->current_char == ':'|| lexer->current_char == '.' ||  lexer->current_char == '_')) { 
        if (i > 127) {
            lexer_error(lexer, "token is too long (max 1023 characters)");
            return token;
        }
        value[i++] = lexer->current_char;
        lexer->current_char = fgetc(lexer->input_file);
    }
    value[i] = '\0';
    
    if (rv32i_is_mnemonic(value) || parser_is_pseudo_instruction(value)) {
        token.type = TOKEN_MNEMONIC;
    }
    else if (rv32i_register_number(value) >= 0) {
        token.type = TOKEN_REGISTER;
    }
    else if (lexer_is_directive(value)) {
        token.type = TOKEN_DIRECTIVE;
    }
    else if (lexer_is_label(value, i)) {
        if (value[i - 1] == ':') {
            token.type = TOKEN_LABEL_DEF;
            value[i - 1] = '\0';
        }
        else {
            token.type = TOKEN_LABEL;
        }
    }
    else {   
        lexer_error(lexer, "token type determination failed");
        return token;
    }

    token.value = malloc(i + 1);
    if(token.value == NULL) {
        lexer_error(lexer, "memory allocation failed");
        return token;
    } 
    strcpy(token.value, value);

    token.value_len = i;

    return token;
}



/* VALIDATION HELPERS */


/*
 * lexer_is_immediate();
 *
 * Helper that checks for correct usage of '-', 'x' and 'b' in a given immediate.
 */
static bool lexer_is_immediate(Lexer *lexer, const char *value, const int value_len) 
{   
    int sign = 0;
    if(value[0] == '-') sign = 1;
    if (value_len > (sign + 2)) {
        if (value[sign] == '0' && (value[sign + 1] == 'b' || value[sign + 1] == 'B')) {
            if (!lexer_is_binary(&value[sign + 2])) {
                lexer_error(lexer, "invalid binary number");

                return false;
            }
            return true;

        }
        if (value[sign] == '0' && (value[sign + 1] == 'x' || value[sign + 1] == 'X')) {
            if (!lexer_is_hex(&value[sign + 2])) {
                lexer_error(lexer, "invalid hexadecimal number");
                return false;
            }
            return true;

        }
    }

    if (!lexer_is_decimal(&value[sign])) {
        lexer_error(lexer, "invalid decimal number");
        return false;
    }

    return true;

}

/*
 * lexer_is_binary();
 *
 * Helper that checks whether the given string is a valid binary number.
 */
static bool lexer_is_binary(const char *value) {
    size_t i = 0;
    while (value[i] != '\0') {
        if (value[i] != '0' && value[i] != '1') {
            return false;
        }
        i++;
    }
  
    return true;
}

/*
 * lexer_is_hex();
 *
 * Helper that checks whether the given string is a valid hexadecimal number.
 */
static bool lexer_is_hex(const char *value) {
    size_t i = 0;
    while (value[i] != '\0') {
        if ((!isdigit(value[i])) && (!(value[i] >= 65 && value[i] <= 70)  && !(value[i] >= 97 && value[i] <= 102))) {
            return false;
        }
        i++;
    }

    return true;
}

/*
 * lexer_is_decimal();
 *
 * Helper that checks whether the given string is a valid decimal number.
 */
static bool lexer_is_decimal(const char *value) {
    size_t i = 0;
    while (value[i] != '\0') {
        if (value[i] < 48 || value[i] > 57 ) {
            return false;
        }
        i++;
    }
  
    return true;

}

/*
 * lexer_is_directive();
 *
 * Helper that checks whether the given string is a valid directive.
 */
static bool lexer_is_directive(const char *value) {
    const char * const DIRECTIVES[] = {".text", ".data", ".globl",".global",                                        
                                      ".align", ".word", ".half", ".byte",
                                      ".ascii", ".asciz", NULL};                         

    size_t i = 0;
    while(DIRECTIVES[i] != NULL) {
        if (strcmp(DIRECTIVES[i], value) == 0) {
            return true;
        }
        i++;
    }
    return false;
}

/*
 * lexer_is_label();
 *
 * Helper that checks whether the given string is a valid label.
 */
static bool lexer_is_label(const char *value, const int i) {
    if (lexer_char_counter(value, i, ':') < 2  && !(value[i-1] == ':' && i == 1)) {
        return true;
    }
    return false;
}

/*
 * lexer_char_counter();
 *
 * Helper that returns the number of occurrences of a given character in a given string.
 */
static int lexer_char_counter(const char *value, const int value_len,  const char c) 
{

    size_t counter = 0;
    for (int i = 0; i < value_len; i++) {
        if (value[i] == c) {
            counter++;
        }
    }
    return counter;
}


/* ERROR REPORTING */

/*
 * lexer_error()
 *
 * Reports a lexer error with current line and column.
 */
static void lexer_error(const Lexer *lexer, const char *message)
{   
    if (lexer == NULL) {
        fprintf(stderr, "Internal error: %s\n", message);
        return;
    }
    fprintf(stderr, "Error at line %zu, column %zu: %s\n", lexer->line, lexer->column, message);

}

