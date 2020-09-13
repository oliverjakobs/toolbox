#include "../src/tb_bits.h"

#include <stdio.h>

int main()
{
    char buf[32];

    uint8_t i = 23;

    printf("%d: %s\n", i, tb_bits_str(buf, i));

    printf("Set Bits: %d\n", tb_bits_count_set(i));

    i = tb_bits_set(i, 3);
    printf("%d: %s\n", i, tb_bits_str(buf, i));

    i = tb_bits_clear(i, 0);
    printf("%d: %s\n", i, tb_bits_str(buf, i));

    i = tb_bits_flip(i, 4);
    printf("%d: %s\n", i, tb_bits_str(buf, i));

    return 0;
}