#ifndef __SET_H__
#define __SET_H__

#include <stdbool.h>
#include <stdint.h>

typedef uint32_t Set;

Set set_empty(void);

Set set_universal(void);

Set set_insert(Set s, uint8_t x);

Set set_remove(Set s, uint8_t x);

bool set_member(Set s, uint8_t x);

Set set_union(Set s, Set t);

Set set_intersect(Set s, Set t);

Set set_difference(Set s, Set t);

Set set_complement(Set s);

#endif
