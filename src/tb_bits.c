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