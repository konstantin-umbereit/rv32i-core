/*
 * symbol_table.h - RV32I Assembler
 *
 * Public API of the symbol table.
 */
 
#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdint.h>  
#include <stdbool.h> 
#include <stddef.h>   

#include "emitter.h"


/* STRUCTS */
 
typedef struct SymbolEntry SymbolEntry;
struct SymbolEntry {
    const char  *name;          
    uint32_t     address;       
    SectionType  section_type;
    bool         is_defined;    /* true after label definition parsed */
    bool         is_global;     /* .global directive flag */
    
    SymbolEntry *next_entry;   
};

typedef struct SymbolTable {
    SymbolEntry **buckets;      /* array of pointers to first SymbolEntry in each bucket */
    size_t        num_buckets;  /* size of the buckets array */
    size_t        num_symbols;  /* total number of symbols tracked */
} SymbolTable;


/* PUBLIC API FUNCTIONS */

/* Initializes the given SymbolTable with the specified number of buckets. */
bool symbol_table_init(SymbolTable *table, const size_t num_buckets);

/* Inserts a new symbol definition or updates an existing one. */
bool symbol_table_define(SymbolTable *table, const char * name, uint32_t  address, SectionType section_type);

/*  Records that a label is referenced, creates a placeholder entry if the label hasn't been seen yet. */
bool symbol_table_reference(SymbolTable *table, const char * name, uint32_t address, SectionType section_type);

/* Looks up a symbol by name, Returns pointer to the full entry or NULL if not found/undefined. */
SymbolEntry *symbol_table_lookup(SymbolTable *table, const char *name);

/* Pure lookup, NEVER creates an entry and NEVER prints 'undefined label' error. */
SymbolEntry *symbol_table_find(SymbolTable *table, const char *name);

/* Frees an entire SymbolTable (all entries + buckets array). */
bool symbol_table_free(SymbolTable *table);


#endif /* SYMBOL_TABLE_H */
