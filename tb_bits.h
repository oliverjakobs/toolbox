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

/*
 * -----------------------------------------------------------------------------
 * ----| IMPLEMENTATION |-------------------------------------------------------
 * -----------------------------------------------------------------------------
 */

#ifdef TB_BITS_IMPLEMENTATION

#include "tb_bits.h"

uint8_t tb_bits_set(uint8_t value, uint8_t position)
{
    return value | (1 << position);
}

uint8_t tb_bits_clear(uint8_t value, uint8_t position)
{
    return value & ~(1 << position);
}

uint8_t tb_bits_flip(uint8_t value, uint8_t position)
{
    return value ^ (1 << position);
}

uint8_t tb_bits_get(uint8_t value, uint8_t position)
{
    return (value & (1 << position)) > 0;
}

uint8_t tb_bits_count_set(uint8_t value)
{
    uint8_t count = 0;
    while (value)
    {
        value &= (value - 1);
        count++;
    }
    return count;
}

char* tb_bits_str(char* buf, uint8_t value)
{
    uint8_t bit = 0;
    uint8_t bits = sizeof(uint8_t) * 8;

    for (uint8_t i = 1 << (bits - 1); i > 0; i = i / 2)
        buf[bit++] = (value & i) ? '1' : '0'; 

    buf[bit] = '\0';

    return buf;
}

#endif /* !TB_BITS_IMPLEMENTATION */

/*
MIT License

Copyright (c) 2020 oliverjakobs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/