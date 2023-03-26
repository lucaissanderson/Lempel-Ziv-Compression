#include "set.h"

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t Set;

// Returns empty set. (0)
Set set_empty(void) {
    Set s;
    s = 0;
    return s;
}

// Returns set containing all 1's.
Set set_universal(void) {
    Set s;
    s = 1;
    return s;
}

// Returns set with given index
// turned "on".
// (set to 1, if it isn't already)
//
// s : Set struct
// x : index of interest in s (from right)
Set set_insert(Set s, uint8_t x) {
    uint32_t bit = 1 << x;
    return s | bit;
}

// Returns set with given index
// turned "off". (set to 0)
//
// s : Set struct
// x : index of interest in s (from right)
Set set_remove(Set s, uint8_t x) {
    uint32_t bit = ~(1 << x);
    return s & bit;
}

// Returns boolean value.
// True if given index is set to
// 1, else false.
//
// s : Set struct
// x : given index of interest
bool set_member(Set s, uint8_t x) {
    uint32_t bit = 1 << x;
    if (s & bit) {
        return true;
    }
    return false;
}

// Returns set union.
//
// s,t : Set struct
Set set_union(Set s, Set t) {
    return s | t;
}

// Returns intersection of
// sets.
//
// s,t : Set struct
Set intersect(Set s, Set t) {
    return s & t;
}

// Returns set difference.
//
// s,t : Set struct
Set set_difference(Set s, Set t) {
    return s & ~(t);
}

Set set_complement(Set s) {
    return ~(s);
}
