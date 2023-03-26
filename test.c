#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(void){
    uint16_t a = 0x6468;
    uint8_t *b = calloc(100, sizeof(uint8_t));
    for(int i = 0; i < 100; i++){
        b[i] = 0;
    }
    uint32_t bit_index = 0;
    printf("a=%X\n", a);
    for(uint32_t i = 0; i < 13; i++){
        if((0x8000 >> i) & a){
            b[bit_index/8] |= (0x80 >> (bit_index % 8));
        }
        bit_index++;
    }
    printf("%X%X%X\n", b[0], b[1], b[2]);
    free(b);
    return 0;
}
