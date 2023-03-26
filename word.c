#include <stdlib.h>
#include <string.h>
#include "code.h"
#include "word.h"

// Create word with syms and length len.
// Returns word pointer.
//
// syms : pointer to array
// len : length of syms
Word *word_create(uint8_t *syms, uint32_t len) {
    Word *w = malloc(sizeof(Word));
    w->syms = calloc(len, sizeof(uint8_t));
    // Copy contents of syms to w->syms.
    // Could also use memcpy().
    memcpy(w->syms, syms, len);
    /*
    for (uint32_t i = 0; i < len; i++) {
        w->syms[i] = syms[i];
    }
    */
    w->len = len;
    return w;
}

// Append symbol to a given word.
// Returns new word with new symbol.
//
// w : word pointer
// sym : symbol to add
Word *word_append_sym(Word *w, uint8_t sym) {
    uint8_t *temp;
    uint32_t len;
    if (w != NULL) {
        temp = calloc(w->len + 1, sizeof(uint8_t));
        memcpy(temp, w->syms, w->len);
        len = w->len + 1;
        // If appending empty word, treat as word_create().
    } else {
        temp = calloc(1, sizeof(uint8_t));
        temp[0] = sym;
        len = 1;
    }
    Word *nw = word_create(temp, len);
    free(temp);
    //    nw->syms = realloc(nw->syms, nw->len);
    nw->syms[nw->len - 1] = sym;
    return nw;
}

// Delete word w.
//
// w : word pointer
void word_delete(Word *w) {
    // Ensure w not NULL.
    if (w) {
        free(w->syms);
    }
    free(w);
}

// Create word table containing only the empty word.
// Returns pointer to word table.
WordTable *wt_create(void) {
    WordTable *wt = calloc(MAX_CODE, sizeof(Word *));
    wt[EMPTY_CODE] = NULL;
    return wt;
}

// Reset word table wt. Deletes all word but empty word.
//
// wt : word table pointer
void wt_reset(WordTable *wt) {
    for (int i = START_CODE; i < MAX_CODE; i++) {
        word_delete(wt[i]);
        wt[i] = NULL;
    }
}

// Delete word table.
//
// wt : word table pointer
void wt_delete(WordTable *wt) {
    word_delete(wt[EMPTY_CODE]);
    free(wt);
}
