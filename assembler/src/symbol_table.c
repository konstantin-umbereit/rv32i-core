/*
 * symbol_table.c - RV32I Assembler
 *
 * Symbol table in the form of hash table.
 */
#define _POSIX_C_SOURCE 200809L /* access to POSIX.1-2008 functions, here: strdup() */

#include <stdbool.h>       
#include <stdio.h>         
#include <string.h>         
#include <ctype.h>          
#include <stdlib.h>

#include "symbol_table.h"   

#define SYMBOL_MAX_LABEL_LENGTH 1024


/* FORWARD DECLARATIONS */

static uint32_t symbol_table_hash (const char * name);


/* PUBLIC API FUNCTIONS */ 

/* 
 * symbol_table_init()
 * 
 * Initiates the given SymbolTable.
 * 
 * Details:
 * - Allocates memory for the given number of buckets
 * - Initiates the other fields
 */
bool symbol_table_init(SymbolTable *table, const size_t num_buckets) 
{
    if (table == NULL) {
        fprintf(stderr, "Error: in symbol_table_init(): table is NULL pointer\n");
        return false;
    }
    if (num_buckets < 1) {
        fprintf(stderr, "Error: in symbol_table_init(): num_buckets must be > 0\n");
        return false;
    }
   
    const size_t size = num_buckets * sizeof(*table->buckets);
    table->buckets = malloc(size);
    if(table->buckets == NULL) {
        fprintf(stderr, "Error: in symbol_table_init(): memory allocation failed\n");
        return false;
    }
    memset(table->buckets, 0, size);
    
    table->num_buckets = num_buckets;
    table->num_symbols = 0;

    return true;
}

/*
 * symbol_table_define()
 * 
 * Inserts a new symbol into the symbol table, or updates an existing one.
 * 
 * Details:
 * - Hashes name and goes to correct bucket's linked list
 * - Goes through the list and handles a duplicated definition
 * - Adds new entry as a new head of the list
 * - Marks entry as defined
 * - Increases the symbol tables symbol counter
 */
bool symbol_table_define(SymbolTable *table, const char * name, uint32_t address, SectionType section_type) 
{
    if (table == NULL) {
        fprintf(stderr, "Error: in symbol_table_define(): table is NULL pointer\n");
        return false;
    }
    if (name == NULL) {
        fprintf(stderr, "Error: in symbol_table_define(): name is NULL pointer\n");
        return  false;
    }

    size_t bucket_index = (size_t)(symbol_table_hash(name)) % table->num_buckets;
    SymbolEntry *current_entry = table->buckets[bucket_index];
    while(current_entry != NULL) {
        if (strcmp(current_entry->name, name) == 0) {
            if (current_entry->is_defined == true) {
                fprintf(stderr, "Warning: symbol_table_define(): redefinition of label %s\n", name);
            }
        
            current_entry->is_defined = true;
            current_entry->address = address;
            current_entry->section_type = section_type;
        
            return true;
        }     
        current_entry = current_entry->next_entry;
    }

    SymbolEntry *new_entry = malloc(sizeof(**table->buckets));
    if(new_entry == NULL) {
        fprintf(stderr, "Error: in symbol_table_define(): memory allocation failed\n");
        return false;
    }
    new_entry->name = strdup(name);
    if(new_entry->name == NULL) {
        fprintf(stderr, "Error: in symbol_table_define(): memory allocation failed\n");
        free(new_entry);
        return false;
    }
    new_entry->address = address;
    new_entry->is_defined = true;
    new_entry->is_global = false;
    new_entry->section_type = section_type;
    new_entry->next_entry = table->buckets[bucket_index];
    table->buckets[bucket_index] = new_entry;

    table->num_symbols++;
    return true;
}

/*
 * symbol_table_reference()
 * 
 * Inserts a reference to a used label into the symbol table.
 * 
 * Details:
 * - Hashes name and goes to correct bucket's linked list
 * - Goes through the list and returns if a corresponding entry already exists
 * - If not, Adds new entry as a new head of the list
 * - Marks entry as undefined
 * - Increases the symbol tables symbol counter
 * 
 * Note: Address parameter is only included for future usage.
 */
bool symbol_table_reference(SymbolTable *table, const char * name, uint32_t address, SectionType section_type) 
{
    if (table == NULL) {
        fprintf(stderr, "Error: in symbol_table_reference(): table is NULL pointer\n");
        return false;
    }
    if (name == NULL) {
        fprintf(stderr, "Error: in symbol_table_reference(): name is NULL pointer\n");
        return  false;
    }

    size_t bucket_index = (size_t)(symbol_table_hash(name)) % table->num_buckets;
    SymbolEntry *current_entry = table->buckets[bucket_index];
    while(current_entry != NULL) {
        if (strcmp(current_entry->name, name) == 0) {
            return true;
        }     
        current_entry = current_entry->next_entry;
    }

    SymbolEntry *new_entry = malloc(sizeof(**table->buckets));
    if(new_entry == NULL) {
        fprintf(stderr, "Error: in symbol_table_reference(): memory allocation failed\n");
        return false;
    }
    new_entry->name = strdup(name);
    if(new_entry->name == NULL) {
        fprintf(stderr, "Error: in symbol_table_reference(): memory allocation failed\n");
        free(new_entry);
        return false;
    }
    new_entry->address = address;
    new_entry->is_defined = false;
    new_entry->is_global = false;
    new_entry->section_type = section_type;
    new_entry->next_entry = table->buckets[bucket_index];
    table->buckets[bucket_index] = new_entry;

    table->num_symbols++;
    return true;
}

/*
 * symbol_table_lookup()
 * 
 * Looks up a symbol name in the symbol table and returns it's full entry.
 * 
 * Details:
 * - Hashes name and goes to correct bucket's linked list
 * - Goes through the list 
 * - Returns pointer to the SymbolEntry (or NULL if not found)
 * - */
SymbolEntry *symbol_table_lookup(SymbolTable *table, const char *name) 
{
    if (table == NULL) {
        fprintf(stderr, "Error: in symbol_table_lookup(): table is NULL pointer\n");
        return NULL;
    }
    if (name == NULL) {
        fprintf(stderr, "Error: in symbol_table_lookup(): name is NULL pointer\n");
        return  NULL;
    }

    size_t bucket_index = (size_t)(symbol_table_hash(name)) % table->num_buckets;
    SymbolEntry *current_entry = table->buckets[bucket_index];

    while(current_entry != NULL) {
        if (strcmp(current_entry->name, name) == 0) {
            if (current_entry->is_defined == false) {
                fprintf(stderr, "Error: in symbol_table_lookup(): undefined label %s \n", name);
                return NULL;
            }
            return current_entry;
        }     
        current_entry = current_entry->next_entry;
    }
    return NULL;
}

/* 
 * symbol_table_find()
 *
 * Pure lookup.
 *
 * Details: 
 * - Returns the entry if it exists (even if is_defined == false), otherwise NULL.
 * - NEVER creates an entry.
 * - NEVER prints "undefined label" error.
 * 
 * Note: The address field value of the returned entry is to be treated as undefined.
 */
SymbolEntry *symbol_table_find(SymbolTable *table, const char *name)
{
    if (table == NULL) {
        fprintf(stderr, "Error: in symbol_table_lookup(): table is NULL pointer\n");
        return NULL;
    }
    if (name == NULL) {
        fprintf(stderr, "Error: in symbol_table_lookup(): name is NULL pointer\n");
        return  NULL;
    }

    size_t bucket_index = (size_t)(symbol_table_hash(name)) % table->num_buckets;
    SymbolEntry *current_entry = table->buckets[bucket_index];

    while(current_entry != NULL) {
        if (strcmp(current_entry->name, name) == 0) {
            return current_entry;
        }     
        current_entry = current_entry->next_entry;
    }
    return NULL;
}

/*
 * symbol_table_free()
 * 
 * Frees an entire SymbolTable.
 * 
 * Details:
 * - goes through the symbol tables linked list and through each bucket's linked list
 * - frees the allocated memory of each symbol entry inside the buckets
 * - frees the allocated memory of the buckets themselves
 * - resets the bucket and the symbol counter
 */
bool symbol_table_free(SymbolTable *table) 
{
    if (table == NULL) return false;

    for (size_t i = 0; i < table->num_buckets; i++) {
        if(table->buckets[i] != NULL) {
            SymbolEntry *current_entry = table->buckets[i];

            while(current_entry != NULL) {
                SymbolEntry *temp_entry = current_entry->next_entry;
                free((void*)current_entry->name);
                free(current_entry);
                current_entry = temp_entry;
            }  
        }
    }
    free(table->buckets);
    table->buckets = NULL;
    table->num_buckets = 0;
    table->num_symbols = 0;

    return true;
}

/* STATIC HELPER FUNCTIONS */

/*
* symbol_table_hash()
*
* Computes and returns the hash for a given name using the FNV-1a function.
*/
static uint32_t symbol_table_hash (const char * name) 
{   
    if (name == NULL) return 0x811C9DC5;

    uint32_t fnv_offset_basis = 0x811C9DC5;
    uint32_t fnv_prime = 0x01000193;

    uint32_t hash = fnv_offset_basis;
    size_t i = 0;
    while(name[i] != '\0' && i < SYMBOL_MAX_LABEL_LENGTH) {
        hash = hash ^ name[i];
        hash = hash * fnv_prime;
        i++;
    }
    return hash;
}
