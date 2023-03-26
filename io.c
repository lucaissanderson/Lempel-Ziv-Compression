#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "word.h"
#include "code.h"
#include "endian.h"
#include "io.h"

static void reset_buffer(uint8_t *);

static uint8_t char_buf[BLOCK];
static uint8_t bit_buf[BLOCK];
static uint16_t bit_index = 0;
static uint16_t char_index = 0;
// Terminal (last) index for char_index.
static uint16_t terminal = BLOCK;

uint64_t total_bits = 0;
uint64_t total_syms = 0;

// Helper function to set all elements in buffer to zero.
//
// buf : pointer to buffer array
void reset_buffer(uint8_t *buf) {
    memset(buf, 0, BLOCK);
}

// Helper to ensure to_read bytes are read into buf.
// Returns number of bytes actually read.
//
// infile : file descriptor; needs be be readable
// buf : pointer to buffer array
// to_read : number of bytes to read
int read_bytes(int infile, uint8_t *buf, int to_read) {
    int bytes = 0;
    int temp;
    do {
        temp = read(infile, buf + bytes, to_read - bytes);
        bytes += temp;
        // Stop when to_read bytes are read OR
        // read() reads 0 bytes, thus EOF.
    } while (bytes < to_read && temp);
    return bytes;
}

// Helper to ensure to_write bytes are written from buf.
// Returns number of bytes actually written.
//
// outfile : file descriptor; needs to be writable
// buf : buffer array pointer
// to_write : bytes to write
int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int bytes = 0;
    int temp;
    do {
        temp = write(outfile, buf, to_write - bytes);
        bytes += temp;
        // Stop when to_write bytes are written OR
        // write() writes 0 bytes.
    } while (bytes < to_write && temp);
    return bytes;
}

// Reads header from infile to header object.
//
// infile : file descriptor; needs to be readable
// header : header pointer
void read_header(int infile, FileHeader *header) {
    // Update total bits.
    total_bits += (uint64_t) 8 * read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));
    // Check if system is big endian, swap if necessary.
    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
    // Check magic number.
    if (header->magic != MAGIC) {
        fprintf(stderr, "Bad magic number.\n");
        exit(1);
    }
}

// Writes header to outfile from header object.
//
// outfile : file descriptor; needs to be writable
// header : file header
void write_header(int outfile, FileHeader *header) {
    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->magic);
    }
    total_bits += (uint64_t) 8 * write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader));
}

// Read symbol (character) from infile, place into sym.
// Returns boolean: true if characters are left and false otherwise.
//
// infile : file descriptor; open and readable
// sym : address of sym
bool read_sym(int infile, uint8_t *sym) {
    // If position is at the beginning of the buffer or
    // end, then read in more characters to buffer.
    if (char_index == BLOCK || char_index == 0) {
        terminal = read_bytes(infile, char_buf, BLOCK);
        total_syms += (uint64_t) terminal;
        char_index = 0;
    }
    // If end of buffer less than size of buffer and
    // position is at end of buffer, then end loop.
    if (terminal < BLOCK && char_index == terminal) {
        return false;
    }
    // Set character and increment.
    *sym = char_buf[char_index];
    char_index++;
    return true;
}

// Write code and symbol pair to buffer and dump to file if full.
//
// outfile : file descriptor; open and writable
// code : code of current symbol
// sym : character to write
// bitlen : bit length of next node (see encode.c)
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    // Only write bitlen bits from code.
    for (int i = 0; i < bitlen; i++) {
        // Apply bitmask.
        // If a 1, write a 1 to next bit in buffer,
        // else write a 0.
        // (unnecessary to write a 0 since we reset the buffer
        //  but want to explicitly show it)
        if ((0x1 << i) & code) {
            bit_buf[bit_index / 8] |= (0x01 << (bit_index % 8));
        } else {
            bit_buf[bit_index / 8] &= ~(0x01 << (bit_index % 8));
        }
        bit_index++;
        // Check if end of buffer.
        // If it is, write to file and reset.
        if ((bit_index / 8) == BLOCK) {
            flush_pairs(outfile);
            bit_index = 0;
            reset_buffer(bit_buf);
        }
    }
    // Likewise for sym, but it is always 8 bits.
    for (int i = 0; i < 8; i++) {
        if ((0x1 << i) & sym) {
            bit_buf[bit_index / 8] |= (0x01 << (bit_index % 8));
        } else {
            bit_buf[bit_index / 8] &= ~(0x01 << (bit_index % 8));
        }
        bit_index++;
        if ((bit_index / 8) == BLOCK) {
            flush_pairs(outfile);
            bit_index = 0;
            reset_buffer(bit_buf);
        }
    }
}

// Writes buffer to outfile.
//
// outfile : file descriptor; must be open and writable
void flush_pairs(int outfile) {
    total_bits += (uint64_t) 8 * write_bytes(outfile, bit_buf, (bit_index / 8));
}

// Sort of inverse of write_pair(). Read code-sym pair into buffer.
bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    *code = 0;
    *sym = 0;
    for (int i = 0; i < bitlen; i++) {
        // Refill buffer if at end or beginning.
        if ((bit_index / 8) == BLOCK || 0 == bit_index) {
            total_bits += (uint64_t) 8 * read_bytes(infile, bit_buf, BLOCK);
            bit_index = 0;
        }
        // Bitmask buffer element and set given bit.
        if ((0x01 << (bit_index % 8)) & bit_buf[bit_index / 8]) {
            *code |= 1 << i;
        }
        bit_index++;
    }
    for (int i = 0; i < 8; i++) {
        if ((bit_index / 8) == BLOCK || 0 == bit_index) {
            total_bits += (uint64_t) 8 * read_bytes(infile, bit_buf, BLOCK);
            bit_index = 0;
        }
        if ((0x01 << (bit_index % 8)) & bit_buf[bit_index / 8]) {
            *sym |= 1 << i;
        }
        bit_index++;
    }
    // Stop when reach STOP_CODE.
    if (*code == STOP_CODE) {
        return false;
    }
    return true;
}

// Write a word to outfile.
//
// outfile : file descriptor; open and writable
// w : word
void write_word(int outfile, Word *w) {
    // Write each symbol in word.
    for (uint32_t i = 0; i < w->len; i++) {
        // Write out buffer if at end.
        if (char_index == BLOCK) {
            flush_words(outfile);
            char_index = 0;
        }
        char_buf[char_index] = w->syms[i];
        char_index++;
    }
}

// Write out buffer.
//
// outfile : file descriptor; open and writable.
void flush_words(int outfile) {
    total_syms += (uint64_t) write_bytes(outfile, char_buf, char_index);
}
