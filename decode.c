#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <gmp.h>
#include "code.h"
#include "io.h"
#include "word.h"
#include "set.h"

#define OPTIONS "vi:o:h"

int bit_length(uint16_t);

char help_msg[] = "\
SYNOPSIS\n\
   Decompresses files with the LZ78 decompression algorithm.\n\
   Used with files compressed with the corresponding encoder.\n\
\n\
USAGE\n\
   %s [-vh] [-i input] [-o output]\n\
\n\
OPTIONS\n\
   -v          Display decompression statistics\n\
   -i input    Specify input to decompress (stdin by default)\n\
   -o output   Specify output of decompressed input (stdout by default)\n\
   -h          Display program usage\n";

int main(int argc, char **argv) {

    struct stat sb;

    int infile = 0;
    int outfile = 1;

    Set optset = set_empty();
    int opt;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h': optset = set_insert(optset, 0); break;
        case 'v': optset = set_insert(optset, 1); break;
        case 'i':
            optset = set_insert(optset, 2);
            infile = open(optarg, O_RDONLY);
            break;
        case 'o':
            optset = set_insert(optset, 3);
            outfile = open(optarg, O_WRONLY | O_CREAT | O_TRUNC);
            break;
        default:
            printf(help_msg, argv[0]);
            exit(1);
            break;
        }
    }

    if (set_member(optset, 0)) {
        printf(help_msg, argv[0]);
        return 0;
    }

    if (infile == -1) {
        perror("Error opening infile : ");
        exit(1);
    }
    // Set file statistics.
    fstat(infile, &sb);

    if (outfile == -1) {
        perror("Error opening outfile : ");
        exit(1);
    }

    // Check permissions and magic number.
    if (infile != 0) {
        FileHeader header;
        read_header(infile, &header);
        fchmod(outfile, header.protection);
    }

    // Begin of decompression algorithm.
    WordTable *wt = wt_create();
    uint8_t curr_sym = 0;
    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;
    while (read_pair(infile, &curr_code, &curr_sym, bit_length(next_code))) {
        wt[next_code] = word_append_sym(wt[curr_code], curr_sym);
        write_word(outfile, wt[next_code]);
        next_code++;
        if (next_code == MAX_CODE) {
            wt_reset(wt);
            next_code = START_CODE;
        }
    }
    flush_words(outfile);

    // Free table.
    wt_reset(wt);
    wt_delete(wt);
    // Print statistics if specified.
    if (set_member(optset, 1)) {
        mpf_t a, b;
        mpf_init_set_ui(a, total_bits / 8);
        mpf_init_set_ui(b, total_syms);
        mpf_div(a, a, b);
        mpf_ui_sub(a, 1, a);
        mpf_mul_ui(a, a, 100);
        fprintf(stderr, "Compressed file size: %lu bytes\n", total_bits / 8);
        fprintf(stderr, "Uncompressed file size: %lu bytes\n", total_syms);
        gmp_fprintf(stderr, "Space saving:%.2Ff%\n", a);
        mpf_clears(a, b, NULL);
    }

    close(infile);
    close(outfile);
    return 0;
}

int bit_length(uint16_t a) {
    int bits = 0;
    while (a != 0) {
        a = a >> 1;
        bits++;
    }
    return bits;
}
