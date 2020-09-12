#ifndef TB_BITS_H
#define TB_BITS_H

#include <stdint.h>

/* Set the bit at the given position to 1. */
uint8_t tb_bits_set(uint8_t value, uint8_t position);
/* Set the bit at the given position to 0. */
uint8_t tb_bits_clear(uint8_t value, uint8_t position);
/* Flip the bit at the given position. */
uint8_t tb_bits_flip(uint8_t value, uint8_t position);
/* Return the bit at the given position. */
uint8_t tb_bits_get(uint8_t value, uint8_t position);

/*
 * Count the set bits (1) int the binary representation of value
 * using Brian Kernighan’s algorithm.
 */
uint8_t tb_bits_count_set(uint8_t value);

/* 
 * Fill buf with binary representation of value.
 * Make sure buf is big enough.
 */
char* tb_bits_str(char* buf, uint8_t value);

#endif /* !TB_BITS_H */
