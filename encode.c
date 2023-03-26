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
#include "trie.h"
#include "word.h"
#include "set.h"

#define OPTIONS "vi:o:h"

int bit_length(uint16_t);

char help_msg[] = "\
SYNOPSIS\n\
   Compresses files using the LZ78 compression algorithm.\n\
   Compressed files are decompressed with the corresponding decoder.\n\
\n\
USAGE\n\
   %s [-vh] [-i input] [-o output]\n\
\n\
OPTIONS\n\
   -v          Display compression statistics\n\
   -i input    Specify input to compress (stdin by default)\n\
   -o output   Specify output of compressed input (stdout by default)\n\
   -h          Display program help and usage\n";

int main(int argc, char **argv) {

    // stat Struct for getting file info.
    struct stat sb;

    // Initialize header.
    FileHeader header;
    memset(&header, 0, sizeof(FileHeader));

    int infile = 0;
    int outfile = 1;

    // Variables for options.
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
        default: fprintf(stderr, help_msg, argv[0]); exit(1);
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

    // Change outfile permissions to infile's.
    fchmod(outfile, sb.st_mode);

    if (outfile == -1) {
        perror("Error opening outfile : ");
        exit(1);
    }

    // Write protetion bits and magic number.
    // Don't write if stdout specified.
    if (outfile != 1) {
        header.protection = sb.st_mode;
        header.magic = 0xBAADBAAC;
        write_header(outfile, &header);
    }

    // Begin of compression algorithm.
    TrieNode *root = trie_create();
    TrieNode *curr_node = root;
    TrieNode *prev_node = NULL;
    TrieNode *next_node = NULL;
    uint8_t curr_sym = 0;
    uint8_t prev_sym = 0;
    uint16_t next_code = START_CODE;

    while (read_sym(infile, &curr_sym)) {
        next_node = trie_step(curr_node, curr_sym);
        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            write_pair(outfile, curr_node->code, curr_sym, bit_length(next_code));
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code++;
        }
        if (next_code == MAX_CODE) {
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }
        prev_sym = curr_sym;
    }
    if (curr_node != root) {
        write_pair(outfile, prev_node->code, prev_sym, bit_length(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }
    write_pair(outfile, STOP_CODE, 0, bit_length(next_code));
    flush_pairs(outfile);

    // Free our trie.
    trie_delete(root);

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

// Helper function that returns bit length of a.
// Returns bit length.
//
// a : unsigned integer
int bit_length(uint16_t a) {
    int bits = 0;
    while (a != 0) {
        a = a >> 1;
        bits++;
    }
    return bits;
}
