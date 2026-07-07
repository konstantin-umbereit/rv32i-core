/* 
 * main.c - RV32I Assembler
 *
 * See README.md for accepted instructions / directives.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "symbol_table.h"
#include "emitter.h"
#include "parser.h"

static void print_usage(const char *progname)
{
    fprintf(stderr, "Usage: %s <source.s>\n", progname);
    fprintf(stderr, "\nExample: ./assembler test.s\n");
    exit(2);   /* 2 = usage error */
}

int main(int argc, char **argv)
{


    /* INPUT VALIDATION */
    
    const char *input_file  = NULL;     
    const char *output_file = "output.bin";  
    if (argc < 2) {
        fprintf(stderr, "Error: missing input assembly file\n");
        print_usage(argv[0]);  
        
    }
    input_file = argv[1];
    printf("Assembling '%s' → '%s'\n", input_file, output_file);


    /* MODULE INITIALIZATION */
    
    Lexer       lexer   = {0};
    SymbolTable symtab  = {0};
    Emitter     emitter = {0};
    Parser      parser  = {0};
    if (!symbol_table_init(&symtab, 64)) {         
        fprintf(stderr, "Fatal: failed to allocate symbol table\n");
        return 1;
    }
    if (!lexer_initialize(&lexer, input_file)) {
        fprintf(stderr, "Error: cannot open input file '%s'\n", input_file);
        goto cleanup_symtab; 
    }
    if (!emitter_initialize(&emitter, output_file)) {
        fprintf(stderr, "Error: cannot create output file '%s'\n", output_file);
        goto cleanup_lexer;
    }
    if (!parser_initialize(&parser, &lexer, &symtab, &emitter)) {
        fprintf(stderr, "Error: parser initialization failed\n");
        goto cleanup_emitter;
    }


    /* EXECUTION */
    
    bool success = parser_run(&parser);   /* Runs parser pass 1 and parser pass 2 */
    if (success) {
         uint32_t text_bytes = 0;
        if(!emitter_get_section_offset(&emitter, ST_TEXT, &text_bytes)) {
            fprintf(stderr, "Warning: failed counting total emitted text bytes\n");
        }
        uint32_t data_bytes = 0;
        if(!emitter_get_section_offset(&emitter, ST_DATA, &data_bytes)) {
            fprintf(stderr, "Warning: failed counting total emitted data bytes\n");
        }
        printf("Success: %u bytes total (%u text bytes + %u data bytes) written to %s\n",
              text_bytes + data_bytes, text_bytes, data_bytes, output_file);
    } else if (!success) {
        fprintf(stderr, "Assembly failed - see error messages above.\n");
    }


    /* CLEANUP*/
    
    parser_free(&parser);      
cleanup_emitter:
    emitter_free(&emitter);
cleanup_lexer:
    lexer_free(&lexer);
cleanup_symtab:
    symbol_table_free(&symtab);


    return success ? 0 : 1; /* 0 = success, 1 = error */
}
