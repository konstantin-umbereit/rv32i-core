/*
 * emitter.h RV32I Assembler
 * 
 * Public API of the emitter.
 */
 
#ifndef EMITTER_H
#define EMITTER_H

#include <stdio.h>     
#include <stdint.h>    
#include <stddef.h>    
#include <stdbool.h>   


/* ENUMS */

typedef enum {
    EMIT_MODE_LAYOUT, EMIT_MODE_REAL,
} EmitMode;
typedef enum {
    ST_TEXT, ST_DATA,
} SectionType;


/* STRUCTS */

typedef struct Section {
    uint8_t    *buffer;         /* Dynamic byte buffer */
    size_t      capacity;       /* Allocated size */
    size_t      size;           /* Current written bytes */
    uint32_t    offset;
} SectionBuffer;

typedef struct Emitter {
    EmitMode        mode;
    FILE           *file;             /* output file */

    SectionType     current_section;

    SectionBuffer   text;
    SectionBuffer   data;

    /* start locations of the sections relative to the full instruction buffer, 
    gets set at the start of parser pass 2 because all statements have been build then */
    uint32_t section_start_text;  
    uint32_t section_start_data;
    
    const char     *filename;         /* For error messages and emitter_finish() */
    bool            is_initialized;   /* Safety guard */
} Emitter;


/* PUBLIC API */

/* Initializes the given emitter struct. */
bool emitter_initialize(Emitter *emitter, const char *output_filename);
/* Frees the given emitter struct. */
bool emitter_free(Emitter *emitter); 
/* Readies the emitter fiels for parser pass 1. */
bool emitter_begin_pass1(Emitter *emitter);
/* Readies the emitter fields for parser pass 2. */
bool emitter_begin_pass2(Emitter *emitter);

/* Sends a given amount of bytes to the byte appender. */
bool emitter_emit_bytes(Emitter *emitter, const uint8_t *data, size_t length);
/* Sends a given byte to the byte appender. */
bool emitter_emit_byte(Emitter *emitter, uint8_t value);
/* Splits a given 16 bit value into 2 bytes and appends them to the byte appender. */ 
bool emitter_emit_half(Emitter *emitter, uint16_t value);
/* Splits a given 32 bit value into 4 bytes and sends them to the byte appender. */
bool emitter_emit_word(Emitter *emitter, uint32_t value) ;

/* Adjusts the emitters state to the given alignment. */
bool emitter_align(Emitter *emitter, uint32_t alignment);
/* Advances the offset of the emitters current sections buffer. */
bool emitter_advance(Emitter *emitter, uint32_t bytes);
/* Sets the emitters current_section field to the given section type. */
bool emitter_switch_section(Emitter *emitter, SectionType section_type);

/* Writes the offset of the current sections buffer into the given output. */
bool emitter_current_offset(Emitter *emitter, uint32_t *out_offset);
/* Writes the offset of the sections buffer (corresponding to the given section type) into the given output. */
bool emitter_get_section_offset(Emitter *emitter, SectionType section_type, uint32_t *out_offset);

/* Writes all section buffer into the output file. */
bool emitter_finish(Emitter *emitter);


#endif /* EMITTER_H */
