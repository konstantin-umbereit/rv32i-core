/* 
 * emitter.c - RV32I Assembler
 *
 * Writes bytes into the section buffers and finally writes those buffers into a single output file.
 */
 
#define _POSIX_C_SOURCE 200809L /* access to POSIX.1-2008 functions, here: strdup() */

#include <stdio.h>              
#include <stdint.h>             
#include <string.h>             
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>            

#include "emitter.h"


/* SECTION BUFFERS INITIAL SIZES */

#define EMITTER_INITIAL_TEXT_BUFFER_SIZE 4096
#define EMITTER_INITIAL_DATA_BUFFER_SIZE 4096

#define EMITTER_CURRENT_SECTION(emitter) \
    ((emitter)->current_section == ST_TEXT ? &(emitter)->text : &(emitter)->data)

static bool emitter_append_bytes(Emitter *emitter, const uint8_t *data, size_t length);

/*
 * emitter_initialize()
 *
 * Initializes the given emitter struct.
 *
 * Details:
 * - Ensures a fresh state of the emitter (Frees all allocated memory and NULLS all fields)
 * - Opens the given output file for writing
 * - Allocates memory for the section buffers
 * - Copies filename
 */
bool emitter_initialize(Emitter *emitter, const char *output_filename) 
{
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: emitter_initialize(): emitter is NULL pointer\n");
        return false;
    }
    if (output_filename == NULL) {
        fprintf(stderr, "Internal Error: emitter_initialize(): output_filename is NULL pointer\n");
        return false;
    }

    emitter_free(emitter);
    memset(emitter, 0, sizeof(*emitter));

    emitter->file = fopen(output_filename, "wb");
    if (emitter->file == NULL) {
        fprintf(stderr, "Error: cannot open output file '%s'\n", output_filename);
        return false;
    }

    emitter->filename = strdup(output_filename);
    if (emitter->filename == NULL) {
        fprintf(stderr, "Internal Error: emitter_initialize(): strdup failed\n");
        fclose(emitter->file);
        emitter->file = NULL;
        return false;
    }

    emitter->text.buffer = malloc(EMITTER_INITIAL_TEXT_BUFFER_SIZE);
    if (emitter->text.buffer == NULL) {
        fprintf(stderr, " Error: emitter_initialize(): memory allocation for buffer failed\n");
        fclose(emitter->file);
        emitter->file = NULL;
        free((void*)emitter->filename);
        emitter->filename = NULL;
        return false;
    }

    emitter->data.buffer = malloc(EMITTER_INITIAL_DATA_BUFFER_SIZE);
    if (emitter->data.buffer == NULL) {
        fprintf(stderr, " Error: emitter_initialize(): memory allocation for buffer failed\n");
        fclose(emitter->file);
        emitter->file = NULL;
        free((void*)emitter->filename);
        emitter->filename = NULL;
        free(emitter->text.buffer);
        emitter->text.buffer = NULL;
        return false;
    }

    emitter->current_section     = ST_TEXT;
    emitter->text.capacity       = EMITTER_INITIAL_TEXT_BUFFER_SIZE;
    emitter->text.size           = 0; 
    emitter->text.offset         = 0;
    emitter->data.capacity       = EMITTER_INITIAL_DATA_BUFFER_SIZE;
    emitter->data.size           = 0; 
    emitter->data.offset         = 0;
    emitter->is_initialized      = true;   
    
    return true;
}

/* 
 * emitter_free()
 * 
 * Frees the given emitter struct.
 *
 * Details:
 * - Closes the output file
 * - Frees allocated memory of all section buffers + copied filename
 */
bool emitter_free(Emitter *emitter) 
{
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: in emitter_free(): emitter is NULL pointer\n");
        return false;
    }

    if (emitter->file != NULL) {
        fclose(emitter->file);
        emitter->file = NULL;
    }
    if (emitter->text.buffer != NULL) {
        free(emitter->text.buffer);
        emitter->text.buffer = NULL;
    }
    if (emitter->data.buffer != NULL) {
        free(emitter->data.buffer);
        emitter->data.buffer = NULL;
    }

    if (emitter->filename != NULL) {
        free((void*)emitter->filename);   
        emitter->filename = NULL;
    }

    emitter->is_initialized = false;

    return true;
}

/*
 * emitter beginn_pass1()
 * 
 * Readies the emittter fiels for parser pass 1.
 *
 * Details:
 * - Sets mode to EMIT_MODE_LAYOUT (thereby preventing any emitting during parser pass 1)
 * - Makes 'text' the default section
 * - Nulls the section offsets + size trackers
 */
bool emitter_begin_pass1(Emitter *emitter) {
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: emitter_beginn_pass1(): emitter is NULL pointer\n");
        return false;
    }
    emitter->mode = EMIT_MODE_LAYOUT;
    emitter->current_section = ST_TEXT;
    emitter->text.offset = 0; emitter->text.size = 0;
    emitter->data.offset = 0; emitter->data.size = 0;

    return true;
}

/*
 * emitter beginn_pass2()
 * 
 * Readies the emitter fields for parser pass 2.
 *
 * Details:
 * - Sets mode to EMIT_MODE_REAL (thereby explicitly allowing emitting during parser pass 2)
 * - Makes 'text' the default section
 * - Nulls the section offsets + size trackers
 */
bool emitter_begin_pass2(Emitter *emitter) {
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: emitter_beginn_pass2(): emitter is NULL pointer\n");
        return false;
    }
    emitter->mode = EMIT_MODE_REAL;
    emitter->current_section = ST_TEXT;
    emitter->text.offset = 0; emitter->text.size = 0;
    emitter->data.offset = 0; emitter->data.size = 0;

    return true;
}

/* 
 * emitter_append_bytes()
 * / also referenced as 'the byte appender'
 
 * Appends a given amount of bytes to the current section buffer.
 * 
 * Details:
 * - Checks the capacity off the current buffer and doubles the size if it's to small.
 */
static bool emitter_append_bytes(Emitter *emitter, const uint8_t *data, size_t length)
{   
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: emitter_append_bytes(): emitter is NULL pointer\n");
        return false;
    }
    if (data == NULL) {
        fprintf(stderr, "Internal Error: emitter_append_bytes(): data is NULL poiter\n");
        return false;
    }
    if (length == 0) {
        fprintf(stderr, "Internal Error: emitter_append_bytes(): length must be > 0\n");
        return false;
    }
    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal Error: emitter_append_bytes(): emitter not initialized\n");
        return false;
    }
    if (emitter->mode != EMIT_MODE_REAL) {
        fprintf(stderr, "Internal Error: emitter_append_bytes(): emitter mode must EMIT_MODE_REAL\n");
        return false;
    }
    SectionBuffer *section = EMITTER_CURRENT_SECTION(emitter);
    if (section->size + length > section->capacity) {
        size_t new_capacity = section->capacity * 2;
        if (new_capacity < section->size + length) {
            new_capacity = section->size + length;   
        }

        uint8_t *temp = realloc(section->buffer, new_capacity);
        if (temp == NULL) {
            fprintf(stderr, "Internal Error: emitter_append_bytes(): buffer reallocation failed\n");
            return false;
        }

        section->buffer   = temp;
        section->capacity = new_capacity;
    }

    memcpy(&section->buffer[section->size], data, length);

    section->size          += length;
    section->offset        += (uint32_t)length;

    return true;
}

/*
 * emitter_emit_byte()
 *
 * Sends a given byte to the byte appender.
 */
bool emitter_emit_byte(Emitter *emitter, uint8_t value) 
{
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: emitter_emit_byte(): emitter is NULL pointer\n");
        return false;
    }
    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal Error: emitter_emit_byte(): emitter not initialized\n");
        return false;
    }
    if (emitter->mode != EMIT_MODE_REAL) {
        fprintf(stderr, "Internal Error: emitter_emit_byte(): emitter mode must be EMIT_MODE_REAL\n");
        return false;
    }

    uint8_t b  = value;

    return emitter_append_bytes(emitter, &b, 1);
}

/*
 * emitter_emit_half()
 * 
 * Splits a given 16 bit value into 2 bytes and appends them to the byte appender.
 */
bool emitter_emit_half(Emitter *emitter, uint16_t value)
{
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: emitter_emit_half(): emitter is NULL pointer\n");
        return false;
    }
    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal Error: emitter_emit_half(): emitter not initialized\n");
        return false;
    }
    if (emitter->mode != EMIT_MODE_REAL) {
        fprintf(stderr, "Internal Error: emitter_append_half(): emitter mode must be EMIT_MODE_REAL\n");
        return false;
    }

    uint8_t b[2] = {
        (uint8_t)(value & (uint16_t) 0xFF), 
        (uint8_t)(value >> 8)
    };

    return emitter_append_bytes(emitter, b, 2);
}

/*
 * emitter_emit_word()
 * 
 * Splits a given 32 bit value into 4 bytes and sends them to the byte appender.
 */
bool emitter_emit_word(Emitter *emitter, uint32_t value) 
{
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: emitter_emit_word(): emitter is NULL pointer\n");
        return false;
    }
    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal Error: emitter_emit_word(): emitter not initialized\n");
        return false;
    }
    if (emitter->mode != EMIT_MODE_REAL) {
        fprintf(stderr, "Internal Error: emitter_append_word(): emitter mode must EMIT_MODE_REAL\n");
        return false;
    }

    uint8_t b[4] = {
        (uint8_t)(value & (uint32_t)0xFF), 
        (uint8_t)((value >> 8) & (uint32_t)0xFF),
        (uint8_t)((value >> 16) & (uint32_t)0xFF),
        (uint8_t)(value >> 24),
    };

    return emitter_append_bytes(emitter, b, 4);
}

/*
 * emitter_emit_bytes()
 * 
 * Sends a given amount of bytes to the byte appender.
 */
bool emitter_emit_bytes(Emitter *emitter, const uint8_t *data, size_t length)
{
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: emitter_emit_bytes(): emitter is NULL pointer\n");
        return false;
    }
    if (data == NULL) {
        fprintf(stderr, "Internal Error: emitter_emit_bytes(): data is NULL poiter\n");
        return false;
    }
    if (length == 0) {
        fprintf(stderr, "Internal Error: emitter_emit_bytes(): length must be > 0\n");
        return false;
    }
    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal Error: emitter_emit_bytes(): emitter not initialized\n");
        return false;
    }
    if (emitter->mode != EMIT_MODE_REAL) {
        fprintf(stderr, "Internal Error: emitter_emit_bytes(): emitter mode must be EMIT_MODE_REAL\n");
        return false;
    }

    return emitter_append_bytes(emitter, data, length);
}

/*
 * emitter_align()
 *
 * Adjusts the emitters state to the given alignment.
 *
 * Details:
 * - Calculates amount of padding bytes needed.
 * - Adjusts the current sections offset to the padding.
 * - If emitter is in EMIT_MODE_REAL(parser pass 2): emits the padding bytes in the current sections buffer.
 */
bool emitter_align(Emitter *emitter, uint32_t alignment) 
{
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: emitter_align(): emitter is NULL pointer\n");
        return false;
    }

    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal Error: emitter_align(): emitter not initialized\n");
        return false;
    }

    if (alignment == 0 || alignment == 1) {
        return true;
    }

    SectionBuffer *section = EMITTER_CURRENT_SECTION(emitter);

    uint32_t align    = (1u << alignment);
    uint32_t base     = section->offset;
    uint32_t padding  = ((base + (align - 1u)) & ~(align - 1u)) - base;
    
    if (emitter->mode == EMIT_MODE_LAYOUT) { /* no emission in layout mode (meaning parser pass 1) */
        section->offset += padding;
        return true;
    }

    for(size_t i = 0; i< padding; i++) {
        if(!emitter_emit_byte(emitter, 0)) {
            return false;
        }
    }

    return true;
}

/*
 * emitter_current_offset()
 *
 * Writes the offset of the current sections buffer into the given output.
 */
bool emitter_current_offset(Emitter *emitter, uint32_t *out_offset)
{
    if (emitter == NULL) {
        fprintf(stderr, "Internal error: emitter_get_section_offset(): emitter is NULL pointer\n");
        return false;
    }
     if (out_offset == NULL) {
        fprintf(stderr, "Internal error: emitter_current_offset(): out_offset is NULL pointer\n");
        return false;
    }
    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal error: emitter_current_offset(): emitter not initialized\n");
        return false;
    }

    SectionBuffer *section = EMITTER_CURRENT_SECTION(emitter);
    *out_offset = section->offset;
    return true;
}

/*
 * emitter_get_section_offset()
 *
 * Writes the offset of the sections buffer (corresponding to the given section type) into the given output.
 */
bool emitter_get_section_offset(Emitter *emitter, SectionType section_type, uint32_t *out_offset)
{
     if (emitter == NULL) {
        fprintf(stderr, "Internal error: emitter_get_section_offset(): emitter is NULL pointer\n");
        return false;
    }
     if (out_offset == NULL) {
        fprintf(stderr, "Internal error: emitter_get_section_offset(): out_offset is NULL pointer\n");
        return false;
    }
    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal error: emitter_get_section_offset(): emitter not initialized\n");
        return false;
    }

    switch (section_type) {
        case ST_TEXT:   *out_offset = emitter->text.offset; return true;
        case ST_DATA:   *out_offset = emitter->data.offset; return true;
        default:
            fprintf(stderr, "Error: emitter_get_section_offset(): unsupported section type %d\n", section_type);
            return false;
    }
}

/*
 * emitter_switch_section()
 * 
 * Sets the emitters current_section field to the given section type.
 */
bool emitter_switch_section(Emitter *emitter, SectionType section_type) 
{
    if (emitter == NULL) {
        fprintf(stderr, "Internal error: emitter_switch_section(): emitter is NULL pointer\n");
        return false;
    }
    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal error: emitter_switch_section(): emitter not initialized\n");
        return false;
    }
    if (section_type != ST_TEXT && section_type != ST_DATA) {
        fprintf(stderr, "Error: emitter_switch_section(): unsupported section type\n");
        return false;
    }

    emitter->current_section = section_type;
    return true;
}

/* 
 * emitter_advance()
 *
 * Advances the offset of the emitters current sections buffer.
 *
 * Details:
 * - Is only used during parser pass 1 to keep track of the current location.
 */
bool emitter_advance(Emitter *emitter, uint32_t bytes)
{
     if (emitter == NULL) {
        fprintf(stderr, "Internal error: emitter_advance(): emitter is NULL pointer\n");
        return false;
    }
    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal error: emitter_advance(): emitter not initialized\n");
        return false;
    }
    if (emitter->mode != EMIT_MODE_LAYOUT) {
        fprintf(stderr, "Internal error: emitter_advance(): emitter mode must be EMIT_MODE_LAYOUT\n");
        return false;
    }
   
    SectionBuffer *section = EMITTER_CURRENT_SECTION(emitter);
    
    section->offset += bytes;

    /* section->size stays 0 until real emission in Pass 2 (buffers filled later) */
    return true;
}

/*
 * emitter_finish()
 * 
 * Writes all section buffers into the output file.
 *
 * Details:
 * - Writes the full text buffer first, then the full data buffer into the output file.
 * - Closes the output file.
 */
bool emitter_finish(Emitter *emitter) 
{
    if (emitter == NULL) {
        fprintf(stderr, "Internal Error: emitter_finish(): emitter is NULL pointer\n");
        return false;
    }
    if (!emitter->is_initialized) {
        fprintf(stderr, "Internal Error: emitter_finish(): emitter not initialized\n");
        return false;
    }
    if (emitter->mode != EMIT_MODE_REAL) {
        fprintf(stderr, "Internal error: emitter_finish(): emitter mode must be EMIT_MODE_REAL\n");
        return false;
    }

    size_t written_bytes = fwrite(emitter->text.buffer, 1, emitter->text.size, emitter->file);
    written_bytes += fwrite(emitter->data.buffer, 1, emitter->data.size, emitter->file);

    if (written_bytes != emitter->text.size + emitter->data.size) {
        fprintf(stderr, "Error: only wrote %zu of %zu bytes to '%s'\n", 
                        written_bytes, emitter->text.size + emitter->text.size, emitter->filename);
        return false;
    }
    fclose(emitter->file);
    emitter->file = NULL;
    return true;
}

