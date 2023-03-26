#include <stdlib.h>
#include "trie.h"
#include "code.h"

// Create a trie node with code index.
// Returns pointer to trie node.
//
// index : code of trie node
TrieNode *trie_node_create(uint16_t index) {
    TrieNode *node = malloc(sizeof(TrieNode));
    node->code = index;
    for (int i = 0; i < ALPHABET; i++) {
        node->children[i] = NULL;
    }
    return node;
}

// Delete trie node.
//
// n : trie node pointer
void trie_node_delete(TrieNode *n) {
    free(n);
}

// Create root node with default code EMPTY_CODE.
// Returns trie node pointer.
TrieNode *trie_create(void) {
    TrieNode *root = trie_node_create(EMPTY_CODE);
    return root;
}

// Reset a trie; deletes all of root's children.
//
// root : trie node pointer (root)
void trie_reset(TrieNode *root) {
    for (int i = 0; i < ALPHABET; i++) {
        if (root->children[i] != NULL) {
            trie_delete(root->children[i]);
            root->children[i] = NULL;
        }
    }
}

// Delete a subtrie from root n. (including n)
//
// n : pointer to root of subtrie
void trie_delete(TrieNode *n) {
    // For all of n's children.
    for (int i = 0; i < ALPHABET; i++) {
        // Base case. At leaf node.
        if (n->children[i] == NULL) {
            // Doesn't seem like I need to do anything
            // in the base case. The trie is already
            // NULL and no memory was allocated to it.
            // ¯\_(ツ)_/¯
            // Recursive call. Go to next child.
        } else {
            trie_delete(n->children[i]);
        }
    }
    trie_node_delete(n);
    n = NULL;
}

// Step down to n's child, sym.
// Return's trie node pointer of child sym.
//
// n : trie node pointer
// sym : index of n to step down to
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    for (int i = 0; i < ALPHABET; i++) {
        if (i == sym) {
            return n->children[sym];
        }
    }
    // If invalid sym, NULL returned.
    return NULL;
}
